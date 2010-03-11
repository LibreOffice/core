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
import com.sun.star.configuration.backend.XMultiLayerStratum;
import com.sun.star.configuration.backend.XUpdatableLayer;
import com.sun.star.lang.XMultiServiceFactory;

import lib.MultiMethodTest;

import util.XLayerHandlerImpl;


public class _XMultiLayerStratum extends MultiMethodTest {
    public XMultiLayerStratum oObj;
    protected String aLayerID;

    public void _getLayer() {
        boolean res = true;

        try {
            XLayer aLayer = oObj.getLayer("", "");
            log.println("Exception expected -- FAILED");
            res = false;
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("expected Exception -- OK");
        }

        try {
            XLayer aLayer = oObj.getLayer(aLayerID, "");
            res &= (aLayer != null);

            if (aLayer == null) {
                log.println("\treturned Layer is NULL -- FAILED");
            }

            res &= checkLayer(aLayer);
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        }

        tRes.tested("getLayer()", res);
    }

    public void _getLayers() {
        boolean res = true;

        try {
            String[] LayerIds = new String[2];
            LayerIds[0] = "1 /org/openoffice/Office/Jobs.xcu";
            LayerIds[1] = "2 /org/openoffice/Office/Linguistic.xcu";

            XLayer[] Layers = oObj.getLayers(LayerIds, "");
            res = Layers.length == 2;
            log.println("Getting two XLayers -- OK");
            log.println("Checking first on "+LayerIds[0]);
            res &= checkLayer(Layers[0]);
            log.println("Checking second on "+LayerIds[1]);
            res &= checkLayer(Layers[1]);
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        }

        tRes.tested("getLayers()", res);
    }

    public void _getMultipleLayers() {
        boolean res = true;

        try {
            String[] LayerIds = new String[2];
            LayerIds[0] = "1 /org/openoffice/Office/Jobs.xcu";
            LayerIds[1] = "2 /org/openoffice/Office/Linguistic.xcu";
            String[] Times = new String[2];
            Times[0] = "";
            Times[1] = "";

            XLayer[] Layers = oObj.getMultipleLayers(LayerIds, Times);
            res = Layers.length == 2;
            log.println("Getting two XLayers -- OK");
            log.println("Checking first on "+LayerIds[0]);
            res &= checkLayer(Layers[0]);
            log.println("Checking second on "+LayerIds[1]);
            res &= checkLayer(Layers[1]);
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        }

        tRes.tested("getMultipleLayers()", res);
    }

    public void _getUpdatableLayer() {
        boolean res = true;

        try {
            XUpdatableLayer aLayer = oObj.getUpdatableLayer("");
            log.println("Exception expected -- FAILED");
            res = false;
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception " + e + " -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("expected Exception -- OK");
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        }

        try {
            XUpdatableLayer aLayer = oObj.getUpdatableLayer(aLayerID);
            res &= (aLayer != null);

            if (aLayer == null) {
                log.println("\treturned Layer is NULL -- FAILED");
            }

            res &= checkLayer(aLayer);
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        }

        tRes.tested("getUpdatableLayer()", res);
    }

    public void _getUpdateLayerId() {
        boolean res = true;

        try {
            String UpdateLayerID = oObj.getUpdateLayerId(
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
            log.println("unexpected Exception -- FAILED");
            res = false;
        }

        try {
            String ent = util.utils.getOfficeURL(
                                 (XMultiServiceFactory) tParam.getMSF()) +
                         "/../share/registry";
            String UpdateLayerID = oObj.getUpdateLayerId(
                                           "org.openoffice.Office.Linguistic", ent);
            res &= UpdateLayerID.endsWith("Linguistic.xcu");

            if (!UpdateLayerID.endsWith("Linguistic.xcu")) {
                log.println("\tExpected the id to end with Linguistic.xcu");
                log.println("\tBut got " + UpdateLayerID);
                log.println("\t=> FAILED");
            }
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception "+e+" -- FAILED");
            res = false;
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        }

        tRes.tested("getUpdateLayerId()", res);
    }

    public void _listLayerIds() {
        boolean res = true;

        try {
            String[] LayerIDs = oObj.listLayerIds(
                                        "org.openoffice.Office.TypeDetection",
                                        "illegal");
            log.println("Exception expected -- FAILED");
            res = false;
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("expected Exception -- OK");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        }

        try {
            String ent = util.utils.getOfficeURL(
                                 (XMultiServiceFactory) tParam.getMSF()) +
                         "/../share/registry";
            String[] LayerIDs = oObj.listLayerIds("org.openoffice.Office.Jobs",
                                                  ent);
            res &= LayerIDs[0].endsWith("Jobs.xcu");
            aLayerID = LayerIDs[0];

            if (!LayerIDs[0].endsWith("Jobs.xcu")) {
                log.println("\tExpected the id to end with Jobs.xcu");
                log.println("\tBut got " + LayerIDs[0]);
                log.println("\t=> FAILED");
            }
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("unexpected Exception -- FAILED");
            res = false;
        }

        tRes.tested("listLayerIds()", res);
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
}