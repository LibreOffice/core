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

// __________ Imports __________



// __________ Implementation __________

/**
 * Implemets a simple status indicator, which
 * provide informations about state of a load request.
 * It can be used as an argument e.g. for loadComponentFromURL().
 */
public class StatusIndicator implements com.sun.star.task.XStatusIndicator
{
    // ____________________

    /**
     * @const SHOWSTATUS_NO         don't show the status - but save information about using of this indicator object
     * @const SHOWSTATUS_LOG        the possible set protocol object will be used (it covers STDOUT, STDERR automaticly too)
     * @const SHOWSTATUS_DIALOG     the status will be shown inside a java dialog
     * @const SHOWSTATUS_LINK       the status will be notified to interested listener (one listener only!)
     */
    public static final int SHOWSTATUS_NO       =   0;
    public static final int SHOWSTATUS_LOG      =   1;
    public static final int SHOWSTATUS_DIALOG   =   4;
    public static final int SHOWSTATUS_LINK     =   8;

    // ____________________

    /**
     * @member m_nRange     max value for any progress
     */
    private int             m_nRange    ;
    private boolean         m_bWasUsed  ;

    // ____________________

    /**
     * ctor
     * It's initialize an object of this class with default values.
     */
    public StatusIndicator( int nOut)
    {
        m_nRange    = 100           ;
        m_bWasUsed  = false;
    }

    // ____________________

    /**
     * It starts the progress and set the initial text and range.
     *
     * @param sText
     *          the initial text for showing
     *
     * @param nRange
     *          the new range for following progress
     */
    public void start( /*IN*/String sText, /*IN*/int nRange )
    {
        synchronized(this)
        {
            m_bWasUsed = true;
            m_nRange = nRange;
        }
        impl_show();
    }

    // ____________________

    /**
     * Finish the progress and reset internal members.
     */
    public void end()
    {
        synchronized(this)
        {
            m_bWasUsed = true;
            m_nRange = 100;
        }
        impl_show();
    }

    // ____________________

    /**
     * Set the new description text.
     *
     * @param sText
     *          the new text for showing
     */
    public void setText( /*IN*/String sText )
    {
        synchronized(this)
        {
            m_bWasUsed = true;
        }
        impl_show();
    }

    // ____________________

    /**
     * Set the new progress value.
     *
     * @param nValue
     *          the new progress value
     *          Must fit the range!
     */
    public void setValue( /*IN*/int nValue )
    {
        synchronized(this)
        {
            m_bWasUsed = true;
        }
        impl_show();
    }

    // ____________________

    /**
     * Reset text and progress value to her defaults.
     */
    public void reset()
    {
        synchronized(this)
        {
            m_bWasUsed = true;
        }
        impl_show();
    }

    // ____________________

    /**
     * Internal helper to show the status.
     * Currently it's implement as normal text out on stdout.
     * But of course other thimngs are possible here too.
     * e.g. a dialog
     */
    private void impl_show()
    {
/*        synchronized(this)
        {
            if ((m_nOut & SHOWSTATUS_LOG) == SHOWSTATUS_LOG)
                //m_aProtocol.log("\t["+m_nValue+"/"+m_nRange+"] "+m_sText+"\n");

            //if ((m_nOut & SHOWSTATUS_DIALOG) == SHOWSTATUS_DIALOG)
                // not supported yet!

            //if ((m_nOut & SHOWSTATUS_LINK) == SHOWSTATUS_LINK)
                // not supported yet!
        } */
    }

    public boolean wasUsed() {
        return m_bWasUsed;
    }
}
