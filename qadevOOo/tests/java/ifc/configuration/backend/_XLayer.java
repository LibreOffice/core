/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XLayer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:17:10 $
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
