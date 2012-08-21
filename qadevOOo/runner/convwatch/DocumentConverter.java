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
import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;

import com.sun.star.lang.XMultiServiceFactory;

import convwatch.DirectoryHelper;
import convwatch.OfficePrint;
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
    void initMember()
        {
            // MUST PARAMETER
            // INPUT_PATH ----------
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
    protected String[] mustInstalledSoftware()
        {
            ArrayList<String> aList = new ArrayList<String>();
            // aList.add("perl -version");
            return aList.toArray(new String[aList.size()]);
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

                aInputPath.getAbsolutePath();
                // a whole directory
                FileFilter aFileFilter = FileHelper.getFileFilter();

                Object[] aList = DirectoryHelper.traverse(m_sInputPath, aFileFilter, aGTA.includeSubDirectories());
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
            }

            if (aGTA.restartOffice())
            {
                // Office shutdown
                aProvider.closeExistingOffice(param, true);
            }
            // }
        }
}

