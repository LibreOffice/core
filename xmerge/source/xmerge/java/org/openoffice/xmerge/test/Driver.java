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

package org.openoffice.xmerge.test;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.Iterator;

import org.openoffice.xmerge.Convert;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConverterFactory;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentMerger;
import org.openoffice.xmerge.util.registry.ConverterInfo;
import org.openoffice.xmerge.util.registry.ConverterInfoMgr;
import org.openoffice.xmerge.util.registry.ConverterInfoReader;

/**
 *  This class is a command-line driver for the converter framework.
 *  It is expected that this code will be later called by the device
 *  server. It does some basic validation of the command-line
 *  parameters.
 */
public final class Driver {

    /**  Command-line parameter. */
    private String fromMime = null;

    /**  Command-line parameter. */
    private String toMime = null;

    /**  mergeFile name. */
    private String mergeFile = null;

    /**  Command-line parmeter. */
    private ArrayList<String> deviceFiles = new ArrayList<String>();

    /**  Command-line parmeter shortcuts. */
    private String mimeTypes[] = {
                                    "sxc", "staroffice/sxc",
                                    "sxw","staroffice/sxw"
    };


    /**
     *  Main.
     *
     *  @param  args  The argument passed on the command line.
     */
    public static void main(String args[]) {

        // Register jarfiles
        //
        String propFile       = "ConverterInfoList.properties";
        ConverterInfoList cil = null;
        try {
            cil = new ConverterInfoList(propFile);
        } catch (Exception e) {
            System.out.println("\nCannot not load " + propFile +
                " property file");
        }

        Iterator<ConverterInfo> jarInfoEnumeration;
        ConverterInfoReader cir;

        Iterator<String> jarFileEnum = cil.getJarFileEnum();
        while (jarFileEnum.hasNext()) {
            String jarName = jarFileEnum.next();
            try {
                cir = new ConverterInfoReader(jarName, false);
                jarInfoEnumeration = cir.getConverterInfoEnumeration();
                ConverterInfoMgr.addPlugIn(jarInfoEnumeration);
            } catch (Exception e) {
                System.out.println("\nCannot not load <" + jarName +
                    "> from the <" + propFile + "> property file");
            }
        }

        try {

            Driver app = new Driver();
            app.parseCommandLine(args);
            app.doConversion();
        } catch (IllegalArgumentException ex) {

            String msg = ex.getMessage();
            if (msg != null) System.out.println("\n" + msg);
            showUsage();

        } catch (Exception ex) {

            String msg = ex.getMessage();
            if (msg != null) System.out.println("\n" + msg);
            ex.printStackTrace();
        }
    }


    /**
     *  Gets a <code>Convert</code> object using the
     *  <code>ConverterFactory</code> and  does the conversion using
     *  this object.
     *
     *  @throws  IllegalArgumentException  If an argument is invalid.
     */
    private void doConversion() throws IllegalArgumentException {

        ConverterFactory cf = new ConverterFactory();
        Convert myConvert   = cf.getConverter(fromMime, toMime);
        String processFile  = null;

        if (myConvert == null) {
            System.out.println("\nNo plug-in exists to convert from <" +
                fromMime + "> to <" + toMime + ">");
            throw new IllegalArgumentException();
        }

        try {
            Iterator<String> dfEnum = deviceFiles.iterator();
            while (dfEnum.hasNext()) {
                processFile = dfEnum.next();
                File f = new File(processFile);

                // Make sure the input file actually exists before using it
                if (!f.exists()) {
                    System.out.println(processFile + " does not exist!");
                    System.exit(0);
                }
                FileInputStream fis = new FileInputStream(f);
                myConvert.addInputStream(f.getName(), fis);
            }
        } catch (Exception addExcept) {
            System.out.println("\nFile <" + processFile + "> is not in <" +
                fromMime + "> format");
            throw new IllegalArgumentException();
        }

        ConvertData dataOut = null;

        try {
            dataOut = myConvert.convert();
        } catch (Exception convertExcept) {
            System.out.println("\nThere was an error in the conversion");
            convertExcept.printStackTrace();
        }

        if (dataOut != null ) {

            if (mergeFile == null) {
                Iterator<Object> docEnum = dataOut.getDocumentEnumeration();
                while (docEnum.hasNext()) {
                    Document docOut      = (Document)docEnum.next();
                    String fileName      = docOut.getFileName();
                    try {
                        FileOutputStream fos = new FileOutputStream(fileName);
                        docOut.write(fos);
                        fos.flush();
                        fos.close();
                    } catch (Exception writeExcept) {
                        System.out.println("\nThere was an writing out file <" +
                            fileName + ">");
                        writeExcept.printStackTrace();
                    }
                }
            } else {
                try {
                    FileInputStream mergeIS = new FileInputStream(mergeFile);
                    Document mergeDoc = myConvert.getOfficeDocument(mergeFile, mergeIS);
                    DocumentMerger merger = myConvert.getDocumentMerger(mergeDoc);
                    Iterator<Object> mergeDocEnum = dataOut.getDocumentEnumeration();
                    Document convertedFile = (Document)mergeDocEnum.next();

                    merger.merge(convertedFile);
              mergeIS.close();

                    FileOutputStream fos = new FileOutputStream(mergeFile);
                    mergeDoc.write(fos);
                    fos.flush();
                    fos.close();
                } catch (Exception mergeExcept) {
                    System.out.println("\nThere was an error in the merge");
                    mergeExcept.printStackTrace();
                }
            }
        }
    }


