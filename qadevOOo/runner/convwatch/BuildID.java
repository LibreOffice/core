/*************************************************************************
 *
 *  $RCSfile: BuildID.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2004-11-02 11:07:58 $
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

package convwatch;

import java.io.File;
import convwatch.OSHelper;
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
            sApp = "/opt/staroffice8_net/program/soffice -headless -accept=socket,host=localhost,port=8100;urp;";
            String sBuildID;
            sBuildID = getBuildID(sApp);
            System.out.println("BuildID is: " + sBuildID);

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

