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

package com.sun.star.wizards.web;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Desktop;

/**
 * The last class in the WebWizard Dialog class hirarchy.
 * Has no functionality, is just nice to have it instanciated.
 * @author  rpiterman
 */
public class WebWizard extends WWD_Events
{

    public WebWizard(XMultiServiceFactory xmsf) throws Exception
    {
        super(xmsf);
    }

    public static void main(String args[])
    {

        String ConnectStr =
                "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.ServiceManager";
        try
        {
            XMultiServiceFactory xmsf = Desktop.connect(ConnectStr);
            WebWizard ww = new WebWizard(xmsf);
            ww.show();
            ww.cleanup();

        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }
}
