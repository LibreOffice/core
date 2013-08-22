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

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XAreaLinks;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.CellAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.CellAreaLinks</code>.
* This is a collection aff all links present
* in a Spreadsheet document.
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::sheet::XAreaLinks</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.sheet.XAreaLinks
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.sheet._XAreaLinks
* @see com.sun.star.sheet.CellAreaLinks
*/
public class ScAreaLinksObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
   protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

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
    * Retrieves a collection of Area Links using the 'AreaLinks'
    * property of the Spreadsheet document. Adds a new link to this
    * collection, which has a source in the same document. The
    * link collection is passed as a tested object.
    */
    public synchronized TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log){

      XInterface oObj = null;
      TestEnvironment tEnv = null ;

      try {

        // creation of testobject here
        XPropertySet props = UnoRuntime.queryInterface
            (XPropertySet.class, xSheetDoc);
        oObj = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),props.getPropertyValue("AreaLinks")) ;
        XAreaLinks links = null ;

        // adding one link into collection (for best testing)
        links = UnoRuntime.queryInterface(XAreaLinks.class, oObj) ;
        CellAddress addr = new CellAddress ((short) 1,2,3) ;
        String aSourceArea = util.utils.getFullTestURL("calcshapes.sxc");
        links.insertAtPosition (addr, aSourceArea, "a2:b5", "", "") ;

        // creating test environment
        tEnv = new TestEnvironment(oObj);

      } catch (com.sun.star.beans.UnknownPropertyException e) {
        log.println ("Exception occurred while creating test Object.") ;
        e.printStackTrace(log) ;
        throw new StatusException("Couldn't create test object", e);
      } catch (com.sun.star.lang.WrappedTargetException e) {
        log.println ("Exception occurred while creating test Object.") ;
        e.printStackTrace(log) ;
        throw new StatusException("Couldn't create test object", e);
      } catch (com.sun.star.lang.IllegalArgumentException e) {
        log.println ("Exception occurred while creating test Object.") ;
        e.printStackTrace(log) ;
        throw new StatusException("Couldn't create test object", e);
      }

       return tEnv ;
    }

}    // finish class ScAreaLinksObj

