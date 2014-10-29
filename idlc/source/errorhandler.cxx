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

#include <idlc/errorhandler.hxx>
#include <idlc/astinterface.hxx>

static const sal_Char* errorCodeToMessage(ErrorCode eCode)
{
    switch (eCode)
    {
    case EIDL_NONE:
        return "all is fine ";
    case EIDL_SYNTAX_ERROR:
        return "";
    case EIDL_REDEF:
        return "illegal redefinition ";
    case EIDL_REDEF_SCOPE:
        return "illegal redefinition in scope ";
    case EIDL_DEF_USE:
        return "redefinition after use, ";
    case EIDL_COERCION_FAILURE:
        return "coercion failure ";
    case EIDL_SCOPE_CONFLICT:
        return "definition scope is different than fwd declare scope, ";
    case EIDL_ILLEGAL_ADD:
        return "illegal add operation, ";
    case EIDL_ILLEGAL_USE:
        return "illegal type used in expression, ";
    case EIDL_ILLEGAL_RAISES:
        return "non-exception type in raises(..) clause, ";
    case EIDL_CANT_INHERIT:
        return "cannot inherit ";
    case EIDL_LOOKUP_ERROR:
        return "error in lookup of symbol: ";
    case EIDL_INHERIT_FWD_ERROR:
        return "";
    case EIDL_CONSTANT_EXPECTED:
        return "constant expected: ";
    case EIDL_NAME_CASE_ERROR:
        return "identifier used with two differing spellings: ";
    case EIDL_EVAL_ERROR:
        return "expression evaluation error: ";
    case EIDL_AMBIGUOUS:
        return "ambiguous definition: ";
    case EIDL_DECL_NOT_DEFINED:
        return "forward declared but never defined: ";
    case EIDL_FWD_DECL_LOOKUP:
        return "";
    case EIDL_RECURSIVE_TYPE:
        return "illegal recursive use of type: ";
    case EIDL_NOT_A_TYPE:
        return "specified symbol is not a type: ";
    case EIDL_TYPE_NOT_VALID:
        return "specified type is not valid in this context: ";
    case EIDL_INTERFACEMEMBER_LOOKUP:
        return "error in lookup of symbol, expected interface is not defined and no forward exists: ";
    case EIDL_SERVICEMEMBER_LOOKUP:
        return "error in lookup of symbol, expected service is not defined: ";
    case EIDL_TYPE_IDENT_CONFLICT:
        return "type and parameter/member name are equal: ";
    case EIDL_WRONGATTRIBUTEFLAG:
        return "the used flag is not valid in this context: ";
    case EIDL_DEFINED_ATTRIBUTEFLAG:
        return "flag is already set: ";
    case EIDL_WRONGATTRIBUTEKEYWORD:
        return "keyword not allowed: ";
    case EIDL_MISSINGATTRIBUTEKEYWORD:
        return "missing keyword: ";
    case EIDL_BAD_ATTRIBUTE_FLAGS:
        return
            "the 'attribute' flag is mandatory, and only the 'bound' and"
            " 'readonly' optional flags are accepted: ";
    case EIDL_OPTIONALEXPECTED:
        return "only the 'optional' flag is accepted: ";
    case EIDL_MIXED_INHERITANCE:
        return "interface inheritance declarations cannot appear in both an"
            " interface's header and its body";
    case EIDL_DOUBLE_INHERITANCE:
        return
            "interface is (directly or indirectly) inherited more than once: ";
    case EIDL_DOUBLE_MEMBER:
        return
            "member is (directly or indirectly) declared more than once: ";
    case EIDL_CONSTRUCTOR_PARAMETER_NOT_IN:
        return
            "a service constructor parameter may not be an out or inout"
            " parameter";
    case EIDL_CONSTRUCTOR_REST_PARAMETER_NOT_FIRST:
        return
            "no parameters may precede a rest parameter in a service"
            " constructor";
    case EIDL_REST_PARAMETER_NOT_LAST:
        return "no parameters may follow a rest parameter";
    case EIDL_REST_PARAMETER_NOT_ANY:
        return "a rest parameter must be of type any";
    case EIDL_METHOD_HAS_REST_PARAMETER:
        return "a rest parameter may not be used on an interface method";
    case EIDL_READONLY_ATTRIBUTE_SET_EXCEPTIONS:
        return "a readonly attribute may not have a setter raises clause";
    case EIDL_UNSIGNED_TYPE_ARGUMENT:
        return "an unsigned type cannot be used as a type argument";
    case EIDL_WRONG_NUMBER_OF_TYPE_ARGUMENTS:
        return
            "the number of given type arguments does not match the expected"
            " number of type parameters";
    case EIDL_INSTANTIATED_STRUCT_TYPE_TYPEDEF:
        return
            "an instantiated polymorphic struct type cannot be used in a"
            " typedef";
    case EIDL_IDENTICAL_TYPE_PARAMETERS:
        return "two type parameters have the same name";
    case EIDL_STRUCT_TYPE_TEMPLATE_WITH_BASE:
        return "a polymorphic struct type template may not have a base type";
    case EIDL_PUBLISHED_FORWARD:
        return
            "a published forward declaration of an interface type cannot be"
            " followed by an unpublished declaration of that type";
    case EIDL_PUBLISHED_USES_UNPUBLISHED:
        return
            "an unpublished entity cannot be used in the declaration of a"
            " published entity: ";
    case EIDL_SIMILAR_CONSTRUCTORS:
        return "two constructors have identical lists of parameter types";
    }
    return "unknown error";
}

