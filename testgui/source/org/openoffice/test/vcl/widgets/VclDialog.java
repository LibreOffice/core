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
