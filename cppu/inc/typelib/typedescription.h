/*************************************************************************
 *
 *  $RCSfile: typedescription.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:10:55 $
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
#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#define _TYPELIB_TYPEDESCRIPTION_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _TYPELIB_UIK_H_
#include <typelib/uik.h>
#endif
#ifndef _TYPELIB_TYPECLASS_H_
#include <typelib/typeclass.h>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _typelib_TypeDescription typelib_TypeDescription;

#ifdef SAL_W32
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(8)
#endif

/** Holds a weak reference to a type description.
    <br>
*/
typedef struct _typelib_TypeDescriptionReference
{
    /** reference count of type;
        don't ever modify this by yourself, use
        typelib_typedescriptionreference_acquire() and
        typelib_typedescriptionreference_release()
        <br>
    */
    sal_Int32                           nRefCount;
    /** number of static references of type,
        because of the fact that some types are needed
        until program termination and are commonly held static.
        <br>
    */
    sal_Int32                           nStaticRefCount;
    /** type class of type<br>
    */
    typelib_TypeClass                   eTypeClass;
    /** fully qualified name of type<br>
    */
    rtl_uString *                       pTypeName;
    /** pointer to full typedescription; this value is only valid if
        the type is never swapped out<br>
    */
    typelib_TypeDescription *           pType;
    /** pointer to optimize the runtime; not for public use<br>
    */
    void *                              pUniqueIdentifier;
    /** reserved for future use; 0 if not used<br>
    */
    void *                              pReserved;
} typelib_TypeDescriptionReference;

/** Full type description of a type. Memory layout of this struct
    is identical to the typelib_TypeDescriptionReference for the first six members.
    So a typedescription can be used as type reference.
    <br>
*/
typedef struct _typelib_TypeDescription
{
    /** reference count;
        don't ever modify this by yourself, use
        typelib_typedescription_acquire() and
        typelib_typedescription_release()
        <br>
    */
    sal_Int32                           nRefCount;
    /** number of static references of type,
        because of the fact that some types are needed
        until program termination and are commonly held static.
        <br>
    */
    sal_Int32                           nStaticRefCount;
    /** type class of type<br>
    */
    typelib_TypeClass                   eTypeClass;
    /** fully qualified name of type<br>
    */
    rtl_uString *                       pTypeName;
    /** pointer to self to distinguish reference from description;
        for internal use only<br>
    */
    typelib_TypeDescription *           pSelf;
    /** pointer to optimize the runtime; not for public use<br>
    */
    void *                              pUniqueIdentifier;
    /** reserved for future use; 0 if not used<br>
    */
    void *                              pReserved;

    /** flag to determine whether the description is complete:
        compound and union types lack of member names,
        enums lack of member types and names,
        interfaces lack of members and table init.
        Call typelib_typedescription_complete() if false.
        <br>
    */
    sal_Bool                            bComplete;
    /** size of type<br>
    */
    sal_Int32                           nSize;
    /** alignment of type<br>
    */
    sal_Int32                           nAlignment;
    /** pointer to weak reference<br>
    */
    typelib_TypeDescriptionReference *  pWeakRef;
    /** determines, if type can be unloaded (and it is possible to reloaded it)<br>
    */
    sal_Bool                            bOnDemand;
} typelib_TypeDescription;

typedef struct _typelib_CompoundTypeDescription typelib_CompoundTypeDescription;

/** Type description of a struct or exception.
    <br>
*/
typedef struct _typelib_CompoundTypeDescription
{
    /** inherits all members of typelib_TypeDescription<br>
    */
    typelib_TypeDescription             aBase;

    /** pointer to base type description, else 0<br>
    */
    typelib_CompoundTypeDescription *   pBaseTypeDescription;

    /** number of members<br>
    */
    sal_Int32                           nMembers;
    /** byte offsets of each member including the size the base type<br>
    */
    sal_Int32 *                         pMemberOffsets;
    /** members of the struct or exception<br>
    */
    typelib_TypeDescriptionReference ** ppTypeRefs;
    /** member names of the struct or exception<br>
    */
    rtl_uString **                      ppMemberNames;
} typelib_CompoundTypeDescription;

