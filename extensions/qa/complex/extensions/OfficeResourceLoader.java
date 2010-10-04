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
package complex.extensions;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

import com.sun.star.resource.XResourceBundle;
import com.sun.star.resource.XResourceBundleLoader;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.Locale;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

public class OfficeResourceLoader
{
    XResourceBundleLoader   m_loader;
    XResourceBundle         m_bundle;

    /** Creates a new instance of ValueBinding */
    public OfficeResourceLoader()
    {
    }

    /* ------------------------------------------------------------------ */
//    public String[] getTestMethodNames()
//    {
//        return new String[] {
//            "checkSimpleStringAccess",
//            "checkLocales"
//        };
//    }

    /* ------------------------------------------------------------------ */
//    public String getTestObjectName()
//    {
//        return "Extensions - OfficeResourceLoader";
//    }

    /* ------------------------------------------------------------------ */
    @Before public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet orb = UnoRuntime.queryInterface(XPropertySet.class, getMSF());
        XComponentContext context = UnoRuntime.queryInterface(XComponentContext.class, orb.getPropertyValue("DefaultContext"));

        m_loader = com.sun.star.resource.OfficeResourceLoader.get( context );
    }

    /* ------------------------------------------------------------------ */
    @After public void after() throws com.sun.star.uno.Exception, java.lang.Exception
    {
    }

    /* ------------------------------------------------------------------ */
    @Test public void checkSimpleStringAccess() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // default bundle (UI locale)
        m_bundle = m_loader.loadBundle_Default( "orl" );

        Locale resourceLocale = m_bundle.getLocale();

        String testString = (String)m_bundle.getByName( "string:1000" );

        if  (   resourceLocale.Language.equals( "en" )
            &&  resourceLocale.Country.equals( "US" )
            &&  resourceLocale.Variant.equals( "" )
            )
        {
            assertTrue( "invalid 'en-US' string", testString.equals( "Dummy String" ) );
        }

        if  (   resourceLocale.Language.equals( "de" )
            &&  resourceLocale.Country.equals( "" )
            &&  resourceLocale.Variant.equals( "" )
            )
        {
            assertTrue( "invalid 'de' string", testString.equals( "Attrappen-Zeichenkette" ) );
        }

        if  (   resourceLocale.Language.equals( "" )
            &&  resourceLocale.Country.equals( "" )
            &&  resourceLocale.Variant.equals( "" )
            )
        {
            assertTrue( "invalid unlocalized string", testString.equals( "unlocalized string" ) );
        }
    }

    /* ------------------------------------------------------------------ */
    @Test public void checkLocales() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // en-US bundle (should always be built and thus present and thus found)
        m_bundle = m_loader.loadBundle( "orl", new Locale( "en", "US", "" ) );
        Locale resourceLocale = m_bundle.getLocale();
        assertTrue( "'en-US' could not be loaded",
            resourceLocale.Language.equals( "en" ) && resourceLocale.Country.equals( "US" ) && resourceLocale.Variant.equals( "" ) );

        // some (invalid) locale which is usually no built, and should thus fallback to en-US
        m_bundle = m_loader.loadBundle( "orl", new Locale( "inv", "al", "id" ) );
        resourceLocale = m_bundle.getLocale();
        assertTrue( "non-existing locale request does not fallback to en-US",
            resourceLocale.Language.equals( "en" ) && resourceLocale.Country.equals( "US" ) && resourceLocale.Variant.equals( "" ) );
    }


    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();
}
