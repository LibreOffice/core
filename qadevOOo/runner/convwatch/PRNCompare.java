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


import helper.ProcessHandler;
import java.io.File;
import convwatch.PixelCounter;
import convwatch.StatusHelper;
import helper.OSHelper;
import helper.StringHelper;
import java.util.ArrayList;


// --------------------------------- PRNCompare ---------------------------------

public class PRNCompare
{
    String fs;

    public PRNCompare()
        {
            fs = System.getProperty("file.separator");
        }

    String executeSynchronously(String _sCommand)
        {
            ProcessHandler aHandler = new ProcessHandler(_sCommand);
            aHandler.executeSynchronously();

            String sText = aHandler.getOutputText();

            GlobalLogWriter.get().println("Exitcode: " + String.valueOf(aHandler.getExitCode()));
            return sText;
        }


    int getMaxNumOfFileEntry(String _sDirectory, String _sBasename)
        {
//  TODO: check if directory exist should be done earlier
            File aDirectory = new File(_sDirectory);
            File[] aDirList = aDirectory.listFiles(
                );

            int nMaxNumber = 0;
            for(int n = 0; n<aDirList.length ; n++)
            {
                String sDirEntry = aDirList[n].getName();
                if (sDirEntry.endsWith(".jpg"))
                {
                    int nNum = nMaxNumber;

                    if (sDirEntry.startsWith(_sBasename))
                    {
                        int nJpgIdx = sDirEntry.lastIndexOf(".jpg");
                        String sValue = sDirEntry.substring(_sBasename.length(), nJpgIdx);
                        int nValue = 0;
                        try
                        {
                            nValue = Integer.valueOf(sValue).intValue();
                        }
                        catch(java.lang.NumberFormatException e)
                        {
                        }

                        nNum = nValue;
                    }

                    if (nNum > nMaxNumber)
                    {
                        nMaxNumber = nNum;
                    }
                }
            }
            return nMaxNumber;
        }

    String m_sInputPath;
    String m_sReferencePath;
    String m_sOutputPath;
    String m_sDocFile;
    String m_sReferenceFile;
    String m_sPostScriptFile;
    int m_nMaxPages = 0;
    int m_nResolutionInDPI = 0;
    TriState m_tUseBorderMove;
    String m_sDocumentType;

    public void setInputPath(String _sInputPath) { m_sInputPath = _sInputPath; }

    public void setReferencePath(String _sReferencePath) { m_sReferencePath = _sReferencePath; }

    public void setOutputPath(String _sOutPath) { m_sOutputPath = _sOutPath; }

    public void setDocFile(String _sDocFile) { m_sDocFile = _sDocFile;}

    public void setReferenceFile(String _sPRNFile){ m_sReferenceFile = _sPRNFile;}

    public void setPostScriptFile(String _sPSFile){ m_sPostScriptFile = _sPSFile;}

    public void setBorderMove(TriState _b) {m_tUseBorderMove = _b;}
    public TriState getBorderMove() {return m_tUseBorderMove;}
    public void setMaxPages(int _n) {m_nMaxPages = _n;}
    int getMaxPages() {return m_nMaxPages;}

    public void setResolutionInDPI(int _n) {m_nResolutionInDPI = _n;}
    int getResolutionInDPI() {return m_nResolutionInDPI;}

    public void setDocumentType(String _sTypeName)
        {
            m_sDocumentType = _sTypeName;
        }



    public StatusHelper[] compare()
        {
            createJPEGFromPostscript(m_sOutputPath, m_sReferencePath, m_sReferenceFile, getResolutionInDPI());
//  TODO: Assume, that Postscript is already in the OutputPath, this may change.
            createJPEGFromPostscript(m_sOutputPath, m_sOutputPath, m_sPostScriptFile, getResolutionInDPI());
            StatusHelper[] aList = createDiffs(m_sOutputPath,
                                               m_sOutputPath, m_sReferenceFile,
                                               m_sOutputPath, m_sPostScriptFile,
                                               getMaxPages(), m_tUseBorderMove);

//  TODO: Rename?

            return aList;
        }