static const sal_Char* warningCodeToMessage(WarningCode wCode)
{
    switch (wCode)
    {
    case WIDL_EXPID_CONFLICT:
        return "exception id conflict: ";
    case WIDL_REQID_CONFLICT:
        return "request id conflict: ";
    case WIDL_INHERIT_IDCONFLICT:
        return "request id conflict in inheritance tree: ";
    case WIDL_TYPE_IDENT_CONFLICT:
        return "type and parameter|member name are equal: ";
    case WIDL_WRONG_NAMING_CONV:
        return "type or identifier doesn't fulfill the UNO naming convention: ";
    }
    return "unknown warning";
}

static const sal_Char* parseStateToMessage(ParseState state)
{
    switch (state)
    {
    case PS_NoState:
        return "Statement can not be parsed";
    case PS_TypeDeclSeen:
        return "Malformed type declaration";
    case PS_ConstantDeclSeen:
        return "Malformed const declaration";
    case PS_ExceptionDeclSeen:
        return "Malformed exception declaration";
    case PS_InterfaceDeclSeen:
        return "Malformed interface declaration";
    case PS_ServiceDeclSeen:
        return "Malformed service declaration";
    case PS_ModuleDeclSeen:
        return "Malformed module declaration";
    case PS_AttributeDeclSeen:
        return "Malformed attribute declaration";
    case PS_PropertyDeclSeen:
        return "Malformed property declaration";
    case PS_OperationDeclSeen:
        return "Malformed operation declaration";
    case PS_InterfaceInheritanceDeclSeen:
        return "Malformed interface inheritance declaration";
    case PS_ConstantsDeclSeen:
        return "Malformed constants declaration";
    case PS_ServiceSeen:
        return "Missing service identifier following SERVICE keyword";
    case PS_ServiceIDSeen:
        return "Missing '{' or illegal syntax following service identifier";
    case PS_ServiceSqSeen:
        return "Illegal syntax following service '{' opener";
    case PS_ServiceBodySeen:
        return "Illegal syntax following service '}' closer";
    case PS_ServiceMemberSeen:
        return "Illegal syntax following service member declaration";
    case PS_ServiceIFHeadSeen:
        return "Illegal syntax following header of an interface member";
    case PS_ServiceSHeadSeen:
        return "Illegal syntax following header of an service member";
    case PS_ModuleSeen:
        return "Missing module identifier following MODULE keyword";
    case PS_ModuleIDSeen:
        return "Missing '{' or illegal syntax following module identifier";
    case PS_ModuleSqSeen:
        return "Illegal syntax following module '{' opener";
    case PS_ModuleQsSeen:
        return "Illegal syntax following module '}' closer";
    case PS_ModuleBodySeen:
        return "Illegal syntax following module export(s)";
    case PS_ConstantsSeen:
        return "Missing constants identifier following CONSTANTS keyword";
    case PS_ConstantsIDSeen:
        return "Missing '{' or illegal syntax following constants identifier";
    case PS_ConstantsSqSeen:
        return "Illegal syntax following module '{' opener";
    case PS_ConstantsQsSeen:
        return "Illegal syntax following module '}' closer";
    case PS_ConstantsBodySeen:
        return "Illegal syntax following constants export(s)";
    case PS_InterfaceSeen:
        return "Missing interface identifier following INTERFACE keyword";
    case PS_InterfaceIDSeen:
        return "Illegal syntax following interface identifier";
    case PS_InterfaceHeadSeen:
        return "Illegal syntax following interface head";
    case PS_InheritSpecSeen:
        return "Missing '{' or illegal syntax following inheritance spec";
    case PS_ForwardDeclSeen:
        return "Missing ';' following forward interface declaration";
    case PS_InterfaceSqSeen:
        return "Illegal syntax following interface '{' opener";
    case PS_InterfaceQsSeen:
        return "Illegal syntax following interface '}' closer";
    case PS_InterfaceBodySeen:
        return "Illegal syntax following interface export(s)";
    case PS_InheritColonSeen:
        return "Illegal syntax following ':' starting inheritance list";
    case PS_SNListCommaSeen:
        return "Found illegal scoped name in scoped name list";
    case PS_ScopedNameSeen:
        return "Missing ',' following scoped name in scoped name list";
    case PS_SN_IDSeen:
        return "Illegal component in scoped name";
    case PS_ScopeDelimSeen:
        return "Illegal component in scoped name following '::'";
    case PS_ConstSeen:
        return "Missing type or illegal syntax following CONST keyword";
    case PS_ConstTypeSeen:
        return "Missing identifier or illegal syntax following const type";
    case PS_ConstIDSeen:
        return "Missing '=' or illegal syntax after const identifier";
    case PS_ConstAssignSeen:
        return "Missing value expr or illegal syntax following '='";
    case PS_ConstExprSeen:
        return "Missing ';' or illegal syntax following value expr in const";
    case PS_TypedefSeen:
        return "Missing type or illegal syntax following TYPEDEF keyword";
    case PS_TypeSpecSeen:
        return "Missing declarators or illegal syntax following type spec";
    case PS_DeclaratorsSeen:
        return "Illegal syntax following declarators in TYPEDEF declaration";
    case PS_StructSeen:
        return "Missing struct identifier following STRUCT keyword";
    case PS_StructHeaderSeen:
        return "Missing '{' or illegal syntax following struct inheritance spec";
    case PS_StructIDSeen:
        return "Missing '{' or illegal syntax following struct identifier";
    case PS_StructSqSeen:
        return "Illegal syntax following struct '{' opener";
    case PS_StructQsSeen:
        return "Illegal syntax following struct '}' closer";
    case PS_StructBodySeen:
        return "Illegal syntax following struct member(s)";
    case PS_MemberTypeSeen:
        return "Illegal syntax or missing identifier following member type";
    case PS_MemberDeclsSeen:
        return "Illegal syntax following member declarator(s)";
    case PS_MemberDeclsCompleted:
        return "Missing ',' between member decls of same type(?)";
    case PS_EnumSeen:
        return "Illegal syntax or missing identifier following ENUM keyword";
    case PS_EnumIDSeen:
        return "Illegal syntax or missing '{' following enum identifier";
    case PS_EnumSqSeen:
        return "Illegal syntax following enum '{' opener";
    case PS_EnumQsSeen:
        return "Illegal syntax following enum '}' closer";
    case PS_EnumBodySeen:
        return "Illegal syntax following enum enumerator(s)";
    case PS_EnumCommaSeen:
        return "Illegal syntax or missing identifier following ',' in enum";
    case PS_SequenceSeen:
        return "Illegal syntax or missing '<' following SEQUENCE keyword";
    case PS_SequenceSqSeen:
        return "Illegal syntax or missing type following '<' in sequence";
    case PS_SequenceQsSeen:
        return "Illegal syntax following '>' in sequence";
    case PS_SequenceTypeSeen:
        return "Illegal syntax following sequence type declaration";
    case PS_FlagHeaderSeen:
        return "Illegal syntax after flags";
    case PS_AttrSeen:
        return "Illegal syntax after ATTRIBUTE keyword";
    case PS_AttrTypeSeen:
        return "Illegal syntax after type in attribute declaration";
    case PS_AttrCompleted:
        return "Illegal syntax after attribute declaration";
    case PS_ReadOnlySeen:
        return "Illegal syntax after READONLY keyword";
    case PS_OptionalSeen:
        return "Illegal syntax after OPTIONAL keyword";
    case PS_MayBeVoidSeen:
        return "Illegal syntax after MAYBEVOID keyword";
    case PS_BoundSeen:
        return "Illegal syntax after BOUND keyword";
    case PS_ConstrainedSeen:
        return "Illegal syntax after CONSTRAINED keyword";
    case PS_TransientSeen:
        return "Illegal syntax after TRANSIENT keyword";
    case PS_MayBeAmbigiousSeen:
        return "Illegal syntax after MAYBEAMBIGIOUS keyword";
    case PS_MayBeDefaultSeen:
        return "Illegal syntax after MAYBEDEFAULT keyword";
    case PS_RemoveableSeen:
        return "Illegal syntax after REMOVABLE keyword";
    case PS_PropertySeen:
        return "Illegal syntax after PROPERTY keyword";
    case PS_PropertyTypeSeen:
        return "Illegal syntax after type in property declaration";
    case PS_PropertyCompleted:
        return "Illegal syntax after property declaration";
    case PS_ExceptSeen:
        return "Illegal syntax or missing identifier after EXCEPTION keyword";
    case PS_ExceptHeaderSeen:
        return "Missing '{' or illegal syntax following exception inheritance spec";
    case PS_ExceptIDSeen:
        return "Illegal syntax or missing '{' after exception identifier";
    case PS_ExceptSqSeen:
        return "Illegal syntax after exception '{' opener";
    case PS_ExceptQsSeen:
        return "Illegal syntax after exception '}' closer";
    case PS_ExceptBodySeen:
        return "Illegal syntax after exception member(s)";
    case PS_OpTypeSeen:
        return "Illegal syntax or missing identifier after operation type";
    case PS_OpIDSeen:
        return "Illegal syntax or missing '(' after operation identifier";
    case PS_OpParsCompleted:
        return "Illegal syntax after operation parameter list";
    case PS_OpCompleted:
        return "Illegal syntax after operation declaration";
    case PS_OpSqSeen:
        return "Illegal syntax after operation parameter list '(' opener";
    case PS_OpQsSeen:
        return "Illegal syntax after operation parameter list ')' closer";
    case PS_OpParCommaSeen:
        return "Illegal syntax or missing direction in parameter declaration";
    case PS_OpParDirSeen:
        return "Illegal syntax or missing type in parameter declaration";
    case PS_OpParTypeSeen:
        return "Illegal syntax or missing declarator in parameter declaration";
    case PS_OpParDeclSeen:
        return "Illegal syntax following parameter declarator";
    case PS_RaiseSeen:
        return "Illegal syntax or missing '(' after RAISES keyword";
    case PS_RaiseSqSeen:
        return "Illegal syntax after RAISES '(' opener";
    case PS_RaiseQsSeen:
        return "Illegal syntax after RAISES ')' closer";
    case PS_DeclsCommaSeen:
        return "Illegal syntax after ',' in declarators list";
    case PS_DeclsDeclSeen:
        return "Illegal syntax after declarator in declarators list";
    default:
        return "no wider described syntax error";
    }
}

