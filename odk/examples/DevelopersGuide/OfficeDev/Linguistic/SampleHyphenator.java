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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

// supported Interfaces
import com.sun.star.linguistic2.XHyphenator;
import com.sun.star.linguistic2.XLinguServiceEventBroadcaster;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XServiceDisplayName;

// Exceptions
import com.sun.star.uno.Exception;
import com.sun.star.lang.IllegalArgumentException;

//used Interfaces
import com.sun.star.linguistic2.XLinguServiceEventListener;
import com.sun.star.linguistic2.XHyphenatedWord;
import com.sun.star.linguistic2.XPossibleHyphens;
import com.sun.star.lang.Locale;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.uno.AnyConverter;
import java.util.ArrayList;

public class SampleHyphenator extends ComponentBase implements
        XHyphenator,
        XLinguServiceEventBroadcaster,
        XInitialization,
        XServiceDisplayName,
        XServiceInfo
{
    PropChgHelper_Hyph          aPropChgHelper;
    ArrayList<?>                   aEvtListeners;
    boolean                     bDisposing;

    public SampleHyphenator()
    {
        // names of relevant properties to be used
        String[] aProps = new String[]
            {
                "IsIgnoreControlCharacters",
                "IsUseDictionaryList",
                "IsGermanPreReform",
                "HyphMinLeading",
                "HyphMinTrailing",
                "HyphMinWordLength"
            };
        aPropChgHelper  = new PropChgHelper_Hyph( (XHyphenator) this, aProps );
        aEvtListeners   = new ArrayList<Object>();;
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

    private short GetValueToUse(
            String          aPropName,
            short           nDefaultVal,
            PropertyValue[] aProps )
    {
        short nRes = nDefaultVal;

        try
        {
            // use temporary value if supplied
            for (int i = 0;  i < aProps.length;  ++i)
            {
                if (aPropName.equals( aProps[i].Name ))
                {
                    Object aObj = aProps[i].Value;
                    if (AnyConverter.isShort( aObj ))
                    {
                        nRes = AnyConverter.toShort( aObj );
                        return nRes;
                    }
                }
            }

            // otherwise use value from property set (if available)
            XPropertySet xPropSet = aPropChgHelper.GetPropSet();
            if (xPropSet != null)   // should always be the case
            {
                    Object aObj = xPropSet.getPropertyValue( aPropName );
                    if (AnyConverter.isShort( aObj ))
                        nRes = AnyConverter.toShort( aObj );
            }
        }
        catch (Exception e) {
            nRes = nDefaultVal;
        }

        return nRes;
    }

    // __________ interface methods __________


    //*****************
    //XSupportedLocales
    //*****************
    public Locale[] getLocales()
        throws com.sun.star.uno.RuntimeException
    {
        Locale aLocales[] =
        {
            new Locale( "de", "DE", "" ),
            new Locale( "en", "US", "" )
        };

        return aLocales;
    }

    public boolean hasLocale( Locale aLocale )
        throws com.sun.star.uno.RuntimeException
    {
        boolean bRes = false;
        if ( IsEqual( aLocale, new Locale( "de", "DE", "" ) )  ||
             IsEqual( aLocale, new Locale( "en", "US", "" ) ))
            bRes = true;
        return bRes;
    }

    //***********
    //XHyphenator
    //***********
    public XHyphenatedWord hyphenate(
            String aWord, Locale aLocale,
            short nMaxLeading, PropertyValue[] aProperties )
        throws com.sun.star.uno.RuntimeException,
               IllegalArgumentException
    {
        if (IsEqual( aLocale, new Locale() ) || aWord.length() == 0)
            return null;

        // linguistic is currently not allowed to throw exceptions
        // thus we return null fwhich means 'word cannot be hyphenated'
        if (!hasLocale( aLocale ))
            return null;

        // get values of relevant properties that may be used.
        //! The values for 'IsIgnoreControlCharacters' and 'IsUseDictionaryList'
        //! are handled by the dispatcher! Thus there is no need to access
        //! them here.
        boolean bIsIgnoreControlCharacters  = GetValueToUse( "IsIgnoreControlCharacters", true, aProperties );
        boolean bIsUseDictionaryList        = GetValueToUse( "IsUseDictionaryList", true, aProperties );
        boolean bIsGermanPreReform          = GetValueToUse( "IsGermanPreReform", false, aProperties );
        short   nHyphMinLeading             = GetValueToUse( "HyphMinLeading", (short)2, aProperties );
        short   nHyphMinTrailing            = GetValueToUse( "HyphMinTrailing", (short)2, aProperties );
        short   nHyphMinWordLen             = GetValueToUse( "HyphMinWordLength", (short)5, aProperties );

        XHyphenatedWord xRes = null;

        if (aWord.length() >= nHyphMinWordLen)
        {
            String  aHyphenatedWord = aWord;
            short   nHyphenationPos = -1;
            short   nHyphenPos = -1;

            //!! This code needs to be replaced by code calling the actual
            //!! implementation of your hyphenator
            if (IsEqual( aLocale, new Locale( "de", "DE", "" ) ) )
            {
                if (bIsGermanPreReform && aWord.equals( "Schiffahrt" ))
                {
                    // Note: there is only one position where the word
                    // can be hyphenated...

                    aHyphenatedWord = "Schifffahrt";
                    nHyphenationPos = 4;
                    nHyphenPos = 5;
                }
                else if (!bIsGermanPreReform && aWord.equals( "Schifffahrt" ))
                {
                    nHyphenationPos = nHyphenPos = 5;
                }
            }
            else if (IsEqual( aLocale, new Locale( "en", "US", "" ) ) )
            {
                int nLast = aWord.length() - 1 - nHyphMinTrailing;

                if ( aWord.equals( "waterfall" ) )
                {
                    if (4 <= nLast)
                        nHyphenationPos = nHyphenPos = 4;
                    else
                        nHyphenationPos = nHyphenPos = 1;
                }
                else if ( aWord.equals( "driving" ) )
                {
                    nHyphenationPos = nHyphenPos = 3;
                }
            }

            // check if hyphenation pos is valid,
            // a value of -1 indicates that hyphenation is not possible
            if (  nHyphenationPos != -1 &&
                !(nHyphenationPos <  nHyphMinLeading) &&
                !(nHyphenationPos >= aWord.length() - nHyphMinTrailing))
            {
                xRes = new XHyphenatedWord_impl(aWord, aLocale,
                                nHyphenationPos, aHyphenatedWord, nHyphenPos);
            }
        }
        return xRes;
    }

    public XHyphenatedWord queryAlternativeSpelling(
            String aWord, Locale aLocale,
            short nIndex, PropertyValue[] aProperties )
        throws com.sun.star.uno.RuntimeException,
               IllegalArgumentException
    {
        if (IsEqual( aLocale, new Locale() ) || aWord.length() == 0)
            return null;

        // linguistic is currently not allowed to throw exceptions
        // thus we return null which means 'word cannot be hyphenated'
        if (!hasLocale( aLocale ))
            return null;

        // get values of relevant properties that may be used.
        //! The values for 'IsIgnoreControlCharacters' and 'IsUseDictionaryList'
        //! are handled by the dispatcher! Thus there is no need to access
        //! them here.
        boolean bIsIgnoreControlCharacters  = GetValueToUse( "IsIgnoreControlCharacters", true, aProperties );
        boolean bIsUseDictionaryList        = GetValueToUse( "IsUseDictionaryList", true, aProperties );
        boolean bIsGermanPreReform          = GetValueToUse( "IsGermanPreReform", false, aProperties );
        short   nHyphMinLeading             = GetValueToUse( "HyphMinLeading", (short)2, aProperties );
        short   nHyphMinTrailing            = GetValueToUse( "HyphMinTrailing", (short)2, aProperties );
        short   nHyphMinWordLen             = GetValueToUse( "HyphMinWordLength", (short)5, aProperties );

        XHyphenatedWord xRes = null;

        //!! This code needs to be replaced by code calling the actual
        //!! implementation of your hyphenator
        if ( IsEqual( aLocale, new Locale( "de", "DE", "" ) ) )
        {
            // there is an alternative spelling only when the
            // word is hyphenated between the "ff" and old german spelling
            // is set.
            if (aWord.equals( "Schiffahrt" ) &&
                bIsGermanPreReform && nIndex == 4)
            {
                xRes = new XHyphenatedWord_impl(aWord, aLocale,
                              (short)4, "Schifffahrt", (short)5 );
            }
        }
        else if ( IsEqual( aLocale, new Locale( "en", "US", "" ) ) )
        {
            // There are no alternative spellings in the English language
        }

        return xRes;
    }

    public XPossibleHyphens createPossibleHyphens(
            String aWord, Locale aLocale,
            PropertyValue[] aProperties )
        throws com.sun.star.uno.RuntimeException,
               IllegalArgumentException
    {
        if (IsEqual( aLocale, new Locale() ) || aWord.length() == 0)
            return null;

        // linguistic is currently not allowed to throw exceptions
        // thus we return null which means 'word cannot be hyphenated'
        if (!hasLocale( aLocale ))
            return null;

        // get values of relevant properties that may be used.
        //! The values for 'IsIgnoreControlCharacters' and 'IsUseDictionaryList'
        //! are handled by the dispatcher! Thus there is no need to access
        //! them here.
        boolean bIsIgnoreControlCharacters  = GetValueToUse( "IsIgnoreControlCharacters", true, aProperties );
        boolean bIsUseDictionaryList        = GetValueToUse( "IsUseDictionaryList", true, aProperties );
        boolean bIsGermanPreReform          = GetValueToUse( "IsGermanPreReform", false, aProperties );
        short   nHyphMinLeading             = GetValueToUse( "HyphMinLeading", (short)2, aProperties );
        short   nHyphMinTrailing            = GetValueToUse( "HyphMinTrailing", (short)2, aProperties );
        short   nHyphMinWordLen             = GetValueToUse( "HyphMinWordLength", (short)5, aProperties );

        XPossibleHyphens xRes = null;

        //!! This code needs to be replaced by code calling the actual
        //!! implementation of your hyphenator
        if ( IsEqual( aLocale, new Locale( "de", "DE", "" ) ) )
        {
            if (bIsGermanPreReform && aWord.equals( "Schiffahrt" ))
            {
                short aPos[] = new short[] { (short) 4 };
                xRes = new XPossibleHyphens_impl(aWord, aLocale,
                            "Schiff=fahrt", aPos);
            }
            else if (!bIsGermanPreReform && aWord.equals( "Schifffahrt" ))
            {
                short aPos[] = new short[] { (short) 5 };
                xRes = new XPossibleHyphens_impl(aWord, aLocale,
                            "Schiff=fahrt", aPos);
            }
        }
        else if ( IsEqual( aLocale, new Locale( "en", "US", "" ) ) )
        {
            if ( aWord.equals( "waterfall" ) )
            {
                short aPos[] = new short[]
                        { (short) 1, (short) 4 };
                xRes = new XPossibleHyphens_impl(aWord, aLocale,
                            "wa=ter=fall", aPos);
            }
            else if ( aWord.equals( "driving" ) )
            {
                short aPos[] = new short[]
                        { (short) 3 };
                xRes = new XPossibleHyphens_impl(aWord, aLocale,
                            "driv=ing", aPos);
            }
        }

        return xRes;
    }

    //*****************************
    //XLinguServiceEventBroadcaster
    //*****************************
    public boolean addLinguServiceEventListener (
            XLinguServiceEventListener xLstnr )
        throws com.sun.star.uno.RuntimeException
    {
        boolean bRes = false;
        if (!bDisposing && xLstnr != null)
            bRes = aPropChgHelper.addLinguServiceEventListener( xLstnr );
        return bRes;
    }

    public boolean removeLinguServiceEventListener(
            XLinguServiceEventListener xLstnr )
        throws com.sun.star.uno.RuntimeException
    {
        boolean bRes = false;
        if (!bDisposing && xLstnr != null)
            bRes = aPropChgHelper.removeLinguServiceEventListener( xLstnr );
        return bRes;
    }

    //********************
    // XServiceDisplayName
    //********************
    public String getServiceDisplayName( Locale aLocale )
        throws com.sun.star.uno.RuntimeException
    {
        return "Java Samples";
    }

    //****************
    // XInitialization
    //****************
    public void initialize( Object[] aArguments )
        throws com.sun.star.uno.Exception,
               com.sun.star.uno.RuntimeException
    {
        int nLen = aArguments.length;
        if (2 == nLen)
        {
            XPropertySet xPropSet = (XPropertySet)UnoRuntime.queryInterface(
                                         XPropertySet.class, aArguments[0]);
            // start listening to property changes
            aPropChgHelper.AddAsListenerTo( xPropSet );
        }
    }


    //*************
    // XServiceInfo
    //*************
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

    public static String _aSvcImplName = SampleHyphenator.class.getName();

    public static String[] getSupportedServiceNames_Static()
    {
        String[] aResult = { "com.sun.star.linguistic2.Hyphenator" };
        return aResult;
    }


    /**
     * Returns a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleServiceFactory</code> for creating the component
     * @param   implName     the name of the implementation for which a service is desired
     * @param   multiFactory the service manager to be used if needed
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleServiceFactory __getServiceFactory(
        String aImplName,
        XMultiServiceFactory xMultiFactory,
        com.sun.star.registry.XRegistryKey xRegKey )
    {
        XSingleServiceFactory xSingleServiceFactory = null;
        if( aImplName.equals( _aSvcImplName ) )
        {
            xSingleServiceFactory = new OneInstanceFactory(
                    SampleHyphenator.class, _aSvcImplName,
                    getSupportedServiceNames_Static(),
                    xMultiFactory );
        }
        return xSingleServiceFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   xRegKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    // This method not longer necessary since OOo 3.4 where the component registration
    // was changed to passive component registration. For more details see
    // http://wiki.services.openoffice.org/wiki/Passive_Component_Registration

//     public static boolean __writeRegistryServiceInfo(
//             com.sun.star.registry.XRegistryKey xRegKey )
//     {
//         boolean bResult = true;
//         String[] aServices = getSupportedServiceNames_Static();
//         int i, nLength = aServices.length;
//         for( i = 0; i < nLength; ++i )
//         {
//             bResult = bResult && com.sun.star.comp.loader.FactoryHelper.writeRegistryServiceInfo(
//                 _aSvcImplName, aServices[i], xRegKey );
//         }
//         return bResult;
//     }
}

