/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <utility>

#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Specifiers.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"

#include "config_clang.h"

#if CLANG_VERSION >= 170000
#include <optional>
#else
#include "llvm/ADT/Optional.h"
#endif

// Compatibility wrapper to abstract over (trivial) changes in the Clang API:
namespace compat {

template<typename T>
#if CLANG_VERSION >= 170000
using optional = std::optional<T>;
#else
using optional = llvm::Optional<T>;
#endif

template<typename T>
constexpr bool has_value(optional<T> const & o) {
#if CLANG_VERSION >= 150000
    return o.has_value();
#else
    return o.hasValue();
#endif
}

template<typename T>
constexpr T const & value(optional<T> const & o) {
#if CLANG_VERSION >= 150000
    return *o;
#else
    return o.getValue();
#endif
}

inline std::string toString(llvm::APSInt const & i, unsigned radix) {
#if CLANG_VERSION >= 130000
    return llvm::toString(i, radix);
#else
    return i.toString(radix);
#endif
}

inline bool starts_with(llvm::StringRef s, llvm::StringRef Prefix) {
#if CLANG_VERSION >= 160000
    return s.starts_with(Prefix);
#else
    return s.startswith(Prefix);
#endif
}

inline bool ends_with(llvm::StringRef s, llvm::StringRef Suffix) {
#if CLANG_VERSION >= 160000
    return s.ends_with(Suffix);
#else
    return s.endswith(Suffix);
#endif
}

inline std::pair<clang::SourceLocation, clang::SourceLocation> getImmediateExpansionRange(
    clang::SourceManager const & SM, clang::SourceLocation Loc)
{
    auto const csr = SM.getImmediateExpansionRange(Loc);
    if (csr.isCharRange()) { /*TODO*/ }
    return {csr.getBegin(), csr.getEnd()};
}

/// Utility method
inline clang::Expr const * IgnoreParenImplicit(clang::Expr const * expr) {
    return expr->IgnoreImplicit()->IgnoreParens()->IgnoreImplicit();
}

#if CLANG_VERSION >= 130000
constexpr clang::ExprValueKind VK_PRValue = clang::VK_PRValue;
#else
constexpr clang::ExprValueKind VK_PRValue = clang::VK_RValue;
#endif

namespace CXXConstructionKind
{
#if CLANG_VERSION >= 180000
constexpr clang::CXXConstructionKind Complete = clang::CXXConstructionKind::Complete;
#else
constexpr clang::CXXConstructExpr::ConstructionKind Complete = clang::CXXConstructExpr::CK_Complete;
#endif
}

namespace CharacterLiteralKind
{
#if CLANG_VERSION >= 180000
constexpr clang::CharacterLiteralKind Ascii = clang::CharacterLiteralKind::Ascii;
#else
constexpr clang::CharacterLiteral::CharacterKind Ascii = clang::CharacterLiteral::Ascii;
#endif
}

namespace ElaboratedTypeKeyword
{
#if CLANG_VERSION >= 180000
constexpr clang::ElaboratedTypeKeyword None = clang::ElaboratedTypeKeyword::None;
#else
constexpr clang::ElaboratedTypeKeyword None = clang::ETK_None;
#endif
}

namespace Linkage
{
#if CLANG_VERSION >= 180000
constexpr clang::Linkage External = clang::Linkage::External;
constexpr clang::Linkage Module = clang::Linkage::Module;
#else
constexpr clang::Linkage External = clang::ExternalLinkage;
constexpr clang::Linkage Module = clang::ModuleLinkage;
#endif
}

namespace StringLiteralKind
{
#if CLANG_VERSION >= 180000
constexpr clang::StringLiteralKind UTF8 = clang::StringLiteralKind::UTF8;
#else
constexpr clang::StringLiteral::StringKind UTF8 = clang::StringLiteral::UTF8;
#endif
}

namespace TagTypeKind
{
#if CLANG_VERSION >= 180000
constexpr clang::TagTypeKind Class = clang::TagTypeKind::Class;
constexpr clang::TagTypeKind Struct = clang::TagTypeKind::Struct;
constexpr clang::TagTypeKind Union = clang::TagTypeKind::Union;
#else
constexpr clang::TagTypeKind Class = clang::TTK_Class;
constexpr clang::TagTypeKind Struct = clang::TTK_Struct;
constexpr clang::TagTypeKind Union = clang::TTK_Union;
#endif
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

inline bool isOrdinary(clang::StringLiteral const * expr) {
#if CLANG_VERSION >= 150000
    return expr->isOrdinary();
#else
    return expr->isAscii();
#endif
}

inline bool isPureVirtual(clang::FunctionDecl const * decl) {
#if CLANG_VERSION >= 180000
    return decl->isPureVirtual();
#else
    return decl->isPure();
#endif
}

inline bool isUnnamedBitField(clang::FieldDecl const * decl) {
#if CLANG_VERSION >= 190000
    return decl->isUnnamedBitField();
#else
    return decl->isUnnamedBitfield();
#endif
}

inline clang::TemplateTypeParmDecl const * getReplacedParameter(
    clang::SubstTemplateTypeParmType const * type)
{
#if CLANG_VERSION >= 160000
    return type->getReplacedParameter();
#else
    return type->getReplacedParameter()->getDecl();
#endif
}

// Printing `std::size_t n` via `report(...) << n` is ambiguous prior to
// <https://github.com/llvm/llvm-project/commit/afdac5fbcb6a375245d435e4427086a376de59ff> "[clang]
// Allow printing 64 bit ints in diagnostics" (in Clang 14.x) and its follow-up
// <https://github.com/llvm/llvm-project/commit/ac7a9ef0ae3a5c63dc4e641f9912d8b659ebd720> "Resolve
// overload ambiguity on Mac OS when printing size_t in diagnostics" (in Clang 15.x):
inline
#if CLANG_VERSION >= 150000
std::size_t
#else
unsigned
#endif
diagnosticSize(std::size_t n) { return n; }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
