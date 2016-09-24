/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

// uno
import com.sun.star.lib.uno.helper.ComponentBase;
import com.sun.star.uno.UnoRuntime;

// factories
import com.sun.star.lang.XSingleComponentFactory;

// supported Interfaces
import com.sun.star.linguistic2.XThesaurus;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceDisplayName;

// Exceptions
import com.sun.star.uno.Exception;
//used Interfaces
import com.sun.star.linguistic2.XMeaning;
import com.sun.star.lang.Locale;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.uno.AnyConverter;
import java.util.ArrayList;

public class SampleThesaurus extends ComponentBase implements
        XThesaurus,
        XInitialization,
        XServiceDisplayName,
        XServiceInfo
{
    PropChgHelper               aPropChgHelper;
    ArrayList<?>                   aEvtListeners;
    boolean                     bDisposing;

    public SampleThesaurus()
    {
        // names of relevant properties to be used
        String[] aProps = new String[]
            {
                "IsIgnoreControlCharacters",
                "IsUseDictionaryList",
                "IsGermanPreReform",
            };

        // this service has no listeners thus we may use the base class,
        // which is here basically used only to keep track of the
        // property set (and its lifetime) since it gets used in the
        // 'GetValueToUse' function
        aPropChgHelper  = new PropChgHelper( this, aProps );

        aEvtListeners   = new ArrayList<Object>();
        bDisposing      = false;
    }

    private boolean IsEqual( Locale aLoc1, Locale aLoc2 )
    {
        return aLoc1.Language.equals( aLoc2.Language ) &&
               aLoc1.Country .equals( aLoc2.Country )  &&
               aLoc1.Variant .equals( aLoc2.Variant );
    }

    private boolean GetValueToUse(
            String          aPropName,
            boolean         bDefaultVal,
            PropertyValue[] aProps )
    {
        boolean bRes = bDefaultVal;

        try
        {
            // use temporary value if supplied
            for (int i = 0;  i < aProps.length;  ++i)
            {
                if (aPropName.equals( aProps[i].Name ))
                {
                    Object aObj = aProps[i].Value;
                    if (AnyConverter.isBoolean( aObj ))
                    {
                        bRes = AnyConverter.toBoolean( aObj );
                        return bRes;
                    }
                }
            }

            // otherwise use value from property set (if available)
            XPropertySet xPropSet = aPropChgHelper.GetPropSet();
            if (xPropSet != null)   // should always be the case
            {
                Object aObj = xPropSet.getPropertyValue( aPropName );
                if (AnyConverter.isBoolean( aObj ))
                    bRes = AnyConverter.toBoolean( aObj );
            }
        }
        catch (Exception e) {
            bRes = bDefaultVal;
        }

        return bRes;
    }

    // __________ interface methods __________



    //XSupportedLocales

    public Locale[] getLocales()
        throws com.sun.star.uno.RuntimeException
    {
        Locale aLocales[] =
        {
            new Locale( "en", "US", "" )
        };

        return aLocales;
    }

    public boolean hasLocale( Locale aLocale )
        throws com.sun.star.uno.RuntimeException
    {
        boolean bRes = false;
        if (IsEqual( aLocale, new Locale( "en", "US", "" ) ))
            bRes = true;
        return bRes;
    }


    //XThesaurus

    public XMeaning[] queryMeanings(
            String aTerm, Locale aLocale,
            PropertyValue[] aProperties )
        throws com.sun.star.lang.IllegalArgumentException,
               com.sun.star.uno.RuntimeException
    {
        if (IsEqual( aLocale, new Locale() ) || aTerm.length() == 0)
            return null;

        // linguistic is currently not allowed to throw exceptions
        // thus we return null fwhich means 'word cannot be looked up'
        if (!hasLocale( aLocale ))
            return null;

        // get values of relevant properties that may be used.
        //! The values for 'IsIgnoreControlCharacters' and 'IsUseDictionaryList'
        //! are handled by the dispatcher! Thus there is no need to access
        //! them here.
        boolean bIsGermanPreReform      = GetValueToUse( "IsGermanPreReform", false, aProperties );

        XMeaning[] aRes = null;

        //!! This code needs to be replaced by code calling the actual
        //!! implementation of your thesaurus
        if (aTerm.equals( "house" ) &&
            IsEqual( aLocale, new Locale( "en", "US", "" ) ) )
        {
            aRes = new XMeaning[]
                {
                    new XMeaning_impl( "a building where one lives",
                            new String[]{ "home", "place", "dwelling" } ),
                    new XMeaning_impl( "a group of people sharing common ancestry",
                            new String[]{ "family", "clan", "kindred" } ),
                    new XMeaning_impl( "to provide with lodging",
                            new String[]{ "room", "board", "put up" } )
                };
        }

        return aRes;
    }



    // XServiceDisplayName

    public String getServiceDisplayName( Locale aLocale )
        throws com.sun.star.uno.RuntimeException
    {
        return "Java Samples";
    }


    // XInitialization

    public void initialize( Object[] aArguments )
        throws com.sun.star.uno.Exception,
               com.sun.star.uno.RuntimeException
    {
        int nLen = aArguments.length;
        if (2 == nLen)
        {
            XPropertySet xPropSet = UnoRuntime.queryInterface(
                                         XPropertySet.class, aArguments[0]);
            // start listening to property changes
            aPropChgHelper.AddAsListenerTo( xPropSet );
        }
    }


    // XServiceInfo

    public boolean supportsService( String aServiceName )
        throws com.sun.star.uno.RuntimeException
    {
        String[] aServices = getSupportedServiceNames_Static();
        int i, nLength = aServices.length;
        boolean bResult = false;

        for( i = 0; !bResult && i < nLength; ++i )
            bResult = aServiceName.equals( aServices[ i ] );

        return bResult;
    }

    public String getImplementationName()
        throws com.sun.star.uno.RuntimeException
    {
        return _aSvcImplName;
    }

    public String[] getSupportedServiceNames()
        throws com.sun.star.uno.RuntimeException
    {
        return getSupportedServiceNames_Static();
    }

    // __________ static things __________

    public static String _aSvcImplName = SampleThesaurus.class.getName();

    public static String[] getSupportedServiceNames_Static()
    {
        String[] aResult = { "com.sun.star.linguistic2.Thesaurus" };
        return aResult;
    }


    /**
     * Returns a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleComponentFactory</code> for creating the component
     * @param   aImplName     the name of the implementation for which a service is desired
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleComponentFactory __getComponentFactory(
        String aImplName )
    {
        XSingleComponentFactory xSingleComponentFactory = null;
        if( aImplName.equals( _aSvcImplName ) )
        {
            xSingleComponentFactory = new OneInstanceFactory(
                    SampleThesaurus.class, _aSvcImplName,
                    getSupportedServiceNames_Static() );
        }
        return xSingleComponentFactory;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
