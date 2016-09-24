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

package ifc.configuration.backend;

import com.sun.star.configuration.backend.XLayer;
import lib.MultiMethodTest;
import util.XLayerHandlerImpl;

public class _XLayer extends MultiMethodTest {

    public XLayer oObj;

    public void _readData() {
        boolean res = false;

        log.println("Checking for Exception in case of nul argument");

        try {
            oObj.readData(null);
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Expected Exception -- OK");
            res = true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception ("+e+") -- FAILED");
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception ("+e+") -- FAILED");
        }

        log.println("checking read data with own XLayerHandler implementation");
        try {
            XLayerHandlerImpl xLayerHandler = new XLayerHandlerImpl();
            oObj.readData(xLayerHandler);
            String implCalled = xLayerHandler.getCalls();
            log.println(implCalled);
            int sl = implCalled.indexOf("startLayer");
            if (sl < 0) {
                log.println("startLayer wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("startLayer was called -- OK");
                res &= true;
            }
            int el = implCalled.indexOf("endLayer");
            if (el < 0) {
                log.println("endLayer wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("endLayer was called -- OK");
                res &= true;
            }
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Unexpected Exception ("+e+") -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception ("+e+") -- FAILED");
            res &= false;
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception ("+e+") -- FAILED");
            res &= false;
        }

        tRes.tested("readData()",res);
    }

}
