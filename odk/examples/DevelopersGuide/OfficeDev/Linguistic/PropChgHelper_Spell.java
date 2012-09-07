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

import com.sun.star.linguistic2.LinguServiceEvent;
import com.sun.star.linguistic2.LinguServiceEventFlags;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.uno.XInterface;

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

