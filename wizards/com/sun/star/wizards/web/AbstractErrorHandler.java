/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AbstractErrorHandler.java,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.wizards.web;

import com.sun.star.awt.VclWindowPeerAttribute;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.SystemDialog;

/**
 * An abstract implementation of ErrorHandler, which
 * uses a renderer method geMessageFor(Exception, Object, int, int)
 * (in this class still abstract...)
 * to render the errors, and displays
 * error messeges.
 */
public abstract class AbstractErrorHandler implements ErrorHandler
{

    XMultiServiceFactory xmsf;
    XWindowPeer peer;

    protected AbstractErrorHandler(XMultiServiceFactory xmsf, XWindowPeer peer_)
    {
        this.xmsf = xmsf;
        peer = peer_;
    }

    /**
     * Implementation of ErrorHandler:
     * shows a message box with the rendered error.
     * @param arg identifies the error. This object is passed to the render method
     * which returns the right error message.
     * @return true/false for continue/abort.
     */
    public boolean error(Exception ex, Object arg, int ix, int errorType)
    {
        //ex.printStackTrace();
        switch (errorType)
        {
            case ErrorHandler.ERROR_FATAL:
                return !showMessage(getMessageFor(ex, arg, ix, errorType), errorType);
            case ErrorHandler.ERROR_PROCESS_FATAL:
                return !showMessage(getMessageFor(ex, arg, ix, errorType), errorType);
            case ErrorHandler.ERROR_NORMAL_ABORT:
                return showMessage(getMessageFor(ex, arg, ix, errorType), errorType);
            case ErrorHandler.ERROR_NORMAL_IGNORE:
                return showMessage(getMessageFor(ex, arg, ix, errorType), errorType);
            case ErrorHandler.ERROR_QUESTION_CANCEL:
                return showMessage(getMessageFor(ex, arg, ix, errorType), errorType);
            case ErrorHandler.ERROR_QUESTION_OK:
                return showMessage(getMessageFor(ex, arg, ix, errorType), errorType);
            case ErrorHandler.ERROR_QUESTION_NO:
                return showMessage(getMessageFor(ex, arg, ix, errorType), errorType);
            case ErrorHandler.ERROR_QUESTION_YES:
                return showMessage(getMessageFor(ex, arg, ix, errorType), errorType);
            case ErrorHandler.ERROR_WARNING:
                return showMessage(getMessageFor(ex, arg, ix, errorType), errorType);
            case ErrorHandler.ERROR_MESSAGE:
                return showMessage(getMessageFor(ex, arg, ix, errorType), errorType);
        }
        throw new IllegalArgumentException("unknown error type");
    }

    /**
     * @deprecated
     * @param message
     * @param errorType
     * @return true if the ok/yes button is clicked, false otherwise.
     */
    protected boolean showMessage(String message, int errorType)
    {
        return showMessage(xmsf, peer, message, errorType);
    }

    /**
     * display a message
     * @deprecated
     * @param xmsf
     * @param message the message to display
     * @param errorType an int constant from the ErrorHandler interface.
     * @return
     */
    public static boolean showMessage(XMultiServiceFactory xmsf, XWindowPeer peer, String message, int errorType)
    {
        String serviceName = getServiceNameFor(errorType);
        int attribute = getAttributeFor(errorType);
        int b = SystemDialog.showMessageBox(xmsf, peer, serviceName, attribute, message);
        return b == getTrueFor(errorType);
    }

    public static boolean showMessage(XMultiServiceFactory xmsf, XWindowPeer peer,
            String message,
            String dialogtype,
            int buttons,
            int defaultButton,
            int returnTrueOn)
    {
        int b = SystemDialog.showMessageBox(xmsf, peer, dialogtype, defaultButton + buttons, message);
        return b == returnTrueOn;
    }

