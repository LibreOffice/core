/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package complex.loadAllDocuments;

import com.sun.star.beans.PropertyValue;

import com.sun.star.uno.Exception;
import com.sun.star.uno.RuntimeException;

import com.sun.star.task.XInteractionHandler;
import com.sun.star.task.XInteractionAbort;
import com.sun.star.task.XInteractionRetry;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;

//import java.lang.*;


/**
 * Implemets a simple interaction handler,
 * which can abort all incoming interactions only ... but make it possible to
 * log it. So it can be used for debug and test purposes.
 */
public class InteractionHandler implements XInteractionHandler
{
    // ____________________

    /**
     * @const   RETRY_COUNT it defines the max count of
     * retrying of an interaction
     */
    private static final int RETRY_COUNT    = 3;

    // ____________________

    /**
     * @member  m_aRequest      the origianl interaction request
     * safed for later analyzing
     * @member  m_bWasUsed      true if the interaction handler was used
     * @member  m_nTry          count using of RETRY continuations
     */
    private Object          m_aRequest  ;
    private int             m_nTry      ;
    private boolean         m_bWasUsed  ;


    /**
     * ctor
     * It's initialize an object of this class with default values
     * and set the protocol stack. So the outside code can check
     * if this handler was used or not.
     */
    public InteractionHandler()
    {
        m_aRequest  = null     ;
        //m_aProtocol = aProtocol;
        m_nTry      = 0        ;
        m_bWasUsed = false;
    }

    /**
     * Called to start the interaction, because the outside code whish to solve
     * a detected problem or to inform the user about something.
     * We safe the informations here and can handle two well known continuations
     * only.
     * [abort and retry].
     *
     * @param xRequest
     *          describe the interaction
     */
    public void handle(com.sun.star.task.XInteractionRequest xRequest)
    {
        m_bWasUsed = true;

        // first sav thje original request
        // Our user can use this information later for some debug analyzing
        Object aRequest = xRequest.getRequest();
        synchronized(this)
        {
            m_aRequest = aRequest;
        }

        // analyze the possible continuations.
        // We can abort all incoming interactions only.
        // But additional we can try to continue it several times too.
        // Of course after e.g. three loops we have to stop and abort it.
        com.sun.star.task.XInteractionContinuation[] lContinuations = xRequest.getContinuations();

        com.sun.star.task.XInteractionAbort xAbort     = null;
        com.sun.star.task.XInteractionRetry xRetry     = null;
        com.sun.star.uno.Type               xAbortType = new com.sun.star.uno.Type(com.sun.star.task.XInteractionAbort.class);
        com.sun.star.uno.Type               xRetryType = new com.sun.star.uno.Type(com.sun.star.task.XInteractionRetry.class);

        for (int i=0; i<lContinuations.length; ++i)
        {
            try
            {
                if (xAbort == null)
                    xAbort = (com.sun.star.task.XInteractionAbort)AnyConverter.toObject(xAbortType, lContinuations[i]);
                if (xRetry == null)
                    xRetry = (com.sun.star.task.XInteractionRetry)AnyConverter.toObject(xRetryType, lContinuations[i]);
            }
            catch(com.sun.star.lang.IllegalArgumentException exArg) {}
        }

        // try it again, but only if it wasn't tried to much before.
        if (xRetry != null)
        {
            synchronized(this)
            {
                if (m_nTry < RETRY_COUNT)
                {
                    ++m_nTry;
                    xRetry.select();
                    return;
                }
            }
        }

        // otherwhise we can abort this interaction only
        if (xAbort != null)
        {
            xAbort.select();
        }
    }

    public boolean wasUsed() {
        return m_bWasUsed;
    }
}
