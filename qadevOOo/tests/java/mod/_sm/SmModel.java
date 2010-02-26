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

package mod._sm;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;

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
 * threads concurently.
 *
 * @see com.sun.star.formula.FormulaProperties
 * @see ifc.formula._FormulaProperties
 */
public class SmModel extends TestCase {
    XComponent xMathDoc;

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates a <code>StarMath</code> document and passes it as
     * tested component.
     */
    public synchronized TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log )
            throws StatusException {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF() );
        try {
            xMathDoc = SOF.openDoc("smath","_blank");
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        } catch (com.sun.star.io.IOException ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        }

        String Iname = util.utils.getImplName(xMathDoc);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(xMathDoc);
        return tEnv;
    }

    /**
     * Disposes the document created in <code>createTestEnvironment</code>
     * method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println( "    disposing xMathDoc " );
        xMathDoc.dispose();
    }


}    // finish class SmModel

