/*************************************************************************
 *
 *  $RCSfile: AppletMessageHandler.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:54:03 $
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
 *
 ************************************************************************/

package stardiv.app;

import java.util.ResourceBundle;
import java.util.MissingResourceException;
import java.text.MessageFormat;

/**
 * An hanlder of localized messages.
 *
 * @version     1.8, 03/03/97
 * @author      Koji Uno
 */
public class AppletMessageHandler {
    private static ResourceBundle rb;
    private String baseKey = null;

    static {
        try {
            rb = ResourceBundle.getBundle("stardiv.app.MsgAppletViewer");
        } catch (MissingResourceException e) {
            System.out.println(e.getMessage());
        }
    };

    public AppletMessageHandler(String baseKey) {
        this.baseKey = baseKey;
    }

    public String getMessage(String key) {
        return (String)rb.getString(getQualifiedKey(key));
    }

    public String getMessage(String key, Object arg){
        String basemsgfmt = (String)rb.getString(getQualifiedKey(key));
        MessageFormat msgfmt = new MessageFormat(basemsgfmt);
        Object msgobj[] = new Object[1];
        if (arg == null) {
            arg = "null"; // mimic java.io.PrintStream.print(String)
        }
        msgobj[0] = arg;
        return msgfmt.format(msgobj);
    }

    public String getMessage(String key, Object arg1, Object arg2) {
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

    public String getMessage(String key, Object arg1, Object arg2, Object arg3) {
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

    public String getMessage(String key, Object arg[]) {
        String basemsgfmt = (String)rb.getString(getQualifiedKey(key));
        MessageFormat msgfmt = new MessageFormat(basemsgfmt);
        return msgfmt.format(arg);
    }

    public String getQualifiedKey(String subKey) {
        return baseKey + "." + subKey;
    }
}
