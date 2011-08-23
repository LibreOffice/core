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

#ifndef OOX_HELPER_ATTRIBUTELIST_HXX
#define OOX_HELPER_ATTRIBUTELIST_HXX

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include "oox/helper/helper.hxx"
#include "tokens.hxx"

namespace oox {

// ============================================================================

/** Static helpers for conversion of strings to attribute values of various
    different data types.
 */
class AttributeConversion
{
public:
    /** Returns the decoded string value. All characters in the format
        '_xHHHH_' (H being a hexadecimal digit), will be decoded. */
    static ::rtl::OUString decodeXString( const ::rtl::OUString& rValue );

    /** Returns the double value from the passed string. */
    static double       decodeDouble( const ::rtl::OUString& rValue );

    /** Returns the 32-bit signed integer value from the passed string (decimal). */
    static sal_Int32    decodeInteger( const ::rtl::OUString& rValue );

    /** Returns the 32-bit unsigned integer value from the passed string (decimal). */
    static sal_uInt32   decodeUnsigned( const ::rtl::OUString& rValue );

    /** Returns the 64-bit signed integer value from the passed string (decimal). */
    static sal_Int64    decodeHyper( const ::rtl::OUString& rValue );

    /** Returns the 32-bit signed integer value from the passed string (hexadecimal). */
    static sal_Int32    decodeIntegerHex( const ::rtl::OUString& rValue );

    /** Returns the 32-bit unsigned integer value from the passed string (hexadecimal). */
    static sal_uInt32   decodeUnsignedHex( const ::rtl::OUString& rValue );

    /** Returns the 64-bit signed integer value from the passed string (hexadecimal). */
    static sal_Int64    decodeHyperHex( const ::rtl::OUString& rValue );
};

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
    bool                hasAttribute( sal_Int32 nAttrToken ) const;

    // optional return values -------------------------------------------------

    /** Returns the token identifier of the value of the specified attribute. */
    OptValue< sal_Int32 > getToken( sal_Int32 nAttrToken ) const;

    /** Returns the string value of the specified attribute. */
    OptValue< ::rtl::OUString > getString( sal_Int32 nAttrToken ) const;

    /** Returns the string value of the specified attribute. All characters in
        the format '_xHHHH_' (H being a hexadecimal digit), will be decoded. */
    OptValue< ::rtl::OUString > getXString( sal_Int32 nAttrToken ) const;

    /** Returns the double value of the specified attribute. */
    OptValue< double >  getDouble( sal_Int32 nAttrToken ) const;

    /** Returns the 32-bit signed integer value of the specified attribute (decimal). */
    OptValue< sal_Int32 > getInteger( sal_Int32 nAttrToken ) const;

    /** Returns the 32-bit unsigned integer value of the specified attribute (decimal). */
    OptValue< sal_uInt32 > getUnsigned( sal_Int32 nAttrToken ) const;

    /** Returns the 64-bit signed integer value of the specified attribute (decimal). */
    OptValue< sal_Int64 > getHyper( sal_Int32 nAttrToken ) const;

    /** Returns the 32-bit signed integer value of the specified attribute (hexadecimal). */
    OptValue< sal_Int32 > getIntegerHex( sal_Int32 nAttrToken ) const;

    /** Returns the 32-bit unsigned integer value of the specified attribute (hexadecimal). */
    OptValue< sal_uInt32 > getUnsignedHex( sal_Int32 nAttrToken ) const;

    /** Returns the 64-bit signed integer value of the specified attribute (hexadecimal). */
    OptValue< sal_Int64 > getHyperHex( sal_Int32 nAttrToken ) const;

    /** Returns the boolean value of the specified attribute. */
    OptValue< bool >    getBool( sal_Int32 nAttrToken ) const;

    /** Returns the date/time value of the specified attribute. */
    OptValue< ::com::sun::star::util::DateTime > getDateTime( sal_Int32 nAttrToken ) const;

    // defaulted return values ------------------------------------------------

    /** Returns the token identifier of the value of the specified attribute,
        or the passed default identifier if the attribute is missing. */
    sal_Int32           getToken( sal_Int32 nAttrToken, sal_Int32 nDefault ) const;

    /** Returns the string value of the specified attribute, or the passed
        default string if the attribute is missing. */
    ::rtl::OUString     getString( sal_Int32 nAttrToken, const ::rtl::OUString& rDefault ) const;

    /** Returns the decoded string value of the specified attribute, or the
        passed default string if the attribute is missing. */
    ::rtl::OUString     getXString( sal_Int32 nAttrToken, const ::rtl::OUString& rDefault ) const;

    /** Returns the double value of the specified attribute, or the passed
        default value if the attribute is missing or not convertible to a double. */
    double              getDouble( sal_Int32 nAttrToken, double fDefault ) const;

    /** Returns the 32-bit signed integer value of the specified attribute, or the
        passed default value if the attribute is missing or not convertible to integer. */
    sal_Int32           getInteger( sal_Int32 nAttrToken, sal_Int32 nDefault ) const;

    /** Returns the 32-bit unsigned integer value of the specified attribute, or the
        passed default value if the attribute is missing or not convertible to unsigned. */
    sal_uInt32          getUnsigned( sal_Int32 nAttrToken, sal_uInt32 nDefault ) const;

    /** Returns the 64-bit signed integer value of the specified attribute, or the
        passed default value if the attribute is missing or not convertible to integer. */
    sal_Int64           getHyper( sal_Int32 nAttrToken, sal_Int64 nDefault ) const;

    /** Returns the 32-bit signed integer value of the specified attribute (hexadecimal),
        or the passed default value if the attribute is missing or not convertible. */
    sal_Int32           getIntegerHex( sal_Int32 nAttrToken, sal_Int32 nDefault ) const;

    /** Returns the 32-bit unsigned integer value of the specified attribute (hexadecimal),
        or the passed default value if the attribute is missing or not convertible. */
    sal_uInt32          getUnsignedHex( sal_Int32 nAttrToken, sal_uInt32 nDefault ) const;

    /** Returns the 64-bit signed integer value of the specified attribute (hexadecimal),
        or the passed default value if the attribute is missing or not convertible. */
    sal_Int64           getHyperHex( sal_Int32 nAttrToken, sal_Int64 nDefault ) const;

    /** Returns the boolean value of the specified attribute, or the passed
        default value if the attribute is missing or not convertible to bool. */
    bool                getBool( sal_Int32 nAttrToken, bool bDefault ) const;

    /** Returns the date/time value of the specified attribute, or the default
        value if the attribute is missing or not convertible to a date/time value. */
    ::com::sun::star::util::DateTime getDateTime( sal_Int32 nAttrToken, const ::com::sun::star::util::DateTime& rDefault ) const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >
                        mxAttribs;
};

// ============================================================================

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
