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
import org.openoffice.test.vcl.client.VclHookException;

/**
 * VCL message box proxy.
 *
 */
public class VclMessageBox extends VclControl {

    private String message = null;

    public VclMessageBox() {
        super(Constant.UID_ACTIVE);
    }

    public VclMessageBox(String id) {
        super(id);
    }

    public VclMessageBox(String id, String msg) {
        super(id);
        this.message = msg;
    }

    public VclMessageBox(VclApp app, String id) {
        super(app, id);
    }

    public VclMessageBox(VclApp app, String id, String msg) {
        super(app, id);
        this.message = msg;
    }


    /**
     * Get the message on the message box
     * @return
     */
    public String getMessage() {
        return (String) invoke(Constant.M_GetText);
    }

    public boolean exists() {
        try {
            boolean exists = super.exists();
            if (!exists)
                return false;

            if (Constant.WINDOW_MESSBOX != getType())
                return false;

            if (message != null) {
                String msg = getMessage();
                return msg.contains(message);
            }

            return true;
        } catch (VclHookException e) {
            return false;
        }
    }

    /**
     * Click the yes button on the message box
     *
     */
    public void yes(){
        invoke(Constant.M_Yes);
    }

    /**
     * Click the no button on the message box
     *
     */
    public void no(){
        invoke(Constant.M_No);
    }


    /**
     * Closes a dialog by pressing the Cancel button.
     */
    public void cancel() {
        invoke(Constant.M_Cancel);
    }

    /**
     * Closes a dialog with the Close button.
     */
    public void close() {
        invoke(Constant.M_Close);
    }

    /**
     * Closes a dialog with the Default button.
     */
    public void doDefault()  {
        invoke(Constant.M_Default);
    }

    /**
     * Presses the Help button to open the help topic for the dialog.
     *
     */
    public void help()  {
        invoke(Constant.M_Help);
    }

    /**
     * Closes a dialog with the OK button.
     */
    public void ok()  {
        invoke(Constant.M_OK);
    }


    /**
     * Closes a dialog with the OK button.
     */
    public void repeat()  {
        invoke(Constant.M_Repeat);
    }

    /**
     * Get the check box text if it exists
     *
     * @return the check box text
     */
    public String getCheckBoxText() {
        return (String) invoke(Constant.M_GetCheckBoxText);
    }

    /**
     * Get the status of check box on the message box
     * @return
     */
    public boolean isChecked() {
        return (Boolean) invoke(Constant.M_IsChecked);
    }

    /**
     * Check the check box on the message box
     *
     */
    public void check() {
        invoke(Constant.M_Check);
    }

    /**
     * Uncheck the check box on the message box
     *
     */
    public void uncheck() {
        invoke(Constant.M_UnCheck);
    }

}
