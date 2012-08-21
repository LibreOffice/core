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

package graphical;

import helper.OSHelper;
import helper.ProcessHandler;
import java.io.File;

/**
 * Helper class to interpret a jpg filename
 */
class NameDPIPage
{

    String Name;
    int DPI;
    int Page;

    private NameDPIPage(String _sName, int _nDPI, int _nPage)
    {
        Name = _sName;
        DPI = _nDPI;
        Page = _nPage;
    }

    public static NameDPIPage interpret(String _sFilename)
    {
        String sBasename = FileHelper.getBasename(_sFilename);         // if exist a path, remove it
        String sNameNoSuffix = FileHelper.getNameNoSuffix(sBasename);  // remove extension (.jpg)

        // check if there exist a 'DPI_' at specific position
        String sDPICheck = sNameNoSuffix.substring(sNameNoSuffix.length() - 8, sNameNoSuffix.length() - 4);
        String sName;
        int nDPI = -1;
        int nPage = -1;
        if (sDPICheck.equals("DPI_"))
        {
            // seems to be a generated filename by us.
            int nDPIStart = sNameNoSuffix.lastIndexOf("_", sNameNoSuffix.length() - 8);
            sName = sNameNoSuffix.substring(0, nDPIStart);
            if (nDPIStart > 0)
            {
                String sDPI = sNameNoSuffix.substring(nDPIStart + 1, sNameNoSuffix.length() - 8);
                try
                {
                    nDPI = Integer.valueOf(sDPI).intValue();
                }
                catch (java.lang.NumberFormatException e)
                {
                    GlobalLogWriter.println("DPI: Number format exception");
                }
                String sPage = sNameNoSuffix.substring(sNameNoSuffix.length() - 4);
                try
                {
                    nPage = Integer.valueOf(sPage).intValue();
                }
                catch (java.lang.NumberFormatException e)
                {
                    GlobalLogWriter.println("Page: Number format exception");
                }
            }
        }
        else
        {
            sName = sNameNoSuffix;
        }

        return new NameDPIPage(sName, nDPI, nPage);
    }
}

class CountNotXXXPixelsFromImage extends Thread
{

    private String m_sFilename;
    protected int m_nValue;

    CountNotXXXPixelsFromImage(String _sFilename)
    {
        m_sFilename = _sFilename;
    }

    public int getValue()
    {
        return m_nValue;
    }

    protected void setValue(int _nValue)
    {
        m_nValue = _nValue;
    }

    protected String getFilename()
    {
        return m_sFilename;
    }
}

class CountNotWhitePixelsFromImage extends CountNotXXXPixelsFromImage
{

    CountNotWhitePixelsFromImage(String _sFilename)
    {
        super(_sFilename);
    }

    public void run()
    {
        try
        {
            final int nNotWhiteCount = PixelCounter.countNotWhitePixelsFromImage(getFilename());
            setValue(nNotWhiteCount);
        }
        catch (java.io.IOException e)
        {
            m_nValue = -1;
        }
    }
}

class CountNotBlackPixelsFromImage extends CountNotXXXPixelsFromImage
{

    CountNotBlackPixelsFromImage(String _sFilename)
    {
        super(_sFilename);
    }

    public void run()
    {
        try
        {
            final int nNotBlackCount = PixelCounter.countNotBlackPixelsFromImage(getFilename());
            setValue(nNotBlackCount);
        }
        catch (java.io.IOException e)
        {
            m_nValue = -1;
        }
    }
}

/**
 *
 * @author ll93751
 */
public class JPEGComparator extends EnhancedComplexTestCase
{
    // @Override

    public String[] getTestMethodNames()
    {
        return new String[]{"CompareJPEGvsJPEG"};
    }
    private Tolerance m_aTolerance;

    /**
     * test function.
     */
    public void CompareJPEGvsJPEG()
    {
        GlobalLogWriter.set(log);
        ParameterHelper aParam = new ParameterHelper(param);

        // run through all documents found in Inputpath
        foreachJPEGcompareWithJPEG(aParam);
    }

