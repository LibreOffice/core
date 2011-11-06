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

package complex.embedding;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;

import com.sun.star.lang.*;
import com.sun.star.embed.*;
import com.sun.star.packages.*;
import com.sun.star.io.*;
import com.sun.star.beans.*;

import share.LogWriter;

public class TestHelper  {

    LogWriter m_aLogWriter;
    String m_sTestPrefix;

    public TestHelper( LogWriter aLogWriter, String sTestPrefix )
    {
        m_aLogWriter = aLogWriter;
        m_sTestPrefix = sTestPrefix;
    }

    public void Error( String sError )
    {
        m_aLogWriter.println( m_sTestPrefix + "Error: " + sError );
    }

    public void Message( String sMessage )
    {
        m_aLogWriter.println( m_sTestPrefix + sMessage );
    }
}

