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

#ifndef OOX_HELPER_PROPERTYMAP_HXX
#define OOX_HELPER_PROPERTYMAP_HXX

#include <vector>
#include <map>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include "oox/token/properties.hxx"

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
class PropertyMap : public PropertyMapBase
{
public:
    explicit            PropertyMap();

    /** Returns the name of the passed property identifier. */
    static const ::rtl::OUString& getPropertyName( sal_Int32 nPropId );

    /** Returns true, if the map contains a property with the passed identifier. */
    inline bool         hasProperty( sal_Int32 nPropId ) const
                            { return find( nPropId ) != end(); }

    /** Returns the property value of the specified property, or 0 if not found. */
    const ::com::sun::star::uno::Any* getProperty( sal_Int32 nPropId ) const;

    /** Sets the specified property to the passed value. Does nothing, if the
        identifier is invalid. */
    inline bool         setAnyProperty( sal_Int32 nPropId, const ::com::sun::star::uno::Any& rValue )
                            { if( nPropId < 0 ) return false; (*this)[ nPropId ] = rValue; return true; }

    /** Sets the specified property to the passed value. Does nothing, if the
        identifier is invalid. */
    template< typename Type >
    inline bool         setProperty( sal_Int32 nPropId, const Type& rValue )
                            { if( nPropId < 0 ) return false; (*this)[ nPropId ] <<= rValue; return true; }

    /** Inserts all properties contained in the passed property map. */
    inline void         assignUsed( const PropertyMap& rPropMap )
                            { insert( rPropMap.begin(), rPropMap.end() ); }

    /** Returns a sequence of property values, filled with all contained properties. */
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                        makePropertyValueSequence() const;

    /** Fills the passed sequences of names and anys with all contained properties. */
    void                fillSequences(
                            ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames,
                            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rValues ) const;

    /** Creates a property set supporting the XPropertySet interface and inserts all properties. */
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                        makePropertySet() const;

private:
    const PropertyNameVector* mpPropNames;
};

// ============================================================================

} // namespace oox

#endif
