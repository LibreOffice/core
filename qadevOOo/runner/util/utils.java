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
package util;

import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;

import java.util.StringTokenizer;
import java.io.*;
import java.util.ArrayList;
import java.net.Socket;
import java.net.ServerSocket;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Arrays;

import com.sun.star.awt.XToolkitExperimental;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.Property;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.ucb.InteractiveAugmentedIOException;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XMacroExpander;

import java.text.DecimalFormat;
import java.util.Calendar;
import java.util.Collections;
import java.util.GregorianCalendar;

public class utils {

    /**
     *
     * This method adds the DOCPTH to a given file
     *
     * @param sDocName the file which should be completed to the test doc path
     * @return $TESTDOCPATH/sDocName
     */
    public static String getFullTestDocName(String sDocName) {
        String docpth = System.getProperty("DOCPTH");
        if (docpth.endsWith("\\") || docpth.endsWith("/")) {
            docpth = docpth.substring(0, docpth.length() - 1);
        }

        System.out.println("docpth:" + docpth);

        String pthSep = System.getProperty("file.separator");

        if (docpth.equals("unknown")) {
            System.out.println("try to get tDoc from $SRC_ROOT/qadevOOo");
            String srcRoot = System.getProperty(PropertyName.SRC_ROOT);
            if (srcRoot != null) {
                File srcR = new File(srcRoot);
                String[] list = srcR.list(new FilenameFilter() {

                    public boolean accept(File dir, String name) {
                        return name.startsWith("qadevOOo");
                    }
                });

                if (list != null && list[0] != null) {
                    String tDoc = srcRoot.concat(pthSep).concat(list[0]).concat(pthSep).concat("testdocs");

                    if (new File(tDoc).exists()) {
                        docpth = tDoc;
                    }
                }
            }
        }

        if (docpth.startsWith("http:")) {
            return docpth + "/" + sDocName;
        }
        String testdocPth = "";

        if (docpth.equals("unknown")) {
            System.out.println("try to get tDoc from OBJDSCS");
            String objdscPth = System.getProperty("OBJDSCS");
            if (objdscPth != null) {
                int i = objdscPth.indexOf("objdsc");
                String arcPth = objdscPth.substring(0, i - 1);
                testdocPth = arcPth + pthSep + "doc" + pthSep + "java" +
                    pthSep + "testdocs" + pthSep + sDocName;
            }
        } else {
            testdocPth = docpth + pthSep + sDocName;
        }
        return testdocPth;
    }

    /**
     *
     * This method adds the DOCPTH to a given file
     * and changes the format to an file URL
     *
     */
    public static String getFullTestURL(String sDocName) {
        String fullDocPath = getFullTestDocName(sDocName);
        if (fullDocPath.startsWith("http:")) {
            return fullDocPath;
        }
        if (fullDocPath.startsWith("file:")) {
            return fullDocPath;
        }
        String prefix = null;

        //  Windows: \\\\margritte\\qaapi\\workspace\\qadev\\testdocs/emptyChart.sds
        if (fullDocPath.startsWith("\\\\")) {
            prefix = "file:";
        }

        fullDocPath = fullDocPath.replace('\\', '/');
        if (prefix == null) {
            if (fullDocPath.startsWith("//")) {
                prefix = "file:/";
            } else if (fullDocPath.startsWith("/")) {
                prefix = "file://";
            } else {
                prefix = "file:///";
            }
        }
        if (!fullDocPath.endsWith("/")) {
            File aFile = new File(fullDocPath);
            if (aFile.isDirectory()) {
                fullDocPath += "/";
            }
        }
        String fulldocURL = prefix + fullDocPath;
        return fulldocURL;
    }

    /**
     *
     * This method changes a given URL to a valid file URL
     *
     */
    public static String getFullURL(String sDocName) {
        String fullDocPath = sDocName;
        fullDocPath = fullDocPath.replace('\\', '/');

        if (fullDocPath.startsWith("http:")) {
            return fullDocPath;
        }
        if (fullDocPath.startsWith("ftp:")) {
            return fullDocPath;
        }
        String prefix = "";
        if (!fullDocPath.startsWith("file:///")) {
            if (fullDocPath.startsWith("//")) {
                prefix = "file:";
            } else {
                if (fullDocPath.startsWith("/")) {
                    prefix = "file://";
                }
                else
                {
                    prefix = "file:///";
                }
            }
        }
        if (!fullDocPath.endsWith("/")) {
            File aFile = new File(fullDocPath);
            if (aFile.isDirectory()) {
                fullDocPath += "/";
            }
        }
        String fulldocURL = prefix + fullDocPath;

        return fulldocURL;
    }

