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
/*
 * the main Class
 *
 * Command Line arguments are reviewed
 * and a Converter is constructed
 */
package com.sun.star.tooling.converter;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Calendar;

import com.sun.star.tooling.DirtyTags.DirtyTagWrapper;

/**
 * The main class of the converter tool
 *
 * The converter tool is command line based.
 * Its classes allow the converting between the
 * file formats sdf, gsi and <a href="http://www.oasis-open.org/committees/xliff/documents/cs-xliff-core-1.1-20031031.htm">xliff</a>.
 *
 * Those file formats are used in localization
 * of Star-Office and Open-Office.
 *
 * Information about the whole localization process can be found in
 * <a href="http://ded-1.germany.sun.com/webcontent/guidelines/pdf/L10NSO8.pdf">http://ded-1.germany.sun.com/webcontent/guidelines/pdf/L10NSO8.pdf</a>
 *
 * @author Christian Schmidt 2005
 *
 */
public class Convert {

    private static Calendar     cal;

    private static final String EMPTY              = "";

    /**
     * The name of the  file containing the debug information
     * that where found while converting (every output goes here too)
     */
    private static String       dbgName            = EMPTY;

    /**
     * the character that separates the extension from the file name
     */
    private static final char   extensionSeperator = '.';

    /**
     * the log File
     */
    private static File         log;

    /**
     * the name of the log file
     */
    private static String       logString          = EMPTY;

    /**
     * indicates whether existing sources should be overwritten
     * without asking
     */
    private static boolean      overwrite          = false;

    /**
     * A second Source File needed for GSI Merging
     */
    private static File secondSource;

    //private static final char   pathSeperator      = '\\';

    /**
     * The language that should be the source language
     * that means the language to translate from
     */
    private static String       sourceLanguage     = "en-US";

    /**
     * the name of the source file
     */
    private static String       sourceName         = EMPTY;

    /**
     * the type of the source file (sdf,xliff,...)
     */
    private static String       sourceType         = EMPTY;

    /**
     * The time when converting started
     */
    private static String       startTime          = EMPTY;
    /**
    * The language that should be the target language
    * that means the language to translate to
    */
    private static String       TargetLanguage     = EMPTY;

    /**
     * the name of the target file
     */
    private static String       targetName         = EMPTY;

    /**
     * the type of the target file (sdf,xliff,...)
     */
    private static String       targetType         = EMPTY;

    /**
     * Store the current version ID and number of this tool
     */
    final static String         version            = " Prod.20050710:1255 ";

    /**
     * The name of the (original) sdf file used as second source for gsi->sdf merging
     */
    private static String secondSourceName=EMPTY;
    /**
     * Indicate whether strings in xliff files should
     * be wrapped with ept/bpt or sub tags to enable translation tools
     * to synchronize source language string with there translation
     *
     * @see <a href="http://ded-1.germany.sun.com/webcontent/guidelines/pdf/L10NSO8.pdf">http://ded-1.germany.sun.com/webcontent/guidelines/pdf/L10NSO8.pdf</a>
     */
    private static boolean doWrap=true;



            public static void main(String[] args) throws IOException, Exception {
                try{

                    //go, parse and check the command line parameters
                    ParameterChecker.checkClParameters(args);
                    ParameterChecker.createContentOfClParameters();
                    //Initialize the tagWrapper
                    DirtyTagWrapper.setWrapping(doWrap);
                    //create an instance of converter
                    Converter conv = new Converter(sourceType, sourceName, sourceLanguage,
                            targetType, targetName, TargetLanguage,secondSourceName, overwrite);
                    // get aktual time
                    cal = Calendar.getInstance();
                    startTime = cal.getTime().toString();
                    //show infos
                    printPreamble();
                    //do the job
                    conv.convert();

                    showStatistic();

                    //close log, debug...
                    OutputHandler.closeAll();

                } catch(Exception e){
                    System.out.print("An EXCEPTION occurred, please check your commad line settings \n"+e.getMessage());
                    System.exit(-1);
                }catch(Throwable t){
                    System.out.print("A FATAL ERROR occurred, please check your commad line settings \n"+t.getMessage());
                    System.exit(-1);
                }

    }

