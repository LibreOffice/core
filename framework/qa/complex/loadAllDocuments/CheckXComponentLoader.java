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

//-----------------------------------------------
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
    //-------------------------------------------
    // some const

    /** used to classify the result of a loadComponentFromURL() request. */
    private static final int RESULT_UNKNOWN                  = 0;
    private static final int RESULT_VALID_DOC                = 1;
    private static final int RESULT_EMPTY_DOC                = 2;
    private static final int RESULT_ILLEGALARGUMENTEXCEPTION = 3;
    private static final int RESULT_IOEXCEPTION              = 4;
    private static final int RESULT_RUNTIMEEXCEPTION         = 5;
    private static final int RESULT_EXCEPTION                = 6;

    /** File/URL separators. */
    private static final String fs_url = "/";
    // private static final String fs_sys = System.getProperty("file.separator");

    /** used for testing password protected files. */
    private static final String SUFFIX_PASSWORD_TEMPFILE = "password_";
    private static final String PREFIX_PASSWORD_TEMPFILE = ".sxw";
    private static final String DEFAULT_PASSWORD         = "DefaultPasswordForComponentLoaderTest";

    //-------------------------------------------
    // member

    /** points to the global uno service manager. */
    private XMultiServiceFactory m_xMSF = null;

    /** provides XComponentLoader interface. */
    private XFrame m_xDesktop = null;

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

    //-------------------------------------------
    // test environment

    //-------------------------------------------
    /** @short  A function to tell the framework,
                which test functions are available.

        @return All test methods.
        @todo   Think about selection of tests from outside ...
     */
