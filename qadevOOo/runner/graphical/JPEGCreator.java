/*
 * ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
 * ***********************************************************************
 */

package graphical;

import helper.OSHelper;
import helper.ProcessHandler;
import helper.StringHelper;
import java.io.File;

/**
 *
 * @author ll93751
 */
public class JPEGCreator extends EnhancedComplexTestCase
{
    // @Override
    public String[] getTestMethodNames()
    {
        return new String[]{"PostscriptOrPDFToJPEG"};
    }

    /**
     * test function.
     */
    public void PostscriptOrPDFToJPEG()
    {
        GlobalLogWriter.set(log);
        ParameterHelper aParam = new ParameterHelper(param);

        // run through all documents found in Inputpath
        foreachPSorPDFinInputPath(aParam);
    }


    public void checkOneFile(String _sDocumentName, String _sResult, ParameterHelper _aParams) throws OfficeException
    {
         GlobalLogWriter.println("  Document: " + _sDocumentName);
         GlobalLogWriter.println("   results: " + _sResult);
//         IOffice aOffice = new Office(_aParams, _sResult);
//         aOffice.start();
//         aOffice.load(_sDocumentName);
//         aOffice.storeAsPostscript();
//         aOffice.close();
         String sJPEGNameSchema = createJPEG(_sDocumentName, "", _aParams);

         // store information only if jpeg files exists
         int nPages = countPages(sJPEGNameSchema);
         if (nPages > 0)
         {
             createSmallPictures(sJPEGNameSchema);

             // read out tolerance file
             String sFileDir = FileHelper.getPath(_sDocumentName);
             String sBasename = FileHelper.getBasename(_sDocumentName);
             int nTolerance = 0;
             String sToleranceFile = FileHelper.appendPath(sFileDir, "tolerance.ini");
             File aToleranceFile = new File(sToleranceFile);
             if (aToleranceFile.exists())
             {
                 IniFile aIniFile = new IniFile(sToleranceFile);
                 nTolerance = aIniFile.getIntValue(sBasename, "accept", 0); // default for all pages
                 aIniFile.close();
             }

             String sIndexFile = FileHelper.appendPath(_sResult, "index.ini");
             File aIndexFile = new File(sIndexFile);
             if (aIndexFile.exists())
             {
                 // store only if an index file exists
                 IniFile aIniFile = new IniFile(sIndexFile);
                 aIniFile.insertValue(sBasename, "jpegschema", sJPEGNameSchema);
                 aIniFile.insertValue(sBasename, "pages", nPages);
                 aIniFile.insertValue(sBasename, "tolerance", nTolerance);
                 aIniFile.close();
             }
         }
         else
         {
             assure("There are no pages in document:'" + _sDocumentName + "', maybe document currupt?", false, true);
         }
    }

/**
 * Create a lot of smaller and nicer Pictures of the big fat pages.
 * Looks better
 * @param _sJPEGSchema
 */
    public void createSmallPictures(String _sJPEGSchema)
    {
        ParameterHelper aParam = new ParameterHelper(param);
        if (! aParam.createSmallPictures())
        {
            return;
        }

        int nPages = 0;
        if (_sJPEGSchema.length() > 0)
        {
            // TODO: if there doesn't exists a '%04d' in the schema we will return 9999 which is a little bit wrong here.
            for (int i=1;i<10000;i++)
            {
                String sJPEGFilename = getFilenameForJPEGSchema(_sJPEGSchema, i);
                if (FileHelper.exists(sJPEGFilename))
                {
                    convertToNearSameFileWithWidth340(sJPEGFilename);
                    // m_aFileList.add(sNewJPEGFilename); // as long as the files exist, fill the array
                    nPages ++;
                }
                else
                {
                    break;                             // stop file check
                }
            }
        }
        // return nPages;
    }

/**
 * convert a picture to a new picture with 340 pixel width.
 * @param _sJPEGFilename
 */
public static void convertToNearSameFileWithWidth340(String _sJPEGFilename)
{
    ParameterHelper aParam = new ParameterHelper(param);
    if (! aParam.createSmallPictures())
    {
        return;
    }
    String sJPEGFilename = _sJPEGFilename.replaceAll("\\\\", "/");
//    if (OSHelper.isWindows())
//    {
//        sJPEGFilename = sJPEGFilename.replaceAll("/", "\\\\");
//    }
    String sNewJPEGFilename;
    sNewJPEGFilename = util.utils.replaceAll13(sJPEGFilename, ".jpg", "_w340.jpg");
    convertToWidth340(sJPEGFilename, sNewJPEGFilename);
}

