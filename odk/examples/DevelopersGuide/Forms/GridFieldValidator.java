/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



/**************************************************************************/
import com.sun.star.uno.*;
import com.sun.star.beans.*;
import com.sun.star.form.*;
import com.sun.star.lang.*;
import com.sun.star.sdb.*;
import com.sun.star.sdbc.*;
import com.sun.star.sdbcx.*;
import com.sun.star.container.*;
import com.sun.star.awt.*;
import com.sun.star.task.*;

/**************************************************************************/
/** helper class for validating a grid field before it is updated

    <p>Actually, the mechanism for validating the field is not restricted to
    grid control fields. Instead, it can be used for any bound controls.</p>
*/
class GridFieldValidator implements XUpdateListener
{
    private DocumentHelper      m_aDocument;
    private XComponentContext   m_xCtx;
    private XPropertySet        m_xWatchedColumn;

    private boolean         m_bWatching;

    /* ------------------------------------------------------------------ */
    public GridFieldValidator( XComponentContext xCtx, XPropertySet xWatchedGridColumn )
    {
        // remember
        m_xCtx = xCtx;
        m_xWatchedColumn = xWatchedGridColumn;
        m_aDocument = DocumentHelper.getDocumentForComponent(xWatchedGridColumn,
                                                             xCtx);

        m_bWatching = false;
    }

    /* ------------------------------------------------------------------ */
    public void enableColumnWatch( boolean bEnable )
    {
        if ( bEnable == m_bWatching )
            return;

        XUpdateBroadcaster xUpdate = (XUpdateBroadcaster)UnoRuntime.queryInterface(
            XUpdateBroadcaster.class, m_xWatchedColumn );

        if ( bEnable )
            xUpdate.addUpdateListener( this );
        else
            xUpdate.removeUpdateListener( this );

        m_bWatching = bEnable;
    }

    /* ------------------------------------------------------------------ */
    /** shows a message that we can't do several things due to an implementation error
    */
    private void showInvalidValueMessage( )
    {
        try
        {
            // build the message we want to show
            String sMessage = "The column \"";
            sMessage += FLTools.getLabel( m_xWatchedColumn );
            sMessage += "\" is not allowed to contain empty strings.";

            SQLContext aError = new SQLContext(
                new String( "Invalid Value Entered" ),
                null,
                new String( "S1000" ),
                0,
                new Any( new Type(), null ),
                sMessage
            );

            // instantiate an interaction handler who can handle SQLExceptions
            XInteractionHandler xHandler = (XInteractionHandler)UnoRuntime.queryInterface(
                XInteractionHandler.class,
                m_xCtx.getServiceManager().createInstanceWithContext(
                    "com.sun.star.task.InteractionHandler", m_xCtx ) );

            // create a new request and execute it
            InteractionRequest aRequest = new InteractionRequest( aError );
            xHandler.handle( aRequest );
        }
        catch( com.sun.star.uno.Exception e )
        {
            System.out.println(e);
            e.printStackTrace();
        }
    }

    /* ------------------------------------------------------------------ */
    // XUpdateListener overridables
    /* ------------------------------------------------------------------ */
    public boolean approveUpdate( EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
        boolean bApproved = true;
        try
        {
            // the control model which fired the event
            XPropertySet xSourceProps = UNO.queryPropertySet( aEvent.Source );

            String sNewText = (String)xSourceProps.getPropertyValue( "Text" );
            if ( 0 == sNewText.length() )
            {
                // say that the value is invalid
                showInvalidValueMessage( );
                bApproved = false;

                // reset the control value
                // for this, we take the current value from the row set field the control
                // is bound to, and forward it to the control model
                XColumn xBoundColumn = UNO.queryColumn( xSourceProps.getPropertyValue( "BoundField" ) );
                if ( null != xBoundColumn )
                {
                    xSourceProps.setPropertyValue( "Text", xBoundColumn.getString() );
                }
            }
        }
        catch( com.sun.star.uno.Exception e )
        {
            System.out.println(e);
            e.printStackTrace();
        }
        return bApproved;
    }

    /* ------------------------------------------------------------------ */
    public void updated( EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
    }

    /* ------------------------------------------------------------------ */
    // XEventListener overridables
    /* ------------------------------------------------------------------ */
    public void disposing( EventObject aEvent )
    {
        // not interested in
    }
};
