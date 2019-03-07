/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include <string>

#include "plugin.hxx"

// Having an extern prototype for a method in a module and not actually declaring that method is dodgy.
//

namespace {

class ExternAndNotDefined:
    public loplugin::FilteringPlugin<ExternAndNotDefined>
{
public:
    explicit ExternAndNotDefined(loplugin::InstantiationData const & data): FilteringPlugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(const FunctionDecl * decl);
};

bool ExternAndNotDefined::VisitFunctionDecl(const FunctionDecl * functionDecl) {
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    if (functionDecl->isDefined() || functionDecl->isPure()
      || (functionDecl->getLinkageAndVisibility().getLinkage()
          != ExternalLinkage)) {
        return true;
    }
    //TODO, filtering out anything template for now:
    if (functionDecl->isDependentContext()) {
        return true;
    }
    CXXRecordDecl const * r = dyn_cast<CXXRecordDecl>(functionDecl->getDeclContext());
    if (r != nullptr && r->getTemplateSpecializationKind() != TSK_Undeclared) {
        return true;
    }
    // this is the bison/flex C API, it has to be defined this way
    std::string functionName = functionDecl->getNameAsString();
    if (functionName == "yyerror" || functionName == "yyparse" || functionName == "yylex") {
        return true;
    }
    // see vcl/unx/gtk/app/gtksys.cxx, typename conflicts prevent using the right include
    if (functionName == "gdk_x11_screen_get_screen_number") {
        return true;
    }
    if (!compiler.getSourceManager().isInMainFile(functionDecl->getLocation()))
    {
        return true;
    }
    StringRef fileName { getFileNameOfSpellingLoc(functionDecl->getLocation()) };
    // the filters use some kind of dynamic loading stunt
    if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/filter/qa/")) {
        return true;
    }
    report(
        DiagnosticsEngine::Warning,
        "extern prototype in main file without definition",
        functionDecl->getLocation())
      << functionDecl->getSourceRange();
    return true;
}


loplugin::Plugin::Registration< ExternAndNotDefined > externandnotdefined("externandnotdefined");

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
