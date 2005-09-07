/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JavaThreadPoolFactory_Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:12:06 $
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

package com.sun.star.lib.uno.environments.remote;

import complexlib.ComplexTestCase;

public final class JavaThreadPoolFactory_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws InterruptedException {
        ThreadId i1 = JavaThreadPoolFactory.getThreadId();
        assure(i1.equals(JavaThreadPoolFactory.getThreadId()));
        final ThreadId[] i2 = new ThreadId[1];
        new Thread() {
            public void run() {
                synchronized (i2) {
                    i2[0] = JavaThreadPoolFactory.getThreadId();
                    i2.notify();
                }
            }
        }.start();
        synchronized (i2) {
            while (i2[0] == null) {
                i2.wait();
            }
        }
        assure(!i1.equals(i2[0]));
    }
}
