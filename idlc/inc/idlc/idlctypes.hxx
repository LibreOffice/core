/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _IDLC_IDLCTYPES_HXX_
#define _IDLC_IDLCTYPES_HXX_

#include <stdio.h>

#include <hash_map>
#include <list>
#include <vector>
#include <string>
#include <set>

#include <sal/types.h>
#include <rtl/ustring.hxx>

struct EqualString
{
    sal_Bool operator()(const ::rtl::OString& str1, const ::rtl::OString& str2) const
    {
        return (str1 == str2);
    }
};

struct HashString
{
    sal_Int32 operator()(const ::rtl::OString& str) const
    {
        return str.hashCode();
    }
};

struct LessString
{
    sal_Bool operator()(const ::rtl::OString& str1, const ::rtl::OString& str2) const
    {
        return (str1 < str2);
    }
};

typedef ::std::list< ::rtl::OString >               StringList;
typedef ::std::vector< ::rtl::OString >             StringVector;
typedef ::std::set< ::rtl::OString, LessString >    StringSet;

class AstExpression;
typedef ::std::list< AstExpression* >   ExprList;

class AstUnionLabel;
typedef ::std::list< AstUnionLabel* >   LabelList;

class AstDeclaration;

typedef ::std::hash_map< ::rtl::OString, AstDeclaration*, HashString, EqualString > DeclMap;
typedef ::std::list< AstDeclaration* > DeclList;

class AstScope;
AstDeclaration* SAL_CALL scopeAsDecl(AstScope* pScope);
AstScope* SAL_CALL declAsScope(AstDeclaration* pDecl);

#ifdef _MSC_VER
#pragma warning( disable : 4541 )
#endif

// flags used for attributes, properties and services
#define AF_INVALID          0x0000
#define AF_READONLY         0x0001
#define AF_OPTIONAL         0x0002
#define AF_MAYBEVOID        0x0004
#define AF_BOUND            0x0008
#define AF_CONSTRAINED      0x0010
#define AF_TRANSIENT        0x0020
#define AF_MAYBEAMBIGUOUS   0x0040
#define AF_MAYBEDEFAULT     0x0080
#define AF_REMOVEABLE       0x0100
#define AF_ATTRIBUTE        0x0200
#define AF_PROPERTY         0x0400

enum ParseState
{
    PS_NoState,
    PS_TypeDeclSeen,        // Seen complete typedef declaration
    PS_ConstantDeclSeen,    // Seen complete const declaration
    PS_ExceptionDeclSeen,   // Seen complete exception declaration
    PS_InterfaceDeclSeen,   // Seen complete interface declaration
    PS_ServiceDeclSeen,     // Seen complete service declaration
    PS_SingletonDeclSeen,   // Seen complete singleton declaration
    PS_ModuleDeclSeen,      // Seen complete module declaration
    PS_AttributeDeclSeen,   // Seen complete attribute declaration
    PS_PropertyDeclSeen,    // Seen complete property declaration
    PS_OperationDeclSeen,   // Seen complete operation declaration
    PS_InterfaceInheritanceDeclSeen, // Seen complete interface inheritance decl
    PS_ConstantsDeclSeen,   // Seen complete constants declaration

    PS_ServiceSeen,         // Seen a SERVICE keyword
    PS_ServiceIDSeen,       // Seen the service ID
    PS_ServiceSqSeen,       // '{' seen for service
    PS_ServiceQsSeen,       // '}' seen for service
    PS_ServiceBodySeen,     // Seen complete service body
    PS_ServiceMemberSeen,   // Seen a service member
    PS_ServiceIFHeadSeen,   // Seen an interface member header
    PS_ServiceSHeadSeen,    // Seen an service member header

