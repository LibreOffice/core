/*************************************************************************
 *
 *  $RCSfile: FormControlTest.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:53:03 $
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
package integration.forms;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Any;

import com.sun.star.container.XNameAccess;
import com.sun.star.container.XIndexContainer;
import com.sun.star.container.XIndexAccess;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;

import com.sun.star.util.XCloseable;
import com.sun.star.util.URL;

import com.sun.star.sdb.CommandType;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbcx.XTablesSupplier;

import com.sun.star.form.XImageProducerSupplier;

import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;

import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDispatch;

import com.sun.star.awt.XImageProducer;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleEditableText;

import complexlib.ComplexTestCase;
import integration.forms.dbfTools;
import integration.forms.DocumentHelper;
import integration.forms.FormLayer;

import util.utils;
import java.util.*;
import java.io.*;
import java.lang.*;

public class FormControlTest extends ComplexTestCase
{
    private static String s_tableName        = "CTC_form_controls";

    private XDataSource             m_dataSource;
    private XPropertySet            m_dataSourceProps;
    private XMultiServiceFactory    m_orb;
    private DocumentHelper          m_document;
    private FormLayer               m_formLayer;
    private XPropertySet            m_masterForm;
    private String                  m_sImageURL;

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkFirstRow",
            "checkInsertRow",
            "checkImageControl",
            "checkCrossUpdates_checkBox",
            "checkCrossUpdates_radioButton",
            "checkRowUpdates"
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
        userTextInput( tinyFormattedModel, "0", false );
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
        userTextInput( textModel, "normal", false );
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
        if ( !verifyCleanInsertRow() )
        {
            failed( "insert row not in expected clean state" );
            return;
        }

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
    private boolean ensureDataSource() throws com.sun.star.uno.Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();

        XNameAccess aDSContext = (XNameAccess)UnoRuntime.queryInterface( XNameAccess.class,
            m_orb.createInstance( "com.sun.star.sdb.DatabaseContext" ) );

        String sDataSourceName = (String)param.get( "datasource" );
        if ( null == sDataSourceName )
            sDataSourceName = "FormControlTest";
        if ( !aDSContext.hasByName( sDataSourceName ) )
        {
            log.println( "\n" );
            log.println( "There is no data source named '" + sDataSourceName + "'\n" );
            log.println( "You need to create a data source in your office installation, pointing" );
            log.println( "to a database (preferably MySQL, other types not tested so far)" );
            log.println( "which this test should work with.\n" );
            log.println( "The test will automatically create and populate a sample table in this" );
            log.println( "data source.\n" );
            log.println( "Unless specified otherwise in the FormControlTest.props, the expected" );
            log.println( "name of the data source is \"FormControlTest\".\n\n" );
            return false;
        }
        log.println( "using data source " + sDataSourceName );

        m_dataSource = (XDataSource)UnoRuntime.queryInterface( XDataSource.class,
            aDSContext.getByName( sDataSourceName ) );
        m_dataSourceProps = dbfTools.queryXPropertySet( m_dataSource );
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
        // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
        m_document = DocumentHelper.blankTextDocument( m_orb );
        m_formLayer = new FormLayer( m_document );

        // insert some controls
        XPropertySet xIDField =     m_formLayer.insertControlLine( "DatabaseNumericField",  "ID",           "",       3 );
                                    m_formLayer.insertControlLine( "DatabaseFormattedField","f_integer",    "",       11 );
                                    m_formLayer.insertControlLine( "DatabaseTextField",     "f_text",       "",       19 );
        XPropertySet xDecField =    m_formLayer.insertControlLine( "DatabaseNumericField",  "f_decimal",    "",       27 );
                                    m_formLayer.insertControlLine( "DatabaseDateField",     "f_date",       "",       35 );
        XPropertySet xTimeField =   m_formLayer.insertControlLine( "DatabaseTimeField",     "f_time",       "",       43 );
                                    m_formLayer.insertControlLine( "DatabaseDateField",     "f_timestamp",  "_date",  51 );
                                    m_formLayer.insertControlLine( "DatabaseTimeField",     "f_timestamp",  "_time",  59 );
        XPropertySet xImageField =  m_formLayer.insertControlLine( "DatabaseImageControl",  "f_blob",       "",       2, 67, 40 );
                                    m_formLayer.insertControlLine( "DatabaseTextField",     "f_text_enum",  "_text",  80, 25, 6 );
        XPropertySet xCheckBox =    m_formLayer.insertControlLine( "DatabaseCheckBox",      "f_tinyint",    "",       80, 33, 6 );
                                    m_formLayer.insertControlLine( "DatabaseFormattedField","f_tinyint",    "_format",80, 41, 6 );
                                    m_formLayer.insertControlLine( "DatabaseTextField",     "dummy",        "", 150 );

        xIDField.setPropertyValue( "DecimalAccuracy", new Short( (short)0 ) );
        xImageField.setPropertyValue( "ScaleImage", new Boolean( (boolean)true) );
        xImageField.setPropertyValue( "Tabstop", new Boolean( true) );
        xCheckBox.setPropertyValue( "TriState", new Boolean( true ) );
        xCheckBox.setPropertyValue( "DefaultState", new Short( (short)2 ) );
        xTimeField.setPropertyValue( "TimeFormat", new Short( (short)1 ) );
        xTimeField.setPropertyValue( "TimeMax", new Integer( 23595999 ) );

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
        String sCreateTableStatement = "CREATE TABLE CTC_form_controls (";
        sCreateTableStatement += "ID int(11) NOT NULL default '0',";
        sCreateTableStatement += "f_integer int(11) default NULL,";
        sCreateTableStatement += "f_text varchar(50) default NULL,";
        sCreateTableStatement += "f_decimal decimal(10,2) default NULL,";
        sCreateTableStatement += "f_date date default NULL,";
        sCreateTableStatement += "f_time time default NULL,";
        sCreateTableStatement += "f_timestamp datetime default NULL,";
        sCreateTableStatement += "f_blob blob,";
        sCreateTableStatement += "f_text_enum varchar(50) default NULL,";
        sCreateTableStatement += "f_tinyint tinyint(4) default NULL,";
        sCreateTableStatement += "PRIMARY KEY  (ID)";
        sCreateTableStatement += ") TYPE=MyISAM;";
        return sCreateTableStatement;
    }

    /* ------------------------------------------------------------------ */
    private String[] getSampleDataValueString( ) throws java.lang.Exception
    {
        String[] aValues =  new String[] {
            "1,42,'the answer',0.12,'2003-09-22','15:00:00','2003-09-23 17:15:23',NULL,'none',1",
            "2,13,'the question',12.43,'2003-09-24','16:18:00','2003-09-24 08:45:12',NULL,'none',0"
        };
        return aValues;
    }

    /* ------------------------------------------------------------------ */
    private boolean ensureTables() throws com.sun.star.uno.Exception,  java.lang.Exception
    {
        // conenct to the data source
        String user = (String)param.get( "user" );
        if ( ( user == null ) || ( user.length() == 0 ) )
        {
            user = (String)m_dataSourceProps.getPropertyValue( "User" );
            if ( user.length() == 0 )
                user = "testtool";
        }

        String password = (String)param.get( "password" );
        boolean needPasswordOverride = true;

        if ( ( password == null ) || ( password.length() == 0 ) )
        {
            password = (String)m_dataSourceProps.getPropertyValue( "Password" );
            if ( password.length() == 0 )
                password = "testtool";
            else
                needPasswordOverride = false;
        }

        XConnection xConn = m_dataSource.getConnection( user, password );
        if ( xConn == null )
        {
            failed( "could not connect to the data source, authenticating with '" + user + "'/'**********'" );
            return false;
        }
        if ( ( user.length() == 0 ) && ( password.length() == 0 ) )
            log.println( "connected to the data source" );
        else
            log.println( "connected to the data source, authenticated with '" + user + "'/'**********'" );

        // if we successfully connected, remember the password. Later on, when the form document is switched
        // to alive mode, this is needed, since we unfortunately have no chance to pass the password to the
        // form to use when connecting
        if ( needPasswordOverride )
        {
            m_dataSourceProps.setPropertyValue( "User", user );
            m_dataSourceProps.setPropertyValue( "Password", password );
        }

        // drop the table, if it already exists
        if  (  !implExecuteStatement( xConn, "DROP TABLE IF EXISTS CTC_form_controls" )
            || !implExecuteStatement( xConn, getCreateTableStatement() )
            )
        {
            failed( "could not create the required sample table!" );
            return false;
        }

        String sInsertionPrefix = "INSERT INTO CTC_form_controls VALUES (";
        String[] aValues = getSampleDataValueString();
        for ( int i=0; i<aValues.length; ++i )
            if ( !implExecuteStatement( xConn, sInsertionPrefix + aValues[ i ] + ")" ) )
            {
                failed( "could not create the required sample data" );
                return false;
            }

        XTablesSupplier xSuppTables = (XTablesSupplier)UnoRuntime.queryInterface( XTablesSupplier.class,
            xConn );
        XNameAccess xTables = xSuppTables.getTables();

        if ( !xTables.hasByName( s_tableName ) )
        {
            String sNewTableName = (String)param.get( "TablePrefix" );
            sNewTableName += '.' + s_tableName;
            if ( !xTables.hasByName( sNewTableName ) )
            {
                failed( "could not find the table " + s_tableName );
                return false;
            }
            s_tableName = sNewTableName;
        }

        // do not need the connection anymore
        dbfTools.disposeComponent( xConn );

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
        return m_formLayer.getRadioModel( m_masterForm, name, refValue );
    }

    /* ------------------------------------------------------------------ */
    /** executes the given statement on the given connection
    */
    protected boolean implExecuteStatement( XConnection xConn, String sStatement ) throws java.lang.Exception
    {
        try
        {
            XStatement xStatement = xConn.createStatement( );
            xStatement.execute( sStatement );
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
        userTextInput( getControlModel( modelName ), text, withCommit );
    }

    /* ------------------------------------------------------------------ */
    /** simulates a user's text input into a control given by control model
     */
    private void userTextInput( XPropertySet controlModel, String text, boolean withCommit ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // the form runtime environment (namely the form controller) rely on focus events for recognizing
        // control content changes ...
        if ( withCommit )
            m_document.getCurrentView().grabControlFocus( controlModel );

        // we will *not* simply set the value property at the model. This is not the same as
        // doing a user input, as the latter will trigger a lot of notifications, which the forms runtime environment
        // (namely the FormController) relies on to notice that the control changed.
        // Instead, we use the Accessibility interfaces of the control to simulate text input
        XAccessible formattedAccessible = (XAccessible)UnoRuntime.queryInterface( XAccessible.class,
            m_document.getCurrentView().getControl( controlModel )
        );
        XAccessibleEditableText textAccess = (XAccessibleEditableText)UnoRuntime.queryInterface( XAccessibleEditableText.class,
            formattedAccessible.getAccessibleContext() );
        textAccess.setText( text );

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
        XDispatch xDispatch = (XDispatch)m_document.getCurrentView().getDispatcher( slotURL );
        PropertyValue[] aArgs = new PropertyValue[0];
        URL url = new URL();
        url.Complete = slotURL;
        xDispatch.dispatch( url, aArgs );
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
            wait();
        }
        xProducer.removeConsumer( compareImages );

        return compareImages.imagesEqual( );
    }

}
