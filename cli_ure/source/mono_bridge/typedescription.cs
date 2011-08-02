/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

namespace uno.Typelib {

using System;
using System.Runtime.InteropServices;

/** Holds a weak reference to a type description.
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct TypeDescriptionReference
{
    /** reference count of type; don't ever modify this by yourself, use
        typedescriptionreference_acquire() and typedescriptionreference_release()
    */
    public int                             nRefCount;
    /** number of static references of type, because of the fact that some types are needed
        until program termination and are commonly held static.
    */
    public int                             nStaticRefCount;
    /** type class of type
    */
    public int                             eTypeClass;
    /** fully qualified name of type.
    */
    public uno.rtl.UString *               pTypeName;
    /** pointer to full typedescription; this value is only valid if the type is never swapped out
    */
    public TypeDescription *               pType;
    /** pointer to optimize the runtime; not for public use
    */
    public void *                          pUniqueIdentifier;
    /** reserved for future use; 0 if not used
    */
    public void *                          pReserved;

    [ DllImport("uno_cppu", EntryPoint="typelib_typedescriptionreference_acquire") ]
    public static extern void Acquire(/* TypeDescriptionReference */ void *td);

    [ DllImport("uno_cppu", EntryPoint="typelib_typedescriptionreference_release") ]
    public static extern void Release(/* TypeDescriptionReference */ void *td);

    [ DllImport("uno_cppu", EntryPoint="typelib_typedescriptionreference_new") ]
    public static extern void New(TypeDescriptionReference **ppTDR,
                                  int /* enum typelib_TypeClass */ eTypeClass,
                                  /* uno.rtl.UString */ void *pTypeName);

    [ DllImport("uno_cppu", EntryPoint="typelib_static_type_getByTypeClass") ]
    public static extern TypeDescriptionReference **GetByTypeClass(
        int /* enum typelib_TypeClass */ eTypeClass);

    [ DllImport("uno_cppu", EntryPoint="typelib_typedescriptionreference_getDescription") ]
    public static extern void GetDescription(
        TypeDescription ** ppRet, /* TypeDescriptionReference */ void * pRef);
}

/** Full type description of a type. Memory layout of this struct is identical to the
    TypeDescriptionReference for the first six members.
    So a typedescription can be used as type reference.
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct TypeDescription
{
    /** reference count; don't ever modify this by yourself, use
        typedescription_acquire() and typedescription_release()
    */
    public int                             nRefCount;
    /** number of static references of type, because of the fact that some types are needed
        until program termination and are commonly held static.
    */
    public int                             nStaticRefCount;
    /** type class of type
    */
    public int                             eTypeClass;
    /** fully qualified name of type.
    */
    public uno.rtl.UString *               pTypeName;
    /** pointer to self to distinguish reference from description; for internal use only
    */
    public TypeDescription *               pSelf;
    /** pointer to optimize the runtime; not for public use
    */
    public void *                          pUniqueIdentifier;
    /** reserved for future use; 0 if not used
    */
    public void *                          pReserved;

    /** flag to determine whether the description is complete:
        compound and union types lack of member names, enums lack of member types and names,
        interfaces lack of members and table init.
        Call typedescription_complete() if false.
    */
    public byte                            bComplete;
    /** size of type
    */
    public int                             nSize;
    /** alignment of type
    */
    public int                             nAlignment;
    /** pointer to weak reference
    */
    public TypeDescriptionReference *      pWeakRef;
    /** determines, if type can be unloaded (and it is possible to reloaded it)
    */
    public byte                            bOnDemand;

    /* FIXME move to TypeDescriptionReference */
    public static TypeDescriptionReference *VoidType
    {
        get { return null; /* FIXME, use typelib_static_type_getByTypeClass(VOID) */ }
    }

    [ DllImport("uno_cppu", EntryPoint="typelib_typedescription_getByName") ]
    public static extern void GetByName(TypeDescription **pTD,
                                        /* uno.rtl.UString */ void *name);

    [ DllImport("uno_cppu", EntryPoint="typelib_typedescription_acquire") ]
    public static extern void Acquire(/* TypeDescription */ void *td);

    [ DllImport("uno_cppu", EntryPoint="typelib_typedescription_release") ]
    public static extern void Release(/* TypeDescription */ void *td);

    [ DllImport("uno_cppu", EntryPoint="typelib_typedescription_complete") ]
    public static extern bool Complete(TypeDescription **pTD);
}