    /**
     * show the command line help
     */
    private static void printHelp() {

        final String ls = System.getProperty("line.separator");
        System.out
                .println(

                "File Converting Tool 'converter' Version "
                        + Convert.version
                        + ls
                        + "Converts SDF files to wellformed XLIFF or GSI files and vice versa" + ls
                        + ls
                        + "Use: " + ls
                        + "convert [-h]|[SourcePath [TargetPath] [-T Type] [[-S Type [secondSourcePath]]  " + ls
                        + "        [-s LanguageID] [-t LanguageID] [-l [LogPath]] [-o]]" + ls
                        + ls
                        + "-h               show this help." + ls
                        + "SourcePath       path of the file to convert." + ls
                        + "secondSourcePath path of the SDF file to merge to (GSI -> SDF only!)." + ls
                        + "TargetPath       path where to store the result." + ls
                        + "LogPath          path of the log file" + ls
                        + "-T Type          the type of the target file (xliff,sdf,gsi)" + ls
                        + "-S Type          the type of the source file (xliff,sdf,gsi)" + ls
                        + "-s LanguageID    the ISO language code of the source language (de, fr...)." + ls
                        + "                 Default is 'en-US' " + ls
                        + "-t LanguageID    the language code of the target language (de, fr...)." + ls
                        + "                 Default is first found Language other than source language." + ls
                        + "-l [LogPath]     write a log file,  you can name the file." + ls
                        + "-o               overwrite existing files without asking." + ls
                        + "-nw              disable the wrapping with ept/bpt tags." + ls
                        + ls
                        + "The created files were stored in the SourceFile Path if nothing else is given. " + ls
                        + "The extension is '.xliff','.sdf' depending on the source file and '.log' " + ls
                        + "for the logfile." + ls);
    }

    /**
     * show the parameters the converter starts with
     *
     * @throws IOException
     */
    final private static void printPreamble() throws IOException {
        OutputHandler.out(EMPTY);
        OutputHandler.out("Source File is: " + sourceName);
        OutputHandler.out("Target File is: " + targetName);
        if (OutputHandler.doLog) {
            OutputHandler.out("Log File    is: " + logString);
        } else {
            OutputHandler.out("Log File    is: disabled");
        }

        OutputHandler.out(EMPTY);

    }

    /**
     * show some statistic data
     *
     * @throws IOException
     */
    final private static void showStatistic() throws IOException {
        OutputHandler.dbg(EMPTY);
        OutputHandler
                .out((targetType.equalsIgnoreCase("xliff") ? "TransUnits written:   "
                        : "Lines written :         ")
                        + Converter.getLineCounter());
        OutputHandler.dbg(EMPTY);
        OutputHandler.out("Started    at :         " + Convert.startTime);
        Convert.cal = Calendar.getInstance();
        OutputHandler.out("Finished   at :         " + Convert.cal.getTime());
        OutputHandler.dbg(EMPTY);

    }

    /**
     * Get the extensiion of a file name
     * (sdf,xliff,gsi)
     *
     * @param sourceString     the file name
     * @return                 the extension
     */
    static protected String extractExtension(String sourceString) {
        String ext = sourceString.substring(sourceString
                .lastIndexOf(Convert.extensionSeperator) + 1);
        return ext;
    }

//    static protected String extractFileName(String sourceString) {
//        String sName = EMPTY;
//        sName = (sourceString.substring(sourceString
//                .lastIndexOf(File.separator) + 1, sourceString
//                .lastIndexOf(Convert.extensionSeperator)));
//
//        return sName;
//    }
//
//    static protected String extractPath(String sourceString) {
//        String sPath = sourceString.substring(0, sourceString
//                .lastIndexOf(File.separator) + 1);
//        return sPath;
//    }

    /**
     *
     */
    public Convert() {
    }

    /**
     *
     * Verify a parameter array and create content useable by the programm
     * from the switches and attributes set at command line
     *
     * @author Christian Schmidt 2005
     */
    private static class ParameterChecker {

        /**
         * Holds the path of the source file
         */
        private static String filePath;
        /**
         * Holds the name of the source file
         */
        private static String fileName;

