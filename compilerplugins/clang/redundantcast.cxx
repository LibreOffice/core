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

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"
#include <iostream>

namespace {

bool isVoidPointer(QualType type) {
    return type->isPointerType()
        && type->getAs<clang::PointerType>()->getPointeeType()->isVoidType();
}

bool isRedundantConstCast(CXXConstCastExpr const * expr) {
    auto const sub = compat::getSubExprAsWritten(expr);
    return
        (expr->getType().getCanonicalType()
         == sub->getType().getCanonicalType())
        && (expr->getValueKind() != VK_XValue
            || sub->getValueKind() == VK_XValue);
}

bool canConstCastFromTo(Expr const * from, Expr const * to) {
    auto const k1 = from->getValueKind();
    auto const k2 = to->getValueKind();
    return (k2 == VK_LValue && k1 == VK_LValue)
        || (k2 == VK_XValue
            && (k1 != VK_RValue || from->getType()->isRecordType()));
}

char const * printExprValueKind(ExprValueKind k) {
    switch (k) {
    case VK_RValue:
        return "prvalue";
    case VK_LValue:
        return "lvalue";
    case VK_XValue:
        return "xvalue";
    };
    llvm_unreachable("unknown ExprValueKind");
}

enum class AlgebraicType { None, Integer, FloatingPoint };

AlgebraicType algebraicType(clang::Type const & type) {
    if (type.isIntegralOrEnumerationType()) {
        return AlgebraicType::Integer;
    } else if (type.isRealFloatingType()) {
        return AlgebraicType::FloatingPoint;
    } else {
        return AlgebraicType::None;
    }
}

// Do not look through FunctionToPointerDecay, but through e.g. NullToPointer:
Expr const * stopAtFunctionPointerDecay(ExplicitCastExpr const * expr) {
    auto const e1 = expr->getSubExpr();
    if (auto const e2 = dyn_cast<ImplicitCastExpr>(e1)) {
        if (e2->getCastKind() != CK_FunctionToPointerDecay) {
            return e2->getSubExpr();
        }
    }
    return e1;
}

class RedundantCast:
    public loplugin::FilteringRewritePlugin<RedundantCast>
{
public:
    explicit RedundantCast(loplugin::InstantiationData const & data):
        FilteringRewritePlugin(data)
    {}

    virtual void run() override {
        if (compiler.getLangOpts().CPlusPlus) {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool TraverseInitListExpr(InitListExpr * expr, DataRecursionQueue * queue = nullptr) {
        return WalkUpFromInitListExpr(expr)
            && TraverseSynOrSemInitListExpr(
                expr->isSemanticForm() ? expr : expr->getSemanticForm(), queue);
    }

    bool VisitImplicitCastExpr(ImplicitCastExpr const * expr);

    bool VisitCXXStaticCastExpr(CXXStaticCastExpr const * expr);

    bool VisitCXXReinterpretCastExpr(CXXReinterpretCastExpr const * expr);

    bool VisitCXXConstCastExpr(CXXConstCastExpr const * expr);

    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const * expr);

    bool VisitCXXDynamicCastExpr(CXXDynamicCastExpr const * expr);

    bool VisitCallExpr(CallExpr const * expr);

    bool VisitCXXDeleteExpr(CXXDeleteExpr const * expr);

    bool VisitCStyleCastExpr(CStyleCastExpr const * expr);

    bool VisitBinaryOperator(BinaryOperator const * expr) {
        auto const op = expr->getOpcode();
        if (op == BO_Sub || expr->isRelationalOp() || expr->isEqualityOp()) {
            return visitBinOp(expr);
        }
        if (op == BO_Assign) {
            if (ignoreLocation(expr)) {
                return true;
            }
            visitAssign(expr->getLHS()->getType(), expr->getRHS());
            return true;
        }
        return true;
    }

    bool VisitVarDecl(VarDecl const * varDecl);

private:
    bool visitBinOp(BinaryOperator const * expr);
    void visitAssign(QualType lhs, Expr const * rhs);
    bool isOverloadedFunction(FunctionDecl const * decl);

    bool isInIgnoredMacroBody(Expr const * expr) {
        auto const loc = compat::getBeginLoc(expr);
        return compiler.getSourceManager().isMacroBodyExpansion(loc)
            && ignoreLocation(compiler.getSourceManager().getSpellingLoc(loc));
    }
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
            if (e != nullptr && !isRedundantConstCast(e)) {
                auto t1 = e->getSubExpr()->getType().getCanonicalType();
                auto t3 = expr->getType().getCanonicalType();
                bool ObjCLifetimeConversion;
                if (t1.getTypePtr() == t3.getTypePtr()
                    || (compiler.getSema().IsQualificationConversion(
                            t1, t3, false, ObjCLifetimeConversion)
                        && (e->getType().getCanonicalType().getTypePtr()
                            != t3.getTypePtr())))
                {
                    report(
                        DiagnosticsEngine::Warning,
                        ("redundant const_cast from %0 to %1, result is"
                         " implicitly cast to %2"),
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
                if (expr->getType()->getAs<clang::PointerType>()
                    ->getPointeeType().isAtLeastAsQualifiedAs(
                        cc->getSubExpr()->getType()
                        ->getAs<clang::PointerType>()->getPointeeType()))
                {
                    report(
                        DiagnosticsEngine::Warning,
                        ("redundant const_cast from %0 to %1, result is"
                         " ultimately implicitly cast to %2"),
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
                       && !compiler.getSourceManager().isMacroBodyExpansion(
                           compat::getBeginLoc(e)))
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
                if (expr->getType()->getAs<clang::PointerType>()
                    ->getPointeeType().isAtLeastAsQualifiedAs(
                        cc->getSubExpr()->getType()
                        ->getAs<clang::PointerType>()->getPointeeType()))
                {
                    report(
                        DiagnosticsEngine::Warning,
                        ("redundant const_cast from %0 to %1, result is"
                         " ultimately implicitly cast to %2"),
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
                         " ultimately implicitly cast to %2"),
                        cc->getExprLoc())
                        << cc->getSubExprAsWritten()->getType() << cc->getType()
                        << expr->getType() << expr->getSourceRange();
                }
                e = cc->getSubExpr()->IgnoreParenImpCasts();
            }
        }
        break;
    case CK_FloatingToIntegral:
    case CK_IntegralToFloating:
        if (auto e = dyn_cast<ExplicitCastExpr>(expr->getSubExpr()->IgnoreParenImpCasts())) {
            if ((isa<CXXStaticCastExpr>(e) || isa<CXXFunctionalCastExpr>(e))
                && (algebraicType(*e->getSubExprAsWritten()->getType())
                    == algebraicType(*expr->getType())))
            {
                report(
                    DiagnosticsEngine::Warning,
                    ("suspicious %select{static_cast|functional cast}0 from %1 to %2, result is"
                     " implicitly cast to %3"),
                    e->getExprLoc())
                    << isa<CXXFunctionalCastExpr>(e) << e->getSubExprAsWritten()->getType()
                    << e->getTypeAsWritten() << expr->getType() << expr->getSourceRange();
            }
        }
        break;
    default:
        break;
    }
    return true;
}

bool RedundantCast::VisitCStyleCastExpr(CStyleCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(expr)))) {
        return true;
    }
    auto t1 = compat::getSubExprAsWritten(expr)->getType();
    auto t2 = expr->getTypeAsWritten();
    if (auto templateType = dyn_cast<SubstTemplateTypeParmType>(t1)) {
        t1 = templateType->desugar();
    }
    if (t1 != t2) {
        return true;
    }
    if (!t1->isBuiltinType() && !loplugin::TypeCheck(t1).Enum() && !loplugin::TypeCheck(t1).Typedef()) {
        return true;
    }
    if (!loplugin::isOkToRemoveArithmeticCast(compiler.getASTContext(), t1, t2, expr->getSubExpr()))
    {
        return true;
    }
    // Ignore FD_ISSET expanding to "...(SOCKET)(fd)..." in some Microsoft
    // winsock2.h (TODO: improve heuristic of determining that the whole
    // expr is part of a single macro body expansion):
    auto l1 = compat::getBeginLoc(expr);
    while (compiler.getSourceManager().isMacroArgExpansion(l1)) {
        l1 = compiler.getSourceManager().getImmediateMacroCallerLoc(l1);
    }
    auto l2 = expr->getExprLoc();
    while (compiler.getSourceManager().isMacroArgExpansion(l2)) {
        l2 = compiler.getSourceManager().getImmediateMacroCallerLoc(l2);
    }
    auto l3 = compat::getEndLoc(expr);
    while (compiler.getSourceManager().isMacroArgExpansion(l3)) {
         l3 = compiler.getSourceManager().getImmediateMacroCallerLoc(l3);
    }
    if (compiler.getSourceManager().isMacroBodyExpansion(l1)
        && compiler.getSourceManager().isMacroBodyExpansion(l2)
        && compiler.getSourceManager().isMacroBodyExpansion(l3)
        && ignoreLocation(compiler.getSourceManager().getSpellingLoc(l2)))
    {
        return true;
    }
    report(
        DiagnosticsEngine::Warning,
        "redundant cstyle cast from %0 to %1", expr->getExprLoc())
        << t1 << t2 << expr->getSourceRange();
    return true;
}

