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
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.util.Vector;
import java.util.HashMap;
import java.util.Enumeration;
import java.util.StringTokenizer;

public class SVersionRCFile {

    public static final String DEFAULT_NAME =
        System.getProperty("os.name").startsWith("Windows") == true ?
            System.getProperty("user.home") + File.separator +
            "Application Data" + File.separator + "sversion.ini" :
            System.getProperty("user.home") + File.separator +
            ".sversionrc";

    public static final String FILE_URL_PREFIX =
        System.getProperty("os.name").startsWith("Windows") == true ?
            "file:///" : "file://";

    public static final String PKGCHK =
        System.getProperty("os.name").startsWith("Windows") == true ?
            "pkgchk.exe" : "pkgchk";

    private static final String VERSIONS_LINE = "[Versions]";

    private static final String UNOILJAR =
        "skip_registration" + File.separator + "unoil.jar";

    private static final String UNOPACKAGEDIR =
        File.separator + "user" + File.separator + "uno_packages" +
        File.separator + "cache" + File.separator + "uno_packages";

    /* Make sure this is in LowerCase !!!!! */
    private static final String SCRIPTF = "scriptf";

    private static final HashMap<String, SVersionRCFile> files = new HashMap<String, SVersionRCFile>(3);

    private File sversionrc = null;
    private OfficeInstallation defaultversion = null;
    private Vector<OfficeInstallation> versions = null;
    private long lastModified = 0;

    public SVersionRCFile() {
        this(DEFAULT_NAME);
    }

    public SVersionRCFile(String name) {
        sversionrc = new File(name);
        versions = new Vector<OfficeInstallation>(5);
    }

    public static SVersionRCFile createInstance() {
        return(createInstance(DEFAULT_NAME));
    }

    public static SVersionRCFile createInstance(String name) {
        SVersionRCFile result = null;

        synchronized(SVersionRCFile.class) {
            result = files.get(name);

            if (result == null) {
                result = new SVersionRCFile(name);
                files.put(name, result);
            }
        }
        return result;
    }

    public OfficeInstallation getDefaultVersion() throws IOException {
        if (defaultversion == null) {
            getVersions();
        }

        return defaultversion;
    }

    public Enumeration<OfficeInstallation> getVersions() throws IOException {

        long l = sversionrc.lastModified();

        if (l > lastModified) {
            BufferedReader br = null;

            try {
                br = new BufferedReader(new FileReader(sversionrc));
                load(br);
                lastModified = l;
            }
            catch (FileNotFoundException fnfe) {
                throw new IOException(fnfe.getMessage());
            }
            finally {
                if (br != null)
                    br.close();
            }
        }
        return versions.elements();
    }

    private void load(BufferedReader br) throws IOException {
        String s;

        while ((s = br.readLine()) != null &&
              (s.equals(VERSIONS_LINE)) != true);

        while ((s = br.readLine()) != null &&
              (s.equals("")) != true) {
            StringTokenizer tokens = new StringTokenizer(s, "=");
            int count = tokens.countTokens();

            if (count != 2)
                continue;

            String name = tokens.nextToken();
            String path = tokens.nextToken();
            OfficeInstallation oi = new OfficeInstallation(name, path);
            if (oi.supportsFramework()) {
                versions.add(oi);
                defaultversion = oi;
            }
        }
    }

    public static String toFileURL(String path) {
        File f = new File(path);

        if (!f.exists())
            return null;

        try {
            path = f.getCanonicalPath();
        }
        catch (IOException ioe) {
            return null;
        }

        if (System.getProperty("os.name").startsWith("Windows"))
            path = path.replace(File.separatorChar, '/');

        StringBuffer buf = new StringBuffer(FILE_URL_PREFIX);
        buf.append(path);

        if (f.isDirectory())
            buf.append("/");

        return buf.toString();
    }

    public static String getPathForUnoil(String officeInstall)
    {
        File unopkgdir = new File(officeInstall, UNOPACKAGEDIR);
        if(!unopkgdir.exists())
        {
            return null;
        }
        File scriptf = null;
        String[] listunopkg = unopkgdir.list();
        int size = listunopkg.length;
        for(int i=0; i<size; i++)
        {
            if (listunopkg[i].toLowerCase().indexOf(SCRIPTF)>-1)
            {
                scriptf = new File(unopkgdir, listunopkg[i]);
            }
        }
        if(scriptf != null)
        {
            File unoil = new File(scriptf, UNOILJAR);
            if(unoil.exists())
            {
                String path = unoil.getParent();
                path = path.substring(path.indexOf(UNOPACKAGEDIR));
                return officeInstall + path;
            }
        }
        return null;
    }

    public static void main(String[] args) {
        SVersionRCFile ov;

        if (args.length == 0)
            ov = new SVersionRCFile();
        else
            ov = new SVersionRCFile(args[0]);

        Enumeration<OfficeInstallation> enumer;

        try {
            enumer = ov.getVersions();
        }
        catch (IOException ioe) {
            System.err.println("Error getting versions: " + ioe.getMessage());
            return;
        }

        while (enumer.hasMoreElements()) {
            OfficeInstallation oi = enumer.nextElement();
            System.out.println("Name: " + oi.getName() + ", Path: " + oi.getPath() +
                ", URL: " + oi.getURL());
        }
    }
}
