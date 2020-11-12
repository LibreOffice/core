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

#ifndef INCLUDED_REGISTRY_TYPES_H
#define INCLUDED_REGISTRY_TYPES_H

#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>

/** specifies the typeclass of a binary type blob.

    The general structure of a binary type blob is always the same.  It depends
    on the typeclass which parts of the blob are filled with data or not.
 */
enum RTTypeClass
{
    /** specifies that the structure of the given blob is unknown and can't be
        read.
     */
    RT_TYPE_INVALID,

    /** specifies that the blob represents an interface type.  An interface blob
        can contain a base interface, attributes and methods.
     */
    RT_TYPE_INTERFACE,

    /** specifies that the blob represents a module type.  A module blob can
        contain a base module and constant members (fields).
     */
    RT_TYPE_MODULE,

    /** specifies that the blob represents a struct type.  A struct blob can
        contain a base struct and members (fields).
     */
    RT_TYPE_STRUCT,

    /** specifies that the blob represents an enum type.  An enum blob can
        contain enum values which are accessible as fields.
     */
    RT_TYPE_ENUM,

    /** specifies that the blob represents an exception type.  An exception blob
        can contain a base exception and members (fields).
     */
    RT_TYPE_EXCEPTION,

    /** specifies that the blob represents a typedef type.  A typedef blob can
        contain a base type.
     */
    RT_TYPE_TYPEDEF,

    /** specifies that the blob represents a service type.  A service blob can
        contain a base service, properties (fields), references to services or
        interfaces.
     */
    RT_TYPE_SERVICE,

    /** specifies that the blob represents a singleton type (a named object)
        which refers exactly one existing service.
     */
    RT_TYPE_SINGLETON,

    /// deprecated, not used.
    RT_TYPE_OBJECT,

    /** specifies that the blob represents a constants type.  A constants blob
        can contain constant types as fields.
     */
    RT_TYPE_CONSTANTS,

    /** @deprecated
        a union type was evaluated but currently not supported.
     */
    RT_TYPE_UNION,

    /// @cond INTERNAL
    /**
       Flag for published entities.

       Used in combination with RT_TYPE_INTERFACE, RT_TYPE_STRUCT, RT_TYPE_ENUM,
       RT_TYPE_EXCEPTION, RT_TYPE_TYPEDEF, RT_TYPE_SERVICE, RT_TYPE_SINGLETON,
       or RT_TYPE_CONSTANTS to mark an entity as published.

       (The value of this enumerator is chosen so that it is unlikely that its
       addition changes the underlying type of this enumeration for any C/C++
       compiler.)

       @since UDK 3.2.0
     */
    RT_TYPE_PUBLISHED = 0x4000
    /// @endcond
};

/** specifies the type for the field access.

    Fields in a type blob are used for different types.  Among others they were
    used for properties of services and these properties can have several flags.

    @see RTFieldAccess::INVALID
    @see RTFieldAccess::READONLY
    @see RTFieldAccess::OPTIONAL
    @see RTFieldAccess::MAYBEVOID
    @see RTFieldAccess::BOUND
    @see RTFieldAccess::CONSTRAINED
    @see RTFieldAccess::TRANSIENT
    @see RTFieldAccess::MAYBEAMBIGUOUS
    @see RTFieldAccess::MAYBEDEFAULT
    @see RTFieldAccess::REMOVABLE
    @see RTFieldAccess::ATTRIBUTE
    @see RTFieldAccess::PROPERTY
    @see RTFieldAccess::CONST
    @see RTFieldAccess::READWRITE
    @see RTFieldAccess::DEFAULT
    @see RTFieldAccess::PARAMETERIZED_TYPE
    @see RTFieldAccess::PUBLISHED
 */
