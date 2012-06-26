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
 * Proxy used to access VCL EditField/MultiLineEditField
 *
 */
public class VclEditBox extends VclControl {

    /**
     * Construct the control with its string ID
     * @param uid
     */
    public VclEditBox(String uid) {
        super(uid);
    }

    public VclEditBox(SmartId smartId) {
        super(smartId);
    }

    /**
     * Set the text of edit box
     * @param str
     */
    public void setText(String str) {
        invoke(Constant.M_SetText, new Object[]{str});
    }


    /**
     * Is the edit box writable?
     * @return true if it is writable, false otherwise
     */
    public boolean isWritable() {
        return (Boolean)invoke(Constant.M_IsWritable);
    }


    /**
     * Get the text of edit box
     * @return the text of edit box
     */
    public String getText(){
        return (String) invoke(Constant.M_GetText);
    }
}
