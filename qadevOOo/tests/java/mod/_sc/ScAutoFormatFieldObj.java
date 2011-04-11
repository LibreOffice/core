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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.TableAutoFormatField</code>.
* In StarCalc application there is a collection of autoformats
* for tables (you can select a predefined format for a
* table or create your own). Each autoformat has a number
* of fields with definitions of font parameters, number
* formats etc. for different parts of a table (column and
* row names, footers, data). This object represents the
* field of the same kind. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::TableAutoFormatField</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.sheet.TableAutoFormatField
* @see com.sun.star.beans.XPropertySet
* @see ifc.sheet._TableAutoFormatField
* @see ifc.beans._XPropertySet
*/
public class ScAutoFormatFieldObj extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Using SOffice ServiceManager an instance of
    * <code>com.sun.star.sheet.TableAutoFormatField</code> service
    * is created. From this collection one Format is retrieved
    * and then from this format one of its compound fields is
    * retrieved.
    */
    public synchronized TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log ) throws StatusException {

        XInterface oObj = null;

        try {

                  log.println ("create Object ...") ;
            // creation of testobject here
            XInterface formats = (XInterface)((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.sheet.TableAutoFormats");
            XIndexAccess formatsIndex = (XIndexAccess)UnoRuntime.queryInterface
                (XIndexAccess.class, formats);
            XInterface format = (XInterface) AnyConverter.toObject(
                        new Type(XInterface.class),formatsIndex.getByIndex(0));
            XIndexAccess formatIndex = (XIndexAccess) UnoRuntime.queryInterface
                (XIndexAccess.class, format);
            oObj = (XInterface) AnyConverter.toObject(
                        new Type(XInterface.class),formatIndex.getByIndex(0));

        } catch (com.sun.star.uno.Exception e) {
            log.println ("Exception occurred while creating test Object.") ;
            e.printStackTrace(log) ;
        }

        TestEnvironment tEnv = new TestEnvironment(oObj) ;

        return tEnv;
    }

}    // finish class ScAutoFormatFieldObj

