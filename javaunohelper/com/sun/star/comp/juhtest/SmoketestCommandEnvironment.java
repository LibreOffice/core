/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SmoketestCommandEnvironment.java,v $
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
package com.sun.star.comp.juhtest;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.ucb.XCommandEnvironment;

/** This service is for use by the smoketest which checks the installation of
 * extensions. The service provides the XCommandEnvironment interface, which
 * is needed for adding extensions.
 */
public class SmoketestCommandEnvironment extends WeakBase
    implements XServiceInfo, XCommandEnvironment {

    static private final String __serviceName =
    "com.sun.star.deployment.test.SmoketestCommandEnvironment";

    private XComponentContext m_cmpCtx;
    private XMultiComponentFactory m_xMCF;


    public SmoketestCommandEnvironment(XComponentContext xCompContext) {
        try {
            m_cmpCtx = xCompContext;
            m_xMCF = m_cmpCtx.getServiceManager();
        }
        catch( Exception e ) {
            e.printStackTrace();
        }
    }

    public static String[] getServiceNames() {
        String[] sSupportedServiceNames = { __serviceName};
        return sSupportedServiceNames;
    }

    //XServiceInfo -------------------------------------------------------------
    public String[] getSupportedServiceNames() {
        return getServiceNames();
    }


    public boolean supportsService( String sServiceName ) {
        boolean bSupported = false;
        if (sServiceName.equals(__serviceName))
            bSupported = true;
        return bSupported;
    }

    public String getImplementationName() {
        return  SmoketestCommandEnvironment.class.getName();
    }

    //XCommandEnvironment ================================================
    public com.sun.star.task.XInteractionHandler getInteractionHandler()
    {
        return new InteractionImpl();
    }

    public com.sun.star.ucb.XProgressHandler getProgressHandler()
    {
        return new ProgressImpl();
    }
}




class InteractionImpl implements com.sun.star.task.XInteractionHandler
{
    public void handle( com.sun.star.task.XInteractionRequest xRequest )
    {
        Object request = xRequest.getRequest();

        boolean approve = true;
        boolean abort = false;
//             Object install_Exception =
//                 AnyConverter.toObject(
//                     com.sun.star.deployment.InstallException.class, request);
//             if (install_Exception != null)
//             {
//                 approve = true;
//             }

        com.sun.star.task.XInteractionContinuation[] conts = xRequest.getContinuations();
        for (int i = 0; i < conts.length; i++)
        {
            if (approve)
            {
                com.sun.star.task.XInteractionApprove xApprove =
                    (com.sun.star.task.XInteractionApprove)
                    UnoRuntime.queryInterface(com.sun.star.task.XInteractionApprove.class, conts[i]);
                if (xApprove != null)
                    xApprove.select();
                //don't query again for ongoing extensions
                approve = false;
            }
            else if (abort)
            {
                com.sun.star.task.XInteractionAbort xAbort =
                    (com.sun.star.task.XInteractionAbort)
                    UnoRuntime.queryInterface(com.sun.star.task.XInteractionAbort.class, conts[i]);
                if (xAbort != null)
                    xAbort.select();
                //don't query again for ongoing extensions
                abort = false;
            }
        }
    }
}

class ProgressImpl implements com.sun.star.ucb.XProgressHandler
{
    public void push(Object status)
    {
    }

    public void update(Object status)
    {
    }

    public void pop()
    {
    }
}
