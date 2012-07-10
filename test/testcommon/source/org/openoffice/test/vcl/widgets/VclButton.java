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
 *
 * Button/CheckBox/RadioBox/TriStateBox
 *
 */
public class VclButton extends VclControl {

    public VclButton(String id) {
        super(id);
    }

    public VclButton(VclApp app, String id) {
        super(app, id);
    }

    /**
     *
     * Click the check box
     */
    public void click() {
        invoke(Constant.M_Click);
    }

    /**
     * Check if the check box is tristate
     */
    public boolean isTristate() {
        return (Boolean)invoke(Constant.M_IsTristate);
    }

    /**
     * Set the check box to triState status
     */
    public void triState() {
        invoke(Constant.M_TriState);
    }

    /**
     * Check if the check box is checked
     */
    public boolean isChecked() {
        return (Boolean) invoke(Constant.M_IsChecked);
    }

    /**
     * Set the check box to checked status
     *
     */
    public void check() {
        invoke(Constant.M_Check);
    }

    /**
     * Set the check box to unchecked status
     */
    public void uncheck() {
        invoke(Constant.M_UnCheck);
    }

    /**
     * Set the status to checked or unchecked
     * @param checked
     */
    public void setChecked(boolean checked) {
        if (checked)
            this.check();
        else
            this.uncheck();
    }
}
