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

package com.sun.star.uno;

import org.junit.Test;
import static org.junit.Assert.*;

public final class Any_Test {
    @Test public void testAnyAny() {
        boolean caught = false;
        try {
            new Any(Type.ANY, null);
        } catch (IllegalArgumentException e) {
            caught = true;
        }
        assertTrue(caught);
    }

    @Test public void testComplete() {
        assertSame(Any.VOID, Any.complete(Any.VOID));
        assertEquals(
            new Any(Type.LONG, new Integer(10)), Any.complete(new Integer(10)));
        assertEquals(
            new Any(new Type(XInterface.class), null), Any.complete(null));
        XInterface x = new XInterface() {};
        assertEquals(new Any(new Type(XInterface.class), x), Any.complete(x));
    }
}
