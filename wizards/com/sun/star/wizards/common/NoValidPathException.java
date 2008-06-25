package com.sun.star.wizards.common;

import com.sun.star.lang.XMultiServiceFactory;

public class NoValidPathException extends Exception
{
    public NoValidPathException(XMultiServiceFactory xMSF, String _sText)
    {
        super(_sText);
        // TODO: NEVER open a dialog in an exception
        if (xMSF != null)
        {
            SystemDialog.showErrorBox(xMSF, "dbwizres", "dbw", 521); // OfficePathnotavailable
        }
    }
}