    public void checkOneFile(String _sDocumentName, String _sResult, ParameterHelper _aParams) throws OfficeException
    {
        // private void callEveryPictureInIniFile(IniFile _aIniFile, String _sSectionName, ParameterHelper _aParam)
        // {
        String sPath = FileHelper.getPath(_sDocumentName);
        String sSectionName = FileHelper.getBasename(_sDocumentName);

        // take the build id out of the ini file in the reference file and put it into the current parameter helper
        String sIniFileForRefBuildID = FileHelper.appendPath(sPath, sSectionName + ".ini");
        IniFile aIniFileForRefBuildID = new IniFile(sIniFileForRefBuildID);
        String sRefBuildID = aIniFileForRefBuildID.getValue("global", "buildid");
        aIniFileForRefBuildID.close();

        _aParams.getTestParameters().put("RefBuildId", sRefBuildID);

        String sIniFile = FileHelper.appendPath(sPath, "index.ini");
        IniFile aIniFile = new IniFile(sIniFile);
        if (aIniFile.hasValue(sSectionName, "pages"))
        {
            // only which has 'pages' has also pictures
            int nPages = aIniFile.getIntValue(sSectionName, "pages", 0);
            String sJPEGSchema = aIniFile.getValue(sSectionName, "jpegschema");
            int nTolerance = aIniFile.getIntValue(sSectionName, "tolerance", 0);
            m_aTolerance = new Tolerance(nTolerance);
            for (int i = 1; i <= nPages; i++)
            {
                String sJPEGFilename = JPEGCreator.getFilenameForJPEGSchema(sJPEGSchema, i);
                // String sPath = FileHelper.getPath(_aParam.getInputPath());
                String sJPEGPath = FileHelper.getPath(sJPEGFilename);
                if (!sPath.equals(sJPEGPath))
                {
                    GlobalLogWriter.println("Path where to find the index and where to file the JPEG pictures are not the same.");

                }
                // String sEntry = FileHelper.appendPath(sPath, sSection);
                File aFile = new File(sJPEGFilename);
                assure("File '" + sJPEGFilename + "' doesn't exists.", aFile.exists(), true);
                if (aFile.exists())
                {
                    GlobalLogWriter.println("Page: " + i);
                    checkOnePicture(sJPEGFilename, _sResult, _aParams);
                }
            }
        }
        else
        {
            GlobalLogWriter.println("The document '" + sSectionName + "' seems to have no picture representation.");
        }

        String sResultIniFile = FileHelper.appendPath(_sResult, sSectionName);
        evaluateResult(sResultIniFile, _aParams);
    }

    private void evaluateResult(String _sDocument, ParameterHelper _aParams)
    {
        String sResultIniFile = _sDocument + ".ini";
        File aFile = new File(sResultIniFile);
        assure("Result file doesn't exists " + sResultIniFile, aFile.exists());

        int good = 0;
        int bad = 0;
        int ugly = 0;
        int ok_status = 1; // 1=ok 2=bad 3=ugly

        IniFile aResultIniFile = new IniFile(sResultIniFile);
        int nPages = aResultIniFile.getIntValue("global", "pages", 0);
        for (int i = 0; i < nPages; i++)
        {
            String sCurrentPage = "page" + String.valueOf(i + 1);
            int nPercent = aResultIniFile.getIntValue(sCurrentPage, "percent", -1);
            if (nPercent == 0)
            {
                good++;
            }
            else if (nPercent <= 5)
            {
                bad++;
                ok_status = 2;
            }
            else
            {
                ugly++;
                ok_status = 3;
            }
        }

        assure("Error: document doesn't contains pages", nPages > 0);

// TODO: this information has to come out of the ini files
        String sStatusRunThrough = "PASSED, ";
        String sPassed = "OK";

        String sStatusMessage = "From " + nPages + " page(s) are: ";
        String sGood = "";
        String sBad = "";
        String sUgly = "";

        if (good > 0)
        {
            sGood = " good:=" + good;
            sStatusMessage += sGood;
        }
        if (bad > 0)
        {
            sBad = " bad:=" + bad;
            sStatusMessage += sBad;
        }
        if (ugly > 0)
        {
            sUgly = " ugly:=" + ugly;
            sStatusMessage += sUgly;
        }

        // Failure matrix
        //         0     1
        // ugly    OK    FAILED
        // bad     OK
        // good    OK

        if (ugly > 0)
        {
            sPassed = "FAILED";
        }
        else
        {
            if (bad > 0)
            {
                sPassed = "NEED A LOOK";
            }
            else
            {
                sPassed = "OK";
            }
        }
        sStatusRunThrough += sPassed;
        aResultIniFile.insertValue("global", "state", sStatusRunThrough);
        aResultIniFile.insertValue("global", "info", sStatusMessage);
        aResultIniFile.close();

        _aParams.getTestParameters().put("current_state", sStatusRunThrough);
        _aParams.getTestParameters().put("current_info", sStatusMessage);
        _aParams.getTestParameters().put("current_ok_status", ok_status);

        // if we have a ugly page, we must return this as a FAILED STATUS in Log file!
        assure("There exist pages marked as ugly.", ugly == 0);
    }

