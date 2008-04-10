/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basic_fat.java,v $
 * $Revision: 1.6 $
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

import base.TestBase;

import basicrunner.BasicIfcTest;
import basicrunner.BasicTestCase;

import com.sun.star.lang.XMultiServiceFactory;

import helper.APIDescGetter;
import helper.AppProvider;
import helper.OfficeProvider;
import helper.OfficeWatcher;
import helper.ProcessHandler;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.PrintWriter;

import java.util.Vector;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import share.DescEntry;
import share.DescGetter;
import share.LogWriter;

import stats.OutProducerFactory;
import stats.Summarizer;

import util.DynamicClassLoader;


/**
 * The testbase for executing basic tests.
 *  @see lib.TestBase
 */
public class basic_fat implements TestBase {
    public static boolean debug = false;

    /**
     *  Execute a test.
     *  @param param The test parameters.
     *  @param return True, if the test was executed.
     */
    public boolean executeTest(TestParameters param) {
        DescGetter dg = new APIDescGetter();
        String job = (String) param.get("TestJob");
        OfficeProvider office = null;
        debug = param.getBool("DebugIsActive");


        //get Job-Descriptions
        System.out.print("Getting Descriptions for Job: " + job + " from ");

        DescEntry[] entries = dg.getDescriptionFor(job,
                                                   (String) param.get(
                                                           "DescriptionPath"),
                                                   debug);

        if (entries == null) {
            System.out.println("Couldn't get Description for Job");

            return false;
        }

        String ExclusionFile = (String) param.get("ExclusionList");
        Vector exclusions = null;

        if (ExclusionFile != null) {
            exclusions = getExclusionList(ExclusionFile, debug);
        }

        String conStr = (String) param.get("ConnectionString");
        System.out.println("");
        System.out.print("> Connecting the Office ");
        System.out.println("With " + conStr);

        for (int l = 0; l < entries.length; l++) {
            if (entries[l].hasErrorMsg) {
                System.out.println(entries[l].ErrorMsg);

                continue;
            }

            office = new OfficeProvider();

            XMultiServiceFactory msf = (XMultiServiceFactory) office.getManager(
                                               param);

            if (msf == null) {
                return false;
            }

            param.put("ServiceFactory", msf);

            DescEntry entry = entries[l];

            //get some helper classes
            Summarizer sumIt = new Summarizer();
            DynamicClassLoader dcl = new DynamicClassLoader();

            TestCase tCase = null;

            tCase = (TestCase) new BasicTestCase(entry);

            if (tCase == null) {
                sumIt.summarizeDown(entry, entry.ErrorMsg);

                LogWriter sumObj = OutProducerFactory.createOutProducer(param);
                sumObj.initialize(entry, true);
                sumObj.summary(entry);

                continue;
            }

            System.out.println("Creating: " + tCase.getObjectName());

            LogWriter log = (LogWriter) dcl.getInstance(
                                    (String) param.get("LogWriter"));
            log.initialize(entry, true);
            entry.UserDefinedParams = param;
            tCase.setLogWriter((PrintWriter) log);

            try {
                tCase.initializeTestCase(param);
            } catch (RuntimeException e) {
                helper.ProcessHandler ph = (helper.ProcessHandler) param.get(
                                                   "AppProvider");

                if (ph != null) {
                    OfficeWatcher ow = (OfficeWatcher) param.get("Watcher");

                    if ((ow != null) && ow.isAlive()) {
                        ow.finish = true;
                    }

                    ph.kill();
                    shortWait(5000);
                }

                continue;
            }

            TestEnvironment tEnv = tCase.getTestEnvironment(param);

            if (tEnv == null) {
                sumIt.summarizeDown(entry, "Unable to create testcase");

                LogWriter sumObj = OutProducerFactory.createOutProducer(param);
                sumObj.initialize(entry, true);
                sumObj.summary(entry);

                helper.ProcessHandler ph = (helper.ProcessHandler) param.get(
                                                   "AppProvider");

                if (ph != null) {
                    OfficeWatcher ow = (OfficeWatcher) param.get("Watcher");

                    if ((ow != null) && ow.isAlive()) {
                        ow.finish = true;
                    }

                    ph.kill();
                    shortWait(5000);
                }

                continue;
            }

            System.out.println("created " + tCase.getObjectName());

            for (int j = 0; j < entry.SubEntryCount; j++) {
                if (!entry.SubEntries[j].isToTest) {
                    Summarizer.summarizeDown(entry.SubEntries[j],
                                             "not part of the job");

                    continue;
                }

                if ((exclusions != null) &&
                        (exclusions.contains(entry.SubEntries[j].longName))) {
                    Summarizer.summarizeDown(entry.SubEntries[j],
                                             "known issue");

                    continue;
                }

                System.out.println("running: " +
                                   entry.SubEntries[j].entryName);

                LogWriter ifclog = (LogWriter) dcl.getInstance(
                                           (String) param.get("LogWriter"));
                ifclog.initialize(entry.SubEntries[j], true);
                entry.SubEntries[j].UserDefinedParams = param;
                entry.SubEntries[j].Logger = ifclog;

                if ((tEnv == null) || tEnv.isDisposed()) {
                    helper.ProcessHandler ph = (helper.ProcessHandler) param.get(
                                                       "AppProvider");

                    if (ph != null) {
                        office.closeExistingOffice(param, true);
                        shortWait(5000);
                    }

                    tEnv = getEnv(entry, param);
                }

                BasicIfcTest ifc = null;
                lib.TestResult res = null;
                ifc = new BasicIfcTest(entry.SubEntries[j].longName);
                res = ifc.run(entry.SubEntries[j], tEnv, param);

                sumIt.summarizeUp(entry.SubEntries[j]);

                LogWriter sumIfc = OutProducerFactory.createOutProducer(param);

                sumIfc.initialize(entry.SubEntries[j], true);
                sumIfc.summary(entry.SubEntries[j]);
            }

            try {
                tCase.cleanupTestCase(param);
            } catch (Exception e) {
                log.println("TestCase already gone");

                helper.ProcessHandler ph = (helper.ProcessHandler) param.get(
                                                   "AppProvider");

                if (ph != null) {
                    office.closeExistingOffice(param, true);
                    shortWait(5000);
                }
            }

            sumIt.summarizeUp(entry);

            LogWriter sumObj = OutProducerFactory.createOutProducer(param);
            sumObj.initialize(entry, true);
            sumObj.summary(entry);
        }

        if (entries.length > 1) {
            System.out.println();

            int counter = 0;
            System.out.println(
                    "Failures that appeared during scenario execution:");

            for (int i = 0; i < entries.length; i++) {
                if (!entries[i].State.endsWith("OK")) {
                    System.out.println("\t " + entries[i].longName);
                    counter++;
                }
            }

            System.out.println(counter + " of " + entries.length +
                               " tests failed");
        }

        helper.ProcessHandler ph = (helper.ProcessHandler) param.get(
                                           "AppProvider");

        if (ph != null) {
            office.closeExistingOffice(param, true);
        }

        return true;
    }

