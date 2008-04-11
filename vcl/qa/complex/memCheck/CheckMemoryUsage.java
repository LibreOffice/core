/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CheckMemoryUsage.java,v $
 * $Revision: 1.4 $
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
package complex.memCheck;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import complexlib.ComplexTestCase;
import helper.ProcessHandler;
import java.io.File;
import java.io.FilePermission;
import java.io.FileWriter;
import java.io.FilenameFilter;
import java.io.PrintWriter;
import java.util.Enumeration;
import java.util.StringTokenizer;
import java.util.Vector;
import util.DesktopTools;
import util.WriterTools;
import util.utils;

/**
 * Documents are opened and exported with StarOffice. The memory usage of
 * StarOffice is monitored and if the usage exceeds the allowed kilobytes,
 * the test is failed. Used for monitoring the StarOffice process is the
 * command line tool 'pmap', available on Solaris or Linux. This test will not
 * run on Windows.<br>Test procedure: every given document type is searched in
 * the source directory
 * Needed paramters:
 * <ul>
 *   <li>"TestDocumentPath" - the path where test documents are located.</li>
 *   <li>"AllowMemoryIncrease" (optional) - the allowed memory increase measured in kByte per exported document. The default is 10 kByte.</li>
 *   <li>"ExportDocCount" (optional) - the amount of exports for each document that is loaded. Is defaulted to 25.
 *   <li>"FileExportFilter" (optional) - a relation between loaded document type and used export filter. Is defaulted to
 *       writer, calc and impress. This parameter can be set with a number to give more than one relation. Example:<br>
 *       "FileExportFilter1=sxw,writer_pdf_Export"<br>
 *       "FileExportFilter2=sxc,calc_pdf_Export"<br>
 *       "FileExportFilter3=sxi,impress_pdf_Export"<br></li>
 *       All parameters are used for iteration over the test document path.
 * </ul>
 */
public class CheckMemoryUsage extends ComplexTestCase {
    private final String sWriterDoc = "sxw,writer_pdf_Export";
    private final String sCalcDoc = "sxc,calc_pdf_Export";
    private final String sImpressDoc = "sxi,impress_pdf_Export";
    private String sProcessId = "ps -ef | grep $USER | grep soffice | grep -v grep";
    private String sMemoryMonitor = "pmap <processID> | grep total";
    private String sChmod = "chmod 777 ";
    private String sProcessIdCommand = null;
    private String sOfficeMemoryCommand = null;
    private String sTempDir = null;
    private String sFS = null;
    private String sMemoryMap1 = null;
    private String sMemoryMap2 = null;
    private String bash = "#!/bin/bash";
    private String sDocumentPath = "";
    private String[][] sDocTypeExportFilter;
    private String[][] sDocuments;
    private int iAllowMemoryIncrease = 10;
    private int iExportDocCount = 25;

    /**
     * Get all test methods
     * @return The test methods.
     */
    public String[] getTestMethodNames() {
        return new String[] {"loadAndSaveDocuments"};
    }

    /**
     * Collect all documnets to load and all filters used for export.
     */
    public void before() {
        // test does definitely not run on Windows.
        if (param.get("OperatingSystem").equals("wntmsci")) {
            log.println("Test can only reasonably be executed with a tool that "
                        + "displays the memory usage of StarOffice.");
            failed("Test does not run on Windows, only on Solaris or Linux.");
        }

        // how many times is every document exported.
        int count = param.getInt("ExportDocCount");
        if (count != 0)
            iExportDocCount = count;

        // get the temp dir for creating the command scripts.
        sTempDir = System.getProperty("java.io.tmpdir");
        sProcessIdCommand = sTempDir + "getPS";
        sOfficeMemoryCommand = sTempDir + "getPmap";

        // get the file extension, export filter connection
        Enumeration keys = param.keys();
        Vector v = new Vector();
        while(keys.hasMoreElements()) {
            String key = (String)keys.nextElement();
            if (key.startsWith("FileExportFilter")) {
                v.add(param.get(key));
            }
        }
        // if no param given, set defaults.
        if (v.size() == 0){
            v.add(sWriterDoc);
            v.add(sCalcDoc);
            v.add(sImpressDoc);
        }
        // store a file extension
        sDocTypeExportFilter = new String[v.size()][2];
        for (int i=0; i<v.size(); i++) {
            // 2do: error routine for wrong given params
            StringTokenizer t = new StringTokenizer((String)v.get(i), ",");
            sDocTypeExportFilter[i][0] = t.nextToken();
            sDocTypeExportFilter[i][1] = t.nextToken();
        }

        // get files to load and export
        sDocumentPath = (String)param.get("TestDocumentPath");
        File f = new File(sDocumentPath);
        sDocumentPath = f.getAbsolutePath();
        String sFS = System.getProperty("file.separator");
        sDocuments = new String[sDocTypeExportFilter.length][];
        for (int j=0; j<sDocTypeExportFilter.length; j++) {
            FileFilter filter = new FileFilter(sDocTypeExportFilter[j][0]);
            String[] doc = f.list(filter);
            sDocuments[j] = new String[doc.length];
            for (int i=0; i<doc.length; i++) {
                if (sDocumentPath.endsWith(sFS))
                    sDocuments[j][i] = sDocumentPath + doc[i];
                else
                    sDocuments[j][i] = sDocumentPath + sFS + doc[i];
                sDocuments[j][i] = utils.getFullURL(sDocuments[j][i]);
            }
        }
    }

