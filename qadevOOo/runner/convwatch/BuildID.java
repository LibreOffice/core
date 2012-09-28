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

package convwatch;

import java.io.File;
import helper.OSHelper;
import convwatch.IniFile;

public class BuildID
{
    public static String getBuildID(String _sApp)
        {
            String sOfficePath = "";
            if (_sApp.startsWith("\""))
            {
                int nIdx = _sApp.indexOf("\"", 1);
                if (nIdx == -1)
                {
                }
                else
                {
                    // leave double qoute out.
                    sOfficePath = _sApp.substring(1, nIdx);
                }
            }
            else
            {
                // check if a space exist, so we get all until space
                int nIdx = _sApp.indexOf(" ", 1);
                if (nIdx == -1)
                {
                    sOfficePath = _sApp;
                }
                else
                {
                    sOfficePath = _sApp.substring(0, nIdx);
                }
            }
            GlobalLogWriter.get().println("Office path: " + sOfficePath);

            String fs = System.getProperty("file.separator");
            String sBuildID = "";
            File aSOfficeFile = new File(sOfficePath);
            if (aSOfficeFile.exists())
            {
                int nIdx = sOfficePath.lastIndexOf(fs);
                sOfficePath = sOfficePath.substring(0, nIdx);
                // ok. System.out.println("directory: " + sOfficePath);
                sBuildID = getBuildIDFromBootstrap(sOfficePath);
                if (sBuildID.length() == 0)
                {
                    sBuildID = getBuildIDFromVersion(sOfficePath);
                }
            }
            else
            {
                GlobalLogWriter.get().println("soffice executable not found.");
            }

            return sBuildID;
        }

    private static String getBuildIDFromBootstrap(String _sOfficePath)
        {
            String fs = System.getProperty("file.separator");
            String sBuildID = "";
            String sOfficePath = _sOfficePath;
            if (OSHelper.isWindows())
            {
                sOfficePath += fs + "bootstrap.ini";
            }
            else
            {
                sOfficePath += fs + "bootstraprc";
            }
            IniFile aIniFile = new IniFile(sOfficePath);
            if (aIniFile.is())
            {
                sBuildID = aIniFile.getValue("Bootstrap", "buildid");
            }
            else
            {
                GlobalLogWriter.get().println("Property Build, can't open file '" + sOfficePath + "', please check.");
            }
            return sBuildID;
        }

    private static String getBuildIDFromVersion(String _sOfficePath)
        {
            String fs = System.getProperty("file.separator");
            String sBuildID = "";
            String sOfficePath = _sOfficePath;
            if (OSHelper.isWindows())
            {
                sOfficePath += fs + "version.ini";
            }
            else
            {
                sOfficePath += fs + "versionrc";
            }
            IniFile aIniFile = new IniFile(sOfficePath);
            if (aIniFile.is())
            {
                sBuildID = aIniFile.getValue("Version", "buildid");
            }
            else
            {
                GlobalLogWriter.get().println("Property Build, can't open file '" + sOfficePath + "', please check.");
            }
            return sBuildID;
        }
//    public static void main(String[] args)
//        {
//            String sApp;
//            sApp = "/opt/staroffice8_m116/program/soffice --headless --accept=socket,host=localhost,port=8100;urp;";
//            String sBuildID;
//            sBuildID = getBuildID(sApp);
//            System.out.println("BuildID is: " + sBuildID);
//
//            Date aDate = new Date();
//            long nStart = aDate.getTime();
//            System.out.println("Time:" + nStart);
//            // LLA: Just some more tests for getBuildID
//            // sApp = "/opt/staroffice8_net/program/soffice";
//            // sBuildID = getBuildID(sApp);
//            // System.out.println("BuildID is: " + sBuildID);
//            //
//            // sApp = "\"/opt/staroffice8_net/program/soffice\" test blah";
//            // sBuildID = getBuildID(sApp);
//            //
//            // System.out.println("BuildID is: " + sBuildID);
//            System.exit(1);
//        }

}

