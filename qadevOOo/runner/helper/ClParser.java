/*************************************************************************
 *
 *  $RCSfile: ClParser.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change:$Date: 2003-10-06 12:38:46 $
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

package helper;

import lib.TestParameters;
import java.util.Properties;
/**
 * This class parses commandline Argument and stores <br>
 * them into TestParameter
 */
public class ClParser {

    /*
     * Parses the commandline argument and puts them<br>
     * into the TestParameters
     */
    public void getCommandLineParameter(TestParameters param,String[] args) {
        Properties mapping = getMapping();
        boolean isTestJob = false;
        for (int i=0; i<args.length;) {
            String pName = getParameterFor(mapping,args[i]).trim();
            String pValue = "";
            if (pName.equals("TestJob")) {
                pValue = args[i].trim()+" "+args[i+1].trim();
                i+=2;
            }
            else{
                if (i+1<args.length) {
                    pValue = args[i+1].trim();
                    if (pValue.startsWith("-")) {
                        i++;
                        pValue="yes";
                    }
                    else {
                        i+=2;
                    }
                    if (pName.equals("TestDocumentPath")) {
                        System.setProperty("DOCPTH",pValue);
                    }
                }
                else {
                    pValue="yes";
                    i++;
                }
            }
            param.put(pName,pValue);
        }
    }

    /*
     * This method returns the path to a Configuration file <br>
     * if defined as command line parameter, an empty String elsewhere
     */

    public String getIniPath(String[] args) {
        String iniFile="";
        for (int i=0;i<args.length;i++) {
            if (args[i].equals("-ini")) {
                iniFile=args[i+1];
            }
        }
        return iniFile;
    }

    /*
     * This method maps commandline Parameters to TestParameters
     */

    protected Properties getMapping() {
        Properties map = new Properties();
        map.setProperty("-cs","ConnectionString");
        map.setProperty("-tb","TestBase");
        map.setProperty("-tdoc","TestDocumentPath");
        map.setProperty("-objdsc","DescriptionPath");
        map.setProperty("-cmd","AppExecutionCommand");
        map.setProperty("-o","TestJob");
        map.setProperty("-sce","TestJob");
        map.setProperty("-aca", "AdditionalConnectionArguments");
        return map;
    }

    protected String getParameterFor(Properties map, String name) {
        String ret = map.getProperty(name);
        if (ret == null) {
            ret = name.substring(1);
        }
        return ret;
    }
}
