/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JavaUNOHelperServices.java,v $
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
package com.sun.star.comp;

import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.Type;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.comp.juhtest.SmoketestCommandEnvironment;


/** This class is the registration class for all services in this module.
 *
 * Note that not all files which can be used as service need to be registered
 * by this class.
 */
public class JavaUNOHelperServices {

        static private final String __service_smoketestCommandEnv =
        "com.sun.star.deployment.test.SmoketestCommandEnvironment";

     /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns a <code>XSingleComponentFactory</code> for creating
     *          the component
     * @param   sImplName the name of the implementation for which a
     *          service is desired
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleComponentFactory __getComponentFactory(String sImplName)
    {
        XSingleComponentFactory xFactory = null;

        if ( sImplName.equals( SmoketestCommandEnvironment.class.getName() ) )
            xFactory = Factory.createComponentFactory(SmoketestCommandEnvironment.class,
                                            SmoketestCommandEnvironment.getServiceNames());

        return xFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey the registryKey
     * @see     com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return Factory.writeRegistryServiceInfo(SmoketestCommandEnvironment.class.getName(),
                                                SmoketestCommandEnvironment.getServiceNames(),
                                                regKey);
    }
}


