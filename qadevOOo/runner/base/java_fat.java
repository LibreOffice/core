/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: java_fat.java,v $
 * $Revision: 1.15.2.1 $
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

import com.sun.star.lang.XMultiServiceFactory;

import helper.APIDescGetter;
import helper.AppProvider;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.PrintWriter;

import java.util.Vector;

import lib.MultiMethodTest;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import lib.TestResult;

import share.DescEntry;
import share.DescGetter;
import share.LogWriter;

import stats.OutProducerFactory;
import stats.Summarizer;

import util.DynamicClassLoader;

/**
 *
 * this class handles tests written in java and running on a fat Office
 */
public class java_fat implements TestBase
{

    private static boolean debug = false;
    private static boolean keepdocument = false;
    private static boolean logging = true;
    private static boolean newOffice = false;
    private DynamicClassLoader dcl = null;

    private lib.TestParameters m_aParams;
    private AppProvider m_aOffice;

    public boolean executeTest(lib.TestParameters param)
        {
            m_aParams = param;

            dcl = new DynamicClassLoader();

            DescGetter dg = new APIDescGetter();
            String job = (String) param.get("TestJob");
            String ExclusionFile = (String) param.get("ExclusionList");
            Vector exclusions = null;
            boolean retValue = true;
            debug = param.getBool("DebugIsActive");
            logging = param.getBool("LoggingIsActive");
            keepdocument = param.getBool("KeepDocument");
            newOffice = param.getBool(util.PropertyName.NEW_OFFICE_INSTANCE);
            if (keepdocument)
            {
                System.setProperty("KeepDocument", "true");
            }
            if (ExclusionFile != null)
            {
                exclusions = getExclusionList(ExclusionFile, debug);
            }
            //get Job-Descriptions
            System.out.println("Getting Descriptions for Job: " + job);

            String sDescriptionPath = (String) param.get("DescriptionPath");
            DescEntry[] entries = dg.getDescriptionFor(job, sDescriptionPath, debug);

            // System.out.println();

            if (entries == null)
            {
                System.out.println("Couldn't get Description for Job: " + job);

                return false;
            }

//        String officeProviderName = (String) param.get("OfficeProvider");
//        AppProvider office = (AppProvider) dcl.getInstance(officeProviderName);
//
//        if (office == null) {
//            System.out.println("ERROR: Wrong parameter 'OfficeProvider', " +
//                               " it cannot be instantiated.");
//            System.exit(-1);
//        }

            m_aOffice = startOffice(param);

            boolean firstRun = true;

            for (int l = 0; l < entries.length; l++)
            {
                if (entries[l] == null)
                {
                    continue;
                }

                if (entries[l].hasErrorMsg)
                {
                    System.out.println(entries[l].ErrorMsg);
                    retValue = false;
                    continue;
                }

                if (!firstRun && newOffice)
                {
                    if (!m_aOffice.closeExistingOffice(param, true))
                    {
                        m_aOffice.disposeManager(param);
                    }
                    startOffice(param);
                }
                firstRun = false;

//            XMultiServiceFactory msf = (XMultiServiceFactory) office.getManager(
//                                               param);

                XMultiServiceFactory msf = (XMultiServiceFactory) param.getMSF();

                if (msf == null)
                {
                    retValue = false;

                    continue;
                }

//            param.put("ServiceFactory", msf);

                DescEntry entry = entries[l];

                //get some helper classes
                Summarizer sumIt = new Summarizer();

                TestCase tCase = null;

                try
                {
                    tCase = (TestCase) dcl.getInstance("mod._" + entry.entryName);
                }
                catch (java.lang.IllegalArgumentException ie)
                {
                    entry.ErrorMsg = ie.getMessage();
                    entry.hasErrorMsg = true;
                }
                catch (java.lang.NoClassDefFoundError ie)
                {
                    entry.ErrorMsg = ie.getMessage();
                    entry.hasErrorMsg = true;
                }

                if (tCase == null)
                {
                    Summarizer.summarizeDown(entry, entry.ErrorMsg);

                    LogWriter sumObj = OutProducerFactory.createOutProducer(param);
                    entry.UserDefinedParams = param;
                    sumObj.initialize(entry, logging);
                    sumObj.summary(entry);

                    continue;
                }

                if (debug)
                {
                    System.out.println("sleeping 5 seconds..");
                }
                util.utils.shortWait(5000);

                System.out.println("Creating: " + entry.entryName);

                LogWriter log = (LogWriter) dcl.getInstance((String) param.get("LogWriter"));
                log.initialize(entry, logging);
                entry.UserDefinedParams = param;

                TestEnvironment tEnv = null;

                try
                {
                    tCase.setLogWriter((PrintWriter) log);
                    tCase.initializeTestCase(param);
                    tEnv = tCase.getTestEnvironment(param);
                }
                catch (Exception e)
                {
                    System.out.println("Exception while creating " + tCase.getObjectName());
                    System.out.println("Message " + e.getMessage());
                    e.printStackTrace();
                    tEnv = null;
                }
                catch (java.lang.UnsatisfiedLinkError e)
                {
                    System.out.println("Exception while creating " + tCase.getObjectName());
                    System.out.println("Message " + e.getMessage());
                    tEnv = null;
                }
                catch (java.lang.NoClassDefFoundError e)
                {
                    System.out.println("Exception while creating " + tCase.getObjectName());
                    System.out.println("Message " + e.getMessage());
                    tEnv = null;
                }

                if (tEnv == null)
                {
                    Summarizer.summarizeDown(entry, "Couldn't create " + tCase.getObjectName());

                    LogWriter sumObj = OutProducerFactory.createOutProducer(param);
                    entry.UserDefinedParams = param;
                    sumObj.initialize(entry, logging);
                    sumObj.summary(entry);

                    continue;
                }

                System.out.println(tCase.getObjectName() + " recreated ");

                for (int j = 0; j < entry.SubEntryCount; j++)
                {
                    DescEntry aSubEntry = entry.SubEntries[j];
                    if (!aSubEntry.isToTest)
                    {
                        Summarizer.summarizeDown(aSubEntry, "not part of the job");

                        continue;
                    }

                    if ((exclusions != null) && (exclusions.contains(aSubEntry.longName)))
                    {
                        Summarizer.summarizeDown(aSubEntry, "known issue");

                        continue;
                    }

                    System.out.println("running: '" + aSubEntry.entryName + "'");

                    LogWriter ifclog = (LogWriter) dcl.getInstance( (String) param.get("LogWriter"));

                    ifclog.initialize(aSubEntry, logging);
                    aSubEntry.UserDefinedParams = param;
                    aSubEntry.Logger = ifclog;

                    if ((tEnv == null) || tEnv.isDisposed())
                    {
                        closeExistingOffice();
                        tEnv = getEnv(entry, param);
                    }

                    MultiMethodTest ifc = null;
                    lib.TestResult res = null;

                    // run the interface test twice if it failed.
                    int countInterfaceTestRun = 0;
                    boolean finished = false;
                    while (!finished)
                    {
                        try
                        {
                            countInterfaceTestRun++;
                            finished = true;
                            res = executeInterfaceTest(aSubEntry, tEnv, param);
                        }
                        catch (IllegalArgumentException iae)
                        {
                            System.out.println("Couldn't load class " + aSubEntry.entryName);
                            System.out.println("**** " + iae.getMessage() + " ****");
                            Summarizer.summarizeDown(aSubEntry, iae.getMessage());
                        }
                        catch (java.lang.NoClassDefFoundError iae)
                        {
                            System.out.println("Couldn't load class " + aSubEntry.entryName);
                            System.out.println("**** " + iae.getMessage() + " ****");
                            Summarizer.summarizeDown(aSubEntry, iae.getMessage());
                        }
                        catch (java.lang.RuntimeException e)
                        {
                            closeExistingOffice();
                            tEnv = getEnv(entry, param);
                            if (countInterfaceTestRun < 2)
                            {
                                finished = false;
                            }
                            else
                            {
                                Summarizer.summarizeDown(aSubEntry, e.toString() + ".FAILED");
                            }
                        }
                    }
                    if (res != null)
                    {
                        for (int k = 0; k < aSubEntry.SubEntryCount; k++)
                        {
                            DescEntry aSubSubEntry = aSubEntry.SubEntries[k];
                            if (res.hasMethod( aSubSubEntry.entryName))
                            {
                                aSubSubEntry.State = res.getStatusFor(aSubSubEntry.entryName).toString();
                            }
                        }
                    }

                    sumIt.summarizeUp(aSubEntry);

                    LogWriter sumIfc = OutProducerFactory.createOutProducer(param);
                    aSubEntry.UserDefinedParams = param;
                    sumIfc.initialize(aSubEntry, logging);
                    sumIfc.summary(aSubEntry);
                }

                try
                {
                    if (!keepdocument)
                    {
                        tCase.cleanupTestCase(param);
                    }
                }
                catch (Exception e)
                {
                    System.out.println("couldn't cleanup");
                }
                catch (java.lang.NoClassDefFoundError e)
                {
                    System.out.println("couldn't cleanup");
                }

                sumIt.summarizeUp(entry);

                LogWriter sumObj = OutProducerFactory.createOutProducer(param);

                sumObj.initialize(entry, logging);
                sumObj.summary(entry);
            }

            if (entries.length > 0)
            {
                System.out.println();

                int counter = 0;
                System.out.println("Failures that appeared during scenario execution:");

                for (int i = 0; i < entries.length; i++)
                {
                    if (!entries[i].State.endsWith("OK"))
                    {
                        System.out.println("\t " + entries[i].longName);
                        counter++;
                    }
                }

                System.out.println(counter + " of " + entries.length + " tests failed");
            }

            closeExistingOffice();
            return retValue;
        }

