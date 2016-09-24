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

    /* ------------------------------------------------------------------ */
    @Before public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet orb = UnoRuntime.queryInterface(XPropertySet.class, getMSF());
        XComponentContext context = UnoRuntime.queryInterface(XComponentContext.class, orb.getPropertyValue("DefaultContext"));

        m_loader = com.sun.star.resource.OfficeResourceLoader.get( context );
    }

    /* ------------------------------------------------------------------ */
    @After public void after() throws java.lang.Exception
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
    @Test public void checkLocales() throws java.lang.Exception
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
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
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