    public StatusHelper[] compare_new()
        {
            String[] aRefList = createJPEGFromPostscript(m_sOutputPath, m_sReferencePath, m_sReferenceFile, getResolutionInDPI());
//  TODO: Assume, that Postscript is already in the OutputPath, this may change.
            String[] aPSList = createJPEGFromPostscript(m_sOutputPath, m_sOutputPath, m_sPostScriptFile, getResolutionInDPI());
            StatusHelper[] aList = createDiffs(m_sOutputPath,
                                               aRefList,
                                               aPSList,
                                               getMaxPages(), m_tUseBorderMove);

            return aList;
        }

    static String getJPEGName(String _sOutputPath, String _sBasename, String _sGS_PageOutput)
        {
            String fs = System.getProperty("file.separator");

            String sJPEGName = _sOutputPath + fs + _sBasename + _sGS_PageOutput + ".jpg";
            return sJPEGName;
        }

    /**
     * Create via ghostscript (gs) from the reference file for every page a JPEG file
     *
     * MUST set:
     * m_sOutputPath, m_sReferenceFile, m_sReferencePath
     *
     * return exit code from gs command
     */
    public static String[] createJPEGFromPostscript(String _sOutputPath, String _sSourcePath, String _sSourceFile, int _nResolutionInDPI)
        {
            String sGS_PageOutput = "%04d";

            FileHelper.makeDirectories("", _sOutputPath);

            // create a jpeg from original prn
            String fs = System.getProperty("file.separator");

            String sJPGFilename = getJPEGName(_sOutputPath, _sSourceFile, sGS_PageOutput);
            String sOriginalFile = _sSourcePath + fs + _sSourceFile;
            String sGS = "gs";
            if (OSHelper.isWindows())
            {
                sGS = "gswin32c.exe";
            }

            String[] sCommandArray =
                {
                    sGS,
                    "-dNOPROMPT",
                    "-dBATCH",
                    "-sDEVICE=jpeg",
                    "-r" + String.valueOf(_nResolutionInDPI),
                    "-dNOPAUSE",
                    "-sOutputFile=" + sJPGFilename,
                    sOriginalFile
                };

            ProcessHandler aHandler = new ProcessHandler(sCommandArray);
            aHandler.executeSynchronously();

            // TODO: return a real filename, due to the fact we don't know how much files are created, maybe better to return a list

            ArrayList<String> m_aFileList = new ArrayList<String>();
            for (int i=1;i<9999;i++)
            {
                String sNewJPEGFilename = sJPGFilename.replace(sGS_PageOutput, StringHelper.createValueString(i, 4));
                if (FileHelper.exists(sNewJPEGFilename))
                {
                    m_aFileList.add(sNewJPEGFilename); // as long as the files exist, fill the array
                }
                else
                {
                    break;                             // stop file check
                }
            }
            String[] aList = new String[m_aFileList.size()];
            aList = m_aFileList.toArray(aList);
            return aList; // sNewJPEGFilename;
        }