bool RedundantCast::VisitVarDecl(VarDecl const * varDecl) {
    if (ignoreLocation(varDecl)) {
        return true;
    }
    if (!varDecl->getInit())
        return true;
    visitAssign(varDecl->getType(), varDecl->getInit());
    return true;
}

void RedundantCast::visitAssign(QualType t1, Expr const * rhs)
{
    auto staticCastExpr = dyn_cast<CXXStaticCastExpr>(rhs->IgnoreImplicit());
    if (!staticCastExpr)
        return;

    auto const t2 = staticCastExpr->getSubExpr()->IgnoreImplicit()->getType();

    // if there is more than one copy of the LHS, this cast is resolving ambiguity
    bool foundOne = false;
    if (t1->isRecordType())
    {
        foundOne = loplugin::derivedFromCount(t2, t1) == 1;
    }
    else
    {
        auto pointee1 = t1->getPointeeCXXRecordDecl();
        auto pointee2 = t2->getPointeeCXXRecordDecl();
        if (pointee1 && pointee2)
            foundOne = loplugin::derivedFromCount(pointee2, pointee1) == 1;
    }

    if (foundOne)
    {
        report(
            DiagnosticsEngine::Warning, "redundant static_cast from %0 to %1",
            staticCastExpr->getExprLoc())
            << t2 << t1 << staticCastExpr->getSourceRange();
    }
}