    private TestEnvironment getEnv(DescEntry entry, TestParameters param)
        {
//        if (dcl == null)
//            dcl = new DynamicClassLoader();
//        String officeProviderName = (String) param.get("OfficeProvider");
//        AppProvider office = (AppProvider) dcl.getInstance(officeProviderName);
//
//        if (office == null) {
//            System.out.println("ERROR: Wrong parameter 'OfficeProvider', " +
//                               " it cannot be instantiated.");
//            System.exit(-1);
//        }
//
//        XMultiServiceFactory msf = (XMultiServiceFactory) office.getManager(
//                                           param);
//
//        if (msf == null) {
//            return null;
//        }
//
//        param.put("ServiceFactory", msf);

            // AppProvider office = startOffice(param);

            TestCase tCase = null;

            try
            {
                tCase = (TestCase) dcl.getInstance("mod._" + entry.entryName);
            }
            catch (java.lang.IllegalArgumentException ie)
            {
                entry.ErrorMsg = ie.getMessage();
                entry.hasErrorMsg = true;
            }
            catch (java.lang.NoClassDefFoundError ie)
            {
                entry.ErrorMsg = ie.getMessage();
                entry.hasErrorMsg = true;
            }

            System.out.println("Creating: " + entry.entryName);

            entry.UserDefinedParams = param;

            LogWriter log = (LogWriter) dcl.getInstance((String) param.get("LogWriter"));
            log.initialize(entry, logging);
            tCase.setLogWriter((PrintWriter) log);

            TestEnvironment tEnv = null;

            try
            {
                tCase.initializeTestCase(param);
                tEnv = tCase.getTestEnvironment(param);
            }
            catch (com.sun.star.lang.DisposedException de)
            {
                System.out.println("Office disposed");
                closeExistingOffice();
            }
            catch (lib.StatusException e)
            {
                System.out.println(e.getMessage());

                closeExistingOffice();

                entry.ErrorMsg = e.getMessage();
                entry.hasErrorMsg = true;
            }

            return tEnv;
        }