    /**
     *  Display usage.
     */
    private static void showUsage() {

        System.out.println("\nUsage:");
        System.out.println("\n   java org.openoffice.xmerge.test.Driver <args>");
        System.out.println("\n   where <args> is as follows:");
        System.out.println("   -from <MIMETYPE> -to <MIMETYPE> [ -merge <OrigDoc ] <document>\n");
    }


    /**
     *  Parse command-line arguments.
     *
     *  @param  args  Array of command line arguments.
     *
     *  @throws  IllegalArgumentException  If an argument is invalid.
     */
    private void parseCommandLine(String args[])
        throws IllegalArgumentException {

        if (args.length == 0) {
            throw new IllegalArgumentException();
        }

        for (int i = 0; i < args.length; i++) {
            String arg = args[i];

            if ("-to".equals(arg)) {
                toMime = extractArg(i, args);
                for (int j = 0; j < mimeTypes.length; j+=2) {
                    if(mimeTypes[j].equals(extractArg(i, args)))
                        toMime = mimeTypes[j+1];
                }
                i++;
            } else if ("-from".equals(arg)) {
                fromMime = extractArg(i, args);
                for (int j = 0; j < mimeTypes.length; j+=2) {
                    if(mimeTypes[j].equals(extractArg(i, args)))
                        fromMime = mimeTypes[j+1];
                }
                i++;
            } else if ("-merge".equals(arg)) {
                mergeFile = extractArg(i, args);
                if (!isZip(mergeFile)) {
                    throw new
                        IllegalArgumentException("Arg " + i +
                                                 ": expected zip, got " +
                                                 mergeFile);
                }
                i++;
            } else {
                deviceFiles.add(arg);
            }
        }

        System.out.println("\nConverting from " + fromMime + " to " + toMime +
                           ((mergeFile != null) ? " with merge " : " "));
    }


    /**
     *  Extract the next argument from the array, while checking to see
     *  that the array size is not exceeded.  Throw a friendly error
     *  message in case the arg is missing.
     *
     *  @param  i     Argument index.
     *  @param  args  Array of command line arguments.
     *
     *  @return  The argument with the specified index.
     *
     *  @throws  IllegalArgumentException  If an argument is invalid.
     */
    private String extractArg(int i, String args[])
        throws IllegalArgumentException {

        if (i+1 < args.length)
            return args[i+1];
        else throw new
            IllegalArgumentException("Arg " + i +
                                     ": expected arg for " + args[i]);
    }


    /**
     *  Simple validation for Office ZIP files.
     *
     *  @param  zipName  The name of the ZIP file.
     *
     *  @return  true if zipName is valid, false otherwise.
     */
    private boolean isZip(String zipName) {

        String str = zipName.toLowerCase();
        if (str.endsWith("sxw") || zipName.endsWith("sxc")) {
            return true;
        }

        return false;
    }
}

