/*************************************************************************
 *
 *  $RCSfile: GraphicalDifferenceCheck.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Date: 2004-12-10 16:57:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package convwatch;

import convwatch.GraphicalTestArguments;
import convwatch.OfficePrint;
import convwatch.ConvWatch;
import convwatch.ConvWatchCancelException;
import java.io.File;

import helper.URLHelper;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XStorable;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;

public class GraphicalDifferenceCheck
{
    /**
     * Creates references form documents used by the graphical difference check
     *
     * @param _sInputPath       the original document path
     * @param _sReferencePath   the directory where the document will print as file or export as pdf.
     *
     * @throws  ConvWatchException if there are problems, see message
     *
     * Stops rest, if one creation of reference fails.
     */
    public static void createReferences(String _sInputPath, String _sReferencePath, GraphicalTestArguments _aGTA) throws ConvWatchException
        {
//!
//            System.out.println("createReferences() InputPath: " + _sInputPath + " refpath: " + _sReferencePath);
            File aInputPath = new File(_sInputPath);

//            System.out.println("Inputpath in file: " + aInputPath.getAbsolutePath());
//!
//             if (aInputPath.exists())
//             {
//                 System.out.println("Inputpath exists");
//             }
//             else
//             {
//                 System.out.println("Inputpath doesn't exists");
//                 return;
//             }


            if (aInputPath.isDirectory())
            {
                String fs = System.getProperty("file.separator");

                String sRemovePath = aInputPath.getAbsolutePath();
                // a whole directory

                Object[] aList = DirectoryHelper.traverse(_sInputPath, _aGTA.getFileFilter(), _aGTA.includeSubDirectories());
                for (int i=0;i<aList.length;i++)
                {
                    String sEntry = (String)aList[i];
                    String sNewReferencePath = _sReferencePath + fs + FileHelper.removeFirstDirectorysAndBasenameFrom(sEntry, _sInputPath);
                    createOneReferenceFile(sEntry, sNewReferencePath, _aGTA);
                }
            }
            else
            {
//!
                // System.out.println("No directory.");
                createOneReferenceFile(_sInputPath, _sReferencePath, _aGTA);
            }
        }


    /**
     * Creates a reference for a single document used by the graphical difference check
     *
     * @param _sInputFile       the original document
     * @param _sReferencePath   the directory where the document will print as file or export as pdf.
     *
     * @throws  ConvWatchException if the are problems, see containing message
     */
    public static boolean createOneReferenceFile(String _sInputFile, String _sReferencePath, GraphicalTestArguments _aGTA) throws ConvWatchException
        {
            return OfficePrint.buildReference(_aGTA, _sReferencePath, _sInputFile);
        }

    /**
     * Check if a reference exist
     *
     * @param _sInputFile       the original document
     * @param _sReferencePath   the directory where the document will print as file or export as pdf.
     *
     * @throws  ConvWatchException if the are problems, see containing message
     */
    public static boolean isReferenceExists(String _sInputFile, String _sReferencePath, GraphicalTestArguments _aGTA)
        {
            return OfficePrint.isReferenceExists(_aGTA, _sReferencePath, _sInputFile);
        }


    /**
     * Used for the comparance of graphical differences.
     * Method compares one document (_sInputFile) with an older document of the same name in the provided directory (_sReferencePath).
     *
     * @param _sInputPath       the original document path
     * @param _sOutputPath      path where the same directory structure of the given input path will create. All the result documents
     *                          needed very much disk space (up to 10MB per page).
     *                          The path _sOutputPath must be writeable.
     * @param _sReferencePath   the directory where the document will print as file or export as pdf.
     * @param _GTA              Helper class for lot of parameter to control the office.
     *
     * Disadvantage: stops rest if one test file has a problem.
     */
    public static boolean check(String _sInputPath, String _sOutputPath, String _sReferencePath, GraphicalTestArguments _aGTA ) throws ConvWatchException
        {
            return check(_sInputPath, _sOutputPath, _sReferencePath, null, _aGTA);
        }

    /**
     * Used for the comparance of graphical differences.
     * Method compares one document (_sInputFile) with an older document of the same name in the provided directory (_sReferencePath).
     *
     * @param _sInputPath       the original document path
     * @param _sReferencePath   the directory where the document will print as file or export as pdf.
     * @param _sOutputPath      path where the same directory structure of the given input path will create. All the result documents
     *                          needed very much disk space (up to 10MB per page).
     *                          The path _sOutputPath must be writeable.
     * @param _sDiffPath        Path to older differences.
     * @param _GTA              Helper class for lot of parameter to control the office.
     *
     *
     * Stops all, if one creation of reference fails
     */
    public static boolean check(String _sInputPath, String _sOutputPath, String _sReferencePath, String _sDiffPath, GraphicalTestArguments _aGTA ) throws ConvWatchException
        {
            boolean bOk = true;

            File aInputPath = new File(_sInputPath);
            if (aInputPath.isDirectory())
            {
                String fs = System.getProperty("file.separator");
                // a whole directory
                Object[] aList = DirectoryHelper.traverse(_sInputPath, _aGTA.getFileFilter(), _aGTA.includeSubDirectories());
                if (aList.length != 0)
                {
                    for (int i=0;i<aList.length;i++)
                    {
                        String sEntry = (String)aList[i];
                        String sNewSubDir = FileHelper.removeFirstDirectorysAndBasenameFrom(sEntry, _sInputPath);
                        String sNewReferencePath = _sReferencePath;
                        String sNewOutputPath = _sOutputPath;
                        String sNewDiffPath = _sDiffPath;
                        if (sNewSubDir.length() > 0)
                        {
                            if (sNewReferencePath != null)
                            {
                                sNewReferencePath = sNewReferencePath + fs + sNewSubDir;
                            }

                            sNewOutputPath = sNewOutputPath + fs + sNewSubDir;
                            if (sNewDiffPath != null)
                            {
                                sNewDiffPath = sNewDiffPath + fs + sNewSubDir;
                            }
                        }
                        bOk &= checkOneFile(sEntry, sNewOutputPath, sNewReferencePath, sNewDiffPath, _aGTA);
                    }
                }
            }
            else
            {
                bOk = GraphicalDifferenceCheck.checkOneFile(_sInputPath, _sOutputPath, _sReferencePath, _sDiffPath, _aGTA);
            }
            return bOk;
        }

    /**
     * Used for the comparance of graphical differences.
     * Method compares one document (_sInputFile) with an older document of the same name in the provided directory (_sReferencePath).
     *
     * The path _sOutputPath must be writeable
     */
    public static boolean checkOneFile(String _sInputFile, String _sOutputPath, String _sReferencePath, GraphicalTestArguments _aGTA)  throws ConvWatchException
    {
        return checkOneFile( _sInputFile, _sOutputPath, _sReferencePath, null, _aGTA);
    }


    /**
     * Used for the comparance of graphical differences.
     * Method compares one document (_sInputFile) with an older document of the same name in the provided directory (_sReferencePath).
     *
     * For scenarios, where a difference is known and further changes are of interest, differences itself can be compared.
     * This functionality is provided by the difference path parameter (_sDiffPath). If set, the difference of the current comparance (between input and reference),
     * will be compared with the (same named) difference document from a earlier comparance.
     *
     * The path _sOutputPath must be writeable
     */
    public static boolean checkOneFile(String _sInputFile, String _sOutputPath, String _sReferencePath, String _sDiffPath, GraphicalTestArguments _aGTA ) throws ConvWatchException
        {
            boolean bOk = false;
            if (_sDiffPath != null)
            {
                // check with an old diff
                bOk = convwatch.ConvWatch.checkDiffDiff(_aGTA, _sOutputPath, _sInputFile, _sReferencePath, _sDiffPath);
            }
            else
            {
                // one file
                bOk = convwatch.ConvWatch.check(_aGTA, _sOutputPath, _sInputFile, _sReferencePath);
            }
            return bOk;
        }

    /**
     * Instead of providing a saved document for graphical comparance a StarOffice xComponent
     * will be saved and afterwards compared.
     *
     * @param xComponent        the test document to be compared as StarOffice component
     * @param _sOutputPath      Path where test results are supposed to been saved. The path _sOutputPath must be writeable.
     *                          These documents need sufficient disk space (up to 10MB per page).
     *                          A directory structure will be created, which is a mirrored from input path.
     *
     * @param resultDocName     Name by which the xComponent shall be saved as OpenOffice.org XML document.
     *                          If provided without suffix, the suffix will be derived from the export filter.
     * @param _sReferencePath   the directory where the document will print as file or export as pdf.
     * @param _GTA              Helper class for lot of parameter to control the office.
     */
    public static boolean checkOneFile(XComponent xComponent, String _sOutputPath, String _resultDocName, String _sReferencePath, GraphicalTestArguments _aGTA ) throws ConvWatchException
        {
            // one file
            String sInputFile;
            sInputFile = createInputFile(xComponent, _sOutputPath, _resultDocName);
            sInputFile = FileHelper.getSystemPathFromFileURL(sInputFile);
            return convwatch.ConvWatch.check(_aGTA, _sOutputPath, sInputFile, _sReferencePath);
        }


