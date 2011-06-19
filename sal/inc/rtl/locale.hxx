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

#ifndef _RTL_LOCALE_HXX_
#define _RTL_LOCALE_HXX_

#include <rtl/locale.h>
#include <rtl/ustring.hxx>

#ifdef __cplusplus

namespace rtl
{

/**
    A <code>OLocale</code> object represents a specific geographical, political,
    or cultural region. An operation that requires a <code>OLocale</code> to perform
    its task is called <em>locale-sensitive</em> and uses the <code>OLocale</code>
    to tailor information for the user. For example, displaying a number
    is a locale-sensitive operation--the number should be formatted
    according to the customs/conventions of the user's native country,
    region, or culture.

    <P>
    You create a <code>OLocale</code> object using one of the two constructors in
    this class:
    <blockquote>
    <pre>
    OLocale(String language, String country)
    OLocale(String language, String country, String variant)
    </pre>
    </blockquote>
    The first argument to both constructors is a valid <STRONG>ISO
    Language Code.</STRONG> These codes are the lower-case two-letter
    codes as defined by ISO-639.
    You can find a full list of these codes at a number of sites, such as:
    <BR><a href ="http://www.ics.uci.edu/pub/ietf/http/related/iso639.txt">
    <code>http://www.ics.uci.edu/pub/ietf/http/related/iso639.txt</code></a>

    <P>
    The second argument to both constructors is a valid <STRONG>ISO Country
    Code.</STRONG> These codes are the upper-case two-letter codes
    as defined by ISO-3166.
    You can find a full list of these codes at a number of sites, such as:
    <BR><a href="http://www.chemie.fu-berlin.de/diverse/doc/ISO_3166.html">
    <code>http://www.chemie.fu-berlin.de/diverse/doc/ISO_3166.html</code></a>

    <P>
    The second constructor requires a third argument--the <STRONG>Variant.</STRONG>
    The Variant codes are vendor and browser-specific.
    For example, use WIN for Windows, MAC for Macintosh, and POSIX for POSIX.
    Where there are two variants, separate them with an underscore, and
    put the most important one first. For
    example, a Traditional Spanish collation might be referenced, with
    "ES", "ES", "Traditional_WIN".

    <P>
    Because a <code>OLocale</code> object is just an identifier for a region,
    no validity check is performed when you construct a <code>OLocale</code>.
    If you want to see whether particular resources are available for the
    <code>OLocale</code> you construct, you must query those resources. For
    example, ask the <code>NumberFormat</code> for the locales it supports
    using its <code>getAvailableLocales</code> method.
    <BR><STRONG>Note:</STRONG> When you ask for a resource for a particular
    locale, you get back the best available match, not necessarily
    precisely what you asked for. For more information, look at
    <a href="java.util.ResourceBundle.html"><code>ResourceBundle</code></a>.

    <P>
    The <code>OLocale</code> class provides a number of convenient constants
    that you can use to create <code>OLocale</code> objects for commonly used
    locales. For example, the following creates a <code>OLocale</code> object
    for the United States:
    <blockquote>
    <pre>
    OLocale.US
    </pre>
    </blockquote>

    <P>
    Once you've created a <code>OLocale</code> you can query it for information about
    itself. Use <code>getCountry</code> to get the ISO Country Code and
    <code>getLanguage</code> to get the ISO Language Code. You can
    use <code>getDisplayCountry</code> to get the
    name of the country suitable for displaying to the user. Similarly,
    you can use <code>getDisplayLanguage</code> to get the name of
    the language suitable for displaying to the user. Interestingly,
    the <code>getDisplayXXX</code> methods are themselves locale-sensitive
    and have two versions: one that uses the default locale and one
    that uses the locale specified as an argument.

    <P>
    The JDK provides a number of classes that perform locale-sensitive
    operations. For example, the <code>NumberFormat</code> class formats
    numbers, currency, or percentages in a locale-sensitive manner. Classes
    such as <code>NumberFormat</code> have a number of convenience methods
    for creating a default object of that type. For example, the
    <code>NumberFormat</code> class provides these three convenience methods
    for creating a default <code>NumberFormat</code> object:
    <blockquote>
    <pre>
    NumberFormat.getInstance()
    NumberFormat.getCurrencyInstance()
    NumberFormat.getPercentInstance()
    </pre>
    </blockquote>
    These methods have two variants; one with an explicit locale
    and one without; the latter using the default locale.
    <blockquote>
    <pre>
    NumberFormat.getInstance(myLocale)
    NumberFormat.getCurrencyInstance(myLocale)
    NumberFormat.getPercentInstance(myLocale)
    </pre>
    </blockquote>
    A <code>OLocale</code> is the mechanism for identifying the kind of object
    (<code>NumberFormat</code>) that you would like to get. The locale is
    <STRONG>just</STRONG> a mechanism for identifying objects,
    <STRONG>not</STRONG> a container for the objects themselves.

    <P>
    Each class that performs locale-sensitive operations allows you
    to get all the available objects of that type. You can sift
    through these objects by language, country, or variant,
    and use the display names to present a menu to the user.
    For example, you can create a menu of all the collation objects
    suitable for a given language. Such classes must implement these
    three class methods:
    <blockquote>
    <pre>
    public static OLocale[] getAvailableLocales()
    public static String getDisplayName(OLocale objectLocale,
                                       OLocale displayLocale)
    public static final String getDisplayName(OLocale objectLocale)
       // getDisplayName will throw MissingResourceException if the locale
       // is not one of the available locales.
    </pre>
    </blockquote>
 */
class OLocale
{
public:
    OLocale( rtl_Locale * locale )
        : pData( locale ) {}

