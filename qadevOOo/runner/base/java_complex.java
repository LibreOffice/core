/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: java_complex.java,v $
 * $Revision: 1.13 $
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

package base;

import complexlib.ComplexTestCase;
import util.DynamicClassLoader;
import share.DescGetter;
import stats.OutProducerFactory;
import helper.ComplexDescGetter;
import helper.AppProvider;
import helper.CfgParser;
import share.DescEntry;
import share.LogWriter;
import stats.Summarizer;
import lib.TestParameters;
import util.PropertyName;

/**
 * Test base for executing a java complex test.
 * @see base.TestBase
 */
public class java_complex implements TestBase{

    /**
     * This function executes the complex tests given as parameter "-o" or "TestJob". It querys for the correspond class
     * and crates the JobDescription.
     * @param param
     * @return true of all tests run successfuly, esle false
     */
    public boolean executeTest(TestParameters param) {

        // is there an ini file for the complex tests defined?
        String complexIniFileName = ((String)param.get("ComplexIni"));
        if (complexIniFileName != null) {
            CfgParser ini = new CfgParser(complexIniFileName);
            ini.getIniParameters(param);
        }

        // get the test job
        String testJob = ((String)param.get("TestJob"));

        DescGetter descGetter = new ComplexDescGetter();
        // get the test jobs
        DescEntry[] entries = descGetter.getDescriptionFor(testJob,null,true);
        return executeTest(param, entries);

    }


    /**
     * This function run the given DescEntry[] as ComplexTest
     * @param param
     * @param entries
     * @return true of all tests run successfuly, esle false
     */
    public boolean executeTest(TestParameters param, DescEntry[] entries) {
        // is there an ini file for the complex tests defined?
        String complexIniFileName = ((String)param.get("ComplexIni"));
        if (complexIniFileName != null) {
            CfgParser ini = new CfgParser(complexIniFileName);
            ini.getIniParameters(param);
        }

        DynamicClassLoader dcl = new DynamicClassLoader();
        ComplexTestCase testClass = null;
        boolean returnVal = true;

//        the concept of the TimeOut depends on runner logs. If the runner log,
//        for exmaple to start a test method, the timeout was restet. This is not
//        while the test itself log something like "open docuent...".
//        An property of complex test could be that it have only one test method
//        which works for serveral minutes. Ih this case the TimeOut get not trigger
//        and the office was killed.
//        In complex tests just use "ThreadTimeOut" as timout.

        // param.put("TimeOut", new Integer(0));

        for (int i=0; i<entries.length; i++) {

            if (entries[i] == null) continue;
            String iniName = entries[i].longName;
            iniName = iniName.replace('.', '/');
            CfgParser ini = new CfgParser(iniName+".props");
            ini.getIniParameters(param);

            LogWriter log = (LogWriter)dcl.getInstance(
                                                (String)param.get("LogWriter"));

            AppProvider office = null;
            if (!param.getBool("NoOffice")) {
                try {
                    office = (AppProvider)dcl.getInstance("helper.OfficeProvider");
                    Object msf = office.getManager(param);
                    if (msf == null) {
                        returnVal = false;
                        continue;
                    }
                    param.put("ServiceFactory",msf);
                }
                catch(IllegalArgumentException e) {
                    office = null;
                }
            }
            log.initialize(entries[i],param.getBool(PropertyName.LOGGING_IS_ACTIVE));
            entries[i].Logger = log;

            // create an instance
            try {
                testClass = (ComplexTestCase)dcl.getInstance(entries[i].longName);
            }
            catch(java.lang.Exception e) {
                e.printStackTrace();
                return false;
            }
            testClass.executeMethods(entries[i], param);

            Summarizer sum = new Summarizer();
            sum.summarizeUp(entries[i]);

            if (office != null) {
                office.closeExistingOffice(param, false);
            }

            LogWriter out = OutProducerFactory.createOutProducer(param);

            out.initialize(entries[i], true);
            out.summary(entries[i]);
            returnVal &= entries[i].State.endsWith("OK");
        }
        return returnVal;
    }

}
