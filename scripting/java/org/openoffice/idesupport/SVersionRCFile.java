/*************************************************************************
 *
 *  $RCSfile: SVersionRCFile.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: toconnor $ $Date: 2002-11-26 16:57:59 $
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

import java.io.*;
import java.util.*;

public class SVersionRCFile {

    public static final String DEFAULT_NAME =
        System.getProperty("os.name").startsWith("Windows") == true ?
            System.getProperty("user.home") + File.separator +
            "Application Data" + File.separator + "sversion.ini" :
            System.getProperty("user.home") + File.separator +
            ".sversionrc";

    private static final String FILE_URL_PREFIX =
        System.getProperty("os.name").startsWith("Windows") == true ?
            "file:///" : "file://";

    private static final String VERSIONS_LINE = "[Versions]";
    private static final String UNOILJAR = "skip_registration" + File.separator + "unoil.jar";
    private static final String UNOPACKAGEDIR = File.separator + "user" +
                                                File.separator + "uno_packages" +
                                                File.separator + "cache" +
                                                File.separator + "uno_packages";
    /* Make sure this is in LowerCase !!!!! */
    private static final String SCRIPTF = "scriptf";
    private File file = null;

    public SVersionRCFile() {
        this(DEFAULT_NAME);
    }

    public SVersionRCFile(String name) {
        file = new File(name);
    }

    public Hashtable getVersions() throws IOException {
        BufferedReader br;

        try {
            br = new BufferedReader(new FileReader(file));
        }
        catch (FileNotFoundException fnfe) {
            throw new IOException(fnfe.getMessage());
        }

        Hashtable versions = load(br);
        br.close();
        return versions;
    }

    private Hashtable load(BufferedReader br) throws IOException {
        Hashtable versions = new Hashtable();
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
            if (path.startsWith(FILE_URL_PREFIX) == true) {
                path = java.net.URLDecoder.decode(path);

                path = path.substring(FILE_URL_PREFIX.length());

                if (System.getProperty("os.name").startsWith("Windows"))
                    path = path.replace('/', File.separatorChar);
            }

            if (isValidPath(path) == true)
                versions.put(name, path);
        }
        return versions;
    }

    private boolean isValidPath(String path) {
        // System.out.println("Testing: " + path);
        File file = new File(path + File.separator + "program");
        if (file.exists())
            return true;
        return false;
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

    /* public static void main(String[] args) {
        SVersionRCFile ov;
        Hashtable versions;

        if (args.length == 0)
            ov = new SVersionRCFile();
        else
            ov = new SVersionRCFile(args[0]);

        try {
            versions = ov.getVersions();
        }
        catch (IOException ioe) {
            System.err.println("Error getting versions: " + ioe.getMessage());
            return;
        }

        Enumeration enum = versions.keys();

        while (enum.hasMoreElements()) {
            String name = (String)enum.nextElement();
            System.out.println("Name: " + name + ", Path: " +
                (String)versions.get(name));
        }
    } */
}
