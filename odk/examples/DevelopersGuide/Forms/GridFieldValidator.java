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

/**************************************************************************/
import com.sun.star.uno.*;
import com.sun.star.beans.*;
import com.sun.star.form.*;
import com.sun.star.lang.*;
import com.sun.star.sdb.*;
import com.sun.star.task.*;

/**************************************************************************/
/** helper class for validating a grid field before it is updated

    <p>Actually, the mechanism for validating the field is not restricted to
    grid control fields. Instead, it can be used for any bound controls.</p>
*/
class GridFieldValidator implements XUpdateListener
{
    private XComponentContext   m_xCtx;
    private XPropertySet        m_xWatchedColumn;

    private boolean         m_bWatching;

    /* ------------------------------------------------------------------ */
    public GridFieldValidator( XComponentContext xCtx, XPropertySet xWatchedGridColumn )
    {
        // remember
        m_xCtx = xCtx;
        m_xWatchedColumn = xWatchedGridColumn;
        DocumentHelper.getDocumentForComponent(xWatchedGridColumn,
                                                             xCtx);

        m_bWatching = false;
    }

    /* ------------------------------------------------------------------ */
    public void enableColumnWatch( boolean bEnable )
    {
        if ( bEnable == m_bWatching )
            return;

        XUpdateBroadcaster xUpdate = UnoRuntime.queryInterface(
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
                "Invalid Value Entered",
                null,
                "S1000",
                0,
                new Any( new Type(), null ),
                sMessage
            );

            // instantiate an interaction handler who can handle SQLExceptions
            XInteractionHandler xHandler = UnoRuntime.queryInterface(
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
