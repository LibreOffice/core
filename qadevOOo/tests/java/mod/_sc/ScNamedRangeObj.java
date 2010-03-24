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

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XNamedRanges;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.NamedRange</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::sheet::XNamedRange</code></li>
*  <li> <code>com::sun::star::sheet::XCellRangeReferrer</code></li>
* </ul>
* @see com.sun.star.sheet.NamedRange
* @see com.sun.star.container.XNamed
* @see com.sun.star.sheet.XNamedRange
* @see com.sun.star.sheet.XCellRangeReferrer
* @see ifc.container._XNamed
* @see ifc.sheet._XNamedRange
* @see ifc.sheet._XCellRangeReferrer
*/
public class ScNamedRangeObj extends TestCase {
    static XSpreadsheetDocument xSheetDoc = null;

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
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface(XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document
    * and takes one of them. Obtains the value of the property
    * <code>'NamedRanges'</code> that is the collection of named ranges.
    * Creates and adds new range to the collection. This new range is the instance of the
    * service <code>com.sun.star.sheet.NamedRange</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'DATAAREA'</code> for
    *      {@link ifc.sheet._XCellRangeReferrer} (the cell range address of the
    *      created range) </li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        log.println("Getting test object ") ;

        // Getting named ranges.
        XPropertySet docProps = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, xSheetDoc);

        Object ranges = null;
        try {
            ranges = docProps.getPropertyValue("NamedRanges");
        } catch(com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get NamedRanges", e);
        } catch(com.sun.star.beans.UnknownPropertyException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get NamedRanges", e);
        }

        XNamedRanges xNamedRanges = (XNamedRanges)
            UnoRuntime.queryInterface(XNamedRanges.class, ranges);

        CellRangeAddress DataArea = new CellRangeAddress((short)0, 0, 0, 1, 1);
        CellAddress base = new CellAddress(DataArea.Sheet,
                                           DataArea.StartColumn,
                                           DataArea.StartRow);

        if (xNamedRanges.hasByName("ANamedRange")) {
            xNamedRanges.removeByName("ANamedRange");
        }

        xNamedRanges.addNewByName("ANamedRange", "A1:B2", base, 0);

        CellAddress listOutputPosition = new CellAddress((short)0, 1, 1);
        xNamedRanges.outputList(listOutputPosition);

        try {
            oObj = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),xNamedRanges.getByName("ANamedRange"));
        } catch(com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by name", e);
        } catch(com.sun.star.container.NoSuchElementException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by name", e);
        } catch(com.sun.star.lang.IllegalArgumentException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by name", e);
        }

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Other parameters required for interface tests
        tEnv.addObjRelation("DATAAREA", DataArea);

        return tEnv;
    }

}


