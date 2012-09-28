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

import complexlib.ComplexTestCase;
import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;

abstract public class EnhancedComplexTestCase extends ComplexTestCase implements IDocument
{


private void callEntry(String _sEntry, ParameterHelper _aParam)
{
    log.println("      File: " + _sEntry);
    // TODO: check if 'sEntry' is a guilty document.
    File aFile = new File(_aParam.getInputPath());
    String sPath = _aParam.getInputPath();
    // problem here, isFile() checks also if the file exists, but a not existing file is not really a directory
    // therefore we check if the given file a path (isDirectory()) if not it must be a file
    if (aFile.isDirectory())
    {
    }
    else
    {
        // special case, if a file is given in inputpath
        sPath = FileHelper.getPath(_aParam.getInputPath());
    }
    String sNewSubDir = FileHelper.removeFirstDirectorysAndBasenameFrom(_sEntry, sPath);

    String sNewOutputPath = _aParam.getOutputPath();

    // if there exist a subdirectory, add it to all result path
    if (sNewSubDir.length() > 0)
    {

        sNewOutputPath = FileHelper.appendPath(sNewOutputPath, sNewSubDir);
    }
    log.println("Outputpath: " + sNewOutputPath);


    // call interface with parameters
    try
    {
        checkOneFile(_sEntry, sNewOutputPath, _aParam);
    }
    catch (OfficeException e)
    {
        // TODO: unhandled yet.
        GlobalLogWriter.println("Warning: caught OfficeException " + e.getMessage());
        assure("Exception caught: " + e.getMessage(), false);
    }

}

/**
 * Run through all documents found in Inputpath.
 * Call the IDocument interface function call(...);
 * @param _aParam
 */
    public void foreachDocumentinInputPath(ParameterHelper _aParam)
    {
                // TODO: auslagern in eine function, die ein Interface annimmt.
        File aInputPath = new File(_aParam.getInputPath());
        if (aInputPath.isDirectory())
        {
            // check a whole directory
            // a whole directory
            FileFilter aFileFilter = FileHelper.getFileFilter();
            traverseDirectory(aFileFilter, _aParam);
        }
        else
        {
            callEntry(_aParam.getInputPath(), _aParam);
        }
    }

    private void traverseDirectory(FileFilter _aFileFilter, ParameterHelper _aParam)
    {
        Object[] aList = DirectoryHelper.traverse(_aParam.getInputPath(), _aFileFilter, _aParam.isIncludeSubDirectories());
        if (aList.length == 0)
        {
            log.println("Nothing to do, there are no document files found.");
        }
        else
        {
            for (int i=0;i<aList.length;i++)
            {
                String sEntry = (String)aList[i];
                callEntry(sEntry, _aParam);
            }
        }
    }
/**
 * Run through a given index.ini or run through a given directory,
 * find all postscript or pdf files.
 * Call the IDocument interface function call(...);
 * @param _aParam
 */
    public void foreachPSorPDFinInputPath(ParameterHelper _aParam)
    {
        // TODO: auslagern in eine function, die ein Interface annimmt.
        String sInputPath = _aParam.getInputPath();
        File aInputPath = new File(sInputPath);
        if (aInputPath.isDirectory())
        {
            // check a whole directory
            // a whole directory
            FileFilter aFileFilter = FileHelper.getFileFilterPSorPDF();
            traverseDirectory(aFileFilter, _aParam);
        }
        else
        {
            // the inputpath contains a file
            if (sInputPath.toLowerCase().endsWith(".ini"))
            {
                IniFile aIniFile = new IniFile(_aParam.getInputPath());
                while (aIniFile.hasMoreElements())
                {
                    String sKey = aIniFile.nextElement();
                    String sPath = FileHelper.getPath(_aParam.getInputPath());
                    String sEntry = FileHelper.appendPath(sPath, sKey);
                    File aFile = new File(sEntry);
                    assure("File '" + sEntry + "' doesn't exists.", aFile.exists(), true);
                    if (aFile.exists())
                    {
                        callEntry(sEntry, _aParam);
                    }
                }
            }
            else
            {
                // call for a single pdf/ps file
                if (sInputPath.toLowerCase().endsWith(".ps") ||
                    sInputPath.toLowerCase().endsWith(".pdf") ||
                    sInputPath.toLowerCase().endsWith(".prn"))
                {
                    callEntry(sInputPath, _aParam);
                }
                else
                {
                    String sInputPathWithPDF = sInputPath + ".pdf";
                    File aInputPathWithPDF = new File(sInputPathWithPDF);

                    if (aInputPathWithPDF.exists() &&
                        _aParam.getReferenceType().toLowerCase().equals("pdf"))
                    {
                        // create PDF only if a pdf file exists and creatortype is set to PDF
                        callEntry(sInputPathWithPDF, _aParam);
                    }
                    else
                    {
                        String sInputPathWithPS = sInputPath + ".ps";

                        File aInputPathWithPS = new File(sInputPathWithPS);
                        if (aInputPathWithPS.exists())
                        {
                            callEntry(sInputPathWithPS, _aParam);
                        }
                        else
                        {
                            String sPath = FileHelper.getPath(sInputPath);
                            String sBasename = FileHelper.getBasename(sInputPath);

                            // there exist an index file, therefore we assume the given
                            // file is already converted to postscript or pdf
                            runThroughEveryReportInIndex(sPath, sBasename, _aParam);
                        }
                    }
                }
            }
        }
    }

