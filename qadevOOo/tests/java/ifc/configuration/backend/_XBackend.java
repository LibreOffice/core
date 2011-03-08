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
            XUpdateHandler aHandler = oObj.getUpdateHandler(
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
                                              (XMultiServiceFactory) tParam.getMSF());
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
        }

        tRes.tested("getUpdateHandler()", res);
    }

    public void _listLayers() {
        boolean res = true;

        try {
            XStringSubstitution sts = createStringSubstitution(
                                              (XMultiServiceFactory) tParam.getMSF());
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

    public static XStringSubstitution createStringSubstitution(XMultiServiceFactory xMSF) {
        Object xPathSubst = null;

        try {
            xPathSubst = xMSF.createInstance(
                                 "com.sun.star.util.PathSubstitution");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        if (xPathSubst != null) {
            return (XStringSubstitution) UnoRuntime.queryInterface(
                           XStringSubstitution.class, xPathSubst);
        } else {
            return null;
        }
    }
}