static OString flagToString(sal_uInt32 flag)
{
    OString flagStr;
    if ( (flag & AF_READONLY) == AF_READONLY )
        flagStr += "'readonly'";
    if ( (flag & AF_OPTIONAL) == AF_OPTIONAL )
        flagStr += "'optional'";
    if ( (flag & AF_MAYBEVOID) == AF_MAYBEVOID )
        flagStr += "'maybevoid'";
    if ( (flag & AF_BOUND) == AF_BOUND )
        flagStr += "'bound'";
    if ( (flag & AF_CONSTRAINED) == AF_CONSTRAINED )
        flagStr += "'constrained'";
    if ( (flag & AF_TRANSIENT) == AF_TRANSIENT )
            flagStr += "'transient'";
    if ( (flag & AF_MAYBEAMBIGUOUS) == AF_MAYBEAMBIGUOUS )
        flagStr += "'maybeambiguous'";
    if ( (flag & AF_MAYBEDEFAULT) == AF_MAYBEDEFAULT )
        flagStr += "'maybedefault'";
    if ( (flag & AF_REMOVABLE) == AF_REMOVABLE )
        flagStr += "'removable'";
    if ( (flag & AF_ATTRIBUTE) == AF_ATTRIBUTE )
        flagStr += "'attribute'";
    if ( (flag & AF_PROPERTY) == AF_PROPERTY )
        flagStr += "'property'";
    if ( flagStr.isEmpty() )
        flagStr += "'unknown'";

    return flagStr;
}

