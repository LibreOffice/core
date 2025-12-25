/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <utility>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Basic/SourceManager.h"

#include "config_clang.h"

// Compatibility wrapper to abstract over (trivial) changes in the Clang API:
namespace compat {

inline std::pair<clang::SourceLocation, clang::SourceLocation> getImmediateExpansionRange(
    clang::SourceManager const & SM, clang::SourceLocation Loc)
{
    auto const csr = SM.getImmediateExpansionRange(Loc);
    if (csr.isCharRange()) { /*TODO*/ }
    return {csr.getBegin(), csr.getEnd()};
}

inline bool isAtLeastAsQualifiedAs(
    clang::QualType type1, clang::QualType type2, clang::ASTContext const & context)
{
#if CLANG_VERSION >= 200000
    return type1.isAtLeastAsQualifiedAs(type2, context);
#else
    (void) context;
    return type1.isAtLeastAsQualifiedAs(type2);
#endif
}

/// Utility method
inline clang::Expr const * IgnoreParenImplicit(clang::Expr const * expr) {
    return expr->IgnoreImplicit()->IgnoreParens()->IgnoreImplicit();
}

inline bool EvaluateAsInt(clang::Expr const * expr, llvm::APSInt& intRes, const clang::ASTContext& ctx) {
    clang::Expr::EvalResult res;
    bool b = expr->EvaluateAsInt(res, ctx);
    if (b && res.Val.isInt())
        intRes = res.Val.getInt();
    return b;
}

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
      expr = Materialize->getSubExpr();

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

inline bool isUnnamedBitField(clang::FieldDecl const * decl) {
#if CLANG_VERSION >= 190000
    return decl->isUnnamedBitField();
#else
    return decl->isUnnamedBitfield();
#endif
}

inline unsigned getBitWidthValue(clang::FieldDecl const * decl, clang::ASTContext const & context) {
#if CLANG_VERSION >= 200000
    (void) context;
    return decl->getBitWidthValue();
#else
    return decl->getBitWidthValue(context);
#endif
}

inline
clang::QualType getCanonicalTagType(clang::ASTContext const & context, clang::TagDecl const * decl)
{
#if CLANG_VERSION >= 220000
    return context.getCanonicalTagType(decl);
#else
    return context.getTypeDeclType(decl);
#endif
}

inline clang::TagDecl const * getDefinitionOrSelf(clang::TagDecl const * decl) {
#if CLANG_VERSION >= 220000
    return decl->getDefinitionOrSelf();
#else
    if (auto const def = decl->getDefinition()) {
        return def;
    }
    return decl;
#endif
}

inline clang::RecordDecl const * getDecl(clang::RecordType const * type) {
#if CLANG_VERSION >= 220000
    return llvm::cast<clang::RecordDecl>(type->getDecl()->getCanonicalDecl());
#else
    return type->getDecl();
#endif
}

inline clang::EnumDecl const * getDecl(clang::EnumType const * type) {
#if CLANG_VERSION >= 220000
    return type->getDecl()->getCanonicalDecl();
#else
    return type->getDecl();
#endif
}

inline clang::CXXRecordDecl const * getDecl(clang::InjectedClassNameType const * type) {
#if CLANG_VERSION >= 220000
    return type->getDecl()->getCanonicalDecl();
#else
    return type->getDecl();
#endif
}

inline clang::Type const * getClass(clang::MemberPointerType const * type) {
#if CLANG_VERSION >= 220000
    return type->getQualifier().getAsType();
#elif CLANG_VERSION >= 210000
    return type->getQualifier()->getAsType();
#else
    return type->getClass();
#endif
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
