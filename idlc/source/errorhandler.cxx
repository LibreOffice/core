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

#include <errorhandler.hxx>
#include <astinterface.hxx>

static const char* errorCodeToMessage(ErrorCode eCode)
{
    switch (eCode)
    {
    case ErrorCode::SyntaxError:
        return "";
    case ErrorCode::RedefScope:
        return "illegal redefinition in scope ";
    case ErrorCode::CoercionFailure:
        return "coercion failure ";
    case ErrorCode::ScopeConflict:
        return "definition scope is different than fwd declare scope, ";
    case ErrorCode::IllegalAdd:
        return "illegal add operation, ";
    case ErrorCode::IllegalRaises:
        return "non-exception type in raises(..) clause, ";
    case ErrorCode::CantInherit:
        return "cannot inherit ";
    case ErrorCode::IdentNotFound:
        return "error in lookup of symbol: ";
    case ErrorCode::CannotInheritFromForward:
        return "";
    case ErrorCode::ExpectedConstant:
        return "constant expected: ";
    case ErrorCode::Eval:
        return "expression evaluation error: ";
    case ErrorCode::ForwardDeclLookup:
        return "";
    case ErrorCode::RecursiveType:
        return "illegal recursive use of type: ";
    case ErrorCode::NotAType:
        return "specified symbol is not a type: ";
    case ErrorCode::InterfaceMemberLookup:
        return "error in lookup of symbol, expected interface is not defined and no forward exists: ";
    case ErrorCode::ServiceMemberLookup:
        return "error in lookup of symbol, expected service is not defined: ";
    case ErrorCode::DefinedAttributeFlag:
        return "flag is already set: ";
    case ErrorCode::WrongAttributeKeyword:
        return "keyword not allowed: ";
    case ErrorCode::MissingAttributeKeyword:
        return "missing keyword: ";
    case ErrorCode::BadAttributeFlags:
        return
            "the 'attribute' flag is mandatory, and only the 'bound' and"
            " 'readonly' optional flags are accepted: ";
    case ErrorCode::ExpectedOptional:
        return "only the 'optional' flag is accepted: ";
    case ErrorCode::MixedInheritance:
        return "interface inheritance declarations cannot appear in both an"
            " interface's header and its body";
    case ErrorCode::DoubleInheritance:
        return
            "interface is (directly or indirectly) inherited more than once: ";
    case ErrorCode::DoubleMember:
        return
            "member is (directly or indirectly) declared more than once: ";
    case ErrorCode::ConstructorParameterNotIn:
        return
            "a service constructor parameter may not be an out or inout"
            " parameter";
    case ErrorCode::ConstructorRestParameterNotFirst:
        return
            "no parameters may precede a rest parameter in a service"
            " constructor";
    case ErrorCode::RestParameterNotLast:
        return "no parameters may follow a rest parameter";
    case ErrorCode::RestParameterNotAny:
        return "a rest parameter must be of type any";
    case ErrorCode::MethodHasRestParameter:
        return "a rest parameter may not be used on an interface method";
    case ErrorCode::ReadOnlyAttributeSetExceptions:
        return "a readonly attribute may not have a setter raises clause";
    case ErrorCode::UnsignedTypeArgument:
        return "an unsigned type cannot be used as a type argument";
    case ErrorCode::WrongNumberOfTypeArguments:
        return
            "the number of given type arguments does not match the expected"
            " number of type parameters";
    case ErrorCode::InstantiatedStructTypeTypedef:
        return
            "an instantiated polymorphic struct type cannot be used in a"
            " typedef";
    case ErrorCode::IdenticalTypeParameters:
        return "two type parameters have the same name";
    case ErrorCode::StructTypeTemplateWithBase:
        return "a polymorphic struct type template may not have a base type";
    case ErrorCode::PublishedForward:
        return
            "a published forward declaration of an interface type cannot be"
            " followed by an unpublished declaration of that type";
    case ErrorCode::PublishedusesUnpublished:
        return
            "an unpublished entity cannot be used in the declaration of a"
            " published entity: ";
    case ErrorCode::SimilarConstructors:
        return "two constructors have identical lists of parameter types";
    }
    return "unknown error";
}

