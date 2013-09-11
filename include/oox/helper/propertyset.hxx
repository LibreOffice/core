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

#ifndef OOX_HELPER_PROPERTYSET_HXX
#define OOX_HELPER_PROPERTYSET_HXX

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include "oox/dllapi.h"

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

    The functions setProperties() tries to handle all passed values at once,
    using the com.sun.star.beans.XMultiPropertySet interface.  If the
    implementation does not support the XMultiPropertySet interface, all
    properties are handled separately in a loop.
 */
class OOX_DLLPUBLIC PropertySet
{
public:
    PropertySet() {}

    /** Constructs a property set wrapper with the passed UNO property set. */
    explicit     PropertySet(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rxPropSet )
                                { set( rxPropSet ); }

    /** Constructs a property set wrapper after querying the XPropertySet interface. */
    template< typename Type >
    explicit     PropertySet( const Type& rObject ) { set( rObject ); }

    /** Sets the passed UNO property set and releases the old UNO property set. */
    void                set( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rxPropSet );

    /** Queries the passed object (interface or any) for an XPropertySet and releases the old UNO property set. */
    template< typename Type >
    void         set( const Type& rObject )
                            { set( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >( rObject, ::com::sun::star::uno::UNO_QUERY ) ); }

    /** Returns true, if the contained XPropertySet interface is valid. */
    bool         is() const { return mxPropSet.is(); }

    /** Returns the contained XPropertySet interface. */
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        getXPropertySet() const { return mxPropSet; }

    /** Returns true, if the specified property is supported by the property set. */
    bool                hasProperty( sal_Int32 nPropId ) const;

    // Get properties ---------------------------------------------------------

    /** Gets the specified property from the property set.
        @return  the property value, or an empty Any, if the property is missing. */
    ::com::sun::star::uno::Any getAnyProperty( sal_Int32 nPropId ) const;

    /** Gets the specified property from the property set.
        @return  true, if the passed variable could be filled with the property value. */
    template< typename Type >
    bool         getProperty( Type& orValue, sal_Int32 nPropId ) const
                            { return getAnyProperty( nPropId ) >>= orValue; }

    /** Gets the specified boolean property from the property set.
        @return  true = property contains true; false = property contains false or error occurred. */
    bool         getBoolProperty( sal_Int32 nPropId ) const
                            { bool bValue = false; return getProperty( bValue, nPropId ) && bValue; }
    // Set properties ---------------------------------------------------------

    /** Puts the passed any into the property set. */
    bool                setAnyProperty( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue );

    /** Puts the passed value into the property set. */
    template< typename Type >
    bool         setProperty( sal_Int32 nPropId, const Type& rValue )
                            { return setAnyProperty( nPropId, ::com::sun::star::uno::Any( rValue ) ); }

    /** Puts the passed properties into the property set. Tries to use the XMultiPropertySet interface.
        @param rPropNames  The property names. MUST be ordered alphabetically.
        @param rValues  The related property values. */
    void                setProperties(
                            const ::com::sun::star::uno::Sequence< OUString >& rPropNames,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rValues );

    /** Puts the passed property map into the property set. Tries to use the XMultiPropertySet interface.
        @param rPropertyMap  The property map. */
    void                setProperties( const PropertyMap& rPropertyMap );

#ifdef DBG_UTIL
    void dump();
#endif

    // ------------------------------------------------------------------------
private:
    /** Gets the specified property from the property set.
        @return  true, if the any could be filled with the property value. */
    bool                implGetPropertyValue( ::com::sun::star::uno::Any& orValue, const OUString& rPropName ) const;

    /** Puts the passed any into the property set. */
    bool                implSetPropertyValue( const OUString& rPropName, const ::com::sun::star::uno::Any& rValue );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        mxPropSet;          ///< The mandatory property set interface.
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet >
                        mxMultiPropSet;     ///< The optional multi property set interface.
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                        mxPropSetInfo;      ///< Property information.
};

// ============================================================================

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
