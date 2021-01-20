/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstddef>
#include <utility>

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"

#include "config_clang.h"

// Compatibility wrapper to abstract over (trivial) changes in the Clang API:
namespace compat {

// Copies code from LLVM's include/llvm/Support/Casting.h:
template<typename... X, typename Y> LLVM_NODISCARD inline bool isa_and_nonnull(Y const & Val) {
#if CLANG_VERSION >= 90000
    return llvm::isa_and_nonnull<X...>(Val);
#else
    if (!Val) {
        return false;
    }
    return isa<X...>(Val);
#endif
}

inline clang::SourceLocation getBeginLoc(clang::Decl const * decl) {
#if CLANG_VERSION >= 80000
    return decl->getBeginLoc();
#else
    return decl->getLocStart();
#endif
}

inline clang::SourceLocation getEndLoc(clang::Decl const * decl) {
#if CLANG_VERSION >= 80000
    return decl->getEndLoc();
#else
    return decl->getLocEnd();
#endif
}

inline clang::SourceLocation getBeginLoc(clang::DeclarationNameInfo const & info) {
#if CLANG_VERSION >= 80000
    return info.getBeginLoc();
#else
    return info.getLocStart();
#endif
}

inline clang::SourceLocation getEndLoc(clang::DeclarationNameInfo const & info) {
#if CLANG_VERSION >= 80000
    return info.getEndLoc();
#else
    return info.getLocEnd();
#endif
}

inline clang::SourceLocation getBeginLoc(clang::Stmt const * stmt) {
#if CLANG_VERSION >= 80000
    return stmt->getBeginLoc();
#else
    return stmt->getLocStart();
#endif
}

inline clang::SourceLocation getEndLoc(clang::Stmt const * stmt) {
#if CLANG_VERSION >= 80000
    return stmt->getEndLoc();
#else
    return stmt->getLocEnd();
#endif
}

inline clang::SourceLocation getBeginLoc(clang::CXXBaseSpecifier const * spec) {
#if CLANG_VERSION >= 80000
    return spec->getBeginLoc();
#else
    return spec->getLocStart();
#endif
}

inline clang::SourceLocation getEndLoc(clang::CXXBaseSpecifier const * spec) {
#if CLANG_VERSION >= 80000
    return spec->getEndLoc();
#else
    return spec->getLocEnd();
#endif
}

inline std::pair<clang::SourceLocation, clang::SourceLocation> getImmediateExpansionRange(
    clang::SourceManager const & SM, clang::SourceLocation Loc)
{
#if CLANG_VERSION >= 70000
    auto const csr = SM.getImmediateExpansionRange(Loc);
    if (csr.isCharRange()) { /*TODO*/ }
    return {csr.getBegin(), csr.getEnd()};
#else
    return SM.getImmediateExpansionRange(Loc);
#endif
}

inline bool isPointWithin(
    clang::SourceManager const & SM, clang::SourceLocation Location, clang::SourceLocation Start,
    clang::SourceLocation End)
{
#if CLANG_VERSION >= 60000
    return SM.isPointWithin(Location, Start, End);
#else
    return
        Location == Start || Location == End
        || (SM.isBeforeInTranslationUnit(Start, Location)
            && SM.isBeforeInTranslationUnit(Location, End));
#endif
}

inline clang::Expr const * IgnoreImplicit(clang::Expr const * expr) {
#if CLANG_VERSION >= 80000
    return expr->IgnoreImplicit();
#else
    using namespace clang;
    // Copy from Clang's lib/AST/Stmt.cpp, including <https://reviews.llvm.org/D50666> "Fix
    // Stmt::ignoreImplicit":
    Stmt const *s = expr;

    Stmt const *lasts = nullptr;

    while (s != lasts) {
        lasts = s;

        if (auto *ewc = dyn_cast<ExprWithCleanups>(s))
            s = ewc->getSubExpr();

        if (auto *mte = dyn_cast<MaterializeTemporaryExpr>(s))
            s = mte->GetTemporaryExpr();

        if (auto *bte = dyn_cast<CXXBindTemporaryExpr>(s))
            s = bte->getSubExpr();

        if (auto *ice = dyn_cast<ImplicitCastExpr>(s))
            s = ice->getSubExpr();
    }

    return static_cast<Expr const *>(s);
#endif
}

inline bool CPlusPlus17(clang::LangOptions const & opts) {
#if CLANG_VERSION >= 60000
    return opts.CPlusPlus17;
#else
    return opts.CPlusPlus1z;
#endif
}

inline bool EvaluateAsInt(clang::Expr const * expr, llvm::APSInt& intRes, const clang::ASTContext& ctx) {
#if CLANG_VERSION >= 80000
    clang::Expr::EvalResult res;
    bool b = expr->EvaluateAsInt(res, ctx);
    if (b && res.Val.isInt())
        intRes = res.Val.getInt();
    return b;
#else
    return expr->EvaluateAsInt(intRes, ctx);
#endif
}

inline llvm::Optional<llvm::APSInt> getIntegerConstantExpr(
    clang::Expr const * expr, clang::ASTContext const & context)
{
#if CLANG_VERSION >= 120000
    return expr->getIntegerConstantExpr(context);
#else
    llvm::APSInt res;
    return expr->isIntegerConstantExpr(res, context) ? res : llvm::Optional<llvm::APSInt>();
#endif
}

inline clang::Expr * getSubExpr(clang::MaterializeTemporaryExpr const * expr) {
#if CLANG_VERSION >= 100000
    return expr->getSubExpr();
#else
    return expr->GetTemporaryExpr();
#endif
}

#if CLANG_VERSION < 80000
inline clang::Expr const * getSubExprAsWritten(clang::CastExpr const * expr)
{ return expr->getSubExprAsWritten(); }
#else
// Work around CastExpr::getSubExprAsWritten firing
//
//   include/llvm/Support/Casting.h:269: typename llvm::cast_retty<X, Y*>::ret_type llvm::cast(Y*)
//   [with X = clang::CXXConstructExpr; Y = clang::Expr;
//   typename llvm::cast_retty<X, Y*>::ret_type = clang::CXXConstructExpr*]: Assertion
//   `isa<X>(Val) && "cast<Ty>() argument of incompatible type!"' failed.
//
// for CastExprs involving ConstantExpr (introduced with
// <https://github.com/llvm/llvm-project/commit/7c44da279e399d302a685c500e7f802f8adf9762> "Create
// ConstantExpr class" towards LLVM 8) like
//
//   CXXFunctionalCastExpr 0xc01c4e8 'class rtl::OStringLiteral<9>':'class rtl::OStringLiteral<9>' functional cast to OStringLiteral <ConstructorConversion>
//   `-ConstantExpr 0xc01c380 'class rtl::OStringLiteral<9>':'class rtl::OStringLiteral<9>'
//     |-value: Struct
//     | |-fields: Int 1073741824, Int 8
//     | `-field: Array size=9
//     |   |-elements: Int 46, Int 111, Int 115, Int 108
//     |   |-elements: Int 45, Int 116, Int 109, Int 112
//     |   `-element: Int 0
//     `-CXXConstructExpr 0xc01c350 'class rtl::OStringLiteral<9>':'class rtl::OStringLiteral<9>' 'void (const char (&)[9])'
//       `-StringLiteral 0xc019ad8 'const char [9]' lvalue ".osl-tmp"
//
// Copies code from Clang's lib/AST/Expr.cpp:
namespace detail {
  inline clang::Expr *skipImplicitTemporary(clang::Expr *expr) {
    // Skip through reference binding to temporary.
    if (clang::MaterializeTemporaryExpr *Materialize
                                  = clang::dyn_cast<clang::MaterializeTemporaryExpr>(expr))
      expr = compat::getSubExpr(Materialize);

    // Skip any temporary bindings; they're implicit.
    if (clang::CXXBindTemporaryExpr *Binder = clang::dyn_cast<clang::CXXBindTemporaryExpr>(expr))
      expr = Binder->getSubExpr();

    return expr;
  }
}
inline clang::Expr *getSubExprAsWritten(clang::CastExpr *This) {
  clang::Expr *SubExpr = nullptr;
  clang::CastExpr *E = This;
  do {
    SubExpr = detail::skipImplicitTemporary(E->getSubExpr());

    // Conversions by constructor and conversion functions have a
    // subexpression describing the call; strip it off.
    if (E->getCastKind() == clang::CK_ConstructorConversion)
      SubExpr =
        detail::skipImplicitTemporary(clang::cast<clang::CXXConstructExpr>(SubExpr->IgnoreImplicit())->getArg(0));
    else if (E->getCastKind() == clang::CK_UserDefinedConversion) {
      assert((clang::isa<clang::CXXMemberCallExpr>(SubExpr) ||
              clang::isa<clang::BlockExpr>(SubExpr)) &&
             "Unexpected SubExpr for CK_UserDefinedConversion.");
      if (clang::isa<clang::CXXMemberCallExpr>(SubExpr))
        SubExpr = clang::cast<clang::CXXMemberCallExpr>(SubExpr)->getImplicitObjectArgument();
    }

    // If the subexpression we're left with is an implicit cast, look
    // through that, too.
  } while ((E = clang::dyn_cast<clang::ImplicitCastExpr>(SubExpr)));

  return SubExpr;
}
inline const clang::Expr *getSubExprAsWritten(const clang::CastExpr *This) {
  return getSubExprAsWritten(const_cast<clang::CastExpr *>(This));
}
#endif

inline clang::QualType getObjectType(clang::CXXMemberCallExpr const * expr) {
#if CLANG_VERSION >= 100000
    return expr->getObjectType();
#else
    // <https://github.com/llvm/llvm-project/commit/88559637641e993895337e1047a0bd787fecc647>
    // "[OpenCL] Improve destructor support in C++ for OpenCL":
    clang::QualType Ty = expr->getImplicitObjectArgument()->getType();
    if (Ty->isPointerType())
        Ty = Ty->getPointeeType();
    return Ty;
#endif
}

inline bool isExplicitSpecified(clang::CXXConstructorDecl const * decl) {
#if CLANG_VERSION >= 90000
    return decl->getExplicitSpecifier().isExplicit();
#else
    return decl->isExplicitSpecified();
#endif
}

inline bool isExplicitSpecified(clang::CXXConversionDecl const * decl) {
#if CLANG_VERSION >= 90000
    return decl->getExplicitSpecifier().isExplicit();
#else
    return decl->isExplicitSpecified();
#endif
}

inline clang::QualType getDeclaredReturnType(clang::FunctionDecl const * decl) {
#if CLANG_VERSION >= 80000
    return decl->getDeclaredReturnType();
#else
    // <https://github.com/llvm/llvm-project/commit/4576a77b809649f5b8d0ff8c7a4be57eeee0ecf9>
    // "PR33222: Require the declared return type not the actual return type to":
    auto *TSI = decl->getTypeSourceInfo();
    clang::QualType T = TSI ? TSI->getType() : decl->getType();
    return T->castAs<clang::FunctionType>()->getReturnType();
#endif
}

// The isComparisonOp method on CXXOperatorCallExpr is not available yet for the clang we require
inline bool isComparisonOp(clang::CXXOperatorCallExpr const * callExpr)
{
    using namespace clang;
    auto op = callExpr->getOperator();
    return op == OO_Less || op == OO_Greater || op == OO_LessEqual || op == OO_GreaterEqual
           || op == OO_EqualEqual || op == OO_ExclaimEqual;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
