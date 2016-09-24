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
package complex.memCheck;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import helper.ProcessHandler;

import java.io.File;
import java.io.FileWriter;
import java.io.FilenameFilter;
import java.io.PrintWriter;
import java.util.StringTokenizer;

import lib.TestParameters;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;

import util.DesktopTools;
import util.PropertyName;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;

/**
 * Documents are opened and exported with StarOffice. The memory usage of
 * StarOffice is monitored and if the usage exceeds the allowed kilobytes,
 * the test is failed. Used for monitoring the StarOffice process is the
 * command line tool 'pmap', available on Solaris or Linux. This test will not
 * run on Windows.<br>Test procedure: every given document type is searched in
 * the source directory
 * Needed parameters:
 * <ul>
 *   <li>"TestDocumentPath" - the path where test documents are located.</li>
 *       All parameters are used for iteration over the test document path.
 * </ul>
 */
class TempDir
{

    private final String m_sTempDir;

    public TempDir(String _sTempDir)
    {
        m_sTempDir = _sTempDir;
    }

    public String getOfficeTempDir()
    {
        return m_sTempDir;
    }

    public String getTempDir()
    {
        final String sTempDir = FileHelper.getJavaCompatibleFilename(m_sTempDir);
        return sTempDir;
    }
}

public class CheckMemoryUsage
{

    TempDir m_aTempDir;
    private String[][] sDocTypeExportFilter;
    private String[][] sDocuments;
    // the allowed memory increase measured in kByte per exported document. The default is 10 kByte.
    // the allowed memory increase per exported document: if the memory increase is higher than this number, the test will fail
    private static final int iAllowMemoryIncrease = 10;
    private int iExportDocCount = 25;

    /**
     * Collect all documents to load and all filters used for export.
     */
    @Before
    public void before()
    {

        final XMultiServiceFactory xMsf = getMSF();

        // some Tests need the qadevOOo TestParameters, it is like a Hashmap for Properties.
        TestParameters param = new TestParameters();

        // test does definitely not run on Windows.
        if (param.get(PropertyName.OPERATING_SYSTEM).equals(PropertyName.WNTMSCI))
        {
            System.out.println("Test can only reasonably be executed with a tool that "
                    + "displays the memory usage of StarOffice.");
            System.out.println("Test does not run on Windows, only on Solaris or Linux.");
            // in an automatic environment it is better to say, there is no error here.
            // it is a limitation, but no error.
            System.exit(0);
        }


        // how many times is every document exported.
        // the amount of exported documents: each loaded document will be written 'ExportDocCount' times
        iExportDocCount = 25;

        // get the temp dir for creating the command scripts.
        m_aTempDir = new TempDir(util.utils.getOfficeTemp/*Dir*/(xMsf));

        // get the file extension, export filter connection
        // the import and export filters
        // store a file extension
        sDocTypeExportFilter = new String[3][2];
        sDocTypeExportFilter[0][0] = "sxw";
        sDocTypeExportFilter[0][1] = "writer_pdf_Export";
        sDocTypeExportFilter[1][0] = "sxc";
        sDocTypeExportFilter[1][1] = "calc_pdf_Export";
        sDocTypeExportFilter[2][0] = "sxi";
        sDocTypeExportFilter[2][1] = "impress_pdf_Export";

        // get files to load and export
        String sDocumentPath = TestDocument.getUrl();
        File f = new File(FileHelper.getJavaCompatibleFilename(sDocumentPath));
        sDocuments = new String[sDocTypeExportFilter.length][];
        for (int j = 0; j < sDocTypeExportFilter.length; j++)
        {
            FileFilter filter = new FileFilter(sDocTypeExportFilter[j][0]);
            String[] doc = f.list(filter);
            sDocuments[j] = new String[doc.length];
            for (int i = 0; i < doc.length; i++)
            {
                sDocuments[j][i] = TestDocument.getUrl(doc[i]);
            }
        }
    }

    /**
     * delete all created files on disk
     */
    @After
    public void after()
    {
    }

