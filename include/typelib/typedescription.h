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
#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#define _TYPELIB_TYPEDESCRIPTION_H_

#include <cppu/cppudllapi.h>
#include <typelib/uik.h>
#include <typelib/typeclass.h>
#include <rtl/ustring.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct _typelib_TypeDescription;

#if defined( SAL_W32)
#pragma pack(push, 8)
#endif

/** Holds a weak reference to a type description.
*/
typedef struct _typelib_TypeDescriptionReference
{
    /** reference count of type; don't ever modify this by yourself, use
        typelib_typedescriptionreference_acquire() and typelib_typedescriptionreference_release()
    */
    sal_Int32                           nRefCount;
    /** number of static references of type, because of the fact that some types are needed
        until program termination and are commonly held static.
    */
    sal_Int32                           nStaticRefCount;
    /** type class of type
    */
    typelib_TypeClass                   eTypeClass;
    /** fully qualified name of type
    */
    rtl_uString *                       pTypeName;
    /** pointer to full typedescription; this value is only valid if the type is never swapped out
    */
    struct _typelib_TypeDescription *   pType;
    /** pointer to optimize the runtime; not for public use
    */
    void *                              pUniqueIdentifier;
    /** reserved for future use; 0 if not used
    */
    void *                              pReserved;
} typelib_TypeDescriptionReference;

/** Full type description of a type. Memory layout of this struct is identical to the
    typelib_TypeDescriptionReference for the first six members.
    So a typedescription can be used as type reference.
*/
typedef struct _typelib_TypeDescription
{
    /** reference count; don't ever modify this by yourself, use
        typelib_typedescription_acquire() and typelib_typedescription_release()
    */
    sal_Int32                           nRefCount;
    /** number of static references of type, because of the fact that some types are needed
        until program termination and are commonly held static.
    */
    sal_Int32                           nStaticRefCount;
    /** type class of type
    */
    typelib_TypeClass                   eTypeClass;
    /** fully qualified name of type
    */
    rtl_uString *                       pTypeName;
    /** pointer to self to distinguish reference from description; for internal use only
    */
    struct _typelib_TypeDescription *   pSelf;
    /** pointer to optimize the runtime; not for public use
    */
    void *                              pUniqueIdentifier;
    /** reserved for future use; 0 if not used
    */
    void *                              pReserved;

    /** flag to determine whether the description is complete:
        compound and union types lack of member names, enums lack of member types and names,
        interfaces lack of members and table init.
        Call typelib_typedescription_complete() if false.
    */
    sal_Bool                            bComplete;
    /** size of type
    */
    sal_Int32                           nSize;
    /** alignment of type
    */
    sal_Int32                           nAlignment;
    /** pointer to weak reference
    */
    typelib_TypeDescriptionReference *  pWeakRef;
    /** determines, if type can be unloaded (and it is possible to reloaded it)
    */
    sal_Bool                            bOnDemand;
} typelib_TypeDescription;

/** Type description for exception types.
*/
typedef struct _typelib_CompoundTypeDescription
{
    /** inherits all members of typelib_TypeDescription
    */
    typelib_TypeDescription             aBase;

    /** pointer to base type description, else 0
    */
    struct _typelib_CompoundTypeDescription * pBaseTypeDescription;

    /** number of members
    */
    sal_Int32                           nMembers;
    /** byte offsets of each member including the size the base type
    */
    sal_Int32 *                         pMemberOffsets;
    /** members of the struct or exception
    */
    typelib_TypeDescriptionReference ** ppTypeRefs;
    /** member names of the struct or exception
    */
    rtl_uString **                      ppMemberNames;
} typelib_CompoundTypeDescription;

/**
   Type description for struct types.

   This is only used to represent plain struct types and instantiated
   polymorphic struct types; there is no representation of polymorphic struct
   type templates at this level.

   @since UDK 3.2.0
 */
typedef struct _typelib_StructTypeDescription
{
    /**
       Derived from typelib_CompoundTypeDescription.
     */
    typelib_CompoundTypeDescription aBase;

    /**
       Flags for direct members, specifying whether they are of parameterized
       type (true) or explict type (false).

       For a plain struct type, this is a null pointer.
     */
    sal_Bool * pParameterizedTypes;
} typelib_StructTypeDescription;

