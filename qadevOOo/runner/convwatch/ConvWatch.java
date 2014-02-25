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

package convwatch;

import convwatch.ConvWatchException;
import convwatch.ConvWatchCancelException;
import convwatch.FileHelper;
import convwatch.OfficePrint;
import convwatch.PRNCompare;
import convwatch.StatusHelper;
import helper.URLHelper;
import java.io.File;

public class ConvWatch
{

    String getBuildID_FromFile(String _sInfoFile)
    {
        String sBuildID = "";
        IniFile aIniFile = new IniFile(_sInfoFile);
        if (aIniFile.is())
        {
            sBuildID = aIniFile.getValue("", "buildid");
        }
        return sBuildID;
    }
    /**
     * Check if given document (_sAbsoluteInputFile) and it's postscript representation (_sAbsoluteReferenceFile) produce
     * the same output like the StarOffice / OpenOffice.org which is accessible with XMultiServiceFactory.
     * Here a simple graphically difference check is run through.
     *
     * Hint: In the OutputPath all needed files will create, there must
     * be very much space. It's not possible to say how much.
     * One page need up to 800kb as jpeg.
     * Sample: If a document contains 2 pages, we need 2*800kb for prn
     * output and 2*800kb for ps output and 2*800kb for it's difference
     * output. So up to 4800kb or 4.8mb.
     *
     * RAM is need least 300mb. Will say, it's tested on a pc with 128mb RAM and 256mb swap.
     *
     * It's also absolutlly impossible to say, how much time this functions consume.
     */

