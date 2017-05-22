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

namespace {

bool isVoidPointer(QualType type) {
    return type->isPointerType()
        && type->getAs<clang::PointerType>()->getPointeeType()->isVoidType();
}

bool isRedundantConstCast(CXXConstCastExpr const * expr) {
    return expr->getTypeAsWritten().getCanonicalType().getTypePtr()
        == (expr->getSubExprAsWritten()->getType().getCanonicalType()
            .getTypePtr());
}

bool isArithmeticOp(Expr const * expr) {
    expr = expr->IgnoreParenImpCasts();
    if (auto const e = dyn_cast<BinaryOperator>(expr)) {
        switch (e->getOpcode()) {
        case BO_Mul:
        case BO_Div:
        case BO_Rem:
        case BO_Add:
        case BO_Sub:
        case BO_Shl:
        case BO_Shr:
        case BO_And:
        case BO_Xor:
        case BO_Or:
            return true;
        case BO_Comma:
            return isArithmeticOp(e->getRHS());
        default:
            return false;
        }
    }
    return isa<UnaryOperator>(expr) || isa<AbstractConditionalOperator>(expr);
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

    bool VisitCXXStaticCastExpr(CXXStaticCastExpr const * expr);

    bool VisitCXXReinterpretCastExpr(CXXReinterpretCastExpr const * expr);

    bool VisitCXXConstCastExpr(CXXConstCastExpr const * expr);

    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const * expr);

    bool VisitCallExpr(CallExpr const * expr);

    bool VisitCXXDeleteExpr(CXXDeleteExpr const * expr);

    bool VisitCStyleCastExpr(CStyleCastExpr const * expr);

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
    bool isOkToRemoveArithmeticCast(QualType t1, QualType t2, const Expr* subExpr);
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
                           e->getLocStart()))
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
    default:
        break;
    }
    return true;
}

bool RedundantCast::VisitCStyleCastExpr(CStyleCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(expr->getLocStart()))) {
        return true;
    }
    auto t1 = compat::getSubExprAsWritten(expr)->getType();
    auto t2 = expr->getTypeAsWritten();
    if (t1 != t2) {
        return true;
    }
    if (!t1->isBuiltinType() && !loplugin::TypeCheck(t1).Enum() && !loplugin::TypeCheck(t1).Typedef()) {
        return true;
    }
    if (!isOkToRemoveArithmeticCast(t1, t2, expr->getSubExpr())) {
        return true;
    }
    // Ignore FD_ISSET expanding to "...(SOCKET)(fd)..." in some Microsoft
    // winsock2.h (TODO: improve heuristic of determining that the whole
    // expr is part of a single macro body expansion):
    auto l1 = expr->getLocStart();
    while (compiler.getSourceManager().isMacroArgExpansion(l1)) {
        l1 = compiler.getSourceManager().getImmediateMacroCallerLoc(l1);
    }
    auto l2 = expr->getExprLoc();
    while (compiler.getSourceManager().isMacroArgExpansion(l2)) {
        l2 = compiler.getSourceManager().getImmediateMacroCallerLoc(l2);
    }
    auto l3 = expr->getLocEnd();
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

bool RedundantCast::isOkToRemoveArithmeticCast(QualType t1, QualType t2, const Expr* subExpr) {
    // Don't warn if the types are arithmetic (in the C++ meaning), and: either
    // at least one is a typedef (and if both are typedefs,they're different),
    // or the sub-expression involves some operation that is likely to change
    // types through promotion, or the sub-expression is an integer literal (so
    // its type generally depends on its value and suffix if any---even with a
    // suffix like L it could still be either long or long long):
    if ((t1->isIntegralType(compiler.getASTContext())
         || t1->isRealFloatingType())
        && ((t1 != t2
             && (loplugin::TypeCheck(t1).Typedef()
                 || loplugin::TypeCheck(t2).Typedef()))
            || isArithmeticOp(subExpr)
            || isa<IntegerLiteral>(subExpr->IgnoreParenImpCasts())))
    {
        return false;
    }
    return true;
}

bool RedundantCast::VisitCXXStaticCastExpr(CXXStaticCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto t1 = compat::getSubExprAsWritten(expr)->getType();
    auto t2 = expr->getTypeAsWritten();
    if (t1.getCanonicalType() != t2.getCanonicalType()) {
        return true;
    }
    if (!isOkToRemoveArithmeticCast(t1, t2, expr->getSubExpr())) {
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
    report(
        DiagnosticsEngine::Warning,
        "redundant static_cast from %0 to %1", expr->getExprLoc())
        << t1 << t2 << expr->getSourceRange();
    return true;
}

bool RedundantCast::VisitCXXReinterpretCastExpr(
    CXXReinterpretCastExpr const * expr)
{
    if (ignoreLocation(expr)) {
        return true;
    }
    if (expr->getSubExpr()->getType()->isVoidPointerType()) {
        auto t = expr->getType()->getAs<clang::PointerType>();
        if (t == nullptr || !t->getPointeeType()->isObjectType()) {
            return true;
        }
        if (rewriter != nullptr) {
            auto loc = expr->getLocStart();
            while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
                loc = compiler.getSourceManager().getImmediateMacroCallerLoc(
                    loc);
            }
            if (compiler.getSourceManager().isMacroBodyExpansion(loc)) {
                auto loc2 = expr->getLocEnd();
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
    }
    return true;
}

bool RedundantCast::VisitCXXConstCastExpr(CXXConstCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    if (isRedundantConstCast(expr)) {
        report(
            DiagnosticsEngine::Warning, "redundant const_cast from %0 to %1",
            expr->getExprLoc())
            << expr->getSubExprAsWritten()->getType()
            << expr->getTypeAsWritten() << expr->getSourceRange();
    }
    return true;
}

bool RedundantCast::VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    // A bit of a rabbit hole here,  these expressions look like
    //   CPPUNIT_ASSERT( bool(aVec.find(p1.get()) == aVec.end()) )
    // If I remove the bool, then another plugin wants me to change it to CPPUNIT_ASSERT_EQUAL,
    // but that fails because CppUnit can't do "std::ostream << iterator".
    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(expr->getLocStart()));
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/o3tl/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sfx2/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/postprocess/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sc/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/cppu/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/vcl/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/cppuhelper/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/comphelper/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/connectivity/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sal/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/salhelper/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sw/qa/"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/svl/qa/"))
        return true;
    // the array-of-struct initialiser here makes clang unhappy if I remove all of the "SchemeInfo" names
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/tools/source/fsys/urlobj.cxx"))
        return true;
    // 2 structs with compiled-generated constructors where I cannot remove the cast even though the cast is a NoOp
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/tools/source/inet/inetmime.cxx"))
        return true;
    // some explicit use of std::initializer_list
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/svx/source/sidebar/area/AreaPropertyPanel.cxx"))
        return true;
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/svx/source/tbxctrls/fillctrl.cxx"))
        return true;

    auto const t1 = expr->getTypeAsWritten();
    auto const t2 = compat::getSubExprAsWritten(expr)->getType();
    if (t1 != t2)
        return true;
    if (!isOkToRemoveArithmeticCast(t1, t2, expr->getSubExpr()))
        return true;
    report(
        DiagnosticsEngine::Warning,
        "redundant functional cast from/to %0", expr->getExprLoc())
        << t1 << expr->getSourceRange();
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
