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
