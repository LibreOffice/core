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
#ifndef _IDLC_ERRORHANDLER_HXX_
#define _IDLC_ERRORHANDLER_HXX_

#include <idlc/astdeclaration.hxx>
#include <idlc/astexpression.hxx>
#include <idlc/astunion.hxx>
#include <idlc/astenum.hxx>

enum ErrorCode
{
    EIDL_NONE,              // No error
    EIDL_SYNTAX_ERROR,      // Syntax error in IDL input
                            // More details will be gleaned from examining
                            // the parse state
    EIDL_REDEF,             // Redefinition
    EIDL_REDEF_SCOPE,       // Redefinition inside defining scope
    EIDL_DEF_USE,           // Definition after use
    EIDL_MULTIPLE_BRANCH,   // More than one union branch with this label
    EIDL_COERCION_FAILURE,  // Coercion failure
    EIDL_SCOPE_CONFLICT,    // Between fwd declare and full declare
    EIDL_DISC_TYPE,         // Illegal discriminator type in union
    EIDL_LABEL_TYPE,        // Mismatch with discriminator type in union
    EIDL_ILLEGAL_ADD,       // Illegal add action
    EIDL_ILLEGAL_USE,       // Illegal type used in expression
    EIDL_ILLEGAL_RAISES,    // Error in "raises" clause
    EIDL_CANT_INHERIT,      // Cannot inherit from non-interface
    EIDL_LOOKUP_ERROR,      // Identifier not found
    EIDL_INHERIT_FWD_ERROR, // Cannot inherit from fwd decl interface
    EIDL_CONSTANT_EXPECTED, // We got something else..
    EIDL_NAME_CASE_ERROR,   // Spelling differences found
    EIDL_ENUM_VAL_EXPECTED, // Expected an enumerator
    EIDL_ENUM_VAL_NOT_FOUND,    // Didnt find an enumerator with that name
    EIDL_EVAL_ERROR,        // Error in evaluating expression
    EIDL_AMBIGUOUS,         // Ambiguous name definition
    EIDL_DECL_NOT_DEFINED,  // Forward declared but never defined
    EIDL_FWD_DECL_LOOKUP,   // Tried to lookup in fwd declared intf
    EIDL_RECURSIVE_TYPE,    // Illegal recursive use of type
    EIDL_NOT_A_TYPE,        // Not a type
    EIDL_TYPE_NOT_VALID,    // Type is not valid in this context
    EIDL_INTERFACEMEMBER_LOOKUP,    // interface is not defined or a fwd declaration not exists
    EIDL_SERVICEMEMBER_LOOKUP,
    EIDL_TYPE_IDENT_CONFLICT,   // type and identifier has equal names
    EIDL_WRONGATTRIBUTEFLAG,
    EIDL_DEFINED_ATTRIBUTEFLAG,
    EIDL_WRONGATTRIBUTEKEYWORD,
    EIDL_MISSINGATTRIBUTEKEYWORD,
    EIDL_BAD_ATTRIBUTE_FLAGS,
    EIDL_OPTIONALEXPECTED,
    EIDL_MIXED_INHERITANCE,
    EIDL_DOUBLE_INHERITANCE,
    EIDL_DOUBLE_MEMBER,
    EIDL_CONSTRUCTOR_PARAMETER_NOT_IN,
    EIDL_CONSTRUCTOR_REST_PARAMETER_NOT_FIRST,
    EIDL_REST_PARAMETER_NOT_LAST,
    EIDL_REST_PARAMETER_NOT_ANY,
    EIDL_METHOD_HAS_REST_PARAMETER,
    EIDL_READONLY_ATTRIBUTE_SET_EXCEPTIONS,
    EIDL_UNSIGNED_TYPE_ARGUMENT,
    EIDL_WRONG_NUMBER_OF_TYPE_ARGUMENTS,
    EIDL_INSTANTIATED_STRUCT_TYPE_TYPEDEF,
    EIDL_IDENTICAL_TYPE_PARAMETERS,
    EIDL_STRUCT_TYPE_TEMPLATE_WITH_BASE,
    EIDL_PUBLISHED_FORWARD,
    EIDL_PUBLISHED_USES_UNPUBLISHED,
    EIDL_SIMILAR_CONSTRUCTORS
};

enum WarningCode
{
    WIDL_EXPID_CONFLICT,        // exception id conflict
    WIDL_REQID_CONFLICT,        // request id conflict
    WIDL_INHERIT_IDCONFLICT,    // request id conflict inheritance tree
    WIDL_TYPE_IDENT_CONFLICT,   // type and identifier has equal names
    WIDL_WRONG_NAMING_CONV      // type or identifier doesn't fulfill the UNO naming convention
};

class ErrorHandler
{
public:
    // Report errors with varying numbers of arguments
    void    error0(ErrorCode e);
    void    error1(ErrorCode e, AstDeclaration const * d);
    void    error2(
        ErrorCode e, AstDeclaration const * d1, AstDeclaration const * d2);
    void    error3(ErrorCode e, AstDeclaration* d1, AstDeclaration* d2, AstDeclaration* d3);

    // Warning
    void    warning0(WarningCode e, const sal_Char* warningmsg);

    // Report a syntax error in IDL input
    void    syntaxError(ParseState state, sal_Int32 lineNumber, const sal_Char* errmsg);

    // Report an unsuccessful coercion attempt
    void    coercionError(AstExpression *pExpr, ExprType et);

    // Report a failed name lookup attempt
    void    lookupError(const OString& n);
    // Report a failed name lookup attempt
    void    lookupError(ErrorCode e, const OString& n, AstDeclaration* pScope);


    // Report a type error
    void    noTypeError(AstDeclaration const * pDecl);

    void    inheritanceError(NodeType nodeType, const OString* name, AstDeclaration* pDecl);

    void    flagError(ErrorCode e, sal_uInt32 flag);

    void    forwardLookupError(AstDeclaration* pForward, const OString& name);

    void    constantExpected(AstDeclaration* pDecl, const OString& name);

    void    evalError(AstExpression* pExpr);

    // Report a situation where an enumerator was expected but we got
    // something else instead. This occurs when a union with an enum
    // discriminator is being parsed and one of the branch labels is
    // not an enumerator in that enum
    void    enumValExpected(AstUnion* pUnion);

    // Report a failed enumerator lookup in an enum
    void    enumValLookupFailure(AstUnion* pUnion, AstEnum* pEnum, const OString& name);

    bool checkPublished(AstDeclaration const * decl, bool bOptiional=false);
};

#endif // _IDLC_ERRORHANDLER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