    /**
     *
     * This method gets the user dir of the connected office
     *
     */
    public static String getOfficeUserPath(XMultiServiceFactory msf) {
        // get a folder located in the user dir
        String userPath = getOfficeSettingsValue(msf, "UserConfig");

        // strip the returned folder to the user dir
        if (userPath.charAt(userPath.length() - 1) == '/') {
            userPath = userPath.substring(0, userPath.length() - 1);
        }
        int index = userPath.lastIndexOf('/');
        if (index != -1) {
            userPath = userPath.substring(0, index);
        }

        return userPath;
    }

    /**
     * In the office there are some settings available. This function
     * returns the value of the given setting name. For Example the setting name "Temp"
     * "Temp" returns the temp folder of the office instance.
     * @param msf a XMultiServiceFactory
     * @param setting  the name of the setting the value should be returned.
     * For example "Temp" returns the temp folder of the current office instance.
     * @see com.sun.star.util.PathSettings
     * @return the value as String
     */
    private static String getOfficeSettingsValue(XMultiServiceFactory msf, String setting) {
        try {
            Object settings = msf.createInstance("com.sun.star.comp.framework.PathSettings");
            XPropertySet pthSettings = (XPropertySet) AnyConverter.toObject(
                new Type(XPropertySet.class), settings);
            return (String) pthSettings.getPropertyValue(setting);
        } catch (com.sun.star.uno.Exception ex) {
            throw new RuntimeException(ex);
        }
    }

    /**
     * This method returns the temp directory of the user.
     * Since Java 1.4 it is not possible to read environment variables. To workaround
     * this, the Java parameter -D could be used.
     */
    public static String getUsersTempDir() {
        String tempDir = System.getProperty("my.temp");
        if (tempDir == null) {
            tempDir = System.getProperty("my.tmp");
            if (tempDir == null) {
                tempDir = System.getProperty("java.io.tmpdir");
            }
        }
        // remove ending file separator
        if (tempDir.endsWith(System.getProperty("file.separator"))) {
            tempDir = tempDir.substring(0, tempDir.length() - 1);
        }

        return tempDir;
    }

    /**
     *
     * This method gets the temp dir of the connected office
     *
     */
    public static String getOfficeTemp(XMultiServiceFactory msf) {
        String url = getOfficeUserPath(msf) + "/test-temp/";
        try {
            new File(new URI(url)).mkdir();
        } catch (URISyntaxException e) {
            throw new RuntimeException(e);
        }
        return url;
    }

    /**
     * Gets StarOffice temp directory without 'file:///' prefix.
     * For example is useful for Registry URL specifying.
     * @msf Office factory for accessing its settings.
     * @return SOffice temporary directory in form for example
     * 'd:/Office60/user/temp/'.
     */
    public static String getOfficeTempDir(XMultiServiceFactory msf) {

        String dir = getOfficeTemp(msf);

        int idx = dir.indexOf("file:///");

        if (idx < 0) {
            return dir;
        }

        dir = dir.substring("file:///".length());

        idx = dir.indexOf(':');

        // is the last character a '/' or a '\'?
        boolean lastCharSet = dir.endsWith("/") || dir.endsWith("\\");

        if (idx < 0) { // linux or solaris
            dir = "/" + dir;
            dir += lastCharSet ? "" : "/";
        } else {  // windows
            dir += lastCharSet ? "" : "\\";
        }

        return dir;
    }

    /**
     * Gets StarOffice temp directory without 'file:///' prefix.
     * and System dependent file separator
     */
    public static String getOfficeTempDirSys(XMultiServiceFactory msf) {

        String dir = getOfficeTemp(msf);
        String sysDir = "";

        int idx = dir.indexOf("file://");

        // remove leading 'file://'
        if (idx < 0) {
            sysDir = dir;
        } else {
            sysDir = dir.substring("file://".length());
        }

        // append '/' if not there (e.g. linux)
        if (sysDir.charAt(sysDir.length() - 1) != '/') {
            sysDir += "/";
        }

        // remove leading '/' and replace others with '\' on windows machines
        if (sysDir.indexOf(':') != -1) {
            sysDir = sysDir.substring(1);
            sysDir = sysDir.replace('/', '\\');
        }
        return sysDir;
    }

