import com.sun.star.uno.*;
import com.sun.star.frame.*;
import com.sun.star.awt.*;
import com.sun.star.lang.*;
import com.sun.star.util.*;
import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.task.*;
import com.sun.star.sdbc.*;
import com.sun.star.sdbcx.*;
import com.sun.star.sdb.*;
import com.sun.star.form.*;

// java base stuff
import java.util.Vector;


/**************************************************************************/
/** a helper class for operating the buttons
*/
public class ButtonOperator implements XActionListener, XStatusListener
{
    private XMultiServiceFactory    m_xMSF;
    private DocumentHelper          m_aDocument;

    private Vector                  m_aButtons;
    private Vector                  m_aDispatchers;

    /* ------------------------------------------------------------------ */
    /** ctor
    */
    public ButtonOperator( XMultiServiceFactory aMSF, DocumentHelper aDocument )
    {
        m_xMSF = aMSF;
        m_aDocument = aDocument;
        m_aButtons = new Vector();
        m_aDispatchers = new Vector();
    }

    /* ------------------------------------------------------------------ */
    /** shows a message that we can't do several things due to an implementation error
    */
    private void showImplementationErrorMessage( XInterface xContext )
    {
        SQLException aBaseError = new SQLException(
            new String( "Due to a multi-threading issue, this method does not work correctly when invoked via remote java." ),
            xContext,
            new String( "S1000" ),
            0,
            null
        );
        SQLContext aError = new SQLContext(
            new String( "Unable to perform request." ),
            xContext,
            new String( "S1000" ),
            0,
            aBaseError,
            new String( "This functionallity has been disabled due to an implementation bug." )
        );

        try
        {
            // instantiate an interaction handler who can handle SQLExceptions
            XInteractionHandler xHandler = (XInteractionHandler)UnoRuntime.queryInterface(
                XInteractionHandler.class, m_xMSF.createInstance( "com.sun.star.sdb.InteractionHandler" ) );

            // create a new request
            InteractionRequest aRequest = new InteractionRequest( aError );
            xHandler.handle( aRequest );
        }
        catch( com.sun.star.uno.Exception e )
        {
        }
    }

    /* ------------------------------------------------------------------ */
    /** reloads the form the given button belongs too
    */
    private void reload( Object aControlModel )
    {
        // this came from the reload button, so reload the form if the user wishes this ....
        com.sun.star.form.XLoadable xLoad = (com.sun.star.form.XLoadable)FLTools.getParent(
            aControlModel, com.sun.star.form.XLoadable.class );
        // (note that this xLoad equals our m_xMasterForm)

        // produce an error saying that we can't really do this
//      showImplementationErrorMessage( xLoad );

        // If you did neither your office nor your jave program fit with the
        // ForceSynchronous=1 parameter, the following line would result in a deadlock due
        // to an implementation bug.
        xLoad.reload();
    }

    /* ------------------------------------------------------------------ */
    private String getTag( Object aModel )
    {
        String sReturn = new String();
        try
        {
            XPropertySet xModelProps = UNO.queryPropertySet( aModel );
            sReturn = (String)xModelProps.getPropertyValue( "Tag" );
        }
        catch( com.sun.star.uno.Exception e )
        {
            // though this is a serious error, we're not interested in
        }
        return sReturn;
    }

    /* ------------------------------------------------------------------ */
    /** get's the button which we operate and which is responsible for a given URL
    */
    private int getButton( String sActionURL )
    {
        int nPos = -1;
        for ( int i=0; ( i < m_aButtons.size() ) && ( -1 == nPos ); ++i )
        {
            if ( sActionURL.equals( getTag( m_aButtons.elementAt( i ) ) ) )
                nPos = i;
        }
        return nPos;
    }

    /* ------------------------------------------------------------------ */
    /** announces a button which the operator should be responsible for
    */
    private int getButtonIndex( XPropertySet xButton )
    {
        int nPos = -1;
        for ( int i=0; ( i < m_aButtons.size() ) && ( -1 == nPos ); ++i )
        {
            if ( xButton.equals( m_aButtons.elementAt( i ) ) )
                nPos = i;
        }
        return nPos;
    }