/** Type description of a union. The type class of this description is typelib_TypeClass_UNION.
*/
typedef struct _typelib_UnionTypeDescription
{
    /** inherits all members of typelib_TypeDescription
    */
    typelib_TypeDescription             aBase;

    /** type of the discriminant
    */
    typelib_TypeDescriptionReference *  pDiscriminantTypeRef;

    /** union default descriminant
    */
    sal_Int64                           nDefaultDiscriminant;
    /** union default member type (may be 0)
     */
    typelib_TypeDescriptionReference *  pDefaultTypeRef;
    /** number of union member types
    */
    sal_Int32                           nMembers;
    /** union member discriminant values (same order as idl declaration)
    */
    sal_Int64 *                         pDiscriminants;
    /** union member value types (same order as idl declaration)
    */
    typelib_TypeDescriptionReference ** ppTypeRefs;
    /** union member value names (same order as idl declaration)
    */
    rtl_uString **                      ppMemberNames;
    /** union value offset for data access
    */
    sal_Int32                           nValueOffset;
} typelib_UnionTypeDescription;

/** Type description of an array or sequence.
*/
typedef struct _typelib_IndirectTypeDescription
{
    /** inherits all members of typelib_TypeDescription
    */
    typelib_TypeDescription             aBase;

    /** array, sequence: pointer to element type
    */
    typelib_TypeDescriptionReference *  pType;
} typelib_IndirectTypeDescription;

/** Type description of an array.
*/
typedef struct _typelib_ArrayTypeDescription
{
    /** inherits all members of typelib_IndirectTypeDescription
    */
    typelib_IndirectTypeDescription     aBase;

    /** number of dimensions
    */
    sal_Int32                           nDimensions;
    /** number of total array elements
    */
    sal_Int32                           nTotalElements;
    /** array of dimensions
    */
    sal_Int32 *                         pDimensions;
} typelib_ArrayTypeDescription;

/** Type description of an enum. The type class of this description is typelib_TypeClass_ENUM.
*/
typedef struct _typelib_EnumTypeDescription
{
    /** inherits all members of typelib_TypeDescription
    */
    typelib_TypeDescription             aBase;

    /** first value of the enum
    */
    sal_Int32                           nDefaultEnumValue;
    /** number of enum values
    */
    sal_Int32                           nEnumValues;
    /** names of enum values
    */
    rtl_uString **                      ppEnumNames;
    /** values of enum (corresponding to names in similar order)
    */
    sal_Int32 *                         pEnumValues;
} typelib_EnumTypeDescription;

/** Description of an interface method parameter.
*/
typedef struct _typelib_MethodParameter
{
    /** name of parameter
    */
    rtl_uString *                       pName;
    /** type of parameter
    */
    typelib_TypeDescriptionReference *  pTypeRef;
    /** true: the call type of this parameter is [in] or [inout]
        false: the call type of this parameter is [out]
    */
    sal_Bool                            bIn;
    /** true: the call type of this parameter is [out] or [inout]
        false: the call type of this parameter is [in]
    */
    sal_Bool                            bOut;
} typelib_MethodParameter;

/** Common base type description of typelib_InterfaceMethodTypeDescription and
    typelib_InterfaceAttributeTypeDescription.
*/
typedef struct _typelib_InterfaceMemberTypeDescription
{
    /** inherits all members of typelib_TypeDescription
    */
    typelib_TypeDescription             aBase;

    /** position of member in the interface including the number of members of
        any base interfaces
    */
    sal_Int32                           nPosition;
    /** name of member
    */
    rtl_uString *                       pMemberName;
} typelib_InterfaceMemberTypeDescription;

/** Type description of an interface method. The type class of this description is
    typelib_TypeClass_INTERFACE_METHOD. The size and the alignment are 0.
*/
typedef struct _typelib_InterfaceMethodTypeDescription
{
    /** inherits all members of typelib_InterfaceMemberTypeDescription
    */
    typelib_InterfaceMemberTypeDescription      aBase;

    /** type of the return value
    */
    typelib_TypeDescriptionReference *          pReturnTypeRef;
    /** number of parameters
    */
    sal_Int32                                   nParams;
    /** array of parameters
    */
    typelib_MethodParameter *                   pParams;
    /** number of exceptions
    */
    sal_Int32                                   nExceptions;
    /** array of exception types
    */
    typelib_TypeDescriptionReference **         ppExceptions;
    /** determines whether method is declared oneway
    */
    sal_Bool                                    bOneWay;

    /** the interface description this method is a member of
    */
    struct _typelib_InterfaceTypeDescription *  pInterface;
    /** the inherited direct base method (null for a method that is not
        inherited)

        @since UDK 3.2.0
    */
    typelib_TypeDescriptionReference *          pBaseRef;
    /** if pBaseRef is null, the member position of this method within
        pInterface, not counting members inherited from bases; if pBaseRef is
        not null, the index of the direct base within pInterface from which this
        method is inherited

        @since UDK 3.2.0
    */
    sal_Int32                                   nIndex;
} typelib_InterfaceMethodTypeDescription;

