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

package mod._ucpdav;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Creates a <code>com.sun.star.ucb.WebDAVContentProvider</code> instance.
 */
public class WebDAVContentProvider extends TestCase {
    /**
     * Creates a <code>com.sun.star.ucb.WebDAVContentProvider</code> instance
     * and adds a <code>XContentIdentifierFactory</code> instance as a "FACTORY"
     * relation for <code>XContentProviderTest</code>.
     */
    public TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XInterface xInt;

        log.println("creating WebDAVContentProvider instance");
        try {
            xInt = (XInterface)xMSF.createInstance(
                    "com.sun.star.ucb.WebDAVContentProvider");
        } catch (Exception e) {
            log.println("Unexpected exception " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        TestEnvironment tEnv = new TestEnvironment(xInt);

        log.println("creating XContentIdentifierFactory");
        XContentIdentifierFactory xCIF;

        try {
            xCIF = (XContentIdentifierFactory)UnoRuntime.queryInterface(
                    XContentIdentifierFactory.class,
                    xMSF.createInstance(
                            "com.sun.star.comp.ucb.UniversalContentBroker"));
        } catch (Exception e) {
            log.println("Unexpected exception " + e.getMessage());
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        tEnv.addObjRelation("FACTORY", xCIF);

        tEnv.addObjRelation("CONTENT1",
            "vnd.sun.star.webdav://localhost/davhome/");

        return tEnv;
    }
}
