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
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#define _COM_SUN_STAR_UNO_REFERENCE_H_

#include <rtl/alloc.h>


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

class RuntimeException;
class XInterface;
class Type;
class Any;

/** Enum defining UNO_REF_NO_ACQUIRE for setting reference without acquiring a given interface.
    Deprecated, please use SAL_NO_ACQUIRE.
    @deprecated
*/
enum UnoReference_NoAcquire
{
    /** This enum value can be used for creating a reference granting a given interface,
        i.e. transferring ownership to it.
    */
    UNO_REF_NO_ACQUIRE
};

/** This base class serves as a base class for all template reference classes and
    has been introduced due to compiler problems with templated operators ==, =!.
*/
class BaseReference
{
protected:
    /** the interface pointer
    */
    XInterface * _pInterface;

    /** Queries given interface for type rType.

        @param pInterface interface pointer
        @param rType interface type
        @return interface of demanded type (may be null)
    */
    inline static XInterface * SAL_CALL iquery( XInterface * pInterface, const Type & rType )
        SAL_THROW( (RuntimeException) );
#ifndef EXCEPTIONS_OFF
    /** Queries given interface for type rType.
        Throws a RuntimeException if the demanded interface cannot be queried.

        @param pInterface interface pointer
        @param rType interface type
        @return interface of demanded type
    */
    inline static XInterface * SAL_CALL iquery_throw( XInterface * pInterface, const Type & rType )
        SAL_THROW( (RuntimeException) );
#endif

public:
    /** Gets interface pointer. This call does not acquire the interface.

        @return UNacquired interface pointer
    */
    inline XInterface * SAL_CALL get() const SAL_THROW(())
        { return _pInterface; }

    /** Checks if reference is null.

        @return true if reference acquires an interface, i.e. true if it is not null
    */
    inline sal_Bool SAL_CALL is() const SAL_THROW(())
        { return (0 != _pInterface); }

    /** Equality operator: compares two interfaces
        Checks if both references are null or refer to the same object.

        @param pInterface another interface
        @return true if both references are null or refer to the same object, false otherwise
    */
    inline sal_Bool SAL_CALL operator == ( XInterface * pInterface ) const SAL_THROW(());
    /** Unequality operator: compares two interfaces
        Checks if both references are null or refer to the same object.

        @param pInterface another interface
        @return false if both references are null or refer to the same object, true otherwise
    */
    inline sal_Bool SAL_CALL operator != ( XInterface * pInterface ) const SAL_THROW(());

    /** Equality operator: compares two interfaces
        Checks if both references are null or refer to the same object.

        @param rRef another reference
        @return true if both references are null or refer to the same object, false otherwise
    */
    inline sal_Bool SAL_CALL operator == ( const BaseReference & rRef ) const SAL_THROW(());
    /** Unequality operator: compares two interfaces
        Checks if both references are null or refer to the same object.

        @param rRef another reference
        @return false if both references are null or refer to the same object, true otherwise
    */
    inline sal_Bool SAL_CALL operator != ( const BaseReference & rRef ) const SAL_THROW(());

    /** Needed by some STL containers.

        @param rRef another reference
        @return true, if this reference is less than rRef
    */
    inline sal_Bool SAL_CALL operator < ( const BaseReference & rRef ) const SAL_THROW(());
};

/** Enum defining UNO_QUERY for implicit interface query.
*/
enum UnoReference_Query
{
    /** This enum value can be used for implicit interface query.
    */
    UNO_QUERY,
};
#ifndef EXCEPTIONS_OFF
/** Enum defining UNO_QUERY_THROW for implicit interface query.
    If the demanded interface is unavailable, then a RuntimeException is thrown.
*/
enum UnoReference_QueryThrow
{
    /** This enum value can be used for implicit interface query.
    */
    UNO_QUERY_THROW,
};
/** Enum defining UNO_SET_THROW for throwing if attempts are made to assign a null
    interface

    @since UDK 3.2.8
*/
enum UnoReference_SetThrow
{
    UNO_SET_THROW
};
#endif

