
import com.sun.star.linguistic2.XLinguServiceEventBroadcaster;
import com.sun.star.linguistic2.XLinguServiceEventListener;
import com.sun.star.linguistic2.LinguServiceEvent;
import com.sun.star.linguistic2.LinguServiceEventFlags;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;

import java.util.ArrayList;

public class PropChgHelper_Hyph extends PropChgHelper
{
    public PropChgHelper_Hyph(
            XInterface      xEvtSource,
            String[]        aPropNames )
    {
        super( xEvtSource, aPropNames );
    }

    //************************
    // XPropertyChangeListener
    //************************
    public void propertyChange( PropertyChangeEvent aEvt )
            throws com.sun.star.uno.RuntimeException
    {
        {
            short nLngSvcFlags = 0;
            if (aEvt.PropertyName.equals( "IsIgnoreControlCharacters" ))
            {
                // nothing to be done
            }
            else if (aEvt.PropertyName.equals( "IsUseDictionaryList" ) ||
                     aEvt.PropertyName.equals( "IsGermanPreReform" ) ||
                     aEvt.PropertyName.equals( "HyphMinLeading" ) ||
                     aEvt.PropertyName.equals( "HyphMinTrailing" ) ||
                     aEvt.PropertyName.equals( "HyphMinWordLength" ))
            {
                nLngSvcFlags = LinguServiceEventFlags.HYPHENATE_AGAIN;
            }

            if (nLngSvcFlags != 0)
            {
                LinguServiceEvent aEvent = new LinguServiceEvent( GetEvtSource(), nLngSvcFlags );
                LaunchEvent( aEvent );
            }
        }
    }
};

