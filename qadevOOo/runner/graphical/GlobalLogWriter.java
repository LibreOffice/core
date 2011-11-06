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

import share.LogWriter;
import stats.SimpleLogWriter;

public class GlobalLogWriter
{
    private static LogWriter m_aGlobalLogWriter = null;

    /**
     * This is just a helper to get clearer code.
     * use this GlobalLogWriter.println(...)
     * @param _sMsg
     */
    protected static synchronized void println(String _sMsg)
    {
        get().println(_sMsg);
    }

    /**
     * @deprecated use GlobalLogWriter.println(...) direct
     * @return
     */
    protected static synchronized LogWriter get()
        {
            if (m_aGlobalLogWriter == null)
            {
                SimpleLogWriter aLog = new SimpleLogWriter();
                m_aGlobalLogWriter = aLog;
            }
            return m_aGlobalLogWriter;
        }

//     public static synchronized void initialize()
//         {
//             get().initialize(null, true);
//         }

    protected static synchronized void set(LogWriter _aLog)
        {
            m_aGlobalLogWriter = _aLog;
        }

}

