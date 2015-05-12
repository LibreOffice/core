/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Warn about certain redundant casts:
//
// * A reinterpret_cast<T*>(...) whose result is then implicitly cast to a void
//   pointer
//
// * A static_cast<T*>(e) where e is of void pointer type and whose result is
//   then implicitly cast to a void pointer
//
// * Various const_casts that are either not needed (like casting away constness
//   in a delete expression) or are implicitly cast back afterwards
//
// C-style casts are ignored because it makes this plugin simpler, and they
// should eventually be eliminated via loplugin:cstylecast and/or
// -Wold-style-cast.  That implies that this plugin is only relevant for C++
// code.

#include "clang/Sema/Sema.h"

#include "compat.hxx"
#include "plugin.hxx"

namespace {

bool isVoidPointer(QualType type) {
    return type->isPointerType()
        && type->getAs<PointerType>()->getPointeeType()->isVoidType();
}

class RedundantCast:
    public RecursiveASTVisitor<RedundantCast>, public loplugin::RewritePlugin
{
public:
    explicit RedundantCast(InstantiationData const & data): RewritePlugin(data)
    {}

    virtual void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitImplicitCastExpr(ImplicitCastExpr const * expr);

    bool VisitCXXReinterpretCastExpr(CXXReinterpretCastExpr const * expr);

    bool VisitCallExpr(CallExpr const * expr);

    bool VisitCXXDeleteExpr(CXXDeleteExpr const * expr);

    bool VisitBinSub(BinaryOperator const * expr)
    { return visitBinOp(expr); }

    bool VisitBinLT(BinaryOperator const * expr)
    { return visitBinOp(expr); }

    bool VisitBinGT(BinaryOperator const * expr)
    { return visitBinOp(expr); }

    bool VisitBinLE(BinaryOperator const * expr)
    { return visitBinOp(expr); }

    bool VisitBinGE(BinaryOperator const * expr)
    { return visitBinOp(expr); }

    bool VisitBinEQ(BinaryOperator const * expr)
    { return visitBinOp(expr); }

    bool VisitBinNE(BinaryOperator const * expr)
    { return visitBinOp(expr); }

private:
    bool visitBinOp(BinaryOperator const * expr);
};

bool RedundantCast::VisitImplicitCastExpr(const ImplicitCastExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    switch (expr->getCastKind()) {
    case CK_NoOp:
        if (expr->getType()->isPointerType()
            || expr->getType()->isObjectType())
        {
            auto e = dyn_cast<CXXConstCastExpr>(
                expr->getSubExpr()->IgnoreParenImpCasts());
            if (e != nullptr) {
                auto t1 = e->getSubExpr()->getType().getCanonicalType();
                auto t2 = expr->getType().getCanonicalType();
                bool ObjCLifetimeConversion;
                if (t1.getTypePtr() == t2.getTypePtr()
                    || compiler.getSema().IsQualificationConversion(
                        t1, t2, false, ObjCLifetimeConversion))
                {
                    report(
                        DiagnosticsEngine::Warning,
                        ("redundant const_cast from %0 to %1, result is"
                         " implictly cast to %2"),
                        e->getExprLoc())
                        << e->getSubExprAsWritten()->getType() << e->getType()
                        << expr->getType() << expr->getSourceRange();
                }
            }
        }
        break;
    case CK_BitCast:
        if (isVoidPointer(expr->getType())
            && expr->getSubExpr()->getType()->isPointerType())
        {
            Expr const * e = expr->getSubExpr()->IgnoreParenImpCasts();
            while (isa<CXXConstCastExpr>(e)) {
                auto cc = dyn_cast<CXXConstCastExpr>(e);
                if (expr->getType()->getAs<PointerType>()->getPointeeType()
                    .isAtLeastAsQualifiedAs(
                        cc->getSubExpr()->getType()
                        ->getAs<PointerType>()->getPointeeType()))
                {
                    report(
                        DiagnosticsEngine::Warning,
                        ("redundant const_cast from %0 to %1, result is"
                         " ultimately implictly cast to %2"),
                        cc->getExprLoc())
                        << cc->getSubExprAsWritten()->getType() << cc->getType()
                        << expr->getType() << expr->getSourceRange();
                }
                e = cc->getSubExpr()->IgnoreParenImpCasts();
            }
            if (isa<CXXReinterpretCastExpr>(e)) {
                report(
                    DiagnosticsEngine::Warning,
                    ("redundant reinterpret_cast, result is implicitly cast to"
                     " void pointer"),
                    e->getExprLoc())
                    << e->getSourceRange();
            } else if (isa<CXXStaticCastExpr>(e)
                       && isVoidPointer(
                           dyn_cast<CXXStaticCastExpr>(e)->getSubExpr()
                           ->IgnoreParenImpCasts()->getType())
                       && !compat::isMacroBodyExpansion(
                           compiler, e->getLocStart()))
            {
                report(
                    DiagnosticsEngine::Warning,
                    ("redundant static_cast from void pointer, result is"
                     " implicitly cast to void pointer"),
                    e->getExprLoc())
                    << e->getSourceRange();
            }
        }
        break;
    case CK_DerivedToBase:
    case CK_UncheckedDerivedToBase:
        if (expr->getType()->isPointerType()) {
            Expr const * e = expr->getSubExpr()->IgnoreParenImpCasts();
            while (isa<CXXConstCastExpr>(e)) {
                auto cc = dyn_cast<CXXConstCastExpr>(e);
                if (expr->getType()->getAs<PointerType>()->getPointeeType()
                    .isAtLeastAsQualifiedAs(
                        cc->getSubExpr()->getType()
                        ->getAs<PointerType>()->getPointeeType()))
                {
                    report(
                        DiagnosticsEngine::Warning,
                        ("redundant const_cast from %0 to %1, result is"
                         " ultimately implictly cast to %2"),
                        cc->getExprLoc())
                        << cc->getSubExprAsWritten()->getType() << cc->getType()
                        << expr->getType() << expr->getSourceRange();
                }
                e = cc->getSubExpr()->IgnoreParenImpCasts();
            }
        } else if (expr->getType()->isReferenceType()) {
            Expr const * e = expr->getSubExpr()->IgnoreParenImpCasts();
            while (isa<CXXConstCastExpr>(e)) {
                auto cc = dyn_cast<CXXConstCastExpr>(e);
                if (expr->getType()->getAs<ReferenceType>()->getPointeeType()
                    .isAtLeastAsQualifiedAs(
                        cc->getSubExpr()->getType()
                        ->getAs<ReferenceType>()->getPointeeType()))
                {
                    report(
                        DiagnosticsEngine::Warning,
                        ("redundant const_cast from %0 to %1, result is"
                         " ultimately implictly cast to %2"),
                        cc->getExprLoc())
                        << cc->getSubExprAsWritten()->getType() << cc->getType()
                        << expr->getType() << expr->getSourceRange();
                }
                e = cc->getSubExpr()->IgnoreParenImpCasts();
            }
        }
        break;
    default:
        break;
    }
    return true;
}

bool RedundantCast::VisitCXXReinterpretCastExpr(
    CXXReinterpretCastExpr const * expr)
{
    if (ignoreLocation(expr)) {
        return true;
    }
    if (expr->getSubExpr()->getType()->isVoidPointerType()) {
        auto t = expr->getType()->getAs<PointerType>();
        if (t == nullptr || !t->getPointeeType()->isObjectType()) {
            return true;
        }
        if (rewriter != nullptr) {
            auto loc = expr->getLocStart();
            while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
                loc = compiler.getSourceManager().getImmediateMacroCallerLoc(
                    loc);
            }
            if (compat::isMacroBodyExpansion(compiler, loc)) {
                auto loc2 = expr->getLocEnd();
                while (compiler.getSourceManager().isMacroArgExpansion(loc2)) {
                    loc2 = compiler.getSourceManager()
                        .getImmediateMacroCallerLoc(loc2);
                }
                if (compat::isMacroBodyExpansion(compiler, loc2)) {
                    //TODO: check loc, loc2 are in same macro body expansion
                    loc = compiler.getSourceManager().getSpellingLoc(loc);
                }
            }
            auto s = compiler.getSourceManager().getCharacterData(loc);
            auto n = Lexer::MeasureTokenLength(
                loc, compiler.getSourceManager(), compiler.getLangOpts());
            std::string tok(s, n);
            if (tok == "reinterpret_cast" && replaceText(loc, n, "static_cast"))
            {
                return true;
            }
        }
        report(
            DiagnosticsEngine::Warning,
            "reinterpret_cast from %0 to %1 can be simplified to static_cast",
            expr->getExprLoc())
            << expr->getSubExprAsWritten()->getType() << expr->getType()
            << expr->getSourceRange();
    } else if (expr->getType()->isVoidPointerType()) {
        auto t = expr->getSubExpr()->getType()->getAs<PointerType>();
        if (t == nullptr || !t->getPointeeType()->isObjectType()) {
            return true;
        }
        report(
            DiagnosticsEngine::Warning,
            ("reinterpret_cast from %0 to %1 can be simplified to static_cast"
             " or an implicit conversion"),
            expr->getExprLoc())
            << expr->getSubExprAsWritten()->getType() << expr->getType()
            << expr->getSourceRange();
    }
    return true;
}

