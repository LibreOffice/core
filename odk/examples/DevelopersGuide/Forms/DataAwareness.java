/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XPropertySet;


// __________ Imports __________
import com.sun.star.beans.XPropertySetInfo;

// base classes
import com.sun.star.container.XIndexContainer;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.form.FormComponentType;
import com.sun.star.form.ListSourceType;
import com.sun.star.form.XGridColumnFactory;
import com.sun.star.form.XReset;
import com.sun.star.form.XResetListener;
import com.sun.star.form.runtime.FormFeature;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.XColumnUpdate;
import com.sun.star.sdbc.ResultSetConcurrency;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDataSource;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**************************************************************************/
/** a class for enumerating a form component tree
*/
class PrintComponentTree extends ComponentTreeTraversal
{
    private String m_sPrefix;

    public PrintComponentTree()
    {
        m_sPrefix = "";
    }

    @Override
    public void handle( Object aFormComponent ) throws com.sun.star.uno.Exception
    {
        // the name of the child
        XNamed xName = UnoRuntime.queryInterface( XNamed.class, aFormComponent );

        // if it's a form control model, check it's type
        XPropertySet xProps = UNO.queryPropertySet( aFormComponent );
        String sTypeName = FLTools.classifyFormComponentType( xProps );

        String sName;
        if ( null == xName )
            sName = "<unnamed>";
        else
            sName = xName.getName();

        // print the component's name
        if ( 0 != sTypeName.length() )
        {
            System.out.println( m_sPrefix + sName + " (" + sTypeName + ")" );
        }
        else
        {
            System.out.println( m_sPrefix + sName );
        }

        // let the super class step down the tree
        m_sPrefix = m_sPrefix + " ";
        super.handle( aFormComponent );
        m_sPrefix = m_sPrefix.substring( 0, m_sPrefix.length() - 1 );
    }
}

/**************************************************************************/
/** a class revoking button models from a ButtonOperator instance
*/
class RevokeButtons extends ComponentTreeTraversal
{
    private ButtonOperator m_aOperator;

    public RevokeButtons( ButtonOperator aOperator )
    {
        m_aOperator = aOperator;
    }

    @Override
    public void handle( Object aFormComponent ) throws com.sun.star.uno.Exception
    {
        // check if it's a button
        XPropertySet xProps = UNO.queryPropertySet( aFormComponent );
        XPropertySetInfo xPI = null;
        if ( null != xProps )
            xPI = xProps.getPropertySetInfo();
        if ( ( null != xPI ) && xPI.hasPropertyByName( "ClassId" ) )
        {
            Short nClassId = (Short)xProps.getPropertyValue( "ClassId" );
            if ( FormComponentType.COMMANDBUTTON == nClassId.shortValue() )
            {
                // yes, it is
                m_aOperator.revokeButton( xProps );
            }
        }

        // let the super class step down the tree (if possible)
        super.handle( aFormComponent );
    }
}

/**************************************************************************/
public class DataAwareness extends DocumentBasedExample implements XPropertyChangeListener, XResetListener
{
    /* ================================================================== */
    private HsqlDatabase            m_database;

    private static final String     s_tableNameSalesmen = "SALESMEN";
    private static final String     s_tableNameCustomers = "CUSTOMERS";
    private static final String     s_tableNameSales = "SALES";

    private XPropertySet            m_xMasterForm;
    private ButtonOperator          m_aOperator;

    private KeyGenerator            m_aSalesmanKeyGenerator;
    private KeyGenerator            m_aSalesKeyGenerator;
    private ControlLock             m_aSalesmenLocker;
    private ControlLock             m_aSalesLocker;
    private GridFieldValidator      m_aSalesNameValidator;

    private boolean                 m_bDefaultSalesDate;
    private boolean                 m_bProtectKeyFields;
    private boolean                 m_bAllowEmptySales;

    /* ------------------------------------------------------------------ */
    public DataAwareness()
    {
        super( DocumentType.WRITER );
        m_bDefaultSalesDate = false;
        m_bProtectKeyFields = false;
        m_bAllowEmptySales = false;
    }

    /* ==================================================================
       = form components
       ================================================================== */

