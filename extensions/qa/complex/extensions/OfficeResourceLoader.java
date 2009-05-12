/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeResourceLoader.java,v $
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
package complex.extensions;

import com.sun.star.uno.UnoRuntime;

import com.sun.star.resource.XResourceBundle;
import com.sun.star.resource.XResourceBundleLoader;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.Locale;

public class OfficeResourceLoader extends complexlib.ComplexTestCase
{
    XResourceBundleLoader   m_loader;
    XResourceBundle         m_bundle;

    /** Creates a new instance of ValueBinding */
    public OfficeResourceLoader()
    {
    }

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkSimpleStringAccess",
            "checkLocales"
        };
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return "Extensions - OfficeResourceLoader";
    }

    /* ------------------------------------------------------------------ */
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet orb = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, param.getMSF() );
        XComponentContext context = (XComponentContext)UnoRuntime.queryInterface( XComponentContext.class,
            orb.getPropertyValue( "DefaultContext" ) );

        m_loader = com.sun.star.resource.OfficeResourceLoader.get( context );
    }

    /* ------------------------------------------------------------------ */
    public void after() throws com.sun.star.uno.Exception, java.lang.Exception
    {
    }

    /* ------------------------------------------------------------------ */
    public void checkSimpleStringAccess() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // default bundle (UI locale)
        m_bundle = m_loader.loadBundle_Default( "orl" );

        Locale resourceLocale = m_bundle.getLocale();

        String testString = (String)m_bundle.getByName( "string:1000" );

        if  (   resourceLocale.Language.equals( "en" )
            &&  resourceLocale.Country.equals( "US" )
            &&  resourceLocale.Variant.equals( "" )
            )
            assure( "invalid 'en-US' string", testString.equals( "Dummy String" ) );

        if  (   resourceLocale.Language.equals( "de" )
            &&  resourceLocale.Country.equals( "" )
            &&  resourceLocale.Variant.equals( "" )
            )
            assure( "invalid 'de' string", testString.equals( "Attrappen-Zeichenkette" ) );

        if  (   resourceLocale.Language.equals( "" )
            &&  resourceLocale.Country.equals( "" )
            &&  resourceLocale.Variant.equals( "" )
            )
            assure( "invalid unlocalized string", testString.equals( "unlocalized string" ) );
    }

    /* ------------------------------------------------------------------ */
    public void checkLocales() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // en-US bundle (should always be built and thus present and thus found)
        m_bundle = m_loader.loadBundle( "orl", new Locale( "en", "US", "" ) );
        Locale resourceLocale = m_bundle.getLocale();
        assure( "'en-US' could not be loaded",
            resourceLocale.Language.equals( "en" ) && resourceLocale.Country.equals( "US" ) && resourceLocale.Variant.equals( "" ) );

        // some (invalid) locale which is usually no built, and should thus fallback to en-US
        m_bundle = m_loader.loadBundle( "orl", new Locale( "inv", "al", "id" ) );
        resourceLocale = m_bundle.getLocale();
        assure( "non-existing locale request does not fallback to en-US",
            resourceLocale.Language.equals( "en" ) && resourceLocale.Country.equals( "US" ) && resourceLocale.Variant.equals( "" ) );
    }
}
