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
 * Proxy used to access all VCL field controls
 *
 */
public class VclField extends VclEditBox{

    public VclField(SmartId smartId) {
        super(smartId);
    }

    /**
     * Construct the field control with its string ID
     * @param uid
     */
    public VclField(String uid) {
        super(uid);
    }

    /**
     * Move one entry higher of Field
     *
     */
    public void more() {
        invoke(Constant.M_More);
    }

    /**
     * Move one entry lower of Field
     *
     */
    public void less() {
        invoke(Constant.M_Less);
    }

    /**
     * Goes to the maximum value of Field
     *
     */
    public void toMax() {
        invoke(Constant.M_ToMax);
    }


    /**
     * Goes to the minimum value of Field
     */
    public void toMin() {
        invoke(Constant.M_ToMin);
    }
}