/** The description of an interface attribute. The type class of this description is
    typelib_TypeClass_INTERFACE_ATTRIBUTE. The size and the alignment are 0.
*/
typedef struct _typelib_InterfaceAttributeTypeDescription
{
    /** inherits all members of typelib_InterfaceMemberTypeDescription
    */
    typelib_InterfaceMemberTypeDescription      aBase;

    /** determines whether attribute is read only
    */
    sal_Bool                                    bReadOnly;
    /** type of the attribute
    */
    typelib_TypeDescriptionReference *          pAttributeTypeRef;

    /** the interface description this attribute is a member of
    */
    struct _typelib_InterfaceTypeDescription *  pInterface;
    /** the inherited direct base attribute (null for an attribute that is not
        inherited)

        @since UDK 3.2.0
    */
    typelib_TypeDescriptionReference *          pBaseRef;
    /** if pBaseRef is null, the member position of this attribute within
        pInterface, not counting members inherited from bases; if pBaseRef is
        not null, the index of the direct base within pInterface from which this
        attribute is inherited

        @since UDK 3.2.0
    */
    sal_Int32                                   nIndex;
    /** number of getter exceptions

        @since UDK 3.2.0
    */
    sal_Int32                                   nGetExceptions;
    /** array of getter exception types

        @since UDK 3.2.0
    */
    typelib_TypeDescriptionReference **         ppGetExceptions;
    /** number of setter exceptions

        @since UDK 3.2.0
    */
    sal_Int32                                   nSetExceptions;
    /** array of setter exception types

        @since UDK 3.2.0
    */
    typelib_TypeDescriptionReference **         ppSetExceptions;
} typelib_InterfaceAttributeTypeDescription;

/** Type description of an interface.

    <p>Not all members are always initialized (not yet initialized members being
    null); there are three levels:</p>
    <ul>
        <li>Minimally, only <code>aBase</code>,
        <code>pBaseTypeDescription</code>, <code>aUik</code>,
        <code>nBaseTypes</code>, and <code>ppBaseTypes</code> are initialized;
        <code>aBase.bComplete</code> is false.  This only happens when an
        interface type description is created with
        <code>typelib_static_mi_interface_type_init</code> or
        <code>typelib_static_interface_type_init</code>.</li>

        <li>At the next level, <code>nMembers</code>, <code>ppMembers</code>,
        <code>nAllMembers</code>, <code>ppAllMembers</code> are also
        initialized; <code>aBase.bComplete</code> is still false.  This happens
        when an interface type description is created with
        <code>typelib_typedescription_newMIInterface</code> or
        <code>typelib_typedescription_newInterface</code>.</li>

        <li>At the final level, <code>pMapMemberIndexToFunctionIndex</code>,
        <code>nMapFunctionIndexToMemberIndex</code>, and
        <code>pMapFunctionIndexToMemberIndex</code> are also initialized;
        <code>aBase.bComplete</code> is true.  This happens after a call to
        <code>typelib_typedescription_complete</code>.</li>
    </ul>
*/
typedef struct _typelib_InterfaceTypeDescription
{
    /** inherits all members of typelib_TypeDescription
    */
    typelib_TypeDescription                     aBase;

    /** pointer to base type description, else 0

        @deprecated
        use nBaseTypes and ppBaseTypes instead
    */
    struct _typelib_InterfaceTypeDescription *  pBaseTypeDescription;
    /** unique identifier of interface

        @deprecated
        should always contain all-zeros
    */
    typelib_Uik                                 aUik;
    /** number of members
    */
    sal_Int32                                   nMembers;
    /** array of members; references attributes or methods
    */
    typelib_TypeDescriptionReference **         ppMembers;
    /** number of members including members of base interface
    */
    sal_Int32                                   nAllMembers;
    /** array of members including members of base interface; references attributes or methods
    */
    typelib_TypeDescriptionReference **         ppAllMembers;
    /** array mapping index of the member description to an index doubling for read-write
        attributes (called function index); size of array is nAllMembers
    */
    sal_Int32 *                                 pMapMemberIndexToFunctionIndex;
    /** number of members plus number of read-write attributes
    */
    sal_Int32                                   nMapFunctionIndexToMemberIndex;
    /** array mapping function index to member index; size of arry is nMapFunctionIndexToMemberIndex
    */
    sal_Int32 *                                 pMapFunctionIndexToMemberIndex;
    /** number of base types

        @since UDK 3.2.0
    */
    sal_Int32                                   nBaseTypes;
    /** array of base type descriptions

        @since UDK 3.2.0
    */
    struct _typelib_InterfaceTypeDescription ** ppBaseTypes;
} typelib_InterfaceTypeDescription;

