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

package graphical;

import java.util.Calendar;

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

    public TimeHelper()
    {}

    public void start()
        {
            Calendar cal = Calendar.getInstance();
            m_nSeconds = cal.get(Calendar.SECOND);
            m_nMilliSeconds = cal.get(Calendar.MILLISECOND);
        }
    public void stop()
        {
            Calendar cal = Calendar.getInstance();
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
