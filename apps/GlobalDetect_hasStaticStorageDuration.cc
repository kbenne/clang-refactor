// GlobalDetect.cc
// Oct. 6, 2016
// (c) Copyright 2016-7 LANSLLC, all rights reserved

//#include "global_matchers.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include <iostream>
#include "summarize_command_line.h"
#include "dump_things.h"
#include "types.h"
#include "utilities.h"
#include "make_replacement.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Basic/SourceLocation.h"
#include <iostream>

using namespace clang::tooling;
using namespace llvm;
using namespace clang::ast_matchers;

const char * addl_help =
    "Report all functions that use global variable, or all sites at which "
    "global variables are used";

// command line options:
static llvm::cl::OptionCategory GDOpts("global-detect options");

class Printer : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  virtual void
  run(corct::result_t const & result) override
  {
    using namespace clang;
    using namespace corct;

    n_matches_++;
    VarDecl const * var = result.Nodes.getNodeAs<VarDecl>("gvarName");
    clang::SourceManager & src_manager(
        const_cast<clang::SourceManager &>(result.Context->getSourceManager()));
    if(var) {
      s_ << "'" << var->getNameAsString() << "' declared at ";
      string_t sr(sourceRangeAsString(var->getSourceRange(), &src_manager));
      s_ << sr;
      s_ << std::endl;
    }
    else {
      check_ptr(var,"var","",s_);
    }
    return;
  }  // run

  explicit Printer(std::ostream & s)
      : s_(s),
        n_matches_(0)
  {}

  std::ostream & s_;
  uint32_t n_matches_;
};  // class Global_Printer

using replacement_map_t = std::map< std::string, Replacements >;

std::string decl2str(clang::VarDecl const *d, clang::SourceManager & sm) {
    clang::LangOptions lopt;
    clang::SourceLocation b(d->getLocStart()), _e(d->getLocEnd());
    clang::SourceLocation e(clang::Lexer::getLocForEndOfToken(_e, 0, sm, lopt));
    return std::string(sm.getCharacterData(b),
        sm.getCharacterData(e) - sm.getCharacterData(b));
}

int
main(int argc, const char ** argv)
{
  using namespace corct;
  using namespace clang::ast_matchers;

  CommonOptionsParser OptionsParser(argc, argv, GDOpts,addl_help);
  RefactoringTool tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  Printer printer(std::cout);

  DeclarationMatcher matcher =
    namespaceDecl(
        hasName("EnergyPlus"),
        forEachDescendant(
          varDecl(
            hasStaticStorageDuration()
          ).bind("gvarName")
        )
    );

  clang::ast_matchers::MatchFinder finder;
  finder.addMatcher(matcher, &printer);

  tool.runAndSave(newFrontendActionFactory(&finder).get());

  return 0;
} // main

// End of file