/** Init struct of compound members for typelib_typedescription_new().
*/
typedef struct _typelib_CompoundMember_Init
{
    /** type class of compound member
    */
    typelib_TypeClass   eTypeClass;
    /** name of type of compound member

        For a member of an instantiated polymorphic struct type that is of
        parameterized type, this will be a null pointer.
    */
    rtl_uString *       pTypeName;
    /** name of compound member
    */
    rtl_uString *       pMemberName;
} typelib_CompoundMember_Init;

/**
   Init struct of members for typelib_typedescription_newStruct().

   @since UDK 3.2.0
 */
typedef struct _typelib_StructMember_Init
{
    /**
       Derived from typelib_CompoundMember_Init;
     */
    typelib_CompoundMember_Init aBase;

    /**
       Flag specifying whether the member is of parameterized type (true) or
       explict type (false).
     */
    sal_Bool bParameterizedType;
} typelib_StructMember_Init;

/** Init struct of interface methods for typelib_typedescription_new().
*/
typedef struct _typelib_Parameter_Init
{
    /** type class of parameter
    */
    typelib_TypeClass   eTypeClass;
    /** name of parameter
    */
    rtl_uString *       pTypeName;
    /** name of parameter
    */
    rtl_uString *       pParamName;
    /** true, if parameter is [in] or [inout]
    */
    sal_Bool            bIn;
    /** true, if parameter is [out] or [inout]
    */
    sal_Bool            bOut;
} typelib_Parameter_Init;

/** Init struct of union types for typelib_typedescription_newUnion().
*/
typedef struct _typelib_Union_Init
{
    /** union member discriminant
    */
    sal_Int64           nDiscriminant;
    /** union member name
    */
    rtl_uString *       pMemberName;
    /** union member type
    */
    typelib_TypeDescriptionReference* pTypeRef;
} typelib_Union_Init;

#if defined( SAL_W32)
#pragma pack(pop)
#endif