/** Type description of a union.<br>
    The type class of this description is typelib_TypeClass_UNION.
    <br>
*/
typedef struct _typelib_UnionTypeDescription
{
    /** inherits all members of typelib_TypeDescription<br>
    */
    typelib_TypeDescription             aBase;

    /** type of the discriminant<br>
    */
    typelib_TypeDescriptionReference *  pDiscriminantTypeRef;

    /** union default descriminant<br>
    */
    sal_Int64                           nDefaultDiscriminant;
    /** union default member type (may be 0)<br>
     */
    typelib_TypeDescriptionReference *  pDefaultTypeRef;
    /** number of union member types<br>
    */
    sal_Int32                           nMembers;
    /** union member discriminant values (same order as idl declaration)<br>
    */
    sal_Int64 *                         pDiscriminants;
    /** union member value types (same order as idl declaration)<br>
    */
    typelib_TypeDescriptionReference ** ppTypeRefs;
    /** union member value names (same order as idl declaration)<br>
    */
    rtl_uString **                      ppMemberNames;
    /** union value offset for data access<br>
    */
    sal_Int32                           nValueOffset;
} typelib_UnionTypeDescription;

/** Type description of an array or sequence.
    <br>
*/
typedef struct _typelib_IndirectTypeDescription
{
    /** inherits all members of typelib_TypeDescription<br>
    */
    typelib_TypeDescription             aBase;

    /** array, sequence: pointer to element type<br>
    */
    typelib_TypeDescriptionReference *  pType;
} typelib_IndirectTypeDescription;

/** Type description of an enum.<br>
    The type class of this description is typelib_TypeClass_ENUM.
    <br>
*/
typedef struct _typelib_EnumTypeDescription
{
    /** inherits all members of typelib_TypeDescription<br>
    */
    typelib_TypeDescription             aBase;

    /** first value of the enum<br>
    */
    sal_Int32                           nDefaultEnumValue;
    /** number of enum values<br>
    */
    sal_Int32                           nEnumValues;
    /** names of enum values<br>
    */
    rtl_uString **                      ppEnumNames;
    /** values of enum (corresponding to names in similar order)<br>
    */
    sal_Int32 *                         pEnumValues;
} typelib_EnumTypeDescription;

/** Description of an interface method parameter.
    <br>
*/
typedef struct _typelib_MethodParameter
{
    /** name of parameter<br>
    */
    rtl_uString *                       pName;
    /** type of parameter<br>
    */
    typelib_TypeDescriptionReference *  pTypeRef;
    /** true: the call type of this parameter is [in] or [inout]
        false: the call type of this parameter is [out]<br>
    */
    sal_Bool                            bIn;
    /** true: the call type of this parameter is [out] or [inout]
        false: the call type of this parameter is [in]<br>
    */
    sal_Bool                            bOut;
} typelib_MethodParameter;

/** Common base type description of
    typelib_InterfaceMemberTypeDescription and
    typelib_InterfaceAttributeTypeDescription.
    <br>
*/
typedef struct _typelib_InterfaceMemberTypeDescription
{
    /** inherits all members of typelib_TypeDescription<br>
    */
    typelib_TypeDescription             aBase;

    /** position of member in the interface including the number of members
        of the base interface<br>
    */
    sal_Int32                           nPosition;
    /** name of member<br>
    */
    rtl_uString *                       pMemberName;
} typelib_InterfaceMemberTypeDescription;

/** Type description of an interface method.<br>
    The type class of this description is typelib_TypeClass_INTERFACE_METHOD.
    The size and the alignment are 0.
    <br>
*/
typedef struct _typelib_InterfaceMethodTypeDescription
{
    /** inherits all members of typelib_InterfaceMemberTypeDescription<br>
    */
    typelib_InterfaceMemberTypeDescription      aBase;

    /** type of the return value<br>
    */
    typelib_TypeDescriptionReference *          pReturnTypeRef;
    /** number of parameters<br>
    */
    sal_Int32                                   nParams;
    /** array of parameters<br>
    */
    typelib_MethodParameter *                   pParams;
    /** number of exceptions<br>
    */
    sal_Int32                                   nExceptions;
    /** array of exception types<br>
    */
    typelib_TypeDescriptionReference **         ppExceptions;
    /** determines whether method is declared oneway<br>
    */
    sal_Bool                                    bOneWay;
} typelib_InterfaceMethodTypeDescription;