// LLA: old!     /**
// LLA: old!      * Returns 'true' if a reference document on the specific output path exists.
// LLA: old!      * The name of the document is corresponding to the input document, which can be
// LLA: old!      * provided by a single name or path.
// LLA: old!      *
// LLA: old!      * @param inputPath       the original document name (possibly including path)
// LLA: old!      * @param referencePath   the directory where the reference document will be stored
// LLA: old!
// LLA: old!      */
// LLA: old!     public static boolean isReferencExistent(String inputDocumentPath, String referencePath)
// LLA: old!     {
// LLA: old!        // isolate the document name
// LLA: old!         if(inputDocumentPath.indexOf(File.separator) != -1)
// LLA: old!             inputDocumentPath = inputDocumentPath.substring(inputDocumentPath.lastIndexOf(File.separator) + 1, inputDocumentPath.length());
// LLA: old!
// LLA: old!        // exchange any arbitray suffix against the refence suffix (.prn)
// LLA: old!         if(inputDocumentPath.indexOf('.') != -1)
// LLA: old!             inputDocumentPath = inputDocumentPath.substring(0, inputDocumentPath.lastIndexOf('.'));
// LLA: old!         inputDocumentPath = inputDocumentPath + ".prn";
// LLA: old! System.out.println("GraphicalDifference CheckReferenceDocument: " + inputDocumentPath);
// LLA: old!
// LLA: old!         File refFile = new File(referencePath + inputDocumentPath);
// LLA: old!         if(refFile.exists()){
// LLA: old!             return true;
// LLA: old!         }else
// LLA: old!             return false;
// LLA: old!    }


    private static String createInputFile(XComponent xComponent, String _sOutputPath, String resultDocName)
        throws ConvWatchCancelException
    {

        // find the adequate XML StarOffice output filter to save the document and adequate suffix
        StringBuffer suffix = new StringBuffer();
        String exportFilter = getXMLOutputFilterforXComponent(xComponent, suffix);
        if(resultDocName == null)
            resultDocName = "OOoTestDocument";
        if(resultDocName.indexOf('.') == -1)
            resultDocName = suffix.insert(0, resultDocName).toString();

        // create a result URL for storing the office document
        String resultURL = URLHelper.getFileURLFromSystemPath(ensureEndingFileSep(_sOutputPath) + resultDocName);

        XStorable xStorable = null;
        xStorable  = (com.sun.star.frame.XStorable)UnoRuntime.queryInterface(com.sun.star.frame.XStorable.class, xComponent);
        if(xStorable == null)
        {
            throw new ConvWatchCancelException("com.sun.star.frame.XStorable could not be instantiated from the office.");
        }

        PropertyValue pvFilterName = new PropertyValue("FilterName", -1, exportFilter, com.sun.star.beans.PropertyState.getDefault());
        PropertyValue pvOverwrite = new PropertyValue("Overwrite", -1, new Boolean(true), com.sun.star.beans.PropertyState.getDefault());

        try
        {
            xStorable.storeAsURL(resultURL, new PropertyValue[]{pvFilterName, pvOverwrite});
        }
        catch (com.sun.star.io.IOException e)
        {
            // wrap IOException
            throw new ConvWatchCancelException("Wrap IOException caught, " + e.getMessage());
        }

        System.out.println("Saving XComponent as " + resultURL);

        return resultURL;
    }


    private static String getXMLOutputFilterforXComponent(XComponent xComponent, StringBuffer suffix){
        XServiceInfo xSI = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, xComponent);
        if (xSI.supportsService("com.sun.star.text.TextDocument")){
            resetBuffer(suffix, ".sxw");
            return "swriter: StarOffice XML (Writer)";
        }else if (xSI.supportsService("com.sun.star.sheet.SpreadsheetDocument")){
            resetBuffer(suffix, ".sxc");
            return "scalc: StarOffice XML (Calc)";
        }else if (xSI.supportsService("com.sun.star.presentation.PresentationDocument")){
            resetBuffer(suffix, ".sxi");
            return "simpress: StarOffice XML (Impress)";
        }else if(xSI.supportsService("com.sun.star.drawing.DrawingDocument")){
            resetBuffer(suffix, ".sxd");
            return "sdraw: StarOffice XML (Draw)";
        }else if (xSI.supportsService("com.sun.star.formula.FormulaProperties")){
            resetBuffer(suffix, ".sxm");
            return "smath: StarOffice XML (Math)";
        }
        return null;
    }

    private static StringBuffer resetBuffer(StringBuffer sb, String suffix)
        {
            if(sb != null)
            {
                sb.replace(0, sb.length(), suffix);
            }
            return sb;
        }

    private static String ensureEndingFileSep(String s)
        {
        if(s != null && !s.equals("") && !s.endsWith(File.separator))
            {
                s = s.trim() + File.separator;
            }
            else
            {
                if(s == null)
                {
                    s = "";
                }
            }

        return s;
    }


}