    /**
     * Create Difference Files from the JPEGs
     * parameter, how much difference files should create
     *
     * m_sPostScriptFile
     * m_sReferenceFile
     * m_sOutputPath
     */
    public StatusHelper[] createDiffs(String _sOutputPath, String _sSourcePath1, String _sSourceFile1, String _sSourcePath2, String _sSourceFile2, int _nMaxDiffs, TriState _tUseBorderMove)
        {
            if (_nMaxDiffs < 1)
            {
                _nMaxDiffs = 1;
            }

            String sS1Basename = FileHelper.getBasename(_sSourceFile1);
            String sS2Basename = FileHelper.getBasename(_sSourceFile2);

            // count, from which file (jpegs) exist more, take the less one
            // more are not compareable
            int nS1_Files = getMaxNumOfFileEntry(_sSourcePath1, sS1Basename);
            int nS2_Files = getMaxNumOfFileEntry(_sSourcePath2, sS2Basename);

            // take the min of both
            int nMin = Math.min(nS1_Files, nS2_Files);
            nMin = Math.min(nMin, _nMaxDiffs);

            StatusHelper[] aList = new StatusHelper[nMin];

//  TODO: if both document do not have same page count, produce an error

            int nStatusIndex = 0;
            for (int i=1;i<=nMin;i++)
            {
                String sOldGfx =  getJPEGName(_sSourcePath1, sS1Basename, StringHelper.createValueString(i, 4));
                String sNewGfx =  getJPEGName(_sSourcePath2, sS2Basename, StringHelper.createValueString(i, 4));
                String sDiffGfx_ = getJPEGName(_sOutputPath, sS1Basename + ".diff", StringHelper.createValueString(i, 4));


                String sDiffGfx = compareJPEGs(sOldGfx, sNewGfx, sDiffGfx_);
                StatusHelper aStatus = new StatusHelper(sOldGfx, sNewGfx, sDiffGfx);

                if (sDiffGfx.length() > 0)
                {
                    int nResult = identify(sDiffGfx);
                    if (nResult == 1)
                    {
                        aStatus.nDiffStatus = StatusHelper.DIFF_NO_DIFFERENCES;
                    }
                    else
                    {
                        try
                        {
                            int nPercent = estimateGfx(sOldGfx, sNewGfx, sDiffGfx);
                            aStatus.nDiffStatus = StatusHelper.DIFF_DIFFERENCES_FOUND;
                            aStatus.nPercent = nPercent;

// TODO: insert here the new BorderRemover if the percentage value is creater than 75%
                            if (nPercent > 75 &&
                                ((_tUseBorderMove == TriState.TRUE ) ||
                                 ((_tUseBorderMove == TriState.UNSET) &&
                                  m_sDocumentType.indexOf("MS PowerPoint") > 0)))
                            {
                                setBorderMove(TriState.TRUE);

                                String sOld_BM_Gfx =  getJPEGName(_sSourcePath1, sS1Basename + ".BM", StringHelper.createValueString(i, 4));
                                String sNew_BM_Gfx =  getJPEGName(_sSourcePath2, sS2Basename + ".BM", StringHelper.createValueString(i, 4));
                                String sDiff_BM_Gfx_ = getJPEGName(_sOutputPath, sS1Basename + ".diff.BM", StringHelper.createValueString(i, 4));
                                aStatus.setFilesForBorderMove(sOld_BM_Gfx, sNew_BM_Gfx, sDiff_BM_Gfx_);
                                try
                                {
                                    BorderRemover a = new BorderRemover();
                                    a.createNewImageWithoutBorder(sOldGfx, sOld_BM_Gfx);
                                    a.createNewImageWithoutBorder(sNewGfx, sNew_BM_Gfx);

                                    String sDiff_BM_Gfx = compareJPEGs( sOld_BM_Gfx, sNew_BM_Gfx, sDiff_BM_Gfx_);

                                    if (sDiff_BM_Gfx.length() > 0)
                                    {
                                        nResult = identify(sDiff_BM_Gfx);
                                        if (nResult == 1)
                                        {
                                            aStatus.nDiffStatus = StatusHelper.DIFF_AFTER_MOVE_DONE_NO_PROBLEMS;
                                            aStatus.nPercent2 = 0;
                                        }
                                        else
                                        {
                                            nPercent = estimateGfx(sOld_BM_Gfx, sNew_BM_Gfx, sDiff_BM_Gfx);
                                            aStatus.nDiffStatus = StatusHelper.DIFF_AFTER_MOVE_DONE_DIFFERENCES_FOUND;
                                            aStatus.nPercent2 = nPercent;
                                        }
                                    }
                                    else
                                    {
                                    }
                                }
                                catch(java.io.IOException e)
                                {
                                    GlobalLogWriter.get().println("Exception caught. At border remove: " + e.getMessage());
                                }
                            }


                        }
                        catch (java.io.IOException e)
                        {
                            GlobalLogWriter.get().println(e.getMessage());
                        }
                    }

                }
                aList[nStatusIndex ++] = aStatus;
            }
            return aList;
        }