/** The description of an interface attribute.<br>
    The type class of this description is typelib_TypeClass_INTERFACE_ATTRIBUTE.
    The size and the alignment are 0.
    <br>
*/
typedef struct _typelib_InterfaceAttributeTypeDescription
{
    /** inherits all members of typelib_InterfaceMemberTypeDescription<br>
    */
    typelib_InterfaceMemberTypeDescription      aBase;

    /** determines whether attribute is read only<br>
    */
    sal_Bool                                    bReadOnly;
    /** type of the attribute<br>
    */
    typelib_TypeDescriptionReference *          pAttributeTypeRef;
} typelib_InterfaceAttributeTypeDescription;

typedef struct _typelib_InterfaceTypeDescription typelib_InterfaceTypeDescription;

/** Type description of an interface.
    <br>
*/
typedef struct _typelib_InterfaceTypeDescription
{
    /** inherits all members of typelib_TypeDescription<br>
    */
    typelib_TypeDescription                     aBase;

    /** pointer to base type description, else 0<br>
    */
    typelib_InterfaceTypeDescription *          pBaseTypeDescription;
    /** unique identifier of interface<br>
    */
    typelib_Uik                                 aUik;
    /** number of members<br>
    */
    sal_Int32                                   nMembers;
    /** array of members; references attributes or methods<br>
    */
    typelib_TypeDescriptionReference **         ppMembers;
    /** number of members including members of base interface<br>
    */
    sal_Int32                                   nAllMembers;
    /** array of members including members of base interface;
        references attributes or methods<br>
    */
    typelib_TypeDescriptionReference **         ppAllMembers;
    /** array mapping index of the member description to an index
        doubling for read-write attributes (called function index);
        size of array is nAllMembers<br>
    */
    sal_Int32 *                                 pMapMemberIndexToFunctionIndex;
    /** number of members plus number of read-write attributes<br>
    */
    sal_Int32                                   nMapFunctionIndexToMemberIndex;
    /** array mapping function index to member index;
        size of arry is nMapFunctionIndexToMemberIndex<br>
    */
    sal_Int32 *                                 pMapFunctionIndexToMemberIndex;
} typelib_InterfaceTypeDescription;

/** Init struct of compound members for typelib_typedescription_new().
    <br>
*/
typedef struct _typelib_CompoundMember_Init
{
    /** type class of compound member<br>
    */
    typelib_TypeClass   eTypeClass;
    /** name of type of compound member<br>
    */
    rtl_uString *       pTypeName;
    /** name of compound member<br>
    */
    rtl_uString *       pMemberName;
} typelib_CompoundMember_Init;

/** Init struct of interface methods for typelib_typedescription_new().
    <br>
*/
typedef struct _typelib_Parameter_Init
{
    /** type class of parameter<br>
    */
    typelib_TypeClass   eTypeClass;
    /** name of parameter<br>
    */
    rtl_uString *       pTypeName;
    /** name of parameter<br>
    */
    rtl_uString *       pParamName;
    /** true, if parameter is [in] or [inout]<br>
    */
    sal_Bool            bIn;
    /** true, if parameter is [out] or [inout]<br>
    */
    sal_Bool            bOut;
} typelib_Parameter_Init;

/** Init struct of union types for typelib_typedescription_newUnion().
    <br>
*/
typedef struct _typelib_Union_Init
{
    /** union member discriminant<br>
    */
    sal_Int64           nDiscriminant;
    /** union member name<br>
    */
    rtl_uString *       pMemberName;
    /** union member type<br>
    */
    typelib_TypeDescriptionReference* pTypeRef;
} typelib_Union_Init;