/** Template reference class for interface type derived from BaseReference.
    A special constructor given the UNO_QUERY identifier queries interfaces
    for reference type.
*/
template< class interface_type >
class Reference : public BaseReference
{
    /** Queries given interface for type interface_type.

        @param pInterface interface pointer
        @return interface of demanded type (may be null)
    */
    inline static XInterface * SAL_CALL iquery( XInterface * pInterface )
        SAL_THROW( (RuntimeException) );
#ifndef EXCEPTIONS_OFF
    /** Queries given interface for type interface_type.
        Throws a RuntimeException if the demanded interface cannot be queried.

        @param pInterface interface pointer
        @return interface of demanded type
    */
    inline static XInterface * SAL_CALL iquery_throw( XInterface * pInterface )
        SAL_THROW( (RuntimeException) );
    /** Returns the given interface if it is not <NULL/>, throws a RuntimeException otherwise.

        @param pInterface interface pointer
        @return pInterface
    */
    inline static interface_type * SAL_CALL iset_throw( interface_type * pInterface )
        SAL_THROW( (RuntimeException) );
#endif

    /** Cast from an "interface pointer" (e.g., BaseReference::_pInterface) to a
        pointer to this interface_type.

        To work around ambiguities in the case of multiple-inheritance interface
        types (which inherit XInterface more than once), use reinterpret_cast
        (resp. a sequence of two static_casts, to avoid warnings about
        reinterpret_cast used between related classes) to switch from a pointer
        to XInterface to a pointer to this derived interface_type.  In
        principle, this is not guaranteed to work.  In practice, it seems to
        work on all supported platforms.
    */
    static inline interface_type * castFromXInterface(XInterface * p) {
        return static_cast< interface_type * >(static_cast< void * >(p));
    }

    /** Cast from a pointer to this interface_type to an "interface pointer"
        (e.g., BaseReference::_pInterface).

        To work around ambiguities in the case of multiple-inheritance interface
        types (which inherit XInterface more than once), use reinterpret_cast
        (resp. a sequence of two static_casts, to avoid warnings about
        reinterpret_cast used between related classes) to switch from a pointer
        to this derived interface_type to a pointer to XInterface.  In
        principle, this is not guaranteed to work.  In practice, it seems to
        work on all supported platforms.
    */
    static inline XInterface * castToXInterface(interface_type * p) {
        return static_cast< XInterface * >(static_cast< void * >(p));
    }

public:
    /// @cond INTERNAL
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new ( ::size_t nSize ) SAL_THROW(())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete ( void * pMem ) SAL_THROW(())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new ( ::size_t, void * pMem ) SAL_THROW(())
        { return pMem; }
    inline static void SAL_CALL operator delete ( void *, void * ) SAL_THROW(())
        {}
    /// @endcond

    /** Destructor: Releases interface if set.
    */
    inline ~Reference() SAL_THROW(());

    /** Default Constructor: Sets null reference.
    */
    inline Reference() SAL_THROW(());

    /** Copy constructor: Copies interface reference.

        @param rRef another reference
    */
    inline Reference( const Reference< interface_type > & rRef ) SAL_THROW(());
    /** Constructor: Sets given interface pointer.

        @param pInterface an interface pointer
    */
    inline Reference( interface_type * pInterface ) SAL_THROW(());

    /** Constructor: Sets given interface pointer without acquiring it.

        @param pInterface another reference
        @param dummy SAL_NO_ACQUIRE to force obvious distinction to other constructors
    */
    inline Reference( interface_type * pInterface, __sal_NoAcquire dummy) SAL_THROW(());
    /** Constructor: Sets given interface pointer without acquiring it.
        Deprecated, please use SAL_NO_ACQUIRE version.

        @deprecated
        @param pInterface another reference
        @param dummy UNO_REF_NO_ACQUIRE to force obvious distinction to other constructors
    */
    inline Reference( interface_type * pInterface, UnoReference_NoAcquire dummy ) SAL_THROW(());

