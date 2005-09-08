/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XBackend.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:16:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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