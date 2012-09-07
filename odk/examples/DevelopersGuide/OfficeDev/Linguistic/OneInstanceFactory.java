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

import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

import java.lang.reflect.Constructor;

//
// purpose of this class is to provide a service factory that instantiates
// the services only once (as long as this factory itself exists)
// and returns only reference to that instance.
//

public class OneInstanceFactory implements
        XSingleServiceFactory,
        XServiceInfo
{
    Class       aMyClass;
    String      aSvcImplName;
    String[]    aSupportedSvcNames;
    XInterface  xInstantiatedService;
    XMultiServiceFactory    xMultiFactory;

    public OneInstanceFactory(
            Class       aMyClass,
            String      aSvcImplName,
            String[]    aSupportedSvcNames,
            XMultiServiceFactory    xMultiFactory )
    {
        this.aMyClass           = aMyClass;
        this.aSvcImplName       = aSvcImplName;
        this.aSupportedSvcNames = aSupportedSvcNames;
        this.xMultiFactory      = xMultiFactory;
        xInstantiatedService = null;
    }

    //**********************
    // XSingleServiceFactory
    //**********************
    public Object createInstance()
        throws com.sun.star.uno.Exception,
               com.sun.star.uno.RuntimeException
    {
        if (xInstantiatedService == null)
        {
            //!! the here used services all have exact one constructor!!
            Constructor [] aCtor = aMyClass.getConstructors();
            try {
                xInstantiatedService = (XInterface) aCtor[0].newInstance( (Object[])null );
            }
            catch( Exception e ) {
            }

            //!! workaround for services not always being created
            //!! via 'createInstanceWithArguments'
            XInitialization xIni = (XInitialization) UnoRuntime.queryInterface(
                XInitialization.class, createInstance());
            if (xIni != null)
            {
                Object[] aArguments = new Object[]{ null, null };
                if (xMultiFactory != null)
                {
                    XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface(
                        XPropertySet.class ,  xMultiFactory.createInstance(
                            "com.sun.star.linguistic2.LinguProperties" ) );
                    aArguments[0] = xPropSet;
                }
                xIni.initialize( aArguments );
            }
        }
        return xInstantiatedService;
    }

    public Object createInstanceWithArguments( Object[] aArguments )
        throws com.sun.star.uno.Exception,
               com.sun.star.uno.RuntimeException
    {
        if (xInstantiatedService == null)
        {
            XInitialization xIni = (XInitialization) UnoRuntime.queryInterface(
                XInitialization.class, createInstance());
            if (xIni != null)
                xIni.initialize( aArguments );
        }
        return xInstantiatedService;
    }


    //*************
    // XServiceInfo
    //*************
    public boolean supportsService( String aServiceName )
        throws com.sun.star.uno.RuntimeException
    {
        boolean bFound = false;
        int nCnt = aSupportedSvcNames.length;
        for (int i = 0;  i < nCnt && !bFound;  ++i)
        {
            if (aServiceName.equals( aSupportedSvcNames[i] ))
                bFound = true;
        }
        return bFound;
    }

    public String getImplementationName()
        throws com.sun.star.uno.RuntimeException
    {
        return aSvcImplName;
    }

    public String[] getSupportedServiceNames()
        throws com.sun.star.uno.RuntimeException
    {
        return aSupportedSvcNames;
    }
};