#ifdef SAL_W32
#pragma pack(pop)
#elif defined(SAL_OS2)
#pragma pack()
#endif


/** Creates a union type description.<br>
    All discriminants are handled as int64 values.
    The pDiscriminantTypeRef must be of type byte, short, ..., up to hyper.
    <br>
    @param ppRet inout union type description
    @param pTypeName name of union type
    @param pDiscriminantTypeRef discriminant type
    @param nDefaultDiscriminant default discriminant
    @param pDefaultTypeRef default value type of union
    @param nMembers number of union members
    @param pMembers init members
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_newUnion(
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName,
    typelib_TypeDescriptionReference * pDiscriminantTypeRef,
    sal_Int64 nDefaultDiscriminant,
    typelib_TypeDescriptionReference * pDefaultTypeRef,
    sal_Int32 nMembers,
    typelib_Union_Init * pMembers )
    SAL_THROW_EXTERN_C();

/** Creates an enum type description.
    <br>
    @param ppRet inout enum type description
    @param pTypeName name of enum
    @param nDefaultEnumValue default enum value
    @param nEnumValues number of enum values
    @param ppEnumNames names of enum values
    @param pEnumValues enum values
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_newEnum(
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName,
    sal_Int32 nDefaultValue,
    sal_Int32 nEnumValues,
    rtl_uString ** ppEnumNames,
    sal_Int32 * pEnumValues )
    SAL_THROW_EXTERN_C();

/** Creates a new type description.
    <br>
    @param ppRet inout type description
    @param eTypeClass type class
    @param pTypeName name of type
    @param pType sequence, array: element type;
                 struct, Exception: base type;
    @param nMembers number of members if struct, exception
    @param pMember array of members if struct, exception
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_new(
    typelib_TypeDescription ** ppRet,
    typelib_TypeClass eTypeClass,
    rtl_uString * pTypeName,
    typelib_TypeDescriptionReference * pType,
    sal_Int32 nMembers,
    typelib_CompoundMember_Init * pMembers )
    SAL_THROW_EXTERN_C();

/** Creates an interface type description.
    <br>
    @param ppRet inout interface type description
    @param pTypeName the fully qualified name of the interface.
    @param nUik1 uik part
    @param nUik2 uik part
    @param nUik3 uik part
    @param nUik4 uik part
    @param nUik5 uik part
    @param pBaseInterface base interface type, else 0
    @param nMembers number of members
    @param ppMembers members; attributes or methods
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_newInterface(
    typelib_InterfaceTypeDescription ** ppRet,
    rtl_uString * pTypeName,
    sal_uInt32 nUik1, sal_uInt16 nUik2, sal_uInt16 nUik3, sal_uInt32 nUik4, sal_uInt32 nUik5,
    typelib_TypeDescriptionReference * pBaseInterface,
    sal_Int32 nMembers,
    typelib_TypeDescriptionReference ** ppMembers )
    SAL_THROW_EXTERN_C();

/** Creates an interface method type description.
    <br>
    @param ppRet inout method type description
    @param nAbsolutePosition position of member including all members of base interfaces
    @param bOneWay determines whether method is declared oneway
    @param pTypeName fully qualified name of method including interface name
    @param eReturnTypeClass type class of return type
    @param pReturnTypeName type name of the return type
    @param nParams number of parameters
    @param pParams parameter types
    @param nExceptions number of exceptions
    @param ppExceptionNames type names of exceptions
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_newInterfaceMethod(
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
    <br>
    @param ppRet inout attribute type description
    @param nAbsolutePosition position of this attribute including all members of base interfaces
    @param pTypeName fully qualified name of attribute including interface name
    @param eAttributeTypeClass type class of attribute type
    @param pAttributeTypeName type name of attribute type
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_newInterfaceAttribute(
    typelib_InterfaceAttributeTypeDescription ** ppRet,
    sal_Int32 nAbsolutePosition,
    rtl_uString * pAttributeName,
    typelib_TypeClass eAttributeTypeClass,
    rtl_uString * pAttributeTypeName,
    sal_Bool bReadOnly )
    SAL_THROW_EXTERN_C();

/** Increments reference count of given type description.
    <br>
    @param pDesc type description
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_acquire(
    typelib_TypeDescription * pDesc )
    SAL_THROW_EXTERN_C();

/** Decrements reference count of given type.<br>
    If reference count reaches 0, the trype description is deleted.
    <br>
    @param pDesc type description
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_release(
    typelib_TypeDescription * pDesc )
    SAL_THROW_EXTERN_C();

/** Registers a type description and creates a type description reference.<br>
    Type descriptions will be registered automatically if they are provided
    via the callback chain.
    @param ppNewDescription inout description to be registered;
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_register(
    typelib_TypeDescription ** ppNewDescription )
    SAL_THROW_EXTERN_C();

/** Tests whether two types descriptions are equal, i.e. type class and names are equal.
    <br>
    @param p1 a type description
    @param p2 another type description
    @return true, if type descriptions are equal
*/
SAL_DLLEXPORT sal_Bool SAL_CALL typelib_typedescription_equals(
    const typelib_TypeDescription * p1, const typelib_TypeDescription * p2 )
    SAL_THROW_EXTERN_C();

