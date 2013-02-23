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

// imports
import java.util.ArrayList;
import java.io.File;
import java.io.FileFilter;

import convwatch.EnhancedComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;

import convwatch.ConvWatchException;
import convwatch.DirectoryHelper;
import convwatch.GraphicalTestArguments;
import convwatch.HTMLOutputter;
import helper.OfficeProvider;
import helper.OfficeWatcher;
import helper.OSHelper;
import convwatch.PerformanceContainer;

/**
 * The following Complex Test will test
 * an already created document and it's postscript output (by an older office version)
 * with a new office version.
 * This test use Ghostscript for the jpeg export and graphically compare tools from ImageMagick.
 * Read the manual for more information.
 *
 * this is only the starter program
 * more is found in qadevOOo/runner/convwatch/*
 */

public class ConvWatchStarter extends EnhancedComplexTestCase
{
    // The first of the mandatory functions:
    /**
     * Return the name of the test.
     * In this case it is the actual name of the service.
     * @return The tested service.
     */
    // public String getTestObjectName() {
    //     return "ConvWatch runner";
    // }

    // The second of the mandatory functions: return all test methods as an
    // array. There is only one test function in this example.
    /**
     * Return all test methods.
     * @return The test methods.
     */
    public String[] getTestMethodNames() {
        return new String[]{"compareGraphicalDiffs"};
    }

    String m_sInputPath = "";
    String m_sReferencePath = "";
    String m_sOutputPath = "";
    String m_sDiffPath = null;

    private void initMember()
        {
            // MUST PARAMETER
            // INPUTFILE ----------
            String sINPATH = (String)param.get( PropertyName.DOC_COMPARATOR_INPUT_PATH );
            boolean bQuit = false;
            if (sINPATH == null || sINPATH.length() == 0)
            {
                log.println("Please set input path (path to documents) " + PropertyName.DOC_COMPARATOR_INPUT_PATH + "=path.");
                bQuit = true;
            }
            else
            {
                log.println("found " + PropertyName.DOC_COMPARATOR_INPUT_PATH + " " + sINPATH);
                m_sInputPath = sINPATH;
            }

            // REFERENCE_PATH ----------
            String sREF = (String)param.get( PropertyName.DOC_COMPARATOR_REFERENCE_PATH );
            if (sREF == null || sREF.length() == 0)
            {
                // log.println("Please set reference file (path to good documents) REFERENCEFILE=path.");
                log.println("Assumtion, reference directory and input directory are the same.");
                m_sReferencePath = m_sInputPath;
            }
            else
            {
                log.println("found " + PropertyName.DOC_COMPARATOR_REFERENCE_PATH + " " + sREF);
                m_sReferencePath = sREF;
            }

            // OUTPUT_PATH ----------
            String sOUT = (String)param.get( PropertyName.DOC_COMPARATOR_OUTPUT_PATH );
            if (sOUT == null || sOUT.length() == 0)
            {
                log.println("Please set output path (path to a temp directory) " + PropertyName.DOC_COMPARATOR_OUTPUT_PATH + "=path.");
                bQuit = true;
            }
            else
            {
                log.println("found " + PropertyName.DOC_COMPARATOR_OUTPUT_PATH + " " + sOUT);
                m_sOutputPath = sOUT;
            }

            if (bQuit == true)
            {
                // log.println("must quit.");
                assure("Must quit", false);
            }


            // DIFF_PATH ----------
            String sDIFF = (String)param.get( PropertyName.DOC_COMPARATOR_DIFF_PATH );
            if (sDIFF == null || sDIFF.length() == 0)
            {
            }
            else
            {
                log.println("found " + PropertyName.DOC_COMPARATOR_DIFF_PATH + " " + sDIFF);
                m_sDiffPath = sDIFF;
            }

            if (m_sInputPath.startsWith("file:") ||
                m_sReferencePath.startsWith("file:") ||
                m_sOutputPath.startsWith("file:"))
            {
                assure("We can't handle file: URL right, use system path instead.", false);
            }

        }


    /**
     *
     * @return a List of software which must accessible as an external executable
     */
    protected String[] mustInstalledSoftware()
        {
            ArrayList<String> aList = new ArrayList<String>();
            // Tools from ImageMagick
            if (! OSHelper.isWindows())
            {
            aList.add( "composite -version" );
            aList.add( "identify -version" );

            // Ghostscript
            aList.add( "gs -version" );
            }
            else
            {
                aList.add( "composite.exe -version" );
                aList.add( "identify.exe -version" );

                // Ghostscript
                aList.add( "gswin32c.exe -version" );
            }

            return aList.toArray(new String[aList.size()]);
        }


    /**
     * The test method itself.
     * Don't try to call it from outside, it is started only from qadevOOo runner
     */

