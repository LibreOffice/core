/*************************************************************************
 *
 *  $RCSfile: RowSetEventListener.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2005-03-18 10:03:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
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
package complex.dbaccess;

import com.sun.star.sdb.XRowSetApproveListener;
import com.sun.star.sdbc.XRowSetListener;
import com.sun.star.sdb.RowChangeEvent;
import com.sun.star.lang.EventObject;
import com.sun.star.beans.XPropertyChangeListener;

public class RowSetEventListener implements XRowSetApproveListener,XRowSetListener,XPropertyChangeListener
{
    public static final int APPROVE_CURSOR_MOVE = 0;
    public static final int APPROVE_ROW_CHANGE  = 1;
    public static final int COLUMN_VALUE        = 2;
    public static final int CURSOR_MOVED        = 3;
    public static final int ROW_CHANGED         = 4;
    public static final int IS_MODIFIED         = 5;
    public static final int IS_NEW              = 6;
    public static final int ROW_COUNT           = 7;
    public static final int IS_ROW_COUNT_FINAL  = 8;

    RowSet rowset;
    int callPos = 1;
    int calling [];

    RowSetEventListener(RowSet _rowset){
        rowset = _rowset;
        calling = new int [9];
        clearCalling();
    }
    public int[] getCalling(){
        return calling;
    }
    public void clearCalling(){
        for(int i = 0 ; i < calling.length; ++i){
            calling[i] = -1;
        }
        callPos = 1;
    }
    // XEventListener
    public void disposing(com.sun.star.lang.EventObject event)
    {
    }
    // XRowSetApproveBroadcaster
    public boolean approveCursorMove(EventObject event)
    {
        calling[APPROVE_CURSOR_MOVE] = callPos++;
        return true;
    }
    public boolean approveRowChange(RowChangeEvent event)
    {
        calling[APPROVE_ROW_CHANGE] = callPos++;
        return true;
    }
    public boolean approveRowSetChange(EventObject event)
    {
        return true;
    }

    // XRowSetListener
    public void cursorMoved(com.sun.star.lang.EventObject event)
    {
        calling[CURSOR_MOVED] = callPos++;
    }
    public void rowChanged(com.sun.star.lang.EventObject event)
    {
        calling[ROW_CHANGED] = callPos++;
    }
    public void rowSetChanged(com.sun.star.lang.EventObject event)
    {
    }

    public void propertyChange(com.sun.star.beans.PropertyChangeEvent propertyChangeEvent) {
        if ( propertyChangeEvent.PropertyName.equals("Value") ){
            calling[COLUMN_VALUE] = callPos++;
        } else if ( propertyChangeEvent.PropertyName.equals("IsModified") ){
            calling[IS_MODIFIED] = callPos++;
        } else if ( propertyChangeEvent.PropertyName.equals("IsNew") ){
            calling[IS_NEW] = callPos++;
        } else if ( propertyChangeEvent.PropertyName.equals("RowCount") ){
            calling[ROW_COUNT] = callPos++;
        } else if ( propertyChangeEvent.PropertyName.equals("IsRowCountFinal") ){
            calling[IS_ROW_COUNT_FINAL] = callPos++;
        }
    }

}
