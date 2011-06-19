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