/*************************************************************************
 *
 *  $RCSfile: errorhandler.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:30:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _IDLC_ERRORHANDLER_HXX_
#include <idlc/errorhandler.hxx>
#endif
#ifndef _IDLC_ASTINTERFACE_HXX_
#include <idlc/astinterface.hxx>
#endif

using namespace ::rtl;

static sal_Char* errorCodeToMessage(ErrorCode eCode)
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
    case EIDL_MULTIPLE_BRANCH:
        return "union with duplicate branch label ";
    case EIDL_COERCION_FAILURE:
        return "coercion failure ";
    case EIDL_SCOPE_CONFLICT:
        return "definition scope is different than fwd declare scope, ";
    case EIDL_ONEWAY_CONFLICT:
        return "oneway operation with OUT|INOUT parameters or raises exceptions, ";
    case EIDL_DISC_TYPE:
        return "union with illegal discriminator type, ";
    case EIDL_LABEL_TYPE:
        return "label type incompatible with union discriminator type, ";
    case EIDL_ILLEGAL_ADD:
        return "illegal add operation, ";
    case EIDL_ILLEGAL_USE:
        return "illegal type used in expression, ";
    case EIDL_ILLEGAL_RAISES:
        return "error in raises(..) clause, ";
    case EIDL_CANT_INHERIT:
        return "cannot inherit from ";
    case EIDL_LOOKUP_ERROR:
        return "error in lookup of symbol: ";
    case EIDL_INHERIT_FWD_ERROR:
        return "";
    case EIDL_CONSTANT_EXPECTED:
        return "constant expected: ";
    case EIDL_NAME_CASE_ERROR:
        return "identifier used with two differing spellings: ";
    case EIDL_ENUM_VAL_EXPECTED:
        return "enumerator expected: ";
    case EIDL_ENUM_VAL_NOT_FOUND:
        return "enumerator by this name not defined: ";
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
    case EIDL_NONVOID_ONEWAY:
        return "non-void return type in oneway operation: ";
    case EIDL_NOT_A_TYPE:
        return "specified symbol is not a type: ";
    case EIDL_TYPE_NOT_VALID:
        return "specified type is not valid in this context: ";
    case EIDL_INTERFACEMEMBER_LOOKUP:
        return "error in lookup of symbol, expected interface is not defined and no forward exists: ";
    case EIDL_SERVICEMEMBER_LOOKUP:
        return "error in lookup of symbol, expected service is not defined: ";
    case EIDL_MULTIBLE_INHERITANCE:
        return "multible inheritance is not allowed, in inheritance tree: ";
    case EIDL_TYPE_IDENT_CONFLICT:
        return "type and parameter/member name are equal: ";
    case EIDL_ONEWAY_RAISE_CONFLICT:
        return "oneway operation cannot raises exceptions: ";
    case EIDL_WRONGATTRIBUTEFLAG:
        return "the used flag is not valid in this context: ";
    case EIDL_DEFINED_ATTRIBUTEFLAG:
        return "flag is already set: ";
    case EIDL_WRONGATTRIBUTEKEYWORD:
        return "keyword not allowed: ";
    case EIDL_MISSINGATTRIBUTEKEYWORD:
        return "missing keyword: ";
    case EIDL_ATTRIBUTEREADONLYEXPECTED:
        return "only the 'attribute'|'readonly' flag is accepted: ";
    case EIDL_OPTIONALEXPECTED:
        return "only the 'optional' flag is accepted: ";
    }
    return "unknown errror";
}

static sal_Char* warningCodeToMessage(WarningCode wCode)
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
    }
    return "unkown warning";
}

static sal_Char* parseStateToMessage(ParseState state)
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
        return "Malformed servicve declaration";
    case PS_ModuleDeclSeen:
        return "Malformed module declaration";
    case PS_AttributeDeclSeen:
        return "Malformed attribute declaration";
    case PS_PropertyDeclSeen:
        return "Malformed property declaration";
    case PS_OperationDeclSeen:
        return "Malformed operation declaration";
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
    case PS_UnionSeen:
        return "Missing identifier following UNION keyword";
    case PS_UnionIDSeen:
        return "Illegal syntax following union identifier";
    case PS_SwitchSeen:
        return "Illegal syntax following SWITCH keyword";
    case PS_SwitchOpenParSeen:
        return "Illegal syntax following '(' in switch in union";
    case PS_SwitchTypeSeen:
        return "Illegal syntax following type decl in switch in union";
    case PS_SwitchCloseParSeen:
        return "Missing union '{' opener";
    case PS_UnionSqSeen:
        return "Illegal syntax following union '{' opener";
    case PS_UnionQsSeen:
        return "Illegal syntax following union '}' closer";
    case PS_DefaultSeen:
        return "Illegal syntax or missing ':' following DEFAULT keyword";
    case PS_UnionLabelSeen:
        return "Illegal syntax following branch label in union";
    case PS_LabelColonSeen:
        return "Illegal syntax following ':' in branch label in union";
    case PS_LabelExprSeen:
        return "Illegal syntax following label expression in union";
    case PS_UnionElemSeen:
    case PS_UnionElemCompleted:
        return "Illegal syntax following union element";
    case PS_CaseSeen:
        return "Illegal syntax following CASE keyword in union";
    case PS_UnionElemTypeSeen:
        return "Illegal syntax following type decl in union element";
    case PS_UnionElemDeclSeen:
        return "Illegal syntax following declarator in union element";
    case PS_UnionBodySeen:
        return "Illegal syntax following union body statement(s)";
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
    case PS_ArrayIDSeen:
        return "Illegal syntax or missing dimensions after array identifier";
    case PS_ArrayCompleted:
        return "Illegal syntax after array declaration";
    case PS_DimSqSeen:
        return "Illegal syntax or missing size expr after '[' in array declaration";
    case PS_DimQsSeen:
        return "Illegal syntax after ']' in array declaration";
    case PS_DimExprSeen:
        return "Illegal syntax or missing ']' after size expr in array declaration";
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
        return "Illegal syntax after REMOVEABLE keyword";
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
    case PS_OpHeadSeen:
        return "Illegasl syntax after operation header";
    case PS_OpTypeSeen:
        return "Illegal syntax or missing identifier after operation type";
    case PS_OpIDSeen:
        return "Illegal syntax or missing '(' after operation identifier";
    case PS_OpParsCompleted:
        return "Illegal syntax after operation parameter list";
    case PS_OpRaiseCompleted:
        return "Illegal syntax after optional RAISES in operation declaration";
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
    case PS_OpRaiseSeen:
        return "Illegal syntax or missing '(' after RAISES keyword";
    case PS_OpRaiseSqSeen:
        return "Illegal syntax after RAISES '(' opener";
    case PS_OpRaiseQsSeen:
        return "Illegal syntax after RAISES ')' closer";
    case PS_OpOnewaySeen:
        return "Illegal syntax after ONEWAY keyword";
    case PS_DeclsCommaSeen:
        return "Illegal syntax after ',' in declarators list";
    case PS_DeclsDeclSeen:
        return "Illegal syntax after declarator in declarators list";
    }
    return "no wider described syntax error";
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
    if ( (flag & AF_REMOVEABLE) == AF_REMOVEABLE )
        flagStr += "'removeable'";
    if ( (flag & AF_ATTRIBUTE) == AF_ATTRIBUTE )
        flagStr += "'attribute'";
    if ( (flag & AF_PROPERTY) == AF_PROPERTY )
        flagStr += "'property'";
    if ( !flagStr.getLength() )
        flagStr += "'unknown'";

    return flagStr;
}

static void errorHeader(ErrorCode eCode, sal_Int32 lineNumber)
{
    OString file;
    if ( idlc()->getFileName() == idlc()->getRealFileName() )
        file = idlc()->getMainFileName();
    else
        file = idlc()->getFileName();

    fprintf(stderr, "%s(%d) : %s", file.getStr(), lineNumber,
            errorCodeToMessage(eCode));
}

static void errorHeader(ErrorCode eCode)
{
    errorHeader(eCode, idlc()->getLineNumber());
}

static void warningHeader(WarningCode wCode)
{
    OString file;
    if ( idlc()->getFileName() == idlc()->getRealFileName() )
        file = idlc()->getMainFileName();
    else
        file = idlc()->getFileName();

    fprintf(stderr, "%s(%d) : WARNING, %s", file.getStr(), idlc()->getLineNumber(),
            warningCodeToMessage(wCode));
}

void ErrorHandler::error0(ErrorCode e)
{
    errorHeader(e);
    fprintf(stderr, "\n");
    idlc()->incErrorCount();
}

void ErrorHandler::error1(ErrorCode e, AstDeclaration* d)
{
    errorHeader(e);
    fprintf(stderr, "'%s'\n", d->getScopedName().getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::error2(ErrorCode e, AstDeclaration* d1, AstDeclaration* d2)
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

void ErrorHandler::warning1(WarningCode w, AstDeclaration* d)
{
    warningHeader(w);
    fprintf(stderr, "'%s'\n", d->getScopedName().getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::warning2(WarningCode w, AstDeclaration* d1, AstDeclaration* d2)
{
    warningHeader(w);
    fprintf(stderr, "'%s', '%s'\n", d1->getScopedName().getStr(),
            d2->getScopedName().getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::syntaxError(ParseState ps, sal_Int32 lineNumber, sal_Char* errmsg)
{
    errorHeader(EIDL_SYNTAX_ERROR, lineNumber);
    fprintf(stderr, "%s%s\n", parseStateToMessage(ps), errmsg + 11);
    idlc()->incErrorCount();
}

void ErrorHandler::nameCaseError(sal_Char *n, sal_Char *t)
{
    idlc()->incErrorCount();
}

void ErrorHandler::coercionError(AstExpression *pExpr, ExprType et)
{
    errorHeader(EIDL_COERCION_FAILURE);
    fprintf(stderr, "'%s' to '%s'\n", pExpr->toString().getStr(),
            exprTypeToString(et));
    idlc()->incErrorCount();
}

void ErrorHandler::lookupError(const ::rtl::OString& n)
{
    errorHeader(EIDL_LOOKUP_ERROR);
    fprintf(stderr, "'%s'\n", n.getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::lookupError(ErrorCode e, const ::rtl::OString& n, AstDeclaration* pScope)
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

void ErrorHandler::noTypeError(AstDeclaration* pDecl)
{
    errorHeader(EIDL_NOT_A_TYPE);
    fprintf(stderr, "'%s'\n", pDecl->getScopedName().getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::inheritanceError(OString* name, AstDeclaration* pDecl)
{
    if ( (pDecl->getNodeType() == NT_interface) &&
         !((AstInterface*)pDecl)->isDefined() )
    {
        errorHeader(EIDL_INHERIT_FWD_ERROR);
        fprintf(stderr, "interface '%s' cannot inherit from forward declared interface '%s'\n",
                name->getStr(), pDecl->getScopedName().getStr());
    } else
    {
        errorHeader(EIDL_CANT_INHERIT);
        fprintf(stderr, "interface '%s' attempts to inherit from '%s'\n",
                name->getStr(), pDecl->getScopedName().getStr());
    }
    idlc()->incErrorCount();
}

void ErrorHandler::forwardLookupError(AstDeclaration* pForward,
                                      const ::rtl::OString& name)
{
    errorHeader(EIDL_FWD_DECL_LOOKUP);
    fprintf(stderr, "trying to look up '%s' in undefined forward declared interface '%s'\n",
            pForward->getScopedName().getStr(), name.getStr());
    idlc()->incErrorCount();
}

void ErrorHandler::constantExpected(AstDeclaration* pDecl,
                                    const ::rtl::OString& name)
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

void ErrorHandler::enumValExpected(AstUnion* pUnion, AstUnionLabel *pLabel)
{
    errorHeader(EIDL_ENUM_VAL_EXPECTED);
    fprintf(stderr, " union %s, ", pUnion->getLocalName().getStr());
//  pLabel->dump();
    fprintf(stderr, "\n");
    idlc()->incErrorCount();
}

void ErrorHandler::enumValLookupFailure(AstUnion* pUnion, AstEnum* pEnum, const ::rtl::OString& name)
{
    errorHeader(EIDL_ENUM_VAL_NOT_FOUND);
    fprintf(stderr, " union %s, enum %s, enumerator %s\n",
            pUnion->getLocalName().getStr(),
            pEnum->getLocalName().getStr(), name.getStr());
    idlc()->incErrorCount();
}
