/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeResourceLoader.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-03-29 12:41:25 $
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
