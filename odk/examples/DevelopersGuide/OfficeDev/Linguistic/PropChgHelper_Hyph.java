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