    PS_SingletonSeen,       // Seen a SINGLETON keyword
    PS_SingletonIDSeen,     // Seen the singleton ID
    PS_SingletonSqSeen,     // '{' seen for singleton
    PS_SingletonQsSeen,     // '}' seen for singleton
    PS_SingletonBodySeen,   // Seen complete singleton body
    PS_SingletonMemberSeen, // Seen a singleton member

    PS_ModuleSeen,          // Seen a MODULE keyword
    PS_ModuleIDSeen,        // Seen the module ID
    PS_ModuleSqSeen,        // '{' seen for module
    PS_ModuleQsSeen,        // '}' seen for module
    PS_ModuleBodySeen,      // Seen complete module body

    PS_ConstantsSeen,       // Seen a CONSTANTS keyword
    PS_ConstantsIDSeen,     // Seen the constants ID
    PS_ConstantsSqSeen,     // '{' seen for constants
    PS_ConstantsQsSeen,     // '}' seen for constants
    PS_ConstantsBodySeen,   // Seen complete constants body

    PS_InterfaceSeen,       // Seen an INTERFACE keyword
    PS_InterfaceIDSeen,     // Seen the interface ID
    PS_InterfaceHeadSeen,   // Seen the interface head
    PS_InheritSpecSeen,     // Seen a complete inheritance spec
    PS_ForwardDeclSeen,     // Forward interface decl seen
    PS_InterfaceSqSeen,     // '{' seen for interface
    PS_InterfaceQsSeen,     // '}' seen for interface
    PS_InterfaceBodySeen,   // Seen an interface body
    PS_InheritColonSeen,    // Seen ':' in inheritance list

    PS_SNListCommaSeen,     // Seen ',' in list of scoped names
    PS_ScopedNameSeen,      // Seen a complete scoped name
    PS_SN_IDSeen,           // Seen an identifier as part of a scoped name
    PS_ScopeDelimSeen,      // Seen a scope delim as party of a scoped name

    PS_ConstSeen,           // Seen a CONST keyword
    PS_ConstTypeSeen,       // Parsed the type of a constant
    PS_ConstIDSeen,         // Seen the constant ID
    PS_ConstAssignSeen,     // Seen the '='
    PS_ConstExprSeen,       // Seen the constant value expression

    PS_TypedefSeen,         // Seen a TYPEDEF keyword
    PS_TypeSpecSeen,        // Seen a complete type specification
    PS_DeclaratorsSeen,     // Seen a complete list of declarators

    PS_StructSeen,          // Seen a STRUCT keyword
    PS_StructHeaderSeen,    // Seen struct header
    PS_StructIDSeen,        // Seen the struct ID
    PS_StructSqSeen,        // '{' seen for struct
    PS_StructQsSeen,        // '}' seen for struct
    PS_StructBodySeen,      // Seen complete body of struct decl

    PS_MemberTypeSeen,      // Seen type of struct or except member
    PS_MemberDeclsSeen,     // Seen decls of struct or except members
    PS_MemberDeclsCompleted,// Completed one struct or except member to ';'

    PS_UnionSeen,           // Seen a UNION keyword
    PS_UnionIDSeen,         // Seen the union ID
    PS_SwitchSeen,          // Seen the SWITCH keyword
    PS_SwitchOpenParSeen,   // Seen the switch open par.
    PS_SwitchTypeSeen,      // Seen the switch type spec
    PS_SwitchCloseParSeen,  // Seen the switch close par.
    PS_UnionSqSeen,         // '{' seen for union
    PS_UnionQsSeen,         // '}' seen for union
    PS_DefaultSeen,         // Seen DEFAULT keyword
    PS_UnionLabelSeen,      // Seen label of union element
    PS_LabelColonSeen,      // Seen ':' of union branch label
    PS_LabelExprSeen,       // Seen expression of union branch label
    PS_UnionElemSeen,       // Seen a union element
    PS_UnionElemCompleted,  // Completed one union member up to ';'
    PS_CaseSeen,            // Seen a CASE keyword
    PS_UnionElemTypeSeen,   // Seen type spec for union element
    PS_UnionElemDeclSeen,   // Seen declarator for union element
    PS_UnionBodySeen,       // Seen completed union body

