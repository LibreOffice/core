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
import util.SOfficeFactory;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.TableAutoFormats</code>. <p>
* In StarCalc application there is a collection of autoformats
* for tables (you can select a predefined format for a
* table or create your own). The object represents
* this collection. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XNameReplace</code></li>
*  <li> <code>com::sun::star::container::XNameContainer</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XNameReplace
* @see com.sun.star.container.XNameContainer
* @see ifc.container._XNameAccess
* @see ifc.container._XElementAccess
* @see ifc.container._XNameReplace
* @see ifc.container._XNameContainer
*/
public class ScAutoFormatsObj extends TestCase{
    static XSpreadsheetDocument xSheetDoc = null;
    static SOfficeFactory SOF = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = (XComponent) UnoRuntime.queryInterface
            (XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }


    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.sheet.TableAutoFormats</code>.
    *     Object relations created :
    * <ul>
    *  <li> <code>'INSTANCE1', ..., 'INSTANCEN'</code> for
    *      {@link ifc.container._XNameContainer},
    *      {@link ifc.container._XNameReplace}  N relations
    *   which represents objects to be inserted - one
    *   object for each interface thread. </li>
    * </ul>
    */
    public synchronized TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log )
            throws StatusException {

        XInterface oObj = null;

        try {
            // creation of testobject here
            // get AutoFormats
            XComponent xComp = (XComponent)UnoRuntime.queryInterface
                (XComponent.class, xSheetDoc);
            oObj = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),((XMultiServiceFactory)Param.getMSF()).createInstance
                                    ("com.sun.star.sheet.TableAutoFormats"));
            Object secondInstance = SOF.createInstance
                (xComp, "com.sun.star.sheet.TableAutoFormat");

            TestEnvironment tEnv = new TestEnvironment(oObj) ;

            //adding ObjRelation for XNameContainer
            tEnv.addObjRelation("SecondInstance",secondInstance);

            // INSTANCEn : _XNameContainer; _XNameReplace
            log.println( "adding INSTANCEn as mod relation to environment" );
            int THRCNT = 1;
            if ((String)Param.get("THRCNT") != null) {
                Integer.parseInt((String)Param.get("THRCNT"));
            }
            for (int n = 1; n < (THRCNT+1) ;n++ ) {
                log.println( "adding INSTANCE" + n
                    +" as mod relation to environment" );
                tEnv.addObjRelation("INSTANCE" + n, SOF.createInstance(xComp,
                    "com.sun.star.sheet.TableAutoFormat"));
            }

            return tEnv;
        } catch (com.sun.star.uno.Exception e) {
            log.println ("Exception occurred while creating test Object.");
            e.printStackTrace(log);
            throw new StatusException("Couldn't create test object", e);
        }
    }

}    // finish class ScAutoFormatsObj

