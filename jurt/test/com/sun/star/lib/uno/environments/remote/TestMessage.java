/*************************************************************************
 *
 *  $RCSfile: TestMessage.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-05-22 09:15:05 $
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
package com.sun.star.lib.uno.environments.remote;


import com.sun.star.uno.Type;

import com.sun.star.uno.ITypeDescription;


class TestMessage implements IMessage {
    boolean  _synchron;
    ITypeDescription     _iTypeDescription;
    String   _oid;
    ThreadId _threadId;
    Object   _result;
    String   _operation;
    Object   _params[];

    TestMessage(boolean synchron, ITypeDescription iTypeDescription, String oid, ThreadId threadId, Object result, String operation, Object params[]) {
        _synchron  = synchron;
        _iTypeDescription      = iTypeDescription;
        _oid       = oid;
        _threadId  = threadId;
        _result    = result;
        _operation = operation;
        _params    = params;
    }

    public String getOperation() {
        return _operation;
    }

    public ThreadId getThreadId() {
        return _threadId;
    }

    public ITypeDescription getInterface() {
        return _iTypeDescription;
    }

    public boolean isSynchron() {
        return _synchron;
    }

    public boolean mustReply() {
        return _synchron;
    }

    public boolean isException() {
        return false;
    }

    public String getOid() {
        return _oid;
    }

    public Object getData(Object params[][]) {
        params[0] = _params;
        return _result;
//              return new Integer(_requestId);
    }
}