    public StatusHelper[] createDiffs(String _sOutputPath, String[] _aRefList, String[] _aPSList, int _nMaxDiffs, TriState _tUseBorderMove)
        {
            if (_nMaxDiffs < 1)
            {
                _nMaxDiffs = 1;
            }

            // count, from which file (jpegs) exist more, take the less one
            // more are not compareable

            // take the min of both
            int nMin = Math.min(_aRefList.length, _aPSList.length);
            nMin = Math.min(nMin, _nMaxDiffs);

            StatusHelper[] aList = new StatusHelper[nMin];

//  TODO: if both document do not have same page count, produce an error

            int nStatusIndex = 0;
            for (int i=1;i<=nMin;i++)
            {
                String sOldGfx =  _aRefList[i];
                String sNewGfx =  _aPSList[i];


                String sDiffGfx = compareJPEGs(sOldGfx, sNewGfx );
                StatusHelper aStatus = new StatusHelper(sOldGfx, sNewGfx, sDiffGfx);

                if (sDiffGfx.length() > 0)
                {
                    int nResult = identify(sDiffGfx);
                    if (nResult == 1)
                    {
                        aStatus.nDiffStatus = StatusHelper.DIFF_NO_DIFFERENCES;
                    }
                    else
                    {
                        try
                        {
                            int nPercent = estimateGfx(sOldGfx, sNewGfx, sDiffGfx);

                            aStatus.nDiffStatus = StatusHelper.DIFF_DIFFERENCES_FOUND;
                            aStatus.nPercent = nPercent;

                            if (nPercent > 75 &&
                                ((_tUseBorderMove == TriState.TRUE ) ||
                                 ((_tUseBorderMove == TriState.UNSET) &&
                                  m_sDocumentType.indexOf("MS PowerPoint") > 0)))
                            {
                                _tUseBorderMove = TriState.TRUE;
//  TODO: problem is here, that we have to create some new names.

                                String sBasename1 = FileHelper.getBasename(sOldGfx);
                                String sNameNoSuffix1 = FileHelper.getNameNoSuffix(sBasename1);
                                String sBasename2 = FileHelper.getBasename(sNewGfx);
                                String sNameNoSuffix2 = FileHelper.getNameNoSuffix(sBasename2);

                                String sTmpDir = util.utils.getUsersTempDir();
                                String fs = System.getProperty("file.separator");

                                String sOld_BM_Gfx =  sTmpDir + fs + sNameNoSuffix1 + "-BM-" + StringHelper.createValueString(i, 4) + ".jpg";
                                String sNew_BM_Gfx =  sTmpDir + fs + sNameNoSuffix2 + "-BM-" + StringHelper.createValueString(i, 4) + ".jpg";
                                try
                                {
                                    BorderRemover a = new BorderRemover();
                                    a.createNewImageWithoutBorder(sOldGfx, sOld_BM_Gfx);
                                    a.createNewImageWithoutBorder(sNewGfx, sNew_BM_Gfx);

                                    String sDiff_BM_Gfx = compareJPEGs( sOld_BM_Gfx, sNew_BM_Gfx );

                                    aStatus.setFilesForBorderMove(sOld_BM_Gfx, sNew_BM_Gfx, sDiff_BM_Gfx);

                                    if (sDiff_BM_Gfx.length() > 0)
                                    {
                                        nResult = identify(sDiff_BM_Gfx);
                                        if (nResult == 1)
                                        {
                                            aStatus.nDiffStatus = StatusHelper.DIFF_AFTER_MOVE_DONE_NO_PROBLEMS;
                                            aStatus.nPercent2 = 0;
                                        }
                                        else
                                        {
                                            nPercent = estimateGfx(sOld_BM_Gfx, sNew_BM_Gfx, sDiff_BM_Gfx);
                                            aStatus.nDiffStatus = StatusHelper.DIFF_AFTER_MOVE_DONE_DIFFERENCES_FOUND;
                                            aStatus.nPercent2 = nPercent;
                                        }
                                    }
                                    else
                                    {
                                    }
                                }
                                catch(java.io.IOException e)
                                {
                                    GlobalLogWriter.get().println("Exception caught. At border remove: " + e.getMessage());
                                }
                            }
                        }
                        catch (java.io.IOException e)
                        {
                            GlobalLogWriter.get().println(e.getMessage());
                        }
                    }

                }
                aList[nStatusIndex ++] = aStatus;
            }
            return aList;
        }

    public static String compareJPEGs(String _sOldGfx, String _sNewGfx)
        {
            String sBasename1 = FileHelper.getBasename(_sOldGfx);
            String sNameNoSuffix1 = FileHelper.getNameNoSuffix(sBasename1);
            String sBasename2 = FileHelper.getBasename(_sNewGfx);
            String sNameNoSuffix2 = FileHelper.getNameNoSuffix(sBasename2);

            String sTmpDir = util.utils.getUsersTempDir();
            String fs = System.getProperty("file.separator");

            String sDiffName = sTmpDir + fs + sNameNoSuffix1 + "-" + sNameNoSuffix2 + "-diff.jpg";

            return compareJPEGs(_sOldGfx, _sNewGfx, sDiffName);
        }

