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

//
// We don't like using C-style casts in C++ code
//

namespace {

class CStyleCast:
    public RecursiveASTVisitor<CStyleCast>, public loplugin::Plugin
{
public:
    explicit CStyleCast(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCStyleCastExpr(const CStyleCastExpr * expr);
};

bool CStyleCast::VisitCStyleCastExpr(const CStyleCastExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    // casting to void is typically used when a parameter or field is only used in
    // debug mode, and we want to eliminate an "unused" warning
    if( expr->getCastKind() == clang::CastKind::CK_ToVoid ) {
        return true;
    }
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(
                              expr->getLocStart());
    StringRef filename = compiler.getSourceManager().getFilename(spellingLocation);
    if ( filename.endswith(".h") || filename.endswith(".c") ) {
        return true;
    }
    if ( compat::isInMainFile(compiler.getSourceManager(), spellingLocation)
        ? (filename == SRCDIR "/nowhere")
        : (filename.startswith(SRCDIR "/include/tools/solar.h")) ) {
        return true;
    }
    report(
        DiagnosticsEngine::Warning,
        "c-style cast",
        expr->getSourceRange().getBegin())
      << expr->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< CStyleCast > X("cstylecast");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
