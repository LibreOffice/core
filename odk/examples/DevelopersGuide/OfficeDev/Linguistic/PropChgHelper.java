
import com.sun.star.linguistic2.XLinguServiceEventBroadcaster;
import com.sun.star.linguistic2.XLinguServiceEventListener;
import com.sun.star.linguistic2.LinguServiceEvent;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.XInterface;

import java.util.ArrayList;

public class PropChgHelper implements
        XPropertyChangeListener,
        XLinguServiceEventBroadcaster
{
    XInterface          xEvtSource;
    String[]            aPropNames;
    XPropertySet        xPropSet;
    ArrayList           aLngSvcEvtListeners;

    public PropChgHelper(
            XInterface      xEvtSource,
            String[]        aPropNames )
    {
        this.xEvtSource = xEvtSource;
        this.aPropNames = aPropNames;
        xPropSet        = null;
        aLngSvcEvtListeners = new ArrayList();
    }

    public XInterface GetEvtSource()
    {
        return xEvtSource;
    }

    public XPropertySet GetPropSet()
    {
        return xPropSet;
    }

    public String[] GetPropNames()
    {
        return aPropNames;
    }

    public void LaunchEvent( LinguServiceEvent aEvt )
    {
        int nCnt = aLngSvcEvtListeners.size();
        for (int i = 0;  i < nCnt;  ++i)
        {
            XLinguServiceEventListener xLstnr =
                    (XLinguServiceEventListener) aLngSvcEvtListeners.get(i);
            if (xLstnr != null)
                xLstnr.processLinguServiceEvent( aEvt );
        }
    }

    public void AddAsListenerTo( XPropertySet xPropertySet )
    {
        // do not listen any longer to the old property set (if any)
        RemoveAsListener();

        // set new property set to be used and register as listener to it
        xPropSet = xPropertySet;
        if (xPropSet != null)
        {
            int nLen = aPropNames.length;
            for (int i = 0;  i < nLen;  ++i)
            {
                if (aPropNames[i].length() != 0)
                {
                    try {
                        xPropSet.addPropertyChangeListener(
                                aPropNames[i], (XPropertyChangeListener) this );
                    }
                    catch( Exception e ) {
                    }
                }
            }
        }
    }

    public void RemoveAsListener()
    {
        if (xPropSet != null)
        {
            int nLen = aPropNames.length;
            for (int i = 0;  i < nLen;  ++i)
            {
                if (aPropNames[i].length() != 0)
                {
                    try {
                        xPropSet.removePropertyChangeListener(
                                aPropNames[i], (XPropertyChangeListener) this );
                    }
                    catch( Exception e ) {
                    }
                }
            }

            xPropSet = null;
        }
    }

    // __________ interface methods __________

    //***************
    // XEventListener
    //***************
    public void disposing( EventObject aSource )
            throws com.sun.star.uno.RuntimeException
    {
        if (aSource.Source == xPropSet)
        {
            RemoveAsListener();
        }
    }

    //************************
    // XPropertyChangeListener
    //************************
    public void propertyChange( PropertyChangeEvent aEvt )
            throws com.sun.star.uno.RuntimeException
    {
        // will be overloaded in derived classes
    }

    //******************************
    // XLinguServiceEventBroadcaster
    //******************************
    public boolean addLinguServiceEventListener(
            XLinguServiceEventListener xListener )
        throws com.sun.star.uno.RuntimeException
    {
        boolean bRes = false;
        if (xListener != null)
        {
            bRes = aLngSvcEvtListeners.add( xListener );
        }
        return bRes;
    }

    public boolean removeLinguServiceEventListener(
            XLinguServiceEventListener xListener )
        throws com.sun.star.uno.RuntimeException
    {
        boolean bRes = false;
        if (xListener != null)
        {
            int nIdx = aLngSvcEvtListeners.indexOf( xListener );
            if (nIdx != -1)
            {
                aLngSvcEvtListeners.remove( nIdx );
                bRes = true;
            }
        }
        return bRes;
    }
};

