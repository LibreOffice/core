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

#ifndef OOX_HELPER_PROPERTYMAP_HXX
#define OOX_HELPER_PROPERTYMAP_HXX

#include <vector>
#include <map>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include "oox/token/properties.hxx"
#include "oox/dllapi.h"

namespace com { namespace sun { namespace star { namespace beans {
    struct PropertyValue;
    class XPropertySet;
} } } }

namespace oox {

struct PropertyNameVector;

// ============================================================================

typedef ::std::map< sal_Int32, ::com::sun::star::uno::Any > PropertyMapBase;

/** A helper that maps property identifiers to property values.

    The property identifiers are generated on compile time and refer to the
    property name strings that are held by a static vector. The identifier to
    name mapping is done internally while the properties are written to
    property sets.
 */
class OOX_DLLPUBLIC PropertyMap : public PropertyMapBase
{
public:
    PropertyMap();

    /** Returns the name of the passed property identifier. */
    static const OUString& getPropertyName( sal_Int32 nPropId );

    /** Returns true, if the map contains a property with the passed identifier. */
    bool                hasProperty( sal_Int32 nPropId ) const
                            { return find( nPropId ) != end(); }

    /** Sets the specified property to the passed value. Does nothing, if the
        identifier is invalid. */
    bool                setAnyProperty( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue )
                            { if( nPropId < 0 ) return false; (*this)[ nPropId ] = rValue; return true; }

    /** Sets the specified property to the passed value. Does nothing, if the
        identifier is invalid. */
    template< typename Type >
    bool                setProperty( sal_Int32 nPropId, const Type& rValue )
                            { if( nPropId < 0 ) return false; (*this)[ nPropId ] <<= rValue; return true; }

    /** Inserts all properties contained in the passed property map. */
    void                assignUsed( const PropertyMap& rPropMap )
                            { insert( rPropMap.begin(), rPropMap.end() ); }

    /** Inserts all properties contained in the passed property map */
    void                assignAll( const PropertyMap& rPropMap );

    /** Returns a sequence of property values, filled with all contained properties. */
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                        makePropertyValueSequence() const;

    /** Fills the passed sequences of names and anys with all contained properties. */
    void                fillSequences(
                            ::com::sun::star::uno::Sequence< OUString >& rNames,
                            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rValues ) const;

    /** Creates a property set supporting the XPropertySet interface and inserts all properties. */
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        makePropertySet() const;

#if OSL_DEBUG_LEVEL > 0
#ifdef DBG_UTIL
  static void dump( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet);
#endif
  static void dumpCode( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rXPropSet);
  void dumpCode();
#endif
private:
    const PropertyNameVector* mpPropNames;
};

// ============================================================================

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
