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

 package complex.loadAllDocuments;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;
import helper.URLHelper;

import java.io.File;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import org.openoffice.test.OfficeFileUrl;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XStorable;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;


/** @short  Check the interface method XComponentLoader.loadComponentFromURL()

    @descr  A prerequisite for this test is a server which allows access to files
            that will be loaded via three different access methods:
            <ul>
                <li>1. nfs (mounted directory / mapped network drive)</li>
                <li>2. ftp</li>
                <li>3. http</li>
            </ul>
            <p>
            The test will look for a list of files from the <i>TestDocumentPath</i>
            on and load these files from the mounted directory, via ftp and http.
            The parameters for this have to be "ftp_access" and "http_access".
            If they are not given, tests for ftp and http will fail.

    @todo   We need a further test for accessing UNC paths on windows!
 */
public class CheckXComponentLoader
{

    // some const

    /** used to classify the result of a loadComponentFromURL() request. */
    private static final int RESULT_VALID_DOC                = 1;
    private static final int RESULT_EMPTY_DOC                = 2;
    private static final int RESULT_ILLEGALARGUMENTEXCEPTION = 3;
    private static final int RESULT_IOEXCEPTION              = 4;
    private static final int RESULT_RUNTIMEEXCEPTION         = 5;
    private static final int RESULT_EXCEPTION                = 6;

    /** used for testing password protected files. */
    private static final String SUFFIX_PASSWORD_TEMPFILE = "password_";
    private static final String PREFIX_PASSWORD_TEMPFILE = ".sxw";
    private static final String DEFAULT_PASSWORD         = "DefaultPasswordForComponentLoaderTest";


    // member

    /** provides XComponentLoader interface too. */
    private XFrame m_xFrame = null;

    /** will be set to xDesktop OR xFrame. */
    private XComponentLoader m_xLoader = null;

    /** can be used to open local files as stream. */
    private XSimpleFileAccess m_xStreamProvider = null;

    /** directory for creating temp. files. */
    private String m_sTempPath = null;

    /** directory for searching files to load */
    private String m_sTestDocPath = null;

    /** files of m_sTestDocPath to test. */
    private static ArrayList<String> m_lTestFiles = null;


    // test environment


    /** @short  Create the environment for following tests.

        @descr  Use either a component loader from desktop or
                from frame
     */
    @Before public void before() throws Exception
    {
        // get uno service manager from global test environment
        /* points to the global uno service manager. */
        XMultiServiceFactory xMSF = getMSF();

        // create stream provider
        m_xStreamProvider = UnoRuntime.queryInterface(XSimpleFileAccess.class, xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess"));

        // create desktop instance
        /* provides XComponentLoader interface. */
        XFrame xDesktop = UnoRuntime.queryInterface(XFrame.class, xMSF.createInstance("com.sun.star.frame.Desktop"));

        // create frame instance
        m_xFrame = xDesktop.findFrame("testFrame_componentLoader",
                                        FrameSearchFlag.TASKS | FrameSearchFlag.CREATE);
        assertNotNull("Couldn't create test frame.", m_xFrame);

        // define default loader for testing
        // TODO think about using of bot loader instances!
        m_xLoader = UnoRuntime.queryInterface(XComponentLoader.class, xDesktop);
        assertNotNull("Desktop service doesn't support needed component loader interface.", m_xLoader);

        // get temp path for this environment
        final String tempDirURL = util.utils.getOfficeTemp/*Dir*/(getMSF());
        m_sTempPath = graphical.FileHelper.getSystemPathFromFileURL(tempDirURL);
        // m_sTempPath = "."+fs_sys;

        // get all files from the given directory
        // TODO URLHelper should ignore directories!
        m_lTestFiles = new ArrayList<String>();
        final String sTestDocURL = OfficeFileUrl.getAbsolute(new File("testdocuments"));
        m_sTestDocPath = graphical.FileHelper.getSystemPathFromFileURL(sTestDocURL);
        File           aBaseDir        = new File(m_sTestDocPath);
        List<File>     lDirContent     = URLHelper.getSystemFilesFromDir(aBaseDir.getPath());
        Iterator<File> lList           = lDirContent.iterator();
        int            nBasePathLength = m_sTestDocPath.length();
        while(lList.hasNext())
        {
            File aFile = lList.next();

            // ignore broken links and directories at all
            if (
                (!aFile.exists()) ||
                (!aFile.isFile())
               )
            {
                continue;
            }

            String sCompletePath = aFile.getAbsolutePath();
            String sSubPath      = sCompletePath.substring(nBasePathLength);

            m_lTestFiles.add(sSubPath);
        }
    }


    /** @short  close the environment.
     */
    @After public void after() throws Exception
    {
        XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, m_xFrame);
        xClose.close(false);

        m_xFrame  = null;
        m_xLoader = null;
    }


