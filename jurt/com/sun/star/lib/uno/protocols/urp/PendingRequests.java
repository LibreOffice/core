/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PendingRequests.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-14 08:26:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2000 by Sun Microsystems, Inc.
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

package com.sun.star.lib.uno.protocols.urp;

import com.sun.star.lib.uno.environments.remote.ThreadId;
import com.sun.star.uno.IMethodDescription;
import java.util.HashMap;
import java.util.Stack;

final class PendingRequests {
    public PendingRequests() {}

    public synchronized void push(ThreadId tid, Item item) {
        Stack s = (Stack) map.get(tid);
        if (s == null) {
            s = new Stack();
            map.put(tid, s);
        }
        s.push(item);
    }

    public synchronized Item pop(ThreadId tid) {
        Stack s = (Stack) map.get(tid);
        Item i = (Item) s.pop();
        if (s.empty()) {
            map.remove(tid);
        }
        return i;
    }

    public static final class Item {
        public Item(
            boolean internal, IMethodDescription function, Object[] arguments)
        {
            this.internal = internal;
            this.function = function;
            this.arguments = arguments;
        }

        public final boolean internal;
        public final IMethodDescription function;
        public final Object[] arguments;
    }

    private final HashMap map = new HashMap(); // from ThreadId to Stack of Item
}
