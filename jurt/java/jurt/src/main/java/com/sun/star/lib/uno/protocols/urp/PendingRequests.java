/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
