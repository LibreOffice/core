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
package helper;

import java.io.File;
import java.util.Properties;

import lib.TestParameters;
import util.PropertyName;
import util.utils;

/**
 * This class parses commandline Argument and stores <br>
 * them into TestParameter
 */
public class ClParser
{
    /*
     * Parses the commandline argument and puts them<br>
     * into the TestParameters
     */

    public void getCommandLineParameter(TestParameters param, String[] args)
    {
        Properties mapping = getMapping();

        for (int i = 0; i < args.length;)
        {
            String pName = getParameterFor(mapping, args[i]).trim();
            String pValue = "";
            if (pName.equals("TestJob"))
            {
                if (args.length > (i + 1))
                {
                    pValue = args[i].trim() + " " + args[i + 1].trim();
                    i += 2;
                }
                else
                {
                    pValue = args[i].trim() + " unknown";
                    i += 2;
                }
            }
            else
            {
                if ((i + 1) < args.length)
                {
                    pValue = args[i + 1].trim();

                    if (pValue.startsWith("-"))
                    {
                        i++;
                        pValue = "yes";
                    }
                    else if (pValue.startsWith("'"))
                    {
                        i++;
                        while (!pValue.endsWith("'"))
                        {
                            i++;
                            pValue = pValue + " " + args[i].trim();

                        }
                        pValue = pValue.replace("'", "");
                        i++;
                    }
                    else
                    {
                        i += 2;
                    }

                    if (pName.equals("TestDocumentPath"))
                    {
                        System.setProperty(
                            "DOCPTH", new File(pValue).getAbsolutePath());
                    }
                    else if (pName.equals(PropertyName.SRC_ROOT))
                    {
                        System.setProperty(pName, pValue);

                    }
                }
                else
                {
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
    public String getIniPath(String[] args)
    {
        String iniFile = "";

        for (int i = 0; i < args.length; i++)
        {
            if (args[i].equals("-ini"))
            {
                iniFile = args[i + 1];
                break;
            }
        }

        return iniFile;
    }

    /*
     * This method returns the path to a Configuration file <br>
     * if defined as command line parameter, an empty String elsewhere
     */
    public String getRunnerIniPath(String[] args)
    {
        String iniFile = "";

        for (int i = 0; i < args.length; i++)
        {
            if (args[i].equals("-runnerini"))
            {
                iniFile = args[i + 1];
                break;
            }
        }

        return iniFile;
    }

    /*
     * This method maps commandline Parameters to TestParameters
     */
    protected Properties getMapping()
    {
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
        map.setProperty("-nca", "NoCwsAttach");

        return map;
    }

    protected String getParameterFor(Properties map, String name)
    {
        String ret = map.getProperty(name);

        if (ret == null)
        {
            ret = name.substring(1);
        }

        return ret;
    }
}