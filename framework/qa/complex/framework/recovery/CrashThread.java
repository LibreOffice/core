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

package complex.framework.recovery;

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
 * Thread to crash the office. This thread dies after the office process
 * is nopt longer available.
 */
public class CrashThread extends Thread {
    public XComponent xDoc = null;
    public XMultiServiceFactory msf = null;

    public CrashThread(XComponent xDoc, XMultiServiceFactory msf) {
        this.xDoc = xDoc;
        this.msf = msf;
    }

    public void run() {
        try{
            XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, xDoc);

            XController xController = xModel.getCurrentController();
            XDispatchProvider xDispProv = (XDispatchProvider) UnoRuntime.queryInterface(
                                                  XDispatchProvider.class,
                                                  xController);
            XURLTransformer xParser = (XURLTransformer) UnoRuntime.queryInterface(
                                              XURLTransformer.class,
                                              msf.createInstance(
                                                      "com.sun.star.util.URLTransformer"));

            // Because it's an in/out parameter we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = ".uno:Crash";
            xParser.parseStrict(aParseURL);

            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);

            if (xDispatcher != null) {
                xDispatcher.dispatch(aURL, null);
            }
        } catch (Exception e){}
    }
}