    private void closeExistingOffice()
        {
            helper.ProcessHandler ph = (helper.ProcessHandler) m_aParams.get("AppProvider");

            if (ph != null)
            {
                m_aOffice.closeExistingOffice(m_aParams, true);
                shortWait(5000);
            }

        }

    private void shortWait(int millis)
        {
            try
            {
                Thread.sleep(millis);
            }
            catch (java.lang.InterruptedException ie)
            {
            }
        }

    private Vector getExclusionList(String url, boolean debug)
        {
            Vector entryList = new Vector();
            String line = "#";
            BufferedReader exclusion = null;

            try
            {
                exclusion = new BufferedReader(new FileReader(url));
            }
            catch (java.io.FileNotFoundException fnfe)
            {
                if (debug)
                {
                    System.out.println("Couldn't find file " + url);
                }

                return entryList;
            }

            while (line != null)
            {
                try
                {
                    if (!line.startsWith("#") && (line.length() > 1))
                    {
                        entryList.add(line.trim());
                    }

                    line = exclusion.readLine();
                }
                catch (java.io.IOException ioe)
                {
                    if (debug)
                    {
                        System.out.println("Exception while reading exclusion list");
                    }

                    return entryList;
                }
            }

            try
            {
                exclusion.close();
            }
            catch (java.io.IOException ioe)
            {
                if (debug)
                {
                    System.out.println("Couldn't close file " + url);
                }

                return entryList;
            }

            return entryList;
        }

    private TestResult executeInterfaceTest(
        DescEntry entry, TestEnvironment tEnv, TestParameters param)
        throws IllegalArgumentException, java.lang.NoClassDefFoundError
        {
            MultiMethodTest ifc = (MultiMethodTest) dcl.getInstance(entry.entryName);
            return ifc.run(entry, tEnv, param);
        }

    private AppProvider startOffice(lib.TestParameters param)
        {

            if (dcl == null)
            {
                dcl = new DynamicClassLoader();
            }

            String officeProviderName = (String) param.get("OfficeProvider");
            AppProvider office = (AppProvider) dcl.getInstance(officeProviderName);

            if (office == null)
            {
                System.out.println("ERROR: Wrong parameter 'OfficeProvider', " + " it cannot be instantiated.");
                System.exit(-1);
            }

            XMultiServiceFactory msf = (XMultiServiceFactory) office.getManager(param);

            if (msf != null)
            {
                param.put("ServiceFactory", msf);
            }

            return office;
        }
}
