/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConvWatchStarter.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 17:40:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package convwatch;

// imports
import java.util.Enumeration;
import java.util.ArrayList;
import java.io.File;
import java.io.FileFilter;

import convwatch.EnhancedComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;

import helper.ProcessHandler;
import convwatch.ConvWatchException;
import convwatch.DirectoryHelper;
import convwatch.GraphicalTestArguments;
import convwatch.NameHelper;
import convwatch.HTMLOutputter;
import helper.OfficeProvider;
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
     * @return a List of software which must accessable as an external executable
     */
    protected Object[] mustInstalledSoftware()
        {
            ArrayList aList = new ArrayList();
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

            return aList.toArray();
        }


    /**
     * The test method itself.
     * Don't try to call it from outside, it is started only from qadevOOo runner
     */

    /* protected */
    public void compareGraphicalDiffs()
        {
            // check if all need software is installed and accessable
            checkEnvironment(mustInstalledSoftware());

            GraphicalTestArguments aGTA = getGraphicalTestArguments();
            if (aGTA == null)
            {
                assure("Must quit", false);
            }
            initMember();

            String sBuildID = aGTA.getBuildID();
            log.println("Current Office has buildid: " + sBuildID);

            // LLA: sample code, how to access all parameters
            // for (Enumeration e = param.keys() ; e.hasMoreElements() ;)
            // {
            //     System.out.println(e.nextElement());
            // }

            HTMLOutputter HTMLoutput = HTMLOutputter.create(m_sOutputPath, "index.html", "", "");
            HTMLoutput.header( m_sOutputPath );
            HTMLoutput.indexSection( m_sOutputPath );
            LISTOutputter LISToutput = LISTOutputter.create(m_sOutputPath, "allfiles.txt");

            File aInputPath = new File(m_sInputPath);
            if (aInputPath.isDirectory())
            {
                String fs = System.getProperty("file.separator");
                // a whole directory
                FileFilter aFileFilter = aGTA.getFileFilter();

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

                        if (aGTA.checkIfUsable(sEntry))
                        {
                            runGDCWithStatus(HTMLoutput, LISToutput, sEntry, sNewOutputPath, sNewReferencePath, sNewDiffPath, sNewSubDir);
                        }
                    }
                }
            }
            else
            {
                if (aGTA.checkIfUsable(m_sInputPath))
                {
                    runGDCWithStatus(HTMLoutput, LISToutput, m_sInputPath, m_sOutputPath, m_sReferencePath, m_sDiffPath, "");
                }
            }

            LISToutput.close();
            HTMLoutput.close();
            log.println("The file '" + HTMLoutput.getFilename() + "' shows a html based status.");
        }


    // -----------------------------------------------------------------------------
    void runGDCWithStatus(HTMLOutputter _aHTMLoutput, LISTOutputter _aLISToutput, String _sInputFile, String _sOutputPath, String _sReferencePath, String _sDiffPath, String _sNewSubDir )
        {
            // start a fresh Office
            GraphicalTestArguments aGTA = getGraphicalTestArguments();

            OfficeProvider aProvider = null;
            if (aGTA.shouldOfficeStart())
            {
                aGTA.getPerformance().startTime(PerformanceContainer.OfficeStart);
                aProvider = new OfficeProvider();
                XMultiServiceFactory xMSF = (XMultiServiceFactory) aProvider.getManager(param);
                param.put("ServiceFactory", xMSF);
                aGTA.getPerformance().stopTime(PerformanceContainer.OfficeStart);

                long nStartTime = aGTA.getPerformance().getTime(PerformanceContainer.OfficeStart);
                aGTA = getGraphicalTestArguments(); // get new TestArguments
                aGTA.getPerformance().setTime(PerformanceContainer.OfficeStart, nStartTime);
            }

            String sStatusRunThrough = "";
            String sStatusMessage = "";
            try
            {
                GraphicalDifferenceCheck.checkOneFile(_sInputFile, _sOutputPath, _sReferencePath, _sDiffPath, aGTA);
                sStatusRunThrough = "PASSED, OK";
            }
            catch(ConvWatchCancelException e)
            {
                assure(e.getMessage(), false, true);
                sStatusRunThrough = "CANCELED, FAILED";
                sStatusMessage = e.getMessage();
            }
            catch(ConvWatchException e)
            {
                assure(e.getMessage(), false, true);
                sStatusMessage = e.getMessage();
                sStatusRunThrough = "PASSED, FAILED";
            }
            catch(com.sun.star.lang.DisposedException e)
            {
                assure(e.getMessage(), false, true);
                sStatusMessage = e.getMessage();
                sStatusRunThrough = "FAILED, FAILED";
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

            System.out.println("----------------------------------------------------------------------");
            System.out.println(" OutputPath: " + _sOutputPath);
            System.out.println("    NewPath: " + _sNewSubDir);
            System.out.println("----------------------------------------------------------------------");

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

            // Office shutdown
            if (aProvider != null)
            {
                aProvider.closeExistingOffice(param, true);
            }
        }


}