/** Type description for exception types.
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct CompoundTypeDescription
{
    /** inherits all members of TypeDescription
    */
    public TypeDescription                 aBase;

    /** pointer to base type description, else 0
    */
    public CompoundTypeDescription *       pBaseTypeDescription;

    /** number of members
    */
    public int                             nMembers;
    /** byte offsets of each member including the size the base type
    */
    public int *                           pMemberOffsets;
    /** members of the struct or exception
    */
    public TypeDescriptionReference **     ppTypeRefs;
    /** member names of the struct or exception.
    */
    public uno.rtl.UString **              ppMemberNames;
}

/**
   Type description for struct types.

   This is only used to represent plain struct types and instantiated
   polymorphic struct types; there is no representation of polymorphic struct
   type templates at this level.

   @since UDK 3.2.0
 */
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct StructTypeDescription
{
    /**
       Derived from CompoundTypeDescription.
     */
    public CompoundTypeDescription         aBase;

    /**
       Flags for direct members, specifying whether they are of parameterized
       type (true) or explict type (false).

       For a plain struct type, this is a null pointer.
     */
    public byte *                          pParameterizedTypes;
}

/** Type description of a union. The type class of this description is TypeClass_UNION.
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct UnionTypeDescription
{
    /** inherits all members of TypeDescription
    */
    public TypeDescription                 aBase;

    /** type of the discriminant
    */
    public TypeDescriptionReference *      pDiscriminantTypeRef;

    /** union default descriminant
    */
    public long                            nDefaultDiscriminant;
    /** union default member type (may be 0)
     */
    public TypeDescriptionReference *      pDefaultTypeRef;
    /** number of union member types
    */
    public int                             nMembers;
    /** union member discriminant values (same order as idl declaration)
    */
    public long *                          pDiscriminants;
    /** union member value types (same order as idl declaration)
    */
    public TypeDescriptionReference **     ppTypeRefs;
    /** union member value names (same order as idl declaration)
    */
    public uno.rtl.UString **              ppMemberNames;
    /** union value offset for data access
    */
    public int                             nValueOffset;
}

/** Type description of an array or sequence.
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct IndirectTypeDescription
{
    /** inherits all members of TypeDescription
    */
    public TypeDescription                 aBase;

    /** array, sequence: pointer to element type
    */
    public TypeDescriptionReference *      pType;
}

/** Type description of an array.
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct ArrayTypeDescription
{
    /** inherits all members of IndirectTypeDescription
    */
    public IndirectTypeDescription         aBase;

    /** number of dimensions
    */
    public int                             nDimensions;
    /** number of total array elements
    */
    public int                             nTotalElements;
    /** array of dimensions
    */
    public int *                           pDimensions;
}

/** Type description of an enum. The type class of this description is TypeClass_ENUM.
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct EnumTypeDescription
{
    /** inherits all members of TypeDescription
    */
    public TypeDescription                 aBase;

    /** first value of the enum
    */
    public int                             nDefaultEnumValue;
    /** number of enum values
    */
    public int                             nEnumValues;
    /** names of enum values
    */
    public uno.rtl.UString **              ppEnumNames;
    /** values of enum (corresponding to names in similar order)
    */
    public int *                           pEnumValues;
}

/** Description of an interface method parameter.
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct MethodParameter
{
    /** name of parameter
    */
    public uno.rtl.UString *               pName;
    /** type of parameter
    */
    public TypeDescriptionReference *      pTypeRef;
    /** true: the call type of this parameter is [in] or [inout]
        false: the call type of this parameter is [out]
    */
    public byte                            bIn;
    /** true: the call type of this parameter is [out] or [inout]
        false: the call type of this parameter is [in]
    */
    public byte                            bOut;
}

