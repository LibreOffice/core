/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.libreoffice.smoketest;

import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.ucb.XCommandEnvironment;

/** This service is for use by the smoketest which checks the installation of
 * extensions. The service provides the XCommandEnvironment interface, which
 * is needed for adding extensions.
 */
public class SmoketestCommandEnvironment extends WeakBase
    implements XServiceInfo, XCommandEnvironment {

    private static final String __serviceName =
    "com.sun.star.deployment.test.SmoketestCommandEnvironment";

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
        boolean approve = true;

        com.sun.star.task.XInteractionContinuation[] conts = xRequest.getContinuations();
        for (int i = 0; i < conts.length; i++)
        {
            if (approve)
            {
                com.sun.star.task.XInteractionApprove xApprove =
                    UnoRuntime.queryInterface(com.sun.star.task.XInteractionApprove.class, conts[i]);
                if (xApprove != null)
                    xApprove.select();
                //don't query again for ongoing extensions
                approve = false;
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