/** Retrieves a type description via its fully qualified name.
    <br>
    @param ppRet inout type description; *ppRet is 0, if type description was not found
    @param pName name demanded type description
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_getByName(
    typelib_TypeDescription ** ppRet, rtl_uString * pName )
    SAL_THROW_EXTERN_C();

/** Sets size of type description cache.
    <br>
    @param nNewSize new size of cache
*/
SAL_DLLEXPORT void SAL_CALL typelib_setCacheSize(
    sal_Int32 nNewSize )
    SAL_THROW_EXTERN_C();

/** Function pointer declaration of callback function get additional descriptions.
    Callbacks <b>must</b> provide <b>complete</b> type descriptions!
    <br>
    @param pContext callback context
    @param ppRet inout type description
    @param pTypeName  name of demanded type description
*/
typedef void (SAL_CALL * typelib_typedescription_Callback)(
    void * pContext, typelib_TypeDescription ** ppRet, rtl_uString * pTypeName );

/** Registers callback function providing additional type descriptions.
    <br>
    @param pContext callback context
    @param pCallback callback function
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_registerCallback(
    void * pContext, typelib_typedescription_Callback pCallback )
    SAL_THROW_EXTERN_C();

/** Revokes a previously registered callback function.
    <br>
    @param pContext callback context
    @param pCallback registered callback function
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescription_revokeCallback(
    void * pContext, typelib_typedescription_Callback pCallback )
    SAL_THROW_EXTERN_C();


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*  Returns true, if the type description reference may lose the type
    description. Otherwise pType is a valid pointer and cannot
    be discarded through the lifetime of this reference.
    Remark: If the pWeakObj of the type is set too, you can avoid
    the call of ...getDescription(...) and use the description
    direct. pWeakObj == 0 means, that the description is not
    initialized.
*/
#define TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK( eTypeClass ) \
    ((eTypeClass) == typelib_TypeClass_INTERFACE_METHOD || \
     (eTypeClass) == typelib_TypeClass_INTERFACE_ATTRIBUTE)

/*  Gets a description from the reference. The description may not be locked
    by this call. You must use the TYPELIB_DANGER_RELEASE macro to release
    the description fetched with this macro.
*/
#define TYPELIB_DANGER_GET( ppDescription, pTypeRef ) \
{ \
    typelib_TypeDescriptionReference * __pMacroTypeRef = (pTypeRef); \
    if (TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK( __pMacroTypeRef->eTypeClass )) \
    { \
        typelib_typedescriptionreference_getDescription( ppDescription, __pMacroTypeRef ); \
    } \
    else if (! __pMacroTypeRef->pType->pWeakRef) \
    { \
        typelib_typedescriptionreference_getDescription( ppDescription, __pMacroTypeRef ); \
        typelib_typedescription_release( *(ppDescription) ); \
    } \
    else \
    { \
        *(ppDescription) = __pMacroTypeRef->pType; \
    } \
}

