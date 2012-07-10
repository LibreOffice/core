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
 * VCL status bar proxy
 *
 */
public class VclStatusBar extends VclControl {

    public VclStatusBar(String id) {
        super(id);
    }

    public VclStatusBar(VclApp app, String id) {
        super(app, id);
    }

    /**
     * Get the text of the item with the given ID
     * @param id
     * @return
     */
    public String getItemTextById(int id) {
        return (String) invoke(Constant.M_StatusGetText, new Object[]{id});
    }

    /**
     * Get the text of the item at the given index
     * @param i
     * @return
     */
    public String getItemText(int i) {
        return getItemTextById(getItemId(i));
    }

    /**
     * Get the item count
     * @return
     */
    public int getItemCount() {
        return ((Long) invoke(Constant.M_StatusGetItemCount)).intValue();
    }

    /**
     * Get the item ID at the given index
     * @param i
     * @return
     */
    public int getItemId(int i) {
        return ((Long) invoke(Constant.M_StatusGetItemId, new Object[]{i + 1})).intValue();
    }

    /**
     * Check if the status box is progress box.
     * @return
     */
    public boolean isProgress() {
        return (Boolean) invoke(Constant.M_StatusIsProgress);
    }
}
