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

package complex.writer;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 *
 * @author od138299
 */
public class CheckCrossReferences {

    private com.sun.star.container.XEnumeration xParaEnum;
    private com.sun.star.container.XEnumeration xPortionEnum;
    private com.sun.star.util.XRefreshable xFldsRefresh;

    public com.sun.star.text.XTextField getNextField()
        throws com.sun.star.uno.Exception
    {
        if ( xPortionEnum != null ) {
            while ( xPortionEnum.hasMoreElements() ) {
                com.sun.star.beans.XPropertySet xPortionProps =
                    (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                        com.sun.star.beans.XPropertySet.class , xPortionEnum.nextElement());
                final String sPortionType =
                    xPortionProps.getPropertyValue( "TextPortionType" ).toString();
                if ( sPortionType.equals( "TextField") ) {
                    com.sun.star.text.XTextField xField = (com.sun.star.text.XTextField)UnoRuntime.queryInterface(
                        com.sun.star.text.XTextField.class,
                        xPortionProps.getPropertyValue( "TextField" ) );
                    assertNotNull("Cannot retrieve next field.", xField);
                    return xField;
                }
            }
        }

        while ( xParaEnum.hasMoreElements() ) {
            com.sun.star.container.XEnumerationAccess aPara =
                (com.sun.star.container.XEnumerationAccess)UnoRuntime.queryInterface(
                    com.sun.star.container.XEnumerationAccess.class, xParaEnum.nextElement());
            xPortionEnum = aPara.createEnumeration();
            while ( xPortionEnum.hasMoreElements() ) {
                com.sun.star.beans.XPropertySet xPortionProps =
                    (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                        com.sun.star.beans.XPropertySet.class , xPortionEnum.nextElement());
                final String sPortionType =
                    xPortionProps.getPropertyValue( "TextPortionType" ).toString();
                if ( sPortionType.equals( "TextField") ) {
                    com.sun.star.text.XTextField xField = (com.sun.star.text.XTextField)UnoRuntime.queryInterface(
                        com.sun.star.text.XTextField.class,
                        xPortionProps.getPropertyValue( "TextField" ) );
                    assertNotNull("Cannot retrieve next field.", xField);
                    return xField;
                }
            }
        }

        fail("Cannot retrieve next field.");
        return null; // unreachable
    }

    public com.sun.star.beans.XPropertySet getFieldProps(
            com.sun.star.text.XTextField xField ) {
        com.sun.star.beans.XPropertySet xProps =
                (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xField );

        assertNotNull("Cannot retrieve field properties.", xProps);

        return xProps;
    }

    public void checkField( com.sun.star.text.XTextField xField,
            com.sun.star.beans.XPropertySet xProps,
            short nFormat,
            String aExpectedFldResult )
        throws com.sun.star.uno.Exception
    {
        // set requested format
        xProps.setPropertyValue("ReferenceFieldPart", new Short(nFormat));

        // refresh fields in order to get new format applied
        xFldsRefresh.refresh();

        String aFldResult = xField.getPresentation( false );
        assertEquals( "set reference field format doesn't result in correct field result",
                      aExpectedFldResult, aFldResult);
    }

    @Test public void checkCrossReferences() throws com.sun.star.uno.Exception {
        // setup paragraph enumeration
        {
            com.sun.star.container.XEnumerationAccess xParaEnumAccess =
                    (com.sun.star.container.XEnumerationAccess)UnoRuntime.queryInterface(
                    com.sun.star.container.XEnumerationAccess.class, document.getText());
            xParaEnum = xParaEnumAccess.createEnumeration();
        }

        // get field refresher
        {
            com.sun.star.text.XTextFieldsSupplier xFieldSupp =
                    (com.sun.star.text.XTextFieldsSupplier)UnoRuntime.queryInterface(
                    com.sun.star.text.XTextFieldsSupplier.class, document);
            xFldsRefresh = (com.sun.star.util.XRefreshable)UnoRuntime.queryInterface(
                    com.sun.star.util.XRefreshable.class, xFieldSupp.getTextFields());
        }

        // check first reference field
        {
            // strings for checking
            final String FldResult1 = "*i*";
            final String FldResult2 = "+b+*i*";
            final String FldResult3 = "-1-+b+*i*";
            final String FldResult4 = "1.";
            final String FldResult5 = " 1.";
            final String FldResult6 = "A. 1.";

            // variables for current field
            com.sun.star.text.XTextField xField = null;
            com.sun.star.beans.XPropertySet xProps = null;

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FldResult2 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult1 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult3 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FldResult1 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult1 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult3 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FldResult3 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult1 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult3 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FldResult5 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult4 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult6 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FldResult4 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult4 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult6 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FldResult6 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult4 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult6 );
        }

