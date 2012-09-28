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
import com.sun.star.lang.XMultiServiceFactory;

import convwatch.DirectoryHelper;
import convwatch.ConvWatchException;
import convwatch.EnhancedComplexTestCase;
import convwatch.PropertyName;
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

public class ReferenceBuilder extends EnhancedComplexTestCase
{
    // The first of the mandatory functions:
    /**
     * Return the name of the test.
     * In this case it is the actual name of the service.
     * @return The tested service.
     */
    public String getTestObjectName() {
        return "ReferenceBuilder runner";
    }

    // The second of the mandatory functions: return all test methods as an
    // array. There is only one test function in this example.
    /**
     * Return all test methods.
     * @return The test methods.
     */
    public String[] getTestMethodNames() {
        return new String[]{"buildreference"};
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
            return new String[] { "perl -version" };
        }

    // the test ======================================================================
    public void buildreference()
        {
            GlobalLogWriter.set(log);
            String sDBConnection = (String)param.get( PropertyName.DB_CONNECTION_STRING );

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
            if (aGTA.cancelRequest())
            {
                return;
            }

            initMember();
            DB.init(aGTA.getDBInfoString() + "," + sDBConnection);
            File aInputPath = new File(m_sInputPath);
            if (aInputPath.isDirectory())
            {
                String fs = System.getProperty("file.separator");

                aInputPath.getAbsolutePath();
                // a whole directory
                FileFilter aFileFilter = FileHelper.getFileFilter();

                Object[] aList = DirectoryHelper.traverse(m_sInputPath, aFileFilter, aGTA.includeSubDirectories());
                // fill into DB
                // DB.filesRemove(aGTA.getDBInfoString());
                // for (int j=0;j<aList.length;j++)
                // {
                //     String sEntry = (String)aList[j];
                //     DB.fileInsert(aGTA.getDBInfoString(), sEntry, sRemovePath);
                // }

                // normal run.
                for (int i=0;i<aList.length;i++)
                {
                    String sEntry = (String)aList[i];

                    String sNewReferencePath = m_sReferencePath + fs + FileHelper.removeFirstDirectorysAndBasenameFrom(sEntry, m_sInputPath);
                    log.println("- next file is: ------------------------------");
                    log.println(sEntry);
                    log.println(sNewReferencePath);

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
                // String sRemovePath = aInputPath.getAbsolutePath();
                // DB.fileInsert(aGTA.getDBInfoString(), m_sInputPath, sRemovePath);
                // DB.updatestate_status(aGTA.getDBInfoString(), "started: " + m_sInputPath);
                if (aGTA.checkIfUsableDocumentType(m_sInputPath))
                {
                    runGDC(m_sInputPath, m_sReferencePath);
                }
            }
        }

    void runGDC(String _sInputPath, String _sReferencePath)
        {
            // first do a check if the reference not already exist, this is a big speedup, due to the fact,
            // we don't need to start a new office.
            GraphicalTestArguments aGTA = getGraphicalTestArguments();
            if (GraphicalDifferenceCheck.isReferenceExists(_sInputPath, _sReferencePath, aGTA) == false)
            {
            // start a fresh Office
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
                    aGTA = getGraphicalTestArguments();
                    aGTA.getPerformance().setTime(PerformanceContainer.OfficeStart, nStartTime);
                }

                // Watcher Object is need in log object to give a simple way to say if a running office is alive.
                // As long as a log comes, it pings the Watcher and says the office is alive, if not an
                // internal counter increase and at a given point (300 seconds) the office is killed.
                GlobalLogWriter.get().println("Set office watcher");
                Object aWatcher = param.get("Watcher");
                GlobalLogWriter.get().setWatcher(aWatcher);
                // initializeWatcher(param);

                try
                {
                    log.println("Reference type is " + aGTA.getReferenceType());
                    DB.source_start();
                    GraphicalDifferenceCheck.createOneReferenceFile(_sInputPath, _sReferencePath, aGTA);
                    DB.source_finished();
                }
                catch(ConvWatchCancelException e)
                {
                    assure(e.getMessage(), false);
                    DB.source_failed(e.getMessage());
                }
                catch(ConvWatchException e)
                {
                    assure(e.getMessage(), false);
                    DB.source_failed(e.getMessage());
                }
                catch(com.sun.star.lang.DisposedException e)
                {
                    assure(e.getMessage(), false, true);
                    DB.source_failed(e.getMessage());
                }

                // Office shutdown
                if (aProvider != null)
                {
                    boolean bClosed = aProvider.closeExistingOffice(param, true);
                    // Hope I can check that the close of the office fails
                    assure("Office closed", bClosed, true);
                    // if (OSHelper.isWindows())
                    // {
                    //     aSemaphore.V(aSemaphore.getSemaphoreFile());
                    //     aSemaphore.sleep(2);
                    //     // wait some time maybe an other process will take the semaphore
                    //     // I know, this is absolutly dirty, but the whole convwatch is dirty and need a big cleanup.
                    // }
                }
            }
            else
            {
                // Reference already exist, do nothing, but DB change
                DB.source_finished();
            }
        }
}

