
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

public class PropChgHelper_Spell extends PropChgHelper
{
    public PropChgHelper_Spell(
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
            boolean bSCWA = false;  // SPELL_CORRECT_WORDS_AGAIN ?
            boolean bSWWA = false;  // SPELL_WRONG_WORDS_AGAIN ?

            boolean bVal = ((Boolean) aEvt.NewValue).booleanValue();

            if (aEvt.PropertyName.equals( "IsIgnoreControlCharacters" ))
            {
                // nothing to be done
            }
            else if (aEvt.PropertyName.equals( "IsGermanPreReform" ))
            {
                bSCWA = bSWWA = true;
            }
            else if (aEvt.PropertyName.equals( "IsUseDictionaryList" ))
            {
                bSCWA = bSWWA = true;
            }
            else if (aEvt.PropertyName.equals( "IsSpellUpperCase" ))
            {
                bSCWA = false == bVal;          // FALSE->TRUE change?
                bSWWA = !bSCWA;                 // TRUE->FALSE change?
            }
            else if (aEvt.PropertyName.equals( "IsSpellWithDigits" ))
            {
                bSCWA = false == bVal;          // FALSE->TRUE change?
                bSWWA = !bSCWA;                 // TRUE->FALSE change?
            }
            else if (aEvt.PropertyName.equals( "IsSpellCapitalization" ))
            {
                bSCWA = false == bVal;      // FALSE->TRUE change?
                bSWWA = !bSCWA;             // TRUE->FALSE change?
            }

            if (bSCWA)
                nLngSvcFlags |= LinguServiceEventFlags.SPELL_CORRECT_WORDS_AGAIN;
            if (bSWWA)
                nLngSvcFlags |= LinguServiceEventFlags.SPELL_WRONG_WORDS_AGAIN;
            if (nLngSvcFlags != 0)
            {
                LinguServiceEvent aEvent = new LinguServiceEvent( GetEvtSource(), nLngSvcFlags );
                LaunchEvent( aEvent );
            }
        }
    }
};

