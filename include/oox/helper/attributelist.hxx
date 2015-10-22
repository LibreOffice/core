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

#ifndef INCLUDED_OOX_HELPER_ATTRIBUTELIST_HXX
#define INCLUDED_OOX_HELPER_ATTRIBUTELIST_HXX

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xml/sax/XFastAttributeList.hpp>
#include <oox/helper/helper.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/dllapi.h>

namespace sax_fastparser {
    class FastAttributeList;
};

namespace oox {



/** Static helpers for conversion of strings to attribute values of various
    different data types.
 */
class OOX_DLLPUBLIC AttributeConversion
{
public:
    /** Returns the XML token identifier from the passed string. */
    static sal_Int32    decodeToken( const OUString& rValue );

    /** Returns the decoded string value. All characters in the format
        '_xHHHH_' (H being a hexadecimal digit), will be decoded. */
    static OUString decodeXString( const OUString& rValue );

    /** Returns the 32-bit signed integer value from the passed string (decimal). */
    static sal_Int32    decodeInteger( const OUString& rValue );

    /** Returns the 32-bit unsigned integer value from the passed string (decimal). */
    static sal_uInt32   decodeUnsigned( const OUString& rValue );

    /** Returns the 64-bit signed integer value from the passed string (decimal). */
    static sal_Int64    decodeHyper( const OUString& rValue );

    /** Returns the 32-bit signed integer value from the passed string (hexadecimal). */
    static sal_Int32    decodeIntegerHex( const OUString& rValue );
};



/** Provides access to attribute values of an element.

    Wraps a com.sun.star.xml.sax.XFastAttributeList object. Provides
    convenience functions that convert the string value of an attribute to
    various other data types.
 */
class OOX_DLLPUBLIC AttributeList
{
public:
    explicit            AttributeList(
                            const css::uno::Reference< css::xml::sax::XFastAttributeList >& rxAttribs );

    /** Returns the wrapped com.sun.star.xml.sax.XFastAttributeList object. */
    css::uno::Reference< css::xml::sax::XFastAttributeList >
                        getFastAttributeList() const { return mxAttribs; }

    /** Returns true, if the specified attribute is present. */
    bool                hasAttribute( sal_Int32 nAttrToken ) const;

    // optional return values -------------------------------------------------

    /** Returns the token identifier of the value of the specified attribute. */
    OptValue< sal_Int32 > getToken( sal_Int32 nAttrToken ) const;

    /** Returns the string value of the specified attribute. */
    OptValue< OUString > getString( sal_Int32 nAttrToken ) const;

    /** Returns the string value of the specified attribute. All characters in
        the format '_xHHHH_' (H being a hexadecimal digit), will be decoded. */
    OptValue< OUString > getXString( sal_Int32 nAttrToken ) const;

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

    /** Returns the boolean value of the specified attribute. */
    OptValue< bool >    getBool( sal_Int32 nAttrToken ) const;

    /** Returns the date/time value of the specified attribute. */
    OptValue< css::util::DateTime > getDateTime( sal_Int32 nAttrToken ) const;

    // defaulted return values ------------------------------------------------

    /** Returns the token identifier of the value of the specified attribute,
        or the passed default identifier if the attribute is missing. */
    sal_Int32           getToken( sal_Int32 nAttrToken, sal_Int32 nDefault ) const;

    /** Returns the string value of the specified attribute, or the passed
        default string if the attribute is missing. */
    OUString     getString( sal_Int32 nAttrToken, const OUString& rDefault ) const;

    /** Returns the decoded string value of the specified attribute, or the
        passed default string if the attribute is missing. */
    OUString     getXString( sal_Int32 nAttrToken, const OUString& rDefault ) const;

    const char* getChar( sal_Int32 nAttrToken ) const;


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

    sal_uInt32          getUnsignedHex( sal_Int32 nAttrToken, sal_uInt32 nDefault ) const;

    /** Returns the boolean value of the specified attribute, or the passed
        default value if the attribute is missing or not convertible to bool. */
    bool                getBool( sal_Int32 nAttrToken, bool bDefault ) const;

    /** Returns the date/time value of the specified attribute, or the default
        value if the attribute is missing or not convertible to a date/time value. */
    css::util::DateTime getDateTime( sal_Int32 nAttrToken, const css::util::DateTime& rDefault ) const;

private:
    css::uno::Reference< css::xml::sax::XFastAttributeList >
                        mxAttribs;
    mutable sax_fastparser::FastAttributeList *mpAttribList;
    sax_fastparser::FastAttributeList *getAttribList() const;
};



} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