/*  Releases the description previouse fetched by TYPELIB_DANGER_GET.
*/
#define TYPELIB_DANGER_RELEASE( pDescription ) \
{ \
    if (TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK( (pDescription)->eTypeClass )) \
        typelib_typedescription_release( pDescription ); \
}

/** Creates a type description reference.<br>
    This is a weak reference <b>not</b> holding the description.
    If the description is already registered, the previous one is returned.
    <br>
    @param ppTDR inout type description reference
    @param eTypeClass type class of type
    @param pTypeName name of type
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_new(
    typelib_TypeDescriptionReference ** ppTDR,
    typelib_TypeClass eTypeClass,
    rtl_uString * pTypeName )
    SAL_THROW_EXTERN_C();

/** Creates a type description reference.<br>
    This is a weak reference <b>not</b> holding the description.
    If the description is already registered, the previous one is returned.
    <br>
    @param ppTDR inout type description reference
    @param eTypeClass type class of type
    @param pTypeName ascii name of type
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_newByAsciiName(
    typelib_TypeDescriptionReference ** ppTDR,
    typelib_TypeClass eTypeClass,
    const sal_Char * pTypeName )
    SAL_THROW_EXTERN_C();

/** Increments reference count of type description reference.
    <br>
    @param pRef type description reference
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_acquire(
    typelib_TypeDescriptionReference * pRef )
    SAL_THROW_EXTERN_C();

/** Increments reference count of type description reference.
    If the reference count reaches 0, the reference is deleted.
    <br>
    @param pRef type description reference
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_release(
    typelib_TypeDescriptionReference * pRef )
    SAL_THROW_EXTERN_C();

/** Retrieves the type description for a given reference.<br>
    If it is not possible to resolve the reference, null is returned.
    <br>
    @param ppRet inout type description
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_getDescription(
    typelib_TypeDescription ** ppRet, typelib_TypeDescriptionReference * pRef )
    SAL_THROW_EXTERN_C();

/** Tests whether two types description references are equal, i.e. type class and names are equal.
    <br>
    @param p1 a type description reference
    @param p2 another type description reference
    @return true, if type description references are equal
*/
SAL_DLLEXPORT sal_Bool SAL_CALL typelib_typedescriptionreference_equals(
    const typelib_TypeDescriptionReference * p1, const typelib_TypeDescriptionReference * p2 )
    SAL_THROW_EXTERN_C();

/** Assigns a type.
    <br>
    @param ppDest destination type
    @param pSource source type
*/
SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_assign(
    typelib_TypeDescriptionReference ** ppDest,
    typelib_TypeDescriptionReference * pSource )
    SAL_THROW_EXTERN_C();

/** Tests if values of type pAssignable can be assigned by values of type pFrom.
    This includes widening conversion (e.g., long assignable from short), as long
    as there is <b>no</b> data loss.
    <br>
    @param pAssignable type description of value to be assigned
    @param pFrom type description of value
*/
SAL_DLLEXPORT sal_Bool SAL_CALL typelib_typedescription_isAssignableFrom(
    typelib_TypeDescription * pAssignable,
    typelib_TypeDescription * pFrom )
    SAL_THROW_EXTERN_C();

/** Tests if values of type pAssignable can be assigned by values of type pFrom.
    This includes widening conversion (e.g., long assignable from short), as long
    as there is <b>no</b> data loss.
    <br>
    @param pAssignable type of value to be assigned
    @param pFrom type of value
*/
SAL_DLLEXPORT sal_Bool SAL_CALL typelib_typedescriptionreference_isAssignableFrom(
    typelib_TypeDescriptionReference * pAssignable,
    typelib_TypeDescriptionReference * pFrom )
    SAL_THROW_EXTERN_C();

