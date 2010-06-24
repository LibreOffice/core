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
import util.utils;

import com.sun.star.document.XStandaloneDocumentInfo;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.document.StandaloneDocumentInfo</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::document::DocumentInfo</code></li>
 *  <li> <code>com::sun::star::document::XDocumentInfo</code></li>
 *  <li> <code>com::sun::star::document::XStandaloneDocumentInfo</code></li>
 *  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 * </ul> <p>
 * The following files used by this test :
 * <ul>
 *  <li><b> SfxStandaloneDocInfoObject.sdw </b> : is copied
 *   to SOffice temporary directory and is used to load and
 *   save its info. </li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.document.DocumentInfo
 * @see com.sun.star.document.XDocumentInfo
 * @see com.sun.star.document.XStandaloneDocumentInfo
 * @see com.sun.star.beans.XFastPropertySet
 * @see com.sun.star.beans.XPropertySet
 * @see ifc.lang._XComponent
 * @see ifc.document._DocumentInfo
 * @see ifc.document._XDocumentInfo
 * @see ifc.document._XStandaloneDocumentInfo
 * @see ifc.beans._XFastPropertySet
 * @see ifc.beans._XPropertySet
 */
public class StandaloneDocumentInfo extends TestCase {

    String destUrl = null ;

    /**
     * Copies file 'SfxStandaloneDocInfoObject.sdw' to tempopary
     * location (overwriting the old file if exists).
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        String srcUrl = utils.getFullTestURL("SfxStandaloneDocInfoObject.sdw") ;
        destUrl = utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) +
            "SfxStandaloneDocInfoObject.sdw";

        utils.doOverwriteFile((XMultiServiceFactory)tParam.getMSF(), srcUrl, destUrl) ;
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.document.StandaloneDocumentInfo</code>.
     *     Object relations created :
     * <ul>
     *  <li> <code>'DOCURL'</code> for
     *      {@link ifc.document._XStandaloneDocumentInfo} :
     *      the Writer file URL in temporary location. </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XInterface oObj = null;

        try {
            oObj = (XInterface)xMSF.createInstance
                ("com.sun.star.document.StandaloneDocumentInfo");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        String Iname = util.utils.getImplName(oObj);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(oObj);
        XStandaloneDocumentInfo the_info = (XStandaloneDocumentInfo)
                UnoRuntime.queryInterface(XStandaloneDocumentInfo.class, oObj);
        try {
            the_info.loadFromURL(destUrl);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't load document: " + destUrl,e);
        }
        tEnv.addObjRelation("DOCURL",destUrl);
        return tEnv;
    }

}    // finish class TestCase

