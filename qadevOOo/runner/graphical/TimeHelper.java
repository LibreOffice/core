/*
 * ************************************************************************
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
 * ***********************************************************************
 */

package graphical;

import java.util.Calendar;

/**
 *
 * @author ll93751
 */
public class TimeHelper
{
    /*
      wait a second the caller don't need to handle the interruptexception
      @param _nSeconds how long should we wait
      @param _sReason  give a good reason, why we have to wait
     */
    static void waitInSeconds(int _nSeconds, String _sReason)
        {
            GlobalLogWriter.println("Wait 0.25 * " + String.valueOf(_nSeconds) + " sec. Reason: " + _sReason);
            try {
                java.lang.Thread.sleep(_nSeconds * 250);
            } catch (java.lang.InterruptedException e2) {}
        }

    private int m_nSeconds;
    private int m_nMilliSeconds;
    private long m_nRealMilliSeconds;

    private boolean m_bIsStopped = false;

    public TimeHelper()
    {}

    public void start()
        {
            m_bIsStopped = false;
            Calendar cal = Calendar.getInstance();
            m_nSeconds = cal.get(Calendar.SECOND);
            m_nMilliSeconds = cal.get(Calendar.MILLISECOND);
        }
    public void stop()
        {
            Calendar cal = Calendar.getInstance();
            m_bIsStopped = true;
            int nSeconds = cal.get(Calendar.SECOND);
            m_nSeconds = nSeconds - m_nSeconds;
            if (m_nSeconds < 0)
            {
                // add a minute
                m_nSeconds += 60;
            }

            int nMilliSeconds = cal.get(Calendar.MILLISECOND);
            m_nMilliSeconds = nMilliSeconds - m_nMilliSeconds;
            m_nRealMilliSeconds = m_nSeconds * 1000 + m_nMilliSeconds;
        }

    public String getTime()
        {
            return String.valueOf(m_nRealMilliSeconds);
        }

}