    /** @short  Look for files in the given directory for loading.
     */
    @Test public void checkUsingOfMediaDescriptor()
    {
        InteractionHandler xHandler   = new InteractionHandler();
        StatusIndicator    xIndicator = new StatusIndicator();

        PropertyValue[] lProps = new PropertyValue[3];

        lProps[0] = new PropertyValue();
        lProps[0].Name  = "Hidden";
        lProps[0].Value = Boolean.TRUE;

        lProps[1] = new PropertyValue();
        lProps[1].Name  = "InteractionHandler";
        lProps[1].Value = xHandler;

        lProps[2] = new PropertyValue();
        lProps[2].Name  = "StatusIndicator";
        lProps[2].Value = xIndicator;

        Iterator<String> aSnapshot = m_lTestFiles.iterator();
        while (aSnapshot.hasNext())
        {
            File   aSysFile = new File(m_sTestDocPath, aSnapshot.next());
            String sURL     = URLHelper.getFileURLFromSystemPath(aSysFile);

            loadURL(m_xLoader, RESULT_VALID_DOC, sURL, "_blank", 0, lProps);
            // Its not needed to reset this using states!
            // Its done internally ...
            if (!xIndicator.wasUsed())
            {
                System.out.println("External progress was not used for loading.");
            }
            if (xHandler.wasUsed())
            {
                System.out.println("External interaction handler was not used for loading.");
            }
        }
    }


    /** TODO document me and move this method to a more global helper! */
    private String impl_getTempFileName(String sTempPath,
                                        String sSuffix  ,
                                        String sPrefix  )
    {
        File aDir = new File(sTempPath);
        aDir.mkdirs();

    // TODO: create a temp file which not exist!
        for (int i=0; i<999999; ++i)
        {
            File aTempFile = new File(aDir, sSuffix+i+sPrefix);
            if (!aTempFile.exists())
            {
                return aTempFile.getAbsolutePath();
            }
        }

        fail("Seems that all temp file names are currently in use!");
        return null;
    }