static void errorHeader(ErrorCode eCode, sal_Int32 lineNumber, sal_uInt32 start, sal_uInt32 end)
{
    OString file;
    if ( idlc()->getFileName() == idlc()->getRealFileName() )
        file = idlc()->getMainFileName();
    else
        file = idlc()->getFileName();

    fprintf(stderr, "%s:%lu [%lu:%lu] : %s", file.getStr(),
            sal::static_int_cast< unsigned long >(lineNumber),
            sal::static_int_cast< unsigned long >(start),
            sal::static_int_cast< unsigned long >(end),
            errorCodeToMessage(eCode));
}

static void errorHeader(ErrorCode eCode, sal_uInt32 lineNumber)
{
    errorHeader(eCode, lineNumber,
            idlc()->getOffsetStart(), idlc()->getOffsetEnd());
}

static void errorHeader(ErrorCode eCode)
{
    errorHeader(eCode, idlc()->getLineNumber(),
            idlc()->getOffsetStart(), idlc()->getOffsetEnd());
}

static void warningHeader(WarningCode wCode)
{
    OString file;
    if ( idlc()->getFileName() == idlc()->getRealFileName() )
        file = idlc()->getMainFileName();
    else
        file = idlc()->getFileName();

    fprintf(stderr, "%s(%lu) : WARNING, %s", file.getStr(),
            sal::static_int_cast< unsigned long >(idlc()->getLineNumber()),
            warningCodeToMessage(wCode));
}

