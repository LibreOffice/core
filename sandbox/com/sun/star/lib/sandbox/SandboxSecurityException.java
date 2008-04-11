/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SandboxSecurityException.java,v $
 * $Revision: 1.4 $
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

/**
 * An applet security exception.
 *
 * @version     1.6, 11/23/96
 * @author  Arthur van Hoff
 */
public class SandboxSecurityException extends SecurityException {
    private String key = null;
    private Object msgobj[] = null;

    public SandboxSecurityException(String name) {
        super(name);
        this.key = name;
    }

    public SandboxSecurityException(String name, String arg) {
        this(name);
        msgobj = new Object[1];
        msgobj[0] = (Object)arg;
    }

    public SandboxSecurityException(String name, String arg1, String arg2) {
        this(name);
        msgobj = new Object[2];
        msgobj[0] = (Object)arg1;
        msgobj[1] = (Object)arg2;
    }


    //The loading of the static member  amh caused a deadlock:
    //Thread 1 holds a monitor on SandboxSecuritymethod and tries to get hold of
    //a lock of the class loader.
    //Thread 2 is the finalizer which holds the class loader monitor an needs the
    //SandboxSecurity monitor.
    //Therefor the method getLocalizedMessage and the member amh have been removed.
//    private static SandboxMessageHandler amh = new SandboxMessageHandler("appletsecurityexception");

}

