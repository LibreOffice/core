/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SandboxThreadGroup.java,v $
 * $Revision: 1.3 $
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

package com.sun.star.lib.sandbox;

public class SandboxThreadGroup extends ThreadGroup {
    private static int instances;
    private Object context = null;

    public SandboxThreadGroup(String name, Object context) {
        this(Thread.currentThread().getThreadGroup(), name, context);
    }

    public SandboxThreadGroup(ThreadGroup parent, String name, Object context) {
        super(parent, name);

        instances ++;

        setMaxPriority(Thread.NORM_PRIORITY - 1);

        this.context = context;
    }

    public Object getContext() {
        return context;
    }

    public void dispose() {
        context = null;
    }

    public void finalize() {
        instances --;
    }
}