    /* ------------------------------------------------------------------ */
    /** enumerates and prints all the elements in the given container, together with the container itself
    */
    protected void enumFormComponents( XNameAccess xContainer ) throws java.lang.Exception
    {
        String sObjectName;

        XNamed xNameAcc = UnoRuntime.queryInterface( XNamed.class, xContainer );
        if ( null == xNameAcc )
            sObjectName = "<unnamed>";
        else
            sObjectName = xNameAcc.getName();
        System.out.println( "enumerating the container named \"" + sObjectName +
            "\"\n" );

        PrintComponentTree aPrinter = new PrintComponentTree();
        aPrinter.handle( xContainer );
    }

    /* ------------------------------------------------------------------ */
    /** enumerates and prints all form elements in the document
    */
    protected void enumFormComponents( ) throws java.lang.Exception
    {
        enumFormComponents( m_document.getFormComponentTreeRoot() );
    }

    /* ==================================================================
       = UNO callbacks
       ================================================================== */

    /* ------------------------------------------------------------------ */
    // XResetListener overridables
    /* ------------------------------------------------------------------ */
    public boolean approveReset( EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
        // not interested in vetoing this
        return true;
    }

    /* ------------------------------------------------------------------ */
    public void resetted( EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
        // check if this reset occurred because we're on a new record
        XPropertySet xFormProps = UNO.queryPropertySet(  aEvent.Source );
        try
        {
            Boolean aIsNew = (Boolean)xFormProps.getPropertyValue( "IsNew" );
            if ( aIsNew.booleanValue() )
            {   // yepp

                if ( !m_bDefaultSalesDate )
                {   // we're interested to do all this only if the user told us to default the sales date
                    // to "today"
                    // As date fields do this defaulting automatically, the semantics is inverted here:
                    // If we're told to default, we must do nothing, if we should not default, we must
                    // reset the value which the date field set automatically.

                    Integer aConcurrency = (Integer)xFormProps.getPropertyValue( "ResultSetConcurrency" );
                    if ( ResultSetConcurrency.READ_ONLY != aConcurrency.intValue() )
                    {
                        // we're going to modify the record, though after that, to the user, it should look
                        // like it has not been modified
                        // So we need to ensure that we do not change the IsModified property with whatever we do
                        Object aModifiedFlag = xFormProps.getPropertyValue( "IsModified" );


                        // get the columns of our master form
                        XColumnsSupplier xSuppCols = UnoRuntime.queryInterface(
                            XColumnsSupplier.class, xFormProps );
                        XNameAccess xCols = xSuppCols.getColumns();

                        // and update the date column with a NULL value
                        XColumnUpdate xDateColumn = UnoRuntime.queryInterface(
                            XColumnUpdate.class, xCols.getByName( "SALEDATE" ) );
                        xDateColumn.updateNull();


                        // then restore the flag
                        xFormProps.setPropertyValue( "IsModified", aModifiedFlag );
                    }
                }
            }
        }
        catch( com.sun.star.uno.Exception e )
        {
            System.out.println(e);
            e.printStackTrace();
        }
    }

    /* ------------------------------------------------------------------ */
    // XPropertyChangeListener overridables
    /* ------------------------------------------------------------------ */
    public void propertyChange( PropertyChangeEvent aEvent ) throws com.sun.star.uno.RuntimeException
    {
        try
        {
            // did it come from a radio button or checkbox?
            if ( aEvent.PropertyName.equals( "State" ) )
            {   // yep
                Short aNewState = (Short)aEvent.NewValue;

                XPropertySet xModel = UNO.queryPropertySet( aEvent.Source );
                String sName = (String)xModel.getPropertyValue( "Name" );

                Short aClassId = (Short)xModel.getPropertyValue( "ClassId" );
                if ( FormComponentType.RADIOBUTTON == aClassId.shortValue() )
                {
                    String sRefValue = (String)xModel.getPropertyValue( "RefValue" );

                    short nNewValue = ((Short)aEvent.NewValue).shortValue();
                    if ( sName.equals( "KeyGen" ) )
                    {
                        // it's one of the options for key generation
                        if ( sRefValue.equals( "none" ) )
                        {   // no automatic generation at all
                            m_aSalesmanKeyGenerator.stopGenerator( );
                            m_aSalesKeyGenerator.stopGenerator( );
                        }
                        else
                        {
                            boolean bGenerateOnReset = true;
                            if ( sRefValue.equals( "update" ) )
                            {   // generate on update
                                bGenerateOnReset = ( 0 == nNewValue );
                            }
                            else if ( sRefValue.equals( "reset" ) )
                            {   // generate on reset
                                bGenerateOnReset = ( 0 != nNewValue );
                            }
                            m_aSalesmanKeyGenerator.activateKeyGenerator( bGenerateOnReset );
                            m_aSalesKeyGenerator.activateKeyGenerator( bGenerateOnReset );
                        }
                    }
                }
                else if ( FormComponentType.CHECKBOX == aClassId.shortValue() )
                {
                    boolean bEnabled = ( 0 != aNewState.shortValue() );
                    if ( sName.equals( "defaultdate" ) )
                    {
                        m_bDefaultSalesDate = bEnabled;
                    }
                    else if ( sName.equals( "protectkeys" ) )
                    {
                        m_bProtectKeyFields = bEnabled;
                        m_aSalesmenLocker.enableLock( m_bProtectKeyFields );
                        m_aSalesLocker.enableLock( m_bProtectKeyFields );
                    }
                    else if ( sName.equals( "emptysales" ) )
                    {
                        m_bAllowEmptySales = bEnabled;
                        m_aSalesNameValidator.enableColumnWatch( m_bAllowEmptySales );
                    }
                }
            }
        }
        catch(com.sun.star.uno.Exception e)
        {
            System.out.println(e);
            e.printStackTrace();
        }
    }

