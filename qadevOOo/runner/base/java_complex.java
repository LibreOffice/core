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
import java.io.UnsupportedEncodingException;

/**
 * Test base for executing a java complex test.
 * @see base.TestBase
 */
public class java_complex implements TestBase
{

    /**
     * This function executes the complex tests given as parameter "-o" or "TestJob". It queries for the correspond class
     * and crates the JobDescription.
     * @return true of all tests run successfully, else false
     */
    public boolean executeTest(TestParameters param)
    {
        // get the test job
        String testJob = ((String) param.get("TestJob"));

        DescGetter descGetter = new ComplexDescGetter();
        // get the test jobs
        DescEntry[] entries = descGetter.getDescriptionFor(testJob, null, true);

        if (entries == null) {
            System.out.println("Couldn't get Description for Job: " + testJob);

            return false;
        }

        return executeTest(param, entries);
    }

    /**
     * This function run the given DescEntry[] as ComplexTest
     * @return true of all tests run successfully, else false
     */
    public boolean executeTest(TestParameters param, DescEntry[] entries)
    {
        DynamicClassLoader dcl = new DynamicClassLoader();
        ComplexTestCase testClass = null;
        boolean returnVal = true;

//        the concept of the TimeOut depends on runner logs. If the runner log,
//        for example to start a test method, the timeout was reset. This is not
//        while the test itself log something like "open document...".
//        A property of complex test could be that it has only one test method
//        which works for several minutes. In this case the TimeOut get not trigger
//        and the office was killed.
//        In complex tests just use "ThreadTimeOut" as timeout.

        for (int i = 0; i < entries.length; i++)
        {

            if (entries[i] == null)
            {
                continue;
            }
            String iniName = entries[i].longName;
            iniName = iniName.replace('.', '/');
            CfgParser ini = new CfgParser(iniName + ".props");
            ini.getIniParameters(param);

            LogWriter log = (LogWriter) dcl.getInstance((String) param.get("LogWriter"));

            AppProvider office = null;
            if (!param.getBool("NoOffice"))
            {
                try
                {
                    office = (AppProvider) dcl.getInstance("helper.OfficeProvider");
                    Object msf = office.getManager(param);
                    if (msf == null)
                    {
                        returnVal = false;
                        continue;
                    }
                    param.put("ServiceFactory", msf);
                }
                catch (IllegalArgumentException e)
                {
                    office = null;
                }
                catch (UnsupportedEncodingException e)
                {
                    office = null;
                }
            }
            log.initialize(entries[i], param.getBool(PropertyName.LOGGING_IS_ACTIVE));
            entries[i].Logger = log;

            // create an instance
            try
            {
                testClass = (ComplexTestCase) dcl.getInstance(entries[i].longName);
            }
            catch (java.lang.Exception e)
            {
                e.printStackTrace();
                return false;
            }
            testClass.executeMethods(entries[i], param);

            Summarizer sum = new Summarizer();
            sum.summarizeUp(entries[i]);

            if (office != null)
            {
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
