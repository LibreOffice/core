import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.form.*;
import com.sun.star.util.*;
import com.sun.star.sdbc.*;


/**************************************************************************/
/** A helper class for recursively locking control models which are bound
    to a specific field
*/

class LockControlModels extends ComponentTreeTraversal
{
    private String  m_sDataField;
    private Boolean m_aLockIt;
    private int     m_nLevel;   // nesting level relative to the form we started with

    /* ------------------------------------------------------------------ */
    public LockControlModels( String sDataField, boolean bLockIt )
    {
        m_sDataField = sDataField;
        m_aLockIt = new Boolean( bLockIt );
        m_nLevel = 0;
    }

    /* ------------------------------------------------------------------ */
    protected boolean shouldStepInto( XIndexContainer xContainer ) throws com.sun.star.uno.Exception
    {
        if ( !super.shouldStepInto( xContainer ) )
            return false;   // don't try to be more clever than our base class

        XForm xForm = (XForm)UnoRuntime.queryInterface( XForm.class, xContainer );
        if ( ( null != xForm ) && ( m_nLevel > 1 ) )
            // don't step into sub forms - we only handle the form we were originally
            // applied to
            return false;

        return true;
    }

    /* ------------------------------------------------------------------ */
    public void handle( Object aFormComponent ) throws com.sun.star.uno.Exception
    {
        // entering this nesting level
        ++m_nLevel;

        // check if the component has a DataField property
        XPropertySet xCompProps = UNO.queryPropertySet( aFormComponent );
        XPropertySetInfo xPSI = null;
        if ( null != xCompProps )
            xPSI = xCompProps.getPropertySetInfo();

        if ( ( null != xPSI ) && xPSI.hasPropertyByName( "DataField" ) )
        {   // indeed it has ....
            String sDataField = (String)xCompProps.getPropertyValue( "DataField" );
            if ( sDataField.equals( m_sDataField ) )
            {   // we found a control model which is bount to what we're looking for
                xCompProps.setPropertyValue( "ReadOnly", m_aLockIt );
            }
        }

        // allow the super class to step down, if possible
        super.handle( aFormComponent );

        // leaving this nesting level
        --m_nLevel;
    }
};

/**************************************************************************/
/** a class which automatically handles control locking.
    <p>The class has to be bound to a form. Upon every movement of the form,
    all controls which are bound to a (to be specified) field are locked
    on existing and unlocked on new records.</p>
*/
class ControlLock implements XRowSetListener
{
    private XPropertySet    m_xForm;
    private String          m_sDataField;
    private boolean         m_bLockingEnabled;
    private boolean         m_bPreviousRoundLock;

    /* ------------------------------------------------------------------ */
    ControlLock( XPropertySet xForm, String sBoundDataField )
    {
        m_xForm = xForm;
        m_sDataField = sBoundDataField;
        m_bLockingEnabled = false;
        m_bPreviousRoundLock = false;
    }

    /* ------------------------------------------------------------------ */
    /** updates the locks on the affected controls
    */
    protected void updateLocks( )
    {
        try
        {
            // first determine if we need to lock
            Boolean aIsNewRecord = (Boolean)m_xForm.getPropertyValue( "IsNew" );

            boolean bNeedLock = m_bLockingEnabled && !aIsNewRecord.booleanValue();

            if ( m_bPreviousRoundLock != bNeedLock )
            {
                LockControlModels aLocker = new LockControlModels( m_sDataField, bNeedLock );
                aLocker.handle( m_xForm );
                m_bPreviousRoundLock = bNeedLock;
            }

            // please note that we choose the expensive way here: We always loop through
            // _all_ control models belonging to the form. This clearly slows down the
            // whole process.
            // A better solution would be to cache the affected control models. Then we
            // could either rely on the fact that the model hierarchy is static, or we
            // could add ourself as container listener to the form.
        }
        catch(com.sun.star.uno.Exception e)
        {
            System.out.println(e);
            e.printStackTrace();
        }
    }

    /* ------------------------------------------------------------------ */
    /** enables the locking in general
        <p>If the control models are really locked depends on the current
        record of the form: on the insert row, controls are never locked.</p>
    */
    public void enableLock( boolean bLock )
    {
        // remember this new setting
        m_bLockingEnabled = bLock;

        // add or remove ourself as listener to get notified of cursor moves
        XRowSet xRowSet = (XRowSet)UnoRuntime.queryInterface(
            XRowSet.class, m_xForm );
        if ( m_bLockingEnabled )
        {
            xRowSet.addRowSetListener( this );
        }
        else
        {
            xRowSet.removeRowSetListener( this );
        }

        // update the locks
        updateLocks();
    }

    /* ==================================================================
       = UNO callbacks
       ================================================================== */

    /* ------------------------------------------------------------------ */
    // XResetListener overridables
    /* ------------------------------------------------------------------ */
    public void cursorMoved( EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
        updateLocks( );
    }

    /* ------------------------------------------------------------------ */
    public void rowChanged( EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
        // not interested in
    }

    /* ------------------------------------------------------------------ */
    public void rowSetChanged( EventObject aEvent ) throws com.sun.star.uno.RuntimeException
    {
        // not interested in
    }

    /* ------------------------------------------------------------------ */
    // XEventListener overridables
    /* ------------------------------------------------------------------ */
    public void disposing( EventObject aEvent )
    {
        // not interested in
    }
}
