/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_IDLC_INC_ERRORHANDLER_HXX
#define INCLUDED_IDLC_INC_ERRORHANDLER_HXX

#include <astdeclaration.hxx>
#include <astexpression.hxx>
#include <astenum.hxx>

enum class ErrorCode
{
    SyntaxError,      // Syntax error in IDL input
                      // More details will be gleaned from examining
                      // the parse state
    RedefScope,       // Redefinition inside defining scope
    CoercionFailure,  // Coercion failure
    ScopeConflict,    // Between fwd declare and full declare
    IllegalAdd,       // Illegal add action
    IllegalRaises,    // Error in "raises" clause
    CantInherit,      // Cannot inherit from non-interface
    IdentNotFound,    // Identifier not found
    CannotInheritFromForward, // Cannot inherit from fwd decl interface
    ExpectedConstant,       // We got something else..
    Eval,                   // Error in evaluating expression
    ForwardDeclLookup,      // Tried to lookup in fwd declared intf
    RecursiveType,          // Illegal recursive use of type
    NotAType,               // Not a type
    InterfaceMemberLookup,  // interface is not defined or a fwd declaration not exists
    ServiceMemberLookup,
    DefinedAttributeFlag,
    WrongAttributeKeyword,
    MissingAttributeKeyword,
    BadAttributeFlags,
    ExpectedOptional,
    MixedInheritance,
    DoubleInheritance,
    DoubleMember,
    ConstructorParameterNotIn,
    ConstructorRestParameterNotFirst,
    RestParameterNotLast,
    RestParameterNotAny,
    MethodHasRestParameter,
    ReadOnlyAttributeSetExceptions,
    UnsignedTypeArgument,
    WrongNumberOfTypeArguments,
    InstantiatedStructTypeTypedef,
    IdenticalTypeParameters,
    StructTypeTemplateWithBase,
    PublishedForward,
    PublishedusesUnpublished,
    SimilarConstructors
};

enum class WarningCode
{
    WrongNamingConvention    // type or identifier doesn't fulfill the UNO naming convention
};

class ErrorHandler
{
public:
    // Report errors with varying numbers of arguments
    static void error0(ErrorCode e);
    static void error1(ErrorCode e, AstDeclaration const * d);
    static void error2(
        ErrorCode e, AstDeclaration const * d1, AstDeclaration const * d2);
    static void error3(ErrorCode e, AstDeclaration* d1, AstDeclaration* d2, AstDeclaration* d3);

    // Warning
    static void warning0(WarningCode e, const sal_Char* warningmsg);

    // Report a syntax error in IDL input
    static void syntaxError(ParseState state, sal_Int32 lineNumber, const sal_Char* errmsg);

    // Report an unsuccessful coercion attempt
    static void coercionError(AstExpression *pExpr, ExprType et);

    // Report a failed name lookup attempt
    static void lookupError(const OString& n);
    // Report a failed name lookup attempt
    static void lookupError(ErrorCode e, const OString& n, AstDeclaration* pScope);


    // Report a type error
    static void noTypeError(AstDeclaration const * pDecl);

    static void inheritanceError(NodeType nodeType, const OString* name, AstDeclaration* pDecl);

    static void flagError(ErrorCode e, sal_uInt32 flag);

    static void forwardLookupError(const AstDeclaration* pForward, const OString& name);

    static void constantExpected(AstDeclaration* pDecl, const OString& name);

    static void evalError(AstExpression* pExpr);

    static bool checkPublished(AstDeclaration const * decl, bool bOptional=false);
};

#endif // INCLUDED_IDLC_INC_ERRORHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
