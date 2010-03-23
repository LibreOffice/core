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

package mod._sfx;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.frame.XStorable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.frame.DocumentTemplates</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::frame::XDocumentTemplates</code></li>
 * </ul> <p>
 *
 */
public class DocumentTemplates extends TestCase {

    XTextDocument xTextDoc;

    protected void initialize( TestParameters tParam, PrintWriter log ) {
        String aURL=util.utils.getFullTestURL("report2.stw");
        xTextDoc = util.WriterTools.loadTextDoc((XMultiServiceFactory)tParam.getMSF(),aURL);
    }

    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        xTextDoc.dispose();
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.frame.DocumentTemplates</code>.
    */
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XInterface oObj = null;
        XStorable store = null;

        try {
            oObj = (XInterface)xMSF.createInstance
                ("com.sun.star.frame.DocumentTemplates");
            store = (XStorable) UnoRuntime.queryInterface
                (XStorable.class,xTextDoc);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        String Iname = util.utils.getImplName(oObj);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(oObj);
        tEnv.addObjRelation("Store",store);
        return tEnv;
    }

}    // finish class TestCase