    StatusHelper[] createPostscriptStartCheck(GraphicalTestArguments _aGTA,
                                              String _sOutputPath, String _sAbsoluteInputFile, String _sAbsoluteReferenceFile)
        throws ConvWatchCancelException
        {
//  TODO: some more checks

            if (! FileHelper.exists(_sAbsoluteInputFile))
            {
                throw new ConvWatchCancelException("createPostscriptStartCheck: Input file: " + _sAbsoluteInputFile + " does not exist.");
            }

            if (_sAbsoluteReferenceFile == null)
            {
                // we assume, that the prn file is near the document
                String sAbsoluteInputFileNoSuffix = FileHelper.getNameNoSuffix(_sAbsoluteInputFile);
                _sAbsoluteReferenceFile = sAbsoluteInputFileNoSuffix + ".prn";
            }

            String fs = System.getProperty("file.separator");
            File aAbsoluteReferenceFile = new File(_sAbsoluteReferenceFile);

            if (aAbsoluteReferenceFile.isDirectory())
            {
                String sBasename = FileHelper.getBasename(_sAbsoluteInputFile);
                String sNameNoSuffix = FileHelper.getNameNoSuffix(sBasename);
                String sAbsoluteReferenceFileInfo = _sAbsoluteReferenceFile + fs + sNameNoSuffix + ".info";
                _sAbsoluteReferenceFile = _sAbsoluteReferenceFile + fs + sNameNoSuffix + ".prn";

                // Read the reference from the info file
                String sRefBuildID = "";
                if (FileHelper.exists(sAbsoluteReferenceFileInfo))
                {
                    sRefBuildID = getBuildID_FromFile(sAbsoluteReferenceFileInfo);
                }
                _aGTA.setRefBuildID(sRefBuildID);

            }
            else
            {
                // java file has problems to check for directories, if the given directory doesn't exist.
                String sName = FileHelper.getBasename(_sAbsoluteReferenceFile);
                // thanks to Mircosoft, every document has a suffix, so if a name doesn't have a suffix, it must be a directory name
                int nIdx = sName.indexOf('.');
                if (nIdx == -1)
                {
                    // must be a directory
                    throw new ConvWatchCancelException("createPostscriptStartCheck: Given reference directory: '" + _sAbsoluteReferenceFile + "' does not exist.");
                }
            }


            boolean bAbsoluteReferenceFile = true;
            if (! FileHelper.exists(_sAbsoluteReferenceFile))
            {
                if (_aGTA.createDefaultReference())
                {
                    GlobalLogWriter.get().println("Reference File doesn't exist, will create a default");
                    bAbsoluteReferenceFile = false;
                }
                else
                {
                    throw new ConvWatchCancelException("createPostscriptStartCheck: Given reference file: " + _sAbsoluteReferenceFile + " does not exist.");
                }
            }

            FileHelper.makeDirectories("", _sOutputPath);

            // runner.convwatch.compare();

            String sAbsoluteInputFileURL = URLHelper.getFileURLFromSystemPath(_sAbsoluteInputFile);

            String sInputFile = FileHelper.getBasename(_sAbsoluteInputFile);
            // System.out.println("InputFile: " + sInputFile);

            FileHelper.getNameNoSuffix(sInputFile);


            String sAbsoluteOutputFile = _sOutputPath + fs + sInputFile;
            String sAbsoluteOutputFileURL = URLHelper.getFileURLFromSystemPath(sAbsoluteOutputFile);

            String sReferenceFile = FileHelper.getBasename(_sAbsoluteReferenceFile);
            String sReferenceFileNoSuffix = FileHelper.getNameNoSuffix(sReferenceFile);
            String sPostScriptFile = sReferenceFileNoSuffix + ".ps";
            // System.out.println("PostscriptFile: " + sPostScriptFile);

            String sAbsolutePrintFile = _sOutputPath + fs + sPostScriptFile;
            String sAbsolutePrintFileURL = URLHelper.getFileURLFromSystemPath(sAbsolutePrintFile);

            // System.out.println("AbsoluteInputFileURL: " + sAbsoluteInputFileURL);
            // System.out.println("AbsoluteOutputFileURL: " + sAbsoluteOutputFileURL);
            // System.out.println("AbsolutePrintFileURL: " + sAbsolutePrintFileURL);

            // store and print the sAbsoluteInputFileURL file with StarOffice / OpenOffice.org
            OfficePrint.printToFile(_aGTA, sAbsoluteInputFileURL, sAbsoluteOutputFileURL, sAbsolutePrintFileURL);

            // wait(2);

            if (! FileHelper.exists(sAbsolutePrintFile))
            {
                throw new ConvWatchCancelException("createPostscriptStartCheck: Printed file " + sAbsolutePrintFile + " does not exist.");
            }

            if (bAbsoluteReferenceFile == false)
            {
                // copy AbsolutePrintFile to AbsoluteReferenceFile
                String sDestinationFile = sAbsolutePrintFile; // URLHelper.getSystemPathFromFileURL(...)
                String sSourceFile = _sAbsoluteReferenceFile;
                FileHelper.copy(sDestinationFile, sSourceFile);
                // now the fix reference of the AbsoluteReferenceFile should exist.
                if (! FileHelper.exists(_sAbsoluteReferenceFile))
                {
                    throw new ConvWatchCancelException("createPostscriptStartCheck: Given reference file: " + _sAbsoluteReferenceFile + " does not exist, after try to copy.");
                }
            }

            PRNCompare a = new PRNCompare();
            String sInputPath = FileHelper.getPath(_sAbsoluteInputFile);
            String sReferencePath = FileHelper.getPath(_sAbsoluteReferenceFile);
            // String sReferenceFile = FileHelper.getBasename(sAbsoluteReferenceFile);

            // System.out.println("InputPath: " + sInputPath);
            // System.out.println("sReferencePath: " + sReferencePath);
            // System.out.println("sReferenceFile: " + sReferenceFile);

            a.setInputPath(     sInputPath );
            a.setReferencePath( sReferencePath );
            a.setOutputPath(    _sOutputPath );
            // a.setDocFile(       "1_Gov.ppt");
            a.setReferenceFile( sReferenceFile );
            a.setPostScriptFile(sPostScriptFile );
            if (_aGTA.printAllPages() == true)
            {
                a.setMaxPages(9999);
            }
            else
            {
                if (_aGTA.getMaxPages() > 0)
                {
                    a.setMaxPages(_aGTA.getMaxPages());
                }
                if (_aGTA.getOnlyPages().length() != 0)
                {
                    // we can't interpret the string of getOnlyPages() right without much logic, so print all pages here!
                    a.setMaxPages(9999);
                }
            }

            a.setResolutionInDPI(_aGTA.getResolutionInDPI());
            a.setBorderMove(_aGTA.getBorderMove());
            a.setDocumentType(_aGTA.getDocumentType());

            StatusHelper[] aList = a.compare();

            _aGTA.setBorderMove(a.getBorderMove());
            return aList;
        }


