/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Runner.java,v $
 * $Revision: 1.5.8.2 $
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

    private static long getStartTime()
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
    private static long meanTime(long _nCurrentTimer)
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
        aTime.append(helper.StringHelper.createValueString((int)hour, 2)).
              append(':').
              append(helper.StringHelper.createValueString((int)min, 2)).
              append(':').
              append(helper.StringHelper.createValueString((int)sec, 2));
        return aTime.toString();
    }

    public static void main(String[] args) {

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

        //parse the commandline arguments
        // TODO: no right error message, if no parameter given!
        cli.getCommandLineParameter(param,args);

        Object tj = param.get("TestJob");

        if (tj==null) {
            System.out.println("==========================================================================");
            System.out.println("No TestJob given, please make sure that you ");
            System.out.println("a.) called the OOoRunner with the paramter -o <job> or -sce <scenarioFile>");
            System.out.println("or");
            System.out.println("b.) have an entry called TestJob in your used properties file");
            System.out.println("==========================================================================");
            System.exit(-1);
        }

        System.out.println("TestJob: "+tj);

        TestBase toExecute = (TestBase) dcl.getInstance("base."+
                                            (String)param.get("TestBase"));

        boolean worked = toExecute.executeTest(param);
        long nTime = meanTime(getStartTime());
        String sBeautifyTime = beautifyTime(nTime);

        System.out.println("Job run took: " + nTime + "ms " + " [" + sBeautifyTime + "]");

        if (!worked)
        {
            System.out.println("Job "+param.get("TestJob")+" failed");
            System.exit(-1);
        }
        else
        {
            System.out.println("Job "+param.get("TestJob")+" done");
            System.exit(0);
        }
    }
}
