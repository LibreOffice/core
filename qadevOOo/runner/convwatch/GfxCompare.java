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

import java.util.ArrayList;
import convwatch.EnhancedComplexTestCase;
import convwatch.PRNCompare;
import convwatch.GraphicalTestArguments;
import helper.URLHelper;
import convwatch.OfficePrint;
import java.io.File;

public class GfxCompare extends EnhancedComplexTestCase
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
        return new String[]{"gfxcompare"};
    }

    /**
     *
     * @return a List of software which must accessible as an external executable
     */
    protected String[] mustInstalledSoftware()
        {
            ArrayList<String> aList = new ArrayList<String>();
            // Tools from ImageMagick
            aList.add( "composite -version" );
            aList.add( "identify -version" );

            // Ghostscript
            aList.add( "gs -version" );
            return aList.toArray(new String[aList.size()]);
        }


    GraphicalTestArguments m_aArguments = null;
    /**
     * The test method itself.
     * Don't try to call it from outside, it is started only from qadevOOo runner
     */

    /* protected */
    public void gfxcompare()
        {
            GlobalLogWriter.set(log);

            // check if all need software is installed and accessible
            checkEnvironment(mustInstalledSoftware());

            m_aArguments = getGraphicalTestArguments();

            String sFile1 = (String)param.get("FILE1");
            String sFile2 = (String)param.get("FILE2");
            compare(sFile1, sFile2);
        }



    String createJPEG(String _sFile, String _sAdditional)
        {
            String sJPEGFile = "";
            if (_sFile.startsWith("file:///"))
            {
                _sFile = FileHelper.getSystemPathFromFileURL(_sFile);
            }
            File aFile = new File(_sFile);
            if (aFile.exists())
            {
                String sAbsFile = aFile.getAbsolutePath();
                if (!sAbsFile.equals(_sFile))
                {
                    _sFile = sAbsFile;
                }
            }
            else
            {
                GlobalLogWriter.get().println("File: '" + _sFile + "' doesn't exist.");
                return "";
            }
            String sFileDir = FileHelper.getPath(_sFile);
            String sBasename = FileHelper.getBasename(_sFile);
            String sNameNoSuffix = FileHelper.getNameNoSuffix(sBasename);

            String fs = System.getProperty("file.separator");
            String sTmpDir = util.utils.getUsersTempDir();
            if (m_aArguments.getOutputPath() != null)
            {
                sTmpDir = m_aArguments.getOutputPath();
            }

            if (_sFile.toLowerCase().endsWith("ps") ||
                _sFile.toLowerCase().endsWith("prn") ||
                _sFile.toLowerCase().endsWith("pdf"))
            {
                // seems to be a Postscript of PDF file

                String[] aList = PRNCompare.createJPEGFromPostscript(sTmpDir, sFileDir, sBasename, m_aArguments.getResolutionInDPI());
                sJPEGFile = aList[0];
            }
            else if (_sFile.toLowerCase().endsWith("jpg") ||
                     _sFile.toLowerCase().endsWith("jpeg"))
            {
                // do nothing, it's already a picture.
                return _sFile;
            }
            else
            {
                // we assume it's an office document.
                String sInputURL;
                String sOutputURL;
                String sPrintFileURL;

                String sInputFile = sFileDir + fs + sBasename;
                sInputURL = URLHelper.getFileURLFromSystemPath(sInputFile);

                String sOutputFile = sTmpDir + fs + sBasename;
                sOutputURL = URLHelper.getFileURLFromSystemPath(sOutputFile);

                String sPrintFile = sTmpDir + fs + sNameNoSuffix + _sAdditional + ".ps";
                sPrintFileURL = URLHelper.getFileURLFromSystemPath(sPrintFile);

                try
                {
                    OfficePrint.printToFile(m_aArguments, sInputURL, sOutputURL, sPrintFileURL);
                    sJPEGFile = createJPEG(sPrintFile, _sAdditional);
                }
                catch (ConvWatchCancelException e)
                {
                    GlobalLogWriter.get().println("Exception caught, can't create:" + sPrintFileURL);
                }
            }
            return sJPEGFile;
        }


    public String compare(String _sFile1, String _sFile2)
        {
            String sJPEGFile1 = createJPEG(_sFile1, "-1");
            String sJPEGFile2 = createJPEG(_sFile2, "-2");

            if (sJPEGFile1.length() > 0 && sJPEGFile2.length() > 0)
            {
                String sDiffFile = PRNCompare.compareJPEGs(sJPEGFile1, sJPEGFile2);

                if (sDiffFile.length() > 0)
                {
                    GlobalLogWriter.get().println("Difference created: " + sDiffFile);
                }
                return sDiffFile;
            }
            return "";
        }

}
