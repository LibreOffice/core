/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ErrorHandler.java,v $
 * $Revision: 1.6 $
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

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public interface ErrorHandler
{

    public static final String MESSAGE_INFO = "infobox";
    public static final String MESSAGE_QUESTION = "querybox";
    public static final String MESSAGE_ERROR = "errorbox";
    public static final String MESSAGE_WARNING = "warningbox";
    public static int BUTTONS_OK = VclWindowPeerAttribute.OK;
    public static int BUTTONS_OK_CANCEL = VclWindowPeerAttribute.OK_CANCEL;
    public static int BUTTONS_YES_NO = VclWindowPeerAttribute.YES_NO;
    public static int RESULT_CANCEL = 0;
    public static int RESULT_OK = 1;
    public static int RESULT_YES = 2;
    public static int DEF_OK = VclWindowPeerAttribute.DEF_OK;
    public static int DEF_CANCEL = VclWindowPeerAttribute.DEF_CANCEL;
    public static int DEF_YES = VclWindowPeerAttribute.DEF_YES;
    public static int DEF_NO = VclWindowPeerAttribute.DEF_NO;
    /**
     * Error type for fatal errors which should abort application
     * execution. Should actually never be used :-)
     */
    public static final int ERROR_FATAL = 0;
    /**
     * An Error type for errors which should stop the current process.
     */
    public static final int ERROR_PROCESS_FATAL = 1;
    /**
     * An Error type for errors to which the user can choose, whether
     * to continue or to abort the current process.
     * default is abort.
     */
    public static final int ERROR_NORMAL_ABORT = 2;
    /**
     * An Error type for errors to which the user can choose, whether
     * to continue or to abort the current process.
     * default is continue.
     */
    public static final int ERROR_NORMAL_IGNORE = 3;
    /**
     * An error type for warnings which requires user interaction.
     * (a question :-) )
     * Default is abort (cancel).
     */
    public static final int ERROR_QUESTION_CANCEL = 4;
    /**
     * An error type for warnings which requires user interaction
     * (a question :-) )
     * Default is to continue (ok).
     */
    public static final int ERROR_QUESTION_OK = 5;
    /**
     * An error type for warnings which requires user interaction.
     * (a question :-) )
     * Default is abort (No).
     */
    public static final int ERROR_QUESTION_NO = 6;
    /**
     * An error type for warnings which requires user interaction
     * (a question :-) )
     * Default is to continue (Yes).
     */
    public static final int ERROR_QUESTION_YES = 7;
    /**
     * An error type which is just a warning...
     */
    public static final int ERROR_WARNING = 8;
    /**
     * An error type which just tells the user something
     * ( like "you look tired! you should take a bath! and so on)
     */
    public static final int ERROR_MESSAGE = 9;

    /**
     * @param ex the exception that accured
     * @param arg an object as help for recognizing the exception
     * @param ix an integer which helps for detailed recognizing of the exception
     * @param errorType one of the int constants defined by this Interface
     * @return true if the execution should continue, false if it should stop.
     */
    public boolean error(Exception ex, Object arg, int ix, int errorType);
}
