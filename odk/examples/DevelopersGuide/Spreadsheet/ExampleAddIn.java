/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
import com.sun.star.sheet.XResultListener;

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

class ExampleAddInResult implements com.sun.star.sheet.XVolatileResult
{
    private final String aName;
    private int nValue;
    private final java.util.ArrayList<XResultListener> aListeners = new java.util.ArrayList<XResultListener>();

    public ExampleAddInResult( String aNewName )
    {
        aName = aNewName;
    }

    private com.sun.star.sheet.ResultEvent getResult()
    {
        com.sun.star.sheet.ResultEvent aEvent =
            new com.sun.star.sheet.ResultEvent();
        aEvent.Value = aName + " " + nValue;
        aEvent.Source = this;
        return aEvent;
    }

    public void addResultListener(com.sun.star.sheet.XResultListener aListener)
    {
        aListeners.add( aListener );

        // immediately notify of initial value
        aListener.modified( getResult() );
    }

    public void removeResultListener(com.sun.star.sheet.XResultListener aListener)
    {
        aListeners.remove( aListener );
    }

    public void incrementValue()
    {
        ++nValue;
        com.sun.star.sheet.ResultEvent aEvent = getResult();

        for( XResultListener l : aListeners)
            l.modified(aEvent);
    }
}

class ExampleAddInThread extends Thread
{
    private final java.util.HashMap<String, ExampleAddInResult> aCounters;

    public ExampleAddInThread( java.util.HashMap<String, ExampleAddInResult> aResults )
    {
        aCounters = aResults;
    }

    @Override
    public void run()
    {
        while ( true )
        {
            try
            {
                sleep(1000);
            }
            catch( InterruptedException exception )
            {
            }

            // increment all counters
            for (ExampleAddInResult r : aCounters.values())
                r.incrementValue();
        }
    }
}

public class ExampleAddIn
{
    public static class _ExampleAddIn extends com.sun.star.lib.uno.helper.WeakBase
           implements org.openoffice.sheet.addin.XExampleAddIn,
                      com.sun.star.sheet.XAddIn,
                      com.sun.star.lang.XServiceName,
                      com.sun.star.lang.XServiceInfo
    {
        private static final String aExampleService = "org.openoffice.sheet.addin.ExampleAddIn";
        private static final String aAddInService = "com.sun.star.sheet.AddIn";
        private static final String aImplName = _ExampleAddIn.class.getName();

        private static final short FUNCTION_INVALID   = -1;

        private static final String[] aFunctionNames =
        {
            "getIncremented",
            "getCounter"
        };
        private static final String[] aDisplayFunctionNames =
        {
            "Increment",
            "Counter"
        };
        private static final String[] aDescriptions =
        {
            "Increments a value",
            "Returns a counter"
        };
        private static final String[] aFirstArgumentNames =
        {
            "Value",
            "Name"
        };
        private static final String[] aFirstArgumentDescriptions =
        {
            "The value that is incremented",
            "The name of the counter"
        };

        private com.sun.star.lang.Locale aFuncLocale;
        private java.util.HashMap<String, ExampleAddInResult> aResults;

        public _ExampleAddIn( com.sun.star.lang.XMultiServiceFactory xFactory )
        {
        }

        private int getFunctionID( String aProgrammaticFunctionName )
        {
            for ( int i = 0; i < aFunctionNames.length; i++ )
                if ( aProgrammaticFunctionName.equals(aFunctionNames[i]) )
                    return i;
            return FUNCTION_INVALID;
        }

        //  XExampleAddIn

        public int getIncremented( int nValue )
        {
            return nValue + 1;
        }

        public com.sun.star.sheet.XVolatileResult getCounter(String aName)
        {
            if ( aResults == null )
            {
                // create the table of results, and start a thread to increment
                // all counters
                aResults = new java.util.HashMap<String, ExampleAddInResult>();
                ExampleAddInThread aThread = new ExampleAddInThread( aResults );
                aThread.start();
            }

            ExampleAddInResult aResult = aResults.get(aName);
            if ( aResult == null )
            {
                aResult = new ExampleAddInResult(aName);
                aResults.put( aName, aResult );
            }
            return aResult;
        }

        //  XAddIn

        public String getProgrammaticFuntionName(String aDisplayName)
        {
            for ( int i = 0; i < aFunctionNames.length; i++ )
                if ( aDisplayName.equals(aDisplayFunctionNames[i]) )
                    return aFunctionNames[i];
            return "";
        }

        public String getDisplayFunctionName(String aProgrammaticName)
        {
            int nFunction = getFunctionID( aProgrammaticName );
            return ( nFunction == FUNCTION_INVALID ) ? "" :
                aDisplayFunctionNames[nFunction];
        }

        public String getFunctionDescription(String aProgrammaticName)
        {
            int nFunction = getFunctionID( aProgrammaticName );
            return ( nFunction == FUNCTION_INVALID ) ? "" :
                aDescriptions[nFunction];
        }

        public String getDisplayArgumentName(String aProgrammaticFunctionName,
                                             int nArgument)
        {
            //  both functions in this example only have a first argument
            int nFunction = getFunctionID( aProgrammaticFunctionName );
            return ( nFunction == FUNCTION_INVALID || nArgument != 0) ? "" :
                aFirstArgumentNames[nFunction];
        }

        public String getArgumentDescription(String aProgrammaticFunctionName,
                                             int nArgument )
        {
            //  both functions in this example only have a first argument
            int nFunction = getFunctionID( aProgrammaticFunctionName );
            return ( nFunction == FUNCTION_INVALID || nArgument != 0) ? "" :
                aFirstArgumentDescriptions[nFunction];
        }

        public String getProgrammaticCategoryName(String aProgrammaticFunctionName)
        {
            return( "Add-In" );
        }

        public String getDisplayCategoryName(String aProgrammaticFunctionName)
        {
            return( "Add-In" );
        }

        //  XLocalizable

        public void setLocale( com.sun.star.lang.Locale aLocale )
        {
            // the locale is stored and used for getLocale, but otherwise
            // ignored in this example
            aFuncLocale = aLocale;
        }

        public com.sun.star.lang.Locale getLocale()
        {
            return aFuncLocale;
        }

        //  XServiceName

        public String getServiceName()
        {
            return aExampleService;
        }

        //  XServiceInfo

        public String getImplementationName()
        {
            return aImplName;
        }

        public String[] getSupportedServiceNames()
        {
            String [] aSupportedServices = new String[ 2 ];
            aSupportedServices[ 0 ] = aExampleService;
            aSupportedServices[ 1 ] = aAddInService;
            return aSupportedServices;
        }

        public boolean supportsService( String aService )
        {
            return (aService.equals( aExampleService ) ||
                    aService.equals( aAddInService ) );
        }

    }


    public static com.sun.star.lang.XSingleServiceFactory __getServiceFactory(
        String implName,
        com.sun.star.lang.XMultiServiceFactory multiFactory,
        com.sun.star.registry.XRegistryKey regKey)
    {
        com.sun.star.lang.XSingleServiceFactory xSingleServiceFactory = null;
        if ( implName.equals(_ExampleAddIn.aImplName) )
            xSingleServiceFactory =
                com.sun.star.comp.loader.FactoryHelper.getServiceFactory(
                    _ExampleAddIn.class, _ExampleAddIn.aExampleService,
                    multiFactory, regKey);
        return xSingleServiceFactory;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
