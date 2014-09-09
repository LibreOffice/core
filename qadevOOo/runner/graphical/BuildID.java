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
            int nIdx = _sApp.indexOf('"', 1);
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
                int nIdx = _sApp.indexOf(' ', 1);
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





}