    /* ------------------------------------------------------------------ */
    /** announces a button which the operator should be responsible for
    */
    public void addButton( XPropertySet xButtonModel, String sActionURL ) throws java.lang.Exception
    {
        // the current view to the document
        DocumentViewHelper aCurrentView = m_aDocument.getCurrentView();

        // add a listener so we get noticed if the user presses the button
        // get the control
        XButton xButtonControl = (XButton)UnoRuntime.queryInterface( XButton.class,
            aCurrentView.getControl( xButtonModel ) );

        xButtonControl.addActionListener( this );

        // remember the action URL
        xButtonModel.setPropertyValue( "Tag", sActionURL );
        // retrieve the dispatcher for the action URL
        XDispatch xActionDispatch = null;
        if ( 0 < sActionURL.length() )
        {
            // query the current document view for a dispatcher for the action URL
            URL[] aURL = new URL[] { new URL() };
            aURL[0].Complete = sActionURL;
            xActionDispatch = aCurrentView.getDispatcher( aURL );
            // and if we found one, add ourself as status listener so we get notified whenever something changes
            if ( null != xActionDispatch )
            {
                xActionDispatch.addStatusListener( this, aURL[0] );
            }
        }

        // remember the button and the dispatcher
        m_aButtons.add( xButtonModel );
        m_aDispatchers.add( xActionDispatch );
    }

    /* ------------------------------------------------------------------ */
    public void revokeButton( XPropertySet xButtonModel )
    {
        int nPos = getButtonIndex( xButtonModel );
        if ( -1 < nPos )
        {
            m_aButtons.remove( nPos );
            m_aDispatchers.remove( nPos );
        }
    }

    /* ------------------------------------------------------------------ */
    /** called when the status of an URL we operate on has changed
    */
    public void statusChanged( FeatureStateEvent aEvent ) throws com.sun.star.uno.RuntimeException
    {
        // get the button which is responsible for URL
        int nButtonPos = getButton( aEvent.FeatureURL.Complete );
        if ( -1 < nButtonPos )
        {
            XPropertySet xButton = (XPropertySet)m_aButtons.elementAt( nButtonPos );
            try
            {
                xButton.setPropertyValue( "Enabled", new Boolean( aEvent.IsEnabled ) );
            }
            catch( java.lang.Exception e )
            {
                System.out.println(e);
                e.printStackTrace();
            }
        }
    }

    /* ==================================================================
       = XActionListener
       ================================================================== */
    /* ------------------------------------------------------------------ */
    /* called when a button has been pressed
    */
    public void actionPerformed( ActionEvent aEvent ) throws com.sun.star.uno.RuntimeException
    {
        // get the model's name
        XNamed xModel = (XNamed)FLTools.getModel( aEvent.Source, XNamed.class );
        String sName = xModel.getName();

        if ( sName.equals( new String( "reload" ) ) )
            reload( xModel );
        else
        {
            // get the action URL the button is bound to
            String sActionURL = getTag( xModel );

            // get the dispatcher responsible for this action URL
            int nButtonPos = getButton( sActionURL );
            if ( -1 < nButtonPos )
            {
                XDispatch xDispatcher = (XDispatch)m_aDispatchers.elementAt( nButtonPos );
                if ( null != xDispatcher )
                {
                    PropertyValue[] aDummyArgs = new PropertyValue[] { };
                    try
                    {
                        xDispatcher.dispatch( FLTools.parseURL( sActionURL, m_xMSF ), aDummyArgs );
                    }
                    catch( java.lang.Exception e )
                    {
                    }
                }
            }

            // below is what we really would like to do - if we would not have these implementation
            // bugs
            // Though the current solution has one more advantage: we don't need to determine
            // the button state ourself. We are told by the form layer framework when the buttons
            // have to be enabled or disabled.
            // Once Issuezilla bug #TODO# is fixed (means we have a chance to reach for the form
            // controller from external components), we can get rid off the implementation specific
            // part of the URLs we use (this "#0/0" mark), and then we would have a working solution
            // which is not to be called hack .....

            // the result set to operate on
//          XResultSet xSet = (XResultSet)FLTools.getParent( xModel, XResultSet.class );
//
//          try
//          {
//              // this is what we would have liked to do
//              if ( sName.equals( new String( "first" ) ) )
//                  xSet.first();
//              else if ( sName.equals( new String( "prev" ) ) )
//                  xSet.previous();
//              else if ( sName.equals( new String( "next" ) ) )
//                  xSet.next();
//              else if ( sName.equals( new String( "last" ) ) )
//                  xSet.last();
//          }
//          catch( SQLException e )
//          {
//              System.err.println( e );
//              e.printStackTrace();
//          }
        }
    }

    /* ==================================================================
       = XEventListener
       ================================================================== */
    public void disposing( EventObject aEvent )
    {
        // not interested in
    }
};

