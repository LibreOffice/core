/*************************************************************************
 *
 *  $RCSfile: ThreadId_Test.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-20 09:23:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
