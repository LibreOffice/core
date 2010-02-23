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

import com.sun.star.lib.uno.environments.remote.Message;
import com.sun.star.lib.uno.environments.remote.ThreadId;
import com.sun.star.uno.IMethodDescription;
import com.sun.star.uno.ITypeDescription;
import com.sun.star.uno.XCurrentContext;

final class UrpMessage extends Message {
    public UrpMessage(
        ThreadId threadId, boolean request, String objectId,
        ITypeDescription type, IMethodDescription method, boolean synchronous,
        XCurrentContext currentContext, boolean abnormalTermination,
        Object result, Object[] arguments, boolean internal)
    {
        super(
            threadId, request, objectId, type, method, synchronous,
            currentContext, abnormalTermination, result, arguments);
        this.internal = internal;
    }

    public boolean isInternal() {
        return internal;
    }

    private final boolean internal;
}