    // This creates a status for exact on document
    static boolean createINIStatus(StatusHelper[] aList, String _sFilenamePrefix, String _sOutputPath, String _sAbsoluteInputFile, String _sBuildID, String _sRefBuildID)
        {
            System.getProperty("file.separator");
            String sBasename = FileHelper.getBasename(_sAbsoluteInputFile);
            String sNameNoSuffix = FileHelper.getNameNoSuffix(sBasename);
//            String sHTMLFile = _sFilenamePrefix + sNameNoSuffix + ".html";
//            HTMLOutputter HTMLoutput = HTMLOutputter.create(_sOutputPath, sHTMLFile, "", "");
//            HTMLoutput.header(sNameNoSuffix);
//  TODO: version info was fine
//            HTMLoutput.checkSection(sBasename);
            // Status end

            String sINIFile = _sFilenamePrefix + sNameNoSuffix + ".ini";
            INIOutputter INIoutput = INIOutputter.create(_sOutputPath, sINIFile, "", "");
            INIoutput.createHeader();
//  TODO: version info was fine

            INIoutput.writeSection("global");
            INIoutput.writeValue("pages", String.valueOf(aList.length));
            INIoutput.writeValue("buildid", _sBuildID);
            INIoutput.writeValue("refbuildid", _sRefBuildID);
            INIoutput.writeValue("diffdiff", "no");
            INIoutput.writeValue("basename", sBasename);

            boolean bResultIsOk = true;          // result over all pages
            for (int i=0;i<aList.length; i++)
            {
                INIoutput.writeSection("page" + String.valueOf(i + 1));   // list start at point 0, but this is page 1 and so on... current_page = (i + 1)
                aList[i].printStatus();

                boolean bCurrentResult = true;   // result over exact one page

                int nCurrentDiffStatus = aList[i].nDiffStatus;

                // check if the status is in a defined range
                if (nCurrentDiffStatus == StatusHelper.DIFF_NO_DIFFERENCES)
                {
                    // ok.
                }
                else if (nCurrentDiffStatus == StatusHelper.DIFF_DIFFERENCES_FOUND && aList[i].nPercent < 5)
                {
                    // ok.
                }
                else if (nCurrentDiffStatus == StatusHelper.DIFF_AFTER_MOVE_DONE_NO_PROBLEMS)
                {
                    // ok.
                }
                else if (nCurrentDiffStatus == StatusHelper.DIFF_AFTER_MOVE_DONE_DIFFERENCES_FOUND && aList[i].nPercent2 < 5)
                {
                    // ok.
                }
                else
                {
                    // failed.
                    bCurrentResult = false; // logic: nDiff==0 = true if there is no difference
                }

                // Status
//                HTMLoutput.checkLine(aList[i], bCurrentResult);
                INIoutput.checkLine(aList[i], bCurrentResult);
                bResultIsOk &= bCurrentResult;
            }
            // Status
//            HTMLoutput.close();
            INIoutput.close();
            return bResultIsOk;
        }



    static void createINIStatus_DiffDiff(StatusHelper[] aDiffDiffList, String _sFilenamePrefix, String _sOutputPath, String _sAbsoluteInputFile, String _sBuildID)
        {
            System.getProperty("file.separator");
            String sBasename = FileHelper.getBasename(_sAbsoluteInputFile);
            String sNameNoSuffix = FileHelper.getNameNoSuffix(sBasename);
            String sINIFile = _sFilenamePrefix + sNameNoSuffix + ".ini";

//            HTMLOutputter HTMLoutput = HTMLOutputter.create(_sOutputPath, sHTMLFile, _sFilenamePrefix, "");
//            HTMLoutput.header(sNameNoSuffix);
//            HTMLoutput.checkDiffDiffSection(sBasename);

            INIOutputter INIoutput = INIOutputter.create(_sOutputPath, sINIFile, _sFilenamePrefix, "");
            INIoutput.createHeader();
            // LLA? what if the are no values in the list? true or false;
            INIoutput.writeSection("global");
            INIoutput.writeValue("pages", String.valueOf(aDiffDiffList.length));
            INIoutput.writeValue("buildid", _sBuildID);
            INIoutput.writeValue("diffdiff", "yes");
            INIoutput.writeValue("basename", sBasename);

            for (int i=0;i<aDiffDiffList.length; i++)
            {
                INIoutput.writeSection("page" + String.valueOf(i + 1));   // list start at point 0, but this is page 1 and so on... current_page = (i + 1)
                boolean bCurrentResult = (aDiffDiffList[i].nDiffStatus == StatusHelper.DIFF_NO_DIFFERENCES); // logic: nDiff==0 = true if there is no difference

//                HTMLoutput.checkDiffDiffLine(aDiffDiffList[i], bCurrentResult);
                INIoutput.checkDiffDiffLine(aDiffDiffList[i], bCurrentResult);
            }
            // Status
//            HTMLoutput.close();
            INIoutput.close();
        }




