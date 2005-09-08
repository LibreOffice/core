/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XJavaThreadRegister_11.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:13:48 $
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

package ifc.java;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.java.XJavaThreadRegister_11;

/**
* Testing <code>com.sun.star.java.XJavaThreadRegister_11</code>
* interface methods :
* <ul>
*  <li><code> isThreadAttached()</code></li>
*  <li><code> registerThread()</code></li>
*  <li><code> revokeThread()</code></li>
* </ul> <p>
* <b> Nothing tested here. </b>
* @see com.sun.star.java.XJavaThreadRegister_11
*/
public class _XJavaThreadRegister_11 extends MultiMethodTest {
    public XJavaThreadRegister_11 oObj;

    protected void after() {
        log.println("Skipping all XJavaThreadRegister_11 methods, since they"
                + " can't be tested in the context");
        throw new StatusException(Status.skipped(true));
    }

    public void _isThreadAttached() {
        // skipping the test
        tRes.tested("isThreadAttached()", true);
    }

    public void _registerThread() {
        // skipping the test
        tRes.tested("registerThread()", true);
    }

    public void _revokeThread() {
        // skipping the test
        tRes.tested("revokeThread()", true);
    }
}
