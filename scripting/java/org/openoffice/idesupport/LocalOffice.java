/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.idesupport;

import java.io.File;
import java.net.ConnectException;
import java.util.Vector;

/**
 * LocalOffice represents a connection to the local office.
 *
 * This class allows to get access to some scripting framework
 * releated functionality of the locally running office. The
 * office has to be started with options appropriate for establishing
 * local connection.
 */
public class LocalOffice
{
    /**
     * Creates an instance of the local office connection.
     *
     * @param parent is an application specific class loader.
     * @param officePath is a platform specific path string
     *   to the office distribution.
     * @param port is a communication port.
     */
    public static final LocalOffice create(
        ClassLoader parent, String officePath, int port)
    {
        Vector<String>      path    = new Vector<String>();
        path.addElement(officePath + "/program/classes/ridl.jar");
        path.addElement(officePath + "/program/classes/jurt.jar");
        path.addElement(officePath + "/program/classes/unoil.jar");
        path.addElement(officePath + "/program/classes/juh.jar");
        path.addElement(System.getProperties().getProperty("netbeans.home") +
            File.separator + "modules" +
            File.separator + "ext" +
            File.separator + "localoffice.jar");
        // commented out so code will compile
        // ClassLoader appcl   = new DefaultScriptClassLoader(parent, path);
        ClassLoader appcl = path.getClass().getClassLoader();
        Class       clazz   = null;
        LocalOffice office  = null;
        try {
            clazz   = appcl.loadClass(
                "org.openoffice.idesupport.localoffice.LocalOfficeImpl");
            office  = (LocalOffice)clazz.newInstance();
            office.connect(officePath, port);
        } catch (java.lang.Exception exp) {
            office  = null;
        }
        return office;
    }

    /**
     * Connects to the running office.
     *
     * @param officePath is a platform specific path string
     *   to the office distribution.
     * @param port is a communication port.
     */
    protected void connect(String officePath, int port)
        throws ConnectException
    {
    }

    /**
     * Closes the connection to the running office.
     */
    public void disconnect()
    {
    }

    /**
     * Refresh the script storage.
     *
     * @param uri is an identifier of storage has to be refreshed.
     */
    public void refreshStorage(String uri)
    {
    }
}