/** Creates a union type description. All discriminants are handled as int64 values.
    The pDiscriminantTypeRef must be of type byte, short, ..., up to hyper.

    @param ppRet inout union type description
    @param pTypeName name of union type
    @param pDiscriminantTypeRef discriminant type
    @param nDefaultDiscriminant default discriminant
    @param pDefaultTypeRef default value type of union
    @param nMembers number of union members
    @param pMembers init members
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newUnion(
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName,
    typelib_TypeDescriptionReference * pDiscriminantTypeRef,
    sal_Int64 nDefaultDiscriminant,
    typelib_TypeDescriptionReference * pDefaultTypeRef,
    sal_Int32 nMembers,
    typelib_Union_Init * pMembers )
    SAL_THROW_EXTERN_C();

/** Creates an enum type description.

    @param ppRet inout enum type description
    @param pTypeName name of enum
    @param nDefaultValue default enum value
    @param nEnumValues number of enum values
    @param ppEnumNames names of enum values
    @param pEnumValues enum values
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newEnum(
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName,
    sal_Int32 nDefaultValue,
    sal_Int32 nEnumValues,
    rtl_uString ** ppEnumNames,
    sal_Int32 * pEnumValues )
    SAL_THROW_EXTERN_C();

/** Creates an array type description.

    @param ppRet inout enum type description
    @param pElementTypeRef element type
    @param nDimensions number of dimensions
    @param pDimensions dimensions
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newArray(
    typelib_TypeDescription ** ppRet,
    typelib_TypeDescriptionReference * pElementTypeRef,
    sal_Int32 nDimensions,
    sal_Int32 * pDimensions )
    SAL_THROW_EXTERN_C ();

/** Creates a new type description.

    Since this function can only be used to create type descriptions for plain
    struct types, not for instantiated polymorphic struct types, the function
    typelib_typedescription_newStruct should be used instead for all struct
    types.

    @param ppRet inout type description
    @param eTypeClass type class
    @param pTypeName name of type
    @param pType sequence, array: element type;
                 struct, Exception: base type;
    @param nMembers number of members if struct, exception
    @param pMembers array of members if struct, exception
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_new(
    typelib_TypeDescription ** ppRet,
    typelib_TypeClass eTypeClass,
    rtl_uString * pTypeName,
    typelib_TypeDescriptionReference * pType,
    sal_Int32 nMembers,
    typelib_CompoundMember_Init * pMembers )
    SAL_THROW_EXTERN_C();

/** Creates a new struct type description.

    @param ppRet inout type description
    @param pTypeName name of type
    @param pType base type;
    @param nMembers number of members
    @param pMembers array of members

    @since UDK 3.2.0
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newStruct(
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName,
    typelib_TypeDescriptionReference * pType,
    sal_Int32 nMembers,
    typelib_StructMember_Init * pMembers )
    SAL_THROW_EXTERN_C();

/** Creates an interface type description.

    @param ppRet inout interface type description
    @param pTypeName the fully qualified name of the interface.
    @param nUik1 uik part; deprecated and ignored, should always be 0
    @param nUik2 uik part; deprecated and ignored, should always be 0
    @param nUik3 uik part; deprecated and ignored, should always be 0
    @param nUik4 uik part; deprecated and ignored, should always be 0
    @param nUik5 uik part; deprecated and ignored, should always be 0
    @param pBaseInterface base interface type, else 0
    @param nMembers number of members
    @param ppMembers members; attributes or methods

    @deprecated
    use typelib_typedescription_newMIInterface instead
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newInterface(
    typelib_InterfaceTypeDescription ** ppRet,
    rtl_uString * pTypeName,
    sal_uInt32 nUik1, sal_uInt16 nUik2, sal_uInt16 nUik3, sal_uInt32 nUik4, sal_uInt32 nUik5,
    typelib_TypeDescriptionReference * pBaseInterface,
    sal_Int32 nMembers,
    typelib_TypeDescriptionReference ** ppMembers )
    SAL_THROW_EXTERN_C();

/** Creates a multiple-inheritance interface type description.

    @param ppRet inout interface type description
    @param pTypeName the fully qualified name of the interface.
    @param nUik1 uik part; deprecated and ignored, should always be 0
    @param nUik2 uik part; deprecated and ignored, should always be 0
    @param nUik3 uik part; deprecated and ignored, should always be 0
    @param nUik4 uik part; deprecated and ignored, should always be 0
    @param nUik5 uik part; deprecated and ignored, should always be 0
    @param nBaseInterfaces number of base interface types
    @param ppBaseInterfaces base interface types
    @param nMembers number of members
    @param ppMembers members; attributes or methods

    @since UDK 3.2.0
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newMIInterface(
    typelib_InterfaceTypeDescription ** ppRet,
    rtl_uString * pTypeName,
    sal_uInt32 nUik1, sal_uInt16 nUik2, sal_uInt16 nUik3, sal_uInt32 nUik4, sal_uInt32 nUik5,
    sal_Int32 nBaseInterfaces,
    typelib_TypeDescriptionReference ** ppBaseInterfaces,
    sal_Int32 nMembers,
    typelib_TypeDescriptionReference ** ppMembers )
    SAL_THROW_EXTERN_C();

/** Creates an interface method type description.

    @param ppRet inout method type description
    @param nAbsolutePosition position of member including all members of base interfaces
    @param bOneWay determines whether method is declared oneway
    @param pMethodName fully qualified name of method including interface name
    @param eReturnTypeClass type class of return type
    @param pReturnTypeName type name of the return type
    @param nParams number of parameters
    @param pParams parameter types
    @param nExceptions number of exceptions
    @param ppExceptionNames type names of exceptions
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newInterfaceMethod(
    typelib_InterfaceMethodTypeDescription ** ppRet,
    sal_Int32 nAbsolutePosition,
    sal_Bool bOneWay,
    rtl_uString * pMethodName,
    typelib_TypeClass eReturnTypeClass,
    rtl_uString * pReturnTypeName,
    sal_Int32 nParams,
    typelib_Parameter_Init * pParams,
    sal_Int32 nExceptions,
    rtl_uString ** ppExceptionNames )
    SAL_THROW_EXTERN_C();

/** Creates an interface attribute type description.

    @param ppRet inout attribute type description
    @param nAbsolutePosition position of this attribute including all members of base interfaces
    @param pAttributeName fully qualified name of attribute including interface
    name
    @param eAttributeTypeClass type class of attribute type
    @param pAttributeTypeName type name of attribute type
    @param bReadOnly determines whether attribute is read-only

    @deprecated
    use typelib_typedescription_newExtendedInterfaceAttribute instead
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newInterfaceAttribute(
    typelib_InterfaceAttributeTypeDescription ** ppRet,
    sal_Int32 nAbsolutePosition,
    rtl_uString * pAttributeName,
    typelib_TypeClass eAttributeTypeClass,
    rtl_uString * pAttributeTypeName,
    sal_Bool bReadOnly )
    SAL_THROW_EXTERN_C();

/** Creates an extended interface attribute type description.

    @param ppRet inout attribute type description
    @param nAbsolutePosition position of this attribute including all members of
    base interfaces
    @param pAttributeName fully qualified name of attribute including interface
    name
    @param eAttributeTypeClass type class of attribute type
    @param pAttributeTypeName type name of attribute type
    @param bReadOnly determines whether attribute is read-only
    @param nGetExceptions number of getter exceptions
    @param ppGetExceptionNames type names of getter exceptions
    @param nSetExceptions number of setter exceptions
    @param ppSetExceptionNames type names of setter exceptions

    @since UDK 3.2.0
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newExtendedInterfaceAttribute(
    typelib_InterfaceAttributeTypeDescription ** ppRet,
    sal_Int32 nAbsolutePosition,
    rtl_uString * pAttributeName,
    typelib_TypeClass eAttributeTypeClass,
    rtl_uString * pAttributeTypeName,
    sal_Bool bReadOnly,
    sal_Int32 nGetExceptions, rtl_uString ** ppGetExceptionNames,
    sal_Int32 nSetExceptions, rtl_uString ** ppSetExceptionNames )
    SAL_THROW_EXTERN_C();

/** Increments reference count of given type description.

    @param pDesc type description
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_acquire(
    typelib_TypeDescription * pDesc )
    SAL_THROW_EXTERN_C();

/** Decrements reference count of given type. If reference count reaches 0, the trype description
    is deleted.

    @param pDesc type description
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_release(
    typelib_TypeDescription * pDesc )
    SAL_THROW_EXTERN_C();

/** Registers a type description and creates a type description reference. Type descriptions
    will be registered automatically if they are provided via the callback chain.

    @param ppNewDescription inout description to be registered;
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_register(
    typelib_TypeDescription ** ppNewDescription )
    SAL_THROW_EXTERN_C();

/** Tests whether two types descriptions are equal, i.e. type class and names are equal.

    @param p1 a type description
    @param p2 another type description
    @return true, if type descriptions are equal
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL typelib_typedescription_equals(
    const typelib_TypeDescription * p1, const typelib_TypeDescription * p2 )
    SAL_THROW_EXTERN_C();

/** Retrieves a type description via its fully qualified name.

    @param ppRet inout type description; *ppRet is 0, if type description was not found
    @param pName name demanded type description
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_getByName(
    typelib_TypeDescription ** ppRet, rtl_uString * pName )
    SAL_THROW_EXTERN_C();

/** Sets size of type description cache.

    @param nNewSize new size of cache
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_setCacheSize(
    sal_Int32 nNewSize )
    SAL_THROW_EXTERN_C();

/** Function pointer declaration of callback function get additional descriptions. Callbacks
    must provide complete type descriptions (see typelib_typedescription_complete())!

    @param pContext callback context
    @param ppRet inout type description
    @param pTypeName  name of demanded type description
*/
typedef void (SAL_CALL * typelib_typedescription_Callback)(
    void * pContext, typelib_TypeDescription ** ppRet, rtl_uString * pTypeName );

