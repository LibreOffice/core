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
    private DocumentHelper          m_aDocument;
    private XMultiServiceFactory    m_xMSF;
    private XPropertySet            m_xWatchedColumn;

    private boolean         m_bWatching;

    /* ------------------------------------------------------------------ */
    public GridFieldValidator( XMultiServiceFactory xMSF, XPropertySet xWatchedGridColumn )
    {
        // remember
        m_xMSF = xMSF;
        m_xWatchedColumn = xWatchedGridColumn;
        m_aDocument = DocumentHelper.getDocumentForComponent( xWatchedGridColumn, xMSF );

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
                XInteractionHandler.class, m_xMSF.createInstance( "com.sun.star.sdb.InteractionHandler" ) );

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
