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

package complex.dbaccess;

import com.sun.star.sdb.XRowSetApproveListener;
import com.sun.star.sdbc.XRowSetListener;
import com.sun.star.sdb.RowChangeEvent;
import com.sun.star.lang.EventObject;
import com.sun.star.beans.XPropertyChangeListener;

public final class RowSetEventListener implements XRowSetApproveListener,XRowSetListener,XPropertyChangeListener
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

    int callPos = 1;
    int calling [];

    RowSetEventListener(){
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
        if ( "Value".equals(propertyChangeEvent.PropertyName) ){
            calling[COLUMN_VALUE] = callPos++;
        } else if ( "IsModified".equals(propertyChangeEvent.PropertyName) ){
            calling[IS_MODIFIED] = callPos++;
        } else if ( "IsNew".equals(propertyChangeEvent.PropertyName) ){
            calling[IS_NEW] = callPos++;
        } else if ( "RowCount".equals(propertyChangeEvent.PropertyName) ){
            calling[ROW_COUNT] = callPos++;
        } else if ( "IsRowCountFinal".equals(propertyChangeEvent.PropertyName) ){
            calling[IS_ROW_COUNT_FINAL] = callPos++;
        }
    }

}
