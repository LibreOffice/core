/*************************************************************************
 *
 *  $RCSfile: LocalOffice.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-09-10 10:45:50 $
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
 *
 * @author misha <misha@openoffice.org>
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
        Vector      path    = new Vector();
        path.addElement(officePath + "/program/classes/ridl.jar");
        path.addElement(officePath + "/program/classes/jurt.jar");
        path.addElement(officePath + "/program/classes/sandbox.jar");
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
