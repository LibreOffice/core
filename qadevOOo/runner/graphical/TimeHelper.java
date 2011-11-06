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
