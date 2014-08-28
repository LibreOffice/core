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

#include "clang/AST/ASTContext.h"
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
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3
#define LO_COMPILERPLUGINS_CLANG_COMPAT_HAVE_isAtEndOfImmediateMacroExpansion \
    true
#else
#define LO_COMPILERPLUGINS_CLANG_COMPAT_HAVE_isAtEndOfImmediateMacroExpansion \
    false
#endif

// Compatibility wrapper to abstract over (trivial) changes in the Clang API:
namespace compat {

inline bool isExternCContext(clang::DeclContext const & ctxt) {
#if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3
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

#if (__clang_major__ == 3 && __clang_minor__ >= 3) || __clang_major__ > 3
typedef clang::LinkageInfo LinkageInfo;
#else
typedef clang::NamedDecl::LinkageInfo LinkageInfo;
#endif

inline clang::Linkage getLinkage(LinkageInfo const & info) {
#if (__clang_major__ == 3 && __clang_minor__ >= 3) || __clang_major__ > 3
    return info.getLinkage();
#else
    return info.linkage();
#endif
}

inline clang::Visibility getVisibility(LinkageInfo const & info) {
#if (__clang_major__ == 3 && __clang_minor__ >= 3) || __clang_major__ > 3
    return info.getVisibility();
#else
    return info.visibility();
#endif
}

inline bool isFirstDecl(clang::FunctionDecl const & decl) {
#if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3
    return decl.isFirstDecl();
#else
    return decl.isFirstDeclaration();
#endif
}

inline clang::QualType getReturnType(clang::FunctionDecl const & decl) {
#if (__clang_major__ == 3 && __clang_minor__ >= 5) || __clang_major__ > 3
    return decl.getReturnType();
#else
    return decl.getResultType();
#endif
}

inline clang::SourceRange getReturnTypeSourceRange(clang::FunctionDecl const & decl) {
#if (__clang_major__ == 3 && __clang_minor__ > 5) || __clang_major__ > 3
    return decl.getReturnTypeSourceRange();
#elif (__clang_major__ == 3 && __clang_minor__ == 5)
    const clang::TypeSourceInfo *TSI = decl.getTypeSourceInfo();
    if (!TSI)
        return clang::SourceRange();
    clang::FunctionTypeLoc FTL =
        TSI->getTypeLoc().IgnoreParens().getAs<clang::FunctionTypeLoc>();
    if (!FTL)
        return clang::SourceRange();

    // Skip self-referential return types.
    const clang::SourceManager &SM = decl.getASTContext().getSourceManager();
    clang::SourceRange RTRange = FTL.getReturnLoc().getSourceRange();
    clang::SourceLocation Boundary = decl.getNameInfo().getLocStart();
    if (RTRange.isInvalid() || Boundary.isInvalid() ||
        !SM.isBeforeInTranslationUnit(RTRange.getEnd(), Boundary))
        return clang::SourceRange();

    return RTRange;
#else
#endif
}

inline clang::QualType getReturnType(clang::FunctionProtoType const & type) {
#if (__clang_major__ == 3 && __clang_minor__ >= 5) || __clang_major__ > 3
    return type.getReturnType();
#else
    return type.getResultType();
#endif
}

inline unsigned getNumParams(clang::FunctionProtoType const & type) {
#if (__clang_major__ == 3 && __clang_minor__ >= 5) || __clang_major__ > 3
    return type.getNumParams();
#else
    return type.getNumArgs();
#endif
}

inline clang::QualType getParamType(
    clang::FunctionProtoType const & type, unsigned i)
{
#if (__clang_major__ == 3 && __clang_minor__ >= 5) || __clang_major__ > 3
    return type.getParamType(i);
#else
    return type.getArgType(i);
#endif
}

inline unsigned getBuiltinCallee(clang::CallExpr const & expr) {
#if (__clang_major__ == 3 && __clang_minor__ >= 5) || __clang_major__ > 3
    return expr.getBuiltinCallee();
#else
    return expr.isBuiltinCall();
#endif
}

inline bool isInMainFile(
    clang::SourceManager const & manager, clang::SourceLocation Loc)
{
#if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3
    return manager.isInMainFile(Loc);
#else
    return manager.isFromMainFile(Loc);
#endif
}

inline unsigned getCustomDiagID(
    clang::DiagnosticsEngine & engine, clang::DiagnosticsEngine::Level L,
    llvm::StringRef FormatString)
{
#if (__clang_major__ == 3 && __clang_minor__ >= 5) || __clang_major__ > 3
    return engine.getDiagnosticIDs()->getCustomDiagID(
        static_cast<clang::DiagnosticIDs::Level>(L), FormatString);
#else
    return engine.getCustomDiagID(L, FormatString);
#endif
}

inline std::unique_ptr<llvm::raw_fd_ostream> create_raw_fd_ostream(
    char const * Filename, std::string & ErrorInfo)
{
#if (__clang_major__ == 3 && __clang_minor__ >= 5) || __clang_major__ > 3
    return std::unique_ptr<llvm::raw_fd_ostream>(
        new llvm::raw_fd_ostream(Filename, ErrorInfo, llvm::sys::fs::F_None));
#else
    return std::unique_ptr<llvm::raw_fd_ostream>(
        new llvm::raw_fd_ostream(Filename, ErrorInfo));
#endif
}

inline clang::NamedDecl * const * begin(
    clang::DeclContextLookupConstResult const & result)
{
#if (__clang_major__ == 3 && __clang_minor__ >= 3) || __clang_major__ > 3
    return result.begin();
#else
    return result.first;
#endif
}

inline clang::NamedDecl * const * end(
    clang::DeclContextLookupConstResult const & result)
{
#if (__clang_major__ == 3 && __clang_minor__ >= 3) || __clang_major__ > 3
    return result.end();
#else
    return result.second;
#endif
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
