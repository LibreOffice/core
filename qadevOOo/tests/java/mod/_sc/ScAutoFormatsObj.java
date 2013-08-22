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
    private XSpreadsheetDocument xSheetDoc = null;
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
        XComponent oComp = UnoRuntime.queryInterface
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
            XComponent xComp = UnoRuntime.queryInterface
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