    /**
 * convert chart2_Regression.ods.ps_180DPI_0001.jpg -filter Catrom -resize
340x chart2_Regression.ods.ps_180DPI_0001_w340.jpg

Point wie bisher
Cubic schlecht, weil unscharf
...
Triangle ganz brauchbar (default?)
Catrom am besten

 * @param _sFrom
 * @param _To
 */
private static void convertToWidth340(String _sFrom, String _To)
{
            // int nResult = 0;

            String sConvertEXE = "convert";
            if (OSHelper.isLinuxIntel())
            {
                sConvertEXE = "convert";
            }
            if (OSHelper.isWindows())
            {
                // TODO!
                // HACK Hard coded!
                // sConvertEXE = "C:\\Programme\\ImageMagick-6.0.3-q8\\convert.exe";
                sConvertEXE = "convert.exe";
                String sConvertPath = (String)param.get("imagemagick.path");
                if (sConvertPath != null)
                {
                    sConvertEXE = FileHelper.appendPath(sConvertPath, sConvertEXE);
                }
            }

            String[] sCommandArray =
                {
                    sConvertEXE,
                    _sFrom,
                     "-filter", "Catrom",
                    "-resize", "340x",
                    _To
                };
            ProcessHandler aHandler = new ProcessHandler(sCommandArray);
            boolean bBackValue = aHandler.executeSynchronously();
            int nExitCode = aHandler.getExitCode();

            String sBack = aHandler.getOutputText();
            if (sBack.length() > 0)
            {
                GlobalLogWriter.println("'" + sBack + "'");
            }
            // try to interpret the result, which we get as a String
//            try
//            {
//                int nIdx = sBack.indexOf("\n");
//                if (nIdx > 0)
//                {
//                    sBack = sBack.substring(0, nIdx);
//                }
//
//                nResult = Integer.valueOf(sBack).intValue();
//            }
//            catch(java.lang.NumberFormatException e)
//            {
//                GlobalLogWriter.get().println("Number format exception");
//                nResult = 0;
//            }
            // return nResult;
}

/**
 * create out of a given Postscript/PDF _sFile a list of JPEGs, one for every page
 * @param _sFile
 * @param _sAdditional
 * @param _aParam
 * @return the schema of the first jpeg name
 */
    String createJPEG(String _sFile, String _sAdditional, ParameterHelper _aParam)
        {
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
                GlobalLogWriter.println("File: '" + _sFile + "' doesn't exist.");
                return "";
            }
            String sFileDir = FileHelper.getPath(_sFile);
            String sBasename = FileHelper.getBasename(_sFile);
//            String sNameNoSuffix = FileHelper.getNameNoSuffix(sBasename);

            String sTmpDir = util.utils.getUsersTempDir();
            if (_aParam.getOutputPath() != null)
            {
                sTmpDir = _aParam.getOutputPath();
            }

