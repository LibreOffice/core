/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestHelper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 17:27:15 $
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
package complex.standalonedocumentinfo;

import share.LogWriter;

public class TestHelper {
    LogWriter m_aLogWriter;
    String m_sTestPrefix;

    /** Creates a new instance of TestHelper */
    public TestHelper ( LogWriter aLogWriter, String sTestPrefix ) {
        m_aLogWriter = aLogWriter;
        m_sTestPrefix = sTestPrefix;
    }

    public void Error ( String sError ) {
        m_aLogWriter.println ( m_sTestPrefix + "Error: " + sError );
    }

    public void Message ( String sMessage ) {
        m_aLogWriter.println ( m_sTestPrefix + sMessage );
    }
}
