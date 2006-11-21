/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GlobalLogWriter.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 14:09:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package convwatch;

import share.LogWriter;
import stats.SimpleLogWriter;

public class GlobalLogWriter
{
    private static LogWriter m_aGlobalLogWriter = null;
    public static synchronized LogWriter get()
        {
            if (m_aGlobalLogWriter == null)
            {
                SimpleLogWriter aLog = new SimpleLogWriter();
                m_aGlobalLogWriter = aLog;
            }
            return m_aGlobalLogWriter;
        }

    public static synchronized void set(LogWriter _aLog)
        {
            m_aGlobalLogWriter = _aLog;
        }

}

