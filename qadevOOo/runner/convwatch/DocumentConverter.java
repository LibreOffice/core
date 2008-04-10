/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DocumentConverter.java,v $
 * $Revision: 1.8 $
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

package convwatch;

// imports
import java.util.Enumeration;
import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;

import com.sun.star.lang.XMultiServiceFactory;

import convwatch.DirectoryHelper;
import convwatch.OfficePrint;
import convwatch.ConvWatchException;
import convwatch.EnhancedComplexTestCase;
import convwatch.PropertyName;
import helper.OfficeProvider;

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

public class DocumentConverter extends EnhancedComplexTestCase
{
    // The first of the mandatory functions:
    /**
     * Return the name of the test.
     * In this case it is the actual name of the service.
     * @return The tested service.
     */
    public String getTestObjectName() {
        return "DocumentConverter runner";
    }

    // The second of the mandatory functions: return all test methods as an
    // array. There is only one test function in this example.
    /**
     * Return all test methods.
     * @return The test methods.
     */
    public String[] getTestMethodNames() {
        return new String[]{"convert"};
    }

    // This test is fairly simple, so there is no need for before() or after()
    // methods.

    public void before()
        {
            // System.out.println("before()");
        }

    public void after()
        {
            // System.out.println("after()");
        }

    // The test method itself.
    private String m_sInputPath = "";
    private String m_sReferencePath = "";
    private boolean m_bIncludeSubdirectories = true;

    void initMember()
        {
            // MUST PARAMETER
            // INPUT_PATH ----------
            String sINPATH = (String)param.get( PropertyName.DOC_COMPARATOR_INPUT_PATH );
            boolean bQuit = false;
            String sError = "";
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
                log.println("Please set output path (path to a directory, where the references should stay) " + PropertyName.DOC_COMPARATOR_REFERENCE_PATH + "=path.");
                bQuit = true;
            }
            else
            {
                log.println("found " + PropertyName.DOC_COMPARATOR_REFERENCE_PATH + " " + sREF);
                m_sReferencePath = sREF;
            }

            if (bQuit == true)
            {
                // log.println("must quit.");
                assure("Must quit, Parameter problems.", false);
            }

            if (m_sInputPath.startsWith("file:") ||
                m_sReferencePath.startsWith("file:"))
            {
                assure("We can't handle file: URL right, use system path instead.", false);
            }

        }

    /**
     * Function returns a List of software which must accessable as an external executable
     */
    protected Object[] mustInstalledSoftware()
        {
            ArrayList aList = new ArrayList();
            // aList.add("perl -version");
            return aList.toArray();
        }

    // the test ======================================================================
    public void convert()
        {
            GlobalLogWriter.set(log);
            // check if all need software is installed and accessable
            checkEnvironment(mustInstalledSoftware());

            // test_removeFirstDirectorysAndBasenameFrom();
            // Get the MultiServiceFactory.
            // XMultiServiceFactory xMSF = (XMultiServiceFactory)param.getMSF();
            GraphicalTestArguments aGTA = getGraphicalTestArguments();
            if (aGTA == null)
            {
                assure("Must quit", false);
            }

            initMember();

            File aInputPath = new File(m_sInputPath);
            if (aInputPath.isDirectory())
            {
                String fs = System.getProperty("file.separator");

                String sRemovePath = aInputPath.getAbsolutePath();
                // a whole directory
                FileFilter aFileFilter = aGTA.getFileFilter();

                Object[] aList = DirectoryHelper.traverse(m_sInputPath, aGTA.getFileFilter(), aGTA.includeSubDirectories());
                for (int i=0;i<aList.length;i++)
                {
                    String sEntry = (String)aList[i];

                    String sNewReferencePath = m_sReferencePath + fs + FileHelper.removeFirstDirectorysAndBasenameFrom(sEntry, m_sInputPath);
                    log.println("- next file is: ------------------------------");
                    log.println(sEntry);

                    if (aGTA.checkIfUsableDocumentType(sEntry))
                    {
                        runGDC(sEntry, sNewReferencePath);
                    }
                    if (aGTA.cancelRequest())
                    {
                        break;
                    }
                }
            }
            else
            {
                if (aGTA.checkIfUsableDocumentType(m_sInputPath))
                {
                    runGDC(m_sInputPath, m_sReferencePath);
                }
            }
        }

    void runGDC(String _sInputFile, String _sReferencePath)
        {
            // first do a check if the reference not already exist, this is a big speedup, due to the fact,
            // we don't need to start a new office.
            GraphicalTestArguments aGTA_local = getGraphicalTestArguments();
            // if (GraphicalDifferenceCheck.isReferenceExists(_sInputFile, _sReferencePath, aGTA_local) == false)
            // {
            // start a fresh Office
            OfficeProvider aProvider = null;
            if (aGTA_local.restartOffice())
            {
                aProvider = new OfficeProvider();
                XMultiServiceFactory xMSF = (XMultiServiceFactory) aProvider.getManager(param);
                param.put("ServiceFactory", xMSF);
            }
            GraphicalTestArguments aGTA = getGraphicalTestArguments();

            if (aGTA.getOfficeProgram().toLowerCase().equals("msoffice"))
            {
                // ReferenceType is MSOffice
                GlobalLogWriter.get().println("USE MSOFFICE AS EXPORT FORMAT.");
                MSOfficePrint a = new MSOfficePrint();
                try
                {
                    String sInputFileBasename = FileHelper.getBasename(_sInputFile);
                    String fs = System.getProperty("file.separator");
                    FileHelper.makeDirectories("", _sReferencePath);
                    String sOutputFile = _sReferencePath;
                    if (sOutputFile.endsWith(fs))
                    {
                        sOutputFile += sInputFileBasename;
                    }
                    else
                    {
                        sOutputFile += fs + sInputFileBasename;
                    }

                    a.storeToFileWithMSOffice(aGTA, _sInputFile, sOutputFile);
                }
                catch(ConvWatchCancelException e)
                {
                    GlobalLogWriter.get().println(e.getMessage());
                }
                catch(java.io.IOException e)
                {
                    GlobalLogWriter.get().println(e.getMessage());
                }
            }
            else
            {
                try
                {
                    OfficePrint.convertDocument(_sInputFile, _sReferencePath, aGTA);
                }
                catch(ConvWatchCancelException e)
                {
                    assure(e.getMessage(), false);
                }
                catch(ConvWatchException e)
                {
                    assure(e.getMessage(), false);
                }
            }

            if (aGTA.restartOffice())
            {
                // Office shutdown
                aProvider.closeExistingOffice(param, true);
            }
            // }
        }
}

