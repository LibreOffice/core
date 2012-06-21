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
package complex.sfx2.tools;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

/**
 * This class opens a given dialog in a separate Thread by dispatching an url
 *
 */
public class DialogThread extends Thread {
    public XComponent m_xDoc = null;
    public XMultiServiceFactory m_xMSF = null;
    public String m_url = "";

    public DialogThread(XComponent xDoc, XMultiServiceFactory msf, String url) {
        this.m_xDoc = xDoc;
        this.m_xMSF = msf;
        this.m_url = url;
    }

    @Override
    public void run() {
        XModel aModel = UnoRuntime.queryInterface( XModel.class, m_xDoc );

        XController xController = aModel.getCurrentController();

        //Opening Dialog
        try {
            XDispatchProvider xDispProv = UnoRuntime.queryInterface( XDispatchProvider.class, xController.getFrame() );
            XURLTransformer xParser = UnoRuntime.queryInterface( XURLTransformer.class,
                m_xMSF.createInstance( "com.sun.star.util.URLTransformer" ) );

            // Because it's an in/out parameter
            // we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = m_url;
            xParser.parseStrict(aParseURL);

            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", com.sun.star.frame.FrameSearchFlag.SELF |
                                    com.sun.star.frame.FrameSearchFlag.CHILDREN);
            PropertyValue[] dispatchArguments = new PropertyValue[0];

            if (xDispatcher != null) {
                xDispatcher.dispatch(aURL, dispatchArguments);
            } else {
                System.out.println("xDispatcher is null");
            }
        } catch (com.sun.star.uno.Exception e) {
            System.out.println("Couldn't open dialog");
        }
    }
}