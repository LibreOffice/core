/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SandboxMessageHandler.java,v $
 * $Revision: 1.3 $
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

package com.sun.star.lib.sandbox;

import java.util.ResourceBundle;
import java.util.MissingResourceException;
import java.text.MessageFormat;

/**
 * An hanlder of localized messages.
 *
 * @version     1.8, 03/03/97
 * @author      Koji Uno
 */
class SandboxMessageHandler {
    private static ResourceBundle rb;
    private String baseKey = null;

    static {
        try {
            rb = ResourceBundle.getBundle("stardiv.security.resources.MsgAppletViewer");
        } catch (MissingResourceException e)
        {
            System.out.println(e.getMessage());
        }
    };

    SandboxMessageHandler(String baseKey) {
        this.baseKey = baseKey;
    }

    String getMessage(String key) {
        return (String)rb.getString(getQualifiedKey(key));
    }

    String getMessage(String key, Object arg){
        String basemsgfmt = (String)rb.getString(getQualifiedKey(key));
        MessageFormat msgfmt = new MessageFormat(basemsgfmt);
        Object msgobj[] = new Object[1];
    if (arg == null) {
        arg = "null"; // mimic java.io.PrintStream.print(String)
    }
    msgobj[0] = arg;
    return msgfmt.format(msgobj);
    }

    String getMessage(String key, Object arg1, Object arg2) {
        String basemsgfmt = (String)rb.getString(getQualifiedKey(key));
        MessageFormat msgfmt = new MessageFormat(basemsgfmt);
        Object msgobj[] = new Object[2];
    if (arg1 == null) {
        arg1 = "null";
    }
    if (arg2 == null) {
        arg2 = "null";
    }
    msgobj[0] = arg1;
    msgobj[1] = arg2;
    return msgfmt.format(msgobj);
    }

    String getMessage(String key, Object arg1, Object arg2, Object arg3) {
        String basemsgfmt = (String)rb.getString(getQualifiedKey(key));
        MessageFormat msgfmt = new MessageFormat(basemsgfmt);
        Object msgobj[] = new Object[3];
    if (arg1 == null) {
        arg1 = "null";
    }
    if (arg2 == null) {
        arg2 = "null";
    }
    if (arg3 == null) {
        arg3 = "null";
    }
    msgobj[0] = arg1;
    msgobj[1] = arg2;
    msgobj[2] = arg3;
    return msgfmt.format(msgobj);
    }

    String getMessage(String key, Object arg[]) {
        String basemsgfmt = (String)rb.getString(getQualifiedKey(key));
        MessageFormat msgfmt = new MessageFormat(basemsgfmt);
        return msgfmt.format(arg);
    }

    String getQualifiedKey(String subKey) {
        return baseKey + "." + subKey;
    }
}

