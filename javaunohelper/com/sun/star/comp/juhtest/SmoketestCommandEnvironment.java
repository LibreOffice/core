/*************************************************************************
 *
 *  $RCSfile: SmoketestCommandEnvironment.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 17:05:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
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