    protected TestEnvironment getEnv(DescEntry entry, TestParameters param) {
        DynamicClassLoader dcl = new DynamicClassLoader();
        String officeProviderName = (String) param.get("OfficeProvider");
        AppProvider office = (AppProvider) dcl.getInstance(officeProviderName);

        if (office == null) {
            System.out.println("ERROR: Wrong parameter 'OfficeProvider', " +
                               " it cannot be instantiated.");
            System.exit(-1);
        }

        XMultiServiceFactory msf = (XMultiServiceFactory) office.getManager(
                                           param);

        if (msf == null) {
            return null;
        }

        param.put("ServiceFactory", msf);

        TestCase tCase = (TestCase) new BasicTestCase(entry);

        System.out.println("Creating: " + tCase.getObjectName());

        LogWriter log = (LogWriter) dcl.getInstance(
                                (String) param.get("LogWriter"));
        log.initialize(entry, true);
        entry.UserDefinedParams = param;
        tCase.setLogWriter((PrintWriter) log);

        TestEnvironment tEnv = null;

        try {
            tCase.initializeTestCase(param);
            tEnv = tCase.getTestEnvironment(param);
        } catch (java.lang.RuntimeException e) {
            System.out.println(e.getMessage());

            helper.ProcessHandler ph = (helper.ProcessHandler) param.get(
                                               "AppProvider");

            if (ph != null) {
                office.closeExistingOffice(param, true);
                shortWait(5000);
            }

            entry.ErrorMsg = e.getMessage();
            entry.hasErrorMsg = true;
        }

        return tEnv;
    }

    protected void shortWait(int millis) {
        try {
            Thread.sleep(millis);
        } catch (java.lang.InterruptedException ie) {
        }
    }

    protected Vector getExclusionList(String url, boolean debug) {
        Vector entryList = new Vector();
        String line = "#";
        BufferedReader exclusion = null;

        try {
            exclusion = new BufferedReader(new FileReader(url));
        } catch (java.io.FileNotFoundException fnfe) {
            if (debug) {
                System.out.println("Couldn't find file " + url);
            }

            return entryList;
        }

        while (line != null) {
            try {
                if (!line.startsWith("#")) {
                    entryList.add(line);
                }

                line = exclusion.readLine();
            } catch (java.io.IOException ioe) {
                if (debug) {
                    System.out.println(
                            "Exception while reading exclusion list");
                }

                return entryList;
            }
        }

        try {
            exclusion.close();
        } catch (java.io.IOException ioe) {
            if (debug) {
                System.out.println("Couldn't close file " + url);
            }

            return entryList;
        }

        return entryList;
    }
}