bool RedundantCast::VisitCXXStaticCastExpr(CXXStaticCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto const t2 = expr->getTypeAsWritten();
    bool const fnptr = t2->isFunctionPointerType() || t2->isMemberFunctionPointerType();
    auto const sub = fnptr ? stopAtFunctionPointerDecay(expr) : compat::getSubExprAsWritten(expr);
    auto const t1 = sub->getType();
    auto const nonClassObjectType = t2->isObjectType()
        && !(t2->isRecordType() || t2->isArrayType());
    if (nonClassObjectType && t2.hasLocalQualifiers()) {
        report(
            DiagnosticsEngine::Warning,
            ("in static_cast from %0 %1 to %2 %3, remove redundant top-level"
             " %select{const qualifier|volatile qualifier|const volatile"
             " qualifiers}4"),
            expr->getExprLoc())
            << t1 << printExprValueKind(sub->getValueKind())
            << t2 << printExprValueKind(expr->getValueKind())
            << ((t2.isLocalConstQualified() ? 1 : 0)
                + (t2.isLocalVolatileQualified() ? 2 : 0) - 1)
            << expr->getSourceRange();
        return true;
    }
    if (auto const impl = dyn_cast<ImplicitCastExpr>(expr->getSubExpr())) {
        if (impl->getCastKind() == CK_ArrayToPointerDecay && impl->getType() == t2)
            //TODO: instead of exact QualType match, allow some variation?
        {
            auto const fn = handler.getMainFileName();
            if (!(loplugin::isSamePathname(
                      fn, SRCDIR "/sal/qa/rtl/strings/test_ostring_concat.cxx")
                  || loplugin::isSamePathname(
                      fn, SRCDIR "/sal/qa/rtl/strings/test_ostring_stringliterals.cxx")
                  || loplugin::isSamePathname(
                      fn, SRCDIR "/sal/qa/rtl/strings/test_oustring_concat.cxx")
                  || loplugin::isSamePathname(
                      fn, SRCDIR "/sal/qa/rtl/strings/test_oustring_stringliterals.cxx")
                  || isInIgnoredMacroBody(expr)))
            {
                report(
                    DiagnosticsEngine::Warning, "redundant static_cast from %0 to %1",
                    expr->getExprLoc())
                    << expr->getSubExprAsWritten()->getType() << t2 << expr->getSourceRange();
            }
            return true;
        }
    }
    auto const t3 = expr->getType();
    auto const c1 = t1.getCanonicalType();
    auto const c3 = t3.getCanonicalType();
    if (nonClassObjectType || !canConstCastFromTo(sub, expr)
        ? c1.getTypePtr() != c3.getTypePtr() : c1 != c3)
    {
        bool ObjCLifetimeConversion;
        if (nonClassObjectType
            || (c1.getTypePtr() != c3.getTypePtr()
                && !compiler.getSema().IsQualificationConversion(
                    c1, c3, false, ObjCLifetimeConversion)))
        {
            return true;
        }
        report(
            DiagnosticsEngine::Warning,
            "static_cast from %0 %1 to %2 %3 should be written as const_cast",
            expr->getExprLoc())
            << t1 << printExprValueKind(sub->getValueKind())
            << t2 << printExprValueKind(expr->getValueKind())
            << expr->getSourceRange();
        return true;
    }
    if (!loplugin::isOkToRemoveArithmeticCast(compiler.getASTContext(), t1, t2, expr->getSubExpr()))
    {
        return true;
    }
    // Don't warn if the types are 'void *' and at least one involves a typedef
    // (and if both involve typedefs, they're different) (this covers cases like
    // 'oslModule', or 'CURL *', or casts between 'LPVOID' and 'HANDLE' in
    // Windows-only code):
    if (loplugin::TypeCheck(t1).Pointer().NonConstVolatile().Void()) {
        if (auto const td1 = t1->getAs<TypedefType>()) {
            auto const td2 = t2->getAs<TypedefType>();
            if (td2 == nullptr || td2 != td1) {
                return true;
            }
        } else if (auto const td2 = t2->getAs<TypedefType>()) {
            auto const td1 = t1->getAs<TypedefType>();
            if (td1 == nullptr || td1 != td2) {
                return true;
            }
        } else {
            auto const pt1 = t1->getAs<clang::PointerType>()->getPointeeType();
            auto const pt2 = t2->getAs<clang::PointerType>()->getPointeeType();
            if (auto const ptd1 = pt1->getAs<TypedefType>()) {
                auto const ptd2 = pt2->getAs<TypedefType>();
                if (ptd2 == nullptr || ptd2 != ptd1) {
                    return true;
                }
            } else if (auto const ptd2 = pt2->getAs<TypedefType>()) {
                auto const ptd1 = pt1->getAs<TypedefType>();
                if (ptd1 == nullptr || ptd1 != ptd2) {
                    return true;
                }
            }
        }
    }
    auto const k1 = sub->getValueKind();
    auto const k3 = expr->getValueKind();
    if ((k3 == VK_XValue && k1 != VK_XValue)
        || (k3 == VK_LValue && k1 == VK_XValue))
    {
        return true;
    }
    // Don't warn if a static_cast on a pointer to function or member function is used to
    // disambiguate an overloaded function:
    if (fnptr) {
        auto e = sub->IgnoreParenImpCasts();
        if (auto const e1 = dyn_cast<UnaryOperator>(e)) {
            if (e1->getOpcode() == UO_AddrOf) {
                e = e1->getSubExpr()->IgnoreParenImpCasts();
            }
        }
        if (auto const e1 = dyn_cast<DeclRefExpr>(e)) {
            if (auto const fdecl = dyn_cast<FunctionDecl>(e1->getDecl())) {
                if (isOverloadedFunction(fdecl)) {
                    return true;
                }
            }
        }
    }
    // Suppress warnings from static_cast<bool> in C++ definition of assert in
    // <https://sourceware.org/git/?p=glibc.git;a=commit;
    // h=b5889d25e9bf944a89fdd7bcabf3b6c6f6bb6f7c> "assert: Support types
    // without operator== (int) [BZ #21972]":
    if (t1->isBooleanType() && t2->isBooleanType()) {
        auto loc = compat::getBeginLoc(expr);
        if (compiler.getSourceManager().isMacroBodyExpansion(loc)
            && (Lexer::getImmediateMacroName(
                    loc, compiler.getSourceManager(), compiler.getLangOpts())
                == "assert"))
        {
            return true;
        }
    }
    report(
        DiagnosticsEngine::Warning,
        ("static_cast from %0 %1 to %2 %3 is redundant%select{| or should be"
         " written as an explicit construction of a temporary}4"),
        expr->getExprLoc())
        << t1 << printExprValueKind(k1) << t2 << printExprValueKind(k3)
        << (k3 == VK_RValue && (k1 != VK_RValue || t1->isRecordType()))
        << expr->getSourceRange();
    return true;
}

