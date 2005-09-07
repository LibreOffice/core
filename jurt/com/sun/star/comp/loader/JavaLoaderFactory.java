/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JavaLoaderFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:51:38 $
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