/** Gets static type reference of standard types by type class.
    ==OPTIMIZATION HACK==:
    provides Type com.sun.star.uno.Exception for typelib_TypeClass_EXCEPTION
    and com.sun.star.uno.XInterface for typelib_TypeClass_INTERFACE.
    <br>
    Thread synchronizes on typelib mutex.
    <br>
    @param eTypeClass type class of basic type
    @return pointer to type reference pointer
*/
SAL_DLLEXPORT typelib_TypeDescriptionReference ** SAL_CALL typelib_static_type_getByTypeClass(
    typelib_TypeClass eTypeClass )
    SAL_THROW_EXTERN_C();

/** Inits static type reference.
    Thread synchronizes on typelib init mutex.
    <br>
    @param ppRef pointer to type reference pointer
    @param eTypeClass type class of type
    @param pTypeName ascii name of type
*/
SAL_DLLEXPORT void SAL_CALL typelib_static_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const sal_Char * pTypeName )
    SAL_THROW_EXTERN_C();

/** Inits static sequence type reference.
    Thread synchronizes on typelib init mutex.
    <br>
    @param ppRef pointer to type reference pointer
    @param pElementType element type of sequence
*/
SAL_DLLEXPORT void SAL_CALL typelib_static_sequence_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeDescriptionReference * pElementType )
    SAL_THROW_EXTERN_C();

/** Inits <b>in</b>complete static compound type reference.
    Thread synchronizes on typelib init mutex.
    <br>
    @param ppRef pointer to type reference pointer
    @param eTypeClass typelib_TypeClass_STRUCT or typelib_TypeClass_EXCEPTION
    @param pTypeName name of type
    @param pBaseType base type
    @param nMembers number of members
    @param ppMembers member types
*/
SAL_DLLEXPORT void SAL_CALL typelib_static_compound_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType,
    sal_Int32 nMembers, typelib_TypeDescriptionReference ** ppMembers )
    SAL_THROW_EXTERN_C();

/** Inits <b>in</b>complete static interface type reference.
    Thread synchronizes on typelib init mutex.
    <br>
    @param ppRef pointer to type reference pointer
    @param pTypeName name of interface
    @param pBaseType base type
*/
SAL_DLLEXPORT void SAL_CALL typelib_static_interface_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType )
    SAL_THROW_EXTERN_C();

/** Inits <b>in</b>complete static enum type reference.
    Thread synchronizes on typelib init mutex.
    <br>
    @param ppRef pointer to type reference pointer
    @param pTypeName name of enum
    @param nDefaultEnumValue default enum value
*/
SAL_DLLEXPORT void SAL_CALL typelib_static_enum_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    sal_Int32 nDefaultValue )
    SAL_THROW_EXTERN_C();

/** Inits <b>in</b>complete static compound type reference.
    Thread synchronizes on typelib init mutex.
    All discriminants are handled as int64 values.
    The pDiscriminantTypeRef must be of type byte, short, ..., up to hyper.
    <br>
    @param ppRef pointer to type reference pointer
    @param pTypeName name of union type
    @param pDiscriminantType discriminant type
    @param nDefaultDiscriminant default discriminant
    @param pDefaultType default value type of union
    @param nMembers number of union members
    @param pDiscriminants member discriminants
    @param pMemberTypes member types
*/
SAL_DLLEXPORT void SAL_CALL typelib_static_union_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pDiscriminantType,
    sal_Int64 nDefaultDiscriminant,
    typelib_TypeDescriptionReference * pDefaultType,
    sal_Int32 nMembers,
    sal_Int64 * pDiscriminants,
    typelib_TypeDescriptionReference ** pMemberTypes )
    SAL_THROW_EXTERN_C();

/** Completes a typedescription to be used for, e.g., marshalling values.
    COMPOUND, UNION, INTERFACE and ENUM type descriptions may be partly
    initialized (see typelib_static_...(), typelib_TypeDescription::bComplete).
    For interface type descriptions, this will also init index tables.
    <br>
    @param ppTypeDescr [inout] type description to be completed (may be exchanged!)
    @return true, if type description is complete
*/
SAL_DLLEXPORT sal_Bool SAL_CALL typelib_typedescription_complete(
    typelib_TypeDescription ** ppTypeDescr )
    SAL_THROW_EXTERN_C();

#ifdef __cplusplus
}
#endif

#endif
