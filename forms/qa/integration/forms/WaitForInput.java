/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WaitForInput.java,v $
 * $Revision: 1.4 $
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
package integration.forms;

class WaitForInput extends java.lang.Thread
{
    private Object      m_aToNotify;
    private boolean     m_bDone;

    public WaitForInput( Object aToNotify )
    {
        m_aToNotify = aToNotify;
        m_bDone = false;
    }

    public boolean isDone()
    {
        return m_bDone;
    }

    public void run()
    {
        try
        {
            System.out.println( "\npress enter to exit" );
            System.in.read();

            m_bDone = true;
            // notify that the user pressed the key
            synchronized (m_aToNotify)
            {
                m_aToNotify.notify();
            }
        }
        catch( java.lang.Exception e )
        {
            // not really interested in
            System.err.println( e );
        }
    }
};