    public static String compareJPEGs(String _sOldGfx, String _sNewGfx, String _sDiffGfx)
        {
            String sComposite = "composite";
            if (OSHelper.isWindows())
            {
                sComposite = "composite.exe";
            }

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

            String sBack = aHandler.getOutputText();
            GlobalLogWriter.get().println("'" + sBack + "'");

            if (FileHelper.exists(_sDiffGfx))
            {
                return _sDiffGfx;
            }
            return "";
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

            String sIdentify = "identify";
            if (OSHelper.isWindows())
            {
                sIdentify = "identify.exe";
            }

            String[] sCommandArray =
                {
                    sIdentify,
                    "-format",
                    sIM_Format,
                    _sDiffGfx
                };
            ProcessHandler aHandler = new ProcessHandler(sCommandArray);
            aHandler.executeSynchronously();
            String sBack = aHandler.getOutputText();
            GlobalLogWriter.get().println("'" + sBack + "'");

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
            catch(java.lang.NumberFormatException e)
            {
                GlobalLogWriter.get().println("Number format exception");
                nResult = 0;
            }
            return nResult;
        }

    /*
     * Check 2 different differ files
     * return 1 if there is no difference between both diff files.
     */

//  TODO: Maybe a StatusHelper is a better return value
    public StatusHelper checkDiffDiff(String _sOutputPath, String _sSourcePath1, String _sSourceFile1, String _sSourcePath2, String _sSourceFile2)
        {
            String sNewGfx =  _sSourcePath1 + fs + _sSourceFile1;
            String sOldGfx =  _sSourcePath2 + fs + _sSourceFile2;

            int nNumber = 1;
            String sDiffGfx;
            sDiffGfx = getJPEGName(_sOutputPath, _sSourceFile1 + ".diff", StringHelper.createValueString(nNumber, 4));

            StatusHelper aCurrentStatus = new StatusHelper(sOldGfx, sNewGfx, sDiffGfx);

            compareJPEGs(sOldGfx, sNewGfx, sDiffGfx);

            if (FileHelper.exists(sDiffGfx))
            {
                int nResult = identify(sDiffGfx);
                if (nResult == 1)
                {
                    aCurrentStatus.nDiffStatus = StatusHelper.DIFF_NO_DIFFERENCES;
                }
                else
                {
                    aCurrentStatus.nDiffStatus = StatusHelper.DIFF_DIFFERENCES_FOUND;
                    try
                    {
                        aCurrentStatus.nPercent = estimateGfx(sOldGfx, sNewGfx, sDiffGfx);
                    }
                    catch (java.io.IOException e)
                    {
                        GlobalLogWriter.get().println(e.getMessage());
                        aCurrentStatus.nPercent = -1;
                    }
                }
            }
            else
            {
                GlobalLogWriter.get().println("composite can't create the diffdiff file.");
            }

            return aCurrentStatus;
        }


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
     * <100% Take a look into the difference graphics, maybe the difference graphics shows
     *       text like outlined or the text is little bit move left, right up or down.
     *
     * >100% Yes it's possible that there is a difference more than 100%, maybe a font problem
     *       between old and new graphics. The font of the new graphics is little bit bigger,
     *       so the pixel count between old graphics and new graphics is twice the more.
     *
     */
    public int estimateGfx(String _sOldGfx, String _sNewGfx, String _sDiffGfx)
        throws java.io.IOException
        {
            // new count pixels
            int nNotWhiteCount_OldGraphic = PixelCounter.countNotWhitePixelsFromImage(_sOldGfx);
            int nNotWhiteCount_NewGraphic = PixelCounter.countNotWhitePixelsFromImage(_sNewGfx);
            int nNotBlackCount_DiffGraphic = PixelCounter.countNotBlackPixelsFromImage(_sDiffGfx);

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
            GlobalLogWriter.get().println( "Graphics check, pixel based:" + String.valueOf(nPercent) + "% pixel differ ");
            return nPercent;
        }

}
