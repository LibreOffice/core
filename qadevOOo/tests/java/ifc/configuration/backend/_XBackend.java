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
import com.sun.star.configuration.backend.XLayer;
import com.sun.star.configuration.backend.XUpdateHandler;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XStringSubstitution;

import lib.MultiMethodTest;

import util.XLayerHandlerImpl;


public class _XBackend extends MultiMethodTest {
    public XBackend oObj;

    public void _getOwnUpdateHandler() {
        boolean res = true;

        String noUpdate = (String) tEnv.getObjRelation("noUpdate");

        if (noUpdate != null) {
            log.println(noUpdate);
            tRes.tested("getOwnUpdateHandler()", res);

            return;
        }

        try {
            XUpdateHandler aHandler = oObj.getOwnUpdateHandler(
                                              "org.openoffice.Office.Linguistic");
            res &= (aHandler != null);

            if (aHandler == null) {
                log.println("\treturned Layer is NULL -- FAILED");
            }
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        }

        tRes.tested("getOwnUpdateHandler()", res);
    }

    public void _getUpdateHandler() {
        boolean res = true;

        String noUpdate = (String) tEnv.getObjRelation("noUpdate");

        if (noUpdate != null) {
            log.println(noUpdate);
            tRes.tested("getUpdateHandler()", res);

            return;
        }

        try {
            oObj.getUpdateHandler(
                                              "org.openoffice.Office.TypeDetection",
                                              "illegal");
            log.println("Exception expected -- FAILED");
            res = false;
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("expected Exception -- OK");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        }

        try {
            XStringSubstitution sts = createStringSubstitution(
                                              tParam.getMSF());
            String ent = sts.getSubstituteVariableValue("$(inst)") +
                         "/share/registry";
            XUpdateHandler aHandler = oObj.getUpdateHandler(
                                              "org.openoffice.Office.Jobs",
                                              ent);

            if (aHandler == null) {
                log.println("\treturned Layer is NULL -- FAILED");
            }
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        } catch (com.sun.star.uno.Exception e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        }

        tRes.tested("getUpdateHandler()", res);
    }

    public void _listLayers() {
        boolean res = true;

        try {
            XStringSubstitution sts = createStringSubstitution(tParam.getMSF());
            String ent = sts.getSubstituteVariableValue("$(inst)") +
                         "/share/registry";
            XLayer[] Layers = oObj.listLayers(
                                      "org.openoffice.Office.Linguistic", ent);

            for (int i = 0; i < Layers.length; i++) {
                log.println("Checking Layer " + i);
                res &= checkLayer(Layers[i]);
            }
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        } catch (com.sun.star.uno.Exception e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        }

        tRes.tested("listLayers()", res);
    }

    public void _listOwnLayers() {
        boolean res = true;

        try {
            XLayer[] Layers = oObj.listOwnLayers(
                                      "org.openoffice.Office.Common");

            for (int i = 0; i < Layers.length; i++) {
                log.println("Checking Layer " + i);
                res &= checkLayer(Layers[i]);
            }
            if (Layers.length==0) {
                System.out.println("No Layers found -- FAILED");
                res &= false;
            }
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        }

        tRes.tested("listOwnLayers()", res);
    }

    protected boolean checkLayer(XLayer aLayer) {
        boolean res = false;

        log.println("Checking for Exception in case of null argument");

        try {
            aLayer.readData(null);
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Expected Exception -- OK");
            res = true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        }

        log.println("checking read data with own XLayerHandler implementation");

        try {
            XLayerHandlerImpl xLayerHandler = new XLayerHandlerImpl();
            aLayer.readData(xLayerHandler);

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
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        }

        return res;
    }

    public static XStringSubstitution createStringSubstitution(XMultiServiceFactory xMSF) throws com.sun.star.uno.Exception
    {
        Object xPathSubst = xMSF.createInstance(
                    "com.sun.star.util.PathSubstitution");
        return UnoRuntime.queryInterface(XStringSubstitution.class, xPathSubst);
    }
}
