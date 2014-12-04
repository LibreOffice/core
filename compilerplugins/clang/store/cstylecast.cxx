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

    virtual void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitCStyleCastExpr(const CStyleCastExpr * expr);
};

static const char * recommendedFix(clang::CastKind ck) {
    switch(ck) {
        case CK_IntegralToPointer: return "reinterpret_cast";
        case CK_PointerToIntegral: return "reinterpret_cast";
        case CK_BaseToDerived: return "static_cast";
        default: return "???";
    }
}

bool CStyleCast::VisitCStyleCastExpr(const CStyleCastExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    // casting to void is typically used when a parameter or field is only used in
    // debug mode, and we want to eliminate an "unused" warning
    if( expr->getCastKind() == CK_ToVoid ) {
        return true;
    }
    // ignore integral-type conversions for now, there is unsufficient agreement about
    // the merits of C++ style casting in this case
    if( expr->getCastKind() == CK_IntegralCast ) {
        return true;
    }
    if( expr->getCastKind() == CK_NoOp ) {
        return true;
    }
    // ignore pointer-type conversions for now
    if( expr->getCastKind() == CK_BitCast ) {
        return true;
    }
    // ignore stuff from inside templates for now
    if( expr->getCastKind() == CK_Dependent ) {
        return true;
    }
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(
                              expr->getLocStart());
    StringRef filename = compiler.getSourceManager().getFilename(spellingLocation);
    // ignore C code
    if ( filename.endswith(".h") ) {
        return true;
    }
    if ( compat::isInMainFile(compiler.getSourceManager(), spellingLocation) ) {
        if (filename.startswith(SRCDIR "/sal") // sal has tons of weird stuff going on that I don't understand enough to fix
           || filename.startswith(SRCDIR "/bridges")) { // I'm not messing with this code - far too dangerous
            return true;
        }
    } else {
        if (filename.startswith(SRCDIR "/include/tools/solar.h")
           // the GetSalDisplay/GetGtkDisplay methods are problematic
           || filename.startswith(SRCDIR "/vcl/inc/generic/gendata.hxx")
           || filename.startswith(SRCDIR "/include/cppuhelper/")) {
            return true;
        }
    }
    report(
        DiagnosticsEngine::Warning,
        "c-style cast, type=%0, from=%1, to=%2, recommendedFix=%3",
        expr->getSourceRange().getBegin())
      << expr->getCastKind()
      << expr->getSubExprAsWritten()->getType()
      << expr->getType()
      << recommendedFix(expr->getCastKind())
      << expr->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< CStyleCast > X("cstylecast");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