    /** Constructor: Queries given interface for reference interface type (interface_type).

        @param rRef another reference
        @param dummy UNO_QUERY to force obvious distinction to other constructors
    */
    inline Reference( const BaseReference & rRef, UnoReference_Query dummy ) SAL_THROW( (RuntimeException) );
    /** Constructor: Queries given interface for reference interface type (interface_type).

        @param pInterface an interface pointer
        @param dummy UNO_QUERY to force obvious distinction to other constructors
    */
    inline Reference( XInterface * pInterface, UnoReference_Query dummy) SAL_THROW( (RuntimeException) );
    /** Constructor: Queries given any for reference interface type (interface_type).

        @param rAny an any
        @param dummy UNO_QUERY to force obvious distinction to other constructors
    */
    inline Reference( const Any & rAny, UnoReference_Query dummy) SAL_THROW( (RuntimeException) );
#ifndef EXCEPTIONS_OFF
    /** Constructor: Queries given interface for reference interface type (interface_type).
        Throws a RuntimeException if the demanded interface cannot be queried.

        @param rRef another reference
        @param dummy UNO_QUERY_THROW to force obvious distinction
                     to other constructors
    */
    inline Reference( const BaseReference & rRef, UnoReference_QueryThrow dummy ) SAL_THROW( (RuntimeException) );
    /** Constructor: Queries given interface for reference interface type (interface_type).
        Throws a RuntimeException if the demanded interface cannot be queried.

        @param pInterface an interface pointer
        @param dummy UNO_QUERY_THROW to force obvious distinction
                     to other constructors
    */
    inline Reference( XInterface * pInterface, UnoReference_QueryThrow dummy ) SAL_THROW( (RuntimeException) );
    /** Constructor: Queries given any for reference interface type (interface_type).
        Throws a RuntimeException if the demanded interface cannot be queried.

        @param rAny an any
        @param dummy UNO_QUERY_THROW to force obvious distinction
                     to other constructors
    */
    inline Reference( const Any & rAny, UnoReference_QueryThrow dummy ) SAL_THROW( (RuntimeException) );
    /** Constructor: assigns from the given interface of the same type. Throws a RuntimeException
        if the source interface is NULL.

        @param rRef another interface reference of the same type
        @param dummy UNO_SET_THROW to distinguish from default copy constructor

        @since UDK 3.2.8
    */
    inline Reference( const Reference< interface_type > & rRef, UnoReference_SetThrow dummy ) SAL_THROW( (RuntimeException) );
    /** Constructor: assigns from the given interface of the same type. Throws a RuntimeException
        if the source interface is NULL.

        @param pInterface an interface pointer
        @param dummy UNO_SET_THROW to distinguish from default assignment constructor

        @since UDK 3.2.8
    */
    inline Reference( interface_type * pInterface, UnoReference_SetThrow dummy ) SAL_THROW( (RuntimeException) );
#endif

    /** Cast operator to Reference< XInterface >: Reference objects are binary compatible and
        any interface must be derived from com.sun.star.uno.XInterface.
        This a useful direct cast possibility.
    */
    inline SAL_CALL operator const Reference< XInterface > & () const SAL_THROW(())
        { return * reinterpret_cast< const Reference< XInterface > * >( this ); }

    /** Dereference operator: Used to call interface methods.

        @return UNacquired interface pointer
    */
    inline interface_type * SAL_CALL operator -> () const SAL_THROW(())
        { return castFromXInterface(_pInterface); }

    /** Gets interface pointer. This call does not acquire the interface.

        @return UNacquired interface pointer
    */
    inline interface_type * SAL_CALL get() const SAL_THROW(())
        { return castFromXInterface(_pInterface); }

    /** Clears reference, i.e. releases interface. Reference is null after clear() call.
    */
    inline void SAL_CALL clear() SAL_THROW(());

    /** Sets the given interface. An interface already set will be released.

        @param rRef another reference
        @return true, if non-null interface was set
    */
    inline sal_Bool SAL_CALL set( const Reference< interface_type > & rRef ) SAL_THROW(());
    /** Sets the given interface. An interface already set will be released.

        @param pInterface another interface
        @return true, if non-null interface was set
    */
    inline sal_Bool SAL_CALL set( interface_type * pInterface ) SAL_THROW(());

    /** Sets interface pointer without acquiring it. An interface already set will be released.

        @param pInterface an interface pointer
        @param dummy SAL_NO_ACQUIRE to force obvious distinction to set methods
        @return true, if non-null interface was set
    */
    inline sal_Bool SAL_CALL set( interface_type * pInterface, __sal_NoAcquire dummy) SAL_THROW(());
    /** Sets interface pointer without acquiring it. An interface already set will be released.
        Deprecated, please use SAL_NO_ACQUIRE version.

        @deprecated
        @param pInterface an interface pointer
        @param dummy UNO_REF_NO_ACQUIRE to force obvious distinction to set methods
        @return true, if non-null interface was set
    */
    inline sal_Bool SAL_CALL set( interface_type * pInterface, UnoReference_NoAcquire dummy) SAL_THROW(());

