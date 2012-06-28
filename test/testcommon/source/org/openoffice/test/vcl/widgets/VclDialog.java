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
import org.openoffice.test.vcl.client.SmartId;



/**
 * Proxy to access the VCL dialog
 */
public class VclDialog extends VclWindow {

    /**
     * Define the dialog with its string ID
     * @param id
     */
    public VclDialog(String uid) {
        super(uid);
    }


    public VclDialog(SmartId id) {
        super(id);
    }


    /**
     * Closes a dialog by pressing the Cancel button.
     */
    public void cancel() {
        invoke(Constant.M_Cancel);
    }

    /**
     * Closes a dialog with the Default button.
     */
    public void restoreDefaults()  {
        invoke(Constant.M_Default);
    }

    /**
     * Closes a dialog with the OK button.
     */
    public void ok()  {
        invoke(Constant.M_OK);
    }
}