    private void checkOnePicture(String _sDocumentName, String _sResult, ParameterHelper _aParams)
    {
        GlobalLogWriter.println("JPEG: Compare difference between '" + _sDocumentName + "'  and '" + _sResult + "'");
        File aResultFile = new File(_sResult);
        if (aResultFile.isDirectory())
        {
            // result is just a directory, so we search for the basename of the source and take this.
            String sBasename = FileHelper.getBasename(_sDocumentName);
            String sResultFilename = FileHelper.appendPath(_sResult, sBasename);
            aResultFile = new File(sResultFilename);
            if (aResultFile.exists())
            {
                // Original and Result exists
                String sInputPath = _aParams.getInputPath();
                if (sInputPath.toLowerCase().endsWith("index.ini"))
                {
                    // special case
                    // we want to get the buildid from the info file.
                }

                compareJPEG(_sDocumentName, sResultFilename, _aParams);

            }
            else
            {
                String sResultFilenamePDF = util.utils.replaceAll13(sResultFilename, ".ps_", ".pdf_");
                File aResultPDFFile = new File(sResultFilenamePDF);
                if (aResultPDFFile.exists())
                {
                    // Original and Result exists
                    String sInputPath = _aParams.getInputPath();
                    if (sInputPath.toLowerCase().endsWith("index.ini"))
                    {
                        // special case
                        // we want to get the buildid from the info file.
                    }

                    compareJPEG(_sDocumentName, sResultFilenamePDF, _aParams);
                }
                else
                {
                    GlobalLogWriter.println("Warning: Result JPEG doesn't exists '" + sResultFilename + "'");
                }
            }
        }
        else
        {
            // result is also a file
            if (aResultFile.exists())
            {
                compareJPEG(_sDocumentName, _sResult, _aParams);
            }
            else
            {
                GlobalLogWriter.println("Warning: Result JPEG doesn't exists '" + _sResult + "'");
            }
        }
    }