//    public String[] getTestMethodNames()
//    {
//        // TODO think about trigger of sub-tests from outside
//        return new String[]
//        {
//            "checkURLEncoding"           ,
//            "checkURLHandling"           ,
//            "checkUsingOfMediaDescriptor",
//            "checkStreamLoading"         ,
//            "checkLoadingWithPassword"
//        };
//    }

    //-------------------------------------------
    /** @short  Create the environment for following tests.

        @descr  Use either a component loader from desktop or
                from frame
     */
    @Before public void before()
    {
        // get uno service manager from global test environment
        m_xMSF = getMSF();

        // create stream provider
        try
        {
            m_xStreamProvider = UnoRuntime.queryInterface(XSimpleFileAccess.class, m_xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess"));
        }
        catch(java.lang.Throwable ex)
        {
            fail("Could not create a stream provider instance.");
        }

        // create desktop instance
        try
        {
            m_xDesktop = UnoRuntime.queryInterface(XFrame.class, m_xMSF.createInstance("com.sun.star.frame.Desktop"));
        }
        catch(java.lang.Throwable ex)
        {
            fail("Could not create the desktop instance.");
        }

        // create frame instance
        m_xFrame = m_xDesktop.findFrame("testFrame_componentLoader",
                                        FrameSearchFlag.TASKS | FrameSearchFlag.CREATE);
        assertNotNull("Couldn't create test frame.", m_xFrame);

        // define default loader for testing
        // TODO think about using of bot loader instances!
        m_xLoader = UnoRuntime.queryInterface(XComponentLoader.class, m_xDesktop);
        assertNotNull("Desktop service doesnt support needed component loader interface.", m_xLoader);

        // get temp path for this environment
        final String tempDirURL = util.utils.getOfficeTemp/*Dir*/(getMSF());
        m_sTempPath = graphical.FileHelper.getSystemPathFromFileURL(tempDirURL);
        // m_sTempPath = "."+fs_sys;

        // get all files from the given directory
        // TODO URLHelper should ignore directories!
        m_lTestFiles = new ArrayList<String>();
        final String sTestDocURL = OfficeFileUrl.getAbsolute(new File("testdocuments"));
        m_sTestDocPath = graphical.FileHelper.getSystemPathFromFileURL(sTestDocURL);
        try
        {
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

                // Some test files are checked into CVS. ignore CVS  helper files!
//                if (sSubPath.indexOf("CVS") > -1)
//                {
//                    continue;
//                }

                m_lTestFiles.add(sSubPath);
            }
        }
        catch(java.lang.Throwable ex)
        {
            fail("Couldn't find test documents.");
        }
    }

    //-------------------------------------------
    /** @short  close the environment.
     */
    @After public void after()
    {
        XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, m_xFrame);
        try
        {
            xClose.close(false);
        }
        catch(com.sun.star.util.CloseVetoException exVeto)
            { fail("Test frame couldn't be closed successfully."); }

        m_xFrame  = null;
        m_xLoader = null;
    }

    //-------------------------------------------
    /** @short  Look for files in the given directory for loading.
     */
    @Test public void checkUsingOfMediaDescriptor()
    {
        InteractionHandler xHandler   = new InteractionHandler();
        StatusIndicator    xIndicator = new StatusIndicator(StatusIndicator.SHOWSTATUS_LOG);

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

            if (/*! (sURL.endsWith(".jpg") ||
                   sURL.endsWith(".gif"))*/
                    true
                  )
            {
                loadURL(m_xLoader, RESULT_VALID_DOC, sURL, "_blank", 0, lProps);
                // Its not needed to reset this using states!
                // Its done internaly ...
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
    }

    //-------------------------------------------
    /** TODO document me and move this method to a more global helper! */
    private String impl_getTempFileName(String sTempPath,
                                        String sSuffix  ,
                                        String sPrefix  )
    {
        File aDir = new File(sTempPath);
        aDir.mkdirs();
//        if (!aDir.exists())
//        {
//            fail("Could not access temp directory \"" + sTempPath + "\".");
//        }

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

    //-------------------------------------------
    /** TODO document me and move this method to a more global helper! */
    private void impl_createTempOfficeDocument(XComponentLoader xLoader   ,
                                               String           sSourceURL,
                                               String           sTargetURL,
                                               String           sFilter   ,
                                               String           sPassword )
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
        try
        {
            // load it
            xDoc = xLoader.loadComponentFromURL(sSourceURL, "_blank", 0, lLoadProps);
            assertNotNull("Could create office document, which should be saved as temp one.", xDoc);

            // save it as temp file
            XStorable xStore = UnoRuntime.queryInterface(XStorable.class, xDoc);
            xStore.storeAsURL(sTargetURL, lSaveProps);

            // Dont forget to close this file. Otherwise the temp file is locked!
            XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xDoc);
            xClose.close(false);
        }
        catch(java.lang.Throwable ex)
        {
            fail("Could not create temp office document.");
        }
    }

    //-------------------------------------------
    /** @short  Check the password handling.

        @descr  The used password is the one given
                as password for the ftp connection,
                or - if none given a default one.
     */
    @Test public void checkLoadingWithPassword()
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
    @Test public void checkURLEncoding() {
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
            try {
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
            } catch (java.io.UnsupportedEncodingException e) {
                fail("Unsopported Encoding: " + sEncoding[i] +
                       "\n Not able to test encoding on this platform.");
            }
        }
    }

    /**
     * Check url handling with a load of URLs.
     * 1. unsupported URLs.
     * 2. "stupid" URLs
     * 3. case sensitive URLs
     * 4. FTP URLs
     * 5. HTTP URLs
     */
