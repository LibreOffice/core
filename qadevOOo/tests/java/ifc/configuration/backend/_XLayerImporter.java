/*************************************************************************
 *
 *  $RCSfile: _XLayerImporter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:21:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