bool RedundantCast::VisitCXXReinterpretCastExpr(
    CXXReinterpretCastExpr const * expr)
{
    if (ignoreLocation(expr)) {
        return true;
    }
    if (auto const sub = dyn_cast<ImplicitCastExpr>(expr->getSubExpr())) {
        if (sub->getCastKind() == CK_ArrayToPointerDecay && sub->getType() == expr->getType())
            //TODO: instead of exact QualType match, allow some variation?
        {
            if (loplugin::TypeCheck(sub->getType()).Pointer().Const().Char()) {
                if (auto const lit = dyn_cast<clang::StringLiteral>(expr->getSubExprAsWritten())) {
                    if (lit->getKind() == clang::StringLiteral::UTF8) {
                        // Don't warn about
                        //
                        //   redundant_cast<char const *>(u8"...")
                        //
                        // in pre-C++2a code:
                        return true;
                    }
                }
            }
            report(
                DiagnosticsEngine::Warning, "redundant reinterpret_cast from %0 to %1",
                expr->getExprLoc())
                << expr->getSubExprAsWritten()->getType() << expr->getTypeAsWritten()
                << expr->getSourceRange();
            return true;
        }
    }
    if (expr->getSubExpr()->getType()->isVoidPointerType()) {
        auto t = expr->getType()->getAs<clang::PointerType>();
        if (t == nullptr || !t->getPointeeType()->isObjectType()) {
            return true;
        }
        if (rewriter != nullptr) {
            auto loc = compat::getBeginLoc(expr);
            while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
                loc = compiler.getSourceManager().getImmediateMacroCallerLoc(
                    loc);
            }
            if (compiler.getSourceManager().isMacroBodyExpansion(loc)) {
                auto loc2 = compat::getEndLoc(expr);
                while (compiler.getSourceManager().isMacroArgExpansion(loc2)) {
                    loc2 = compiler.getSourceManager()
                        .getImmediateMacroCallerLoc(loc2);
                }
                if (compiler.getSourceManager().isMacroBodyExpansion(loc2)) {
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
        auto t = expr->getSubExpr()->getType()->getAs<clang::PointerType>();
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
    } else if (expr->getType()->isFundamentalType()) {
        if (auto const sub = dyn_cast<CXXConstCastExpr>(
                expr->getSubExpr()->IgnoreParens()))
        {
            report(
                DiagnosticsEngine::Warning,
                ("redundant const_cast from %0 to %1 within reinterpret_cast to"
                 " fundamental type %2"),
                expr->getExprLoc())
                << sub->getSubExprAsWritten()->getType()
                << sub->getTypeAsWritten() << expr->getTypeAsWritten()
                << expr->getSourceRange();
            return true;
        }
    }
    return true;
}

bool RedundantCast::VisitCXXConstCastExpr(CXXConstCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto const sub = compat::getSubExprAsWritten(expr);
    if (isRedundantConstCast(expr)) {
        report(
            DiagnosticsEngine::Warning,
            "redundant const_cast from %0 %1 to %2 %3", expr->getExprLoc())
            << sub->getType() << printExprValueKind(sub->getValueKind())
            << expr->getTypeAsWritten()
            << printExprValueKind(expr->getValueKind())
            << expr->getSourceRange();
        return true;
    }
    if (auto const dce = dyn_cast<CXXStaticCastExpr>(
            sub->IgnoreParenImpCasts()))
    {
        auto const sub2 = compat::getSubExprAsWritten(dce);
        auto t1 = sub2->getType().getCanonicalType();
        auto isNullptr = t1->isNullPtrType();
        auto t2 = dce->getType().getCanonicalType();
        auto t3 = expr->getType().getCanonicalType();
        auto redundant = false;
        for (;;) {
            if ((t2.isConstQualified()
                 && (isNullptr || !t1.isConstQualified())
                 && !t3.isConstQualified())
                || (t2.isVolatileQualified()
                    && (isNullptr || !t1.isVolatileQualified())
                    && !t3.isVolatileQualified()))
            {
                redundant = true;
                break;
            }
            if (!isNullptr) {
                auto const p1 = t1->getAs<clang::PointerType>();
                if (p1 == nullptr) {
                    break;
                }
                t1 = p1->getPointeeType();
                isNullptr = t1->isNullPtrType();
            }
            auto const p2 = t2->getAs<clang::PointerType>();
            if (p2 == nullptr) {
                break;
            }
            t2 = p2->getPointeeType();
            auto const p3 = t3->getAs<clang::PointerType>();
            if (p3 == nullptr) {
                break;
            }
            t3 = p3->getPointeeType();
        }
        if (redundant) {
            report(
                DiagnosticsEngine::Warning,
                ("redundant static_cast/const_cast combination from %0 via %1"
                 " to %2"),
                expr->getExprLoc())
                << sub2->getType() << dce->getTypeAsWritten()
                << expr->getTypeAsWritten() << expr->getSourceRange();
        }
    }
    return true;
}

bool RedundantCast::VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    // Restrict this to "real" casts (compared to uses of braced-init-list, like
    //
    //   Foo{bar, baz}
    //
    // or
    //
    //   std::initializer_list<Foo>{bar, baz}
    //
    // ), and only to cases where the sub-expression already is a prvalue of
    // non-class type (and thus the cast is unlikely to be meant to create a
    // temporary):
    auto const t1 = expr->getTypeAsWritten();
    bool const fnptr = t1->isFunctionPointerType() || t1->isMemberFunctionPointerType();
    auto const sub = fnptr ? stopAtFunctionPointerDecay(expr) : compat::getSubExprAsWritten(expr);
    if ((sub->getValueKind() != VK_RValue && !fnptr) || expr->getType()->isRecordType()
        || isa<InitListExpr>(sub) || isa<CXXStdInitializerListExpr>(sub))
    {
        return true;
    }

    // See "There might even be good reasons(?) not to warn inside explicit
    // casts" block in compilerplugins/clang/test/cppunitassertequals.cxx:
    auto const eloc = expr->getExprLoc();
    if (compiler.getSourceManager().isMacroArgExpansion(eloc)) {
        auto const name = Lexer::getImmediateMacroName(
            eloc, compiler.getSourceManager(), compiler.getLangOpts());
        if (name == "CPPUNIT_ASSERT" || name == "CPPUNIT_ASSERT_MESSAGE") {
            return true;
        }
    }

    // Don't warn if a functional cast on a pointer to function or member function is used to
    // disambiguate an overloaded function:
    if (fnptr) {
        auto e = sub->IgnoreParenImpCasts();
        if (auto const e1 = dyn_cast<UnaryOperator>(e)) {
            if (e1->getOpcode() == UO_AddrOf) {
                e = e1->getSubExpr()->IgnoreParenImpCasts();
            }
        }
        if (auto const e1 = dyn_cast<DeclRefExpr>(e)) {
            if (auto const fdecl = dyn_cast<FunctionDecl>(e1->getDecl())) {
                if (isOverloadedFunction(fdecl)) {
                    return true;
                }
            }
        }
    }

    // See the commit message of d0e7d020fa405ab94f19916ec96fbd4611da0031
    // "socket.c -> socket.cxx" for the reason to have
    //
    //   bool(FD_ISSET(...))
    //
    // in sal/osl/unx/socket.cxx:
    //TODO: Better check that sub is exactly an expansion of FD_ISSET:
    if (compat::getEndLoc(sub).isMacroID()) {
        for (auto loc = compat::getBeginLoc(sub);
             loc.isMacroID()
                 && (compiler.getSourceManager()
                     .isAtStartOfImmediateMacroExpansion(loc));
             loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc))
        {
            if (Lexer::getImmediateMacroName(
                    loc, compiler.getSourceManager(), compiler.getLangOpts())
                == "FD_ISSET")
            {
                return true;
            }
        }
    }

    auto const t2 = sub->getType();
    if (t1.getCanonicalType() != t2.getCanonicalType())
        return true;
    if (!loplugin::isOkToRemoveArithmeticCast(compiler.getASTContext(), t1, t2, expr->getSubExpr()))
        return true;
    report(
        DiagnosticsEngine::Warning,
        "redundant functional cast from %0 to %1", expr->getExprLoc())
        << t2 << t1 << expr->getSourceRange();
    return true;
}