    /** Queries given interface for reference interface type (interface_type) and sets it.
        An interface already set will be released.

        @param pInterface an interface pointer
        @param dummy UNO_QUERY to force obvious distinction to set methods
        @return true, if non-null interface was set
    */
    inline sal_Bool SAL_CALL set( XInterface * pInterface, UnoReference_Query dummy ) SAL_THROW( (RuntimeException) );
    /** Queries given interface for reference interface type (interface_type) and sets it.
        An interface already set will be released.

        @param rRef another reference
        @param dummy UNO_QUERY to force obvious distinction to set methods
        @return true, if non-null interface was set
    */
    inline sal_Bool SAL_CALL set( const BaseReference & rRef, UnoReference_Query dummy) SAL_THROW( (RuntimeException) );

    /** Queries given any for reference interface type (interface_type)
        and sets it.  An interface already set will be released.

        @param rAny
               an Any containing an interface
        @param dummy
               UNO_QUERY to force obvious distinction
               to set methods
        @return
                true, if non-null interface was set
    */
    inline bool set( Any const & rAny, UnoReference_Query dummy );

#ifndef EXCEPTIONS_OFF
    /** Queries given interface for reference interface type (interface_type) and sets it.
        An interface already set will be released.
        Throws a RuntimeException if the demanded interface cannot be set.

        @param pInterface an interface pointer
        @param dummy UNO_QUERY_THROW to force obvious distinction
                     to set methods
    */
    inline void SAL_CALL set( XInterface * pInterface, UnoReference_QueryThrow dummy ) SAL_THROW( (RuntimeException) );
    /** Queries given interface for reference interface type (interface_type) and sets it.
        An interface already set will be released.
        Throws a RuntimeException if the demanded interface cannot be set.

        @param rRef another reference
        @param dummy UNO_QUERY_THROW to force obvious distinction
               to set methods
    */
    inline void SAL_CALL set( const BaseReference & rRef, UnoReference_QueryThrow dummy ) SAL_THROW( (RuntimeException) );

    /** Queries given any for reference interface type (interface_type) and
        sets it.  An interface already set will be released.
        Throws a RuntimeException if the demanded interface cannot be set.

        @param rAny
               an Any containing an interface
        @param dummy
               UNO_QUERY_THROW to force obvious distinction to set methods
    */
    inline void set( Any const & rAny, UnoReference_QueryThrow dummy);
    /** sets the given interface
        An interface already set will be released.
        Throws a RuntimeException if the source interface is @b NULL.

        @param pInterface an interface pointer
        @param dummy UNO_SET_THROW to force obvious distinction to other set methods

        @since UDK 3.2.8
    */
    inline void SAL_CALL set( interface_type * pInterface, UnoReference_SetThrow dummy) SAL_THROW( (RuntimeException) );
    /** sets the given interface
        An interface already set will be released.
        Throws a RuntimeException if the source interface is @b NULL.

        @param rRef an interface reference
        @param dummy UNO_SET_THROW to force obvious distinction to other set methods

        @since UDK 3.2.8
    */
    inline void SAL_CALL set( const Reference< interface_type > & rRef, UnoReference_SetThrow dummy) SAL_THROW( (RuntimeException) );

#endif

    /** Assignment operator: Acquires given interface pointer and sets reference.
        An interface already set will be released.

        @param pInterface an interface pointer
        @return this reference
    */
    inline Reference< interface_type > & SAL_CALL operator = ( interface_type * pInterface ) SAL_THROW(());
    /** Assignment operator: Acquires given interface reference and sets reference.
        An interface already set will be released.

        @param rRef an interface reference
        @return this reference
    */
    inline Reference< interface_type > & SAL_CALL operator = ( const Reference< interface_type > & rRef ) SAL_THROW(());

    /** Queries given interface reference for type interface_type.

        @param rRef interface reference
        @return interface reference of demanded type (may be null)
    */
    inline static Reference< interface_type > SAL_CALL query( const BaseReference & rRef ) SAL_THROW( (RuntimeException) );
    /** Queries given interface for type interface_type.

        @param pInterface interface pointer
        @return interface reference of demanded type (may be null)
    */
    inline static Reference< interface_type > SAL_CALL query( XInterface * pInterface ) SAL_THROW( (RuntimeException) );
};

/// @cond INTERNAL
/** Enables boost::mem_fn and boost::bind to recognize Reference.
*/
template <typename T>
inline T * get_pointer( Reference<T> const& r )
{
    return r.get();
}
/// @endcond

}
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