    public static boolean check(GraphicalTestArguments _aGTA,
                             String _sOutputPath, String _sAbsoluteInputFile, String _sAbsoluteReferenceFile)
        throws ConvWatchCancelException, ConvWatchException
        {
            ConvWatch a = new ConvWatch();
            StatusHelper[] aList = a.createPostscriptStartCheck(_aGTA, _sOutputPath, _sAbsoluteInputFile, _sAbsoluteReferenceFile);
            DB.writeNumberOfPages(aList.length);

            boolean bResultIsOk = createINIStatus(aList, "", _sOutputPath, _sAbsoluteInputFile, _aGTA.getBuildID(), _aGTA.getRefBuildID());

            if (! bResultIsOk)
            {
                // it could be that this will store in a DB, there are problems with '\'
                String sErrorMessage = "Graphical compare failed with file ";
                String sErrorFile = _sAbsoluteInputFile.replace('\\', '/');
                sErrorMessage = sErrorMessage + "'" + sErrorFile + "'";
                DB.writeErrorFile(sErrorFile);
                throw new ConvWatchException(sErrorMessage);
            }
            return bResultIsOk;
        }


    public static boolean checkDiffDiff(GraphicalTestArguments _aGTA,
                                     String _sOutputPath, String _sAbsoluteInputFile, String _sAbsoluteReferenceFile,
                                     String _sAbsoluteDiffPath)
        throws ConvWatchCancelException, ConvWatchException
        {
            ConvWatch a = new ConvWatch();
            _aGTA.setBorderMove(TriState.FALSE);
            StatusHelper[] aList = a.createPostscriptStartCheck(_aGTA, _sOutputPath, _sAbsoluteInputFile, _sAbsoluteReferenceFile);

            createINIStatus(aList, "", _sOutputPath, _sAbsoluteInputFile, _aGTA.getBuildID(), _aGTA.getRefBuildID());

            StatusHelper[] aDiffDiffList = new StatusHelper[aList.length];

            String fs = System.getProperty("file.separator");

            boolean bDiffIsOk = true;
            boolean bFoundAOldDiff = false;

            PRNCompare aCompare = new PRNCompare();
            // LLA? what if the are no values in the list? true or false;
            for (int i=0;i<aList.length; i++)
            {
                String sOrigDiffName = aList[i].m_sDiffGfx;
                String sDiffBasename = FileHelper.getBasename(sOrigDiffName);

                String sNewDiffName = _sAbsoluteDiffPath + fs + sDiffBasename;
                if (! FileHelper.exists(sNewDiffName))
                {
                    GlobalLogWriter.get().println("checkDiffDiff: Old diff file: '" + sNewDiffName + "' does not exist." );
                    continue;
                }
                // String sNewDiffName = _sAbsoluteDiffPath + fs + sDiffBasename;

                // make a simple difference between these both diff files.
                String sSourcePath1 = FileHelper.getPath(sOrigDiffName);
                String sSourceFile1 = sDiffBasename;
                String sSourcePath2 = _sAbsoluteDiffPath;
                String sSourceFile2 = sDiffBasename;

                StatusHelper aCurrentStatus = aCompare.checkDiffDiff(_sOutputPath, sSourcePath1, sSourceFile1, sSourcePath2, sSourceFile2);
                boolean bCurrentResult = (aCurrentStatus.nDiffStatus == StatusHelper.DIFF_NO_DIFFERENCES); // logic: nDiff==0 = true if there is no difference
                bDiffIsOk &= bCurrentResult;
                bFoundAOldDiff = true;

                aDiffDiffList[i] = aCurrentStatus;
            }

            createINIStatus_DiffDiff(aDiffDiffList, "DiffDiff_", _sOutputPath, _sAbsoluteInputFile, _aGTA.getBuildID());

            if (bFoundAOldDiff == false)
            {
                throw new ConvWatchCancelException("No old difference file found." );
            }
            if (! bDiffIsOk)
            {
                throw new ConvWatchException("Graphical difference compare failed with file '" + _sAbsoluteInputFile + "'");
            }
            return bDiffIsOk;
        }

    // public static void main( String[] argv )
    //     {
    //         PRNCompare a = new PRNCompare();
    //         a.setInputPath(     "/cws/so-cwsserv06/qadev18/SRC680/src.m47/convwatch.keep/input/msoffice/xp/PowerPoint");
    //         a.setDocFile(       "1_Gov.ppt");
    //         a.setReferencePath( "/cws/so-cwsserv06/qadev18/SRC680/src.m47/convwatch.keep/input/msoffice/xp/PowerPoint");
    //         a.setReferenceFile( "1_Gov.prn" );

    //         a.setOutputPath(    "/tmp/convwatch_java");
    //         a.setPostScriptFile("1_Gov.ps" );
    //     }
}