        // insert a certain cross-reference bookmark and a reference field to this bookmark
        {
            // restart paragraph enumeration
            com.sun.star.container.XEnumerationAccess xParaEnumAccess =
                    (com.sun.star.container.XEnumerationAccess)UnoRuntime.queryInterface(
                    com.sun.star.container.XEnumerationAccess.class, document.getText());
            xParaEnum = xParaEnumAccess.createEnumeration();

            // iterate on the paragraphs to find certain paragraph to insert the bookmark
            com.sun.star.text.XTextRange xParaTextRange = null;
            while ( xParaEnum.hasMoreElements() ) {
                xParaTextRange = (com.sun.star.text.XTextRange)UnoRuntime.queryInterface(
                    com.sun.star.text.XTextRange.class, xParaEnum.nextElement());
                if ( xParaTextRange.getString().equals( "J" ) ) {
                    break;
                }
                else {
                    xParaTextRange = null;
                }
            }
            assertNotNull(
                "Cannot find paragraph to insert cross-reference bookmark.",
                xParaTextRange);

            // insert bookmark
            XMultiServiceFactory xFac = (XMultiServiceFactory)UnoRuntime.queryInterface(
                    XMultiServiceFactory.class, document);
            final String cBookmarkName = "__RefNumPara__47114711";
            com.sun.star.text.XTextContent xBookmark =
                (com.sun.star.text.XTextContent)UnoRuntime.queryInterface(
                    com.sun.star.text.XTextContent.class,
                    xFac.createInstance( "com.sun.star.text.Bookmark" ) );
            if ( xBookmark != null ) {
                com.sun.star.container.XNamed xName =
                        (com.sun.star.container.XNamed)UnoRuntime.queryInterface(
                        com.sun.star.container.XNamed.class, xBookmark );
                xName.setName( cBookmarkName );
                xBookmark.attach(xParaTextRange.getStart());
            }

            // insert reference field, which references the inserted bookmark
            com.sun.star.text.XTextContent xNewField =
                (com.sun.star.text.XTextContent)UnoRuntime.queryInterface(
                    com.sun.star.text.XTextContent.class,
                    xFac.createInstance( "com.sun.star.text.TextField.GetReference" ) );
            if ( xNewField != null ) {
                com.sun.star.beans.XPropertySet xFieldProps =
                        (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                        com.sun.star.beans.XPropertySet.class, xNewField );
                xFieldProps.setPropertyValue( "ReferenceFieldPart", new Short(com.sun.star.text.ReferenceFieldPart.TEXT) );
                xFieldProps.setPropertyValue( "ReferenceFieldSource", new Short(com.sun.star.text.ReferenceFieldSource.BOOKMARK) );
                xFieldProps.setPropertyValue( "SourceName", cBookmarkName );
                com.sun.star.text.XTextRange xFieldTextRange =
                        (com.sun.star.text.XTextRange)UnoRuntime.queryInterface(
                        com.sun.star.text.XTextRange.class, xParaEnum.nextElement());
                xNewField.attach(xFieldTextRange.getEnd());
                xFldsRefresh.refresh();
            }

            // check inserted reference field
            com.sun.star.text.XTextField xField =
                    (com.sun.star.text.XTextField)UnoRuntime.queryInterface(
                    com.sun.star.text.XTextField.class, xNewField );
            assertEquals( "inserted reference field doesn't has correct field result",
                          "J", xField.getPresentation( false ) );

            xParaTextRange.getStart().setString( "Hallo new bookmark: " );
            xFldsRefresh.refresh();
            assertEquals( "inserted reference field doesn't has correct field result",
                          "Hallo new bookmark: J", xField.getPresentation( false ) );
        }
    }

    @Before public void setUpDocument() throws com.sun.star.uno.Exception {
        document = util.WriterTools.loadTextDoc(
            UnoRuntime.queryInterface(
                XMultiServiceFactory.class,
                connection.getComponentContext().getServiceManager()),
            TestDocument.getUrl("CheckCrossReferences.odt"));
    }

    @After public void tearDownDocument() {
        util.DesktopTools.closeDoc(document);
    }

    private XTextDocument document = null;

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();
}
