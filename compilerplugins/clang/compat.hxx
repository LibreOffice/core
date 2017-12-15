/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPILERPLUGINS_CLANG_COMPAT_HXX
#define INCLUDED_COMPILERPLUGINS_CLANG_COMPAT_HXX

#include <cstddef>

#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/StringRef.h"

#include "config_clang.h"

// Compatibility wrapper to abstract over (trivial) changes in the Clang API:
namespace compat {

inline llvm::StringRef take_front(llvm::StringRef ref, std::size_t N = 1) {
#if CLANG_VERSION >= 40000
    return ref.take_front(N);
#else
    auto const size = ref.size();
    return N >= size ? ref : ref.drop_back(size - N);
#endif
}


#if CLANG_VERSION >= 30900
inline clang::ArrayRef<clang::ParmVarDecl *> parameters(
    clang::FunctionDecl const & decl)
{
    return decl.parameters();
}
#else
inline clang::FunctionDecl::param_const_range parameters(
    clang::FunctionDecl const & decl)
{
    return decl.params();
}
#endif


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

inline bool isMacroArgExpansion(
    clang::CompilerInstance& compiler, clang::SourceLocation location,
    clang::SourceLocation * startLocation)
{
#if CLANG_VERSION >= 30900
    return compiler.getSourceManager().isMacroArgExpansion(
        location, startLocation);
#else
    bool b = compiler.getSourceManager().isMacroArgExpansion(location);
    if (b) {
        *startLocation = compiler.getSourceManager()
            .getSLocEntry(compiler.getSourceManager().getFileID(location))
            .getExpansion().getExpansionLocStart();
    }
    return b;
#endif
}

inline llvm::StringRef getImmediateMacroNameForDiagnostics(
    clang::SourceLocation Loc, clang::SourceManager const & SM,
    clang::LangOptions const &LangOpts)
{
#if CLANG_VERSION >= 30900
    return clang::Lexer::getImmediateMacroNameForDiagnostics(Loc, SM, LangOpts);
#else
    using namespace clang;
    // Verbatim copy from Clang's lib/Lex/Lexer.cpp:

    assert(Loc.isMacroID() && "Only reasonable to call this on macros");
    // Walk past macro argument expansion.
    while (SM.isMacroArgExpansion(Loc))
        Loc = SM.getImmediateExpansionRange(Loc).first;

    // If the macro's spelling has no FileID, then it's actually a token paste
    // or stringization (or similar) and not a macro at all.
    if (!SM.getFileEntryForID(SM.getFileID(SM.getSpellingLoc(Loc))))
        return StringRef();

    // Find the spelling location of the start of the non-argument expansion
    // range. This is where the macro name was spelled in order to begin
    // expanding this macro.
    Loc = SM.getSpellingLoc(SM.getImmediateExpansionRange(Loc).first);

    // Dig out the buffer where the macro name was spelled and the extents of
    // the name so that we can render it into the expansion note.
    std::pair<FileID, unsigned> ExpansionInfo = SM.getDecomposedLoc(Loc);
    unsigned MacroTokenLength = Lexer::MeasureTokenLength(Loc, SM, LangOpts);
    StringRef ExpansionBuffer = SM.getBufferData(ExpansionInfo.first);
    return ExpansionBuffer.substr(ExpansionInfo.second, MacroTokenLength);
#endif
}

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
namespace detail {
  inline clang::Expr *skipImplicitTemporary(clang::Expr *expr) {
    // Skip through reference binding to temporary.
    if (clang::MaterializeTemporaryExpr *Materialize
                                  = clang::dyn_cast<clang::MaterializeTemporaryExpr>(expr))
      expr = Materialize->GetTemporaryExpr();

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
        detail::skipImplicitTemporary(clang::cast<clang::CXXConstructExpr>(SubExpr)->getArg(0));
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

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
