/*************************************************************************
 *
 *  $RCSfile: ScHeaderFooterTextObj.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:10:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DefaultDsc;
import util.InstCreator;
import util.ParagraphDsc;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XHeaderFooterContent;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.text.Text</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::text::XTextRangeMover</code></li>
*  <li> <code>com::sun::star::text::XSimpleText</code></li>
*  <li> <code>com::sun::star::text::XTextRange</code></li>
*  <li> <code>com::sun::star::text::XRelativeTextContentInsert</code></li>
*  <li> <code>com::sun::star::text::XTextRangeCompare</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
*  <li> <code>com::sun::star::text::XText</code></li>
* </ul>
* @see com.sun.star.text.Text
* @see com.sun.star.text.XTextRangeMover
* @see com.sun.star.text.XSimpleText
* @see com.sun.star.text.XTextRange
* @see com.sun.star.text.XRelativeTextContentInsert
* @see com.sun.star.text.XTextRangeCompare
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XEnumerationAccess
* @see com.sun.star.text.XText
* @see ifc.text._XTextRangeMover
* @see ifc.text._XSimpleText
* @see ifc.text._XTextRange
* @see ifc.text._XRelativeTextContentInsert
* @see ifc.text._XTextRangeCompare
* @see ifc.container._XElementAccess
* @see ifc.container._XEnumerationAccess
* @see ifc.text._XText
*/
public class ScHeaderFooterTextObj extends TestCase {

    XSpreadsheetDocument xSpreadsheetDoc;

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
        oComp.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of style families available in the document
    * using the interface <code>XStyleFamiliesSupplier</code>.
    * Obtains default style from the style family <code>'PageStyles'</code>.
    * Retrieves value of the property <code>'RightPageHeaderContent'</code>.
    * Creates text cursor for the text which is printed in the center part of
    * the header or footer using the interface <code>XHeaderFooterContent</code>.
    * Insert some lines using the created cursor and sets new value of the
    * property <code>'RightPageHeaderContent'</code>. The value of the property
    * <code>'RightPageHeaderContent'</code> is the instance of the service
    * <code>com.sun.star.text.Text</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'RangeForMove'</code> for
    *      {@link ifc.text._XTextRangeMover} (the range to be moved)</li>
    *  <li> <code>'XTextRange'</code> for
    *      {@link ifc.text._XTextRangeMover} (the range that includes moving
    *       range)</li>
    *  <li> <code>'XTEXTINFO'</code> for
    *      {@link ifc.text._XRelativeTextContentInsert}(the instance creator
    *       which can create instances of some text content service)</li>
    *  <li> <code>'PARA'</code> for
    *      {@link ifc.text._XRelativeTextContentInsert}(the instance creator
    *       which can create instances of <code>com.sun.star.text.Paragraph</code>
    *       service)</li>
    * </ul>
    * @see com.sun.star.text.Text
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

        XTextRange text_to_move = null;

        log.println( "inserting some lines" );
        try {
            XTextCursor oCursor = center.createTextCursor();
            center.insertControlCharacter(
                oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
            center.insertControlCharacter(
                oCursor, ControlCharacter.LINE_BREAK, false );
            center.insertString(oCursor,"Paragraph 1", false);
            center.insertString(oCursor,": ScHeaderFooterTextObj", false);
            center.insertControlCharacter(
                oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
            center.insertString(oCursor, "THE QUICK BROWN FOX JUMPS OVER THE" +
                " LAZY DOG: ScHeaderFooterTextObj", false );
            center.insertControlCharacter(
                oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
            center.insertControlCharacter(
                oCursor, ControlCharacter.LINE_BREAK, false );
            oCursor.setString("TextForMove");
            text_to_move = oCursor;

            XTextCursor oCursor1 = center.createTextCursorByRange(center.getEnd());
            center.insertString(oCursor1,"Paragraph 2", false);
            center.insertString(oCursor1,": ScHeaderFooterTextObj", false);
            center.insertControlCharacter(
                oCursor1, ControlCharacter.PARAGRAPH_BREAK, false );
            center.insertString( oCursor1, "THE QUICK BROWN FOX JUMPS OVER THE" +
                " LAZY DOG: ScHeaderFooterTextObj", false);
            center.insertControlCharacter(
                oCursor1, ControlCharacter.PARAGRAPH_BREAK, false );
            center.insertControlCharacter(
                oCursor1, ControlCharacter.LINE_BREAK, false );
        } catch(com.sun.star.lang.IllegalArgumentException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't insert Text ", e);
        }

        try {
            PropSet.setPropertyValue("RightPageHeaderContent", RPHC);
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
        oObj = center;
        TestEnvironment tEnv = new TestEnvironment(oObj);

        DefaultDsc tDsc = new DefaultDsc(
            "com.sun.star.text.XTextContent","com.sun.star.text.TextField.Time");

        log.println( "adding InstCreator object" );
        tEnv.addObjRelation(
            "XTEXTINFO", new InstCreator( xSpreadsheetDoc, tDsc ) );

        ParagraphDsc pDsc = new ParagraphDsc();
        tEnv.addObjRelation( "PARA", new InstCreator(xSpreadsheetDoc, pDsc) );

        log.println("adding TextRange for XTextRangeMover" );
        tEnv.addObjRelation("RangeForMove", text_to_move);
        tEnv.addObjRelation("XTextRange", RPHC.getCenterText());

        return tEnv;

    } // finish method getTestEnvironment
}    // finish class ScHeaderFooterTextObj
