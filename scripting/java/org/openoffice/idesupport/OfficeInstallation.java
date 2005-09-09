/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeInstallation.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:06:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package org.openoffice.idesupport;

import java.io.File;
import java.net.URLDecoder;

public class OfficeInstallation implements java.io.Serializable {

    private String name;
    private String path;
    private String url;
    private boolean hasFW = false;
    private boolean supportsFW = false;

    public static final String FILE_URL_PREFIX = SVersionRCFile.FILE_URL_PREFIX;

    public OfficeInstallation(String path) {
        this(path, path);
    }

    public OfficeInstallation(String name, String path) {

        this.name = name;

        if (path.startsWith(FILE_URL_PREFIX)) {
            this.url = path;
            path = URLDecoder.decode(path);
            path = path.substring(FILE_URL_PREFIX.length());

            if (System.getProperty("os.name").startsWith("Windows"))
                path = path.replace('/', File.separatorChar);

            this.path = path;
        }
        else {
            this.path = path;

            if (System.getProperty("os.name").startsWith("Windows"))
                path = path.replace(File.separatorChar, '/');

            this.url = FILE_URL_PREFIX + path;
        }
    }

    public String getName() {
        return name;
    }

    public String getPath() {
        return path;
    }

    public String getPath(String name) {
        if (!name.startsWith(File.separator))
            name = File.separator + name;

        return path + name;
    }

    public String getURL() {
        return url;
    }

    public String getURL(String name) {
        if (System.getProperty("os.name").startsWith("Windows"))
            name = name.replace(File.separatorChar, '/');

        if (!name.startsWith("/"))
            name = "/" + name;

        return url + name;
    }

    public boolean hasFramework() {
        return hasFW;
    }

    public boolean supportsFramework() {
        return true;
    }

    public String toString() {
        return getName();
    }
}
