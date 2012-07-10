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



package org.openoffice.test.vcl.widgets;

import org.openoffice.test.vcl.client.Constant;



/**
 * Proxy used to access VCL Docking window
 *
 */
public class VclDockingWin extends VclWindow {
    /**
     * Define VCL Docking window
     * @param uid the string id
     */
    public VclDockingWin(String uid) {
        super(uid);
    }



    public VclDockingWin(VclApp app, String id) {
        super(app, id);
    }



    /**
     * Docks a window on one edge of the desktop.
     */
    public void dock() {
        if (!isDocked())
            invoke(Constant.M_Dock);
    }

    /**
     * Undocks a docking window.
     */
    public void undock() {
        if (isDocked())
            invoke(Constant.M_Undock);
    }

    /**
     * Returns the docking state.
     * @return Returns TRUE if the window is docking, otherwise FALSE is
     *         returned.
     */
    public boolean isDocked() {
        return (Boolean) invoke(Constant.M_IsDocked);
    }
}
