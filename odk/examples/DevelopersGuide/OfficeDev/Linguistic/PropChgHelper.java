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
    private final XInterface          xEvtSource;
    private final String[]      aPropNames;
    private XPropertySet        xPropSet;
    private final ArrayList<XLinguServiceEventListener> aLngSvcEvtListeners;

    public PropChgHelper(
            XInterface      xEvtSource,
            String[]        aPropNames )
    {
        this.xEvtSource = xEvtSource;
        this.aPropNames = aPropNames;
        xPropSet        = null;
        aLngSvcEvtListeners = new ArrayList<XLinguServiceEventListener>();
    }

    public XInterface GetEvtSource()
    {
        return xEvtSource;
    }

    public XPropertySet GetPropSet()
    {
        return xPropSet;
    }



    public void LaunchEvent( LinguServiceEvent aEvt )
    {
        int nCnt = aLngSvcEvtListeners.size();
        for (int i = 0;  i < nCnt;  ++i)
        {
            XLinguServiceEventListener xLstnr =
                    aLngSvcEvtListeners.get(i);
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
                                aPropNames[i], this );
                    }
                    catch( Exception e ) {
                    }
                }
            }
        }
    }

    private void RemoveAsListener()
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
                                aPropNames[i], this );
                    }
                    catch( Exception e ) {
                    }
                }
            }

            xPropSet = null;
        }
    }

    // __________ interface methods __________


    // XEventListener

    public void disposing( EventObject aSource )
            throws com.sun.star.uno.RuntimeException
    {
        if (aSource.Source == xPropSet)
        {
            RemoveAsListener();
        }
    }


    // XPropertyChangeListener

    public void propertyChange( PropertyChangeEvent aEvt )
            throws com.sun.star.uno.RuntimeException
    {
        // will be overridden in derived classes
    }


    // XLinguServiceEventBroadcaster

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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
