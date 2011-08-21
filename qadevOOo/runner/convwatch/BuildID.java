/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package convwatch;

import java.io.File;
import helper.OSHelper;
import convwatch.IniFile;
import java.util.Date;

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

            int dummy = 0;
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

