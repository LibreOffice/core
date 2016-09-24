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

import com.sun.star.uno.*;
import com.sun.star.beans.*;
import com.sun.star.form.*;
import com.sun.star.lang.*;
import com.sun.star.sdb.*;
import com.sun.star.sdbc.*;
import com.sun.star.sdbcx.*;
import com.sun.star.container.*;
import com.sun.star.awt.*;

/**************************************************************************/
/** base class for helpers dealing with unique column values
*/
class UniqueColumnValue
{
    /* ------------------------------------------------------------------ */
    /** extracts the name of the table a form is based on.

        <p>This method works for forms based directly on tables, and for forms based on statements, which
        themself are based on one table.<br>
        Everything else (especially forms based on queries) is not yet implemented.</p>
    */
    private String extractTableName( XPropertySet xForm ) throws com.sun.star.uno.Exception
    {
        String sReturn;

        Integer aCommandType = (Integer)xForm.getPropertyValue( "CommandType" );
        String sCommand = (String)xForm.getPropertyValue( "Command" );

        if ( CommandType.COMMAND == aCommandType.intValue() )
        {
            // get the connection from the form
            XConnection xFormConn = UnoRuntime.queryInterface( XConnection.class,
                xForm.getPropertyValue( "ActiveConnection" ) );
            // and let it create a composer for us
            XSQLQueryComposerFactory xComposerFac =
                UnoRuntime.queryInterface(
                XSQLQueryComposerFactory.class, xFormConn );
            XSQLQueryComposer xComposer = xComposerFac.createQueryComposer( );

            // let this composer analyze the command
            xComposer.setQuery( sCommand );

            // and ask it for the table(s)
            XTablesSupplier xSuppTables = UnoRuntime.queryInterface(
                XTablesSupplier.class, xComposer );
            XNameAccess xTables = xSuppTables.getTables();

            // simply take the first table name
            String[] aNames = xTables.getElementNames( );
            sCommand = aNames[0];
        }

        return sCommand;
    }

    /* ------------------------------------------------------------------ */
    /** generates a statement which can be used to create a unique (in all conscience) value
        for the column given.
        <p>Currently, the implementation uses a very simple approach - it just determines the maximum of currently
        existing values in the column. If your concrete data source supports a more sophisticated approach of generating
        unique values, you probably want to adjust the <code>SELECT</code> statement below accordingly.</p>

        @returns
            a String which can be used as statement to retrieve a unique value for the given column.
            The result set resulting from such a execution contains the value in its first column.
    */
    private String composeUniqueyKeyStatement( XPropertySet xForm, String sFieldName ) throws com.sun.star.uno.Exception
    {
        String sStatement = "SELECT MAX( ";
        sStatement += sFieldName;
        sStatement += ") + 1 FROM ";
        // the table name is a property of the form
        sStatement += extractTableName( xForm );

        // note that the implementation is imperfect (besides the problem that MAX is not a really good solution
        // for a database with more that one client):
        // It does not quote the field and the table name. This needs to be done if the database is intolerant
        // against such things - the XDatabaseMetaData, obtained from the connection, would be needed then
        // Unfortunately, there is no UNO service doing this - it would need to be implemented manually.

        return sStatement;
    }

    /* ------------------------------------------------------------------ */
    /** generates a unique (in all conscience) key into the column given
        @param xForm
            the form which contains the column in question
        @param sFieldName
            the name of the column
    */
    private int generatePrimaryKey( XPropertySet xForm, String sFieldName ) throws com.sun.star.uno.Exception
    {
        // get the current connection of the form
        XConnection xConn = UnoRuntime.queryInterface(
            XConnection.class, xForm.getPropertyValue( "ActiveConnection" ) );
        // let it create a new statement
        XStatement xStatement = xConn.createStatement();

        // build the query string to determine a free value
        String sStatement = composeUniqueyKeyStatement( xForm, sFieldName );

        // execute the query
        XResultSet xResults = xStatement.executeQuery( sStatement );

        // move the result set to the first record
        xResults.next( );

        // get the value
        XRow xRow = UnoRuntime.queryInterface( XRow.class, xResults );
        int nFreeValue = xRow.getInt( 1 );

        // dispose the temporary objects
        FLTools.disposeComponent( xStatement );
            // this should get rid of the result set, too

        return nFreeValue;
    }

