/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UrpMessage.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-14 08:26:44 $
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
