/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.vcl.widgets;

import org.openoffice.test.vcl.client.Constant;
import org.openoffice.test.vcl.client.SmartId;
import org.openoffice.test.vcl.client.VclHookException;

/**
 * VCL message box proxy.
 *
 */
public class VclMessageBox extends VclControl {

    private String message = null;


    public VclMessageBox(SmartId id) {
        super(id);
    }

    /**
     * Construct the active message box with a given message.
     * The message can be used to distinguish message boxes.
     * @param msg
     */
    public VclMessageBox(SmartId id, String msg) {
        super(id);
        this.message = msg;
    }

    public VclMessageBox(String id, String msg) {
        super(id);
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

            if (WINDOW_MESSBOX != getType())
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
