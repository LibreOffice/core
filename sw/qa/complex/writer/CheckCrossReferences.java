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

public class CheckCrossReferences {

    private com.sun.star.container.XEnumeration xParaEnum;
    private com.sun.star.container.XEnumeration xPortionEnum;
    private com.sun.star.util.XRefreshable xFieldsRefresh;

    public com.sun.star.text.XTextField getNextField()
        throws com.sun.star.uno.Exception
    {
        while (true) {
            while (xPortionEnum == null) {
                if (!xParaEnum.hasMoreElements())
                    fail("Cannot retrieve next field.");

                com.sun.star.container.XEnumerationAccess aPara =
                    UnoRuntime.queryInterface(
                    com.sun.star.container.XEnumerationAccess.class, xParaEnum.nextElement());
                xPortionEnum = aPara.createEnumeration();
            }

            if ( xPortionEnum == null )
                break;

            while ( xPortionEnum.hasMoreElements() ) {
                com.sun.star.beans.XPropertySet xPortionProps =
                    UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertySet.class , xPortionEnum.nextElement());
                final String sPortionType =
                    xPortionProps.getPropertyValue( "TextPortionType" ).toString();
                if ( sPortionType.equals( "TextField") ) {
                    com.sun.star.text.XTextField xField = UnoRuntime.queryInterface(
                        com.sun.star.text.XTextField.class,
                        xPortionProps.getPropertyValue( "TextField" ) );
                    assertNotNull("Cannot retrieve next field.", xField);
                    return xField;
                }
            }
            xPortionEnum = null;
        }

        return null; // unreachable
    }

    public com.sun.star.beans.XPropertySet getFieldProps(
            com.sun.star.text.XTextField xField ) {
        com.sun.star.beans.XPropertySet xProps =
                UnoRuntime.queryInterface(
        com.sun.star.beans.XPropertySet.class, xField );

        assertNotNull("Cannot retrieve field properties.", xProps);

        return xProps;
    }

    public void checkField( com.sun.star.text.XTextField xField,
            com.sun.star.beans.XPropertySet xProps,
            short nFormat,
            String aExpectedFieldResult )
        throws com.sun.star.uno.Exception
    {
        // set requested format
        xProps.setPropertyValue("ReferenceFieldPart", Short.valueOf(nFormat));

        // refresh fields in order to get new format applied
        xFieldsRefresh.refresh();

        String aFieldResult = xField.getPresentation( false );
        assertEquals( "set reference field format doesn't result in correct field result",
                      aExpectedFieldResult, aFieldResult);
    }

    @Test public void checkCrossReferences() throws com.sun.star.uno.Exception {
        // setup paragraph enumeration
        {
            com.sun.star.container.XEnumerationAccess xParaEnumAccess =
                    UnoRuntime.queryInterface(
            com.sun.star.container.XEnumerationAccess.class, document.getText());
            xParaEnum = xParaEnumAccess.createEnumeration();
        }

        // get field refresher
        {
            com.sun.star.text.XTextFieldsSupplier xFieldSupp =
                    UnoRuntime.queryInterface(
            com.sun.star.text.XTextFieldsSupplier.class, document);
            xFieldsRefresh = UnoRuntime.queryInterface(
                    com.sun.star.util.XRefreshable.class, xFieldSupp.getTextFields());
        }

        // check first reference field
        {
            // strings for checking
            final String FieldResult1 = "*i*";
            final String FieldResult2 = "+b+*i*";
            final String FieldResult3 = "-1-+b+*i*";
            final String FieldResult4 = "1";
            final String FieldResult5 = "1";
            final String FieldResult6 = "A.1";
            final String FieldResult7 = "2(a)";
            final String FieldResult8 = "2(b)";
            final String FieldResult9 = "2";
            final String FieldResult10 = "1(a)";
            final String FieldResult11 = "(b)";
            final String FieldResult12 = "(a)";

            // variables for current field
            com.sun.star.text.XTextField xField = null;
            com.sun.star.beans.XPropertySet xProps = null;

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult2 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult1 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult3 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult1 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult1 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult3 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult3 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult1 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult3 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult5 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult4 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult6 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult4 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult4 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult6 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult6 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult4 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult6 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult7 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult12 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult7 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult8 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult11 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult8 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult9 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult9 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult9 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult4 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult4 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult4 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult10 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult12 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult10 );

            xField = getNextField();
            xProps = getFieldProps( xField );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER, FieldResult12 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_NO_CONTEXT, FieldResult12 );
            checkField( xField, xProps, com.sun.star.text.ReferenceFieldPart.NUMBER_FULL_CONTEXT, FieldResult7 );
        }

        // insert a certain cross-reference bookmark and a reference field to this bookmark
        {
            // restart paragraph enumeration
            com.sun.star.container.XEnumerationAccess xParaEnumAccess =
                    UnoRuntime.queryInterface(
            com.sun.star.container.XEnumerationAccess.class, document.getText());
            xParaEnum = xParaEnumAccess.createEnumeration();

            // iterate on the paragraphs to find certain paragraph to insert the bookmark
            com.sun.star.text.XTextRange xParaTextRange = null;
            while ( xParaEnum.hasMoreElements() ) {
                xParaTextRange = UnoRuntime.queryInterface(
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
            XMultiServiceFactory xFac = UnoRuntime.queryInterface(
                    XMultiServiceFactory.class, document);
            final String cBookmarkName = "__RefNumPara__47114711";
            com.sun.star.text.XTextContent xBookmark =
                UnoRuntime.queryInterface(
                com.sun.star.text.XTextContent.class,
                xFac.createInstance( "com.sun.star.text.Bookmark" ) );
            if ( xBookmark != null ) {
                com.sun.star.container.XNamed xName =
                        UnoRuntime.queryInterface(
                com.sun.star.container.XNamed.class, xBookmark );
                xName.setName( cBookmarkName );
                xBookmark.attach(xParaTextRange.getStart());
            }

            // insert reference field, which references the inserted bookmark
            com.sun.star.text.XTextContent xNewField =
                UnoRuntime.queryInterface(
                com.sun.star.text.XTextContent.class,
                xFac.createInstance( "com.sun.star.text.TextField.GetReference" ) );
            if ( xNewField != null ) {
                com.sun.star.beans.XPropertySet xFieldProps =
                        UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xNewField );
                xFieldProps.setPropertyValue( "ReferenceFieldPart", Short.valueOf(com.sun.star.text.ReferenceFieldPart.TEXT) );
                xFieldProps.setPropertyValue( "ReferenceFieldSource", Short.valueOf(com.sun.star.text.ReferenceFieldSource.BOOKMARK) );
                xFieldProps.setPropertyValue( "SourceName", cBookmarkName );
                com.sun.star.text.XTextRange xFieldTextRange =
                        UnoRuntime.queryInterface(
                com.sun.star.text.XTextRange.class, xParaEnum.nextElement());
                xNewField.attach(xFieldTextRange.getEnd());
                xFieldsRefresh.refresh();
            }

            // check inserted reference field
            com.sun.star.text.XTextField xField =
                    UnoRuntime.queryInterface(
            com.sun.star.text.XTextField.class, xNewField );
            assertEquals( "inserted reference field doesn't has correct field result",
                          "J", xField.getPresentation( false ) );

            xParaTextRange.getStart().setString( "Hallo new bookmark: " );
            xFieldsRefresh.refresh();
            assertEquals( "inserted reference field doesn't has correct field result",
                          "Hallo new bookmark: J", xField.getPresentation( false ) );
        }
    }

    @Before public void setUpDocument() {
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
