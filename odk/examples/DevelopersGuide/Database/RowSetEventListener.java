/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.sdb.XRowSetApproveListener;
import com.sun.star.sdbc.XRowSetListener;
import com.sun.star.sdb.RowChangeEvent;
import com.sun.star.lang.EventObject;

public class RowSetEventListener implements XRowSetApproveListener,XRowSetListener
{
    // XEventListener
    public void disposing(com.sun.star.lang.EventObject event)
    {
        System.out.println("RowSet will be destroyed!");
    }
    // XRowSetApproveBroadcaster
    public boolean approveCursorMove(EventObject event)
    {
        System.out.println("Before CursorMove!");
        return true;
    }
    public boolean approveRowChange(RowChangeEvent event)
    {
        System.out.println("Before row change!");
        return true;
    }
    public boolean approveRowSetChange(EventObject event)
    {
        System.out.println("Before RowSet change!");
        return true;
    }

    // XRowSetListener
    public void cursorMoved(com.sun.star.lang.EventObject event)
    {
        System.out.println("Cursor moved!");
    }
    public void rowChanged(com.sun.star.lang.EventObject event)
    {
        System.out.println("Row changed!");
    }
    public void rowSetChanged(com.sun.star.lang.EventObject event)
    {
        System.out.println("RowSet changed!");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
