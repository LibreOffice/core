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
 * provide information about state of a load request.
 * It can be used as an argument e.g. for loadComponentFromURL().
 */
public class StatusIndicator implements com.sun.star.task.XStatusIndicator
{

    private boolean         m_bWasUsed  ;



    /**
     * ctor
     * It's initialize an object of this class with default values.
     */
    public StatusIndicator()
    {
        m_bWasUsed  = false;
    }



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
        }
        impl_show();
    }



    /**
     * Finish the progress and reset internal members.
     */
    public void end()
    {
        synchronized(this)
        {
            m_bWasUsed = true;
        }
        impl_show();
    }



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



    /*
     * Internal helper to show the status.
     * Currently it's implement as normal text out on stdout.
     * But of course other things are possible here too.
     * e.g. a dialog
     */
    private void impl_show()
    {
    }

    public boolean wasUsed() {
        return m_bWasUsed;
    }
}
