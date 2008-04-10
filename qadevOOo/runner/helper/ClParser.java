/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ClParser.java,v $
 * $Revision: 1.11 $
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
package helper;

import java.util.Properties;

import lib.TestParameters;


/**
 * This class parses commandline Argument and stores <br>
 * them into TestParameter
 */
public class ClParser {
    /*
     * Parses the commandline argument and puts them<br>
     * into the TestParameters
     */
    public void getCommandLineParameter(TestParameters param, String[] args) {
        Properties mapping = getMapping();
        boolean isTestJob = false;

        for (int i = 0; i < args.length;) {
            String pName = getParameterFor(mapping, args[i]).trim();
            String pValue = "";
            if (pName.equals("TestJob")) {
                if (args.length > (i + 1)) {
                    pValue = args[i].trim() + " " + args[i + 1].trim();
                    i += 2;
                } else {
                    pValue = args[i].trim() + " unknown";
                    i += 2;
                }
            } else {
                if ((i + 1) < args.length) {
                    pValue = args[i + 1].trim();

                    if (pValue.startsWith("-")) {
                        i++;
                        pValue = "yes";
                    } else {
                        i += 2;
                    }

                    if (pName.equals("TestDocumentPath")) {
                        System.setProperty("DOCPTH", pValue);
                    }
                } else {
                    pValue = "yes";
                    i++;
                }
            }

            param.put(pName, pValue);
        }
    }

    /*
     * This method returns the path to a Configuration file <br>
     * if defined as command line parameter, an empty String elsewhere
     */
    public String getIniPath(String[] args) {
        String iniFile = "";

        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-ini")) {
                iniFile = args[i + 1];
            }
        }

        return iniFile;
    }

    /*
     * This method maps commandline Parameters to TestParameters
     */
    protected Properties getMapping() {
        Properties map = new Properties();
        map.setProperty("-cs", "ConnectionString");
        map.setProperty("-tb", "TestBase");
        map.setProperty("-tdoc", "TestDocumentPath");
        map.setProperty("-objdsc", "DescriptionPath");
        map.setProperty("-cmd", "AppExecutionCommand");
        map.setProperty("-o", "TestJob");
        map.setProperty("-sce", "TestJob");
        map.setProperty("-p", "TestJob");
        map.setProperty("-aca", "AdditionalConnectionArguments");
        map.setProperty("-xcl", "ExclusionList");
        map.setProperty("-debug", "DebugIsActive");
        map.setProperty("-log", "LoggingIsActive");
        map.setProperty("-dbout", "DataBaseOut");

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