/** Common base type description of InterfaceMethodTypeDescription and
    InterfaceAttributeTypeDescription.
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct InterfaceMemberTypeDescription
{
    /** inherits all members of TypeDescription
    */
    public TypeDescription                 aBase;

    /** position of member in the interface including the number of members of
        any base interfaces
    */
    public int                             nPosition;
    /** name of member
    */
    public uno.rtl.UString *               pMemberName;
}

/** Type description of an interface method. The type class of this description is
    TypeClass_INTERFACE_METHOD. The size and the alignment are 0.
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct InterfaceMethodTypeDescription
{
    /** inherits all members of InterfaceMemberTypeDescription
    */
    public InterfaceMemberTypeDescription  aBase;

    /** type of the return value
    */
    public TypeDescriptionReference *      pReturnTypeRef;
    /** number of parameters
    */
    public int                             nParams;
    /** array of parameters
    */
    public MethodParameter *               pParams;
    /** number of exceptions
    */
    public int                             nExceptions;
    /** array of exception types
    */
    public TypeDescriptionReference **     ppExceptions;
    /** determines whether method is declared oneway
    */
    public byte                            bOneWay;

    /** the interface description this method is a member of

        @since #i21150#
    */
    public InterfaceTypeDescription *      pInterface;
    /** the inherited direct base method (null for a method that is not
        inherited)

        @since UDK 3.2.0
    */
    public TypeDescriptionReference *      pBaseRef;
    /** if pBaseRef is null, the member position of this method within
        pInterface, not counting members inherited from bases; if pBaseRef is
        not null, the index of the direct base within pInterface from which this
        method is inherited

        @since UDK 3.2.0
    */
    public int                             nIndex;
}

/** The description of an interface attribute. The type class of this description is
    TypeClass_INTERFACE_ATTRIBUTE. The size and the alignment are 0.
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct InterfaceAttributeTypeDescription
{
    /** inherits all members of InterfaceMemberTypeDescription
    */
    public InterfaceMemberTypeDescription  aBase;

    /** determines whether attribute is read only
    */
    public byte                            bReadOnly;
    /** type of the attribute
    */
    public TypeDescriptionReference *      pAttributeTypeRef;

    /** the interface description this attribute is a member of

        @since #i21150#
    */
    public InterfaceTypeDescription *      pInterface;
    /** the inherited direct base attribute (null for an attribute that is not
        inherited)

        @since UDK 3.2.0
    */
    public TypeDescriptionReference *      pBaseRef;
    /** if pBaseRef is null, the member position of this attribute within
        pInterface, not counting members inherited from bases; if pBaseRef is
        not null, the index of the direct base within pInterface from which this
        attribute is inherited

        @since UDK 3.2.0
    */
    public int                             nIndex;
    /** number of getter exceptions

        @since UDK 3.2.0
    */
    public int                             nGetExceptions;
    /** array of getter exception types

        @since UDK 3.2.0
    */
    public TypeDescriptionReference **     ppGetExceptions;
    /** number of setter exceptions

        @since UDK 3.2.0
    */
    public int                             nSetExceptions;
    /** array of setter exception types

        @since UDK 3.2.0
    */
    public TypeDescriptionReference **     ppSetExceptions;
}

