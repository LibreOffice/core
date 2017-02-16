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
#include "plugin.hxx"

namespace {

// Work around <http://reviews.llvm.org/D22128>:
//
// SfxErrorHandler::GetClassString (svtools/source/misc/ehdl.cxx):
//
//   ErrorResource_Impl aEr(aId, (sal_uInt16)lClassId);
//   if(aEr)
//   {
//       rStr = static_cast<ResString>(aEr).GetString();
//   }
//
// expr->dump():
//  CXXStaticCastExpr 0x2b74e8e657b8 'class ResString' static_cast<class ResString> <ConstructorConversion>
//  `-CXXBindTemporaryExpr 0x2b74e8e65798 'class ResString' (CXXTemporary 0x2b74e8e65790)
//    `-CXXConstructExpr 0x2b74e8e65758 'class ResString' 'void (class ResString &&) noexcept(false)' elidable
//      `-MaterializeTemporaryExpr 0x2b74e8e65740 'class ResString' xvalue
//        `-CXXBindTemporaryExpr 0x2b74e8e65720 'class ResString' (CXXTemporary 0x2b74e8e65718)
//          `-ImplicitCastExpr 0x2b74e8e65700 'class ResString' <UserDefinedConversion>
//            `-CXXMemberCallExpr 0x2b74e8e656d8 'class ResString'
//              `-MemberExpr 0x2b74e8e656a0 '<bound member function type>' .operator ResString 0x2b74e8dc1f00
//                `-DeclRefExpr 0x2b74e8e65648 'struct ErrorResource_Impl' lvalue Var 0x2b74e8e653b0 'aEr' 'struct ErrorResource_Impl'
// expr->getSubExprAsWritten()->dump():
//  MaterializeTemporaryExpr 0x2b74e8e65740 'class ResString' xvalue
//  `-CXXBindTemporaryExpr 0x2b74e8e65720 'class ResString' (CXXTemporary 0x2b74e8e65718)
//    `-ImplicitCastExpr 0x2b74e8e65700 'class ResString' <UserDefinedConversion>
//      `-CXXMemberCallExpr 0x2b74e8e656d8 'class ResString'
//        `-MemberExpr 0x2b74e8e656a0 '<bound member function type>' .operator ResString 0x2b74e8dc1f00
//          `-DeclRefExpr 0x2b74e8e65648 'struct ErrorResource_Impl' lvalue Var 0x2b74e8e653b0 'aEr' 'struct ErrorResource_Impl'
//
// Copies code from Clang's lib/AST/Expr.cpp:
namespace {
  Expr *skipImplicitTemporary(Expr *expr) {
    // Skip through reference binding to temporary.
    if (MaterializeTemporaryExpr *Materialize
                                  = dyn_cast<MaterializeTemporaryExpr>(expr))
      expr = Materialize->GetTemporaryExpr();

    // Skip any temporary bindings; they're implicit.
    if (CXXBindTemporaryExpr *Binder = dyn_cast<CXXBindTemporaryExpr>(expr))
      expr = Binder->getSubExpr();

    return expr;
  }
}
Expr *getSubExprAsWritten(CastExpr *This) {
  Expr *SubExpr = nullptr;
  CastExpr *E = This;
  do {
    SubExpr = skipImplicitTemporary(E->getSubExpr());

    // Conversions by constructor and conversion functions have a
    // subexpression describing the call; strip it off.
    if (E->getCastKind() == CK_ConstructorConversion)
      SubExpr =
        skipImplicitTemporary(cast<CXXConstructExpr>(SubExpr)->getArg(0));
    else if (E->getCastKind() == CK_UserDefinedConversion) {
      assert((isa<CXXMemberCallExpr>(SubExpr) ||
              isa<BlockExpr>(SubExpr)) &&
             "Unexpected SubExpr for CK_UserDefinedConversion.");
      if (isa<CXXMemberCallExpr>(SubExpr))
        SubExpr = cast<CXXMemberCallExpr>(SubExpr)->getImplicitObjectArgument();
    }

    // If the subexpression we're left with is an implicit cast, look
    // through that, too.
  } while ((E = dyn_cast<ImplicitCastExpr>(SubExpr)));

  return SubExpr;
}
const Expr *getSubExprAsWritten(const CastExpr *This) {
  return getSubExprAsWritten(const_cast<CastExpr *>(This));
}

bool isVoidPointer(QualType type) {
    return type->isPointerType()
        && type->getAs<clang::PointerType>()->getPointeeType()->isVoidType();
}

bool isRedundantConstCast(CXXConstCastExpr const * expr) {
    return expr->getTypeAsWritten().getCanonicalType().getTypePtr()
        == (expr->getSubExprAsWritten()->getType().getCanonicalType()
            .getTypePtr());
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
                if (expr->getType()->getAs<clang::PointerType>()
                    ->getPointeeType().isAtLeastAsQualifiedAs(
                        cc->getSubExpr()->getType()
                        ->getAs<clang::PointerType>()->getPointeeType()))
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

bool RedundantCast::VisitCXXStaticCastExpr(CXXStaticCastExpr const * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    auto t1 = getSubExprAsWritten(expr)->getType();
    auto t2 = expr->getTypeAsWritten();
    if (t1.getCanonicalType() != t2.getCanonicalType()
        || t1->isArithmeticType())
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