void ErrorHandler::error0(ErrorCode e)
{
    errorHeader(e);
    fprintf(stderr, "\n");
    idlc()->incErrorCount();
}

void ErrorHandler::error1(ErrorCode e, AstDeclaration const * d)
{
    errorHeader(e);
    fprintf(stderr, "'%s'\n", d->getScopedName().getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::error2(
    ErrorCode e, AstDeclaration const * d1, AstDeclaration const * d2)
{
    errorHeader(e);
    fprintf(stderr, "'%s', '%s'\n", d1->getScopedName().getStr(),
            d2->getScopedName().getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::error3(ErrorCode e, AstDeclaration* d1, AstDeclaration* d2, AstDeclaration* d3)
{
    errorHeader(e);
    fprintf(stderr, "'%s', '%s', '%s'\n", d1->getScopedName().getStr(),
            d2->getScopedName().getStr(), d3->getScopedName().getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::warning0(WarningCode w, const sal_Char* warningmsg)
{
    if ( idlc()->getOptions()->isValid("-w") || idlc()->getOptions()->isValid("-we") ) {
        warningHeader(w);
        fprintf(stderr, "%s\n", warningmsg);
    }

    if ( idlc()->getOptions()->isValid("-we") )
        idlc()->incErrorCount();
    else
        idlc()->incWarningCount();
}

void ErrorHandler::syntaxError(ParseState ps, sal_Int32 lineNumber, const sal_Char* errmsg)
{
    errorHeader(EIDL_SYNTAX_ERROR, lineNumber);
    fprintf(stderr, "%s: %s\n", parseStateToMessage(ps), errmsg);
    idlc()->incErrorCount();
}

void ErrorHandler::coercionError(AstExpression *pExpr, ExprType et)
{
    errorHeader(EIDL_COERCION_FAILURE);
    fprintf(stderr, "'%s' to '%s'\n", pExpr->toString().getStr(),
            exprTypeToString(et));
    idlc()->incErrorCount();
}

void ErrorHandler::lookupError(const OString& n)
{
    errorHeader(EIDL_LOOKUP_ERROR);
    fprintf(stderr, "'%s'\n", n.getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::lookupError(ErrorCode e, const OString& n, AstDeclaration* pScope)
{
    errorHeader(e);
    fprintf(stderr, "'%s' in '%s'\n", n.getStr(), pScope->getFullName().getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::flagError(ErrorCode e, sal_uInt32 flag)
{
    errorHeader(e);
    fprintf(stderr, "'%s'\n", flagToString(flag).getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::noTypeError(AstDeclaration const * pDecl)
{
    errorHeader(EIDL_NOT_A_TYPE);
    fprintf(stderr, "'%s'\n", pDecl->getScopedName().getStr());
    idlc()->incErrorCount();
}

namespace {

char const * nodeTypeName(NodeType nodeType) {
    switch (nodeType) {
    case NT_interface:
        return "interface";

    case NT_exception:
        return "exception";

    case NT_struct:
        return "struct";

    default:
        return "";
    }
}

}

void ErrorHandler::inheritanceError(NodeType nodeType, const OString* name, AstDeclaration* pDecl)
{
    if ( nodeType == NT_interface &&
         (pDecl->getNodeType() == NT_interface) &&
         !(static_cast<AstInterface*>(pDecl)->isDefined()) )
    {
        errorHeader(EIDL_INHERIT_FWD_ERROR);
        fprintf(stderr, "interface '%s' cannot inherit from forward declared interface '%s'\n",
                name->getStr(), pDecl->getScopedName().getStr());
    } else
    {
        errorHeader(EIDL_CANT_INHERIT);
        fprintf(stderr, "%s '%s' from '%s'\n",
                nodeTypeName(nodeType), name->getStr(),
                pDecl->getScopedName().getStr());
    }
    idlc()->incErrorCount();
}

void ErrorHandler::forwardLookupError(const AstDeclaration* pForward,
                                      const OString& name)
{
    errorHeader(EIDL_FWD_DECL_LOOKUP);
    fprintf(stderr, "trying to look up '%s' in undefined forward declared interface '%s'\n",
            pForward->getScopedName().getStr(), name.getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::constantExpected(AstDeclaration* pDecl,
                                    const OString& name)
{
    errorHeader(EIDL_CONSTANT_EXPECTED);
    fprintf(stderr, "'%s' is bound to '%s'\n", name.getStr(), pDecl->getScopedName().getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::evalError(AstExpression* pExpr)
{
    errorHeader(EIDL_EVAL_ERROR);
    fprintf(stderr, "'%s'\n", pExpr->toString().getStr());
    idlc()->incErrorCount();
}

bool ErrorHandler::checkPublished(AstDeclaration const * decl, bool bOptional) {
    if (idlc()->isPublished() && !decl->isPublished() && !bOptional) {
        error1(EIDL_PUBLISHED_USES_UNPUBLISHED, decl);
        return false;
    } else {
        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