/// @HTML
/** Type description of an interface.

    <p>Not all members are always initialized (not yet initialized members being
    null); there are three levels:</p>
    <ul>
        <li>Minimally, only <code>aBase</code>,
        <code>pBaseTypeDescription</code>, <code>aUik</code>,
        <code>nBaseTypes</code>, and <code>ppBaseTypes</code> are initialized;
        <code>aBase.bComplete</code> is false.  This only happens when an
        interface type description is created with
        <code>static_mi_interface_type_init</code> or
        <code>static_interface_type_init</code>.</li>

        <li>At the next level, <code>nMembers</code>, <code>ppMembers</code>,
        <code>nAllMembers</code>, <code>ppAllMembers</code> are also
        initialized; <code>aBase.bComplete</code> is still false.  This happens
        when an interface type description is created with
        <code>typedescription_newMIInterface</cocde> or
        <code>typedescription_newInterface</code>.</li>

        <li>At the final level, <code>pMapMemberIndexToFunctionIndex</code>,
        <code>nMapFunctionIndexToMemberIndex</code>, and
        <code>pMapFunctionIndexToMemberIndex</code> are also initialized;
        <code>aBase.bComplete</code> is true.  This happens after a call to
        <code>typedescription_complete</code>.</li>
    </ul>
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct InterfaceTypeDescription
/// @NOHTML
{
    /** inherits all members of TypeDescription
    */
    public TypeDescription                 aBase;

    /** pointer to base type description, else 0

        @deprecated
        use nBaseTypes and ppBaseTypes instead
    */
    public InterfaceTypeDescription *      pBaseTypeDescription;
    /** unique identifier of interface
    */
    public Uik                             aUik;
    /** number of members
    */
    public int                             nMembers;
    /** array of members; references attributes or methods
    */
    public TypeDescriptionReference **     ppMembers;
    /** number of members including members of base interface
    */
    public int                             nAllMembers;
    /** array of members including members of base interface; references attributes or methods
    */
    public TypeDescriptionReference **     ppAllMembers;
    /** array mapping index of the member description to an index doubling for read-write
        attributes (called function index); size of array is nAllMembers
    */
    public int *                           pMapMemberIndexToFunctionIndex;
    /** number of members plus number of read-write attributes
    */
    public int                             nMapFunctionIndexToMemberIndex;
    /** array mapping function index to member index; size of arry is nMapFunctionIndexToMemberIndex
    */
    public int *                           pMapFunctionIndexToMemberIndex;
    /** number of base types

        @since UDK 3.2.0
    */
    public int                             nBaseTypes;
    /** array of base type descriptions

        @since UDK 3.2.0
    */
    public InterfaceTypeDescription **     ppBaseTypes;

    [ DllImport("uno_cppu", EntryPoint="typelib_typedescription_release") ]
    public static extern void Release(/* InterfaceTypeDescription */ void *td);

    [ DllImport("uno_cppu", EntryPoint="typelib_typedescription_acquire") ]
    public static extern void Acquire(/* InterfaceTypeDescription */ void *td);

    [ DllImport("uno_cppu", EntryPoint="typelib_typedescription_equals") ]
    public static extern bool Equal(/* InterfaceTypeDescription */ void *td1, /* InterfaceTypeDescription */ void *td2);
}

/** Init struct of compound members for typedescription_new().
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct CompoundMember_Init
{
    /** type class of compound member
    */
    public int                             eTypeClass;
    /** name of type of compound member

        For a member of an instantiated polymorphic struct type that is of
        parameterized type, this will be a null pointer.
    */
    public uno.rtl.UString *               pTypeName;
    /** name of compound member
    */
    public uno.rtl.UString *               pMemberName;
}

/**
   Init struct of members for typedescription_newStruct().

   @since UDK 3.2.0
 */
[ StructLayout(LayoutKind.Sequential) ]
public struct StructMember_Init
{
    /**
       Derived from CompoundMember_Init;
     */
    public CompoundMember_Init             aBase;

    /**
       Flag specifying whether the member is of parameterized type (true) or
       explict type (false).
     */
    public byte                            bParameterizedType;
}

/** Init struct of interface methods for typedescription_new().
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct Parameter_Init
{
    /** type class of parameter
    */
    public int                             eTypeClass;
    /** name of parameter
    */
    public uno.rtl.UString *               pTypeName;
    /** name of parameter
    */
    public uno.rtl.UString *               pParamName;
    /** true, if parameter is [in] or [inout]
    */
    public byte                            bIn;
    /** true, if parameter is [out] or [inout]
    */
    public byte                            bOut;
}

/** Init struct of union types for typedescription_newUnion().
*/
[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct Union_Init
{
    /** union member discriminant
    */
    public long                            nDiscriminant;
    /** union member name
    */
    public uno.rtl.UString *               pMemberName;
    /** union member type
    */
    public TypeDescriptionReference *      pTypeRef;
}

}
