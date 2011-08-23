/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package org.openoffice.xmerge.test;

import java.util.Vector;
import java.util.Enumeration;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.File;

import org.openoffice.xmerge.Convert;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConverterFactory;
import org.openoffice.xmerge.test.ConverterInfoList;
import org.openoffice.xmerge.util.registry.ConverterInfoMgr;
import org.openoffice.xmerge.util.registry.ConverterInfoReader;
import org.openoffice.xmerge.DocumentMerger;

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
    private Vector deviceFiles = new Vector();

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

        Enumeration jarInfoEnumeration;
        ConverterInfoReader cir;

        Enumeration jarFileEnum = cil.getJarFileEnum();
        while (jarFileEnum.hasMoreElements()) {
            String jarName = (String) jarFileEnum.nextElement();
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
            Enumeration dfEnum = deviceFiles.elements();
            while (dfEnum.hasMoreElements()) {
                processFile = (String)dfEnum.nextElement();
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
                Enumeration docEnum = dataOut.getDocumentEnumeration();
                while (docEnum.hasMoreElements()) {
                    Document docOut      = (Document)docEnum.nextElement();
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
                    Enumeration mergeDocEnum = dataOut.getDocumentEnumeration();
                    Document convertedFile = (Document)mergeDocEnum.nextElement();

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