static const char* warningCodeToMessage(WarningCode wCode)
{
    switch (wCode)
    {
    case WarningCode::WrongNamingConvention:
        return "type or identifier doesn't fulfill the UNO naming convention: ";
    }
    return "unknown warning";
}

static const char* parseStateToMessage(ParseState state)
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
        return "Illegal syntax following header of a service member";
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
    case PS_MayBeAmbiguousSeen:
        return "Illegal syntax after MAYBEAMBIGUOUS keyword";
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

void ErrorHandler::error3(ErrorCode e, AstDeclaration const * d1, AstDeclaration const * d2, AstDeclaration const * d3)
{
    errorHeader(e);
    fprintf(stderr, "'%s', '%s', '%s'\n", d1->getScopedName().getStr(),
            d2->getScopedName().getStr(), d3->getScopedName().getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::warning0(WarningCode w, const char* warningmsg)
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

void ErrorHandler::syntaxError(ParseState ps, sal_Int32 lineNumber, const char* errmsg)
{
    errorHeader(ErrorCode::SyntaxError, lineNumber);
    fprintf(stderr, "%s: %s\n", parseStateToMessage(ps), errmsg);
    idlc()->incErrorCount();
}

void ErrorHandler::coercionError(AstExpression *pExpr, ExprType et)
{
    errorHeader(ErrorCode::CoercionFailure);
    fprintf(stderr, "'%s' to '%s'\n", pExpr->toString().getStr(),
            exprTypeToString(et));
    idlc()->incErrorCount();
}

void ErrorHandler::lookupError(const OString& n)
{
    errorHeader(ErrorCode::IdentNotFound);
    fprintf(stderr, "'%s'\n", n.getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::lookupError(ErrorCode e, const OString& n, AstDeclaration const * pScope)
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
    errorHeader(ErrorCode::NotAType);
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

void ErrorHandler::inheritanceError(NodeType nodeType, const OString* name, AstDeclaration const * pDecl)
{
    if ( nodeType == NT_interface &&
         (pDecl->getNodeType() == NT_interface) &&
         !(static_cast<AstInterface const *>(pDecl)->isDefined()) )
    {
        errorHeader(ErrorCode::CannotInheritFromForward);
        fprintf(stderr, "interface '%s' cannot inherit from forward declared interface '%s'\n",
                name->getStr(), pDecl->getScopedName().getStr());
    } else
    {
        errorHeader(ErrorCode::CantInherit);
        fprintf(stderr, "%s '%s' from '%s'\n",
                nodeTypeName(nodeType), name->getStr(),
                pDecl->getScopedName().getStr());
    }
    idlc()->incErrorCount();
}

void ErrorHandler::forwardLookupError(const AstDeclaration* pForward,
                                      const OString& name)
{
    errorHeader(ErrorCode::ForwardDeclLookup);
    fprintf(stderr, "trying to look up '%s' in undefined forward declared interface '%s'\n",
            pForward->getScopedName().getStr(), name.getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::constantExpected(AstDeclaration const * pDecl,
                                    const OString& name)
{
    errorHeader(ErrorCode::ExpectedConstant);
    fprintf(stderr, "'%s' is bound to '%s'\n", name.getStr(), pDecl->getScopedName().getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::evalError(AstExpression* pExpr)
{
    errorHeader(ErrorCode::Eval);
    fprintf(stderr, "'%s'\n", pExpr->toString().getStr());
    idlc()->incErrorCount();
}

bool ErrorHandler::checkPublished(AstDeclaration const * decl, bool bOptional) {
    if (idlc()->isPublished() && !decl->isPublished() && !bOptional) {
        error1(ErrorCode::PublishedusesUnpublished, decl);
        return false;
    } else {
        return true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
