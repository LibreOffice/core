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
