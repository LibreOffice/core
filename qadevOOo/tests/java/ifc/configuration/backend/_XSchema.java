/*************************************************************************
 *
 *  $RCSfile: _XSchema.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:21:35 $
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

import com.sun.star.configuration.backend.XSchema;

import lib.MultiMethodTest;

import util.XSchemaHandlerImpl;

public class _XSchema extends MultiMethodTest {
    public XSchema oObj;
    XSchemaHandlerImpl xSchemaHandlerImpl = new XSchemaHandlerImpl();

    public void _readComponent() {
        requiredMethod("readTemplates()");
        boolean res = false;

        log.println("Checking for Exception in case of null argument");

        try {
            oObj.readComponent(null);
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Expected Exception -- OK");
            res = true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        }

        log.println(
                "checking readComponent with own XSchemeHandler implementation");

        try {
            xSchemaHandlerImpl.cleanCalls();
            oObj.readComponent(xSchemaHandlerImpl);

            String implCalled = xSchemaHandlerImpl.getCalls();

            System.out.println(implCalled);

            int sc = implCalled.indexOf("startComponent");

            if (sc < 0) {
                log.println("startComponent wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("startComponent was called -- OK");
                res &= true;
            }

            int ec = implCalled.indexOf("endComponent");

            if (ec < 0) {
                log.println("endComponent wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("endComponent was called -- OK");
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

        tRes.tested("readComponent()", res);
    }

    public void _readSchema() {
        requiredMethod("readComponent()");
        boolean res = false;

        log.println("Checking for Exception in case of null argument");

        try {
            xSchemaHandlerImpl.cleanCalls();
            oObj.readSchema(null);
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Expected Exception -- OK");
            res = true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        }

        log.println(
                "checking read data with own XSchemeHandler implementation");

        try {
            xSchemaHandlerImpl.cleanCalls();
            oObj.readSchema(xSchemaHandlerImpl);

            String implCalled = xSchemaHandlerImpl.getCalls();

            int sc = implCalled.indexOf("startSchema");

            if (sc < 0) {
                log.println("startSchema wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("startSchema was called -- OK");
                res &= true;
            }

            int ec = implCalled.indexOf("endSchema");

            if (ec < 0) {
                log.println("endSchema wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("endSchema was called -- OK");
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

        tRes.tested("readSchema()", res);
    }

    public void _readTemplates() {
        boolean res = false;

        log.println("Checking for Exception in case of null argument");

        try {
            oObj.readTemplates(null);
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Expected Exception -- OK");
            res = true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        }

        log.println(
                "checking readComponent with own XSchemeHandler implementation");

        try {
            xSchemaHandlerImpl.cleanCalls();
            oObj.readComponent(xSchemaHandlerImpl);

            String implCalled = xSchemaHandlerImpl.getCalls();

            int sc = implCalled.indexOf("startGroup");

            if (sc < 0) {
                log.println("startGroup wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("startGroup was called -- OK");
                res &= true;
            }

            int ec = implCalled.indexOf("endGroup");

            if (ec < 0) {
                log.println("endGroup wasn't called -- FAILED");
                res &= false;
            } else {
                log.println("endGroup was called -- OK");
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

        tRes.tested("readTemplates()", res);

    }
}