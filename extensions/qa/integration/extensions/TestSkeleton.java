/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestSkeleton.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-31 12:18:14 $
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

package integration.extensions;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;

public class TestSkeleton extends complexlib.ComplexTestCase
{
    private XMultiServiceFactory    m_orb;

    /** Creates a new instance of ValueBinding */
    public TestSkeleton()
    {
    }

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkSomething"
        };
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return "Test Skeleton";
    }

    /* ------------------------------------------------------------------ */
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
    }

    /* ------------------------------------------------------------------ */
    public void after() throws com.sun.star.uno.Exception, java.lang.Exception
    {
    }

    /* ------------------------------------------------------------------ */
    public void checkSomething() throws com.sun.star.uno.Exception, java.lang.Exception
    {
    }
}
