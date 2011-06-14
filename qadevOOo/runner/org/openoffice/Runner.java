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
package org.openoffice;

import java.util.Enumeration;
import java.util.Properties;
import java.util.StringTokenizer;
import lib.TestParameters;
import util.DynamicClassLoader;
import base.TestBase;
import helper.ClParser;
import helper.CfgParser;

/**
 * The main class, will call ClParser and CfgParser to <br>
 * fill the TestParameters.<br>
 * Will then call the appropriate Testbase to run the tests.
 */
public class Runner
{

    private static long m_nStartTime;

    public static long getRunnerStartTime()
    {
        return m_nStartTime;
    }
    /*
    simple helper functions to start/stop a timer, to know how long a process need in milliseconds
     */

    private static long getTime()
    {
        return System.currentTimeMillis();
    }

    private static void setStartTime(long _nStartTime)
    {
        m_nStartTime = _nStartTime;
    }

    /*
    return the time, which is done until last startTime()
     */
    public static long meanTime(long _nCurrentTimer)
    {
        if (_nCurrentTimer == 0)
        {
            System.out.println("Forgotten to initialise a start timer?");
            return 0;
        }
        long nMeanTime = getTime();
        return nMeanTime - _nCurrentTimer;
    }

    private static String beautifyTime(long _nTime)
    {
        long sec = (_nTime / 1000) % 60;
        long min = (_nTime / (60 * 1000)) % 60;
        long hour = _nTime / (60 * 60 * 1000);
        StringBuffer aTime = new StringBuffer();
        aTime.append(helper.StringHelper.createValueString((int) hour, 2)).
                append(':').
                append(helper.StringHelper.createValueString((int) min, 2)).
                append(':').
                append(helper.StringHelper.createValueString((int) sec, 2));
        return aTime.toString();
    }

    /**
     Helper to check if there are problems with Cygwin Path variables.
     */
    private static boolean checkVariableForCygwin(String _sVariable)
    {
        if (_sVariable == null)
        {
            return false;
        }
        if (_sVariable.startsWith("/cygdrive"))
        {
            return true;
        }
        return false;
    }

    private static boolean checkPathVariable(String _sPath, String delim)
    {
        String sPath = System.getProperty(_sPath);
        if (sPath != null)
        {
            StringTokenizer aTokenEnum = new StringTokenizer(sPath, delim);
            while (aTokenEnum.hasMoreElements())
            {
                String sToken = (String) aTokenEnum.nextElement();
                if (checkVariableForCygwin(sToken))
                {
                    System.err.println("ERROR: OOoRunner detect cygwin path in '" + _sPath + "'");
                    return true;
                }
            }
        }
        return false;
    }

    private static void checkAllVariablesForCygwinPath(TestParameters _aParams)
    {
        // ----- check all System.getProperty(key) variables -----
        String sOsName = System.getProperty("os.name");
        if (!sOsName.toLowerCase().startsWith("windows"))
        {
            // we need to check only on windows
            return;
        }

        Properties aProps = System.getProperties();
        Enumeration aEnum = aProps.propertyNames();
        // Enumeration aEnum = aProps.elements();        // these are only the values
        boolean bEmergencyStop = false;

        while (aEnum.hasMoreElements())
        {
            String sKey = (String) aEnum.nextElement();
            String sValue = System.getProperty(sKey);

            if (checkVariableForCygwin(sValue))
            {
                System.err.println("ERROR: OOoRunner detect cygwin path in '" + sKey + ":=" + sValue + "'");
                bEmergencyStop = true;
            }
        }

        // ----- check path variables separatly -----
        String sDelim = System.getProperty("path.separator");
        bEmergencyStop |= checkPathVariable("java.library.path", sDelim);
        bEmergencyStop |= checkPathVariable("java.class.path", sDelim);
        bEmergencyStop |= checkPathVariable("sun.boot.class.path", sDelim);

        // ----- check all TestParameters -----
        aEnum = _aParams.keys();
        while (aEnum.hasMoreElements())
        {
            String sKey = (String) aEnum.nextElement();
            if (_aParams.get(sKey) instanceof String)
            {
                String sValue = (String) _aParams.get(sKey);

                if (checkVariableForCygwin(sValue))
                {
                    System.err.println("ERROR: OOoRunner detect cygwin path in '" + sKey + ":=" + sValue + "'");
                    bEmergencyStop = true;
                }
            }
        }

        if (bEmergencyStop)
        {
            System.exit(-1);
        }
    }

    public static boolean run(String... args)
    {
        System.out.println("OOoRunner Main() version from 20101118 (yyyymmdd)");

        setStartTime(getTime());

        DynamicClassLoader dcl = new DynamicClassLoader();

        // get a class for test parameters
        TestParameters param = new TestParameters();

        ClParser cli = new ClParser();

        //parse the commandline arguments if an ini-parameter is given
        String iniFile = cli.getIniPath(args);

        //initialize cfgParser with ini-path
        CfgParser ini = new CfgParser(iniFile);

        //parse ConfigFile
        ini.getIniParameters(param);


        //parse the commandline arguments if an runnerprops-parameter is given
        String runnerIniFile = cli.getRunnerIniPath(args);

        //initialize cfgParser with ini-path
        CfgParser runnerIni = new CfgParser(runnerIniFile);

        //parse ConfigFile
        runnerIni.getIniParameters(param);

        //parse the commandline arguments
        // TODO: no right error message, if no parameter given!
        cli.getCommandLineParameter(param, args);

        Object tj = param.get("TestJob");

        if (tj == null)
        {
            System.out.println("==========================================================================");
            System.out.println("No TestJob given, please make sure that you ");
            System.out.println("a.) called the OOoRunner with the paramter -o <job> or -sce <scenarioFile>");
            System.out.println("or");
            System.out.println("b.) have an entry called TestJob in your used properties file");
            System.out.println("==========================================================================");
            System.exit(-1);
        }

        System.out.println("TestJob: " + tj);
        String sName = "base." + (String) param.get("TestBase");
        TestBase toExecute = (TestBase) dcl.getInstance(sName);

        checkAllVariablesForCygwinPath(param);

        boolean worked = toExecute.executeTest(param);
        long nTime = meanTime(getRunnerStartTime());
        String sBeautifyTime = beautifyTime(nTime);

        System.out.println("Job run took: " + nTime + "ms " + " [" + sBeautifyTime + "]");

        if (!worked)
        {
            System.out.println("Job " + param.get("TestJob") + " failed");
        }
        else
        {
            System.out.println("Job " + param.get("TestJob") + " done");
        }
        return worked;
    }

    public static void main(String[] args)
    {
        System.exit(run(args) ? 0 : -1);
    }
}