    OLocale( const OLocale & obj)
        : pData(obj.pData) {}

    OLocale & operator = ( const OLocale & obj)
        {
            pData = obj.pData;
            return *this;
        }

    /**
         Construct a locale from language, country, variant.
        @param language lowercase two-letter ISO-639 code.
        @param country uppercase two-letter ISO-3166 code.
        @param variant vendor and browser specific code. See class description.
     */
    static OLocale registerLocale( const OUString & language, const OUString & country,
                            const OUString & variant )
    {
        return rtl_locale_register( language.getStr(), country.getStr(), variant.getStr() );
    }

    /**
         Construct a locale from language, country.
        @param language lowercase two-letter ISO-639 code.
        @param country uppercase two-letter ISO-3166 code.
     */
    static OLocale registerLocale( const OUString & language, const OUString & country )
    {
        return rtl_locale_register( language.getStr(), country.getStr(), NULL );
    }

    /** @deprecated
     */
    static OLocale getDefault()  { return rtl_locale_getDefault(); }

    /** @deprecated
     */
    static void setDefault( const OUString & language, const OUString & country,
                            const OUString & variant )
             { rtl_locale_setDefault(language.getStr(), country.getStr(), variant.getStr()); }

    /**
         Getter for programmatic name of field,
         an lowercased two-letter ISO-639 code.
     */
    OUString getLanguage() const { return pData->Language; }

    /**
         Getter for programmatic name of field,
         an uppercased two-letter ISO-3166 code.
     */
    OUString getCountry() const { return pData->Country; }

    /**
         Getter for programmatic name of field.
     */
    OUString getVariant() const { return pData->Variant; }


    /**
         Returns the hash code of the locale This.
     */
    sal_Int32 hashCode() const { return pData->HashCode; }

    sal_Bool operator == (const OLocale & obj ) const
    {
        return pData == obj.pData;
    }

    rtl_Locale *    getData() const { return pData; }

private:
    /**
         Must be the first member in this class. OUString access this member with
         *(rtl_Locale **)&locale.
     */
    rtl_Locale *    pData;

    OLocale()
        : pData(rtl_locale_getDefault()) {}
/*
    OLocale( const OLocale & obj)
        : pData(obj.pData) {}

    OLocale & operator = ( const OLocale & obj)
        { pData = obj.pData;
          return *this;
        }
*/
};

}

#endif /* __cplusplus */
#endif /* _RTL_LOCALE_HXX_ */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
