/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package mod._sm;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XComponent;

/**
 * Test for object which is represented by
 * <code>'StarMath'</code> document. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::formula::FormulaProperties</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 *
 * @see com.sun.star.formula.FormulaProperties
 * @see ifc.formula._FormulaProperties
 */
public class SmModel extends TestCase {
    XComponent xMathDoc;

    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     * Creates a <code>StarMath</code> document and passes it as
     * tested component.
     */
    @Override
    public TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log )
            throws Exception {

        SOfficeFactory SOF = SOfficeFactory.getFactory( Param.getMSF() );
        xMathDoc = SOF.openDoc("smath","_blank");

        String Iname = util.utils.getImplName(xMathDoc);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(xMathDoc);
        return tEnv;
    }

    /**
     * Disposes the document created in <code>createTestEnvironment</code>
     * method.
     */
    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println( "    disposing xMathDoc " );
        xMathDoc.dispose();
    }


}    // finish class SmModel