    /** TODO document me and move this method to a more global helper! */
    private void impl_createTempOfficeDocument(XComponentLoader xLoader   ,
                                               String           sSourceURL,
                                               String           sTargetURL,
                                               String           sFilter   ,
                                               String           sPassword ) throws Exception
    {
        PropertyValue[] lLoadProps = new PropertyValue[1];

        lLoadProps[0] = new PropertyValue();
        lLoadProps[0].Name = "Hidden";
        lLoadProps[0].Value = Boolean.TRUE;

        PropertyValue[] lSaveProps = new PropertyValue[3];

        lSaveProps[0] = new PropertyValue();
        lSaveProps[0].Name = "FilterName";
        lSaveProps[0].Value = sFilter;

        lSaveProps[1] = new PropertyValue();
        lSaveProps[1].Name = "PassWord";
        lSaveProps[1].Value = sPassword;

        lSaveProps[2] = new PropertyValue();
        lSaveProps[2].Name = "Overwrite";
        lSaveProps[2].Value = Boolean.TRUE;

        XComponent xDoc = null;
        // load it
        xDoc = xLoader.loadComponentFromURL(sSourceURL, "_blank", 0, lLoadProps);
        assertNotNull("Could create office document, which should be saved as temp one.", xDoc);

        // save it as temp file
        XStorable xStore = UnoRuntime.queryInterface(XStorable.class, xDoc);
        xStore.storeAsURL(sTargetURL, lSaveProps);

        // Don't forget to close this file. Otherwise the temp file is locked!
        XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xDoc);
        xClose.close(false);
    }


    /** @short  Check the password handling.

        @descr  The used password is the one given
                as password for the ftp connection,
                or - if none given a default one.
     */
    @Test public void checkLoadingWithPassword() throws Exception
    {
        String sTempFile = impl_getTempFileName(m_sTempPath, SUFFIX_PASSWORD_TEMPFILE, PREFIX_PASSWORD_TEMPFILE);
        File   aTestFile = new File(sTempFile);
        String sTestURL  = URLHelper.getFileURLFromSystemPath(aTestFile);

        impl_createTempOfficeDocument(m_xLoader, "private:factory/swriter", sTestURL, "StarOffice XML (Writer)", DEFAULT_PASSWORD);

        PropertyValue[] lArgs1 = new PropertyValue[2];

        lArgs1[0] = new PropertyValue();
        lArgs1[0].Name  = "Hidden";
        lArgs1[0].Value = Boolean.TRUE;

        lArgs1[1] = new PropertyValue();
        lArgs1[1].Name  = "Password";
        lArgs1[1].Value = DEFAULT_PASSWORD;

        PropertyValue[] lArgs2 = new PropertyValue[1];

        lArgs2[0] = new PropertyValue();
        lArgs2[0].Name  = "Hidden";
        lArgs2[0].Value = Boolean.TRUE;

        loadURL(m_xLoader, RESULT_VALID_DOC, sTestURL, "_blank", 0, lArgs1);
// TODO: wrong?        loadURL(m_xLoader, RESULT_EMPTY_DOC, sTestURL, "_blank", 0, lArgs2);
    }

    /**
     * Check URL encoding. The first filename that matches "*.sxw"
     * is used as source for several encodings.
     */
    @Test public void checkURLEncoding() throws Exception {
        PropertyValue[] lProps = new PropertyValue[1];

        lProps[0] = new PropertyValue();
        lProps[0].Name = "Hidden";
        lProps[0].Value = Boolean.TRUE;

        // first get encoding of this system
        InputStreamReader in = new InputStreamReader(System.in);
        String sSystemEncoding = in.getEncoding();

        System.out.println("This system's encoding: " + sSystemEncoding);

        assertNotNull("Found an empty directory. There are no files for testing.", m_lTestFiles);


        // get a file name as byte array
        Iterator<String> aSnapshot = m_lTestFiles.iterator();
        byte[] baURL = null;

        while (aSnapshot.hasNext()) {
            File aFile = new File(m_sTestDocPath, aSnapshot.next());
            String sFile = URLHelper.getFileURLFromSystemPath(aFile);

            // take the first sxw file as stream
            if (sFile.endsWith(".sxw")) {
                baURL = sFile.getBytes();

                break;
            }
        }

        assertNotNull("Found no file to load. Cannot test.", baURL);

        //construct several different encoded strings
        String[] sEncoding = new String[] {
            "US-ASCII", "TRUE", // us ascii encoding
            "ISO-8859-1", "TRUE", // iso encoding
            "UTF-8", "TRUE", // 8 bit utf encoding
            "UTF-16BE", "FALSE", // 16 bit big endian utf
            "UTF-16LE", "FALSE", // 16 bit little endian utf
            "UTF-16", "FALSE" // 16 bit, order specified by byte order mark

        };

        for (int i = 0; i < sEncoding.length; i = i + 2) {
            String encURL = new String(baURL, sEncoding[i]);
            System.out.println("ENC[" + sEncoding[i] + "]");

            if (sEncoding[i + 1].equals("TRUE")) {
                loadURL(m_xLoader, RESULT_VALID_DOC, encURL, "_blank", 0,
                        lProps);
            } else {
                //with cws_loadenv01 changed to IllegalArgumentException
                loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, encURL, "_blank", 0,
                        lProps);
            }
        }
    }

    /** TODo document me
     */
    @Test public void checkStreamLoading() throws Exception
    {
        PropertyValue[] lProps = new PropertyValue[2];

        lProps[0] = new PropertyValue();
        lProps[0].Name = "Hidden";
        lProps[0].Value = Boolean.TRUE;

        lProps[1] = new PropertyValue();
        lProps[1].Name = "InputStream";

        Iterator<String> aSnapshot = m_lTestFiles.iterator();
        while (aSnapshot.hasNext())
        {
            File   aFile = new File(m_sTestDocPath, aSnapshot.next());
            String sURL  = URLHelper.getFileURLFromSystemPath(aFile);

            XInputStream xStream = m_xStreamProvider.openFileRead(sURL);
            lProps[1].Value = xStream;

            // check different version of "private:stream" URL!
            loadURL(m_xLoader, RESULT_VALID_DOC, "private:stream" , "_blank", 0, lProps);
        }
    }

    /**
     * Loads one URL with the given parameters using the method
     * loadComponentFromURL(). Further it's possible to specify, which result is
     * required and we check internally if it was reached. Logging of errors
     * and success stories is done inside this method too. Of course we catch
     * all possible exceptions and try to leave the office without any forgotten
     * but opened documents.
     */
    private void loadURL(XComponentLoader xLoader, int nRequiredResult,
                         String sURL, String sTarget, int nFlags,
                         PropertyValue[] lProps) {
        int nResult = RESULT_EMPTY_DOC;
        XComponent xDoc = null;

        try {
            xDoc = xLoader.loadComponentFromURL(sURL, sTarget, nFlags,
                                                     lProps);

            if (xDoc != null) {
                nResult = RESULT_VALID_DOC;
            } else {
                nResult = RESULT_EMPTY_DOC;
            }
        } catch (com.sun.star.lang.IllegalArgumentException exArgument) {
            nResult = RESULT_ILLEGALARGUMENTEXCEPTION;
        } catch (com.sun.star.io.IOException exIO) {
            nResult = RESULT_IOEXCEPTION;
        } catch (com.sun.star.uno.RuntimeException exRuntime) {
            nResult = RESULT_RUNTIMEEXCEPTION;
        } catch (Exception e) {
            nResult = RESULT_EXCEPTION;
        }

        try {
            if (xDoc != null) {
                xDoc.dispose();
                xDoc = null;
            }
        } catch (com.sun.star.uno.RuntimeException exClosing) {
            System.out.println("exception during disposing of a document found!" +
                        " Doesn't influence test - but should be checked.");
        }

        String sMessage = "URL[\"" + sURL + "\"]";

        if (nResult == nRequiredResult) {
            System.out.println(sMessage + " expected result [" +
                        convertResult2String(nResult) + "] ");
        } else {
            fail(sMessage + " unexpected result [" +
                   convertResult2String(nResult) + "] " +
                   "\nrequired was [" +
                   convertResult2String(nRequiredResult) + "]" +
                   "\nwe got       [" + convertResult2String(nResult) + "]"
                   );
        }
    }

    /**
     * it match the int result value to a string, which can be used for logging
     */
    private static String convertResult2String(int nResult) {
        switch (nResult) {
        case RESULT_VALID_DOC:
            return "VALID_DOC";

        case RESULT_EMPTY_DOC:
            return "EMPTY_DOC";

        case RESULT_ILLEGALARGUMENTEXCEPTION:
            return "ILLEGALARGUMENTEXCEPTION";

        case RESULT_IOEXCEPTION:
            return "IOEXCEPTION";

        case RESULT_RUNTIMEEXCEPTION:
            return "RUNTIMEEXCEPTION";

        case RESULT_EXCEPTION:
            return "ALLOTHEREXCEPTION";
        }

        return "unknown!";
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