    /**
     * The test function: load documents and save them using the given filters
     * for each given document type.
     */
    @Test
    public void loadAndSaveDocuments() throws Exception
    {
        int nOk = 0;
        int nRunThrough = 0;

        // At first:
        // we load the document, there will be some post work in office like late initialisations
        // we store exact one time the document
        // so the memory footprint should be right

        // iterate over all document types
        for (int k = 0; k < sDocTypeExportFilter.length; k++)
        {
            // iterate over all documents of this type
            for (int i = 0; i < sDocuments[k].length; i++)
            {

                final String sDocument = sDocuments[k][i];
                final String sExtension = sDocTypeExportFilter[k][1];

                loadAndSaveNTimesDocument(sDocument, 1, sExtension);

            }
            System.out.println();
            System.out.println();
        }

        System.out.println("Wait for: " + 10000 + "ms");
        util.utils.pause(10000);

        // Now the real test, load document and store 25 times

        // iterate over all document types
        for (int k = 0; k < sDocTypeExportFilter.length; k++)
        {
            // iterate over all documents of this type
            for (int i = 0; i < sDocuments[k].length; i++)
            {

                final String sDocument = sDocuments[k][i];
                final String sExtension = sDocTypeExportFilter[k][1];

                OfficeMemchecker aChecker = new OfficeMemchecker();
                aChecker.setDocumentName(FileHelper.getBasename(sDocument));
                aChecker.setExtension(sExtension);
                aChecker.start();

                loadAndSaveNTimesDocument(sDocument, iExportDocCount, sExtension);

                aChecker.stop();
                final int nConsumMore = aChecker.getConsumMore();

                nOk += checkMemory(nConsumMore);
                nRunThrough++;
            }
            System.out.println();
            System.out.println();
        }
        System.out.println("Find the output of used 'pmap' here: " + m_aTempDir.getTempDir() + " if test failed.");
        assertTrue("Office consumes too many memory.", nOk == nRunThrough);
    }

    /**
     * Checks how much memory should consume
     * @param storageBefore
     * @return 1 if consume is ok, else 0
     */
    private int checkMemory(int nConsumMore)
    {
        int nAllowed = iAllowMemoryIncrease * iExportDocCount;
        System.out.println("The Office consumes now " + nConsumMore
                + "K more memory than at the start of the test; allowed were "
                + nAllowed + "K.");
        if (nConsumMore > nAllowed)
        {
            System.out.println("ERROR: This is not allowed.");
            return 0;
        }
        System.out.println("OK.");
        return 1;
    }

    /**
     * load and save exact one document
     */
    private void loadAndSaveNTimesDocument(String _sDocument, int _nCount, String _sStoreExtension) throws Exception
    {
        System.out.println("Document: " + _sDocument);
        XComponent xComponent = DesktopTools.loadDoc(getMSF(), _sDocument, null);
        XStorable xStorable = UnoRuntime.queryInterface(XStorable.class, xComponent);
        if (xStorable != null)
        {
            // export each document iExportDocCount times
            for (int j = 0; j < _nCount; j++)
            {
                final String sDocumentName = FileHelper.getBasename(_sDocument) + "_" + j + ".pdf";
                final String sFilename = FileHelper.appendPath(m_aTempDir.getOfficeTempDir(), sDocumentName);
                String url = sFilename; // graphical.FileHelper.getFileURLFromSystemPath(sFilename);
                try
                {
                    PropertyValue[] props = new PropertyValue[1];
                    props[0] = new PropertyValue();
                    props[0].Name = "FilterName";
                    // use export filter for this doc type
                    props[0].Value = _sStoreExtension;
                    xStorable.storeToURL(url, props);
                }
                catch (com.sun.star.io.IOException e)
                {
                    fail("Could not store to '" + url + "'");
                }
            }
            // close the doc
            XCloseable xCloseable = UnoRuntime.queryInterface(XCloseable.class, xStorable);
            xCloseable.close(true);
        }
        else
        {
            System.out.println("Cannot query for XStorable interface on document '" + _sDocument + "'");
            System.out.println(" -> Skipping storage.");
        }

    }


    private class OfficeMemchecker
    {

        /**
         * After called start() it contains the memory need at startup
         */
        private int m_nMemoryStart;
        /**
         * After called stop() it contains the memory usage
         */
        private int m_nMemoryUsage;
        private String m_sDocumentName;
        private String m_sExtension;

        public OfficeMemchecker()
        {
            m_nMemoryStart = 0;
        }

        public void setDocumentName(String _sDocName)
        {
            m_sDocumentName = _sDocName;
        }

        public void setExtension(String _sExt)
        {
            m_sExtension = _sExt;
        }

        public void start()
        {
            m_nMemoryStart = getOfficeMemoryUsage(createModeName("start", 0));
        }

        private String createModeName(String _sSub, int _nCount)
        {
            StringBuffer aBuf = new StringBuffer();
            aBuf.append(_sSub);
            aBuf.append('_').append(m_sDocumentName).append('_').append(m_sExtension);
            aBuf.append('_').append(_nCount);
            return aBuf.toString();
        }

        public void stop()
        {
            // short wait for the office to 'calm down' and free some memory
            System.out.println("Wait for: " + 20000 + "ms");
            util.utils.pause(20000);
            // wait util memory is not freed anymore.
            int storageAfter = getOfficeMemoryUsage(createModeName("stop", 0));
            int mem = 0;
            int count = 0;
            while (storageAfter != mem && count < 10)
            {
                count++;
                mem = storageAfter;
                storageAfter = getOfficeMemoryUsage(createModeName("stop", count));
                System.out.println("Wait for: " + 1000 + "ms");
                util.utils.pause(1000);
            }
            m_nMemoryUsage = (storageAfter - m_nMemoryStart);
        }

