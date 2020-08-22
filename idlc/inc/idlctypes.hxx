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
#ifndef INCLUDED_IDLC_INC_IDLCTYPES_HXX
#define INCLUDED_IDLC_INC_IDLCTYPES_HXX

#include <stdio.h>

#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <sal/types.h>
#include <rtl/ustring.hxx>


class AstDeclaration;

typedef std::list< AstDeclaration* > DeclList;

class AstScope;
AstDeclaration* scopeAsDecl(AstScope* pScope);
AstScope* declAsScope(AstDeclaration* pDecl);

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
#define AF_REMOVABLE        0x0100
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
    PS_ServiceSHeadSeen,    // Seen a service member header

    PS_SingletonSeen,       // Seen a SINGLETON keyword
    PS_SingletonIDSeen,     // Seen the singleton ID
    PS_SingletonSqSeen,     // '{' seen for singleton
    PS_SingletonQsSeen,     // '}' seen for singleton
    PS_SingletonBodySeen,   // Seen complete singleton body

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
    PS_MayBeAmbiguousSeen,  // Seen MAYBEAMBIGUOUS keyword
    PS_MayBeDefaultSeen,    // Seen MAYBEDEFAULT keyword
    PS_RemoveableSeen,      // Seen REMOVABLE keyword

    PS_PropertySeen,        // Seen PROPERTY keyword
    PS_PropertyTypeSeen,    // Seen type decl for property
    PS_PropertyCompleted,   // Seen complete property declaration

    PS_ExceptSeen,          // Seen EXCEPTION keyword
    PS_ExceptHeaderSeen,    // Seen exception header keyword
    PS_ExceptIDSeen,        // Seen exception identifier
    PS_ExceptSqSeen,        // Seen '{' for exception
    PS_ExceptQsSeen,        // Seen '}' for exception
    PS_ExceptBodySeen,      // Seen complete exception body

    PS_OpTypeSeen,          // Seen operation return type
    PS_OpIDSeen,            // Seen operation ID
    PS_OpParsCompleted,     // Completed operation param list
    PS_OpSqSeen,            // Seen '(' for operation
    PS_OpQsSeen,            // Seen ')' for operation
    PS_OpParCommaSeen,      // Seen ',' in list of op params
    PS_OpParDirSeen,        // Seen parameter direction
    PS_OpParTypeSeen,       // Seen parameter type
    PS_OpParDeclSeen,       // Seen parameter declaration

    PS_RaiseSeen,           // Seen RAISES keyword
    PS_RaiseSqSeen,         // Seen '(' for RAISES
    PS_RaiseQsSeen,         // Seen ')' for RAISES

    PS_DeclsCommaSeen,      // Seen ',' in declarators list
    PS_DeclsDeclSeen        // Seen complete decl in decls list
};

#endif // INCLUDED_IDLC_INC_IDLCTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