            String sJPEGNameSchema = "";
            if (_sFile.toLowerCase().endsWith("ps") ||
                _sFile.toLowerCase().endsWith("prn") ||
                _sFile.toLowerCase().endsWith("pdf"))
            {
                // seems to be a Postscript of PDF file

                sJPEGNameSchema = createJPEGsFromPostscript(sTmpDir, sFileDir, sBasename, _aParam.getResolutionInDPI());
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
                // Office files will not handled here, we need a PS or PDF file
            }
            return sJPEGNameSchema;
        }

        private String getJPEGName(String _sOutputPath, String _sBasename, int _nResolutionInDPI, String _sGS_PageOutput)
        {
            // String fs = System.getProperty("file.separator");
            String sName = _sBasename + "_" + String.valueOf(_nResolutionInDPI) + "DPI_" + _sGS_PageOutput + ".jpg";
            String sJPEGName = FileHelper.appendPath(_sOutputPath, sName);
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

     private static final String m_sGS_PageOutput = "%04d";

     private String createJPEGsFromPostscript(String _sOutputPath, String _sSourcePath, String _sSourceFile, int _nResolutionInDPI)
        {
            FileHelper.makeDirectories("", _sOutputPath);

            // create a jpeg from original prn
            // String fs = System.getProperty("file.separator");

            String sJPEGNameSchema = getJPEGName(_sOutputPath, _sSourceFile, _nResolutionInDPI, m_sGS_PageOutput);
            String sPostscriptOrPDFFile = FileHelper.appendPath(_sSourcePath, _sSourceFile);
            String sGhostscriptEXE = "gs";
            if (OSHelper.isWindows())
            {
                sGhostscriptEXE = "gswin32c.exe";
                String sGhostscriptEXE2 = (String)param.get("gs.exe");
                if (sGhostscriptEXE2 != null)
                {
                    sGhostscriptEXE = sGhostscriptEXE2;
                }
                String sGhostscriptPath = (String)param.get("gs.path");
                if (sGhostscriptPath != null)
                {
                    sGhostscriptEXE = FileHelper.appendPath(sGhostscriptPath, sGhostscriptEXE);
                }
            }

//            String sCommand = sGhostscriptEXE + " -dNOPROMPT -dBATCH -sDEVICE=jpeg -r" + String.valueOf(_nResolutionInDPI) + " -dNOPAUSE -sOutputFile=" + StringHelper.doubleQuoteIfNeed(sJPGFilename) + " " + StringHelper.doubleQuoteIfNeed(sOriginalFile);
            String[] sCommandArray =
                {
                    sGhostscriptEXE,
                    "-dNOPROMPT",
                    "-dBATCH",
                    "-sDEVICE=jpeg",
                    "-r" + String.valueOf(_nResolutionInDPI),
                    "-dNOPAUSE",
                    "-sOutputFile=" + sJPEGNameSchema,
                    sPostscriptOrPDFFile



                };
            // System.out.println("Start Command array");
            // try
            // {
            //     Runtime.getRuntime().exec(sCommandArray);
            // } catch (Exception e) {
            //     System.out.println("FAILED");
            // }
            // System.out.println("done");

// TODO: gives ghostscript an error we can handle?
            ProcessHandler aHandler = new ProcessHandler(sCommandArray);
            boolean bBackValue = aHandler.executeSynchronously();
            assure("There seems to be a problem with ghostscript", bBackValue, true);
            int nExitCode = aHandler.getExitCode();
            // ArrayList m_aFileList = new ArrayList();

            // String sJPEGNameSchema = "";
            if (nExitCode == 0)
            {
                // TODO: return a real filename, due to the fact we don't know how much files are created, maybe better to return a list
                int nPages = countPages(sJPEGNameSchema);
                if (nPages == 0)
                {
                    // return only a valid schema name if there at least one page.
                    sJPEGNameSchema = "";
                    assure("Document '" + sPostscriptOrPDFFile + "' doesn't create pages.", false, true);
                }
            }
            else
            {
                assure("There seems to be a problem with ghostscript and '" + sPostscriptOrPDFFile + "' exit code: " + nExitCode, false, true);
                GlobalLogWriter.println("Warning: There seems to be a problem with '" + sGhostscriptEXE + "'...");
            }
            // String[] aList = new String[m_aFileList.size()];
            // aList = (String[])m_aFileList.toArray(aList);
            return sJPEGNameSchema; // sNewJPEGFilename;
        }

    public static String getFilenameForJPEGSchema(String _sJPEGSchema, int _nPage)
        {
            if (_nPage < 1 || _nPage > 9999)
            {
                return "";
            }
            final String sFilename = util.utils.replaceAll13(_sJPEGSchema, m_sGS_PageOutput, StringHelper.createValueString(_nPage, 4));
            return sFilename;
        }

        /**
         * Return the number of really exists JPEG files
         * Give a JPEG Schema which contains something like '%04d' which will be converted by a number between 0001 and 9999 always
         * with leading '0' (zero)
         * @param _sJPEGSchema
         * @return number of really existing pages of a given JPEG schema
         */
    public int countPages(String _sJPEGSchema)
    {
        int nPages = 0;
        if (_sJPEGSchema.length() > 0)
        {
            // TODO: if there doesn't exists a '%04d' in the schema we will return 9999 which is a little bit wrong here.
            for (int i=1;i<10000;i++)
            {
                String sNewJPEGFilename = getFilenameForJPEGSchema(_sJPEGSchema, i);
                if (FileHelper.exists(sNewJPEGFilename))
                {
                    // m_aFileList.add(sNewJPEGFilename); // as long as the files exist, fill the array
                    nPages ++;
                }
                else
                {
                    break;                             // stop file check
                }
            }
        }
        return nPages;
    }

//    public static void main(String [] _args)
//    {
//// DONE: give an index.ini file ok
////        String args[] = {
////            "-TimeOut", "3600000",
////            "-tb", "java_complex",
////            "-o", "graphical.JPEGCreator",
////            "-DOC_COMPARATOR_INPUT_PATH", "C:\\CWS\\temp\\output\\index.ini",
////            "-DOC_COMPARATOR_OUTPUT_PATH", "C:\\CWS\\temp\\output",
////            "-DOC_COMPARATOR_PRINT_MAX_PAGE", "9999",
////            "-DOC_COMPARATOR_GFX_OUTPUT_DPI_RESOLUTION", "180",
////            "-DOC_COMPARATOR_HTML_OUTPUT_PREFIX", "http://so-gfxcmp-lin.germany.sun.com/gfxcmp_ui/cw.php?inifile=",
//////            "-DOC_COMPARATOR_REFERENCE_CREATOR_TYPE", "PDF",      /* default: "OOo" */
//////            "-DOC_COMPARATOR_REFERENCE_CREATOR_TYPE", "msoffice", /* default: "OOo" */
//////            "-OFFICE_VIEWABLE", "false",
////            "-AppExecutionCommand", "\"C:/Programme/sun/staroffice 9/program/soffice.exe\"  -norestore -nocrashreport -accept=pipe,name=ll93751;urp;",
////            "-NoOffice"
////        };
//
//// Done: give a directory, where exist pdf/ps files ok.
//// Done: inputpath (given file) doesn't exists, ok.
//// Done: give a ps/pdf file. ok.
//
//        String args[] = {
//            "-TimeOut", "3600000",
//            "-tb", "java_complex",
//            "-o", "graphical.JPEGCreator",
//            "-DOC_COMPARATOR_INPUT_PATH", "C:\\CWS\\temp\\output\\Names7.odt.pdf",
//            "-DOC_COMPARATOR_OUTPUT_PATH", "C:\\CWS\\temp\\output",
//            "-DOC_COMPARATOR_PRINT_MAX_PAGE", "9999",
//            "-DOC_COMPARATOR_GFX_OUTPUT_DPI_RESOLUTION", "180",
//            "-DOC_COMPARATOR_HTML_OUTPUT_PREFIX", "http://so-gfxcmp-lin.germany.sun.com/gfxcmp_ui/cw.php?inifile=",
////            "-DOC_COMPARATOR_REFERENCE_CREATOR_TYPE", "PDF",      /* default: "OOo" */
////            "-DOC_COMPARATOR_REFERENCE_CREATOR_TYPE", "msoffice", /* default: "OOo" */
////            "-OFFICE_VIEWABLE", "false",
//            "-AppExecutionCommand", "\"C:/Programme/sun/staroffice 9/program/soffice.exe\"  -norestore -nocrashreport -accept=pipe,name=ll93751;urp;",
//            "-NoOffice"
//        };
//
//        org.openoffice.Runner.main(args);
//    }

}
