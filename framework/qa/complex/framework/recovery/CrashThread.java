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
    private final XComponent xDoc;
    private final XMultiServiceFactory msf;

    public CrashThread(XComponent xDoc, XMultiServiceFactory msf) {
        this.xDoc = xDoc;
        this.msf = msf;
    }

    @Override
    public void run() {
        try{
            XModel xModel = UnoRuntime.queryInterface(XModel.class, xDoc);

            XController xController = xModel.getCurrentController();
            XDispatchProvider xDispProv = UnoRuntime.queryInterface(
                                                  XDispatchProvider.class,
                                                  xController);
            XURLTransformer xParser = UnoRuntime.queryInterface(
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
