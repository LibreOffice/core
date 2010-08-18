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
