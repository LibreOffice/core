/*************************************************************************
 *
 *  $RCSfile: basic_fat.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-11-18 16:12:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package base;

import base.TestBase;

import basicrunner.BasicIfcTest;
import basicrunner.BasicTestCase;

import com.sun.star.lang.XMultiServiceFactory;

import helper.APIDescGetter;
import helper.OfficeProvider;
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


        //get Job-Descriptions
        System.out.print("Getting Descriptions for Job: " + job + " from ");

        DescEntry[] entries = dg.getDescriptionFor(job,
                                                   (String) param.get(
                                                           "DescriptionPath"),
                                                   true);

        if (entries == null) {
            System.out.println("Couldn't get Description for Job");

            return false;
        }

        String ExclusionFile = (String) param.get("ExclusionList");
        Vector exclusions = null;
        debug = param.getBool("DebugIsActive");

        if (ExclusionFile != null) {
            exclusions = getExclusionList(ExclusionFile, debug);
        }

        String conStr = (String) param.get("ConnectionString");
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

                LogWriter sumObj = (LogWriter) dcl.getInstance(
                                           (String) param.get("OutProducer"));
                sumObj.initialize(entry, true);
                sumObj.summary(entry);

                return true;
            }

            System.out.println("Creating: " + tCase.getObjectName());

            LogWriter log = (LogWriter) dcl.getInstance(
                                    (String) param.get("LogWriter"));
            log.initialize(entry, true);
            entry.UserDefinedParams = param;
            tCase.setLogWriter((PrintWriter) log);
            tCase.initializeTestCase(param);

            TestEnvironment tEnv = tCase.getTestEnvironment(param);

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

                if (tEnv.isDisposed()) {
                    tEnv = tCase.getTestEnvironment(param);
                    System.out.println("created " + tCase.getObjectName());
                }

                BasicIfcTest ifc = null;
                lib.TestResult res = null;
                ifc = new BasicIfcTest(entry.SubEntries[j].longName);
                res = ifc.run(entry.SubEntries[j], tEnv, param);

                sumIt.summarizeUp(entry.SubEntries[j]);

                LogWriter sumIfc = (LogWriter) dcl.getInstance(
                                           (String) param.get("OutProducer"));

                sumIfc.initialize(entry.SubEntries[j], true);
                sumIfc.summary(entry.SubEntries[j]);
            }

            try {
                tCase.cleanupTestCase(param);
            } catch (Exception e) {
                log.println("TestCase already gone");
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