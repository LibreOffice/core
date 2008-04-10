/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertysequence.hxx,v $
 * $Revision: 1.3 $
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

#ifndef OOX_HELPER_PROPERTYSEQUENCE_HXX
#define OOX_HELPER_PROPERTYSEQUENCE_HXX

#include <vector>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }

namespace oox {

class PropertySet;

// ============================================================================

/** Generic helper class for reading multiple properties from and writing
    multiple properties to UNO property sets.

    Usage:
    1)  Call the constructor with a null-terminated array of ASCII strings. The
        array is not required to be ordered alphabetically. Sorting will be
        handled internally.

    2a) To READ properties from a property set: Call the readFromPropertySet()
        function or use the operator<< stream operator with the property set,
        then get the properties with the readValue() functions or the
        operator>> stream operator. The properties are returned in order of the
        array of property names passed in the constructor.

    2b) To WRITE properties to a property set: Set the values with the
        writeValue() functions or the operator<< stream operator. The order of
        the properties is equal to the array of property names passed in the
        constructor. Finally, call the writeToPropertySet() function or use the
        operator>> stream operator passing the property set.
 */
class PropertySequence
{
public:
    /** Constructs a new property sequence object from the passed property names.

        @param ppcPropNames  A null-terminated array of ASCII property names.
        @param ppcPropNames2  An optional array of additional property names.
        @param ppcPropNames3  An optional array of additional property names.
     */
    explicit            PropertySequence(
                            const sal_Char* const* ppcPropNames,
                            const sal_Char* const* ppcPropNames2 = 0,
                            const sal_Char* const* ppcPropNames3 = 0 );

    /** Makes all anys of type void by calling their clear() function. */
    void                clearAllAnys();

    // read properties --------------------------------------------------------

    /** Reads all values from the passed property set. */
    void                readFromPropertySet( const PropertySet& rPropSet );

    /** Reads the next value from the value sequence. */
    template< typename Type >
    bool                readValue( Type& rValue );

    /** Reads an any from the value sequence. */
    bool                readValue( ::com::sun::star::uno::Any& rAny );

    // write properties -------------------------------------------------------

    /** Writes the next value to the value sequence. */
    template< typename Type >
    void                writeValue( const Type& rValue );

    /** Writes an any to the value sequence. */
    void                writeValue( const ::com::sun::star::uno::Any& rAny );

    /** Writes all values to the passed property set. */
    void                writeToPropertySet( PropertySet& rPropSet );

    /** Creates a sequence of com.sun.star.beans.PropertyValue objects. */
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                        createPropertySequence();

    // ------------------------------------------------------------------------
private:
    /** Returns a pointer to the next Any to be written to. */
    ::com::sun::star::uno::Any* getNextAny();

private:
    typedef ::com::sun::star::uno::Sequence< ::rtl::OUString >              OUStringSequence;
    typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >   UnoAnySequence;
    typedef ::std::vector< sal_Int32 >                                      Int32Vec;

    OUStringSequence    maNameSeq;          /// Sequence of property names.
    UnoAnySequence      maValueSeq;         /// Sequence of property values.
    Int32Vec            maNameOrder;        /// Maps initial order to alphabetical order.
    size_t              mnNextIndex;        /// Counter for next Any to be processed.
};

// ----------------------------------------------------------------------------

template< typename Type >
bool PropertySequence::readValue( Type& rValue )
{
    ::com::sun::star::uno::Any* pAny = getNextAny();
    return pAny && (*pAny >>= rValue);
}

template< typename Type >
void PropertySequence::writeValue( const Type& rValue )
{
    if( ::com::sun::star::uno::Any* pAny = getNextAny() )
        *pAny <<= rValue;
}

template< typename Type >
inline PropertySequence& operator>>( PropertySequence& rPropSeq, Type& rValue )
{
    rPropSeq.readValue( rValue );
    return rPropSeq;
}

template< typename Type >
inline PropertySequence& operator<<( PropertySequence& rPropSeq, const Type& rValue )
{
    rPropSeq.writeValue( rValue );
    return rPropSeq;
}

inline PropertySequence& operator<<( PropertySequence& rPropSeq, const PropertySet& rPropSet )
{
    rPropSeq.readFromPropertySet( rPropSet );
    return rPropSeq;
}

inline PropertySequence& operator>>( PropertySequence& rPropSeq, PropertySet& rPropSet )
{
    rPropSeq.writeToPropertySet( rPropSet );
    return rPropSeq;
}

// ============================================================================

} // namespace oox

#endif