/** Registers callback function providing additional type descriptions.

    @param pContext callback context
    @param pCallback callback function
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_registerCallback(
    void * pContext, typelib_typedescription_Callback pCallback )
    SAL_THROW_EXTERN_C();

/** Revokes a previously registered callback function.

    @param pContext callback context
    @param pCallback registered callback function
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_revokeCallback(
    void * pContext, typelib_typedescription_Callback pCallback )
    SAL_THROW_EXTERN_C();


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/** Creates a type description reference. This is a weak reference not holding the description.
    If the description is already registered, the previous one is returned.

    @param ppTDR inout type description reference
    @param eTypeClass type class of type
    @param pTypeName name of type
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_new(
    typelib_TypeDescriptionReference ** ppTDR,
    typelib_TypeClass eTypeClass,
    rtl_uString * pTypeName )
    SAL_THROW_EXTERN_C();

/** Creates a type description reference. This is a weak reference not holding the description.
    If the description is already registered, the previous one is returned.

    @param ppTDR inout type description reference
    @param eTypeClass type class of type
    @param pTypeName ascii name of type
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_newByAsciiName(
    typelib_TypeDescriptionReference ** ppTDR,
    typelib_TypeClass eTypeClass,
    const sal_Char * pTypeName )
    SAL_THROW_EXTERN_C();

/** Increments reference count of type description reference.

    @param pRef type description reference
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_acquire(
    typelib_TypeDescriptionReference * pRef )
    SAL_THROW_EXTERN_C();

/** Increments reference count of type description reference. If the reference count reaches 0,
    then the reference is deleted.

    @param pRef type description reference
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_release(
    typelib_TypeDescriptionReference * pRef )
    SAL_THROW_EXTERN_C();

/** Retrieves the type description for a given reference. If it is not possible to resolve the
    reference, null is returned.

    @param[in,out] ppRet type description
    @param[in] pRef type description reference
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_getDescription(
    typelib_TypeDescription ** ppRet, typelib_TypeDescriptionReference * pRef )
    SAL_THROW_EXTERN_C();

/** Tests whether two types description references are equal, i.e. type class and names are equal.

    @param p1 a type description reference
    @param p2 another type description reference
    @return true, if type description references are equal
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL typelib_typedescriptionreference_equals(
    const typelib_TypeDescriptionReference * p1, const typelib_TypeDescriptionReference * p2 )
    SAL_THROW_EXTERN_C();

/** Assigns a type.

    @param ppDest destination type
    @param pSource source type
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_assign(
    typelib_TypeDescriptionReference ** ppDest,
    typelib_TypeDescriptionReference * pSource )
    SAL_THROW_EXTERN_C();

/** Tests if values of type pAssignable can be assigned by values of type pFrom. This includes
    widening conversion (e.g., long assignable from short), as long as there is no data loss.

    @param pAssignable type description of value to be assigned
    @param pFrom type description of value
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL typelib_typedescription_isAssignableFrom(
    typelib_TypeDescription * pAssignable,
    typelib_TypeDescription * pFrom )
    SAL_THROW_EXTERN_C();

/** Tests if values of type pAssignable can be assigned by values of type pFrom. This includes
    widening conversion (e.g., long assignable from short), as long as there is no data loss.

    @param pAssignable type of value to be assigned
    @param pFrom type of value
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL typelib_typedescriptionreference_isAssignableFrom(
    typelib_TypeDescriptionReference * pAssignable,
    typelib_TypeDescriptionReference * pFrom )
    SAL_THROW_EXTERN_C();

/** Gets static type reference of standard types by type class.
    ADDITIONAL OPT: provides Type com.sun.star.uno.Exception for typelib_TypeClass_EXCEPTION
                    and com.sun.star.uno.XInterface for typelib_TypeClass_INTERFACE.

    Thread synchronizes on typelib mutex.

    @param eTypeClass type class of basic type
    @return pointer to type reference pointer
*/
CPPU_DLLPUBLIC typelib_TypeDescriptionReference ** SAL_CALL typelib_static_type_getByTypeClass(
    typelib_TypeClass eTypeClass )
    SAL_THROW_EXTERN_C();