//    public void checkURLHandling() {
//        PropertyValue[] lProps = new PropertyValue[1];
//
//        lProps[0] = new PropertyValue();
//        lProps[0].Name = "Hidden";
//        lProps[0].Value = Boolean.TRUE;
//
//        System.out.println("check possible but unsupported URLs");
//
//        String[] sIllegalArgs = new String[] {
//            "slot:5000", "slot:10909", ".uno:SaveAs", ".uno:Open",
//        };
//        loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sIllegalArgs,
//                "_blank", 0, lProps);
//
//        System.out.println("check stupid URLs");
//
//        sIllegalArgs = new String[] {
//            "slot:xxx", "slot:111111111", ".uno:save_as", ".uno:open_this",
//            ".UnO:*",
//        };
//        loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sIllegalArgs,
//                "_blank", 0, lProps);
//
//        String[] sEmptyDocs = new String[] {
//            "mailo:hansi.meier@germany.sun.com", "file:/c:\\test/file.cxx",
//            "file:///c|:\\test/file.cxx", "http_server://staroffice-doc\\",
//            "c:\\\\test///\\test.sxw", "news_:staroffice-doc",
//            "newsletter@blubber", "private_factory/swriter",
//            "private:factory//swriter", "private:factory/swriter/___",
//            "c:\\test\\test.sxw", "macro:///ImportWizard.Main.Main",
//            "macro:///Euro.AutoPilotRun.StartAutoPilot",
//            "service:com.sun.star.frame.Frame",
//            "mailto:steffen.grund@germany.sun.com", "news:staroffice-doc",
//            "macro:/ExportWizard", "macro://Euro.AutoPilotRun.StartAutoPilot",
//            "service:com.sun.star.frame."
//        };
//
//        //with cws_loadenv01 changed to IllegalArgumentException
//        loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sEmptyDocs, "_blank", 0,
//                lProps);
//
//        System.out.println("check case senstive URLs");
//
//        sIllegalArgs = new String[] {
//            "sLot:5000", "sloT:10909", ".unO:SaveAs", ".uno:OPEN",
//        };
//        loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sIllegalArgs,
//                "_blank", 0, lProps);
//
//        sEmptyDocs = new String[] {
//            "private:factory/SWRITER", "private:factory/SWRITER/WEB",
//            "macro:///importwizard.main.main",
//            "Macro:///euro.autopilotrun.startautopilot",
//            "Service:Com.Sun.Star.Frame.Frame",
//            "Mailto:andreas.schluens@germany.sun.com", "neWs:staroffice-doc",
//            "News:Staroffice-doc"
//        };
//
//        //with cws_loadenv01 changed to IllegalArgumentException
//        loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sEmptyDocs, "_blank", 0,
//                lProps);
//
//        System.out.println("check FTP URLs");
//
//        String sFTPURL = (String) param.get("FtpAccess");
//        Enumeration aSnapshot = m_lTestFiles.elements();
//
//        while (aSnapshot.hasMoreElements()) {
//            String doc = (String) aSnapshot.nextElement();
//
//
//            // if os is windows
//            doc = doc.replace('\\', '/');
//      if (doc.indexOf("CVS")<0) {
//          loadURL(m_xLoader, RESULT_VALID_DOC, sFTPURL + "/" + doc,
//                    "_blank", 0, lProps);
//      }
//        }
//
//        System.out.println("check HTTP URLs");
//
//        String sHTTPURL = (String) param.get("HttpAccess");
//        aSnapshot = m_lTestFiles.elements();
//
//        while (aSnapshot.hasMoreElements()) {
//            String doc = (String) aSnapshot.nextElement();
//
//
//            // if os is windows
//            doc = doc.replace('\\', '/');
//      if (doc.indexOf("CVS")<0) {
//          loadURL(m_xLoader, RESULT_VALID_DOC, sHTTPURL + "/" + doc,
//                    "_blank", 0, lProps);
//      }
//        }
//    }

    /** TODo document me
     */
    @Test public void checkStreamLoading()
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

//            if (sURL.indexOf("CVS") > -1)
//            {
//                continue;
//            }

            try
            {
                XInputStream xStream = m_xStreamProvider.openFileRead(sURL);
                lProps[1].Value = xStream;
            }
            catch(com.sun.star.uno.Exception e)
            {
                fail("Could not open test file \""+sURL+"\" for stream test.");
            }

            // check different version of "private:stream" URL!
            loadURL(m_xLoader, RESULT_VALID_DOC, "private:stream" , "_blank", 0, lProps);
            // loadURL(m_xLoader, RESULT_VALID_DOC, "private:stream" , "_blank", 0, lProps);
            // loadURL(m_xLoader, RESULT_VALID_DOC, "private:stream/", "_blank", 0, lProps);
    }
    }

    /**
     * Loads one URL with the given parameters using the method
     * loadComponentFromURL(). Further it's possible to specify, whch result is
     * required and we check internally if it was reached. Logging of errors
     * and success stories is done inside this method too. Of course we catch
     * all possible exceptions and try to leave the office without any forgotten
     * but opened documents.
     */
    private void loadURL(XComponentLoader m_xLoader, int nRequiredResult,
                         String sURL, String sTarget, int nFlags,
                         PropertyValue[] lProps) {
        int nResult = RESULT_EMPTY_DOC;
        XComponent xDoc = null;

        try {
            xDoc = m_xLoader.loadComponentFromURL(sURL, sTarget, nFlags,
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
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
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
