/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Any_Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-05-04 08:07:19 $
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

package com.sun.star.uno;

import complexlib.ComplexTestCase;

public final class Any_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "testAnyAny", "testComplete" };
    }

    public void testAnyAny() {
        boolean caught = false;
        try {
            new Any(Type.ANY, null);
        } catch (IllegalArgumentException e) {
            caught = true;
        }
        assure(caught);
    }

    public void testComplete() {
        assure(Any.complete(Any.VOID) == Any.VOID);
        assure(
            Any.complete(new Integer(10)).equals(
                new Any(Type.LONG, new Integer(10))));
        assure(
            Any.complete(null).equals(
                new Any(new Type(XInterface.class), null)));
        XInterface x = new XInterface() {};
        assure(Any.complete(x).equals(new Any(new Type(XInterface.class), x)));
    }
}
