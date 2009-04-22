/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: attributelist.hxx,v $
 * $Revision: 1.6 $
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

#ifndef OOX_HELPER_ATTRIBUTELIST_HXX
#define OOX_HELPER_ATTRIBUTELIST_HXX

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include "oox/helper/helper.hxx"
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

    // optional return values -------------------------------------------------

    /** Returns the token identifier of the value of the specified attribute. */
    OptValue< sal_Int32 > getToken( sal_Int32 nElement ) const;

    /** Returns the string value of the specified attribute. */
    OptValue< ::rtl::OUString > getString( sal_Int32 nElement ) const;

    /** Returns the double value of the specified attribute. */
    OptValue< double >  getDouble( sal_Int32 nElement ) const;

    /** Returns the integer value of the specified attribute. */
    OptValue< sal_Int32 > getInteger( sal_Int32 nElement ) const;

    /** Returns the unsigned integer value of the specified attribute. */
    OptValue< sal_uInt32 > getUnsignedInteger( sal_Int32 nElement ) const;

    /** Returns the 64-bit integer value of the specified attribute. */
    OptValue< sal_Int64 > getInteger64( sal_Int32 nElement ) const;

    /** Returns the integer value of the specified hexadecimal attribute. */
    OptValue< sal_Int32 > getHex( sal_Int32 nElement ) const;

    /** Returns the boolean value of the specified attribute. */
    OptValue< bool >    getBool( sal_Int32 nElement ) const;

    /** Returns the date/time value of the specified attribute. */
    OptValue< ::com::sun::star::util::DateTime > getDateTime( sal_Int32 nElement ) const;

    // defaulted return values ------------------------------------------------

    /** Returns the token identifier of the value of the specified attribute,
        or the passed default identifier if the attribute is missing. */
    sal_Int32           getToken( sal_Int32 nElement, sal_Int32 nDefault ) const;

    /** Returns the string value of the specified attribute, or the passed
        default string if the attribute is missing. */
    ::rtl::OUString     getString( sal_Int32 nElement, const ::rtl::OUString& rDefault ) const;

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

    /** Returns the date/time value of the specified attribute, or the default
        value if the attribute is missing or not convertible to a date/time value. */
    ::com::sun::star::util::DateTime getDateTime( sal_Int32 nElement, const ::com::sun::star::util::DateTime& rDefault ) const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >
                        mxAttribs;
};

// ============================================================================

} // namespace oox

#endif