    /* ------------------------------------------------------------------ */
    /** inserts a unique (in all conscience) key into the column given
        @param xForm
            the form which contains the column in question
        @param sFieldName
            the name of the column
    */
    public void insertPrimaryKey( XPropertySet xForm, String sFieldName ) throws com.sun.star.uno.Exception
    {
        // check the privileges
        Integer aConcurrency = (Integer)xForm.getPropertyValue( "ResultSetConcurrency" );
        if ( ResultSetConcurrency.READ_ONLY != aConcurrency.intValue() )
        {
            // get the column object
            XColumnsSupplier xSuppCols = UnoRuntime.queryInterface(
                XColumnsSupplier.class, xForm );
            XNameAccess xCols = xSuppCols.getColumns();
            XColumnUpdate xCol = UnoRuntime.queryInterface(
                XColumnUpdate.class, xCols.getByName( sFieldName ) );

            xCol.updateInt( generatePrimaryKey( xForm, sFieldName ) );
        }
    }
}

/**************************************************************************/
/** base class for helpers dealing with unique column values
*/
class KeyGeneratorForReset extends UniqueColumnValue implements XResetListener
{
    /* ------------------------------------------------------------------ */
    private DocumentViewHelper  m_aView;
    private String              m_sFieldName;

    /* ------------------------------------------------------------------ */
    /** ctor
        @param aView
            the view which shall be used to focus controls
        @param sFieldName
            the name of the field for which keys should be generated
    */
    public KeyGeneratorForReset( String sFieldName, DocumentViewHelper aView )
    {
        m_sFieldName = sFieldName;
        m_aView = aView;
    }

    /* ------------------------------------------------------------------ */
    /** sets the focus to the first control which is no fixed text, and not the
        one we're defaulting
    */
    private void defaultNewRecordFocus( XPropertySet xForm ) throws com.sun.star.uno.Exception
    {
        XIndexAccess xFormAsContainer = UnoRuntime.queryInterface(
            XIndexAccess.class, xForm );
        for ( int i = 0; i<xFormAsContainer.getCount(); ++i )
        {
            // the model
            XPropertySet xModel = UNO.queryPropertySet( xFormAsContainer.getByIndex( i ) );

            // check if it's a valid leaf (no sub form or such)
            XPropertySetInfo xPSI = xModel.getPropertySetInfo( );
            if ( ( null == xPSI ) || !xPSI.hasPropertyByName( "ClassId" ) )
                continue;

            // check if it's a fixed text
            Short nClassId = (Short)xModel.getPropertyValue( "ClassId" );
            if ( FormComponentType.FIXEDTEXT == nClassId.shortValue() )
                continue;

            // check if it is bound to the field we are responsible for
            if ( !xPSI.hasPropertyByName( "DataField" ) )
                continue;

            String sFieldDataSource = (String)xModel.getPropertyValue( "DataField" );
            if ( sFieldDataSource.equals( m_sFieldName ) )
                continue;

            // both conditions do not apply
            // -> set the focus into the respective control
            XControlModel xCM = UNO.queryControlModel( xModel );
            m_aView.grabControlFocus( xCM);
            break;
        }
    }

    /* ------------------------------------------------------------------ */
    // XResetListener overridables
    /* ------------------------------------------------------------------ */
    public boolean approveReset( com.sun.star.lang.EventObject rEvent ) throws com.sun.star.uno.RuntimeException
    {
        // not interested in vetoing this
        return true;
    }

    /* ------------------------------------------------------------------ */
    public void resetted( com.sun.star.lang.EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
        // check if this reset occurred because we're on a new record
        XPropertySet xFormProps = UNO.queryPropertySet( aEvent.Source );
        try
        {
            Boolean aIsNew = (Boolean)xFormProps.getPropertyValue( "IsNew" );
            if ( aIsNew.booleanValue() )
            {   // yepp

                // we're going to modify the record, though after that, to the user, it should look
                // like it has not been modified
                // So we need to ensure that we do not change the IsModified property with whatever we do
                Object aModifiedFlag = xFormProps.getPropertyValue( "IsModified" );

                // now set the value
                insertPrimaryKey( xFormProps, m_sFieldName );

                // then restore the flag
                xFormProps.setPropertyValue( "IsModified", aModifiedFlag );

                // still one thing ... would be nice to have the focus in a control which is
                // the one which's value we just defaulted
                defaultNewRecordFocus( xFormProps );
            }
        }
        catch( com.sun.star.uno.Exception e )
        {
            System.out.println(e);
            e.printStackTrace();
        }
    }
    /* ------------------------------------------------------------------ */
    // XEventListener overridables
    /* ------------------------------------------------------------------ */
    public void disposing( EventObject aEvent )
    {
        // not interested in
    }
}


