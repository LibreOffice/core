package com.sun.star.wizards.common;

import com.sun.star.lang.XMultiServiceFactory;

public class InvalidQueryException extends java.lang.Throwable {

    public InvalidQueryException(XMultiServiceFactory xMSF, String sCommand) {
        final int RID_REPORT = 2400;
        SystemDialog.showErrorBox(xMSF, "ReportWizard", "dbw", RID_REPORT + 65, "<STATEMENT>", sCommand); // Querycreationnotpossible
    }

}