    private void runThroughEveryReportInIndex(String _sPath, String _sBasename, ParameterHelper _aParam)
    {
        String sIndexFile = FileHelper.appendPath(_sPath, "index.ini");
        File aIndexFile = new File(sIndexFile);
        if (aIndexFile.exists())
        {
            IniFile aIniFile = new IniFile(sIndexFile);

            if (aIniFile.hasSection(_sBasename))
            {
                // special case for odb files
                int nFileCount = aIniFile.getIntValue(_sBasename, "reportcount", 0);
                ArrayList<String> aList = new ArrayList<String>();
                for (int i=0;i<nFileCount;i++)
                {
                    String sValue = aIniFile.getValue(_sBasename, "report" + i);

                    String sPSorPDFName = getPSorPDFNameFromIniFile(aIniFile, sValue);
                    if (sPSorPDFName.length() > 0)
                    {
                        String sEntry = FileHelper.appendPath(_sPath, sPSorPDFName);
                        aList.add(sEntry);
                    }
                }
                aIniFile.close();

                int nOkStatus = 0;
                String sStatusRunThrough = "";
                String sStatusInfo = "";
                // get the bad status and store it into the
                for (int i=0;i<aList.size();i++)
                {
                    String sEntry = aList.get(i);
                    try
                    {
                        callEntry(sEntry, _aParam);
                    }
                    catch (AssureException e)
                    {
                        // we only need to catch the assure()
                        // nOkStatus += 2;
                    }
                    // we want to know the current status of the run through
                    // if the status is greater (more bad) then the current,
                    // we will remember this. Only the very bad status will
                    // seen.
                    int nCurrentOkStatus = _aParam.getTestParameters().getInt("current_ok_status");
                    if (nCurrentOkStatus > nOkStatus)
                    {
                        sStatusRunThrough = (String)_aParam.getTestParameters().get("current_state");
                        sStatusInfo = (String)_aParam.getTestParameters().get("current_info");
                        nOkStatus = nCurrentOkStatus;
                    }
                }
                if (nOkStatus > 0)
                {
                    _aParam.getTestParameters().put("last_state", sStatusRunThrough);
                    _aParam.getTestParameters().put("last_info", sStatusInfo);
                }
            }
            else
            {
                // runThroughOneFileInIndex();
                String sPSorPDFName = getPSorPDFNameFromIniFile(aIniFile, _sBasename);

                aIniFile.close();

                if (sPSorPDFName.length() > 0)
                {
                    String sEntry = FileHelper.appendPath(_sPath, sPSorPDFName);
                    callEntry(sEntry, _aParam);
                }
            }
        }
        else
        {
             assure("File '" + sIndexFile + "' doesn't exists.", aIndexFile.exists(), true);
        }
    }

    private String getPSorPDFNameFromIniFile(IniFile _aIniFile, String _sName)
    {
        boolean bHasPostscriptOrPDF = false;
        String sPSBasename = _sName + ".ps";
        if (_aIniFile.hasSection(sPSBasename))       // checks for Postscript
        {
            bHasPostscriptOrPDF = true;
        }
        else
        {
            sPSBasename = _sName + ".pdf";       // checks for PDF
            if (_aIniFile.hasSection(sPSBasename))
            {
                bHasPostscriptOrPDF = true;
            }
        }
        if (bHasPostscriptOrPDF)
        {
            return sPSBasename;
        }
        return "";
    }