bool RedundantCast::VisitCXXDynamicCastExpr(CXXDynamicCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto const sub = compat::getSubExprAsWritten(expr);
    auto const t1 = expr->getTypeAsWritten();
    auto const t2 = sub->getType();
    QualType qt1 = t1.getCanonicalType();
    QualType qt2 = t2.getCanonicalType();
    if (qt1 == qt2)
    {
        report(
            DiagnosticsEngine::Warning,
            "redundant dynamic cast from %0 to %1", expr->getExprLoc())
            << t2 << t1 << expr->getSourceRange();
        return true;
    }
    if (qt1->isPointerType() && qt2->isPointerType())
    {
        // casting from 'T*' to 'const T*' is redundant, so compare without the qualifiers
        qt1 = qt1->getPointeeType().getUnqualifiedType();
        qt2 = qt2->getPointeeType().getUnqualifiedType();
        if (qt1 == qt2)
        {
            report(
                DiagnosticsEngine::Warning,
                "redundant dynamic cast from %0 to %1", expr->getExprLoc())
                << t2 << t1 << expr->getSourceRange();
            return true;
        }
        if (qt1->getAsCXXRecordDecl() && qt2->getAsCXXRecordDecl()->isDerivedFrom(qt1->getAsCXXRecordDecl()))
        {
            report(
                DiagnosticsEngine::Warning,
                "redundant dynamic upcast from %0 to %1", expr->getExprLoc())
                << t2 << t1 << expr->getSourceRange();
            return true;
        }
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

bool RedundantCast::isOverloadedFunction(FunctionDecl const * decl) {
    auto const ctx = decl->getDeclContext();
    if (!ctx->isLookupContext()) {
        return false;
    }
    auto const canon = decl->getCanonicalDecl();
    auto const res = ctx->lookup(decl->getDeclName());
    for (auto d = res.begin(); d != res.end(); ++d) {
        if (auto const f = dyn_cast<FunctionDecl>(*d)) {
            if (f->getCanonicalDecl() != canon) {
                return true;
            }
        }
    }
    return false;
}

loplugin::Plugin::Registration<RedundantCast> X("redundantcast", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
