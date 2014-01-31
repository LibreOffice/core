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
#include "clang/AST/Type.h"

// Compatibility wrapper to abstract over (trivial) chanes in the Clang API:
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

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