    /**
     * delete all created files on disk
     */
    public void after() {
        // delete the constructed files.
        for (int i=0; i<iExportDocCount; i++) {
            File f = new File(sTempDir + "DocExport" + i + ".pdf");
            f.delete();
        }
        File f = new File(sProcessIdCommand);
        f.delete();
        f = new File(sOfficeMemoryCommand);
        f.delete();
    }

    /**
     * Thet etst function: load documents and save them using the given filters
     * for each given document type.
     */
    public void loadAndSaveDocuments() {
        int storageBefore = getOfficeMemoryUsage();

        XMultiServiceFactory xMSF = (XMultiServiceFactory)param.getMSF();

        // iterate over all document types
        for (int k=0; k<sDocTypeExportFilter.length; k++) {
            // iterate over all documents of this type
            for (int i=0; i<sDocuments[k].length; i++) {
                System.out.println("Document: "+ sDocuments[k][i]);
                XComponent xComponent = DesktopTools.loadDoc(xMSF, sDocuments[k][i], null);
                XStorable xStorable = (XStorable)UnoRuntime.queryInterface(XStorable.class, xComponent);
                if (xStorable != null) {
                    // export each document iExportDocCount times
                    for (int j=0; j<iExportDocCount; j++) {
                        String url = utils.getFullURL(sTempDir + "DocExport" + j + ".pdf");
                        try {
                            PropertyValue[] props = new PropertyValue[1];
                            props[0] = new PropertyValue();
                            props[0].Name = "FilterName";
                            // use export filter for this doc type
                            props[0].Value = sDocTypeExportFilter[k][1];
                            xStorable.storeToURL(url, props);
                        }
                        catch(com.sun.star.io.IOException e) {
                            failed("Could not store to '" + url + "'", true);
                        }
                    }
                    // close the doc
                    XCloseable xCloseable = (XCloseable)UnoRuntime.queryInterface(XCloseable.class, xStorable);
                    try {
                        xCloseable.close(true);
                    }
                    catch(com.sun.star.util.CloseVetoException e) {
                        e.printStackTrace((java.io.PrintWriter)log);
                        failed("Cannot close document: test is futile, Office will surely use more space.");
                    }
                }
                else {
                    log.println("Cannot query for XStorable interface on document '" + sDocuments[i] + "'");
                    log.println(" -> Skipping storage.");
                }
            }
        }
        // short wait for the office to 'calm down' and free some memory
        shortWait(5000);
        // wait util memory is not freed anymore.
        int storageAfter = getOfficeMemoryUsage();
        int mem = 0;
        int count = 0;
        while (storageAfter != mem && count < 10) {
            count++;
            mem = storageAfter;
            storageAfter = getOfficeMemoryUsage();
            shortWait(1000);
        }
        assure("The Office consumes now " + (storageAfter - storageBefore)
            + "K more memory than at the start of the test; allowed were "
            + iAllowMemoryIncrease * iExportDocCount + "K.",
            storageAfter - storageBefore < iAllowMemoryIncrease * iExportDocCount);

    }

    /**
     * Get the process ID from the Office
     * @return the Id as String
     */
    private String getOfficeProcessID() {
        writeExecutableFile(sProcessIdCommand, sProcessId);
        ProcessHandler processID = new ProcessHandler(sProcessIdCommand);
        processID.executeSynchronously();
        String text = processID.getOutputText();
        if (text == null || text.equals("") || text.indexOf(' ') == -1)
            failed("Could not determine Office process ID. Check " + sProcessIdCommand);
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
    private int getOfficeMemoryUsage() {
        String command = sMemoryMonitor.replaceAll("<processID>", getOfficeProcessID());
        writeExecutableFile(sOfficeMemoryCommand, command);
        ProcessHandler processID = new ProcessHandler(sOfficeMemoryCommand);
        processID.executeSynchronously();
        String text = processID.getOutputText();
        if (text == null || text.equals("") || text.indexOf(' ') == -1) {
            failed("Could not determine Office memory usage. Check " + sOfficeMemoryCommand);
        }
        StringTokenizer aToken = new StringTokenizer(text);
        // this works, because the output of pmap is quite standardized.
        aToken.nextToken();
        String mem = aToken.nextToken();
        mem = mem.substring(0, mem.indexOf('K'));
        Integer memory = new Integer(mem);
        return memory.intValue();
    }

    /**
     * Write a script file and set its rights to rwxrwxrwx.
     * @param fileName The name of the created file
     * @param line The commandline that has to be written inside of the file.
     */
    private void writeExecutableFile(String fileName, String line) {
        try {
            PrintWriter fWriter = new PrintWriter(new FileWriter(fileName));
            fWriter.println(bash);
            fWriter.println(line);
            fWriter.close();
            // change rights to rwxrwxrwx
            ProcessHandler processID = new ProcessHandler(sChmod + fileName);
            processID.executeSynchronously();
        }
        catch(java.io.IOException e) {
        }
    }

    /**
     * Let this thread sleep for some time
     * @param milliSeconds time to wait in milliseconds.
     */
    private void shortWait(int milliSeconds) {
        try {
            Thread.sleep(milliSeconds);
        }
        catch(java.lang.InterruptedException e) { // ignore
        }
    }

    /**
     * Own file filter, will just return ok for all files that end with a given
     * suffix
     */
    private class FileFilter implements FilenameFilter {
        private String suffix = null;
        /**
         * C'tor.
         * @param suffix The suffix each filename should end with.
         */
        public FileFilter(String suffix) {
            this.suffix = suffix;
        }
        /**
         * Returns true, if the name of the file has the suffix given to the
         * c'tor.
         * @param name The filename that is tested.
         * @param file Not used.
         * @return True, if name ends with suffix.
         */
        public boolean accept(File file, String name) {
            return name.endsWith(suffix);
        }
    };

}
