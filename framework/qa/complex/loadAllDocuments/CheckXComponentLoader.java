/*************************************************************************
 *
 *  $RCSfile: CheckXComponentLoader.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-04-21 12:02:45 $
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


/**
 * Check "loadComponentFromURL"
 * A prerequisite for this test is a server which allows access to files
 * that will be loaded via three different access methods:
 * 1. nfs (mounted directory / mapped network drive)
 * 2. ftp
 * 3. http
 * The test will look for a list of files from the TestDocumentPath on and
 * load these files from the mounted directory, via ftp and http.
 * The parameters for this have to be "ftp_access" and "http_access".
 * If they are not given, tests for ftp and http will fail.
 */
public class CheckXComponentLoader extends ComplexTestCase {
    private static XMultiServiceFactory xMSF;

    /**
     * some const
     */
    private static final int RESULT_UNKNOWN = 0;
    private static final int RESULT_VALID_DOC = 1;
    private static final int RESULT_EMPTY_DOC = 2;
    private static final int RESULT_ILLEGALARGUMENTEXCEPTION = 3;
    private static final int RESULT_IOEXCEPTION = 4;
    private static final int RESULT_RUNTIMEEXCEPTION = 5;
    private static final int RESULT_EXCEPTION = 6;
    static XFrame xDesktop;
    static XFrame xFrame;
    static XComponentLoader xFrameLoader;

    // directory for seraching files to load
    static String sTestDocPath = null;

    // files to test
    static Vector lTestFiles = null;

    // debug mode with lots of output: deprecated
    static boolean bDebug = false;

    // test mode: test component loader on desktop or on frame;
    // default is false, meaning frame
    static boolean bTestDesktopLoader = true;

    // File separator
    static String fs = "/";

    /**
     * A function to tell the framework, which test functions are available.
     * @return All test methods.
     */
    public String[] getTestMethodNames() {
        return new String[] {
            "checkLoading", "checkPasswordHandling", "checkURLEncoding",
            "checkURLHandling"
        };
    }

    /**
     * Create the environment for following tests.
     * Use either a component loader from desktop or
     * from frame
     */
    public void before() {
        xMSF = (XMultiServiceFactory) param.getMSF();

        if (xDesktop == null) {
            try {
                xDesktop = (XFrame) UnoRuntime.queryInterface(XFrame.class,
                                                              xMSF.createInstance(
                                                                      "com.sun.star.frame.Desktop"));
            } catch (com.sun.star.uno.Exception e) {
                failed("Could not create a desktop instance.");
            }
        }

        xFrame = xDesktop.findFrame("testFrame_componentLoader",
                                    FrameSearchFlag.TASKS |
                                    FrameSearchFlag.CREATE);

        XComponentLoader xDesktopLoader = (XComponentLoader) UnoRuntime.queryInterface(
                                                  XComponentLoader.class,
                                                  xDesktop);

        XComponentLoader aFrameLoader = (XComponentLoader) UnoRuntime.queryInterface(
                                                XComponentLoader.class, xFrame);

        if ((xDesktopLoader == null) || (aFrameLoader == null)) {
            failed("Couldn't create desktop or test frame " +
                   "with the required loader interface");
        }

        if (bTestDesktopLoader) {
            xFrameLoader = xDesktopLoader;
        } else {
            xFrameLoader = aFrameLoader;
        }


        // get all files from the given directory
        sTestDocPath = (String) param.get("TestDocumentPath");
        if (lTestFiles == null) {
            try {
                File aBase = new File(sTestDocPath);
                lTestFiles = URLHelper.getSystemFilesFromDir(aBase.getPath());
            } catch (java.lang.NullPointerException e) {
            e.printStackTrace();
            }
        }

        for (int i = 0; i < lTestFiles.size(); i++) {
            String completePath = ((File) lTestFiles.get(i)).getAbsolutePath();
            String subPath = completePath.substring(sTestDocPath.length() -1 );
        String fs_sys = System.getProperty("file.separator");
        File tempFile = new File(sTestDocPath+fs_sys+subPath);
        if (!tempFile.exists()) {
        subPath = subPath.substring(2);
        }
            lTestFiles.set(i, subPath);
        }

        fs = System.getProperty("file.separator");
    }

