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

