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
#include <memory>
#include <string>

#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Type.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/Linkage.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Visibility.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

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

inline bool isLookupContext(clang::DeclContext const & ctxt) {
#if CLANG_VERSION >= 30700
    return ctxt.isLookupContext();
#else
    return !ctxt.isFunctionOrMethod()
        && ctxt.getDeclKind() != clang::Decl::LinkageSpec;
#endif
}

inline bool forallBases(
    clang::CXXRecordDecl const & decl,
    clang::CXXRecordDecl::ForallBasesCallback BaseMatches,
    void* callbackParam,
    bool AllowShortCircuit)
{
#if CLANG_VERSION >= 30800
    (void) callbackParam;
    return decl.forallBases(BaseMatches, AllowShortCircuit);
#else
    return decl.forallBases(BaseMatches, callbackParam, AllowShortCircuit);
#endif
}

inline clang::QualType getReturnType(clang::FunctionDecl const & decl) {
#if CLANG_VERSION >= 30500
    return decl.getReturnType();
#else
    return decl.getResultType();
#endif
}


#if CLANG_VERSION >= 30900
inline clang::ArrayRef<clang::ParmVarDecl *> parameters(
    clang::FunctionDecl const & decl)
{
    return decl.parameters();
}
#elif CLANG_VERSION >= 30500
inline clang::FunctionDecl::param_const_range parameters(
    clang::FunctionDecl const & decl)
{
    return decl.params();
}
#else
struct FunctionDeclParamsWrapper
{
    clang::FunctionDecl const & decl;
    FunctionDeclParamsWrapper(clang::FunctionDecl const & _decl) : decl(_decl) {}
    clang::FunctionDecl::param_const_iterator begin() const { return decl.param_begin(); }
    clang::FunctionDecl::param_const_iterator end() const { return decl.param_end(); }
};
inline FunctionDeclParamsWrapper parameters(
    clang::FunctionDecl const & decl)
{
    return FunctionDeclParamsWrapper(decl);
}
#endif


inline clang::QualType getReturnType(clang::FunctionProtoType const & type) {
#if CLANG_VERSION >= 30500
    return type.getReturnType();
#else
    return type.getResultType();
#endif
}

inline unsigned getNumParams(clang::FunctionProtoType const & type) {
#if CLANG_VERSION >= 30500
    return type.getNumParams();
#else
    return type.getNumArgs();
#endif
}

inline clang::QualType getParamType(
    clang::FunctionProtoType const & type, unsigned i)
{
#if CLANG_VERSION >= 30500
    return type.getParamType(i);
#else
    return type.getArgType(i);
#endif
}

inline clang::Stmt::const_child_iterator begin(
    clang::Stmt::const_child_range const & range)
{
#if CLANG_VERSION >= 30800
    return range.begin();
#else
    return range.first;
#endif
}

inline clang::Stmt::const_child_iterator end(
    clang::Stmt::const_child_range const & range)
{
#if CLANG_VERSION >= 30800
    return range.end();
#else
    return range.second;
#endif
}

inline unsigned getBuiltinCallee(clang::CallExpr const & expr) {
#if CLANG_VERSION >= 30500
    return expr.getBuiltinCallee();
#else
    return expr.isBuiltinCall();
#endif
}

inline unsigned getCustomDiagID(
    clang::DiagnosticsEngine & engine, clang::DiagnosticsEngine::Level L,
    llvm::StringRef FormatString)
{
#if CLANG_VERSION >= 30500
    return engine.getDiagnosticIDs()->getCustomDiagID(
        static_cast<clang::DiagnosticIDs::Level>(L), FormatString);
#else
    return engine.getCustomDiagID(L, FormatString);
#endif
}

inline std::unique_ptr<llvm::raw_fd_ostream> create_raw_fd_ostream(
    char const * Filename, std::string & ErrorInfo)
{
#if CLANG_VERSION >= 30600
    std::error_code ec;
    std::unique_ptr<llvm::raw_fd_ostream> s(
        new llvm::raw_fd_ostream(Filename, ec, llvm::sys::fs::F_None));
    ErrorInfo = ec ? "error: " + ec.message() : std::string();
    return s;
#elif CLANG_VERSION >= 30500
    return std::unique_ptr<llvm::raw_fd_ostream>(
        new llvm::raw_fd_ostream(Filename, ErrorInfo, llvm::sys::fs::F_None));
#else
    return std::unique_ptr<llvm::raw_fd_ostream>(
        new llvm::raw_fd_ostream(Filename, ErrorInfo));
#endif
}

#if CLANG_VERSION >= 30700
using MacroDefinitionParam = clang::MacroDefinition const &;
#else
using MacroDefinitionParam = clang::MacroDirective const *;
#endif

inline void addPPCallbacks(
    clang::Preprocessor & preprocessor, clang::PPCallbacks * C)
{
#if CLANG_VERSION >= 30600
    preprocessor.addPPCallbacks(std::unique_ptr<clang::PPCallbacks>(C));
#else
    preprocessor.addPPCallbacks(C);
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

inline auto getAsTagDecl(clang::Type const& t) -> clang::TagDecl *
{
#if CLANG_VERSION >= 30500
    // TODO not sure if it works with clang 3.6, trunk is known to work
    return t.getAsTagDecl();
#else
    return t.getAs<clang::TagType>()->getDecl();
#endif
}

inline bool isStdNamespace(clang::DeclContext const & context) {
#if CLANG_VERSION >= 30500
    return context.isStdNamespace();
#else
    // cf. lib/AST/DeclBase.cpp:
    if (!context.isNamespace()) {
        return false;
    }
    const clang::NamespaceDecl *ND = clang::cast<clang::NamespaceDecl>(
        &context);
    if (ND->isInline()) {
        return isStdNamespace(*ND->getParent());
    }
    if (!context.getParent()->getRedeclContext()->isTranslationUnit()) {
        return false;
    }
    const clang::IdentifierInfo *II = ND->getIdentifier();
    return II && II->isStr("std");
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
