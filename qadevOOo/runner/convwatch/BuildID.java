/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BuildID.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 17:40:44 $
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

package convwatch;

import java.io.File;
import convwatch.OSHelper;
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
            System.out.println("Office path: " + sOfficePath);

            String fs = System.getProperty("file.separator");
            String sBuildID = "";
            File aSOfficeFile = new File(sOfficePath);
            if (aSOfficeFile.exists())
            {
                int nIdx = sOfficePath.lastIndexOf(fs);
                sOfficePath = sOfficePath.substring(0, nIdx);
                // ok. System.out.println("directory: " + sOfficePath);
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
                    sBuildID = aIniFile.getKey("Bootstrap", "buildid");
                }
            }
            else
            {
                System.out.println("soffice executable not found.");
            }

            int dummy = 0;
            return sBuildID;
        }

    public static void main(String[] args)
        {
            String sApp;
            sApp = "/opt/staroffice8_m116/program/soffice -headless -accept=socket,host=localhost,port=8100;urp;";
            String sBuildID;
            sBuildID = getBuildID(sApp);
            System.out.println("BuildID is: " + sBuildID);

            Date aDate = new Date();
            long nStart = aDate.getTime();
            System.out.println("Time:" + nStart);
            // LLA: Just some more tests for getBuildID
            // sApp = "/opt/staroffice8_net/program/soffice";
            // sBuildID = getBuildID(sApp);
            // System.out.println("BuildID is: " + sBuildID);
            //
            // sApp = "\"/opt/staroffice8_net/program/soffice\" test blah";
            // sBuildID = getBuildID(sApp);
            //
            // System.out.println("BuildID is: " + sBuildID);
            System.exit(1);
        }

}