bool RedundantCast::VisitCallExpr(CallExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto f = expr->getDirectCallee();
    if (f == nullptr || !f->isVariadic()
        || expr->getNumArgs() <= f->getNumParams())
    {
        return true;
    }
    for (auto i = f->getNumParams(); i != expr->getNumArgs(); ++i) {
        auto a = expr->getArg(i);
        if (a->getType()->isPointerType()) {
            auto e = dyn_cast<CXXConstCastExpr>(a->IgnoreParenImpCasts());
            if (e != nullptr) {
                report(
                    DiagnosticsEngine::Warning,
                    "redundant const_cast of variadic function argument",
                    e->getExprLoc())
                    << expr->getSourceRange();
            }
        }
    }
    return true;
}

bool RedundantCast::VisitCXXDeleteExpr(CXXDeleteExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto e = dyn_cast<CXXConstCastExpr>(
        expr->getArgument()->IgnoreParenImpCasts());
    if (e != nullptr) {
        report(
            DiagnosticsEngine::Warning,
            "redundant const_cast in delete expression", e->getExprLoc())
            << expr->getSourceRange();
    }
    return true;
}

bool RedundantCast::visitBinOp(BinaryOperator const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (expr->getLHS()->getType()->isPointerType()
        && expr->getRHS()->getType()->isPointerType())
    {
        auto e = dyn_cast<CXXConstCastExpr>(
            expr->getLHS()->IgnoreParenImpCasts());
        if (e != nullptr) {
            report(
                DiagnosticsEngine::Warning,
                "redundant const_cast on lhs of pointer %select{comparison|subtraction}0 expression",
                e->getExprLoc())
                << (expr->getOpcode() == BO_Sub) << expr->getSourceRange();
        }
        e = dyn_cast<CXXConstCastExpr>(
            expr->getRHS()->IgnoreParenImpCasts());
        if (e != nullptr) {
            report(
                DiagnosticsEngine::Warning,
                "redundant const_cast on rhs of pointer %select{comparison|subtraction}0 expression",
                e->getExprLoc())
                << (expr->getOpcode() == BO_Sub) << expr->getSourceRange();
        }
    }
    return true;
}

loplugin::Plugin::Registration<RedundantCast> X("redundantcast", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