    /* ------------------------------------------------------------------ */
    // XEventListener overridables
    /* ------------------------------------------------------------------ */
    @Override
    public void disposing( EventObject aEvent )
    {
        // simply disambiguate
        super.disposing( aEvent );
    }

    /* ==================================================================
       = miscellaneous
       ================================================================== */

    /* ------------------------------------------------------------------ */
    /** skips line feeds in the input stream

        @returns
            the first character which does not belong to a line feed
    */
    protected int skipLineFeeds( java.io.InputStream aInput ) throws java.io.IOException
    {
        // read characters, until we encounter something which is not a line feed character
        int nChar = aInput.read( );
        while ( ( 13 == nChar ) || ( 10 == nChar ) )
            nChar = aInput.read( );

        // now read everything which is behind this single character we are interested in
        while ( 0 < aInput.available() )
            aInput.read( );

        return nChar;
    }

    /* ==================================================================
       = table handling
       ================================================================== */
    /* ------------------------------------------------------------------ */
    /** checks if a given table exists.

        <p>The check is made using a SELECT statement, so even if the connection
        is a n SDB-level connection, which may filter tables in its table
        supplier, the result may be reliable ....</p>
    */
    protected boolean existsInvisibleTable( XConnection xConn, String sTableName ) throws java.lang.Exception
    {
        String sStatement = "SELECT * FROM ";
        sStatement += sTableName;
        sStatement += " WHERE 0=1";

        boolean bSuccess = false;
        try
        {
            XStatement xStatement = xConn.createStatement();
            xStatement.execute( sStatement );
            // if we reached this point, the table probably exists
            bSuccess = true;
        }
        catch(com.sun.star.sdbc.SQLException e)
        {
        }
        return bSuccess;
    }

