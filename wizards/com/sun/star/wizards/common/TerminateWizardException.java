package com.sun.star.wizards.common;

import com.sun.star.wizards.common.Resource;
import com.sun.star.lang.XMultiServiceFactory;

public class TerminateWizardException extends Exception {

    public TerminateWizardException(XMultiServiceFactory xMSF) {
        Resource oResource = new Resource(xMSF, "AutoPilot", "dbw");
        String sErrorMessage = oResource.getResText(1006);
        SystemDialog.showMessageBox(xMSF, "ErrorBox", com.sun.star.awt.VclWindowPeerAttribute.OK, sErrorMessage);
        printStackTrace(System.out);
    }

}
