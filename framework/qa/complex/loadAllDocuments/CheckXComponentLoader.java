/*************************************************************************
 *
 *  $RCSfile: CheckXComponentLoader.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Date: 2004-07-23 11:07:45 $
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

 package complex.loadAllDocuments;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XStorable;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import com.sun.star.ucb.XSimpleFileAccess;

import complex.loadAllDocuments.helper.InteractionHandler;
import complex.loadAllDocuments.helper.StatusIndicator;
import complex.loadAllDocuments.helper.StreamSimulator;

import complexlib.ComplexTestCase;

import helper.URLHelper;

import java.io.File;
import java.io.InputStreamReader;

import java.util.Enumeration;
import java.util.StringTokenizer;
import java.util.Vector;

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

    @todo   We need a further test for accessing UNC pathes on windows!
 */
public class CheckXComponentLoader extends ComplexTestCase
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
    private static final String fs_sys = System.getProperty("file.separator");

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
    private static Vector m_lTestFiles = null;

    //-------------------------------------------
    // test environment

    //-------------------------------------------
    /** @short  A function to tell the framework,
                which test functions are available.

        @return All test methods.
        @todo   Think about selection of tests from outside ...
     */
    public String[] getTestMethodNames()
    {
        // TODO think about trigger of sub-tests from outside
        return new String[]
        {
            "checkURLEncoding"           ,
            "checkURLHandling"           ,
            "checkUsingOfMediaDescriptor",
            "checkStreamLoading"         ,
            "checkLoadingWithPassword"
        };
    }

    //-------------------------------------------
    /** @short  Create the environment for following tests.

        @descr  Use either a component loader from desktop or
                from frame
     */
    public void before()
    {
        // get uno service manager from global test environment
        m_xMSF = (XMultiServiceFactory)param.getMSF();

        // create stream provider
        try
        {
            m_xStreamProvider = (XSimpleFileAccess)UnoRuntime.queryInterface(
                                                    XSimpleFileAccess.class,
                                                    m_xMSF.createInstance("com.sun.star.ucb.SimpleFileAccess"));
        }
        catch(java.lang.Throwable ex)
        {
            ex.printStackTrace();
            failed("Could not create a stream provider instance.");
        }

        // create desktop instance
        try
        {
            m_xDesktop = (XFrame)UnoRuntime.queryInterface(
                                    XFrame.class,
                                    m_xMSF.createInstance("com.sun.star.frame.Desktop"));
        }
        catch(java.lang.Throwable ex)
        {
            ex.printStackTrace();
            failed("Could not create the desktop instance.");
        }

        // create frame instance
        m_xFrame = m_xDesktop.findFrame("testFrame_componentLoader"                    ,
                                        FrameSearchFlag.TASKS | FrameSearchFlag.CREATE);
        if (m_xFrame==null)
            failed("Couldn't create test frame.");

        // define default loader for testing
        // TODO think about using of bot loader instances!
        m_xLoader = (XComponentLoader)UnoRuntime.queryInterface(
                                         XComponentLoader.class,
                                         m_xDesktop);
        if (m_xLoader==null)
            failed("Desktop service doesnt support needed component loader interface.");

        // get temp path for this environment
        m_sTempPath = (String) param.get("TempPath");
        m_sTempPath = "."+fs_sys;

        // get all files from the given directory
        // TODO URLHelper should ignore directories!
        m_lTestFiles = new Vector();
        m_sTestDocPath = (String) param.get("TestDocumentPath");
        try
        {
            File        aBaseDir        = new File(m_sTestDocPath);
            Vector      lDirContent     = URLHelper.getSystemFilesFromDir(aBaseDir.getPath());
            Enumeration lList           = lDirContent.elements();
            int         nBasePathLength = m_sTestDocPath.length();
            while(lList.hasMoreElements())
            {
                File aFile = (File)lList.nextElement();

                // ignore broken links and directories at all
                if (
                    (!aFile.exists()) ||
                    (!aFile.isFile())
                   )
                {
                    continue;
                }

                String sCompletePath = aFile.getAbsolutePath();
                String sSubPath      = sCompletePath.substring(nBasePathLength + 1);

                // Some test files are checked into CVS. ignore CVS  helper files!
                if (sSubPath.indexOf("CVS") > -1)
                    continue;

                m_lTestFiles.add(sSubPath);
            }
        }
        catch(java.lang.Throwable ex)
        {
            ex.printStackTrace();
            failed("Couldn't find test documents.");
        }
    }

    //-------------------------------------------
    /** @short  close the environment.
     */
    public void after()
    {
        XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(
                                            XCloseable.class,
                                            m_xFrame);
        try
        {
            xClose.close(false);
        }
        catch(com.sun.star.util.CloseVetoException exVeto)
            { failed("Test frame couldn't be closed successfully."); }

        m_xFrame  = null;
        m_xLoader = null;
    }

    //-------------------------------------------
    /** @short  Look for files in the given directory for loading.
     */
    public void checkUsingOfMediaDescriptor()
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

        Enumeration aSnapshot = m_lTestFiles.elements();
        while (aSnapshot.hasMoreElements())
        {
            File   aSysFile = new File(m_sTestDocPath+fs_sys+(String)aSnapshot.nextElement());
            String sURL     = URLHelper.getFileURLFromSystemPath(aSysFile);

            loadURL(m_xLoader, RESULT_VALID_DOC, sURL, "_blank", 0, lProps);

            // Its not needed to reset this using states!
            // Its done internaly ...
            if (!xIndicator.wasUsed())
                failed("External progress was not used for loading.");
            if (xHandler.wasUsed())
                failed("External interaction handler was not used for loading.");
        }
    }

    //-------------------------------------------
    /** TODO document me and move this method to a more global helper! */
    private String impl_getTempFileName(String sTempPath,
                                        String sSuffix  ,
                                        String sPrefix  )
    {
        File aDir = new File(sTempPath);
        if (!aDir.exists())
            failed("Could not access temp directory \""+sTempPath+"\".");

        for (int i=0; i<999999; ++i)
        {
            File aTempFile = new File(aDir, sSuffix+i+sPrefix);
            if (!aTempFile.exists())
                return aTempFile.getAbsolutePath();
        }

        failed("Seems that all temp file names are currently in use!");
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
            if (xDoc == null)
                failed("Could create office document, which should be saved as temp one.");

            // save it as temp file
            XStorable xStore = (XStorable)UnoRuntime.queryInterface(
                                            XStorable.class,
                                            xDoc);
            xStore.storeAsURL(sTargetURL, lSaveProps);

            // Dont forget to close this file. Otherwise the temp file is locked!
            XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(
                                                XCloseable.class,
                                                xDoc);
            xClose.close(false);
        }
        catch(java.lang.Throwable ex)
        {
            ex.printStackTrace();
            failed("Could not create temp office document.");
        }
    }

    //-------------------------------------------
    /** @short  Check the password handling.

        @descr  The used password is the one given
                as password for the ftp connection,
                or - if none given a default one.
     */
    public void checkLoadingWithPassword()
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
        loadURL(m_xLoader, RESULT_EMPTY_DOC, sTestURL, "_blank", 0, lArgs2);
    }

    /**
     * Check URL encoding. The first filename that matches "*.sxw"
     * is used as source for several encodings.
     */
    public void checkURLEncoding() {
        PropertyValue[] lProps = new PropertyValue[1];

        lProps[0] = new PropertyValue();
        lProps[0].Name = "Hidden";
        lProps[0].Value = Boolean.TRUE;

        // first get encoding of this system
        InputStreamReader in = new InputStreamReader(System.in);
        String sSystemEncoding = in.getEncoding();

        log.println("This system's encoding: " + sSystemEncoding);

        if (m_lTestFiles == null) {
            failed("Found an empty directory. There are no files for testing.");

            return;
        }

        // get a file name as byte array
        Enumeration aSnapshot = m_lTestFiles.elements();
        byte[] baURL = null;

        while (aSnapshot.hasMoreElements()) {
            File aFile = new File(m_sTestDocPath + fs_sys +
                                  aSnapshot.nextElement());
            String sFile = URLHelper.getFileURLFromSystemPath(aFile);

            // take the first sxw file as stream
            if (sFile.endsWith(".sxw")) {
                baURL = sFile.getBytes();

                break;
            }
        }

        if (baURL == null) {
            failed("Found no file to load. Cannot test.");

            return;
        }

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
                log.println("ENC[" + sEncoding[i] + "]");

                if (sEncoding[i + 1].equals("TRUE")) {
                    loadURL(m_xLoader, RESULT_VALID_DOC, encURL, "_blank", 0,
                            lProps);
                } else {
                    //with cws_loadenv01 changed to IllegalArgumentException
                    loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, encURL, "_blank", 0,
                            lProps);
                }
            } catch (java.io.UnsupportedEncodingException e) {
                failed("Unsopported Encoding: " + sEncoding[i] +
                       "\n Not able to test encoding on this platform.", true);
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
    public void checkURLHandling() {
        PropertyValue[] lProps = new PropertyValue[1];

        lProps[0] = new PropertyValue();
        lProps[0].Name = "Hidden";
        lProps[0].Value = Boolean.TRUE;

        log.println("check possible but unsupported URLs");

        String[] sIllegalArgs = new String[] {
            "slot:5000", "slot:10909", ".uno:SaveAs", ".uno:Open",
        };
        loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sIllegalArgs,
                "_blank", 0, lProps);

        log.println("check stupid URLs");

        sIllegalArgs = new String[] {
            "slot:xxx", "slot:111111111", ".uno:save_as", ".uno:open_this",
            ".UnO:*",
        };
        loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sIllegalArgs,
                "_blank", 0, lProps);

        String[] sEmptyDocs = new String[] {
            "mailo:hansi.meier@germany.sun.com", "file:/c:\\test/file.cxx",
            "file:///c|:\\test/file.cxx", "http_server://staroffice-doc\\",
            "c:\\\\test///\\test.sxw", "news_:staroffice-doc",
            "newsletter@blubber", "private_factory/swriter",
            "private:factory//swriter", "private:factory/swriter/___",
            "c:\\test\\test.sxw", "macro:///ImportWizard.Main.Main",
            "macro:///Euro.AutoPilotRun.StartAutoPilot",
            "service:com.sun.star.frame.Frame",
            "mailto:steffen.grund@germany.sun.com", "news:staroffice-doc",
            "macro:/ExportWizard", "macro://Euro.AutoPilotRun.StartAutoPilot",
            "service:com.sun.star.frame."
        };

        //with cws_loadenv01 changed to IllegalArgumentException
        loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sEmptyDocs, "_blank", 0,
                lProps);

        log.println("check case senstive URLs");

        sIllegalArgs = new String[] {
            "sLot:5000", "sloT:10909", ".unO:SaveAs", ".uno:OPEN",
        };
        loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sIllegalArgs,
                "_blank", 0, lProps);

        sEmptyDocs = new String[] {
            "private:factory/SWRITER", "private:factory/SWRITER/WEB",
            "macro:///importwizard.main.main",
            "Macro:///euro.autopilotrun.startautopilot",
            "Service:Com.Sun.Star.Frame.Frame",
            "Mailto:andreas.schluens@germany.sun.com", "neWs:staroffice-doc",
            "News:Staroffice-doc"
        };

        //with cws_loadenv01 changed to IllegalArgumentException
        loadURL(m_xLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sEmptyDocs, "_blank", 0,
                lProps);

        log.println("check FTP URLs");

        String sFTPURL = (String) param.get("FtpAccess");
        Enumeration aSnapshot = m_lTestFiles.elements();

        while (aSnapshot.hasMoreElements()) {
            String doc = (String) aSnapshot.nextElement();


            // if os is windows
            doc = doc.replace('\\', '/');
        if (doc.indexOf("CVS")<0) {
            loadURL(m_xLoader, RESULT_VALID_DOC, sFTPURL + "/" + doc,
                    "_blank", 0, lProps);
        }
        }

        log.println("check HTTP URLs");

        String sHTTPURL = (String) param.get("HttpAccess");
        aSnapshot = m_lTestFiles.elements();

        while (aSnapshot.hasMoreElements()) {
            String doc = (String) aSnapshot.nextElement();


            // if os is windows
            doc = doc.replace('\\', '/');
        if (doc.indexOf("CVS")<0) {
            loadURL(m_xLoader, RESULT_VALID_DOC, sHTTPURL + "/" + doc,
                    "_blank", 0, lProps);
        }
        }
    }

    /** TODo document me
     */
    public void checkStreamLoading()
    {
        PropertyValue[] lProps = new PropertyValue[2];

        lProps[0] = new PropertyValue();
        lProps[0].Name = "Hidden";
        lProps[0].Value = Boolean.TRUE;

        lProps[1] = new PropertyValue();
        lProps[1].Name = "InputStream";

        Enumeration aSnapshot = m_lTestFiles.elements();
        while (aSnapshot.hasMoreElements())
        {
            File   aFile = new File(m_sTestDocPath + fs_sys + (String) aSnapshot.nextElement());
            String sURL  = URLHelper.getFileURLFromSystemPath(aFile);

            if (sURL.indexOf("CVS") > -1)
                continue;

            try
            {
                XInputStream xStream = m_xStreamProvider.openFileRead(sURL);
                lProps[1].Value = xStream;
            }
            catch(com.sun.star.uno.Exception e)
                { failed("Could not open test file \""+sURL+"\" for stream test."); }

            // check different version of "private:stream" URL!
            loadURL(m_xLoader, RESULT_VALID_DOC, "private:stream" , "_blank", 0, lProps);
            loadURL(m_xLoader, RESULT_VALID_DOC, "private:stream/", "_blank", 0, lProps);
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
            exRuntime.printStackTrace();
            nResult = RESULT_RUNTIMEEXCEPTION;
        } catch (Exception e) {
            e.printStackTrace();
            nResult = RESULT_EXCEPTION;
        }

        try {
            if (xDoc != null) {
                xDoc.dispose();
                xDoc = null;
            }
        } catch (com.sun.star.uno.RuntimeException exClosing) {
            log.println("exception during disposing of a document found!" +
                        " Doesn't influence test - but should be checked.");
        }

        String sMessage = "URL[\"" + sURL + "\"]";

        if (nResult == nRequiredResult) {
            log.println(sMessage + " expected result [" +
                        convertResult2String(nResult) + "] ");
        } else {
            failed(sMessage + " unexpected result [" +
                   convertResult2String(nResult) + "] " +
                   "\nrequired was [" +
                   convertResult2String(nRequiredResult) + "]" +
                   "\nwe got       [" + convertResult2String(nResult) + "]",
                   true);
        }
    }

    private void loadURL(XComponentLoader m_xLoader, int nRequiredResult,
                         String[] sURL, String sTarget, int nFlags,
                         PropertyValue[] lProps) {
        for (int i = 0; i < sURL.length; i++)
            loadURL(m_xLoader, nRequiredResult, sURL[i], sTarget, nFlags,
                    lProps);
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
}