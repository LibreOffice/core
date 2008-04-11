/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeInstallation.java,v $
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
