/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

#include "plugin.hxx"
#include "compat.hxx"

// Having an extern prototype for a method in a module and not actually declaring that method is dodgy.
//

namespace {

class ExternAndNotDefined:
    public RecursiveASTVisitor<ExternAndNotDefined>, public loplugin::Plugin
{
public:
    explicit ExternAndNotDefined(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFunctionDecl(const FunctionDecl * decl);
};

bool ExternAndNotDefined::VisitFunctionDecl(const FunctionDecl * functionDecl) {
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    if (!functionDecl->isExternC()) {
        return true;
    }
    if (functionDecl->hasBody()) {
        return true;
    }
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(functionDecl->getNameInfo().getLoc());
    if (!compat::isInMainFile( compiler.getSourceManager(), spellingLocation)) {
        return true;
    }
    StringRef name { compiler.getSourceManager().getFilename(spellingLocation) };
    // the filters use some kind of dynamic loading stunt
    if (name.startswith(SRCDIR "/filter/qa/")) {
        return true;
    }
    report(
        DiagnosticsEngine::Warning,
        "extern prototype",
        functionDecl->getSourceRange().getBegin())
      << functionDecl->getSourceRange();
    return true;
}


loplugin::Plugin::Registration< ExternAndNotDefined > X("externandnotdefined");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
