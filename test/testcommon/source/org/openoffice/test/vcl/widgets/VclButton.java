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


/**
 *
 * Button/CheckBox/RadioBox/TriStateBox
 *
 */
public class VclButton extends VclControl {

    /**
     * Construct the control with its String id
     * @param uid
     */
    public VclButton(String uid) {
        super(uid);
    }


    public VclButton(SmartId id) {
        super(id);
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
