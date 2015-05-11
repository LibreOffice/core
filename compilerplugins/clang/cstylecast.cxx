/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include "plugin.hxx"
#include "compat.hxx"

//
// We don't like using C-style casts in C++ code
//

namespace {

bool hasCLanguageLinkageType(FunctionDecl const * decl) {
    return decl->isExternC() || compat::isInExternCContext(*decl);
}

QualType resolvePointers(QualType type) {
    while (type->isPointerType()) {
        type = type->getAs<PointerType>()->getPointeeType();
    }
    return type;
}

class CStyleCast:
    public RecursiveASTVisitor<CStyleCast>, public loplugin::Plugin
{
public:
    explicit CStyleCast(InstantiationData const & data):
        Plugin(data), externCFunction(false)
    {}

    virtual void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool TraverseFunctionDecl(FunctionDecl * decl);

    bool VisitCStyleCastExpr(const CStyleCastExpr * expr);

private:
    bool externCFunction;
};

static const char * recommendedFix(clang::CastKind ck) {
    switch(ck) {
        case CK_IntegralToPointer: return "reinterpret_cast";
        case CK_PointerToIntegral: return "reinterpret_cast";
        case CK_BaseToDerived: return "static_cast";
        default: return "???";
    }
}

bool CStyleCast::TraverseFunctionDecl(FunctionDecl * decl) {
    bool ext = hasCLanguageLinkageType(decl)
        && decl->isThisDeclarationADefinition();
    if (ext) {
        assert(!externCFunction);
        externCFunction = true;
    }
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(decl);
    if (ext) {
        externCFunction = false;
    }
    return ret;
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
        QualType t1 = expr->getSubExpr()->getType();
        QualType t2 = expr->getType();
        if (t1->isPointerType() && t2->isPointerType()) {
            t1 = t1->getAs<PointerType>()->getPointeeType();
            t2 = t2->getAs<PointerType>()->getPointeeType();
        } else if (t1->isLValueReferenceType() && t2->isLValueReferenceType()) {
            t1 = t1->getAs<LValueReferenceType>()->getPointeeType();
            t2 = t2->getAs<LValueReferenceType>()->getPointeeType();
        } else {
            return true;
        }
        if (expr->getSubExprAsWritten()->getType() != expr->getType()
            && (!t1.isMoreQualifiedThan(t2)
                || (t1.getUnqualifiedType().getCanonicalType().getTypePtr()
                    != (t2.getUnqualifiedType().getCanonicalType()
                        .getTypePtr()))))
        {
            return true;
        }
    }
    std::string incompFrom;
    std::string incompTo;
    if( expr->getCastKind() == CK_BitCast ) {
        if (resolvePointers(expr->getSubExprAsWritten()->getType())
            ->isIncompleteType())
        {
            incompFrom = "incomplete ";
        }
        if (resolvePointers(expr->getType())->isIncompleteType()) {
            incompTo = "incomplete ";
        }
    }
    if (externCFunction || expr->getLocStart().isMacroID()) {
        SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(
            expr->getLocStart());
        StringRef filename = compiler.getSourceManager().getFilename(spellingLocation);
        // ignore C code
        if ( filename.endswith(".h") ) {
            return true;
        }
    }
    report(
        DiagnosticsEngine::Warning,
        "c-style cast, type=%0, from=%1%2, to=%3%4, recommendedFix=%5",
        expr->getSourceRange().getBegin())
      << expr->getCastKind()
      << incompFrom << expr->getSubExprAsWritten()->getType()
      << incompTo << expr->getType()
      << recommendedFix(expr->getCastKind())
      << expr->getSourceRange();
    return true;
}

loplugin::Plugin::Registration< CStyleCast > X("cstylecast");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
