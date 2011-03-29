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

package graphical;

import java.io.File;
import helper.OSHelper;

public class BuildID
{
    private static String getOfficePath(String _sApp)
    {
        String sOfficePath = "";
        // TODO: StringHelper.removeQuote?
        if (_sApp.startsWith("\""))
        {
            int nIdx = _sApp.indexOf("\"", 1);
            if (nIdx != -1)
            {
                // leave double qoute out.
                sOfficePath = _sApp.substring(1, nIdx);
            }
        }
        else
        {
            // check if _sApp ends with the office executable, if not
            if (! (_sApp.endsWith("soffice.exe") || _sApp.endsWith("soffice")))
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
            else
            {
                sOfficePath = _sApp;
            }
        }
        // GlobalLogWriter.get().println("Office path: " + sOfficePath);
        return sOfficePath;
    }

    public static String getBuildID(String _sApp)
        {
            final String sOfficePath = getOfficePath(_sApp);
            final String sBuildID = getBuildID(sOfficePath, "buildid");
            return sBuildID;
    }

    private static String getBuildID(String _sOfficePath, String _sIniSection)
        {
            File aSOfficeFile = new File(_sOfficePath);
            String sBuildID = "";
            if (aSOfficeFile.exists())
            {
                String sOfficePath = FileHelper.getPath(_sOfficePath);
                // ok. System.out.println("directory: " + sOfficePath);
                sBuildID = getBuildIDFromBootstrap(sOfficePath, _sIniSection);
                if (sBuildID.length() == 0)
                {
                    sBuildID = getBuildIDFromVersion(sOfficePath, _sIniSection);
                }
            }
            else
            {
                GlobalLogWriter.println("soffice executable not found.");
            }

//            int dummy = 0;
            return sBuildID;
        }

    private static String getBuildIDFromBootstrap(String _sOfficePath, String _sIniSection)
        {
            String sBuildID = "";
            String sOfficePath;
            if (OSHelper.isWindows())
            {
                sOfficePath = FileHelper.appendPath(_sOfficePath, "bootstrap.ini");
            }
            else
            {
                sOfficePath = FileHelper.appendPath(_sOfficePath, "bootstraprc");
            }
            IniFile aIniFile = new IniFile(sOfficePath);
            if (aIniFile.is())
            {
                sBuildID = aIniFile.getValue("Bootstrap", /*"buildid"*/ _sIniSection);
            }
            else
            {
                GlobalLogWriter.println("Property Build, can't open file '" + sOfficePath + "', please check.");
            }
            return sBuildID;
        }

    private static String getBuildIDFromVersion(String _sOfficePath, String _sIniSection)
        {
            // String fs = System.getProperty("file.separator");
            String sBuildID = "";
            String sOfficePath;
            if (OSHelper.isWindows())
            {
                sOfficePath = FileHelper.appendPath(_sOfficePath, "version.ini");
            }
            else
            {
                sOfficePath = FileHelper.appendPath(_sOfficePath, "versionrc");
            }
            IniFile aIniFile = new IniFile(sOfficePath);
            if (aIniFile.is())
            {
                sBuildID = aIniFile.getValue("Version", /*"buildid"*/ _sIniSection);
            }
            else
            {
                GlobalLogWriter.println("Property Build, can't open file '" + sOfficePath + "', please check.");
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

            public static String getMaster(String _sOfficePath)
            {
                final String sOfficePath = getOfficePath(_sOfficePath);
                final String sMaster = getBuildID(sOfficePath, "ProductSource");
                return sMaster;
            }

            public static String getMinor(String _sOfficePath)
            {
                final String sOfficePath = getOfficePath(_sOfficePath);
                final String sMinor = "m" + getBuildID(sOfficePath, "ProductMinor");
                return sMinor;
            }

            public static String getCWSName(String _sOfficePath)
            {
                final String sOfficePath = getOfficePath(_sOfficePath);
                final String sBuildID = getBuildID(sOfficePath, "buildid");
                String sCWSName = "MWS";
                int nIdx = sBuildID.indexOf("[CWS:");
                if (nIdx > 0)
                {
                    int nIdx2 = sBuildID.indexOf("]", nIdx);
                    sCWSName = sBuildID.substring(nIdx + 5, nIdx2);
                }
                return sCWSName;
            }

//    public static void main(String[] args)
//        {
//            String sApp;
//            sApp = "D:/staroffice9_m63/Sun/StarOffice 9/program/soffice.exe";
//            String sBuildID;
//            sBuildID = getBuildID(sApp);
//            System.out.println("BuildID is: " + sBuildID);
//
//            String sMaster;
//            sMaster = getMaster(sApp);
//            System.out.println("Master is: " + sMaster);
//
//            String sMinor;
//            sMinor = getMinor(sApp);
//            System.out.println("Minor is: " + sMinor);
//
//            String sCWSName;
//            sCWSName = getCWSName(sApp);
//            System.out.println("CWSName is: " + sCWSName);
//
//            System.exit(1);
//        }

}
