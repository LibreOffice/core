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

#include <memory>
#include <string>

#include "clang/AST/Decl.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
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

#if CLANG_VERSION >= 30400
#define LO_COMPILERPLUGINS_CLANG_COMPAT_HAVE_isAtEndOfImmediateMacroExpansion \
    true
#else
#define LO_COMPILERPLUGINS_CLANG_COMPAT_HAVE_isAtEndOfImmediateMacroExpansion \
    false
#endif

// Compatibility wrapper to abstract over (trivial) changes in the Clang API:
namespace compat {

inline bool isLookupContext(clang::DeclContext const & ctxt) {
#if CLANG_VERSION >= 30700
    return ctxt.isLookupContext();
#else
    return !ctxt.isFunctionOrMethod()
        && ctxt.getDeclKind() != clang::Decl::LinkageSpec;
#endif
}

inline bool isExternCContext(clang::DeclContext const & ctxt) {
#if CLANG_VERSION >= 30400
    return ctxt.isExternCContext();
#else
    for (clang::DeclContext const * c = &ctxt;
         c->getDeclKind() != clang::Decl::TranslationUnit; c = c->getParent())
    {
        if (c->getDeclKind() == clang::Decl::LinkageSpec) {
            return llvm::cast<clang::LinkageSpecDecl>(c)->getLanguage()
                == clang::LinkageSpecDecl::lang_c;
        }
    }
    return false;
#endif
}

inline bool isInExternCContext(clang::FunctionDecl const & decl) {
#if CLANG_VERSION >= 30400
    return decl.isInExternCContext();
#else
    return isExternCContext(*decl.getCanonicalDecl()->getDeclContext());
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

#if CLANG_VERSION >= 30300
typedef clang::LinkageInfo LinkageInfo;
#else
typedef clang::NamedDecl::LinkageInfo LinkageInfo;
#endif

inline clang::Linkage getLinkage(LinkageInfo const & info) {
#if CLANG_VERSION >= 30300
    return info.getLinkage();
#else
    return info.linkage();
#endif
}

inline clang::Visibility getVisibility(LinkageInfo const & info) {
#if CLANG_VERSION >= 30300
    return info.getVisibility();
#else
    return info.visibility();
#endif
}

inline bool isFirstDecl(clang::FunctionDecl const & decl) {
#if CLANG_VERSION >= 30400
    return decl.isFirstDecl();
#else
    return decl.isFirstDeclaration();
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

inline bool isInMainFile(
    clang::SourceManager const & manager, clang::SourceLocation Loc)
{
#if CLANG_VERSION >= 30400
    return manager.isInMainFile(Loc);
#else
    return manager.isFromMainFile(Loc);
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
typedef clang::DeclContext::lookup_result DeclContextLookupResult;
typedef clang::DeclContext::lookup_iterator DeclContextLookupIterator;
#else
typedef clang::DeclContext::lookup_const_result DeclContextLookupResult;
typedef clang::DeclContext::lookup_const_iterator DeclContextLookupIterator;
#endif

inline DeclContextLookupIterator begin(DeclContextLookupResult const & result) {
#if CLANG_VERSION >= 30300
    return result.begin();
#else
    return result.first;
#endif
}

inline DeclContextLookupIterator end(DeclContextLookupResult const & result) {
#if CLANG_VERSION >= 30300
    return result.end();
#else
    return result.second;
#endif
}

inline void addPPCallbacks(
    clang::Preprocessor & preprocessor, clang::PPCallbacks * C)
{
#if CLANG_VERSION >= 30600
    preprocessor.addPPCallbacks(std::unique_ptr<clang::PPCallbacks>(C));
#else
    preprocessor.addPPCallbacks(C);
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

inline bool isMacroBodyExpansion(clang::CompilerInstance& compiler, clang::SourceLocation location)
{
#if CLANG_VERSION >= 30300
    return compiler.getSourceManager().isMacroBodyExpansion(location);
#else
    return location.isMacroID()
        && !compiler.getSourceManager().isMacroArgExpansion(location);
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

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