    /* ------------------------------------------------------------------ */
    /** add a specified table name to the table filter of the given data source.
    */
    protected void makeTableVisible( XDataSource xDS, String sTableName ) throws java.lang.Exception
    {
        // get the table filter
        XPropertySet xDSP = UNO.queryPropertySet( xDS );
        String[] aCurrentFilter = (String[])xDSP.getPropertyValue( "TableFilter" );

        // check if the table name is already part of it
        String sAllTables = "*";                                                // all tables

        for ( int i=0; i<aCurrentFilter.length; ++i )
        {
            String sCurrentTableFilter = aCurrentFilter[i];

            if ( sCurrentTableFilter.equals( sTableName ) )
                return;
            if ( sCurrentTableFilter.equals( sAllTables ) )
                return;
        }

        // if we are here, we have to add our table to the filter sequence
        String[] aNewFilter = new String[ aCurrentFilter.length + 1 ];
        // copy the existent filter entries
        for ( int i=0; i<aCurrentFilter.length; ++i )
            aNewFilter[i] = aCurrentFilter[i];
        // add our table
        aNewFilter[ aCurrentFilter.length ] = sTableName;

        xDSP.setPropertyValue( "TableFilter", aNewFilter );
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
    /** creates the table with the given name, using the given statement
    */
    protected boolean implCreateTable( XConnection xConn, String sCreateStatement, String sTableName ) throws java.lang.Exception
    {
        if ( !implExecuteStatement( xConn, sCreateStatement ) )
        {
            System.out.println( "  could not create the table " + sTableName + "." );
            System.out.println( );
            return false;
        }

        return true;
    }

    /* ------------------------------------------------------------------ */
    /** creates the table SALESMEN

        @return
            <TRUE/> if and only if the creation succeeded
    */
    protected boolean createTableSalesman( XConnection xConn ) throws java.lang.Exception
    {
        String sCreateStatement = "CREATE TABLE " + s_tableNameSalesmen + " ";
        sCreateStatement += "(SNR INTEGER NOT NULL, ";
        sCreateStatement += "FIRSTNAME VARCHAR(50), ";
        sCreateStatement += "LASTNAME VARCHAR(100), ";
        sCreateStatement += "STREET VARCHAR(50), ";
        sCreateStatement += "STATE VARCHAR(50), ";
        sCreateStatement += "ZIP INTEGER, ";
        sCreateStatement += "BIRTHDATE DATE, ";
        sCreateStatement += "PRIMARY KEY(SNR))";

        if ( implCreateTable( xConn, sCreateStatement, s_tableNameSalesmen) )
        {
            String sInsertionPrefix = "INSERT INTO " + s_tableNameSalesmen + " VALUES ";

            implExecuteStatement( xConn, sInsertionPrefix + "(1, 'Joseph', 'Smith', 'Bond Street', 'CA', 95460, '1946-07-02')" );
            implExecuteStatement( xConn, sInsertionPrefix + "(2, 'Frank', 'Jones', 'Lake silver', 'CA', 95460, '1963-12-24')" );
            implExecuteStatement( xConn, sInsertionPrefix + "(3, 'Jane', 'Esperansa', '23 Hollywood driver', 'CA', 95460, '1972-04-01')" );

            return true;
        }
        return false;
    }

    /* ------------------------------------------------------------------ */
    /** creates the table CUSTOMERS

        @return
            <TRUE/> if and only if the creation succeeded
    */
    protected boolean createTableCustomer( XConnection xConn ) throws java.lang.Exception
    {
        String sCreateStatement = "CREATE TABLE " + s_tableNameCustomers + " ";
        sCreateStatement += "(COS_NR INTEGER NOT NULL, ";
        sCreateStatement += "LASTNAME VARCHAR(100), ";
        sCreateStatement += "STREET VARCHAR(50), ";
        sCreateStatement += "CITY VARCHAR(50), ";
        sCreateStatement += "STATE VARCHAR(50), ";
        sCreateStatement += "ZIP INTEGER, ";
        sCreateStatement += "PRIMARY KEY(COS_NR))";

        if ( implCreateTable( xConn, sCreateStatement, s_tableNameCustomers ) )
        {
            String sInsertionPrefix = "INSERT INTO " + s_tableNameCustomers + " VALUES ";

            implExecuteStatement( xConn, sInsertionPrefix + "(100, 'Acme, Inc.', '99 Market Street', 'Groundsville', 'CA', 95199)" );
            implExecuteStatement( xConn, sInsertionPrefix + "(101, 'Superior BugSoft', '1 Party Place', 'Mendocino', 'CA', 95460)");
            implExecuteStatement( xConn, sInsertionPrefix + "(102, 'WeKnowAll, Inc.', '100 Coffee Lane', 'Meadows', 'CA', 93699)");

            return true;
        }
        return false;
    }

    /* ------------------------------------------------------------------ */
    /** creates the table SALES

        @return
            <TRUE/> if and only if the creation succeeded
    */
    protected boolean createTableSales( XConnection xConn ) throws java.lang.Exception
    {
        String sCreateStatement = "CREATE TABLE " + s_tableNameSales + " ";
        sCreateStatement += "(SALENR INTEGER NOT NULL, ";
        sCreateStatement += "COS_NR INTEGER NOT NULL, ";
        sCreateStatement += "SNR INTEGER NOT NULL, ";
        sCreateStatement += "NAME VARCHAR(50), ";
        sCreateStatement += "SALEDATE DATE, ";
        sCreateStatement += "PRICE DECIMAL(8,2), ";
        sCreateStatement += "PRIMARY KEY(SALENR))";

        if ( implCreateTable( xConn, sCreateStatement, s_tableNameSales ) )
        {
            String sInsertionPrefix = "INSERT INTO " + s_tableNameSales + " VALUES ";

            implExecuteStatement( xConn, sInsertionPrefix + "(1, 100, 1, 'Fruits', '2005-02-12', 39.99)" );
            implExecuteStatement( xConn, sInsertionPrefix + "(2, 101, 3, 'Beef', '2005-10-18', 15.78)" );
            implExecuteStatement( xConn, sInsertionPrefix + "(3, 102, 3, 'Orange Juice', '2005-09-08', 25.63)" );
            implExecuteStatement( xConn, sInsertionPrefix + "(4, 101, 2, 'Oil', '2005-03-01', 12.30)" );

            return true;
        }

        return false;
    }

    /* ------------------------------------------------------------------ */
    /** ensures that the tables we need for our example exist
    */
    protected void ensureTables() throws java.lang.Exception
    {
        // get the data source
        XDataSource xDS = m_database.getDataSource();
        XPropertySet xDSProps = UNO.queryPropertySet( xDS );

        // connect to this data source
        XConnection xConn = xDS.getConnection( "", "" );
        XComponent xConnComp = UNO.queryComponent( xConn );

        createTableSalesman( xConn );
        createTableCustomer( xConn );
        createTableSales( xConn );

        // free the resources acquired by the connection
        xConnComp.dispose();
    }

    /* ==================================================================
       = sample document handling
       ================================================================== */

    /* ------------------------------------------------------------------ */
    /** creates the button used for demonstrating (amongst others) event handling
        @param nXPos
            x-position of the to be inserted shape
        @param nYPos
            y-position of the to be inserted shape
        @param nXSize
            width of the to be inserted shape
        @param sName
            the name of the model in the form component hierarchy
        @param sLabel
            the label of the button control
        @param sActionURL
            the URL of the action which should be triggered by the button
        @return
            the model of the newly created button
    */
    protected XPropertySet createButton( int nXPos, int nYPos, int nXSize, String sName, String sLabel, short _formFeature ) throws java.lang.Exception
    {
        XPropertySet xButton = m_formLayer.createControlAndShape( "CommandButton", nXPos, nYPos, nXSize, 6 );
        // the name for referring to it later:
        xButton.setPropertyValue( "Name", sName );
        // the label
        xButton.setPropertyValue( "Label", sLabel );
        // use the name as help text
        xButton.setPropertyValue( "HelpText", sName );
        // don't want buttons to be accessible by the "tab" key - this would be uncomfortable when traveling
        // with records with "tab"
        xButton.setPropertyValue( "Tabstop", Boolean.FALSE );
        // similar, they should not steal the focus when clicked
        xButton.setPropertyValue( "FocusOnClick", Boolean.FALSE );

        m_aOperator.addButton( xButton, _formFeature );

        return xButton;
    }

    /* ------------------------------------------------------------------ */
    /** creates a column in a grid
        @param xGridModel
            specifies the model of the grid where the new column should be inserted
        @param sColumnService
            specifies the service name of the column to create (e.g. "NumericField")
        @param sDataField
            specifies the database field to which the column should be bound
        @param nWidth
            specifies the column width (in mm). If 0, no width is set.
        @return
            the newly created column
    */
    XPropertySet createGridColumn( Object aGridModel, String sColumnService, String sDataField, int nWidth )
        throws com.sun.star.uno.Exception
    {
        // the container to insert columns into
        XIndexContainer xColumnContainer = UNO.queryIndexContainer( aGridModel );
        // the factory for creating column models
        XGridColumnFactory xColumnFactory = UnoRuntime.queryInterface(
            XGridColumnFactory.class, aGridModel );

        // (let) create the new col
        XInterface xNewCol = xColumnFactory.createColumn( sColumnService );
        XPropertySet xColProps = UNO.queryPropertySet( xNewCol );

        // some props
        // the field the column is bound to
        xColProps.setPropertyValue( "DataField", sDataField );
        // the "display name" of the column
        xColProps.setPropertyValue( "Label", sDataField );
        // the name of the column within its parent
        xColProps.setPropertyValue( "Name", sDataField );

        if ( nWidth > 0 )
            xColProps.setPropertyValue( "Width", Integer.valueOf( nWidth * 10 ) );

        // insert
        xColumnContainer.insertByIndex( xColumnContainer.getCount(), xNewCol );

        // outta here
        return xColProps;
    }

    /* ------------------------------------------------------------------ */
    /** creates a column in a grid
    */
    XPropertySet createGridColumn( Object aGridModel, String sColumnService, String sDataField )
        throws com.sun.star.uno.Exception
    {
        return createGridColumn( aGridModel, sColumnService, sDataField );
    }

    /* ------------------------------------------------------------------ */
    /** creates our sample document
    */
    @Override
    protected void prepareDocument() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        super.prepareDocument();

        m_database = new HsqlDatabase( m_xCtx );

        // ensure that we have the tables needed for our example
        ensureTables();


        /* create some shapes */
        XPropertySet xSNRField =    m_formLayer.insertControlLine( "NumericField", "SNR", "", 3 );
                                    m_formLayer.insertControlLine( "TextField", "FIRSTNAME", "", 11);
                                    m_formLayer.insertControlLine( "TextField", "LASTNAME", "", 19 );
                                    m_formLayer.insertControlLine( "TextField", "STREET", "", 27 );
                                    m_formLayer.insertControlLine( "TextField", "STATE", "", 35 );
        XPropertySet xZipField =    m_formLayer.insertControlLine( "NumericField", "ZIP", "", 43 );
                                    m_formLayer.insertControlLine( "FormattedField", "BIRTHDATE", "", 51 );

        // for the salesman number / zip code, we don't want to have decimal places:
        xSNRField.setPropertyValue( "DecimalAccuracy", Short.valueOf( (short)0 ) );
        xZipField.setPropertyValue( "DecimalAccuracy", Short.valueOf( (short)0 ) );


        /** need the form the control models belong to
            for this, we simply obtain the parent for any of the control models we have

            Note that this involves knowledge about the implementation: If a control shape is
            inserted into a document, where the control model does not belong to the form component
            hierarchy, yet, it is automatically inserted into the first form, which is created
            if necessary.
        */
        m_xMasterForm = FLTools.getParent( xZipField );

        // set the data source signature at the form
        m_xMasterForm.setPropertyValue( "DataSourceName", m_database.getDocumentURL() );
        m_xMasterForm.setPropertyValue( "CommandType", Integer.valueOf( CommandType.TABLE ) );
        m_xMasterForm.setPropertyValue( "Command", "SALESMEN" );


        // insert the buttons
        // create our button operator, if necessary
        m_aOperator = new ButtonOperator( m_xCtx, m_document, m_xMasterForm );

        createButton( 2, 63, 8, "first", "<<", FormFeature.MoveToFirst );
        createButton( 12, 63, 8, "prev", "<", FormFeature.MoveToPrevious );
        createButton( 22, 63, 8, "next", ">", FormFeature.MoveToNext );
        createButton( 32, 63, 8, "last", ">>", FormFeature.MoveToLast );
        createButton( 42, 63, 8, "new", ">*", FormFeature.MoveToInsertRow );
        createButton( 58, 63, 13, "reload", "reload", FormFeature.ReloadForm );


        // create a sub form for the sales

        // for this, first create a sub form and bind it to the SALES table
        XIndexContainer xSalesForm = m_document.createSubForm( m_xMasterForm, "Sales" );
        XPropertySet xSalesFormProps = UNO.queryPropertySet( xSalesForm );

        xSalesFormProps.setPropertyValue( "DataSourceName", m_database.getDocumentURL() );
        xSalesFormProps.setPropertyValue( "CommandType", Integer.valueOf( CommandType.COMMAND ) );

        String sCommand = "SELECT * FROM ";
        sCommand += s_tableNameSales;
        sCommand += " WHERE " + s_tableNameSales + ".SNR = :salesmen";
        xSalesFormProps.setPropertyValue( "Command", sCommand );

        // the master-details connection
        String[] aMasterFields = new String[] { "SNR" };        // the field in the master form
        String[] aDetailFields = new String[] { "salesmen" };   // the name in the detail form
        xSalesFormProps.setPropertyValue( "MasterFields", aMasterFields );
        xSalesFormProps.setPropertyValue( "DetailFields", aDetailFields );

        // the create thr grid model
        XPropertySet xSalesGridModel = m_formLayer.createControlAndShape( "GridControl", 2, 80, 162, 40, xSalesForm );
        xSalesGridModel.setPropertyValue( "Name", "SalesTable" );
        XPropertySet xKeyColumn      =  createGridColumn( xSalesGridModel, "NumericField", "SALENR", 12 );
        XPropertySet xCustomerColumn =  createGridColumn( xSalesGridModel, "ListBox", "COS_NR", 40 );
        XPropertySet xSalesNameColumn = createGridColumn( xSalesGridModel, "TextField", "NAME", 25 );
                                        createGridColumn( xSalesGridModel, "DateField", "SALEDATE", 24 );
                                        createGridColumn( xSalesGridModel, "CurrencyField", "PRICE", 16 );

            // please note that a better solution for the SALEDATE field would have been to use
            // a FormattedField. But we want to demonstrate some effects with DateFields here ...

        m_aSalesNameValidator = new GridFieldValidator( m_xCtx, xSalesNameColumn );
        m_aSalesNameValidator.enableColumnWatch( m_bAllowEmptySales );

        xKeyColumn.setPropertyValue( "DecimalAccuracy", Short.valueOf( (short)0 ) );

        // init the list box which is for choosing the customer a sale belongs to
        xCustomerColumn.setPropertyValue( "BoundColumn", Short.valueOf( (short)1 ) );
        xCustomerColumn.setPropertyValue( "Label", "Customer" );
        xCustomerColumn.setPropertyValue( "ListSourceType", ListSourceType.SQL );

        String sListSource = "SELECT LASTNAME, COS_NR FROM ";
        sListSource += s_tableNameCustomers;
        String[] aListSource = new String[] { sListSource };
        xCustomerColumn.setPropertyValue( "ListSource", aListSource );

        // We want to demonstrate how to reset fields to NULL, we do this with the SALEDATE field
        // above. For this, we add as reset listener to the form
        XReset xFormReset = UNO.queryReset( xSalesForm );
        xFormReset.addResetListener( this );



        // the option for filtering the sales form
        XIndexContainer xSalesFilterForm = m_document.createSiblingForm( xSalesForm, "SalesFilter" );
        XPropertySet xSFFProps = UNO.queryPropertySet( xSalesFilterForm );
        XPropertySet xLabel = m_formLayer.createControlAndShape( "FixedText", 2, 125, 35, 6, xSalesFilterForm );
        xLabel.setPropertyValue( "Label", "show only sales since" );
        xLabel.setPropertyValue( "Name", "FilterLabel" );

        XPropertySet xFilterSelection = m_formLayer.createControlAndShape( "ListBox", 40, 125, 59, 6, xSalesFilterForm );
        xFilterSelection.setPropertyValue( "Name", "FilterList" );
        xFilterSelection.setPropertyValue( "LabelControl", xLabel );
        XPropertySet xManualFilter = m_formLayer.createControlAndShape( "DateField", 104, 125, 30, 6, xSalesFilterForm );
        xManualFilter.setPropertyValue( "Name", "ManualFilter" );
        XPropertySet xApplyFilter = m_formLayer.createControlAndShape( "CommandButton", 139, 125, 25, 6, xSalesFilterForm );
        xApplyFilter.setPropertyValue( "Name", "ApplyFilter" );
        xApplyFilter.setPropertyValue( "DefaultButton", Boolean.TRUE );
        new SalesFilter( m_document, xSalesFormProps, xFilterSelection,
            xManualFilter, xApplyFilter );



        // the options section
        // for this, we need a form which is a sibling of our master form (don't want to interfere
        // the controls which represent options only with the controls which are used for data access)

        XIndexContainer xOptionsForm = m_document.createSiblingForm( m_xMasterForm, "Options" );

        xLabel = m_formLayer.createControlAndShape( "GroupBox", 98, 0, 66, 62, xOptionsForm );
        xLabel.setPropertyValue( "Name", "Options" );
        xLabel.setPropertyValue( "Label", "Options" );

        // radio buttons which controls how we generate unique keys
        xLabel = m_formLayer.createControlAndShape( "GroupBox", 103, 5, 56, 25, xOptionsForm );
        xLabel.setPropertyValue( "Label", "key generation" );
        xLabel.setPropertyValue( "Name", "KeyGeneration" );
        XPropertySet xKeyGen = m_formLayer.createControlAndShape( "RadioButton", 106, 11, 50, 6, xOptionsForm );
        xKeyGen.setPropertyValue( "Name", "KeyGen" );
        xKeyGen.setPropertyValue( "Label", "no automatic generation" );
        xKeyGen.setPropertyValue( "RefValue", "none" );
        xKeyGen.addPropertyChangeListener( "State", this );

        xKeyGen = m_formLayer.createControlAndShape( "RadioButton", 106, 17, 50, 6, xOptionsForm );
        xKeyGen.setPropertyValue( "Name", "KeyGen" );
        xKeyGen.setPropertyValue( "Label", "before inserting a record" );
        xKeyGen.setPropertyValue( "RefValue", "update" );
        xKeyGen.addPropertyChangeListener( "State", this );

        xKeyGen = m_formLayer.createControlAndShape( "RadioButton", 106, 23, 50, 6, xOptionsForm );
        xKeyGen.setPropertyValue( "Name", "KeyGen" );
        xKeyGen.setPropertyValue( "Label", "when moving to a new record" );
        xKeyGen.setPropertyValue( "RefValue", "reset" );
        xKeyGen.addPropertyChangeListener( "State", this );

        // initialize listeners
        // master form - key generation
        m_aSalesmanKeyGenerator = new KeyGenerator( m_xMasterForm, "SNR", m_xCtx );
        m_aSalesmanKeyGenerator.activateKeyGenerator( true );
        // master form - control locking
        m_aSalesmenLocker = new ControlLock( m_xMasterForm, "SNR" );
        m_aSalesmenLocker.enableLock( m_bProtectKeyFields );

        // details form - key generation
        m_aSalesKeyGenerator = new KeyGenerator( xSalesFormProps, "SALENR", m_xCtx );
        m_aSalesKeyGenerator.activateKeyGenerator( true );

        // details form - control locking
        m_aSalesLocker = new ControlLock( xSalesFormProps, "SALENR" );
        m_aSalesLocker.enableLock( m_bProtectKeyFields );

        // initially, we want to generate keys when moving to a new record
        xKeyGen.setPropertyValue( "DefaultState", Short.valueOf( (short)1 ) );


        // second options block
        xLabel = m_formLayer.createControlAndShape( "GroupBox", 103, 33, 56, 25, xOptionsForm  );
        xLabel.setPropertyValue( "Name", "Misc" );
        xLabel.setPropertyValue( "Label", "Miscellaneous" );

        XPropertySet xCheck = m_formLayer.createControlAndShape( "CheckBox", 106, 39, 60, 6, xOptionsForm  );
        xCheck.setPropertyValue( "Name", "defaultdate" );
        xCheck.setPropertyValue( "Label", "default sales date to \"today\"" );
        xCheck.setPropertyValue( "HelpText", "When checked, newly entered sales records are pre-filled with today's date, else left empty." );
        xCheck.addPropertyChangeListener( "State", this );

        xCheck = m_formLayer.createControlAndShape( "CheckBox", 106, 45, 60, 6, xOptionsForm  );
        xCheck.setPropertyValue( "Name", "protectkeys" );
        xCheck.setPropertyValue( "Label", "protect key fields from editing" );
        xCheck.setPropertyValue( "HelpText", "When checked, you cannot modify the values in the table's key fields (SNR and SALENR)" );
        xCheck.addPropertyChangeListener( "State", this );

        xCheck = m_formLayer.createControlAndShape( "CheckBox", 106, 51, 60, 6, xOptionsForm  );
        xCheck.setPropertyValue( "Name", "emptysales" );
        xCheck.setPropertyValue( "Label", "check for empty sales names" );
        xCheck.setPropertyValue( "HelpText", "When checked, you cannot enter empty values into the NAME column of the 'Sales' table." );
        xCheck.addPropertyChangeListener( "State", this );

        // dump the form component tree
        enumFormComponents( );
    }

