/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package complex.loadAllDocuments;

import com.sun.star.task.XInteractionHandler;
import com.sun.star.uno.AnyConverter;


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
