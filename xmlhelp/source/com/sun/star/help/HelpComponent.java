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

package com.sun.star.help;

import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.registry.XRegistryKey;

/** This class capsulates the class, that implements the minimal component, a
 * factory for creating the service (<CODE>__getComponentFactory</CODE>) and a
 * method, that writes the information into the given registry key
 * (<CODE>__writeRegistryServiceInfo</CODE>).
 */
public class HelpComponent
{
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

        if ( sImplName.equals( HelpSearch._HelpSearch.class.getName() ) )
            xFactory = Factory.createComponentFactory(HelpSearch._HelpSearch.class,
                                             HelpSearch._HelpSearch.getServiceNames());
        else if ( sImplName.equals( HelpIndexer.class.getName() ) )
            xFactory = Factory.createComponentFactory(HelpIndexer.class,
                                             HelpIndexer.getServiceNames());
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
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey)
    {
        boolean bSuccessHelpSearch = Factory.writeRegistryServiceInfo
            (HelpSearch._HelpSearch.class.getName(),
             HelpSearch._HelpSearch.getServiceNames(), regKey);
        boolean bSuccessHelpIndexer = Factory.writeRegistryServiceInfo
            (HelpIndexer.class.getName(),
             HelpIndexer.getServiceNames(), regKey);

        return bSuccessHelpSearch && bSuccessHelpIndexer;
    }
    /** This method is a member of the interface for initializing an object
     * directly after its creation.
     * @param object This array of arbitrary objects will be passed to the
     * component after its creation.
     * @throws Exception Every exception will not be handled, but will be
     * passed to the caller.
     */
    public void initialize( Object[] object )
        throws com.sun.star.uno.Exception
    {
        /* The component describes what arguments its expected and in which
         * order!At this point you can read the objects and can intialize
         * your component using these objects.
         */
    }
}