    /**
     * close the environment
     */
    public void after() {
        log.println("try to close the test frame which " +
                    "implements the new loader interface ... ");

        XCloseable xClose = (XCloseable) UnoRuntime.queryInterface(
                                    XCloseable.class, xFrame);
        xFrame = null;
        xFrameLoader = null;

        try {
            xClose.close(false);
        } catch (com.sun.star.util.CloseVetoException exVeto) {
            XComponent xDispose = (XComponent) UnoRuntime.queryInterface(
                                          XComponent.class, xClose);
            xDispose.dispose();
        }
    }

    /**
     * Look for files in the given directory for loading.
     */
    public void checkLoading() {
        File aBase = null;

        if (lTestFiles == null) {
            log.println(
                    "Found an empty directory. There are no files for testing.");

            return;
        }

        Enumeration aSnapshot = lTestFiles.elements();
        String sURL = null;
        PropertyValue[] lProps = new PropertyValue[3];

        lProps[0] = new PropertyValue();
        lProps[0].Name = "Hidden";
        lProps[0].Value = Boolean.TRUE;

        lProps[1] = new PropertyValue();
        lProps[1].Name = "InteractionHandler";

        InteractionHandler xHandler = new InteractionHandler();
        lProps[1].Value = xHandler;

        lProps[2] = new PropertyValue();
        lProps[2].Name = "StatusIndicator";

        StatusIndicator xIndicator = new StatusIndicator(
                                             StatusIndicator.SHOWSTATUS_LOG);
        lProps[2].Value = xIndicator;

        while (aSnapshot.hasMoreElements()) {
            File aFile = new File(sTestDocPath + fs +
                                  (String) aSnapshot.nextElement());
            sURL = URLHelper.getFileURLFromSystemPath(aFile);
        if (sURL.indexOf("CVS") < 0) {
            loadURL(xFrameLoader, RESULT_VALID_DOC, sURL, "_blank", 0, lProps);
        }

            if (xIndicator.wasUsed()) {
                log.println("Indicator was used");
            }

            if (xHandler.wasUsed()) {
                log.println("Interaction was used");
            }
        }
    }