    PS_EnumSeen,            // Seen an ENUM keyword
    PS_EnumIDSeen,          // Seen the enum ID
    PS_EnumSqSeen,          // Seen '{' for enum
    PS_EnumQsSeen,          // Seen '}' for enum
    PS_EnumBodySeen,        // Seen complete enum body
    PS_EnumCommaSeen,       // Seen ',' in list of enumerators

    PS_SequenceSeen,        // Seen a SEQUENCE keyword
    PS_SequenceSqSeen,      // Seen '<' for sequence
    PS_SequenceQsSeen,      // Seen '>' for sequence
    PS_SequenceTypeSeen,    // Seen type decl for sequence

    PS_ArrayIDSeen,         // Seen array ID
    PS_ArrayTypeSeen,       // Seen array type
    PS_ArrayCompleted,      // Seen completed array declaration
    PS_DimSqSeen,           // Seen '[' for array dimension
    PS_DimQsSeen,           // Seen ']' for array dimension
    PS_DimExprSeen,         // Seen size expression for array dimension


    PS_FlagHeaderSeen,      // Seen the attribute|property|interface member head
    PS_AttrSeen,            // Seen ATTRIBUTE keyword
    PS_AttrTypeSeen,        // Seen type decl for attribute
    PS_AttrCompleted,       // Seen complete attribute declaration
    PS_ReadOnlySeen,        // Seen READONLY keyword
    PS_OptionalSeen,        // Seen OPTIONAL keyword
    PS_MayBeVoidSeen,       // Seen MAYBEVOID yword
    PS_BoundSeen,           // Seen BOUND  keyword
    PS_ConstrainedSeen,     // Seen CONSTRAINED keyword
    PS_TransientSeen,       // Seen TRANSIENT keyword
    PS_MayBeAmbigiousSeen,  // Seen MAYBEAMBIGIOUS keyword
    PS_MayBeDefaultSeen,    // Seen MAYBEDEFAULT keyword
    PS_RemoveableSeen,      // Seen REMOVEABLE keyword

    PS_PropertySeen,        // Seen PROPERTY keyword
    PS_PropertyTypeSeen,    // Seen type decl for property
    PS_PropertyCompleted,   // Seen complete property declaration

    PS_ExceptSeen,          // Seen EXCEPTION keyword
    PS_ExceptHeaderSeen,    // Seen exception header keyword
    PS_ExceptIDSeen,        // Seen exception identifier
    PS_ExceptSqSeen,        // Seen '{' for exception
    PS_ExceptQsSeen,        // Seen '}' for exception
    PS_ExceptBodySeen,      // Seen complete exception body

    PS_OpHeadSeen,          // Seen the operation head
    PS_OpTypeSeen,          // Seen operation return type
    PS_OpIDSeen,            // Seen operation ID
    PS_OpParsCompleted,     // Completed operation param list
    PS_OpCompleted,         // Completed operation statement
    PS_OpSqSeen,            // Seen '(' for operation
    PS_OpQsSeen,            // Seen ')' for operation
    PS_OpParCommaSeen,      // Seen ',' in list of op params
    PS_OpParDirSeen,        // Seen parameter direction
    PS_OpParTypeSeen,       // Seen parameter type
    PS_OpParDeclSeen,       // Seen parameter declaration
    PS_OpOnewaySeen,        // Seen ONEWAY keyword

    PS_RaiseSeen,           // Seen RAISES keyword
    PS_RaiseSqSeen,         // Seen '(' for RAISES
    PS_RaiseQsSeen,         // Seen ')' for RAISES

    PS_DeclsCommaSeen,      // Seen ',' in declarators list
    PS_DeclsDeclSeen        // Seen complete decl in decls list
};

#endif // _IDLC_IDLCTYPES_HXX_

