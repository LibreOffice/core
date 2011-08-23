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

#ifndef OOX_HELPER_PROPERTYSET_HXX
#define OOX_HELPER_PROPERTYSET_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>

namespace oox {

class PropertyMap;

// ============================================================================

/** A wrapper for a UNO property set.

    This class provides functions to silently get and set properties (without
    exceptions, without the need to check validity of the UNO property set).

    An instance is constructed with the reference to a UNO property set or any
    other interface (the constructor will query for the
    com.sun.star.beans.XPropertySet interface then). The reference to the
    property set will be kept as long as the instance of this class is alive.

    The functions getProperties() and setProperties() try to handle all passed
    values at once, using the com.sun.star.beans.XMultiPropertySet interface.
    If the implementation does not support the XMultiPropertySet interface, all
    properties are handled separately in a loop.
 */
class PropertySet
{
public:
    inline explicit     PropertySet() {}

    /** Constructs a property set wrapper with the passed UNO property set. */
    inline explicit     PropertySet(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rxPropSet )
                                { set( rxPropSet ); }

    /** Constructs a property set wrapper after querying the XPropertySet interface. */
    template< typename Type >
    inline explicit     PropertySet( const Type& rObject ) { set( rObject ); }

    /** Sets the passed UNO property set and releases the old UNO property set. */
    void                set( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rxPropSet );

    /** Queries the passed object (interface or any) for an XPropertySet and releases the old UNO property set. */
    template< typename Type >
    inline void         set( const Type& rObject )
                            { set( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >( rObject, ::com::sun::star::uno::UNO_QUERY ) ); }

    /** Returns true, if the contained XPropertySet interface is valid. */
    inline bool         is() const { return mxPropSet.is(); }

    /** Returns the contained XPropertySet interface. */
    inline ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        getXPropertySet() const { return mxPropSet; }

    // Get properties ---------------------------------------------------------

    /** Gets the specified property from the property set.
        @return  true, if the any could be filled with the property value. */
    bool                getAnyProperty( ::com::sun::star::uno::Any& orValue, sal_Int32 nPropId ) const;

    /** Gets the specified property from the property set.
        @return  true, if the passed variable could be filled with the property value. */
    template< typename Type >
    inline bool         getProperty( Type& orValue, sal_Int32 nPropId ) const;

    /** Gets the specified property from the property set.
        @return  the property value, or an empty Any, if the property is missing. */
    ::com::sun::star::uno::Any getAnyProperty( sal_Int32 nPropId ) const;

    /** Gets the specified boolean property from the property set.
        @return  true = property contains true; false = property contains false or error occured. */
    bool                getBoolProperty( sal_Int32 nPropId ) const;

    /** Gets the specified properties from the property set. Tries to use the XMultiPropertySet interface.
        @param orValues  (out-parameter) The related property values.
        @param rPropNames  The property names. MUST be ordered alphabetically. */
    void                getProperties(
                            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& orValues,
                            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rPropNames ) const;

    // Set properties ---------------------------------------------------------

    /** Puts the passed any into the property set. */
    void                setAnyProperty( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue );

    /** Puts the passed value into the property set. */
    template< typename Type >
    inline void         setProperty( sal_Int32 nPropId, const Type& rValue );

    /** Puts the passed properties into the property set. Tries to use the XMultiPropertySet interface.
        @param rPropNames  The property names. MUST be ordered alphabetically.
        @param rValues  The related property values. */
    void                setProperties(
                            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rPropNames,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rValues );

    /** Puts the passed property map into the property set. Tries to use the XMultiPropertySet interface.
        @param rPropertyMap  The property map. */
    void                setProperties( const PropertyMap& rPropertyMap );

#if OSL_DEBUG_LEVEL > 0
    void dump();
#endif

    // ------------------------------------------------------------------------
private:
    /** Gets the specified property from the property set.
        @return  true, if the any could be filled with the property value. */
    bool                getAnyProperty( ::com::sun::star::uno::Any& orValue, const ::rtl::OUString& rPropName ) const;

    /** Puts the passed any into the property set. */
    void                setAnyProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rValue );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        mxPropSet;          /// The mandatory property set interface.
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet >
                        mxMultiPropSet;     /// The optional multi property set interface.
};

// ----------------------------------------------------------------------------

template< typename Type >
inline bool PropertySet::getProperty( Type& orValue, sal_Int32 nPropId ) const
{
    ::com::sun::star::uno::Any aAny;
    return getAnyProperty( aAny, nPropId ) && (aAny >>= orValue);
}

template< typename Type >
inline void PropertySet::setProperty( sal_Int32 nPropId, const Type& rValue )
{
    setAnyProperty( nPropId, ::com::sun::star::uno::Any( rValue ) );
}

// ============================================================================

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