    /**
     * Check the password handling. The used password is the one given
     * as password for the ftp connection, or - if none given, "MyPassword"
     * as default.
     */
    public void checkPasswordHandling() {
        File aTestFile = new File("password_check.sxw");
        String sPassword = "MyPassword";
        String sTestURL = URLHelper.getFileURLFromSystemPath(aTestFile);
        XComponent xDoc = null;
        XStorable xStore = null;

        PropertyValue[] lLoadProps = new PropertyValue[1];
        lLoadProps[0] = new PropertyValue();
        lLoadProps[0].Name = "Hidden";
        lLoadProps[0].Value = Boolean.TRUE;

        PropertyValue[] lSaveProps = new PropertyValue[2];
        lSaveProps[0] = new PropertyValue();
        lSaveProps[0].Name = "PassWord";
        lSaveProps[0].Value = sPassword;
        lSaveProps[1] = new PropertyValue();
        lSaveProps[1].Name = "Overwrite";
        lSaveProps[1].Value = Boolean.TRUE;

        log.println("create a new password protected document ... ");

        try {
            xDoc = xFrameLoader.loadComponentFromURL("private:factory/swriter",
                                                     "_blank", 0, lLoadProps);
        } catch (com.sun.star.io.IOException exIO) {
            failed("IOException");

            return;
        } catch (com.sun.star.lang.IllegalArgumentException exArg) {
            failed("IllegalArgumentException");

            return;
        }

        if (xDoc == null) {
            assure("The document is null so it was not loaded.", false);

            return;
        }

        xStore = (XStorable) UnoRuntime.queryInterface(XStorable.class, xDoc);

        try {
            xStore.storeAsURL(sTestURL, lSaveProps);
        } catch (com.sun.star.io.IOException exIO) {
            failed("IOException");

            return;
        }


        // don't forget to "close" the document.
        // Otherwhise we can't reopen it successfully.
        xDoc.dispose();
        xDoc = null;

        log.println("\topen the password protected document ... ");

        lLoadProps = new PropertyValue[2];
        lLoadProps[0] = new PropertyValue();
        lLoadProps[0].Name = "Hidden";
        lLoadProps[0].Value = Boolean.TRUE;
        lLoadProps[1] = new PropertyValue();
        lLoadProps[1].Name = "Password";
        lLoadProps[1].Value = sPassword;

        try {
            xDoc = xFrameLoader.loadComponentFromURL(sTestURL, "_blank", 0,
                                                     lLoadProps);
        } catch (com.sun.star.io.IOException exIO) {
            failed("IOException");

            return;
        } catch (com.sun.star.lang.IllegalArgumentException exArg) {
            failed("IllegalArgumentException");

            return;
        }

        if (xDoc == null) {
            assure("document was not loaded.", false);

            return;
        }

        xDoc.dispose();
        xDoc = null;
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

        if (lTestFiles == null) {
            failed("Found an empty directory. There are no files for testing.");

            return;
        }

        // get a file name as byte array
        Enumeration aSnapshot = lTestFiles.elements();
        byte[] baURL = null;

        while (aSnapshot.hasMoreElements()) {
            File aFile = new File(sTestDocPath + fs +
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
                    loadURL(xFrameLoader, RESULT_VALID_DOC, encURL, "_blank", 0,
                            lProps);
                } else {
                    //with cws_loadenv01 changed to IllegalArgumentException
                    loadURL(xFrameLoader, RESULT_ILLEGALARGUMENTEXCEPTION, encURL, "_blank", 0,
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
        loadURL(xFrameLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sIllegalArgs,
                "_blank", 0, lProps);

        log.println("check stupid URLs");

        sIllegalArgs = new String[] {
            "slot:xxx", "slot:111111111", ".uno:save_as", ".uno:open_this",
            ".UnO:*",
        };
        loadURL(xFrameLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sIllegalArgs,
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
        loadURL(xFrameLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sEmptyDocs, "_blank", 0,
                lProps);

        log.println("check case senstive URLs");

        sIllegalArgs = new String[] {
            "sLot:5000", "sloT:10909", ".unO:SaveAs", ".uno:OPEN",
        };
        loadURL(xFrameLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sIllegalArgs,
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
        loadURL(xFrameLoader, RESULT_ILLEGALARGUMENTEXCEPTION, sEmptyDocs, "_blank", 0,
                lProps);

        log.println("check FTP URLs");

        String sFTPURL = (String) param.get("FtpAccess");
        Enumeration aSnapshot = lTestFiles.elements();

        while (aSnapshot.hasMoreElements()) {
            String doc = (String) aSnapshot.nextElement();


            // if os is windows
            doc = doc.replace('\\', '/');
        if (doc.indexOf("CVS")<0) {
            loadURL(xFrameLoader, RESULT_VALID_DOC, sFTPURL + "/" + doc,
                    "_blank", 0, lProps);
        }
        }

        log.println("check HTTP URLs");

        String sHTTPURL = (String) param.get("HttpAccess");
        aSnapshot = lTestFiles.elements();

        while (aSnapshot.hasMoreElements()) {
            String doc = (String) aSnapshot.nextElement();


            // if os is windows
            doc = doc.replace('\\', '/');
        if (doc.indexOf("CVS")<0) {
            loadURL(xFrameLoader, RESULT_VALID_DOC, sHTTPURL + "/" + doc,
                    "_blank", 0, lProps);
        }
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
    private void loadURL(XComponentLoader xFrameLoader, int nRequiredResult,
                         String sURL, String sTarget, int nFlags,
                         PropertyValue[] lProps) {
        int nResult = RESULT_EMPTY_DOC;
        XComponent xDoc = null;

        try {
            xDoc = xFrameLoader.loadComponentFromURL(sURL, sTarget, nFlags,
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

    private void loadURL(XComponentLoader xFrameLoader, int nRequiredResult,
                         String[] sURL, String sTarget, int nFlags,
                         PropertyValue[] lProps) {
        for (int i = 0; i < sURL.length; i++)
            loadURL(xFrameLoader, nRequiredResult, sURL[i], sTarget, nFlags,
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