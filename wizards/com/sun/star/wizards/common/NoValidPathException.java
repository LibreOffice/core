package com.sun.star.wizards.common;

import com.sun.star.lang.XMultiServiceFactory;

public class NoValidPathException extends Exception {

    public NoValidPathException(XMultiServiceFactory xMSF) {
        SystemDialog.showErrorBox(xMSF, "dbwizres", "dbw", 506); // OfficePathnotavailable
    }

}
