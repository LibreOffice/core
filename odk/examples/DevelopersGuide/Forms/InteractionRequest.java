import com.sun.star.uno.*;
import com.sun.star.task.*;
import java.util.Vector;

/**************************************************************************/
/** helper class for implementing an interaction request.
*/
class InteractionRequest implements XInteractionRequest
{
    private Object  m_aRequest;
    private Vector  m_aContinuations;

    /* ------------------------------------------------------------------ */
    public InteractionRequest( Object aRequest )
    {
        m_aRequest = aRequest;
        m_aContinuations = new Vector();
    }

    /* ------------------------------------------------------------------ */
    public void addContinuation( XInteractionContinuation xCont )
    {
        if ( null != xCont )
            m_aContinuations.add( xCont );
    }

    /* ------------------------------------------------------------------ */
    public Object getRequest(  )
    {
        return m_aRequest;
    }

    /* ------------------------------------------------------------------ */
    public XInteractionContinuation[] getContinuations(  )
    {
        XInteractionContinuation[] aContinuations = new XInteractionContinuation[ m_aContinuations.size() ];
        for ( int i=0; i<m_aContinuations.size(); ++i )
            aContinuations[ i ] = (XInteractionContinuation)m_aContinuations.elementAt( i );
        return aContinuations;
    }
};

