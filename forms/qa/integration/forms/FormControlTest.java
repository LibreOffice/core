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
package integration.forms;

import com.sun.star.awt.XImageProducer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.form.runtime.XFormController;
import com.sun.star.form.XImageProducerSupplier;
import com.sun.star.frame.XDispatch;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.SQLErrorEvent;
import com.sun.star.sdb.XSQLErrorBroadcaster;
import com.sun.star.sdb.XSQLErrorListener;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XNamingService;
import com.sun.star.util.URL;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XURLTransformer;
import connectivity.tools.HsqlDatabase;
import connectivity.tools.sdb.Connection;
import java.io.FileOutputStream;


public class FormControlTest extends complexlib.ComplexTestCase implements XSQLErrorListener
{
    private static String s_tableName        = "CTC_form_controls";

    private HsqlDatabase            m_databaseDocument;
    private XDataSource             m_dataSource;
    private XPropertySet            m_dataSourceProps;
    private XMultiServiceFactory    m_orb;
    private DocumentHelper          m_document;
    private FormLayer               m_formLayer;
    private XPropertySet            m_masterForm;
    private XFormController         m_masterFormController;
    private String                  m_sImageURL;
    private SQLErrorEvent           m_mostRecentErrorEvent;

    private final String            m_dataSourceName = "integration.forms.FormControlTest";

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkFirstRow",
            "checkInsertRow",
            "checkImageControl",
            "checkCrossUpdates_checkBox",
            "checkCrossUpdates_radioButton",
            "checkRowUpdates",
            "checkEmptyIsNull"
        };
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return "Database Form Controls Test";
    }

    /* ------------------------------------------------------------------ */
    /// pre-test initialization
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // ensure that we have a data source to work with, and the required tables
        if ( !ensureDataSource() || !ensureTables() )
        {
            failed( "could not access the required data source or table therein." );
            return;
        }

        // create the document which we work on
        createSampleDocument();

        createImageFile();
    }

    /* ------------------------------------------------------------------ */
    public void checkFirstRow() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        moveToFirst();

        // and check the content of the various controls
        if  (  !checkRadios( (short)1, (short)0, (short)0 )
            || !checkDoubleValue( 1,            "ID",               "Value"          )
            || !checkDoubleValue( 42,           "f_integer",        "EffectiveValue" )
            || !checkStringValue( "the answer", "f_text",           "Text"           )
            || !checkDoubleValue( 0.12,         "f_decimal",        "Value"          )
            || !checkIntValue   ( 20030922,     "f_date",           "Date"           )
            || !checkIntValue   ( 15000000,     "f_time",           "Time"           )
            || !checkIntValue   ( 20030923,     "f_timestamp_date", "Date"           )
            || !checkIntValue   ( 17152300,     "f_timestamp_time", "Time"           )
            || !checkShortValue ( (short)1,     "f_tinyint",        "State"          )
            )
        {
            failed( "checking the content of one or more controls on the first row failed (see the log for details)" );
            return;
        }
    }

    /* ------------------------------------------------------------------ */
    public void checkInsertRow() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // move the cursor to the insert row
        moveToInsertRow();

        // and check the content of the various controls
        if  ( !verifyCleanInsertRow() )
        {
            failed( "checking the content of one or more controls on the insert row failed (see the log for details)" );
            return;
        }
    }

    /* ------------------------------------------------------------------ */
    /// some tests with the image control
    public void checkImageControl() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // since we did not yet insert any image, the control should not display one ...
        moveToFirst();
        if ( !verifyReferenceImage( new byte[0] ) )
        {
            failed( "image control failed to display empty image" );
            return;
        }

        // check if the image control is able to insert our sample image into the database
        // insert an
        XPropertySet xImageModel = getControlModel( "f_blob" );
        xImageModel.setPropertyValue( "ImageURL", m_sImageURL );

        if ( !verifyReferenceImage( getSamplePictureBytes() ) )
        {
            failed( "image control does not display the sample image as required" );
            return;
        }

        // save the record
        saveRecordByUI();

        // still needs to be the sample image
        if ( !verifyReferenceImage( getSamplePictureBytes() ) )
        {
            failed( "image control does not, after saving the record, display the sample image as required" );
            return;
        }

        // on the next record, the image should be empty
        moveToNext();
        if ( !verifyReferenceImage( new byte[0] ) )
        {
            failed( "image control failed to display empty image, after coming from a non-empty image" );
            return;
        }

        // back to the record where we just inserted the image, it should be our sample image
        moveToFirst();
        if ( !verifyReferenceImage( getSamplePictureBytes() ) )
        {
            failed( "image control does not, after coming back to the record, display the sample image as required" );
            return;
        }

        // okay, now remove the image
        xImageModel.setPropertyValue( "ImageURL", new String() );
        if ( !verifyReferenceImage( new byte[0] ) )
        {
            failed( "image control failed to remove the image" );
            return;
        }
        nextRecordByUI();
        previousRecordByUI();
        if ( !verifyReferenceImage( new byte[0] ) )
        {
            failed( "image still there after coming back, though we just removed it" );
            return;
        }
    }

    /* ------------------------------------------------------------------ */
    /** This is both a test for controls which are bound to the same column (they must reflect
     *  each others updates), and for the immediate updates which need to happen for both check
     *  boxes and radio buttons: They must commit their content to the underlying column as soon
     *  as the change is made, *not* only upon explicit commit
     */
    public void checkCrossUpdates_checkBox() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // move to the first record
        moveToFirst();
        if  (  !checkShortValue ( (short)1, "f_tinyint", "State" )
            || !checkDoubleValue( 1, "f_tinyint_format", "EffectiveValue" )
            )
        {
            failed( "huh? inconsistence in the test!" );
            // we created the sample data in a way that the f_tinyint field should contain a "1" at the first
            // record. We already asserted the proper function of the check box in checkFirstRow, so if this
            // fails here, the script became inconsistent
            return;
        }

        XPropertySet checkModel = getControlModel( "f_tinyint" );
        checkModel.setPropertyValue( "State", new Short( (short)0 ) );

        // setting the state of the check box needs to be reflected in the formatted field immediately
        if ( !checkDoubleValue( 0, "f_tinyint_format", "EffectiveValue" ) )
        {
            failed( "cross-update failed: updating the check box should result in updating the same-bound formatted field (1)!" );
            return;
        }

        // same for the "indetermined" state of the check box
        checkModel.setPropertyValue( "State", new Short( (short)2 ) );
        if ( !checkNullValue( "f_tinyint_format", "EffectiveValue" ) )
        {
            failed( "cross-update failed: updating the check box should result in updating the same-bound formatted field (2)!" );
            return;
        }

        // undo the changes done so far
        undoRecordByUI();
        // and see if this is properly reflected in the controls
        if  (  !checkShortValue ( (short)1, "f_tinyint", "State" )
            || !checkDoubleValue( 1, "f_tinyint_format", "EffectiveValue" )
            )
        {
            failed( "either the check box or the formatted field failed to recognize the UNDO!" );
            return;
        }

        // the other way round - when changing the formatted field - the change should *not*
        // be reflected to the check box, since the formatted field needs an explicit commit
        XPropertySet tinyFormattedModel = getControlModel( "f_tinyint_format" );
        m_document.getCurrentView().grabControlFocus( tinyFormattedModel );
        m_formLayer.userTextInput( tinyFormattedModel, "0" );
        if  (  !checkShortValue ( (short)1, "f_tinyint", "State" )
            )
        {
            failed( "the check box should not be updated here! (did the formatted model commit immediately?)" );
            return;
        }

        // set the focus to *any* other control (since we just have it at hand, we use the check box control)
        // this should result in the formatted control being committed, and thus in the check box updating
        m_document.getCurrentView().grabControlFocus( checkModel );
        if  (  !checkShortValue ( (short)0, "f_tinyint", "State" )
            )
        {
            failed( "formatted field did not commit (or check box did not update)" );
            return;
        }

        // undo the changes done so far, so we leave the document in a clean state for the next test
        undoRecordByUI();
    }

    /* ------------------------------------------------------------------ */
    /** very similar to checkCrossUpdates_checkBox - does nearly the same for the radio buttons. See there for more
     *  explanations.
     */
    public void checkCrossUpdates_radioButton() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // move to the first record
        moveToFirst();
        if  (  !checkRadios( (short)1, (short)0, (short)0 )
            || !checkStringValue( "none", "f_text_enum_text", "Text" )
            )
        {
            failed( "huh? inconsistence in the test!" );
            return;
        }

        XPropertySet radioModel = getRadioModel( "radio_group", "normal" );
        radioModel.setPropertyValue( "State", new Short( (short)1 ) );

        // setting the state of the radio button needs to be reflected in the formatted field immediately
        if ( !checkStringValue( "normal", "f_text_enum_text", "Text" ) )
        {
            failed( "cross-update failed: updating the radio button should result in updating the same-bound text field (1)!" );
            return;
        }

        // same for the "indetermined" state of the check box
        getRadioModel( "radio_group", "important" ).setPropertyValue( "State", new Short( (short)1 ) );
        if ( !checkStringValue( "important", "f_text_enum_text", "Text" ) )
        {
            failed( "cross-update failed: updating the radio button should result in updating the same-bound text field (2)!" );
            return;
        }

        // undo the changes done so far
        undoRecordByUI();
        // and see if this is properly reflected in the controls
        if  (  !checkRadios( (short)1, (short)0, (short)0 )
            || !checkStringValue( "none", "f_text_enum_text", "Text" )
            )
        {
            failed( "either the radio button or the text field failed to recognize the UNDO!" );
            return;
        }

        // the other way round - when changing the formatted field - the change should *not*
        // be reflected to the check box, since the formatted field needs an explicit commit
        XPropertySet textModel = getControlModel( "f_text_enum_text" );
        m_document.getCurrentView().grabControlFocus( textModel );
        m_formLayer.userTextInput( textModel, "normal" );
        if  (  !checkRadios( (short)1, (short)0, (short)0 )
            )
        {
            failed( "the radio buttons should not be updated here! (did the formatted model commit immediately?)" );
            return;
        }

        // set the focus to *any* other control (since we just have it at hand, we use the check box control)
        // this should result in the formatted control being committed, and thus in the check box updating
        m_document.getCurrentView().grabControlFocus( radioModel );
        if  (  !checkRadios( (short)0, (short)1, (short)0 )
            )
        {
            failed( "text field did not commit (or radio button did not update)" );
            return;
        }

        // undo the changes done so far, so we leave the document in a clean state for the next test
        undoRecordByUI();
    }

    /* ------------------------------------------------------------------ */
    /** some tests with updating the table via our controls
     */
    public void checkRowUpdates() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // start with inserting a new record
        moveToInsertRow();
        assure( "insert row not in expected clean state", verifyCleanInsertRow() );

        userTextInput( "ID", "3", true );
        userTextInput( "f_integer", "729", true );
        userTextInput( "f_text", "test", true );
        userTextInput( "f_decimal", "152343", true );
        userTextInput( "f_date", "31.12.1999", true );
        userTextInput( "f_time", "23:59:59", true );

        // move to the next row, this should automatically commit the changes we made
        nextRecordByUI();
        // and back to the row we just inserted
        previousRecordByUI();

        if  (  !checkDoubleValue( 3,        "ID",        "Value" )
            || !checkDoubleValue( 729,      "f_integer", "EffectiveValue" )
            || !checkStringValue( "test",   "f_text",    "Text" )
            || !checkDoubleValue( 152343,   "f_decimal", "Value" )
            || !checkIntValue   ( 19991231, "f_date",    "Date" )
            || !checkIntValue   ( 23595900, "f_time",    "Time" )
            )
        {
            failed( "the changes we made on the insert row have not been committed" );
            return;
        }

        // now change the data, to see if regular updates work, too
        userTextInput( "ID", "4", true );
        userTextInput( "f_integer", "618", true );
        userTextInput( "f_text", "yet another stupid, meaningless text", true );
        userTextInput( "f_required_text", "this must not be NULL", true );
        userTextInput( "f_decimal", "4562", true );
        userTextInput( "f_date", "26.03.2004", true );
        userTextInput( "f_time", "17:05:00", true );

        // move to the next row, this should automatically commit the changes we made
        nextRecordByUI();
        // and back to the row we just inserted
        previousRecordByUI();

        if  (  !checkDoubleValue( 4,        "ID",        "Value" )
            || !checkDoubleValue( 618,      "f_integer", "EffectiveValue" )
            || !checkStringValue( "yet another stupid, meaningless text",   "f_text",    "Text" )
            || !checkDoubleValue( 4562,     "f_decimal", "Value" )
            || !checkIntValue   ( 20040326, "f_date",    "Date" )
            || !checkIntValue   ( 17050000, "f_time",    "Time" )
            )
        {
            failed( "the changes we made on the insert row have not been committed" );
            return;
        }

        m_document.getCurrentView().grabControlFocus( getControlModel( "ID" ) );
    }

    /* ------------------------------------------------------------------ */
    /** checks the "ConvertEmptyToNull" property behavior of an edit control
     *
     */
    public void checkEmptyIsNull() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // start with inserting a new record
        moveToInsertRow();
        assure( "insert row not in expected clean state", verifyCleanInsertRow() );

        // make an input in any field, but leave the edit control which is bound to a required field
        // empty
        userTextInput( "ID", "5", true );
        userTextInput( "f_text", "more text", true );

        // this should *not* fail. Even if we did not input anything into the control bound to the
        // f_required_text column, this control's reset (done when moving to the insertion row) is
        // expected to write an empty string into its bound column, since its EmptyIsNULL property
        // is set to FALSE
        // (#i92471#)
        m_mostRecentErrorEvent = null;
        nextRecordByUI();
        assure( "updating an incomplete record did not work as expected", m_mostRecentErrorEvent == null );
    }

    /* ------------------------------------------------------------------ */
    private boolean verifyCleanInsertRow( ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // and check the content of the various controls
        return (  checkRadios( (short)0, (short)0, (short)0          )
               && checkShortValue( (short)2,    "f_tinyint", "State" )
               && checkStringValue( "", "f_text",  "Text"            )
               && checkNullValue( "ID",        "Value"               )
               && checkNullValue( "f_integer", "EffectiveValue"      )
               && checkNullValue( "f_decimal", "Value"               )
               );
    }

    /* ------------------------------------------------------------------ */
    /// post-test cleanup
    public void after() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // close our document
        if ( m_document != null )
        {
            XCloseable closeDoc = (XCloseable)UnoRuntime.queryInterface( XCloseable.class,
                m_document.getDocument() );
            closeDoc.close( true );
        }
    }

    //=========================================================================
    /* ------------------------------------------------------------------ */
    private boolean ensureDataSource() throws Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();

        XNameAccess databaseContext = (XNameAccess)UnoRuntime.queryInterface( XNameAccess.class,
            m_orb.createInstance( "com.sun.star.sdb.DatabaseContext" ) );
        XNamingService namingService = (XNamingService)UnoRuntime.queryInterface( XNamingService.class,
            databaseContext );

        // revoke the data source, if it previously existed
        if ( databaseContext.hasByName( m_dataSourceName ) )
            namingService.revokeObject( m_dataSourceName );

        // // create a new ODB file, and register it with its URL
        m_databaseDocument = new HsqlDatabase( m_orb );
        String documentURL = m_databaseDocument.getDocumentURL();
        namingService.registerObject( m_dataSourceName, databaseContext.getByName( documentURL ) );

        m_dataSource = (XDataSource)UnoRuntime.queryInterface( XDataSource.class,
            databaseContext.getByName( m_dataSourceName ) );
        m_dataSourceProps = dbfTools.queryPropertySet( m_dataSource );

        XPropertySet dataSourceSettings = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class,
            m_dataSourceProps.getPropertyValue( "Settings" ) );
        dataSourceSettings.setPropertyValue( "FormsCheckRequiredFields", new Boolean( false ) );

        return m_dataSource != null;
    }

    /* ------------------------------------------------------------------ */
    /** retrives the control model with the given name
    */
    private XPropertySet getControlModel( String name ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XNameAccess nameAccess = (XNameAccess)UnoRuntime.queryInterface( XNameAccess.class,
            m_masterForm );
        return (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class,
            nameAccess.getByName( name ) );
    }

    /* ------------------------------------------------------------------ */
    private void createSampleDocument() throws com.sun.star.uno.Exception, java.lang.Exception
    {

        m_document = DocumentHelper.blankTextDocument( m_orb );
        m_formLayer = new FormLayer( m_document );

        // insert some controls
        XPropertySet xIDField =     m_formLayer.insertControlLine( "DatabaseNumericField",  "ID",               "",       3 );
                                    m_formLayer.insertControlLine( "DatabaseFormattedField","f_integer",        "",       11 );
                                    m_formLayer.insertControlLine( "DatabaseTextField",     "f_text",           "",       19 );
        XPropertySet xReqField =    m_formLayer.insertControlLine( "DatabaseTextField",     "f_required_text",  "",       27 );
                                    m_formLayer.insertControlLine( "DatabaseNumericField",  "f_decimal",        "",       35 );
                                    m_formLayer.insertControlLine( "DatabaseDateField",     "f_date",           "",       43 );
        XPropertySet xTimeField =   m_formLayer.insertControlLine( "DatabaseTimeField",     "f_time",           "",       51 );
                                    m_formLayer.insertControlLine( "DatabaseDateField",     "f_timestamp",      "_date",  59 );
                                    m_formLayer.insertControlLine( "DatabaseTimeField",     "f_timestamp",      "_time",  67 );
        XPropertySet xImageField =  m_formLayer.insertControlLine( "DatabaseImageControl",  "f_blob",           "",       2, 75, 40 );
                                    m_formLayer.insertControlLine( "DatabaseTextField",     "f_text_enum",      "_text",  80, 25, 6 );
        XPropertySet xCheckBox =    m_formLayer.insertControlLine( "DatabaseCheckBox",      "f_tinyint",        "",       80, 33, 6 );
                                    m_formLayer.insertControlLine( "DatabaseFormattedField","f_tinyint",        "_format",80, 41, 6 );
                                    m_formLayer.insertControlLine( "DatabaseTextField",     "dummy",            "", 150 );

        xIDField.setPropertyValue( "DecimalAccuracy", new Short( (short)0 ) );
        xImageField.setPropertyValue( "ScaleImage", new Boolean( true) );
        xImageField.setPropertyValue( "Tabstop", new Boolean( true ) );
        xCheckBox.setPropertyValue( "TriState", new Boolean( true ) );
        xCheckBox.setPropertyValue( "DefaultState", new Short( (short)2 ) );
        xTimeField.setPropertyValue( "TimeFormat", new Short( (short)1 ) );
        xTimeField.setPropertyValue( "TimeMax", new Integer( 23595999 ) );
        xReqField.setPropertyValue( "ConvertEmptyToNull", new Boolean( false ) );

        // the logical form
        m_masterForm = (XPropertySet)dbfTools.getParent( xIDField, XPropertySet.class );
        m_masterForm.setPropertyValue( "DataSourceName", m_dataSourceProps.getPropertyValue( "Name" ) );
        m_masterForm.setPropertyValue( "CommandType", new Integer( CommandType.TABLE ) );
        m_masterForm.setPropertyValue( "Command", s_tableName );

        insertRadio( 3, "none", "none" );
        insertRadio( 10, "normal", "normal" );
        insertRadio( 17, "important", "important" );

        // switch the forms into data entry mode
        m_document.getCurrentView( ).toggleFormDesignMode( );

        m_masterFormController = m_document.getCurrentView().getFormController( m_masterForm );
        XSQLErrorBroadcaster errorBroadcaster = (XSQLErrorBroadcaster)UnoRuntime.queryInterface( XSQLErrorBroadcaster.class,
            m_masterFormController );
        errorBroadcaster.addSQLErrorListener( this );

        // set the focus to the ID control
        m_document.getCurrentView().grabControlFocus( xIDField );
    }

    /* ------------------------------------------------------------------ */
    private void insertRadio( int nYPos, String label, String refValue ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet xRadio = m_formLayer.createControlAndShape( "DatabaseRadioButton", 106, nYPos, 25, 6 );
        xRadio.setPropertyValue( "Label", label );
        xRadio.setPropertyValue( "RefValue", refValue );
        xRadio.setPropertyValue( "Name", new String( "radio_group" ) );
        xRadio.setPropertyValue( "DataField", new String( "f_text_enum" ) );
    }

    /* ------------------------------------------------------------------ */
    private String getCreateTableStatement( )
    {
        String sCreateTableStatement = "CREATE TABLE \"" + s_tableName + "\" (";
        sCreateTableStatement += "\"ID\" INTEGER NOT NULL PRIMARY KEY,";
        sCreateTableStatement += "\"f_integer\" INTEGER default NULL,";
        sCreateTableStatement += "\"f_text\" VARCHAR(50) default NULL,";
        sCreateTableStatement += "\"f_required_text\" VARCHAR(50) NOT NULL,";
        sCreateTableStatement += "\"f_decimal\" DECIMAL(10,2) default NULL,";
        sCreateTableStatement += "\"f_date\" DATE default NULL,";
        sCreateTableStatement += "\"f_time\" TIME default NULL,";
        sCreateTableStatement += "\"f_timestamp\" DATETIME default NULL,";
        sCreateTableStatement += "\"f_blob\" VARBINARY,";
        sCreateTableStatement += "\"f_text_enum\" VARCHAR(50) default NULL,";
        sCreateTableStatement += "\"f_tinyint\" TINYINT default NULL";
        sCreateTableStatement += ");";
        return sCreateTableStatement;
    }

    /* ------------------------------------------------------------------ */
    private String[] getSampleDataValueString( ) throws java.lang.Exception
    {
        String[] aValues =  new String[] {
            "1,42,'the answer','foo',0.12,'2003-09-22','15:00:00','2003-09-23 17:15:23',NULL,'none',1",
            "2,13,'the question','bar',12.43,'2003-09-24','16:18:00','2003-09-24 08:45:12',NULL,'none',0"
        };
        return aValues;
    }

    /* ------------------------------------------------------------------ */
    private boolean ensureTables() throws com.sun.star.uno.Exception,  java.lang.Exception
    {
        Connection connection = new Connection( m_dataSource.getConnection( "", "" ) );
        assure( "could not connect to the data source", connection != null );

        // drop the table, if it already exists
        if  (  !implExecuteStatement( "DROP TABLE \"" + s_tableName + "\" IF EXISTS" )
            || !implExecuteStatement( getCreateTableStatement() )
            )
        {
            failed( "could not create the required sample table!" );
            return false;
        }

        String sInsertionPrefix = "INSERT INTO \"" + s_tableName + "\" VALUES (";
        String[] aValues = getSampleDataValueString();
        for ( int i=0; i<aValues.length; ++i )
            if ( !implExecuteStatement( sInsertionPrefix + aValues[ i ] + ")" ) )
            {
                failed( "could not create the required sample data" );
                return false;
            }

        connection.refreshTables();

        // do not need the connection anymore
        connection.close();

        return true;
    }

    /* ------------------------------------------------------------------ */
    /// checks the 3 radio buttons for the given states
    private boolean checkRadios( short stateNone, short stateNormal, short stateImportant ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        if ( ((Short)getRadioModel( "radio_group", "none" ).getPropertyValue( "State" )).shortValue() != stateNone )
        {
            failed( "wrong value of the 'none' radio button!" );
        }
        else if ( ((Short)getRadioModel( "radio_group", "normal" ).getPropertyValue( "State" )).shortValue() != stateNormal )
        {
            failed( "wrong value of the 'normal' radio button!" );
        }
        else if ( ((Short)getRadioModel( "radio_group", "important" ).getPropertyValue( "State" )).shortValue() != stateImportant )
        {
            failed( "wrong value of the 'important' radio button!" );
        }
        else
            return true;

        return false;
    }

    /* ------------------------------------------------------------------ */
    private boolean checkNullValue( String fieldName, String propertyName ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        Object value = getControlModel( fieldName ).getPropertyValue( propertyName );
        if ( !util.utils.isVoid( value ) )
        {
            log.println( "wrong value of the " + fieldName + " field!" );
            log.println( "  expected: <null/>" );
            log.println( "  found   : " + value.toString() );
        }
        else
            return true;

        return false;
    }

    /* ------------------------------------------------------------------ */
    private boolean checkIntValue( int requiredValue, String fieldName, String propertyName ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        try
        {
            if ( ( "f_time" ).equals(fieldName) )
                // http://bugs.mysql.com/bug.php?id=5681
                return true;
            if (fieldName == null) {
                return false;
            }
            int currentValue = ((Integer)getControlModel( fieldName ).getPropertyValue( propertyName )).intValue();
            if ( currentValue != requiredValue )
            {
                log.println( "wrong value of the " + fieldName + " field!" );
                log.println( "  expected: " + String.valueOf( requiredValue ) );
                log.println( "  found   : " + String.valueOf( currentValue ) );
            }
            else
                return true;
        }
        catch( com.sun.star.uno.Exception e )
        {
            log.println( "caught an exception while retrieving property value '" + propertyName + "' of control model '" + fieldName + "'" );
            throw e;
        }

        return false;
    }

    /* ------------------------------------------------------------------ */
    private boolean checkShortValue( short requiredValue, String fieldName, String propertyName ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        try
        {
            short currentValue = ((Short)getControlModel( fieldName ).getPropertyValue( propertyName )).shortValue();
            if ( currentValue != requiredValue )
            {
                log.println( "wrong value of the " + fieldName + " field!" );
                log.println( "  expected: " + String.valueOf( requiredValue ) );
                log.println( "  found   : " + String.valueOf( currentValue ) );
            }
            else
                return true;
        }
        catch( com.sun.star.uno.Exception e )
        {
            log.println( "caught an exception while retrieving property value '" + propertyName + "' of control model '" + fieldName + "'" );
            throw e;
        }

        return false;
    }

    /* ------------------------------------------------------------------ */
    private boolean checkDoubleValue( double requiredValue, String fieldName, String propertyName ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        double currentValue = ((Double)getControlModel( fieldName ).getPropertyValue( propertyName )).doubleValue();
        if ( currentValue != requiredValue )
        {
            log.println( "wrong value of the " + fieldName + " field!" );
            log.println( "  expected: " + String.valueOf( requiredValue ) );
            log.println( "  found   : " + String.valueOf( currentValue ) );
        }
        else
            return true;

        return false;
    }

    /* ------------------------------------------------------------------ */
    private boolean checkStringValue( String requiredValue, String fieldName, String propertyName ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        String currentValue = (String)getControlModel( fieldName ).getPropertyValue( propertyName );
        if ( !currentValue.equals( requiredValue ) )
        {
            log.println( "wrong value of the " + fieldName + " field!" );
            log.println( "  expected: " + requiredValue );
            log.println( "  found   : " + currentValue );
        }
        else
            return true;

        return false;
    }

    /* ------------------------------------------------------------------ */
    private XPropertySet getRadioModel( String name, String refValue ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        return m_formLayer.getRadioModelByRefValue( m_masterForm, name, refValue );
    }

    /* ------------------------------------------------------------------ */
    /** executes the given statement on the given connection
    */
    protected boolean implExecuteStatement( String sStatement ) throws java.lang.Exception
    {
        try
        {
            m_databaseDocument.executeSQL( sStatement );
        }
        catch(com.sun.star.sdbc.SQLException e)
        {
            System.err.println( e );
            return false;
        }

        return true;
    }

    /* ------------------------------------------------------------------ */
    /** simulates a user's text input into a control given by model name
     */
    private void userTextInput( String modelName, String text, boolean withCommit ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet controlModel = getControlModel( modelName );
        // the form runtime environment (namely the form controller) rely on focus events for recognizing
        // control content changes ...
        if ( withCommit )
            m_document.getCurrentView().grabControlFocus( controlModel );

        m_formLayer.userTextInput( controlModel, text );

        // focus back to a dummy control model so the content of the model we just changed will
        // be committed to the underlying database column
        if ( withCommit )
            m_document.getCurrentView().grabControlFocus( getControlModel( "dummy" ) );
    }

    /* ------------------------------------------------------------------ */
    private void moveToInsertRow() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XResultSetUpdate xResultSet = (XResultSetUpdate)UnoRuntime.queryInterface( XResultSetUpdate.class, m_masterForm );
        xResultSet.moveToInsertRow( );
    }

    /* ------------------------------------------------------------------ */
    private void moveToFirst() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XResultSet xResultSet = (XResultSet)UnoRuntime.queryInterface( XResultSet.class, m_masterForm );
        xResultSet.first( );
    }

    /* ------------------------------------------------------------------ */
    private void moveToNext() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XResultSet xResultSet = (XResultSet)UnoRuntime.queryInterface( XResultSet.class, m_masterForm );
        xResultSet.next( );
    }

    /* ------------------------------------------------------------------ */
    /** simulates pressing a toolbox button with the given URL
     */
    private void executeSlot( String slotURL ) throws java.lang.Exception
    {
        XDispatch xDispatch = m_document.getCurrentView().getDispatcher( slotURL );

        URL[] url = new URL[] { new URL() };
        url[0].Complete = slotURL;
        XURLTransformer xTransformer = (XURLTransformer)UnoRuntime.queryInterface(
                XURLTransformer.class, m_orb.createInstance( "com.sun.star.util.URLTransformer" ) );
        xTransformer.parseStrict( url );

        PropertyValue[] aArgs = new PropertyValue[0];
        xDispatch.dispatch( url[0], aArgs );
    }

    /* ------------------------------------------------------------------ */
    /** undos the changes on the current record, by simulating pressing of the respective toolbox button
     */
    private void undoRecordByUI() throws java.lang.Exception
    {
        executeSlot( ".uno:RecUndo" );
    }

    /* ------------------------------------------------------------------ */
    /** saves the current record, by simulating pressing of the respective toolbox button
     */
    private void saveRecordByUI() throws java.lang.Exception
    {
        executeSlot( ".uno:RecSave" );
    }

    /* ------------------------------------------------------------------ */
    /** moves to the next record, by simulating pressing of the respective toolbox button
     */
    private void nextRecordByUI() throws java.lang.Exception
    {
        executeSlot( ".uno:NextRecord" );
    }
    /* ------------------------------------------------------------------ */
    /** moves to the previous record, by simulating pressing of the respective toolbox button
     */
    private void previousRecordByUI() throws java.lang.Exception
    {
        executeSlot( ".uno:PrevRecord" );
    }

    /* ------------------------------------------------------------------ */
    private void createImageFile() throws java.io.IOException
    {
        m_sImageURL = util.utils.getOfficeTempDir( m_orb ) + "image.gif";

        FileOutputStream aFile = new FileOutputStream( m_sImageURL );
        aFile.write( getSamplePicture() );
        aFile.close();
        log.println( "created temporary image file: " + m_sImageURL );

        // for later setting the url at the imaghe control, we need a real URL, no system path
        m_sImageURL = util.utils.getOfficeTemp( m_orb ) + "image.gif";
    }

    /* ------------------------------------------------------------------ */
    private byte[] getSamplePicture()
    {
        byte[] aBytes = new byte[] {
            (byte)0x47, (byte)0x49, (byte)0x46, (byte)0x38, (byte)0x39, (byte)0x61, (byte)0x0A, (byte)0x00, (byte)0x0A, (byte)0x00, (byte)0xB3, (byte)0x00, (byte)0x00, (byte)0xFF, (byte)0x00, (byte)0x00,
            (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0x00, (byte)0x00, (byte)0xFF, (byte)0xFF, (byte)0x00, (byte)0xFF, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0xFF, (byte)0xFF,
            (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF,
            (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0x2C, (byte)0x00, (byte)0x00,
            (byte)0x00, (byte)0x00, (byte)0x0A, (byte)0x00, (byte)0x0A, (byte)0x00, (byte)0x00, (byte)0x04, (byte)0x20, (byte)0x10, (byte)0xC8, (byte)0x49, (byte)0x41, (byte)0xB9, (byte)0xF8, (byte)0xCA,
            (byte)0x12, (byte)0xBA, (byte)0x2F, (byte)0x5B, (byte)0x30, (byte)0x8C, (byte)0x43, (byte)0x00, (byte)0x5A, (byte)0x22, (byte)0x41, (byte)0x94, (byte)0x27, (byte)0x37, (byte)0xA8, (byte)0x6C,
            (byte)0x48, (byte)0xC6, (byte)0xA8, (byte)0xD7, (byte)0xB5, (byte)0x19, (byte)0x56, (byte)0xED, (byte)0x11, (byte)0x00, (byte)0x3B
        };

        return aBytes;
    }

    /* ------------------------------------------------------------------ */
    private byte[] getSamplePictureBytes()
    {
        byte[] aBytes = new byte[] {
            (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x05, (byte)0x05, (byte)0x05, (byte)0x05, (byte)0x05,
            (byte)0x05, (byte)0x05, (byte)0x05, (byte)0x00, (byte)0x00, (byte)0x05, (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x05, (byte)0x00, (byte)0x00, (byte)0x05,
            (byte)0x01, (byte)0x03, (byte)0x03, (byte)0x03, (byte)0x03, (byte)0x01, (byte)0x05, (byte)0x00, (byte)0x00, (byte)0x05, (byte)0x01, (byte)0x03, (byte)0x04, (byte)0x04, (byte)0x03, (byte)0x01,
            (byte)0x05, (byte)0x00, (byte)0x00, (byte)0x05, (byte)0x01, (byte)0x03, (byte)0x04, (byte)0x04, (byte)0x03, (byte)0x01, (byte)0x05, (byte)0x00, (byte)0x00, (byte)0x05, (byte)0x01, (byte)0x03,
            (byte)0x03, (byte)0x03, (byte)0x03, (byte)0x01, (byte)0x05, (byte)0x00, (byte)0x00, (byte)0x05, (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x05, (byte)0x00,
            (byte)0x00, (byte)0x05, (byte)0x05, (byte)0x05, (byte)0x05, (byte)0x05, (byte)0x05, (byte)0x05, (byte)0x05, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00,
            (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00
        };
        return aBytes;
    }

    /* ------------------------------------------------------------------ */
    private boolean verifyReferenceImage( byte[] referenceBytes ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet xImageModel = getControlModel( "f_blob" );

        // check if the image control properly says that there currently is no image on the first record
        XImageProducerSupplier xSuppProducer = (XImageProducerSupplier)UnoRuntime.queryInterface( XImageProducerSupplier.class,
            xImageModel );
        XImageProducer xProducer = xSuppProducer.getImageProducer();

        ImageComparison compareImages = new ImageComparison( referenceBytes, this );
        synchronized( this )
        {
            xProducer.addConsumer( compareImages );
            xProducer.startProduction();
//            wait();
        }
        xProducer.removeConsumer( compareImages );

        return compareImages.imagesEqual( );
    }

    /* ------------------------------------------------------------------ */
    public void errorOccured( SQLErrorEvent _event )
    {
        // just remember for the moment
        m_mostRecentErrorEvent = _event;
    }

    /* ------------------------------------------------------------------ */
    public void disposing( EventObject _event )
    {
        // not interested in
    }
}
