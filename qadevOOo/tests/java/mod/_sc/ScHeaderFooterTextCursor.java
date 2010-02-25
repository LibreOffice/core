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
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.text.TextCursor</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::text::XTextCursor</code></li>
*  <li> <code>com::sun::star::text::XWordCursor</code></li>
*  <li> <code>com::sun::star::style::CharacterPropertiesComplex</code></li>
*  <li> <code>com::sun::star::text::XTextRange</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::container::XContentEnumerationAccess</code></li>
*  <li> <code>com::sun::star::beans::XPropertyState</code></li>
*  <li> <code>com::sun::star::style::CharacterProperties</code></li>
*  <li> <code>com::sun::star::text::XSentenceCursor</code></li>
*  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
*  <li> <code>com::sun::star::text::XParagraphCursor</code></li>
*  <li> <code>com::sun::star::document::XDocumentInsertable</code></li>
*  <li> <code>com::sun::star::util::XSortable</code></li>
*  <li> <code>com::sun::star::style::CharacterPropertiesAsian</code></li>
* </ul>
* @see com.sun.star.text.TextCursor
* @see com.sun.star.text.XTextCursor
* @see com.sun.star.text.XWordCursor
* @see com.sun.star.style.CharacterPropertiesComplex
* @see com.sun.star.text.XTextRange
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.container.XContentEnumerationAccess
* @see com.sun.star.beans.XPropertyState
* @see com.sun.star.style.CharacterProperties
* @see com.sun.star.text.XSentenceCursor
* @see com.sun.star.style.ParagraphProperties
* @see com.sun.star.text.XParagraphCursor
* @see com.sun.star.document.XDocumentInsertable
* @see com.sun.star.util.XSortable
* @see com.sun.star.style.CharacterPropertiesAsian
* @see ifc.text._XTextCursor
* @see ifc.text._XWordCursor
* @see ifc.style._CharacterPropertiesComplex
* @see ifc.text._XTextRange
* @see ifc.beans._XPropertySet
* @see ifc.container._XContentEnumerationAccess
* @see ifc.beans._XPropertyState
* @see ifc.style._CharacterProperties
* @see ifc.text._XSentenceCursor
* @see ifc.style._ParagraphProperties
* @see ifc.text._XParagraphCursor
* @see ifc.document._XDocumentInsertable
* @see ifc.util._XSortable
* @see ifc.style._CharacterPropertiesAsian
*/
public class ScHeaderFooterTextCursor extends TestCase {
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
            UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of style families available in the document
    * using the interface <code>XStyleFamiliesSupplier</code>.
    * Obtains default style from the style family <code>'PageStyles'</code>.
    * Retrieves value of the property <code>'RightPageHeaderContent'</code>.
    * Sets some string for the text which is printed in the center part of the
    * header or footer using the interface <code>XHeaderFooterContent</code>
    * and sets new value of the property <code>'RightPageHeaderContent'</code>.
    * Creates text cursor for the text which is printed in the center part of
    * the header or footer. This text cursor is the instance of the service
    * <code>com.sun.star.text.TextCursor</code>.
    * </ul>
    */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XPropertySet PropSet;
        XNameAccess PageStyles = null;
        XStyle StdStyle = null;

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
        } catch(com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get HeaderContent", e);
        } catch(com.sun.star.beans.UnknownPropertyException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get HeaderContent", e);
        } catch(com.sun.star.lang.IllegalArgumentException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get HeaderContent", e);
        }

        XText center = RPHC.getCenterText();
        center.setString("CENTER");

        try {
            PropSet.setPropertyValue("RightPageHeaderContent",RPHC);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set HeaderContent", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set HeaderContent", e);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set HeaderContent", e);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't set HeaderContent", e);
        }

        // create testobject here
        oObj = center.createTextCursor();
        TestEnvironment tEnv = new TestEnvironment(oObj);

        // add relation for XTextRange
        tEnv.addObjRelation("XTEXT",
            UnoRuntime.queryInterface(XText.class,center));

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class ScHeaderFooterTextCursor