        /**
         * Create a new Instance of ParameterChecker
         *
         *
         */
        public ParameterChecker(){};
        /**
         * Checks the command line parameters
         *
         * @param args          the parameters to check and to parse
         * @throws IOException
         */
        private static void checkClParameters(String[] args) throws IOException {
            try {
                //show help if no attrributes...
                if (args.length == 0) {
                    printHelp();
                    System.exit(-1);
                }
                //...or attribute is -h
                if (args[0].equals("-h")) {
                    printHelp();
                    System.exit(0);
                }
                if (args[0].equals("-ver")) {
                    System.out.println("File Converting Tool Version "+version);
                    System.exit(0);
                }
                //source file Location and path is always first attribute
                sourceName = new String(args[0]);

                File source = new File(sourceName);
                //break if there is no source to convert
                if (!source.exists())
                    throw new IOException("ERROR:Can not find Source File '"
                            + sourceName + "'. Aborting...");

    //            String name=source.getName();
    //            String parent=source.getParent();
    //            String path=source.getPath();

                filePath = (source.getParent()==null)?"":source.getParent()+File.separator; //extractPath(sourceName);
                fileName = source.getName().substring(0,source.getName().lastIndexOf(extensionSeperator));

                for (int i = 1; i < args.length; i++) {


                        if ("-s".equals(args[i])) {
                            if (args.length > i + 1) {
                                sourceLanguage = args[++i];

                            } else {
                                throw new ConverterException(
                                        "missing argument for -s source language");
                            }
                            continue;
                        }

                        if ("-S".equals(args[i])) {
                            if (args.length > i + 1) {
                                sourceType = args[++i];
                                if (args.length > i +1 &&!args[i+1].startsWith("-")) {
                                    secondSourceName = args[++i];
                                }

                            } else {
                                throw new ConverterException(
                                        "missing argument for -S  source type");
                            }
                            continue;
                        }

                        if ("-T".equals(args[i])) {
                            if (args.length > i + 1) {
                                targetType = args[++i];


                            } else {
                                throw new ConverterException(
                                        "missing argument for -T  target type");
                            }
                            continue;
                        }

                        if ("-l".equals(args[i])) {
                            OutputHandler.doLog = true;
                            if (args.length > i + 1
                                    && (!args[i + 1].startsWith("-"))) {
                                logString = args[++i];
                            } else {
                                logString = EMPTY;
                            }
                            continue;
                        }

                        if ("-o".equals(args[i])) {
                            overwrite = true;
                            continue;
                        }

                        if ("-nw".equals(args[i])) {
                            doWrap = false;
                            continue;
                        }

                        if ("-h".equals(args[i])) {
                            printHelp();
                            System.exit(0);
                        }

                        if ("-dbg".equals(args[i])) {
                            OutputHandler.doDebug = true;
                            continue;
                        }

                        if ("-t".equals(args[i])) {
                            if (args.length > i + 1) {
                                TargetLanguage = args[++i];
                            } else {
                                throw new ConverterException(
                                        "missing argument for -t  target language");
                            }
                            continue;
                        }

                        if (i == 1 && !args[i].startsWith("-")) { //target file
                            // found
                            targetName = args[i];
                            continue;
                        }
                        //if we come here we
                        //can not match the Attribute
                        throw new ConverterException("unknown Attribute: "
                                + args[i]);


                }//end for
            } catch (ConverterException e) {
                OutputHandler.out("ERROR: "+e.getMessage());
                System.exit(-1);
            } catch (Throwable t){
                System.out.print("An Error occurred while parsing the command line,\n please check your commad line settings.\n "+t.getMessage());
                System.exit(-1);
            }

        }//end checkClParameters

        /**
         * Creates the appropriate content of what ever data
         * we found in the command line
         *
         * @throws IOException
         */
        private static void createContentOfClParameters() throws IOException {

            try {
                if (OutputHandler.doDebug) {
                    // if the -dbg switch is set, we
                    // create
                    // a file that gets all information
                    // produced by this tool

                    OutputHandler.dbgFile = new BufferedWriter(new FileWriter(
                            new File(new String(filePath + fileName + ".dbg"))));
                }

                if (OutputHandler.doLog) {// create a logfile?
                    //given at command line?
                    if (EMPTY.equals(logString) || logString == null) {
                        logString = new String(filePath + fileName + ".log");
                    }
                    log = FileMaker.newFile(logString, overwrite);
                    OutputHandler.logFile = (new BufferedWriter(new FileWriter(
                            log)));
                }

                if (EMPTY.equals(sourceType) || sourceType == null) {
                    // not given at command line?
                    if (!(EMPTY.equals(sourceName) || sourceName == null)) {
                        sourceType = extractExtension(sourceName);
                    } else {
                        throw new ConverterException("Source type is missing");
                    }
                }

                if(sourceType.equalsIgnoreCase("gsi")){

                    if(EMPTY.equals(Convert.secondSourceName)){
                        Convert.secondSourceName=filePath+fileName+".sdf";
                    }
                    //secondSource=new File(Convert.secondSourceName);

                }

                if (EMPTY.equals(sourceName)) {
                    sourceName = filePath + fileName + "." + sourceType;
                }
                //no target type given at command line?
                if (EMPTY.equals(targetType) || targetType == null) {
                    if (!(EMPTY.equals(targetName) || targetName == null)) {
                        targetType = extractExtension(targetName);
                    } else {
                        throw new ConverterException("Target type is missing");

                    }
                }
                //no target File specified at command line
                if (EMPTY.equals(targetName) || targetName == null) {
                    targetName = filePath + fileName + "." + targetType;
                    if (targetName.equals(Convert.secondSourceName)){
                        OutputHandler.out("ERROR: \nSource '"+Convert.secondSourceName+"' and \nTarget'"+targetName+"' are the same");
                        System.exit(0);
                    }else if (targetName.equals(Convert.sourceName)){
                        OutputHandler.out("ERROR: \nSource '"+Convert.sourceName+"' and \nTarget'"+targetName+"' are the same");
                        System.exit(0);
                    }
                }else if (targetName.equals(Convert.secondSourceName)){
                    OutputHandler.out("ERROR: \nSource '"+Convert.secondSourceName+"' and \nTarget'"+targetName+"' are the same");
                    System.exit(0);
                }else if (targetName.equals(Convert.sourceName)){
                    OutputHandler.out("ERROR: \nSource '"+Convert.sourceName+"' and \nTarget'"+targetName+"' are the same");
                    System.exit(0);
                }


            } catch (ConverterException e) {
                OutputHandler.out(e.getMessage());
            }

        }

    }

}