    /**
     * compare 2 JPEGs, it is a need, that both _sDocumentName and _sResultFilename exist.
     * @param _sDocumentName
     * @param _sResult
     * @param _aParams
     */
    private void compareJPEG(String _sDocumentName, String _sResult, ParameterHelper _aParams)
    {
        NameDPIPage aNameDPIPage = NameDPIPage.interpret(_sDocumentName);

        String sSourceBasename = FileHelper.getBasename(_sDocumentName);
        String sSourcePath = FileHelper.getPath(_sDocumentName);
        String sDestinationBasename = FileHelper.getBasename(_sResult);
        String sDestinationPath = FileHelper.getPath(_sResult);

        if (!sSourcePath.equals(sDestinationPath))
        {
            // we want to have all in one Directory, Original, Reference and the Difference result.
            // copy the original file to the reference path
            String sNewSourceBasename = "Original_" + sSourceBasename;
            // String sSource = FileHelper.appendPath(sSourcePath, sSourceBasename);
            String sSource = _sDocumentName;
            String sDestination = FileHelper.appendPath(sDestinationPath, sNewSourceBasename);
            FileHelper.copy(sSource, sDestination);
            sSourceBasename = sNewSourceBasename;
            //
            JPEGCreator.convertToNearSameFileWithWidth340(sDestination);
        }
        String sDifferenceBasename = "Difference_between_" + FileHelper.getNameNoSuffix(sSourceBasename) + "_and_" + FileHelper.getNameNoSuffix(sDestinationBasename) + ".jpg";
        // String sDifferencePath = sDestinationPath;

        String sSource = FileHelper.appendPath(sDestinationPath, sSourceBasename);
        String sDestination = FileHelper.appendPath(sDestinationPath, sDestinationBasename);
        String sDifference = FileHelper.appendPath(sDestinationPath, sDifferenceBasename);
        int nErr = compareJPEG(sSource, sDestination, sDifference);
        if (nErr == 0 && FileHelper.exists(sDifference))
        {
            // check the difference, returns the count of different colors
            // this means, 1=only one color, no differences found.
            int nResult = identify(sDifference);
            int nPercentColorDiffer = 0;

            String sResult = "YES";

            if (m_aTolerance != null)
            {
                final int nAcceptedTolerance = m_aTolerance.getAccept();
                if (nResult <= nAcceptedTolerance)
                {
                    nResult = 1;
                    sResult = "IN TOLERANCE";
                    GlobalLogWriter.println("The differences are in tolerance.");

                }
            }
            if (nResult != 1)
            {
                sResult = "NO";
                try
                {
                    nPercentColorDiffer = estimateGfx(sSource, sDestination, sDifference);
                }
                catch (java.io.IOException e)
                {
                    GlobalLogWriter.println("Can't estimate the different colors. " + e.getMessage());
                }
            }

            // store the result in a result.ini file
            String sResultFile = FileHelper.appendPath(sDestinationPath, aNameDPIPage.Name + ".ini");
            int nPage = aNameDPIPage.Page;
            if (nPage < 0)
            {
                nPage = 0;
            }
            IniFile aResultIni = new IniFile(sResultFile);

            String[] aComment =
            {
                "; This file is automatically created by a graphical.JPEGComparator run",
                "; ",
                "; If you see this file in a browser you may have forgotten to set the follows in the property file",
                "; " + PropertyName.DOC_COMPARATOR_HTML_OUTPUT_PREFIX + "=http://<computer>/gfxcmp_ui/cw.php?inifile=",
                "; Please check the documentation if you got confused.",
                "; ",
                "; "
            };
            aResultIni.insertFirstComment(aComment);

            // write down the global flags
            int nMaxPage = Math.max(nPage, aResultIni.getIntValue("global", "pages", 0));
            aResultIni.insertValue("global", "pages", nMaxPage);

            // INIoutput.writeValue("buildid", _sBuildID);
            // INIoutput.writeValue("refbuildid", _sRefBuildID);
            String sRefBuildId = (String) _aParams.getTestParameters().get("RefBuildId");
            if (sRefBuildId == null)
            {
                sRefBuildId = "";
            }
            aResultIni.insertValue("global", "refbuildid", sRefBuildId);

            aResultIni.insertValue("global", "diffdiff", "no");
            aResultIni.insertValue("global", "basename", aNameDPIPage.Name);
            aResultIni.insertValue("global", "dpi", aNameDPIPage.DPI);

            // write down flags for each page
            String sSection = "page" + String.valueOf(nPage);

            aResultIni.insertValue(sSection, "oldgfx", sSource);
            aResultIni.insertValue(sSection, "newgfx", sDestination);
            aResultIni.insertValue(sSection, "diffgfx", sDifference);
            aResultIni.insertValue(sSection, "percent", nPercentColorDiffer);
            aResultIni.insertValue(sSection, "BM", "false");
            aResultIni.insertValue(sSection, "result", sResult);

            aResultIni.close();
        }
    }

//    // This creates a status for exact on document
//    static boolean createINIStatus(StatusHelper[] aList, String _sFilenamePrefix, String _sOutputPath, String _sAbsoluteInputFile, String _sBuildID, String _sRefBuildID)
//        {
//            // Status
//            String fs = System.getProperty("file.separator");
//            String sBasename = FileHelper.getBasename(_sAbsoluteInputFile);
//            String sNameNoSuffix = FileHelper.getNameNoSuffix(sBasename);
////            String sHTMLFile = _sFilenamePrefix + sNameNoSuffix + ".html";
////            HTMLOutputter HTMLoutput = HTMLOutputter.create(_sOutputPath, sHTMLFile, "", "");
////            HTMLoutput.header(sNameNoSuffix);
////  TODO: version info was fine
////            HTMLoutput.checkSection(sBasename);
//            // Status end
//
//            String sINIFile = _sFilenamePrefix + sNameNoSuffix + ".ini";
//            INIOutputter INIoutput = INIOutputter.create(_sOutputPath, sINIFile, "", "");
//            INIoutput.createHeader();
////  TODO: version info was fine
//
//            INIoutput.writeSection("global");
//            INIoutput.writeValue("pages", String.valueOf(aList.length));
//            INIoutput.writeValue("buildid", _sBuildID);
//            INIoutput.writeValue("refbuildid", _sRefBuildID);
//            INIoutput.writeValue("diffdiff", "no");
//            INIoutput.writeValue("basename", sBasename);
//
//            boolean bResultIsOk = true;          // result over all pages
//            for (int i=0;i<aList.length; i++)
//            {
//                INIoutput.writeSection("page" + String.valueOf(i + 1));   // list start at point 0, but this is page 1 and so on... current_page = (i + 1)
//                aList[i].printStatus();
//
//                boolean bCurrentResult = true;   // result over exact one page
//
//                int nCurrentDiffStatus = aList[i].nDiffStatus;
//
//                // check if the status is in a defined range
//                if (nCurrentDiffStatus == StatusHelper.DIFF_NO_DIFFERENCES)
//                {
//                    // ok.
//                }
//                else if (nCurrentDiffStatus == StatusHelper.DIFF_DIFFERENCES_FOUND && aList[i].nPercent < 5)
//                {
//                    // ok.
//                }
//                else if (nCurrentDiffStatus == StatusHelper.DIFF_AFTER_MOVE_DONE_NO_PROBLEMS)
//                {
//                    // ok.
//                }
//                else if (nCurrentDiffStatus == StatusHelper.DIFF_AFTER_MOVE_DONE_DIFFERENCES_FOUND && aList[i].nPercent2 < 5)
//                {
//                    // ok.
//                }
//                else
//                {
//                    // failed.
//                    bCurrentResult = false; // logic: nDiff==0 = true if there is no difference
//                }
//
//                // Status
////                HTMLoutput.checkLine(aList[i], bCurrentResult);
//                INIoutput.checkLine(aList[i], bCurrentResult);
//                bResultIsOk &= bCurrentResult;
//            }
//            // Status
////            HTMLoutput.close();
//            INIoutput.close();
//            return bResultIsOk;
//        }
    /**
     * count how much pixel differ and between Old or New and the Difference graphics
     *
     * First, count the old graphics, then the new graphics due to the fact both should be equal
     * it should be legal to take result from old or new. We take the graphics with less values.
     *
     * Second, count the difference graphics, now take the percent algorithm and
     * build a percent value, which contain the number of different pixels as a percent value
     *
     * Interpretation:
     * 0%    there is no difference
     *
     * &lt;100% Take a look into the difference graphics, maybe the difference graphics shows
     *       text like outlined or the text is little bit move left, right up or down.
     *
     * &gt;>100% Yes it's possible that there is a difference more then 100%, maybe a font problem
     *       between old and new graphics. The font of the new graphics is little bit bigger,
     *       so the pixel count between old graphics and new graphics is twice the more.
     *
     * @param _sOldGfx path & name to the jpeg file (1)
     * @param _sNewGfx path & name to the other jpeg file (2)
     * @param _sDiffGfx path & name to the new difference file which shows the difference between (1) and (2)
     * @return the count of different pixels
     * @throws java.io.IOException if file access is not possible
     */
    public static int estimateGfx(String _sOldGfx, String _sNewGfx, String _sDiffGfx)
            throws java.io.IOException
    {
        TimeHelper a = new TimeHelper();
        a.start();
        // Count Pixels
        final int nNotWhiteCount_OldGraphic = PixelCounter.countNotWhitePixelsFromImage(_sOldGfx);
        final int nNotWhiteCount_NewGraphic = PixelCounter.countNotWhitePixelsFromImage(_sNewGfx);
        final int nNotBlackCount_DiffGraphic = PixelCounter.countNotBlackPixelsFromImage(_sDiffGfx);

        // Count Pixels in different threads
//        CountNotWhitePixelsFromImage t1 = new CountNotWhitePixelsFromImage(_sOldGfx);
//        CountNotWhitePixelsFromImage t2 = new CountNotWhitePixelsFromImage(_sNewGfx);
//        CountNotBlackPixelsFromImage t3 = new CountNotBlackPixelsFromImage(_sDiffGfx);
//        t1.start();
//        t2.start();
//        t3.start();
//        try
//        {
//            t1.join();
//        }
//        catch (InterruptedException ex)
//        {
//            GlobalLogWriter.get().println("Thread 1 failed: " + ex.getMessage());
//        }
//        try
//        {
//            t2.join();
//        }
//        catch (InterruptedException ex)
//        {
//            GlobalLogWriter.get().println("Thread 2 failed: " + ex.getMessage());
//        }
//        try
//        {
//            t3.join();
//        }
//        catch (InterruptedException ex)
//        {
//            GlobalLogWriter.get().println("Thread 3 failed: " + ex.getMessage());
//        }
//        final int nNotWhiteCount_OldGraphic = t1.getValue();
//        final int nNotWhiteCount_NewGraphic = t2.getValue();
//        final int nNotBlackCount_DiffGraphic = t3.getValue();

        a.stop();
        GlobalLogWriter.println("Thread Time is: " + a.getTime());

        int nMinNotWhiteCount = Math.min(nNotWhiteCount_NewGraphic, nNotWhiteCount_OldGraphic);

        // check if not zero
        if (nMinNotWhiteCount == 0)
        {
            nMinNotWhiteCount = Math.max(nNotWhiteCount_NewGraphic, nNotWhiteCount_OldGraphic);
            if (nMinNotWhiteCount == 0)
            {
                nMinNotWhiteCount = 1;
            }
        }

        int nPercent = Math.abs(nNotBlackCount_DiffGraphic * 100 / nMinNotWhiteCount);
        GlobalLogWriter.println("Graphics check, pixel based:" + String.valueOf(nPercent) + "% pixel differ ");
        return nPercent;
    }

