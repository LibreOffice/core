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

import helper.APIDescGetter;
import helper.AppProvider;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.PrintWriter;
import java.util.ArrayList;
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

import com.sun.star.lang.XMultiServiceFactory;

/**
 *
 * this class handles tests written in java and running on a fat Office
 */
public class java_fat implements TestBase
{

    private static boolean m_isDebug = false;
    private static boolean keepdocument = false;
    private static boolean logging = true;
    private static boolean newOffice = false;
    private DynamicClassLoader m_aDynamicClassLoader = null;

    private lib.TestParameters m_aParams;
    private AppProvider m_aOffice;

    public boolean executeTest(lib.TestParameters _aParams)
        {
            m_aParams = _aParams;

            m_aDynamicClassLoader = new DynamicClassLoader();

            DescGetter dg = new APIDescGetter();
            String job = (String) m_aParams.get("TestJob");
            String ExclusionFile = (String) m_aParams.get("ExclusionList");
            ArrayList<String> exclusions = null;
            boolean retValue = true;
            m_isDebug = m_aParams.getBool("DebugIsActive");
            logging = m_aParams.getBool("LoggingIsActive");
            keepdocument = m_aParams.getBool("KeepDocument");
            newOffice = m_aParams.getBool(util.PropertyName.NEW_OFFICE_INSTANCE);
            if (keepdocument)
            {
                System.setProperty("KeepDocument", "true");
            }
            if (ExclusionFile != null)
            {
                exclusions = getExclusionList(ExclusionFile, m_isDebug);
            }
            //get Job-Descriptions
            // System.out.println("Getting Descriptions for Job: " + job);

            String sDescriptionPath = (String) m_aParams.get("DescriptionPath");
            DescEntry[] entries = dg.getDescriptionFor(job, sDescriptionPath, m_isDebug);

            // System.out.println();

            if (entries == null)
            {
                System.out.println("Couldn't get Description for Job: " + job);

                return false;
            }

//        String officeProviderName = (String) m_aParams.get("OfficeProvider");
//        AppProvider office = (AppProvider) m_aDynamicClassLoader.getInstance(officeProviderName);
//
//        if (office == null) {
//            System.out.println("ERROR: Wrong parameter 'OfficeProvider', " +
//                               " it cannot be instantiated.");
//            System.exit(-1);
//        }

            m_aOffice = startOffice(m_aParams);

            boolean firstRun = true;

            // Run through all entries (e.g. sw.SwXBookmark.*)

            for (int l = 0; l < entries.length; l++)
            {
                DescEntry entry = entries[l];

                if (entry == null)
                {
                    continue;
                }

                if (entry.hasErrorMsg)
                {
                    System.out.println(entries[l].ErrorMsg);
                    retValue = false;
                    continue;
                }

                if (!firstRun && newOffice)
                {
                    if (!m_aOffice.closeExistingOffice(m_aParams, true))
                    {
                        m_aOffice.disposeManager(m_aParams);
                    }
                    startOffice(m_aParams);
                }
                firstRun = false;

                XMultiServiceFactory msf = (XMultiServiceFactory) m_aParams.getMSF();

                if (msf == null)
                {
                    retValue = false;
                    continue;
                }

                //get some helper classes
                Summarizer sumIt = new Summarizer();
                TestCase tCase = getTestCase(entry);
                if (tCase == null)
                {
                    continue;
                }

//                if (m_isDebug)
//                {
//                    System.out.println("sleeping 2 seconds..");
//                }
                util.utils.shortWait(2000);

                System.out.println("Creating: " + entry.entryName);

                LogWriter log = (LogWriter) m_aDynamicClassLoader.getInstance((String) m_aParams.get("LogWriter"));
                log.initialize(entry, logging);
                entry.UserDefinedParams = m_aParams;

                tCase.setLogWriter((PrintWriter) log);
                tCase.initializeTestCase(m_aParams);

                TestEnvironment tEnv = getTestEnvironment(tCase, entry);
                if (tEnv == null)
                {
                    continue;
                }

                tCase.getObjectName();

                for (int j = 0; j < entry.SubEntryCount; j++)
                {
                    DescEntry aSubEntry = entry.SubEntries[j];
                    final boolean bIsToTest = aSubEntry.isToTest;
                    if (!bIsToTest)
                    {
                        Summarizer.summarizeDown(aSubEntry, "not part of the job");
                        continue;
                    }

                    // final String sEntryName = aSubEntry.entryName;
                    final String sLongEntryName = aSubEntry.longName;

                    if ((exclusions != null) && (exclusions.contains(sLongEntryName)))
                    {
                        Summarizer.summarizeDown(aSubEntry, "known issue");
                        continue;
                    }

                    // System.out.println("running: '" + sLongEntryName + "' testcode: [" + sEntryName + "]");
                    // this will shown in test itself

                    LogWriter ifclog = (LogWriter) m_aDynamicClassLoader.getInstance( (String) m_aParams.get("LogWriter"));

                    ifclog.initialize(aSubEntry, logging);
                    aSubEntry.UserDefinedParams = m_aParams;
                    aSubEntry.Logger = ifclog;

                    if ((tEnv == null) || tEnv.isDisposed())
                    {
                        closeExistingOffice();
                        tEnv = getEnv(entry, m_aParams);
                        tCase = tEnv.getTestCase();
                    }

                    // MultiMethodTest ifc = null;
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
                            res = executeInterfaceTest(aSubEntry, tEnv, m_aParams);
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
                            tEnv = getEnv(entry, m_aParams);
                            tCase = tEnv.getTestCase();
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
                    setState(aSubEntry, res);

                    sumIt.summarizeUp(aSubEntry);

                    LogWriter sumIfc = OutProducerFactory.createOutProducer(m_aParams);
                    aSubEntry.UserDefinedParams = m_aParams;
                    sumIfc.initialize(aSubEntry, logging);
                    sumIfc.summary(aSubEntry);
                }

                try
                {
                    if (!keepdocument)
                    {
                        tCase.cleanupTestCase(m_aParams);
                    }
                }
                catch (Exception e)
                {
                    System.err.println( "couldn't cleanup:" + e.toString() );
                }
                catch (java.lang.NoClassDefFoundError e)
                {
                    System.err.println( "couldn't cleanup:" + e.toString() );
                }

                sumIt.summarizeUp(entry);

                LogWriter sumObj = OutProducerFactory.createOutProducer(m_aParams);

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
                    final String sState = entries[i].State;
                    if (!sState.endsWith("OK"))
                    {
                        System.out.println("\t " + entries[i].longName);
                        counter++;
                    }
                }

                System.out.println(counter + " of " + entries.length + " tests failed");
                if (counter != 0) {
                    retValue = false;
                }
            }

