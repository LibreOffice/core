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

