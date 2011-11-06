/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