    /**
     * converts a fileURL to a system URL
     * @param fileURL a file URL
     * @return a system URL
     */
    public static String getSystemURL(String fileURL) {
        String sysDir = "";

        int idx = fileURL.indexOf("file://");

        // remove leading 'file://'
        if (idx < 0) {
            sysDir = fileURL;
        } else {
            sysDir = fileURL.substring("file://".length());
        }

        // remove leading '/' and replace others with '\' on windows machines
        if (sysDir.indexOf(':') != -1) {
            sysDir = sysDir.substring(1);
            sysDir = sysDir.replace('/', '\\');
        }
        return sysDir;
    }

    /**
     *  This method check via Office the existence of the given file URL
     * @param msf the multiservice factory
     * @param fileURL the file which existence should be checked
     * @return true if the file exists, else false
     */
    public static boolean fileExists(XMultiServiceFactory msf, String fileURL) throws com.sun.star.uno.Exception {
        Object fileacc = msf.createInstance("com.sun.star.comp.ucb.SimpleFileAccess");
        XSimpleFileAccess simpleAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class,
                fileacc);
        return simpleAccess.exists(fileURL);
    }

    /**
     * This method deletes via office the given file URL. It checks the existence
     * of <CODE>fileURL</CODE>. If exists it will be deleted.
     * @param xMsf the multiservice factory
     * @param fileURL the file to delete
     * @return true if the file could be deleted or the file does not exist
     */
    public static boolean deleteFile(XMultiServiceFactory xMsf, String fileURL) {
        boolean delete = true;
        try {

            Object fileacc = xMsf.createInstance("com.sun.star.comp.ucb.SimpleFileAccess");
            XSimpleFileAccess simpleAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class,
                fileacc);
            if (simpleAccess.exists(fileURL)) {
                simpleAccess.kill(fileURL);
            }

        } catch (Exception e) {
            System.out.println("Couldn't delete file '" + fileURL + "'");
            e.printStackTrace();
            delete = false;
        }
        return delete;
    }

    /**
     * This method copies via office a given file to a new one
     * @param xMsf the multi service factory
     * @param source the source file
     * @param destination the destination file
     * @return true at success
     */
    public static boolean copyFile(XMultiServiceFactory xMsf, String source, String destination) {
        boolean res = false;
        try {
            Object fileacc = xMsf.createInstance("com.sun.star.comp.ucb.SimpleFileAccess");
            XSimpleFileAccess simpleAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class,
                fileacc);
            if (!simpleAccess.exists(destination)) {
                simpleAccess.copy(source, destination);
            }

            res = true;
        } catch (Exception e) {
            System.out.println("Couldn't copy file '" + source + "' -> '" + destination + "'");
            e.printStackTrace();
            res = false;
        }
        return res;
    }

    private static void overwriteFile_impl(
        XMultiServiceFactory xMsf, String oldF, String newF)
        throws InteractiveAugmentedIOException
    {
        try {
            Object fileacc = xMsf.createInstance("com.sun.star.comp.ucb.SimpleFileAccess");

            XSimpleFileAccess simpleAccess = UnoRuntime.queryInterface(XSimpleFileAccess.class,
                fileacc);
            if (simpleAccess.exists(newF)) {
                simpleAccess.kill(newF);
            }
            simpleAccess.copy(oldF, newF);
        } catch (InteractiveAugmentedIOException e) {
            throw e;
        } catch (com.sun.star.uno.Exception ex) {
            throw new RuntimeException("Could not copy " + oldF + " to " + newF, ex);
        }
    }

    /**
     * Copies file to a new location using OpenOffice.org features. If the target
     * file already exists, the file is deleted.
     *
     * @returns <code>true</code> if the file was successfully copied,
     * <code>false</code> if some errors occurred (e.g. file is locked, used
     * by another process).
     */
    public static boolean tryOverwriteFile(
        XMultiServiceFactory xMsf, String oldF, String newF)
    {
        try {
            overwriteFile_impl(xMsf, oldF, newF);
        } catch (InteractiveAugmentedIOException e) {
            return false;
        }
        return true;
    }



    public static boolean hasPropertyByName(XPropertySet props, String aName) {
        Property[] list = props.getPropertySetInfo().getProperties();
        boolean res = false;
        for (int i = 0; i < list.length; i++) {
            String the_name = list[i].Name;
            if (aName.equals(the_name)) {
                res = true;
            }
        }
        return res;
    }

    /**
     *
     * This method returns the implementation name of a given object
     *
     */
    public static String getImplName(Object aObject) {
        XServiceInfo xSI = UnoRuntime.queryInterface(XServiceInfo.class, aObject);
        return xSI == null ? "Unknown, does not implement XServiceInfo" : xSI.getImplementationName();
    }

    /**
     *
     * This method checks if an Object is void
     *
     */
    public static boolean isVoid(Object aObject) {
        if (aObject instanceof com.sun.star.uno.Any) {
            com.sun.star.uno.Any oAny = (com.sun.star.uno.Any) aObject;
            return oAny.getType().getTypeName().equals("void");
        } else {
            return false;
        }

    }

    /**
     * Scan localhost for the next free port-number from a starting port
     * on. If the starting port is smaller than 1024, port number starts with
     * 10000 as default, because numbers < 1024 are never free on unix machines.
     * @param startPort The port where scanning starts.
     * @return The next free port.
     */
    public static int getNextFreePort(int startPort) {
        if (startPort < 1024) {
            startPort = 10000;
        }
        for (int port = startPort; port < 65536; port++) {
            System.out.println("Scan port " + port);
            try {
                // first trying to establish a server-socket on localhost
                // fails if there is already a server running
                ServerSocket sSock = new ServerSocket(port);
                sSock.close();
            } catch (IOException e) {
                System.out.println(" -> server: occupied port " + port);
                continue;
            }
            try {
                Socket sock = new Socket("localhost", port);
                System.out.println(" -> socket: occupied port: " + port);
                try {
                    sock.close();
                } catch (IOException ex) {
                    // ignore close exception
                }
            } catch (IOException e) {
                System.out.println(" -> free port");
                return port;
            }
        }
        return 65535;
    }

    public static URL parseURL(XMultiServiceFactory xMSF, String url) {
        URL[] rUrl = new URL[1];
        rUrl[0] = new URL();
        rUrl[0].Complete = url;

        XURLTransformer xTrans = null;
        try {
            Object inst = xMSF.createInstance("com.sun.star.util.URLTransformer");
            xTrans = UnoRuntime.queryInterface(XURLTransformer.class, inst);
        } catch (com.sun.star.uno.Exception e) {
        }

        if (xTrans != null)
            xTrans.parseStrict(rUrl);

        return rUrl[0];
    }

    public static String getOfficeURL(XMultiServiceFactory msf) throws com.sun.star.uno.Exception {
        Object settings = msf.createInstance("com.sun.star.util.PathSettings");
        XPropertySet settingProps = UnoRuntime.queryInterface(XPropertySet.class, settings);
        String path = (String) settingProps.getPropertyValue("Module");
        return path;
    }



    /**
     * Get an array of all property names from the property set. With the include
     * and exclude parameters the properties can be filtered. <br>
     * Set excludePropertyAttribute = 0 and includePropertyAttribute = 0
     * to include all and exclude none.
     * @param props The instance of XPropertySet
     * @param includePropertyAttribute Properties without these attributes are filtered and will not be returned.
     * @param excludePropertyAttribute Properties with these attributes are filtered and will not be returned.
     * @param array of string names of properties that will be skipped
     * @return A String array with all property names.
     * @see com.sun.star.beans.XPropertySet
     * @see com.sun.star.beans.Property
     * @see com.sun.star.beans.PropertyAttribute
     */
    public static String[] getFilteredPropertyNames(XPropertySet props, short includePropertyAttribute,
        short excludePropertyAttribute, String[] skipList) {
        Property[] the_props = props.getPropertySetInfo().getProperties();
        ArrayList<String> l = new ArrayList<String>();
        for (int i = 0; i < the_props.length; i++) {
            if (Arrays.asList(skipList).contains(the_props[i].Name))
                continue;
            boolean exclude = ((the_props[i].Attributes & excludePropertyAttribute) != 0);
            boolean include = (includePropertyAttribute == 0) ||
                ((the_props[i].Attributes & includePropertyAttribute) != 0);
            if (include && !exclude) {
                l.add(the_props[i].Name);
            }
        }
        Collections.sort(l);
        String[] names = new String[l.size()];
        names = l.toArray(names);
        return names;
    }

    /** Causes the thread to sleep some time.
     * This is the default call, which waits for 500ms.
     */
    public static void shortWait() {
        pause(utils.DEFAULT_SHORT_WAIT_MS);
    }

    /** Causes the thread to sleep some time.
     */
    public static void pause(int milliseconds) {
        try {
            Thread.sleep(milliseconds);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }

    public static void waitForEventIdle(XMultiServiceFactory xMSF) {
        try {
            XToolkitExperimental xToolkit = UnoRuntime.queryInterface(
                    XToolkitExperimental.class,
                    xMSF.createInstance("com.sun.star.awt.Toolkit"));
            xToolkit.processEventsToIdle();
        } catch (com.sun.star.uno.Exception ex) {
            throw new RuntimeException(ex);
        }
    }

    /**
     * Validate the AppExecutionCommand. Returned is an error message, starting
     * with "Error:", or a warning, if the command might work.
     * @param appExecCommand The application execution command that is checked.
     * @param os The operating system where the check runs.
     * @return The error message, or OK, if no error was detected.
     */
    public static String validateAppExecutionCommand(String appExecCommand, String os) {
        String errorMessage = "OK";
        appExecCommand = appExecCommand.replace("\"", "");
        appExecCommand = appExecCommand.replace("'", "");
        StringTokenizer commandTokens = new StringTokenizer(appExecCommand, " \t");
        String officeExecCommand = "soffice";
        StringBuilder sb = new StringBuilder();
        // is there a 'soffice' in the command? 2do: eliminate case sensitivity on windows
        int index = -1;
        while (commandTokens.hasMoreTokens() && index == -1) {
            sb.append(commandTokens.nextToken()).append(" ");
            index = sb.indexOf(officeExecCommand);
        }
        if (index == -1) {
            errorMessage = "Error: Your 'AppExecutionCommand' parameter does not " +
                "contain '" + officeExecCommand + "'.";
        } else {
            String officeExecutable = sb.toString();
            // does the directory exist?
            officeExecutable = officeExecutable.trim();
            String officePath = officeExecutable.substring(0, index);
            File f = new File(officePath);
            if (!f.exists() || !f.isDirectory()) {
                errorMessage = "Error: Your 'AppExecutionCommand' parameter does not " +
                    "point to a valid system directory: " + officePath;
            } else {
                // is it an office installation?
                f = new File(officeExecutable);
                // one try for windows platform can't be wrong...
                if (!f.exists() || !f.isFile()) {
                    f = new File(officeExecutable + ".exe");
                }
                if (!f.exists() || !f.isFile()) {
                    errorMessage = "Error: Your 'AppExecutionCommand' parameter does not " +
                        "point to a valid office installation.";
                } else {
                    // do we have the accept parameter?
                    boolean gotNoAccept = true;
                    while (commandTokens.hasMoreElements()) {
                        String officeParam = commandTokens.nextToken();
                        if (officeParam.indexOf("--accept=") != -1) {
                            gotNoAccept = false;
                            errorMessage = validateConnectString(officeParam, true);
                        }
                    }
                    if (gotNoAccept) {
                        errorMessage = "Error: Your 'AppExecutionCommand' parameter does not " +
                            "contain a '--accept' parameter for connecting the office.";
                    }
                }
            }
        }
        return errorMessage;
    }

    /**
     * Validate the connection string. Returned is an error message, starting
     * with "Error:", or a warning, if the command might work.
     * @param connectString The connection string that is checked.
     * @param checkAppExecutionCommand If the AppExecutionCommand is checked, the error message is different.
     * @return The error message, or OK, if no error was detected.
     */
    private static String validateConnectString(String connectString, boolean checkAppExecutionCommand) {
        String acceptPrefix = "";
        if (checkAppExecutionCommand) {
            acceptPrefix = "--accept=";
        }

        String errorMessage = "OK";
        // a warning, if an unknown connection method is used
        if (connectString.indexOf("socket") != -1) {
            if (connectString.indexOf(acceptPrefix + "socket,host=") == -1 ||
                connectString.indexOf("port=") == -1) {
                if (checkAppExecutionCommand) {
                    errorMessage = "Error: The '--accept' parameter contains a syntax error: It should be like: '--accept=socket,host=localhost,port=8100;urp;";
                } else {
                    errorMessage = "Error: The 'ConnectionString' parameter contains a syntax error: It should be like: 'socket,host=localhost,port=8100'";
                }
            }
        } else if (connectString.indexOf("pipe") != -1) {
            if (connectString.indexOf(acceptPrefix + "pipe,name=") == -1) {
                if (checkAppExecutionCommand) {
                    errorMessage = "Error: The '--accept' parameter contains a syntax error: It should be like: '--accept=pipe,name=myuniquename;urp;'";
                } else {
                    errorMessage = "Error: The 'ConnectionString' parameter contains a syntax error: It should be like: 'pipe,name=myuniquename'";
                }
            }
        } else {
            if (checkAppExecutionCommand) {
                errorMessage = "Warning: The '--accept' parameter contains an unknown connection method.";
            } else {
                errorMessage = "Warning: The 'ConnectionString' parameter contains an unknown connection method.";
            }
        }
        return errorMessage;
    }

    /**
     * expand macrofied strings like <CODE>${$ORIGIN/bootstrap.ini:UserInstallation}</CODE> or
     * <CODE>$_OS</CODE>
     * @param xMSF the MultiServiceFactory
     * @param expand the string to expand
     * @throws java.lang.Exception was thrown on any exception
     * @return return the expanded string
     * @see com.sun.star.util.XMacroExpander
     */
    public static String expandMacro(XMultiServiceFactory xMSF, String expand) {
        try {
            XPropertySet xPS = UnoRuntime.queryInterface(XPropertySet.class, xMSF);
            XComponentContext xContext = UnoRuntime.queryInterface(XComponentContext.class,
                xPS.getPropertyValue("DefaultContext"));
            XMacroExpander xME = UnoRuntime.queryInterface(XMacroExpander.class,
                xContext.getValueByName("/singletons/com.sun.star.util.theMacroExpander"));
            return xME.expandMacros(expand);
        } catch (Exception e) {
            throw new RuntimeException("could not expand macro", e);
        }

    }



    /**
     * dispatches given <CODE>URL</CODE> to the document <CODE>XComponent</CODE>
     * @param xMSF the <CODE>XMultiServiceFactory</CODE>
     * @param xDoc the document where to dispatch
     * @param URL the <CODE>URL</CODE> to dispatch
     * @throws java.lang.Exception throws <CODE>java.lang.Exception</CODE> on any error
     */
    public static void dispatchURL(XMultiServiceFactory xMSF, XComponent xDoc, String URL) throws java.lang.Exception {
        XModel aModel = UnoRuntime.queryInterface(XModel.class, xDoc);

        XController xCont = aModel.getCurrentController();

        dispatchURL(xMSF, xCont, URL);

    }

    /**
     * dispatches given <CODE>URL</CODE> to the <CODE>XController</CODE>
     * @param xMSF the <CODE>XMultiServiceFactory</CODE>
     * @param xCont the <CODE>XController</CODE> to query for a XDispatchProvider
     * @param URL the <CODE>URL</CODE> to dispatch
     */
    private static void dispatchURL(XMultiServiceFactory xMSF, XController xCont, String URL) {
        try {

            XDispatchProvider xDispProv = UnoRuntime.queryInterface(XDispatchProvider.class, xCont);

            XURLTransformer xParser = UnoRuntime.queryInterface(
                XURLTransformer.class,
                xMSF.createInstance("com.sun.star.util.URLTransformer"));

            // Because it's an in/out parameter we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = URL;
            xParser.parseStrict(aParseURL);

            URL aURL = aParseURL[0];

            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
            xDispatcher.dispatch(aURL, null);

            waitForEventIdle(xMSF);

        } catch (Exception e) {
            throw new RuntimeException("Could not dispatch URL '" + URL + "'", e);
        }
    }

    /** returns a String which contains the current date and time<br>
     *  format: [DD.MM.YYYY - HH:MM:SS::mm]
     *
     ** @return a String which contains the current date and time
     */
    public static String getDateTime() {

        Calendar cal = new GregorianCalendar();
        DecimalFormat dfmt = new DecimalFormat("00");
        String dateTime = dfmt.format(cal.get(Calendar.DAY_OF_MONTH)) + "." +
            dfmt.format(cal.get(Calendar.MONTH) + 1) + "." +
            dfmt.format(cal.get(Calendar.YEAR)) + " - " +
            dfmt.format(cal.get(Calendar.HOUR_OF_DAY)) + ":" +
            dfmt.format(cal.get(Calendar.MINUTE)) + ":" +
            dfmt.format(cal.get(Calendar.SECOND)) + "," +
            dfmt.format(cal.get(Calendar.MILLISECOND));
        return "[" + dateTime + "]";
    }

    /**
     * Default short wait time for the Office
     */
    public static final int DEFAULT_SHORT_WAIT_MS = 500;

    /// see rtl/math.hxx
    public static boolean approxEqual( double a, double b )
    {
        if( a == b )
            return true;
        double x = a - b;
        return (x < 0.0 ? -x : x)
            < ((a < 0.0 ? -a : a) * (1.0 / (16777216.0 * 16777216.0)));
    }
}
