/*
 * _XSingleLayerStratum.java
 *
 * Created on 23. March 2004, 16:37
 */
package ifc.configuration.backend;
import com.sun.star.configuration.backend.XLayer;
import com.sun.star.configuration.backend.XSingleLayerStratum;
import com.sun.star.configuration.backend.XUpdatableLayer;

import lib.MultiMethodTest;

import util.XLayerHandlerImpl;


public class _XSingleLayerStratum extends MultiMethodTest {
    public XSingleLayerStratum oObj;

    public void _getLayer() {
        String aLayerID = "org.openoffice.Office.Common";
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

    public void _getUpdatableLayer() {
        String aLayerID = "org.openoffice.Office.Common";
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
