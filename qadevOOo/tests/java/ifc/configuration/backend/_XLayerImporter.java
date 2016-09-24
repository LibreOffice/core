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

import com.sun.star.configuration.backend.XBackend;
import com.sun.star.configuration.backend.XLayerImporter;
import lib.MultiMethodTest;

public class _XLayerImporter extends MultiMethodTest {

    public XLayerImporter oObj;
    public XBackend xBackend = null;

    public void _getTargetBackend() {
        xBackend = oObj.getTargetBackend();
        tRes.tested("getTargetBackend()", xBackend != null);
    }

    public void _importLayer() {
        boolean res = false;
        log.println("checking for exception is argument null is given");
        try {
            oObj.importLayer(null);
            log.println("\tException expected -- FAILED");
        } catch (com.sun.star.lang.NullPointerException ne) {
            res = true;
            log.println("\tExpected exception was thrown -- OK");
        } catch (com.sun.star.configuration.backend.MalformedDataException mde) {
            res = false;
            log.println("\tWrong Expected "+mde+" exception was thrown -- FAILED");
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            res = false;
            log.println("\tWrong Expected "+iae+" exception was thrown -- FAILED");
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            res = false;
            log.println("\tWrong Expected "+wte+" exception was thrown -- FAILED");
        }

        log.println("checking own implementation of XLayer");
        try {
            util.XLayerImpl xLayer = new util.XLayerImpl();
            oObj.importLayer(xLayer);
            if (! xLayer.hasBeenCalled()) {
                log.println("\tXLayer hasn't been imported -- FAILED");
                res &= false;
            } else {
                log.println("\tXLayer has been imported -- OK");
                res &= true;
            }
        } catch (com.sun.star.lang.NullPointerException ne) {
            res &= false;
            log.println("\tExpected exception "+ne+" was thrown -- FAILED");
        } catch (com.sun.star.configuration.backend.MalformedDataException mde) {
            res &= false;
            log.println("\tWrong Expected "+mde+" exception was thrown -- FAILED");
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            res &= false;
            log.println("\tWrong Expected "+iae+" exception was thrown -- FAILED");
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            res &= false;
            log.println("\tWrong Expected "+wte+" exception was thrown -- FAILED");
        }

        tRes.tested("importLayer()",res);
    }

    public void _importLayerForEntity() {
        boolean res = false;
        log.println("checking for exception for argument (null,\"\")");
        try {
            oObj.importLayerForEntity(null,"");
            log.println("\tException expected -- FAILED");
        } catch (com.sun.star.lang.NullPointerException ne) {
            res = true;
            log.println("\tExpected exception was thrown -- OK");
        } catch (com.sun.star.configuration.backend.MalformedDataException mde) {
            res = false;
            log.println("\tWrong Expected "+mde+" exception was thrown -- FAILED");
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            res = false;
            log.println("\tWrong Expected "+iae+" exception was thrown -- FAILED");
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            res = false;
            log.println("\tWrong Expected "+wte+" exception was thrown -- FAILED");
        }

        log.println("checking own implementation of XLayer");
        try {
            util.XLayerImpl xLayer = new util.XLayerImpl();
            oObj.importLayerForEntity(xLayer,"");
            if (! xLayer.hasBeenCalled()) {
                log.println("\tXLayer hasn't been imported -- FAILED");
                res &= false;
            } else {
                log.println("\tXLayer has been imported -- OK");
                res &= true;
            }
        } catch (com.sun.star.lang.NullPointerException ne) {
            res &= false;
            log.println("\tExpected exception "+ne+" was thrown -- FAILED");
        } catch (com.sun.star.configuration.backend.MalformedDataException mde) {
            res &= false;
            log.println("\tWrong Expected "+mde+" exception was thrown -- FAILED");
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            res &= false;
            log.println("\tWrong Expected "+iae+" exception was thrown -- FAILED");
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            res &= false;
            log.println("\tWrong Expected "+wte+" exception was thrown -- FAILED");
        }

        tRes.tested("importLayerForEntity()",res);
    }

    public void _setTargetBackend() {
        requiredMethod("getTargetBackend()");
        boolean res = false;
        log.println("checking for exception if argument null is given");
        try {
            oObj.setTargetBackend(null);
            log.println("\tException expected -- FAILED");
        } catch (com.sun.star.lang.NullPointerException ne) {
            res = true;
            log.println("\tExpected exception was thrown -- OK");
        }

        log.println("checking argument previously gained by getTargetBackend");
        try {
            oObj.setTargetBackend(xBackend);
            log.println("\t No Exception thrown -- OK");
            res &= true;
        } catch (com.sun.star.lang.NullPointerException ne) {
            res &= false;
            log.println("\tException was thrown -- FAILED");
        }

        tRes.tested("setTargetBackend()",res);

    }

}
