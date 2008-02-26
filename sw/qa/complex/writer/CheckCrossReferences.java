/*
 * CheckCrossReferences.java
 *
 * Created on November 1, 2007, 1:49 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package complex.writer;

import complexlib.ComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

/**
 *
 * @author od138299
 */
public class CheckCrossReferences extends ComplexTestCase {

    private com.sun.star.text.XTextDocument xDoc;
    private com.sun.star.container.XEnumeration xParaEnum;
    private com.sun.star.container.XEnumeration xPortionEnum;
    private com.sun.star.util.XRefreshable xFldsRefresh;

    public String[] getTestMethodNames() {
        return new String[]{"checkCrossReferences"};
    }

    public com.sun.star.text.XTextField getNextField() {

        com.sun.star.text.XTextField xField = null;
        if ( xPortionEnum != null ) {
            try {
                while ( xPortionEnum.hasMoreElements() ) {
                    com.sun.star.beans.XPropertySet xPortionProps =
                            (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                            com.sun.star.beans.XPropertySet.class , xPortionEnum.nextElement());
                    final String sPortionType =
                            xPortionProps.getPropertyValue( "TextPortionType" ).toString();
                    if ( sPortionType.equals( "TextField") ) {
                        xField = (com.sun.star.text.XTextField)UnoRuntime.queryInterface(
                                com.sun.star.text.XTextField.class,
                                xPortionProps.getPropertyValue( "TextField" ) );
                        if ( xField == null ) {
                            System.out.println("Cannot retrieve next field.");
                            failed("Cannot retrieve next field.");
                            return null;
                        }
                        return xField;
                    }
                }
            } catch (com.sun.star.container.NoSuchElementException e) {
                System.out.println("Cannot retrieve next field.");
                e.printStackTrace();
                failed(e.getMessage());
                return null;
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                System.out.println("Cannot retrieve next field.");
                e.printStackTrace();
                failed(e.getMessage());
                return null;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                System.out.println("Cannot retrieve next field.");
                e.printStackTrace();
                failed(e.getMessage());
                return null;
            }
        }

        while ( xParaEnum.hasMoreElements() ) {
            try {
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
                        xField = (com.sun.star.text.XTextField)UnoRuntime.queryInterface(
                                com.sun.star.text.XTextField.class,
                                xPortionProps.getPropertyValue( "TextField" ) );
                        if ( xField == null ) {
                            System.out.println("Cannot retrieve next field.");
                            failed("Cannot retrieve next field.");
                            return null;
                        }
                        return xField;
                    }
                }
            } catch (com.sun.star.container.NoSuchElementException e) {
                System.out.println("Cannot retrieve next field.");
                e.printStackTrace();
                failed(e.getMessage());
                return null;
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                System.out.println("Cannot retrieve next field.");
                e.printStackTrace();
                failed(e.getMessage());
                return null;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                System.out.println("Cannot retrieve next field.");
                e.printStackTrace();
                failed(e.getMessage());
                return null;
            }
        }

        if ( xField == null ) {
            System.out.println("Cannot retrieve next field.");
            failed("Cannot retrieve next field.");
            return null;
        }

        return xField;
    }

    public com.sun.star.beans.XPropertySet getFieldProps(
            com.sun.star.text.XTextField xField ) {
        com.sun.star.beans.XPropertySet xProps =
                (com.sun.star.beans.XPropertySet)UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xField );

        if ( xProps == null ) {
            System.out.println("Cannot retrieve field properties.");
            failed("Cannot retrieve field properties.");
            return null;
        }

        return xProps;
    }

    public void checkField( com.sun.star.text.XTextField xField,
            com.sun.star.beans.XPropertySet xProps,
            short nFormat,
            String aExpectedFldResult ) {
        // set requested format
        try {
            xProps.setPropertyValue("ReferenceFieldPart", new Short(nFormat));
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Cannot set ReferenceFieldPart property at field.");
            e.printStackTrace();
            failed(e.getMessage());
            return;
        } catch (com.sun.star.beans.PropertyVetoException e) {
            System.out.println("Cannot set ReferenceFieldPart property at field.");
            e.printStackTrace();
            failed(e.getMessage());
            return;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            System.out.println("Cannot set ReferenceFieldPart property at field.");
            e.printStackTrace();
            failed(e.getMessage());
            return;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            System.out.println("Cannot set ReferenceFieldPart property at field.");
            e.printStackTrace();
            failed(e.getMessage());
            return;
        }
        // refresh fields in order to get new format applied
        xFldsRefresh.refresh();

        String aFldResult = xField.getPresentation( false );
        assure( "set reference field format doesn't result in correct field result",
                aFldResult.equals(aExpectedFldResult), true );
    }

    public void checkCrossReferences() throws com.sun.star.uno.Exception {
        // load test document
        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)param.getMSF();
            xDoc = util.WriterTools.loadTextDoc( xMSF, util.utils.getFullTestURL("CheckCrossReferences.odt"));
        } catch(com.sun.star.uno.RuntimeException e) {
            System.out.println("Cannot load test document.");
            e.printStackTrace();
            failed(e.getMessage());
            return;
        }

        // setup paragraph enumeration
        {
            com.sun.star.container.XEnumerationAccess xParaEnumAccess =
                    (com.sun.star.container.XEnumerationAccess)UnoRuntime.queryInterface(
                    com.sun.star.container.XEnumerationAccess.class, xDoc.getText());
            xParaEnum = xParaEnumAccess.createEnumeration();
        }

        // get field refresher
        {
            com.sun.star.text.XTextFieldsSupplier xFieldSupp =
                    (com.sun.star.text.XTextFieldsSupplier)UnoRuntime.queryInterface(
                    com.sun.star.text.XTextFieldsSupplier.class, xDoc);
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

            log.println( "Checking field reference formats NUMBER, NUMBER_NO_CONTEXT and NUMBER_FULL_CONTEXT for existing fields" );
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
            log.println("Checking insert of cross-reference bookmark and corresponding reference field");
            // restart paragraph enumeration
            com.sun.star.container.XEnumerationAccess xParaEnumAccess =
                    (com.sun.star.container.XEnumerationAccess)UnoRuntime.queryInterface(
                    com.sun.star.container.XEnumerationAccess.class, xDoc.getText());
            xParaEnum = xParaEnumAccess.createEnumeration();

            // iterate on the paragraphs to find certain paragraph to insert the bookmark
            com.sun.star.text.XTextRange xParaTextRange = null;
            while ( xParaEnum.hasMoreElements() ) {
                try {
                    xParaTextRange = (com.sun.star.text.XTextRange)UnoRuntime.queryInterface(
                            com.sun.star.text.XTextRange.class, xParaEnum.nextElement());
                    if ( xParaTextRange.getString().equals( "*i*J" ) ) {
                        break;
                    }
                } catch (com.sun.star.container.NoSuchElementException e) {
                    System.out.println("Cannot find paragraph to insert cross-reference bookmark.");
                    e.printStackTrace();
                    failed(e.getMessage());
                    return;
                } catch (com.sun.star.lang.WrappedTargetException e) {
                    System.out.println("Cannot find paragraph to insert cross-reference bookmark.");
                    e.printStackTrace();
                    failed(e.getMessage());
                    return;
                }
            }
            if ( xParaTextRange == null ) {
                System.out.println("Cannot find paragraph to insert cross-reference bookmark.");
                failed("Cannot find paragraph to insert cross-reference bookmark.");
                return;
            }

            // insert bookmark
            XMultiServiceFactory xFac = (XMultiServiceFactory)UnoRuntime.queryInterface(
                    XMultiServiceFactory.class, xDoc);
            final String cBookmarkName = "__RefNumPara__47114711";
            com.sun.star.text.XTextContent xBookmark = null;
            try {
                xBookmark = (com.sun.star.text.XTextContent)UnoRuntime.queryInterface(
                        com.sun.star.text.XTextContent.class,
                        xFac.createInstance( "com.sun.star.text.Bookmark" ) );
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                System.out.println("Cannot create bookmark.");
                e.printStackTrace();
                failed(e.getMessage());
                return;
            } catch (com.sun.star.uno.Exception e) {
                System.out.println("Cannot create bookmark.");
                e.printStackTrace();
                failed(e.getMessage());
                return;
            }
            if ( xBookmark != null ) {
                com.sun.star.container.XNamed xName =
                        (com.sun.star.container.XNamed)UnoRuntime.queryInterface(
                        com.sun.star.container.XNamed.class, xBookmark );
                xName.setName( cBookmarkName );
                xBookmark.attach(xParaTextRange.getStart());
            }

            // insert reference field, which references the inserted bookmark
            com.sun.star.text.XTextContent xNewField = null;
            try {
                xNewField = (com.sun.star.text.XTextContent)UnoRuntime.queryInterface(
                        com.sun.star.text.XTextContent.class,
                        xFac.createInstance( "com.sun.star.text.TextField.GetReference" ) );
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                System.out.println("Cannot create new field.");
                e.printStackTrace();
                failed(e.getMessage());
                return;
            } catch (com.sun.star.uno.Exception e) {
                System.out.println("Cannot create new field.");
                e.printStackTrace();
                failed(e.getMessage());
                return;
            }
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
            assure( "inserted reference field doesn't has correct field result",
                    xField.getPresentation( false ).equals("J"), true );

            xParaTextRange.getStart().setString( "Hallo new bookmark: " );
            xFldsRefresh.refresh();
            assure( "inserted reference field doesn't has correct field result. Instead it's: "+xField.getPresentation( false ),
                    xField.getPresentation( false ).equals("Hallo new bookmark: J"), true );
        }

        // closing test document
        util.DesktopTools.closeDoc( xDoc );
    }
}
