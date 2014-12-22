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
import java.util.HashMap;

import lib.TestParameters;
import util.PropertyName;

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
        for (int i = 0; i < args.length;)
        {
            String pName = getParameterFor(args[i]).trim();
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

    /**
     * Map command-line Parameters to TestParameters
     */
    private static final java.util.Map<String,String> COMMAND_LINE_OPTION_TO_TEST_PARAMETER = new HashMap<String,String>();
    static {
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-cs", "ConnectionString");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-tb", "TestBase");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-tdoc", "TestDocumentPath");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-objdsc", "DescriptionPath");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-cmd", "AppExecutionCommand");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-o", "TestJob");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-sce", "TestJob");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-p", "TestJob");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-aca", "AdditionalConnectionArguments");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-xcl", "ExclusionList");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-debug", "DebugIsActive");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-log", "LoggingIsActive");
        COMMAND_LINE_OPTION_TO_TEST_PARAMETER.put("-dbout", "DataBaseOut");
    }

    private String getParameterFor(String name)
    {
        String ret = COMMAND_LINE_OPTION_TO_TEST_PARAMETER.get(name);

        if (ret == null)
        {
            ret = name.substring(1);
        }

        return ret;
    }
}