/** Inits static type reference. Thread synchronizes on typelib init mutex.

    @param ppRef pointer to type reference pointer
    @param eTypeClass type class of type
    @param pTypeName ascii name of type
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_static_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const sal_Char * pTypeName )
    SAL_THROW_EXTERN_C();

/** Inits static sequence type reference. Thread synchronizes on typelib init mutex.

    @param ppRef pointer to type reference pointer
    @param pElementType element type of sequence
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_static_sequence_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeDescriptionReference * pElementType )
    SAL_THROW_EXTERN_C ();

/** Inits static array type reference. Thread synchronizes on typelib init mutex.

    @param ppRef pointer to type reference pointer
    @param pElementType element type of sequence
    @param nDimensions number of dimensions
    @param ... additional sal_Int32 parameter for each dimension
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_static_array_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nDimensions, ... )
    SAL_THROW_EXTERN_C ();

/** Inits incomplete static compound type reference. Thread synchronizes on typelib init mutex.

    Since this function can only be used to create type descriptions for plain
    struct types, not for instantiated polymorphic struct types, the function
    typelib_static_struct_type_init should be used instead for all struct types.

    @param ppRef pointer to type reference pointer
    @param eTypeClass typelib_TypeClass_STRUCT or typelib_TypeClass_EXCEPTION
    @param pTypeName name of type
    @param pBaseType base type
    @param nMembers number of members
    @param ppMembers member types
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_static_compound_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType,
    sal_Int32 nMembers, typelib_TypeDescriptionReference ** ppMembers )
    SAL_THROW_EXTERN_C();

/** Inits incomplete static struct type reference.

    Thread synchronizes on typelib init mutex.

    @param ppRef pointer to type reference pointer
    @param pTypeName name of type
    @param pBaseType base type
    @param nMembers number of members
    @param ppMembers member types
    @param pParameterizedTypes flags for direct members, specifying whether they
        are of parameterized type (true) or explict type (false); must be null
        for a plain struct type

    @since UDK 3.2.0
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_static_struct_type_init(
    typelib_TypeDescriptionReference ** ppRef, const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType,
    sal_Int32 nMembers, typelib_TypeDescriptionReference ** ppMembers,
    sal_Bool const * pParameterizedTypes )
    SAL_THROW_EXTERN_C();

/** Inits incomplete static interface type reference. Thread synchronizes on typelib init mutex.

    @param ppRef pointer to type reference pointer
    @param pTypeName name of interface
    @param pBaseType base type
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_static_interface_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType )
    SAL_THROW_EXTERN_C();

/** Inits incomplete static multiple-inheritance interface type reference.
    Thread synchronizes on typelib init mutex.

    @param ppRef pointer to type reference pointer
    @param pTypeName name of interface
    @param nBaseTypes number of base types
    @param ppBaseTypes base types

    @since UDK 3.2.0
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_static_mi_interface_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    sal_Int32 nBaseTypes,
    typelib_TypeDescriptionReference ** ppBaseTypes )
    SAL_THROW_EXTERN_C();

/** Inits incomplete static enum type reference. Thread synchronizes on typelib init mutex.

    @param ppRef pointer to type reference pointer
    @param pTypeName name of enum
    @param nDefaultValue default enum value
*/
CPPU_DLLPUBLIC void SAL_CALL typelib_static_enum_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    sal_Int32 nDefaultValue )
    SAL_THROW_EXTERN_C();

