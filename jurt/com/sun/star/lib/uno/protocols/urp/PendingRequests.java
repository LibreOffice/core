/*************************************************************************
 *
 *  $RCSfile: PendingRequests.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:53:15 $
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