    /**
     * normally ok(1) is the value for true.
     * but a question dialog may use yes. so i use this method
     * for each error type to get its type of "true" value.
     * @param errorType
     * @return
     */
    private static int getTrueFor(int errorType)
    {
        switch (errorType)
        {
            case ErrorHandler.ERROR_FATAL:
            case ErrorHandler.ERROR_PROCESS_FATAL:
            case ErrorHandler.ERROR_NORMAL_ABORT:
            case ErrorHandler.ERROR_NORMAL_IGNORE:
            case ErrorHandler.ERROR_QUESTION_CANCEL:
            case ErrorHandler.ERROR_QUESTION_OK:

                return 1;

            case ErrorHandler.ERROR_QUESTION_NO:
            case ErrorHandler.ERROR_QUESTION_YES:

                return 2;

            case ErrorHandler.ERROR_WARNING:
            case ErrorHandler.ERROR_MESSAGE:

                return 1;
        }
        throw new IllegalArgumentException("unkonown error type");
    }

    /**
     * @param errorType
     * @return the Uno attributes for each error type.
     */
    private static int getAttributeFor(int errorType)
    {
        switch (errorType)
        {
            case ErrorHandler.ERROR_FATAL:
                return VclWindowPeerAttribute.OK;
            case ErrorHandler.ERROR_PROCESS_FATAL:
                return VclWindowPeerAttribute.OK;
            case ErrorHandler.ERROR_NORMAL_ABORT:
                return VclWindowPeerAttribute.OK_CANCEL + VclWindowPeerAttribute.DEF_CANCEL;
            case ErrorHandler.ERROR_NORMAL_IGNORE:
                return VclWindowPeerAttribute.OK_CANCEL + VclWindowPeerAttribute.DEF_OK;
            case ErrorHandler.ERROR_QUESTION_CANCEL:
                return VclWindowPeerAttribute.OK_CANCEL + VclWindowPeerAttribute.DEF_CANCEL;
            case ErrorHandler.ERROR_QUESTION_OK:
                return VclWindowPeerAttribute.OK_CANCEL + VclWindowPeerAttribute.DEF_OK;
            case ErrorHandler.ERROR_QUESTION_NO:
                return VclWindowPeerAttribute.YES_NO + VclWindowPeerAttribute.DEF_NO;
            case ErrorHandler.ERROR_QUESTION_YES:
                return VclWindowPeerAttribute.YES_NO + VclWindowPeerAttribute.DEF_YES;
            case ErrorHandler.ERROR_WARNING:
                return VclWindowPeerAttribute.OK;
            case ErrorHandler.ERROR_MESSAGE:
                return VclWindowPeerAttribute.OK;
        }
        throw new IllegalArgumentException("unkonown error type");
    }

    /**
     * @deprecated
     * @param errorType
     * @return the uno service name for each error type
     */
    private static String getServiceNameFor(int errorType)
    {
        switch (errorType)
        {
            case ErrorHandler.ERROR_FATAL:
                return "errorbox";
            case ErrorHandler.ERROR_PROCESS_FATAL:
                return "errorbox";
            case ErrorHandler.ERROR_NORMAL_ABORT:
                return "errorbox";
            case ErrorHandler.ERROR_NORMAL_IGNORE:
                return "warningbox";
            case ErrorHandler.ERROR_QUESTION_CANCEL:
                return "querybox";
            case ErrorHandler.ERROR_QUESTION_OK:
                return "querybox";
            case ErrorHandler.ERROR_QUESTION_NO:
                return "querybox";
            case ErrorHandler.ERROR_QUESTION_YES:
                return "querybox";
            case ErrorHandler.ERROR_WARNING:
                return "warningbox";
            case ErrorHandler.ERROR_MESSAGE:
                return "infobox";
        }
        throw new IllegalArgumentException("unkonown error type");
    }

    /**
     * renders the error
     * @param ex the exception
     * @param arg a free argument
     * @param ix a free argument
     * @param type the error type (from the int constants
     * in ErrorHandler interface)
     * @return a Strings which will be displayed in the message box,
     * and which describes the error, and the needed action from the user.
     */
    protected abstract String getMessageFor(Exception ex, Object arg, int ix, int type);
}