/** Completes a typedescription to be used for, e.g., marshalling values. COMPOUND, UNION,
    INTERFACE and ENUM type descriptions may be partly initialized (see typelib_static_...(),
    typelib_TypeDescription::bComplete). For interface type descriptions, this will also
    init index tables.

    @param ppTypeDescr [inout] type description to be completed (may be exchanged!)
    @return true, if type description is complete
*/
CPPU_DLLPUBLIC sal_Bool SAL_CALL typelib_typedescription_complete(
    typelib_TypeDescription ** ppTypeDescr )
    SAL_THROW_EXTERN_C();

/// @cond INTERNAL

/** Returns true, if the type description reference may lose the type description. Otherwise
    pType is a valid pointer and cannot be discarded through the lifetime of this reference.
    Remark: If the pWeakObj of the type is set too, you can avoid the call of
    ...getDescription(...) and use the description directly. pWeakObj == 0 means, that the
    description is not initialized.
    @internal
*/
inline bool TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK( _typelib_TypeClass eTypeClass )
{
    return (eTypeClass == typelib_TypeClass_INTERFACE_METHOD) ||
     (eTypeClass == typelib_TypeClass_INTERFACE_ATTRIBUTE);
}

/** Gets a description from the reference. The description may not be locked by this call.
    You must use the TYPELIB_DANGER_RELEASE macro to release the description fetched with
    this macro.
    @internal
*/
inline void TYPELIB_DANGER_GET( typelib_TypeDescription** ppMacroTypeDescr,
    typelib_TypeDescriptionReference* pMacroTypeRef )
{
    if (TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK( pMacroTypeRef->eTypeClass ))
    {
        typelib_typedescriptionreference_getDescription( ppMacroTypeDescr, pMacroTypeRef );
    }
    else if (!pMacroTypeRef->pType || !pMacroTypeRef->pType->pWeakRef)
    {
        typelib_typedescriptionreference_getDescription( ppMacroTypeDescr, pMacroTypeRef );
        if (*ppMacroTypeDescr)
        typelib_typedescription_release( *ppMacroTypeDescr );
    }
    else
    {
        *ppMacroTypeDescr = pMacroTypeRef->pType;
    }
}

/** Releases the description previouse fetched by TYPELIB_DANGER_GET.
    @internal
*/
inline void TYPELIB_DANGER_RELEASE( typelib_TypeDescription* pDescription )
{
    if (TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK( pDescription->eTypeClass ))
        typelib_typedescription_release( pDescription );
}

/// @endcond

#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
