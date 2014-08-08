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

    private int callPos = 1;
    private int calling [];

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