enum class RTFieldAccess
{
    NONE = 0x0000,
    /// specifies an unknown flag
    INVALID = 0x0000,
    /// specifies a readonly property/attribute
    READONLY = 0x0001,
    /// specifies a property as optional that means that it must not be implemented.
    OPTIONAL = 0x0002,
    /// @see com::sun::star::beans::PropertyAttribute
    MAYBEVOID = 0x0004,
    /// @see com::sun::star::beans::PropertyAttribute
    BOUND = 0x0008,
    /// @see com::sun::star::beans::PropertyAttribute
    CONSTRAINED = 0x0010,
    /// @see com::sun::star::beans::PropertyAttribute
    TRANSIENT = 0x0020,
    /// @see com::sun::star::beans::PropertyAttribute
    MAYBEAMBIGUOUS = 0x0040,
    /// @see com::sun::star::beans::PropertyAttribute
    MAYBEDEFAULT = 0x0080,
    /// @see com::sun::star::beans::PropertyAttribute
    REMOVABLE = 0x0100,
    /// @see com::sun::star::beans::PropertyAttribute
    ATTRIBUTE = 0x0200,
    /// specifies that the field is a property
    PROPERTY = 0x0400,
    /// specifies that the field is a constant or enum value
    CONST = 0x0800,
    /// specifies that the property/attribute has read/write access
    READWRITE = 0x1000,
    /// only to describe a union default label
    DEFAULT = 0x2000,
    /**
       Indicates that a member of a polymorphic struct type template is of a
       parameterized type.

       Only valid for fields that represent members of polymorphic struct type
       templates.

       @since UDK 3.2.0
     */
    PARAMETERIZED_TYPE = 0x4000,
    /**
       Flag for published individual constants.

       Used in combination with RTFieldAccess::CONST for individual constants (which are
       not members of constant groups).

       @since UDK 3.2.0
     */
    PUBLISHED = 0x8000,

};
namespace o3tl
{
template <> struct typed_flags<RTFieldAccess> : is_typed_flags<RTFieldAccess, 0xffff>
{
};
}

/** specifies the type of a field value.

    A field can have a value if it represents a constant or an enum value.
 */
enum RTValueType
{
    RT_TYPE_NONE,
    RT_TYPE_BOOL,
    RT_TYPE_BYTE,
    RT_TYPE_INT16,
    RT_TYPE_UINT16,
    RT_TYPE_INT32,
    RT_TYPE_UINT32,
    RT_TYPE_INT64,
    RT_TYPE_UINT64,
    RT_TYPE_FLOAT,
    RT_TYPE_DOUBLE,
    RT_TYPE_STRING
};

/** specifies a variable container for field values.
 */
union RTConstValueUnion {
    bool aBool;
    sal_Int8 aByte;
    sal_Int16 aShort;
    sal_uInt16 aUShort;
    sal_Int32 aLong;
    sal_uInt32 aULong;
    sal_Int64 aHyper;
    sal_uInt64 aUHyper;
    float aFloat;
    double aDouble;
    sal_Unicode const* aString;
};

/** specifies the mode of a method.

    A method can be synchron or asynchron (oneway).  The const attribute for
    methods was removed so that the const values are deprecated.
 */
enum class RTMethodMode
{
    /// indicates an invalid mode
    INVALID,

    /// indicates the asynchronous mode of a method
    ONEWAY,

    /// @deprecated
    ONEWAY_CONST,

    /// indicated the synchronous mode of a method
    TWOWAY,

    /// @deprecated
    TWOWAY_CONST,

    /**
       Indicates an extended attribute getter (that has a 'raises' clause) of an
       interface type.

       @since UDK 3.2.0
     */
    ATTRIBUTE_GET,

    /**
       Indicates an extended attribute setter (that has a 'raises' clause) of an
       interface type.

       @since UDK 3.2.0
     */
    ATTRIBUTE_SET
};

/** specifies the mode of a parameter.

    There are three parameter modes which have impact of the handling of the
    parameter in the UNO bridges and the UNO code generation.
 */
enum RTParamMode
{
    /// indicates an invalid parameter mode
    RT_PARAM_INVALID = 0,

    /// indicates a pure in parameter which is used by value
    RT_PARAM_IN = 1,

    /// indicates a pure out parameter which is used by reference
    RT_PARAM_OUT = 2,

    /// indicates a in and out parameter which is used also by reference
    RT_PARAM_INOUT = 3,

    /**
       Indicates a rest parameter (currently only valid for service
       constructors).

       This value can be combined with any of RT_PARAM_IN, RT_PARAM_OUT, and
       RT_PARAM_INOUT (however, service constructors currently only allow
       RT_PARAM_IN, anyway).

       @since UDK 3.2.0
     */
    RT_PARAM_REST = 4
};

/** specifies the type of a reference used in a service description.
 */
enum class RTReferenceType
{
    /// the reference type is unknown
    INVALID,

    /** the service support the interface that means an implementation of this
        service must implement this interface.
     */
    SUPPORTS,

    /** @deprecated
        the service observes the interface.
     */
    OBSERVES,

    /** the service exports the specified service that means this service
        provides also the specified service.
     */
    EXPORTS,

    /** @deprecated
        the service needs the specified service that means in the context of
        this service the specified service will be used or must be available.
     */
    NEEDS,

    /**
       Indicates a type parameter of a polymorphic struct type template.

       @since UDK 3.2.0
     */
    TYPE_PARAMETER
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
