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

#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "llvm/ADT/StringRef.h"

// Compatibility wrapper to abstract over (trivial) changes in the Clang API:
namespace compat {

inline clang::QualType getReturnType(clang::FunctionDecl const & decl) {
#if (__clang_major__ == 3 && __clang_minor__ >= 5) || __clang_major__ > 3
    return decl.getReturnType();
#else
    return decl.getResultType();
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

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