    private static int compareJPEG(String _sOldGfx, String _sNewGfx, String _sDiffGfx)
    {
        String sComposite = "composite";
        if (OSHelper.isWindows())
        {
            sComposite = "composite.exe";
            String sIMPath = (String) param.get("imagemagick.path");
            if (sIMPath != null)
            {
                sComposite = FileHelper.appendPath(sIMPath, sComposite);
            }
        }

        // String sCommand = sComposite + " -compose difference " +
        //     StringHelper.doubleQuoteIfNeed(_sOldGfx) + " " +
        //     StringHelper.doubleQuoteIfNeed(_sNewGfx) + " " +
        //     StringHelper.doubleQuoteIfNeed(_sDiffGfx);

        String[] sCommandArray =
        {
            sComposite,
            "-compose",
            "difference",
            _sOldGfx,
            _sNewGfx,
            _sDiffGfx
        };

        ProcessHandler aHandler = new ProcessHandler(sCommandArray);
        aHandler.executeSynchronously();
        int nExitCode = aHandler.getExitCode();
        if (nExitCode != 0)
        {
            GlobalLogWriter.println("'" + sComposite + "' return with ");
            String sBack = aHandler.getOutputText();
            GlobalLogWriter.println("'" + sBack + "'");
        }
        else
        {
            // creates an extra smaller difference picture
            File aDiffFile = new File(_sDiffGfx);
            if (aDiffFile.exists())
            {
                JPEGCreator.convertToNearSameFileWithWidth340(_sDiffGfx);
            }
        }
        return nExitCode;
    }