/**************************************************************************/
/** base class for helpers dealing with unique column values
*/
class KeyGeneratorForUpdate extends UniqueColumnValue implements XRowSetApproveListener
{
    /* ------------------------------------------------------------------ */
    private String  m_sFieldName;

    /* ------------------------------------------------------------------ */
    public KeyGeneratorForUpdate( String sFieldName )
    {
        m_sFieldName = sFieldName;
    }

    /* ------------------------------------------------------------------ */
    // XRowSetApproveListener overridables
    /* ------------------------------------------------------------------ */
    public boolean approveCursorMove( com.sun.star.lang.EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
        // not interested in vetoing moves
        return true;
    }

    /* ------------------------------------------------------------------ */
    public boolean approveRowChange( RowChangeEvent aEvent ) throws com.sun.star.uno.RuntimeException
    {
        if ( RowChangeAction.INSERT == aEvent.Action )
        {
            try
            {
                // the affected form
                XPropertySet xFormProps = UNO.queryPropertySet( aEvent.Source );
                // insert a new unique value
                insertPrimaryKey( xFormProps, m_sFieldName );
            }
            catch( com.sun.star.uno.Exception e )
            {
                System.out.println(e);
                e.printStackTrace();
            }
        }
        return true;
    }

    /* ------------------------------------------------------------------ */
    public boolean approveRowSetChange( com.sun.star.lang.EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
        // not interested in vetoing executions of the row set
        return true;
    }
    /* ------------------------------------------------------------------ */
    // XEventListener overridables
    /* ------------------------------------------------------------------ */
    public void disposing( EventObject aEvent )
    {
        // not interested in
    }
}

/**************************************************************************/
/** allows to generate unique keys for a field of a Form
*/
public class KeyGenerator
{
    /* ------------------------------------------------------------------ */
    private KeyGeneratorForReset    m_aResetKeyGenerator;
    private KeyGeneratorForUpdate   m_aUpdateKeyGenerator;
    private boolean                 m_bResetListening;
    private boolean                 m_bUpdateListening;

    private XPropertySet            m_xForm;

    /* ------------------------------------------------------------------ */
    /** ctor
        @param xForm
            specified the form to operate on
        @param sFieldName
            specifies the field which's value should be manipulated
    */
    public KeyGenerator( XPropertySet xForm, String sFieldName,
                         XComponentContext xCtx )
    {
        m_xForm = xForm;

        DocumentHelper aDocument = DocumentHelper.getDocumentForComponent( xForm, xCtx );

        m_aResetKeyGenerator = new KeyGeneratorForReset( sFieldName, aDocument.getCurrentView() );
        m_aUpdateKeyGenerator = new KeyGeneratorForUpdate( sFieldName );

        m_bResetListening = m_bUpdateListening = false;
    }

    /* ------------------------------------------------------------------ */
    /** stops any actions on the form
    */
    public void stopGenerator( )
    {
        XReset xFormReset = UNO.queryReset( m_xForm );
        xFormReset.removeResetListener( m_aResetKeyGenerator );

        XRowSetApproveBroadcaster xFormBroadcaster = UnoRuntime.queryInterface(
            XRowSetApproveBroadcaster.class, m_xForm );
        xFormBroadcaster.removeRowSetApproveListener( m_aUpdateKeyGenerator );

        m_bUpdateListening = m_bResetListening = false;
    }

    /* ------------------------------------------------------------------ */
    /** activates one of our two key generators
    */
    public void activateKeyGenerator( boolean bGenerateOnReset )
    {
        // for resets
        XReset xFormReset = UNO.queryReset( m_xForm );
        // for approving actions
        XRowSetApproveBroadcaster xFormBroadcaster = UnoRuntime.queryInterface(
            XRowSetApproveBroadcaster.class, m_xForm );

        if ( bGenerateOnReset )
        {
            if ( !m_bResetListening )
                xFormReset.addResetListener( m_aResetKeyGenerator );
            if ( m_bUpdateListening )
                xFormBroadcaster.removeRowSetApproveListener( m_aUpdateKeyGenerator );

            m_bUpdateListening = false;
            m_bResetListening = true;
        }
        else
        {
            if ( m_bResetListening )
                xFormReset.removeResetListener( m_aResetKeyGenerator );
            if ( !m_bUpdateListening )
                xFormBroadcaster.addRowSetApproveListener( m_aUpdateKeyGenerator );

            m_bResetListening = false;
            m_bUpdateListening = true;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
