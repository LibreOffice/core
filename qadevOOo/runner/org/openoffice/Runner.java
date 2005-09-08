/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Runner.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:25:04 $
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
public class Runner {

    public static void main(String[] args) {

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

        if (!worked) {
            System.out.println("Job "+param.get("TestJob")+" failed");
            System.exit(-1);
        } else {
            System.out.println("Job "+param.get("TestJob")+" done");
            System.exit(0);
        }
    }
}
