/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.lib.uno.protocols.urp;

import com.sun.star.lib.uno.environments.remote.ThreadId;
import com.sun.star.uno.IMethodDescription;
import java.util.HashMap;
import java.util.Stack;

final class PendingRequests {

    public synchronized void push(ThreadId tid, Item item) {
        Stack<Item> s = map.get(tid);
        if (s == null) {
            s = new Stack<Item>();
            map.put(tid, s);
        }
        s.push(item);
    }

    public synchronized Item pop(ThreadId tid) {
        Stack<Item> s = map.get(tid);
        Item i = s.pop();
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

    private final HashMap<ThreadId, Stack<Item>> map = new HashMap<ThreadId, Stack<Item>>(); // from ThreadId to Stack of Item
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
