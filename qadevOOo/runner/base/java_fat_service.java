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


import java.io.PrintWriter;

import lib.TestParameters;
import lib.TestCase;
import lib.MultiMethodTest;
import lib.TestEnvironment;
import util.DynamicClassLoader;

import share.DescEntry;
import share.DescGetter;
import helper.APIDescGetter;

import share.LogWriter;
import stats.Summarizer;

/**
 * this class handles tests written in java and running on a fat Office
 * with the runner as a service.
 */
public class java_fat_service implements TestBase {

    static protected boolean debug = false;
    protected LogWriter log = null;

    public boolean executeTest(lib.TestParameters param) {
        DynamicClassLoader dcl = new DynamicClassLoader();
        log = (LogWriter)dcl.getInstance((String)param.get("LogWriter"));
        debug = ((Boolean) param.get("DebugIsActive")).booleanValue();

        DescGetter dg = new APIDescGetter();
        String job = (String) param.get("TestJob");
        boolean retValue = true;

        //get Job-Descriptions
        log.println("Getting Descriptions for Job: "+job);
        DescEntry[] entries = dg.getDescriptionFor(job,
                (String) param.get("DescriptionPath"),debug);

        if (entries == null ) {
            log.println("Couldn't get Description for Job");
            return false;
        }

        param.get("ConnectionString");

        param.getMSF();

        for (int l=0;l<entries.length;l++) {

            if (entries[l] == null ) {
                continue;
            }

            if (entries[l].hasErrorMsg) {
                log.println(entries[l].ErrorMsg);
                continue;
            }

            DescEntry entry = entries[l];

            //get some helper classes
            Summarizer sumIt = new Summarizer();

            TestCase tCase = null;
            try {
                tCase = (TestCase)
                            dcl.getInstance("mod._"+entry.entryName);
            } catch (java.lang.IllegalArgumentException ie) {
                entry.ErrorMsg=ie.getMessage();
                entry.hasErrorMsg=true;
            }

            if (tCase == null) {
                sumIt.summarizeDown(entry,entry.ErrorMsg);
                LogWriter sumObj = (LogWriter)dcl.getInstance(
                                        (String)param.get("OutProducer"));
                sumObj.initialize(entry,true);
                entry.UserDefinedParams = param;
                sumObj.summary(entry);
                continue;
            }

            log.println("Creating: "+tCase.getObjectName());

            log.initialize(entry,true);
            entry.UserDefinedParams = param;
            TestEnvironment tEnv = null;
            try {
                tCase.setLogWriter((PrintWriter) log);
                tCase.initializeTestCase(param);
                tEnv = tCase.getTestEnvironment(param);
            } catch (Exception e) {
                log.println("Exception while creating "+tCase.getObjectName());
                log.println("Exception: " + e);
                log.println("Message "+e.getMessage());
                tEnv = null;
            }
            if (tEnv == null) {
                sumIt.summarizeDown(entry,"Couldn't create "+tCase.getObjectName());
                LogWriter sumObj = (LogWriter)dcl.getInstance(
                                            (String)param.get("OutProducer"));
                sumObj.initialize(entry,true);
                entry.UserDefinedParams = param;
                sumObj.summary(entry);
                continue;
            }
            log.println("Created "+tCase.getObjectName()+"\n");

            for (int j=0;j<entry.SubEntryCount;j++) {
                if (!entry.SubEntries[j].isToTest) {
                    Summarizer.summarizeDown(entry.SubEntries[j],"not part of the job");
                    continue;
                }

                log.println("running: "+entry.SubEntries[j].entryName);

                LogWriter ifclog = (LogWriter)dcl.getInstance(
                                            (String)param.get("LogWriter"));

                ifclog.initialize(entry.SubEntries[j],true);
                entry.SubEntries[j].UserDefinedParams = param;
                entry.SubEntries[j].Logger = ifclog;

                if (tEnv == null || tEnv.isDisposed()) {
                    tEnv = getEnv(entry,param);
                }

                MultiMethodTest ifc = null;
                lib.TestResult res = null;
                try {
                    ifc = (MultiMethodTest) dcl.getInstance(
                                              entry.SubEntries[j].entryName);
                    res = ifc.run(entry.SubEntries[j],tEnv,param);
                } catch (IllegalArgumentException iae) {
                    log.println("Couldn't load class "+entry.SubEntries[j].entryName);
                    log.println("**** "+iae.getMessage()+" ****");
                    Summarizer.summarizeDown(entry.SubEntries[j],iae.getMessage());
                } catch (java.lang.RuntimeException e) {
                    tEnv = getEnv(entry,param);
                    ifc = (MultiMethodTest) dcl.getInstance(
                                              entry.SubEntries[j].entryName);
                    if ((tEnv != null) && (ifc != null)) {
                        res = ifc.run(entry.SubEntries[j],tEnv,param);
                    }
                    else
                    {
                        res = null;
                    }
                }
                if (res != null) {
                    for (int k=0;k<entry.SubEntries[j].SubEntryCount;k++) {
                        if (res.hasMethod(entry.SubEntries[j].SubEntries[k].entryName)) {
                            entry.SubEntries[j].SubEntries[k].State=
                                res.getStatusFor(entry.SubEntries[j].SubEntries[k].entryName).toString();
                        }
                    }
                }
                sumIt.summarizeUp(entry.SubEntries[j]);

                LogWriter sumIfc = (LogWriter)dcl.getInstance(
                                            (String)param.get("OutProducer"));

                sumIfc.initialize(entry.SubEntries[j],true);
                entry.SubEntries[j].UserDefinedParams = param;
                sumIfc.summary(entry.SubEntries[j]);
            }
            try {
                tCase.cleanupTestCase(param);
            } catch (Exception e) {
                e.printStackTrace((PrintWriter)log);
            }
            sumIt.summarizeUp(entry);
            LogWriter sumObj = (LogWriter)dcl.getInstance(
                                        (String)param.get("OutProducer"));
            sumObj.initialize(entry,true);
            sumObj.summary(entry);
        }
        if (entries.length > 1) {
            log.println("");
            int counter = 0;
            log.println("Failures that appeared during scenario execution:");
            for (int i=0;i<entries.length;i++) {
                if (! entries[i].State.endsWith("OK")) {
                    log.println("\t "+entries[i].longName);
                    counter++;
                }
            }
            log.println(counter +" of "+entries.length + " tests failed" );
        }

        return retValue;
    }

    protected TestEnvironment getEnv(DescEntry entry, TestParameters param) {
            DynamicClassLoader dcl = new DynamicClassLoader();
            log = (LogWriter)dcl.getInstance((String)param.get("LogWriter"));
            param.getMSF();

            TestCase tCase = null;

            try
            {
                tCase = (TestCase) dcl.getInstance("mod._"+entry.entryName);
            }
            catch (java.lang.IllegalArgumentException ie)
            {
                entry.ErrorMsg=ie.getMessage();
                entry.hasErrorMsg=true;
            }

            log.println("Creating: "+tCase.getObjectName());
            // LogWriter log = (LogWriter)dcl.getInstance( (String)param.get("LogWriter"));
            log.initialize(entry,true);
            entry.UserDefinedParams = param;
            tCase.setLogWriter((PrintWriter) log);
            TestEnvironment tEnv = null;
            try
            {
                tCase.initializeTestCase(param);
                tEnv = tCase.getTestEnvironment(param);
            }
            catch (com.sun.star.lang.DisposedException de)
            {
                log.println("Office disposed");
            }
            return tEnv;
    }

//    protected void shortWait(int millis) {
//        try {
//            Thread.sleep(millis);
//        } catch (java.lang.InterruptedException ie) {}
//    }

}
