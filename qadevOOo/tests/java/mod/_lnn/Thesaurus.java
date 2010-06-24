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

package mod._lnn;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.linguistic2.Thesaurus</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XInitialization</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::linguistic2::XSupportedLocales</code></li>
 *  <li> <code>com::sun::star::linguistic2::XThesaurus</code></li>
 *  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
 *  <li> <code>com::sun::star::lang::XServiceDisplayName</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.linguistic2.XSupportedLocales
 * @see com.sun.star.linguistic2.XThesaurus
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.lang.XServiceDisplayName
 * @see ifc.lang._XInitialization
 * @see ifc.lang._XComponent
 * @see ifc.linguistic2._XSupportedLocales
 * @see ifc.linguistic2._XThesaurus
 * @see ifc.lang._XServiceInfo
 * @see ifc.lang._XServiceDisplayName
 */
public class Thesaurus extends TestCase {

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.linguistic2.Thesaurus</code>.
    */
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XInterface oObj = null;

        try {
            oObj = (XInterface)xMSF.createInstance
                ("com.sun.star.linguistic2.Thesaurus");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        String Iname = util.utils.getImplName(oObj);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    }

}    // finish class TestCase

