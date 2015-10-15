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
import com.sun.star.configuration.backend.XMultiLayerStratum;
import com.sun.star.configuration.backend.XUpdatableLayer;
import lib.MultiMethodTest;

import util.XLayerHandlerImpl;


public class _XMultiLayerStratum extends MultiMethodTest {
    public XMultiLayerStratum oObj;
    protected String aLayerID;

    public void _getLayer() {
        boolean res = true;

        try {
            oObj.getLayer("", "");
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
            } else {
                res &= checkLayer(aLayer);
            }
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
            String[] LayerIds = new String[] {
                "1 /org/openoffice/Office/Jobs.xcu",
                "2 /org/openoffice/Office/Linguistic.xcu" };

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
            String[] LayerIds = new String[] {
                "1 /org/openoffice/Office/Jobs.xcu",
                "2 /org/openoffice/Office/Linguistic.xcu" };
            String[] Times = new String[] { "", "" };

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
            oObj.getUpdatableLayer("");
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
            oObj.getUpdateLayerId(
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
                                 tParam.getMSF()) +
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
            oObj.listLayerIds(
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
                                 tParam.getMSF()) +
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
