/*************************************************************************
 *
 *  $RCSfile: AbstractErrorHandler.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:10:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */

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

public abstract class AbstractErrorHandler implements ErrorHandler {

    XMultiServiceFactory xmsf;
    XWindowPeer peer;

    protected AbstractErrorHandler(XMultiServiceFactory xmsf, XWindowPeer peer_) {
        this.xmsf = xmsf;
        peer=peer_;
    }

    /**
     * Implementation of ErrorHandler:
     * shows a message box with the rendered error.
     * @param arg identifies the error. This object is passed to the render method
     * which returns the right error message.
     * @return true/false for continue/abort.
     */
    public boolean error(Exception ex, Object arg, int ix, int errorType) {
        //ex.printStackTrace();
        switch (errorType) {
            case ErrorHandler.ERROR_FATAL :
                return !showMessage(getMessageFor(ex,arg,ix,errorType) ,errorType);
            case ErrorHandler.ERROR_PROCESS_FATAL :
                return !showMessage(getMessageFor(ex,arg,ix,errorType),errorType );
            case ErrorHandler.ERROR_NORMAL_ABORT :
                return showMessage(getMessageFor(ex,arg,ix,errorType), errorType );
            case ErrorHandler.ERROR_NORMAL_IGNORE :
                return showMessage(getMessageFor(ex,arg,ix,errorType), errorType );
            case ErrorHandler.ERROR_QUESTION_CANCEL :
                return showMessage(getMessageFor(ex,arg,ix,errorType),errorType );
            case ErrorHandler.ERROR_QUESTION_OK :
                return showMessage(getMessageFor(ex,arg,ix,errorType),errorType );
            case ErrorHandler.ERROR_QUESTION_NO:
                return showMessage(getMessageFor(ex,arg,ix,errorType),errorType );
            case ErrorHandler.ERROR_QUESTION_YES :
                return showMessage(getMessageFor(ex,arg,ix,errorType),errorType );
            case ErrorHandler.ERROR_WARNING :
                return showMessage(getMessageFor(ex,arg,ix,errorType),errorType );
            case ErrorHandler.ERROR_MESSAGE :
                return showMessage(getMessageFor(ex,arg,ix,errorType),errorType );
        }
        throw new IllegalArgumentException("unkonown error type");
    }

    /**
     *
     * @param message
     * @param errorType
     * @return true if the ok/yes button is clicked, false otherwise.
     */
    protected boolean showMessage(String message, int errorType) {
        return showMessage(xmsf,peer, message,errorType);
    }

    /**
     * display a message
     * @param xmsf
     * @param message the message to display
     * @param errorType an int constant from the ErrorHandler interface.
     * @return
     */
    public static boolean showMessage(XMultiServiceFactory xmsf, XWindowPeer peer, String message, int errorType) {
        int b = SystemDialog.showMessageBox(xmsf,peer, getServiceNameFor(errorType),
        getAttributeFor(errorType),message);
        return b == getTrueFor(errorType);
    }

    /**
     * normally ok(1) is the value for true.
     * but a question dialog may use yes. so i use this method
     * for each error type to get its type of "true" value.
     * @param errorType
     * @return
     */
    private static int getTrueFor(int errorType) {
        switch (errorType) {
            case ErrorHandler.ERROR_FATAL :
            case ErrorHandler.ERROR_PROCESS_FATAL :
            case ErrorHandler.ERROR_NORMAL_ABORT :
            case ErrorHandler.ERROR_NORMAL_IGNORE :
            case ErrorHandler.ERROR_QUESTION_CANCEL :
            case ErrorHandler.ERROR_QUESTION_OK:

                return 1;

            case ErrorHandler.ERROR_QUESTION_NO :
            case ErrorHandler.ERROR_QUESTION_YES:

                return 2;

            case ErrorHandler.ERROR_WARNING :
            case ErrorHandler.ERROR_MESSAGE :

                return 1;
        }
        throw new IllegalArgumentException("unkonown error type");
    }


    /**
     * @param errorType
     * @return the Uno attributes for each error type.
     */
    private static int getAttributeFor(int errorType) {
        switch (errorType) {
            case ErrorHandler.ERROR_FATAL :
                  return VclWindowPeerAttribute.OK;
            case ErrorHandler.ERROR_PROCESS_FATAL :
                return VclWindowPeerAttribute.OK;
            case ErrorHandler.ERROR_NORMAL_ABORT :
                return VclWindowPeerAttribute.OK_CANCEL + VclWindowPeerAttribute.DEF_CANCEL;
            case ErrorHandler.ERROR_NORMAL_IGNORE :
                return VclWindowPeerAttribute.OK_CANCEL + VclWindowPeerAttribute.DEF_OK;
            case ErrorHandler.ERROR_QUESTION_CANCEL :
                return VclWindowPeerAttribute.OK_CANCEL + VclWindowPeerAttribute.DEF_CANCEL;
            case ErrorHandler.ERROR_QUESTION_OK:
                return VclWindowPeerAttribute.OK_CANCEL + VclWindowPeerAttribute.DEF_OK;
            case ErrorHandler.ERROR_QUESTION_NO :
                return VclWindowPeerAttribute.YES_NO + VclWindowPeerAttribute.DEF_NO;
            case ErrorHandler.ERROR_QUESTION_YES:
                return VclWindowPeerAttribute.YES_NO + VclWindowPeerAttribute.DEF_YES;
            case ErrorHandler.ERROR_WARNING :
                return VclWindowPeerAttribute.OK;
            case ErrorHandler.ERROR_MESSAGE :
                return VclWindowPeerAttribute.OK;
        }
        throw new IllegalArgumentException("unkonown error type");
    }

    /**
     *
     * @param errorType
     * @return the uno service name for each error type
     */
    private static String getServiceNameFor(int errorType) {
        switch (errorType) {
            case ErrorHandler.ERROR_FATAL :
                return "errorbox";
            case ErrorHandler.ERROR_PROCESS_FATAL :
                return "errorbox";
            case ErrorHandler.ERROR_NORMAL_ABORT :
                return "errorbox";
            case ErrorHandler.ERROR_NORMAL_IGNORE :
                return "warningbox";
            case ErrorHandler.ERROR_QUESTION_CANCEL :
                return "querybox";
            case ErrorHandler.ERROR_QUESTION_OK :
                return "querybox";
            case ErrorHandler.ERROR_QUESTION_NO:
                return "querybox";
            case ErrorHandler.ERROR_QUESTION_YES:
                return "querybox";
            case ErrorHandler.ERROR_WARNING :
                return "warningbox";
            case ErrorHandler.ERROR_MESSAGE :
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


