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

import com.sun.star.configuration.backend.XSchema;
import com.sun.star.configuration.backend.XSchemaSupplier;

import lib.MultiMethodTest;
import util.XSchemaHandlerImpl;

public class _XSchemaSupplier extends MultiMethodTest {
    public XSchemaSupplier oObj;

    public void _getComponentSchema() {
        boolean res = true;
        XSchema aSchema = null;

        try {
            aSchema = oObj.getComponentSchema("org.openoffice.Office.Linguistic");
            res &= (aSchema != null);

            if (aSchema == null) {
                log.println("\treturned Layer is NULL -- FAILED");
            }

            res &= checkSchema(aSchema);
        } catch (com.sun.star.configuration.backend.BackendAccessException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
            res &= false;
        }
        tRes.tested("getComponentSchema()",res);
    }

    protected boolean checkSchema(XSchema aSchema) {
        boolean res = false;
        XSchemaHandlerImpl xSchemaHandlerImpl = new XSchemaHandlerImpl();
        log.println("Checking for Exception in case of null argument");

        try {
            aSchema.readTemplates(null);
            log.println("NoException thrown for null argument -- FAILED");
        } catch (com.sun.star.lang.NullPointerException e) {
            log.println("Expected Exception -- OK");
            res = true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        } catch (com.sun.star.configuration.backend.MalformedDataException e) {
            log.println("Unexpected Exception (" + e + ") -- FAILED");
        }

        log.println(
                "checking readComponent with own XSchemaHandler implementation");

        try {
            aSchema.readComponent(xSchemaHandlerImpl);

            String implCalled = xSchemaHandlerImpl.getCalls();
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

        return res;

    }
}