            closeExistingOffice();
            return retValue;
        }

//
    private TestEnvironment getTestEnvironment(TestCase tCase, DescEntry entry)
    {
        TestEnvironment tEnv = null;

        try
        {
            tEnv = tCase.getTestEnvironment(m_aParams);
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

            LogWriter sumObj = OutProducerFactory.createOutProducer(m_aParams);
            entry.UserDefinedParams = m_aParams;
            sumObj.initialize(entry, logging);
            sumObj.summary(entry);
        }
        return tEnv;
    }
    // -------------------------------------------------------------------------
    private TestCase getTestCase(DescEntry _aEntry)
    {
        TestCase tCase = null;

        try
        {
            tCase = (TestCase) m_aDynamicClassLoader.getInstance("mod._" + _aEntry.entryName);
        }
        catch (java.lang.IllegalArgumentException ie)
        {
            _aEntry.ErrorMsg = ie.getMessage();
            _aEntry.hasErrorMsg = true;
        }
        catch (java.lang.NoClassDefFoundError ie)
        {
            _aEntry.ErrorMsg = ie.getMessage();
            _aEntry.hasErrorMsg = true;
        }

        if (tCase == null)
        {
            Summarizer.summarizeDown(_aEntry, _aEntry.ErrorMsg);

            LogWriter sumObj = OutProducerFactory.createOutProducer(m_aParams);
            _aEntry.UserDefinedParams = m_aParams;
            sumObj.initialize(_aEntry, logging);
            sumObj.summary(_aEntry);
        }
        return tCase;
}

    private void setState(DescEntry aSubEntry, lib.TestResult res)
    {
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
    }

    private TestEnvironment getEnv(DescEntry entry, TestParameters param)
        {
//        if (m_aDynamicClassLoader == null)
//            m_aDynamicClassLoader = new DynamicClassLoader();
//        String officeProviderName = (String) m_aParams.get("OfficeProvider");
//        AppProvider office = (AppProvider) m_aDynamicClassLoader.getInstance(officeProviderName);
//
//        if (office == null) {
//            System.out.println("ERROR: Wrong parameter 'OfficeProvider', " +
//                               " it cannot be instantiated.");
//            System.exit(-1);
//        }
//
//        XMultiServiceFactory msf = (XMultiServiceFactory) office.getManager(
//                                           m_aParams);
//
//        if (msf == null) {
//            return null;
//        }
//
//        m_aParams.put("ServiceFactory", msf);

            // AppProvider office = startOffice(m_aParams);

            TestCase tCase = null;

            try
            {
                tCase = (TestCase) m_aDynamicClassLoader.getInstance("mod._" + entry.entryName);
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

            LogWriter log = (LogWriter) m_aDynamicClassLoader.getInstance((String) param.get("LogWriter"));
            log.initialize(entry, logging);
            tCase.setLogWriter((PrintWriter) log);

            try
            {
                tCase.initializeTestCase(param);
                return tCase.getTestEnvironment(param);
            }
            catch (com.sun.star.lang.DisposedException de)
            {
                System.out.println("Office disposed");
                closeExistingOffice();
                throw de;
            }
            catch (lib.StatusException e)
            {
                System.out.println(e.getMessage());

                closeExistingOffice();

                entry.ErrorMsg = e.getMessage();
                entry.hasErrorMsg = true;
                throw e;
            }
        }

    private void closeExistingOffice()
        {
            helper.ProcessHandler ph = (helper.ProcessHandler) m_aParams.get("AppProvider");

            if (ph != null)
            {
                m_aOffice.closeExistingOffice(m_aParams, true);
                util.utils.shortWait(5000);
            }

        }

//    private void shortWait(int millis)
//        {
//            try
//            {
//                Thread.sleep(millis);
//            }
//            catch (java.lang.InterruptedException ie)
//            {
//            }
//        }

    private ArrayList<String> getExclusionList(String url, boolean debug)
        {
            ArrayList<String> entryList = new ArrayList<String>();
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
            MultiMethodTest ifc = (MultiMethodTest) m_aDynamicClassLoader.getInstance(entry.entryName);
            return ifc.run(entry, tEnv, param);
        }

    private AppProvider startOffice(lib.TestParameters param)
        {

            if (m_aDynamicClassLoader == null)
            {
                m_aDynamicClassLoader = new DynamicClassLoader();
            }

            String officeProviderName = (String) param.get("OfficeProvider");
            AppProvider office = (AppProvider) m_aDynamicClassLoader.getInstance(officeProviderName);

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
