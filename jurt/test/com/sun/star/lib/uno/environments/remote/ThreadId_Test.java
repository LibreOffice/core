/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ThreadId_Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:13:45 $
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
import java.util.Arrays;

public final class ThreadId_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() {
        ThreadId i1 = ThreadId.createFresh();
        assure(i1.equals(i1));
        assure(!i1.equals(null));
        assure(!i1.equals(new Object()));
        assure(i1.hashCode() == i1.hashCode());
        byte[] i1bytes = i1.getBytes();
        assure(i1bytes != null);
        assure(
            i1bytes.length >= 5 && i1bytes[0] == 'j' && i1bytes[1] == 'a'
            && i1bytes[2] == 'v' && i1bytes[3] == 'a' && i1bytes[4] == ':');
        assure(Arrays.equals(i1bytes, i1.getBytes()));

        ThreadId i2 = ThreadId.createFresh();
        assure(!i1.equals(i2));
        assure(!i2.equals(i1));
        assure(!Arrays.equals(i1bytes, i2.getBytes()));

        ThreadId i3 = new ThreadId(i1bytes);
        assure(i3.equals(i1));
        assure(!i3.equals(i2));
        assure(i1.equals(i3));
        assure(i1.hashCode() == i3.hashCode());
        assure(Arrays.equals(i1bytes, i3.getBytes()));
    }
}