    /* protected */
    public void compareGraphicalDiffs()
        {
            GlobalLogWriter.set(log);
            String sDBConnection = (String)param.get( PropertyName.DB_CONNECTION_STRING );
            // check if all need software is installed and accessible
            checkEnvironment(mustInstalledSoftware());

            GraphicalTestArguments aGTA = getGraphicalTestArguments();
            if (aGTA == null)
            {
                assure("Must quit", false);
            }
            if (aGTA.cancelRequest())
            {
                return;
            }

            initMember();

            aGTA.allowStore();

            String sBuildID = aGTA.getBuildID();
            log.println("Current Office has buildid: " + sBuildID);

            // LLA: sample code, how to access all parameters
            // for (Enumeration e = param.keys() ; e.hasMoreElements() ;)
            // {
            //     System.out.println(e.nextElement());
            // }

            String fs = System.getProperty("file.separator");

            String sHTMLName = "index.html";
            File aInputPathTest = new File(m_sInputPath);
            if (!aInputPathTest.isDirectory())
            {
                int n = m_sInputPath.lastIndexOf(fs);
                sHTMLName = m_sInputPath.substring(n + 1);
                sHTMLName += ".html";
            }
            HTMLOutputter HTMLoutput = HTMLOutputter.create(m_sOutputPath, sHTMLName, "", "");
            HTMLoutput.header( m_sOutputPath );
            HTMLoutput.indexSection( m_sOutputPath );
            LISTOutputter LISToutput = LISTOutputter.create(m_sOutputPath, "allfiles.txt");

            DB.init(aGTA.getDBInfoString() + "," + sDBConnection);

            File aInputPath = new File(m_sInputPath);
            if (aInputPath.isDirectory())
            {
                // check a whole directory
                // a whole directory
                FileFilter aFileFilter = FileHelper.getFileFilter();

                Object[] aList = DirectoryHelper.traverse(m_sInputPath, aFileFilter, aGTA.includeSubDirectories());
                if (aList.length == 0)
                {
                    log.println("Nothing to do, there are no document files found.");
                }
                else
                {
                    for (int i=0;i<aList.length;i++)
                    {
                        String sEntry = (String)aList[i];
                        log.println("- next file is: ------------------------------");
                        log.println(sEntry);

                        String sNewSubDir = FileHelper.removeFirstDirectorysAndBasenameFrom(sEntry, m_sInputPath);

                        String sNewReferencePath = m_sReferencePath;
                        String sNewOutputPath = m_sOutputPath;
                        String sNewDiffPath = m_sDiffPath;
                        if (sNewSubDir.length() > 0)
                        {
                            if (sNewReferencePath != null)
                                sNewReferencePath = sNewReferencePath + fs + sNewSubDir;
                            // String sNameNoSuffix = FileHelper.getNameNoSuffix(FileHelper.getBasename(sEntry));
                            // sNewReferenceFile = sNewReferencePath + fs + sNameNoSuffix + ".prn";

                            sNewOutputPath = sNewOutputPath + fs + sNewSubDir;
                            if (sNewDiffPath != null)
                                sNewDiffPath = sNewDiffPath + fs + sNewSubDir;
                        }

                        // NameHelper aNameContainer = new NameHelper(m_sOutputPath, sNewSubDir, FileHelper.getBasename(sEntry));
                        // aNameContainer.print();

                        if (aGTA.checkIfUsableDocumentType(sEntry))
                        {
                            runGDCWithStatus(HTMLoutput, LISToutput, sEntry, sNewOutputPath, sNewReferencePath, sNewDiffPath, sNewSubDir);
                        }
                        if (aGTA.cancelRequest())
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                // check exact name
                if (aGTA.checkIfUsableDocumentType(m_sInputPath))
                {
                    runGDCWithStatus(HTMLoutput, LISToutput, m_sInputPath, m_sOutputPath, m_sReferencePath, m_sDiffPath, "");
                }
            }

            LISToutput.close();
            HTMLoutput.close();
            log.println("The file '" + HTMLoutput.getFilename() + "' shows a html based status.");
            DB.writeHTMLFile(HTMLoutput.getFilename());
        }


    // -----------------------------------------------------------------------------
    void runGDCWithStatus(HTMLOutputter _aHTMLoutput, LISTOutputter _aLISToutput, String _sInputFile, String _sOutputPath, String _sReferencePath, String _sDiffPath, String _sNewSubDir )
        {
            // start a fresh Office
            GraphicalTestArguments aGTA = getGraphicalTestArguments();

            OfficeProvider aProvider = null;
            // SimpleFileSemaphore aSemaphore = new SimpleFileSemaphore();
            if (aGTA.shouldOfficeStart())
            {
                // if (OSHelper.isWindows())
                // {
                //     aSemaphore.P(aSemaphore.getSemaphoreFile());
                // }

                aGTA.getPerformance().startTime(PerformanceContainer.OfficeStart);
                aProvider = new OfficeProvider();
                XMultiServiceFactory xMSF = (XMultiServiceFactory) aProvider.getManager(param);
                param.put("ServiceFactory", xMSF);
                aGTA.getPerformance().stopTime(PerformanceContainer.OfficeStart);

                long nStartTime = aGTA.getPerformance().getTime(PerformanceContainer.OfficeStart);
                aGTA = getGraphicalTestArguments(); // get new TestArguments
                aGTA.getPerformance().setTime(PerformanceContainer.OfficeStart, nStartTime);
            }

            // Watcher Object is need in log object to give a simple way to say if a running office is alive.
            // As long as a log comes, it pings the Watcher and says the office is alive, if not an
            // internal counter increase and at a given point (300 seconds) the office is killed.
            GlobalLogWriter.get().println("Set office watcher");
            OfficeWatcher aWatcher = (OfficeWatcher)param.get("Watcher");
            GlobalLogWriter.get().setWatcher(aWatcher);
            // initializeWatcher(param);

            String sStatusRunThrough = "";
            String sStatusMessage = "";
            try
            {
                DB.destination_start();
                // better was:
                // load document
                // create postscript from document
                // check file
                GraphicalDifferenceCheck.checkOneFile(_sInputFile, _sOutputPath, _sReferencePath, _sDiffPath, aGTA);
                sStatusRunThrough = "PASSED, OK";
                DB.destination_finished();
            }
            catch(ConvWatchCancelException e)
            {
                assure(e.getMessage(), false, true);
                sStatusRunThrough = "CANCELLED, FAILED";
                sStatusMessage = e.getMessage();
                DB.destination_failed(sStatusRunThrough, sStatusMessage);
            }
            catch(ConvWatchException e)
            {
                assure(e.getMessage(), false, true);
                sStatusMessage = e.getMessage();
                sStatusRunThrough = "PASSED, FAILED";
                DB.destination_failed(sStatusRunThrough, sStatusMessage);
            }
            catch(com.sun.star.lang.DisposedException e)
            {
                assure(e.getMessage(), false, true);
                sStatusMessage = e.getMessage();
                sStatusRunThrough = "FAILED, FAILED";
                DB.destination_failed(sStatusRunThrough, sStatusMessage);
            }

            GlobalLogWriter.get().println("Watcher count is: " + aWatcher.getPing());

            // Office shutdown
            if (aProvider != null)
            {
                aProvider.closeExistingOffice(param, true);
                // if (OSHelper.isWindows())
                // {
                //     aSemaphore.V(aSemaphore.getSemaphoreFile());
                //     aSemaphore.sleep(2);
                //     // wait some time maybe an other process will take the semaphore
                //     // I know, this is absolutly dirty, but the whole convwatch is dirty and need a big cleanup.
                // }
            }

            // -------------------- Status --------------------
            String fs = System.getProperty("file.separator");
            String sBasename = FileHelper.getBasename(_sInputFile);
            String sFilenameNoSuffix = FileHelper.getNameNoSuffix(sBasename);

            // -------------------- List of all files -----------------
            String sListFile;
            if (_sNewSubDir.length() > 0)
            {
                sListFile = _sNewSubDir + fs + sFilenameNoSuffix + ".ini";
            }
            else
            {
                sListFile = sFilenameNoSuffix + ".ini";
            }
            _aLISToutput.writeValue(sListFile);

            // -------------------- HTML --------------------
            String sLink;
            String sLinkDD;
            String sLinkName;
            String sLinkDDName;
            String sHTMLPrefix = aGTA.getHTMLOutputPrefix();

            GlobalLogWriter.get().println("----------------------------------------------------------------------");
            GlobalLogWriter.get().println(" OutputPath: " + _sOutputPath);
            GlobalLogWriter.get().println("    NewPath: " + _sNewSubDir);
            GlobalLogWriter.get().println("----------------------------------------------------------------------");

//             if (_sNewSubDir.length() > 0)
//             {
//                 sLink   = sHTMLPrefix /* + "/cw.php?inifile=" */ + _sOutputPath + fs + _sNewSubDir + fs + sFilenameNoSuffix + ".ini";
//                 sLinkDD = sHTMLPrefix /* + "/cw.php?inifile=" */ + _sOutputPath + fs + _sNewSubDir + fs + "DiffDiff_" + sFilenameNoSuffix + ".ini";
//             }
//             else
//             {
            sLink = sHTMLPrefix   /* + "/cw.php?inifile=" */ + _sOutputPath + fs + sFilenameNoSuffix + ".ini";
                // sLinkDD = sHTMLPrefix /* + "/cw.php?inifile=" */ + _sOutputPath + fs + _sNewSubDir + fs + "DiffDiff_" + sFilenameNoSuffix + ".ini";
            sLinkDD = sHTMLPrefix /* + "/cw.php?inifile=" */ + _sOutputPath + fs + "DiffDiff_" + sFilenameNoSuffix + ".ini";
//             }
            sLinkName = sFilenameNoSuffix;
            sLinkDDName = sFilenameNoSuffix + " (DiffDiff)";

            if (_sDiffPath != null && _sDiffPath.length() > 0)
            {
                _aHTMLoutput.indexLine( sLinkDD, sLinkDDName, sLink, sLinkName, sStatusRunThrough, sStatusMessage );
            }
            else
            {
                _aHTMLoutput.indexLine( sLink, sLinkName, "", "", sStatusRunThrough, sStatusMessage );
            }

        }


}
