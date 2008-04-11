/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JavaLoaderFactory.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.comp.loader;

import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.ServiceNotRegisteredException;
import com.sun.star.lang.WrappedTargetException;

import com.sun.star.uno.RuntimeException;
import com.sun.star.uno.Exception;

public class JavaLoaderFactory implements XSingleServiceFactory, XServiceInfo {

    private static String[] supportedServices = {
        "com.sun.star.loader.Java",
        "com.sun.star.loader.Java2"
    };

    private static final boolean DEBUG = false;

    private static final void DEBUG(String dbg) {
        if (DEBUG)
            System.err.println(" >>> JavaLoaderFactory - " + dbg);
    }

    protected XMultiServiceFactory multiServiceFactory = null;

    /** default constructor
     */
//      public JavaLoaderFactory() {}

    public JavaLoaderFactory(XMultiServiceFactory factory) {
        multiServiceFactory = factory;
    }

    public java.lang.Object createInstance()
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        return new JavaLoader(multiServiceFactory);
    }

    public java.lang.Object createInstanceWithArguments( java.lang.Object[] args )
            throws com.sun.star.uno.Exception,
                   com.sun.star.uno.RuntimeException
    {
        JavaLoader loader = new JavaLoader();
        loader.initialize(args);

        return loader;
    }

    /** implements the XServiceInfo interface
     */
    public String getImplementationName()
            throws com.sun.star.uno.RuntimeException
    {
        return JavaLoader.class.getName();
    }

    /** implements the XServiceInfo interface
     */
    public boolean supportsService(String serviceName)
            throws com.sun.star.uno.RuntimeException
    {
        for ( int i = 0; i < supportedServices.length; i++ ) {
            if ( supportedServices[i].equals(serviceName) )
                return true;
        }
        return false;
    }

    /** implements the XServiceInfo interface
     */
    public String[] getSupportedServiceNames()
            throws com.sun.star.uno.RuntimeException
    {
        return supportedServices;
    }
}

