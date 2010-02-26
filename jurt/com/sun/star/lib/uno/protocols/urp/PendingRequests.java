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
