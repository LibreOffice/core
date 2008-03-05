/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attributelist.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:53:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef OOX_HELPER_ATTRIBUTELIST_HXX
#define OOX_HELPER_ATTRIBUTELIST_HXX

#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include "tokens.hxx"

namespace oox {

// ============================================================================

/** Provides access to attribute values of an element.

    Wraps a com.sun.star.xml.sax.XFastAttributeList object. Provides
    convenience functions that convert the string value of an attribute to
    various other data types.
 */
class AttributeList
{
public:
    explicit            AttributeList(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& rxAttribs );

    /** Returns the wrapped com.sun.star.xml.sax.XFastAttributeList object. */
    inline ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >
                        getFastAttributeList() const { return mxAttribs; }

    /** Returns true, if the specified attribute is present. */
    bool                hasAttribute( sal_Int32 nElement ) const;

    /** Returns the token identifier of the value of the specified attribute,
        or the passed default identifier if the attribute is missing. */
    sal_Int32           getToken( sal_Int32 nElement, sal_Int32 nDefault = XML_TOKEN_INVALID ) const;

    /** Returns the string value of the specified attribute, or an empty string
        if the attribute is missing. */
    ::rtl::OUString     getString( sal_Int32 nElement ) const;

    /** Returns the double value of the specified attribute, or the passed
        default value if the attribute is missing or not convertible to a double. */
    double              getDouble( sal_Int32 nElement, double fDefault ) const;

    /** Returns the integer value of the specified attribute, or the passed
        default value if the attribute is missing or not convertible to integer. */
    sal_Int32           getInteger( sal_Int32 nElement, sal_Int32 nDefault ) const;

    /** Returns the unsigned integer value of the specified attribute, or the passed
        default value if the attribute is missing or not convertible to unsigned integer. */
    sal_uInt32          getUnsignedInteger( sal_Int32 nElement, sal_uInt32 nDefault ) const;

    /** Returns the 64-bit integer value of the specified attribute, or the passed
        default value if the attribute is missing or not convertible to integer. */
    sal_Int64           getInteger64( sal_Int32 nElement, sal_Int64 nDefault ) const;

    /** Returns the integer value of the specified hexadecimal attribute,
        or the passed default value if the attribute is missing or not convertible. */
    sal_Int32           getHex( sal_Int32 nElement, sal_Int32 nDefault ) const;

    /** Returns the boolean value of the specified attribute, or the passed
        default value if the attribute is missing or not convertible to bool. */
    bool                getBool( sal_Int32 nElement, bool bDefault ) const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >
                        mxAttribs;
};

// ============================================================================

} // namespace oox

#endif

