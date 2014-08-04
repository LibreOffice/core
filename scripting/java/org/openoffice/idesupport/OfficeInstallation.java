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
import java.net.URLDecoder;

public class OfficeInstallation implements java.io.Serializable {

    private String name;
    private String path;
    private String url;
    private boolean hasFW = false;

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