        public int getConsumMore()
        {
            return m_nMemoryUsage;
        }

        /**
         * Get the process ID from the Office
         * @return the Id as String
         */
        private String getOfficeProcessID()
        {
            String sProcessIdCommand = FileHelper.appendPath(m_aTempDir.getTempDir(), "getPS");
            final String sofficeArg = org.openoffice.test.Argument.get("soffice");
            final String sPSGrep = "ps -ef | grep $USER | grep <soffice>.bin | grep -v grep";
            final String sProcessId = sPSGrep.replaceAll("<soffice>", FileHelper.getJavaCompatibleFilename(sofficeArg));

            createExecutableFile(sProcessIdCommand, sProcessId);
            ProcessHandler processID = new ProcessHandler(sProcessIdCommand);
            processID.noOutput();
            processID.executeSynchronously();
            String text = processID.getOutputText();
            if (text == null || text.equals("") || text.indexOf(' ') == -1)
            {
                fail("Could not determine Office process ID. Check " + sProcessIdCommand);
            }
            StringTokenizer aToken = new StringTokenizer(text);
            // this is not nice, but ps gives the same output on every machine
            aToken.nextToken();
            String id = aToken.nextToken();
            return id;
        }

        /**
         * Get the memory usage of the Office in KByte.
         * @return The memory used by the Office.
         */
        private int getOfficeMemoryUsage(String _sMode)
        {
            final String sMemoryMonitor = "pmap <processID> |tee <pmapoutputfile> | grep total";
            String sOfficeMemoryCommand = null;
            sOfficeMemoryCommand = FileHelper.appendPath(m_aTempDir.getTempDir(), "getPmap");
            String command = sMemoryMonitor.replaceAll("<processID>", getOfficeProcessID());
            String sPmapOutputFile = FileHelper.appendPath(m_aTempDir.getTempDir(), "pmap_" + _sMode + ".txt");
            command = command.replaceAll("<pmapoutputfile>", sPmapOutputFile);
            createExecutableFile(sOfficeMemoryCommand, command);

            ProcessHandler processID = new ProcessHandler(sOfficeMemoryCommand);
            processID.noOutput();
            processID.executeSynchronously();
            int nError = processID.getExitCode();
            assertTrue("Execute of " + sOfficeMemoryCommand + " failed", nError == 0);
            String text = processID.getOutputText();
            if (text == null || text.equals("") || text.indexOf(' ') == -1)
            {
                fail("Could not determine Office memory usage. Check " + sOfficeMemoryCommand);
            }
            StringTokenizer aToken = new StringTokenizer(text);
            // this works, because the output of pmap is quite standardized.
            aToken.nextToken();
            String mem = aToken.nextToken();
            mem = mem.substring(0, mem.indexOf('K'));
            Integer memory = Integer.valueOf(mem);
            return memory.intValue();
        }

        /**
         * Write a script file and set its rights to rwxrwxrwx.
         * @param fileName The name of the created file
         * @param line The commandline that has to be written inside of the file.
         */
        private void createExecutableFile(String fileName, String line)
        {
            final String sChmod = "chmod a+x ";
            final String bash = "#!/bin/bash";

            try
            {
                String sFilename = FileHelper.getJavaCompatibleFilename(fileName);
                PrintWriter fWriter = new PrintWriter(new FileWriter(sFilename));
                fWriter.println(bash);
                fWriter.println(line);
                fWriter.close();
                // change rights to rwxrwxrwx
                ProcessHandler processID = new ProcessHandler(sChmod + sFilename);
                processID.noOutput();
                processID.executeSynchronously();
                int nError = processID.getExitCode();
                assertTrue("chmod failed. ", nError == 0);
            }
            catch (java.io.IOException e)
            {
            }
        }
    }

    /**
     * Own file filter, will just return ok for all files that end with a given
     * suffix
     */
    private class FileFilter implements FilenameFilter
    {

        private final String suffix;

        /**
         * C'tor.
         * @param suffix The suffix each filename should end with.
         */
        public FileFilter(String suffix)
        {
            this.suffix = suffix;
        }

        /**
         * Returns true, if the name of the file has the suffix given to the
         * c'tor.
         * @param name The filename that is tested.
         * @param file Not used.
         * @return True, if name ends with suffix.
         */
        public boolean accept(File file, String name)
        {
            return name.endsWith(suffix);
        }
    }

    private XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
    }

    // setup and close connections
    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection()
            throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();
}