    public void runThroughOneFileInIndex(String _sPath, String _sBasename, ParameterHelper _aParam)
    {

    }
/**
 * Run through a given index.ini or run through a given directory,
 * find all postscript or pdf files.
 * Call the IDocument interface function call(...);
 * @param _aParam
 */
    public void foreachJPEGcompareWithJPEG(ParameterHelper _aParam)
    {
        // TODO: auslagern in eine function, die ein Interface annimmt.
        String sInputPath = _aParam.getInputPath();
        File aInputPath = new File(sInputPath);
        if (aInputPath.isDirectory())
        {
            // check a whole directory
            // a whole directory
            FileFilter aFileFilter = FileHelper.getFileFilterJPEG();
            traverseDirectory(aFileFilter, _aParam);
        }
        else
        {
            // the inputpath contains a file
            if (sInputPath.toLowerCase().endsWith(".ini"))
            {
                IniFile aIniFile = new IniFile(_aParam.getInputPath());
                while (aIniFile.hasMoreElements())
                {
                    aIniFile.nextElement();
                }
            }
            else
            {
                // call for a single jpeg file
                try
                {
                    String sOutputFilename = _aParam.getOutputPath();
                    if (sInputPath.toLowerCase().endsWith(".jpg") ||
                        sInputPath.toLowerCase().endsWith(".jpeg")    )
                    {
                        checkOneFile(sInputPath, sOutputFilename, _aParam);
                    }
                    else
                    {
                        // check if there exists a ini file
                        String sPath = FileHelper.getPath(sInputPath);
                        String sBasename = FileHelper.getBasename(sInputPath);

                        runThroughEveryReportInIndex(sPath, sBasename, _aParam);

                        String sStatusRunThrough = (String)_aParam.getTestParameters().get("last_state");
                        String sStatusInfo = (String)_aParam.getTestParameters().get("last_info");

                        if (sStatusRunThrough != null &&
                            sStatusInfo != null )
                        {
                            // store the bad status in the <Name>.odb.ps.ini file
                            String sOutputPath = _aParam.getOutputPath();
                            String sBasenameIni = FileHelper.appendPath(sOutputPath, sBasename + ".ps.ini");
                            IniFile aBasenameIni = new IniFile(sBasenameIni);
                            aBasenameIni.insertValue("global", "state", sStatusRunThrough);
                            aBasenameIni.insertValue("global", "info", sStatusInfo);
                            aBasenameIni.close();
                        }

                    }
                }
                catch (OfficeException e)
                {
                    // TODO: unhandled yet.
                    GlobalLogWriter.println("Warning: caught OfficeException " + e.getMessage());
                }
            }
        }
    }


/**
 * Run through a given index.ini or run through a given directory,
 * find all ini files.
 * Call the IDocument interface function call(...);
 * @param _aParam
 */
    public void foreachResultCreateHTML(ParameterHelper _aParam)
    {
        // TODO: auslagern in eine function, die ein Interface annimmt.
        String sInputPath = _aParam.getInputPath();
        new File(sInputPath);

        // call for a single ini file
        if (sInputPath.toLowerCase().endsWith(".ini") )
        {
            callEntry(sInputPath, _aParam);
        }
        else
        {
            // check if there exists an ini file
            String sPath = FileHelper.getPath(sInputPath);
            String sBasename = FileHelper.getBasename(sInputPath);

            runThroughEveryReportInIndex(sPath, sBasename, _aParam);

            // Create a HTML page which shows locally to all files in .odb
            if (sInputPath.toLowerCase().endsWith(".odb"))
            {
                String sIndexFile = FileHelper.appendPath(sPath, "index.ini");
                File aIndexFile = new File(sIndexFile);
                if (aIndexFile.exists())
                {
                    IniFile aIniFile = new IniFile(sIndexFile);

                    if (aIniFile.hasSection(sBasename))
                    {
                        // special case for odb files
                        int nFileCount = aIniFile.getIntValue(sBasename, "reportcount", 0);
                        ArrayList<String> aList = new ArrayList<String>();
                        for (int i=0;i<nFileCount;i++)
                        {
                            String sValue = aIniFile.getValue(sBasename, "report" + i);

                            String sPSorPDFName = getPSorPDFNameFromIniFile(aIniFile, sValue);
                            if (sPSorPDFName.length() > 0)
                            {
                                aList.add(sPSorPDFName);
                            }
                        }
                        if (aList.size() > 0)
                        {
                            // HTML output for the odb file, shows only all other documents.
                            HTMLResult aOutputter = new HTMLResult(sPath, sBasename + ".ps.html" );
                            aOutputter.header("content of DB file: " + sBasename);
                            aOutputter.indexSection(sBasename);

                            for (int i=0;i<aList.size();i++)
                            {
                                String sPSFile = aList.get(i);

                                // Read information out of the ini files
                                String sIndexFile2 = FileHelper.appendPath(sPath, sPSFile + ".ini");
                                IniFile aIniFile2 = new IniFile(sIndexFile2);
                                String sStatusRunThrough = aIniFile2.getValue("global", "state");
                                String sStatusMessage = "";
                                aIniFile2.close();


                                String sHTMLFile = sPSFile + ".html";
                                aOutputter.indexLine(sHTMLFile, sPSFile, sStatusRunThrough, sStatusMessage);
                            }
                            aOutputter.close();

                        }
                    }
                    aIniFile.close();
                }

            }
        }
    }


}