    /* ------------------------------------------------------------------ */
    @Override
    protected void onFormsAlive()
    {
        m_aOperator.onFormsAlive();
    }

    /* ------------------------------------------------------------------ */
    /** performs any cleanup before exiting the program
    */
    @Override
    protected void cleanUp( ) throws java.lang.Exception
    {
        // remove the listeners at the buttons
        RevokeButtons aRevoke = new RevokeButtons( m_aOperator );
        aRevoke.handle( m_document.getFormComponentTreeRoot( ) );

        // remove the key generator listeners from the form
        m_aSalesmanKeyGenerator.stopGenerator( );
        m_aSalesKeyGenerator.stopGenerator( );

        // and the control lockers
        m_aSalesmenLocker.enableLock( false );
        m_aSalesLocker.enableLock( false );

        // the validator for the grid column
        m_aSalesNameValidator.enableColumnWatch( false );

        // remove our own reset listener from the form
        XNameAccess xMasterAsNames = UnoRuntime.queryInterface(
            XNameAccess.class, m_xMasterForm );
        XReset xFormReset = UNO.queryReset( xMasterAsNames.getByName( "Sales" ) );
        xFormReset.removeResetListener( this );

        super.cleanUp();
    }

    /* ------------------------------------------------------------------ */
    /** class entry point
    */
    public static void main(String argv[]) throws java.lang.Exception
    {
        DataAwareness aSample = new DataAwareness();
        aSample.run( argv );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
