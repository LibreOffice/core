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
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XHeaderFooterContent;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.text.TextField</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::text::XTextField</code></li>
*  <li> <code>com::sun::star::text::XTextContent</code></li>
*  <li> <code>com::sun::star::text::TextContent</code></li>
* </ul>
* @see com.sun.star.text.TextField
* @see com.sun.star.lang.XComponent
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.text.XTextField
* @see com.sun.star.text.XTextContent
* @see com.sun.star.text.TextContent
* @see ifc.lang._XComponent
* @see ifc.beans._XPropertySet
* @see ifc.text._XTextField
* @see ifc.text._XTextContent
* @see ifc.text._TextContent
*/
public class ScHeaderFieldObj extends TestCase {
    static XSpreadsheetDocument xSpreadsheetDoc;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSpreadsheetDoc = SOF.createCalcDoc(null);
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
            UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of style families available in the document
    * using the interface <code>XStyleFamiliesSupplier</code>.
    * Obtains default style from the style family <code>'PageStyles'</code>.
    * Retrieves the interface <code>XHeaderFooterContent</code> from the style
    * using the property <code>'RightPageHeaderContent'</code>. Creates the
    * instance of the service <code>com.sun.star.text.TextField.Time</code> and
    * the instance of the service <code>com.sun.star.text.TextField.Date</code>
    * Obtains the text (the interface <code>XText</code>) which is printed in
    * the left part of the header or footer and inserts in it's content
    * the second created instance.
    * Object relations created :
    * <ul>
    *  <li> <code>'CONTENT'</code> for
    *      {@link ifc.text._XTextContent}(the interface <code>XTextContent</code>
    *      that was queried from the second created instance) </li>
    *  <li> <code>'TEXT'</code> for
    *      {@link ifc.text._XTextContent}(the the text which is printed in the
    *       right part of the header or footer) </li>
    * </ul>
    * @see com.sun.star.style.XStyleFamiliesSupplier
    * @see com.sun.star.sheet.XHeaderFooterContent
    * @see com.sun.star.text.XText
    * @see com.sun.star.text.XTextContent
    */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XPropertySet PropSet;
        XNameAccess PageStyles = null;
        XStyle StdStyle = null;
        XTextContent oContent = null;
        XInterface aField = null;

        XStyleFamiliesSupplier StyleFam = (XStyleFamiliesSupplier)
            UnoRuntime.queryInterface(
                XStyleFamiliesSupplier.class,
                xSpreadsheetDoc );

        XNameAccess StyleFamNames = StyleFam.getStyleFamilies();
        try{
            PageStyles = (XNameAccess) AnyConverter.toObject(
                new Type(XNameAccess.class),StyleFamNames.getByName("PageStyles"));
            StdStyle = (XStyle) AnyConverter.toObject(
                        new Type(XStyle.class),PageStyles.getByName("Default"));
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

        //get the property-set
        PropSet = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, StdStyle);

        XHeaderFooterContent RPHC = null;
        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );
        try {
            RPHC = (XHeaderFooterContent) AnyConverter.toObject(
                new Type(XHeaderFooterContent.class),
                    PropSet.getPropertyValue("RightPageHeaderContent"));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get HeaderContent", e);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get HeaderContent", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get HeaderContent", e);
        }

        XText left = RPHC.getLeftText();

        XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface(
                XMultiServiceFactory.class,
                xSpreadsheetDoc );

        XTextContent the_Field = null;
        try {
            oObj = (XInterface)
                oDocMSF.createInstance( "com.sun.star.text.TextField.Time" );

            the_Field = (XTextContent)
                UnoRuntime.queryInterface(XTextContent.class,oObj);

            aField = (XInterface)
                oDocMSF.createInstance("com.sun.star.text.TextField.Date");
        } catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e);
        }

        oContent = (XTextContent)
            UnoRuntime.queryInterface(XTextContent.class, aField);

        XTextCursor the_Cursor = left.createTextCursor();

        try {
            left.insertTextContent(the_Cursor,the_Field, false);
            PropSet.setPropertyValue("RightPageHeaderContent", RPHC);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create a test environment", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("CONTENT",oContent);
        tEnv.addObjRelation("TEXT", RPHC.getRightText());

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class ScHeaderFieldObj