    /**
     * wrapper for ImageMagick identify,
     * function checks how many different colors a picture contains.
     * if it's only one color (nResult==1), like background color, there is no difference.
     */
    int identify(String _sDiffGfx)
    {
        int nResult = 0;
        // would like to know what the meaning of %k is for ImageMagick's 'identify'
        String sIM_Format = "%k";
        // if (OSHelper.isWindows())
        // {
        //     sIM_Format = "%%k";
        // }

        String sIdentify = "identify";
        if (OSHelper.isWindows())
        {
            sIdentify = "identify.exe";
            String sIMPath = (String) param.get("imagemagick.path");
            if (sIMPath != null)
            {
                sIdentify = FileHelper.appendPath(sIMPath, sIdentify);
            }
        }

        // String sCommand = sIdentify + " " + sIM_Format + " " + StringHelper.doubleQuoteIfNeed(_sDiffGfx);

        String[] sCommandArray =
        {
            sIdentify,
            "-format",
            sIM_Format,
            _sDiffGfx
        };
        ProcessHandler aHandler = new ProcessHandler(sCommandArray);
        aHandler.executeSynchronously();
        aHandler.getExitCode();

        String sBack = aHandler.getOutputText();
        GlobalLogWriter.println("'" + sBack + "'");

        // try to interpret the result, which we get as a String
        try
        {
            int nIdx = sBack.indexOf("\n");
            if (nIdx > 0)
            {
                sBack = sBack.substring(0, nIdx);
            }

            nResult = Integer.valueOf(sBack).intValue();
        }
        catch (java.lang.NumberFormatException e)
        {
            GlobalLogWriter.println("identify(): Number format exception");
            nResult = 0;
        }
        return nResult;
    }
//    public static void main(String [] _args)
//    {
//// give an index.ini file, ok
//// give a directory, where exist jpeg files ok
//// inputpath (given file) doesn't exists
//// give a jpeg file.
//
//        String args[] = {
//            "-TimeOut", "3600000",
//            "-tb", "java_complex",
//            "-o", "graphical.JPEGComparator",
//            "-DOC_COMPARATOR_INPUT_PATH", "C:\\CWS\\temp\\output\\index.ini",
//            "-DOC_COMPARATOR_OUTPUT_PATH", "C:\\CWS\\temp\\output2",
////            "-DOC_COMPARATOR_INPUT_PATH", "C:\\CWS\\temp\\output\\GroupReport.odt.pdf_180DPI_0001.jpg",
////            "-DOC_COMPARATOR_OUTPUT_PATH", "C:\\CWS\\temp\\output2\\Report1.odt.pdf_180DPI_0001.jpg",
//            "-DOC_COMPARATOR_HTML_OUTPUT_PREFIX", "http://so-gfxcmp-lin.germany.sun.com/gfxcmp_ui/cw.php?inifile=",
////            "-DOC_COMPARATOR_REFERENCE_CREATOR_TYPE", "PDF",      /* default: "OOo" */
////            "-DOC_COMPARATOR_REFERENCE_CREATOR_TYPE", "msoffice", /* default: "OOo" */
////            "-OFFICE_VIEWABLE", "false",
////            "-AppExecutionCommand", "\"C:/Programme/sun/staroffice 9/program/soffice.exe\"  --norestore --nocrashreport --accept=pipe,name=ll93751;urp;",
//            "-NoOffice"
//        };
//
//        org.openoffice.Runner.main(args);
//    }
}
