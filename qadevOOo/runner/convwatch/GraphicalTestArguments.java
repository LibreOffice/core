/*************************************************************************
 *
 *  $RCSfile: GraphicalTestArguments.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Date: 2004-12-10 16:57:26 $
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

import com.sun.star.lang.XMultiServiceFactory;
import lib.TestParameters;
import java.io.File;
import java.io.FileFilter;

/**
 * This class object is more a Helper or Controller.
 * It stores information like:
 * - How to create a document (with a OpenOffice.org method, or with MS Word, or with OpenOffice.org as pdf)
 * - some more infos for OpenOffice.org method
 *   - a service factory pointer
 *   - if hidden mode should use
 *   - target name
 *
 * - printer name
 *
 * - how to handle .xml files, which in Microsoft could be Excel or Word documents
 *
 * HOWTO USE:
 * For OOo,
 *   create an GraphicalTestArguments with a set of TestParameters
 *    GraphicalTestArguments a = new GraphicalTestArguments(params);
 *
 *  If you wish to use pdf export instead of normal printer output, set also the reference type to 'pdf'
 *    a.setReferenceType("pdf");
 *
 *
 * For MS Office:
 *   create an GraphicalTestArguments and set the reference type to 'msoffice'
 *    GraphicalTestArguments a = new GraphicalTestArguments(params);
 *    a.setReferenceType("msoffice");
 *
 * within windows it's better to set also a printer name so it's simply possible to use for normal work the default printer
 * and for such tests with ConvWatch a extra printer.
 *    a.setPrinterName("CrossOffice Printer");
 *
 */

public class GraphicalTestArguments
{
    /**
    2DO:
    Possible reference types are currently
    // ooo
    // pdf
    // msoffice
    */
    String m_sReferenceType = "OOo";

    String m_sTargetFrameName = "_blank";

    String m_sPrinterName = null;

    // Hidden = true hiddes a used OpenOffice.org, all code is executed in the background
    // This parameter is not used for RefType: msoffice
    boolean m_bHidden = true;

    String m_sDefaultXMLFormatApplication = null;

    boolean m_bIncludeSubdirectories;

    TestParameters m_aCurrentParams;

    int m_nMaxPages = 0;                           // default is 0 (print all pages)
    String m_sOnlyPage = "";                       // default is "", there is no page which we want to print only.

    int m_nResolutionInDPI = 0;

    boolean m_bStoreFile = true;
    boolean m_bResuseOffice = false;

    // CONSTRUCTOR
    private GraphicalTestArguments(){}

    public GraphicalTestArguments(TestParameters param)
        {
            m_aCurrentParams = param;
            // collect interesting information from the ComplexTestCase
            // ....

            // REFERENCE_TYPE ----------
            String sReferenceType = (String)param.get( PropertyName.DOC_COMPARATOR_REFERENCE_TYPE );
            if (sReferenceType == null || sReferenceType.length() == 0)
            {
            }
            else
            {
                // log.println("found REFERENCE_TYPE " + sReferenceType );
                setReferenceType(sReferenceType);
            }

            // PRINTER_NAME ----------
            String sPrinterName = (String)param.get( PropertyName.DOC_COMPARATOR_PRINTER_NAME );
            if (sPrinterName == null || sPrinterName.length() == 0)
            {
            }
            else
            {
                // log.println("found PRINTER_NAME " + sPrinterName );
                setPrinterName(sPrinterName);
            }
            // DEFAULT_XML_FORMAT_APP ------
            String sDefaultXMLFormatApp = (String)param.get( PropertyName.DOC_COMPARATOR_DEFAULT_XML_FORMAT_APP );
            if (sDefaultXMLFormatApp == null || sDefaultXMLFormatApp.length() == 0)
            {
            }
            else
            {
                setDefaultXMLFormatApp(sDefaultXMLFormatApp);
            }

            m_bIncludeSubdirectories = true;
            String sRECURSIVE = (String)param.get( PropertyName.DOC_COMPARATOR_INCLUDE_SUBDIRS );
// TODO: I need to get the boolean value with get("name") because, if it is not given getBool() returns
//       with a default of 'false' which is not very helpful if the default should be 'true'
//       maybe a getBoolean("name", true) could be a better choise.
            if (sRECURSIVE == null)
            {
                sRECURSIVE = "true";
            }
            if (sRECURSIVE.toLowerCase().equals("no") ||
                sRECURSIVE.toLowerCase().equals("false"))
            {
                m_bIncludeSubdirectories = false;
            }

            // ----------------------------------------
            m_nMaxPages = param.getInt( PropertyName.DOC_COMPARATOR_PRINT_MAX_PAGE );
            m_sOnlyPage = (String)param.get(PropertyName.DOC_COMPARATOR_PRINT_ONLY_PAGE);

            m_nResolutionInDPI = param.getInt( PropertyName.DOC_COMPARATOR_GFX_OUTPUT_DPI_RESOLUTION );
            if (m_nResolutionInDPI == 0)
            {
                // 212 DPI is 1754 x 2474 pixel for DIN A4
                m_nResolutionInDPI = 212;
            }

            // ----------------------------------------
            String sImportFilterName = (String)param.get(PropertyName.DOC_CONVERTER_IMPORT_FILTER_NAME);
            if (sImportFilterName != null && sImportFilterName.length() > 0)
            {
                // System.out.println("found " + PropertyName.DOC_CONVERTER_IMPORT_FILTER_NAME + " " + sImportFilterName );
                m_sImportFilterName = sImportFilterName;
            }
            // ----------------------------------------
            String sExportFilterName = (String)param.get(PropertyName.DOC_CONVERTER_EXPORT_FILTER_NAME);
            if (sExportFilterName != null && sExportFilterName.length() > 0)
            {
                // System.out.println("found " + PropertyName.DOC_CONVERTER_EXPORT_FILTER_NAME + " " + sExportFilterName );
                m_sExportFilterName = sExportFilterName;
            }
            // ----------------------------------------
            String sOfficeProgram = (String)param.get(PropertyName.DOC_CONVERTER_OFFICE_PROGRAM);
            if (sOfficeProgram != null && sOfficeProgram.length() > 0)
            {
                m_sOfficeProgram = sOfficeProgram;
            }
            // ----------------------------------------
            String sREUSE_OFFICE = (String)param.get( PropertyName.DOC_CONVERTER_REUSE_OFFICE);
            if (sREUSE_OFFICE == null)
            {
                sREUSE_OFFICE = "false";
            }
            if (sREUSE_OFFICE.toLowerCase().equals("yes") ||
                sREUSE_OFFICE.toLowerCase().equals("true"))
            {
                m_bResuseOffice = true;
            }
            else
            {
                m_bResuseOffice = false;
            }


            String sHTMLOutputPrefix = (String)param.get( PropertyName.DOC_COMPARATOR_HTML_OUTPUT_PREFIX);
            if (sHTMLOutputPrefix == null)
            {
                m_sHTMLOutputPrefix = "";
            }
            else
            {
                m_sHTMLOutputPrefix = sHTMLOutputPrefix;
            }

            String sWithBorderMove = (String)param.get( PropertyName.DOC_COMPARATOR_GFXCMP_WITH_BORDERMOVE);
            if (sWithBorderMove == null)
            {
                sWithBorderMove = "";
            }
            if (sWithBorderMove.toLowerCase().equals("yes") ||
                sWithBorderMove.toLowerCase().equals("true"))
            {
                m_bWithBorderMove = true;
            }
            else
            {
                m_bWithBorderMove = false;
            }
        }

    /*
    public GraphicalTestArguments(TestParameters param, Log xxx)
    {
        // collect interesting information from the ComplexTestCase
        // ....
    }
    */

    // set methods
    public void setReferenceType(String _sType)
        {
            // special casse, null is not allowed, set to default.
            if (_sType == null)
            {
                m_sReferenceType = "OOo";
            }
            else
            {
                m_sReferenceType = _sType;
            }
        }
    public void setTargetFrameName(String _sTargetFrameName) {m_sTargetFrameName = _sTargetFrameName;}
    public void setPrinterName(String _sName) {m_sPrinterName = _sName;}
    public void setHidden() { m_bHidden = true;}
    public void setViewable() {m_bHidden = false;}
    public void setDefaultXMLFormatApp(String _sNameOfApp) {m_sDefaultXMLFormatApplication = _sNameOfApp;}

    // get methods
    public XMultiServiceFactory getMultiServiceFactory()
        {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)m_aCurrentParams.getMSF();

            // check if MultiServiceFactory is given
            if (getReferenceType().toLowerCase().equals("pdf") ||
                getReferenceType().toLowerCase().equals("ooo"))
            {
                if (xMSF == null)
                {
                    System.out.println("ERROR! MultiServiceFactory not given.");
                }
            }
            return xMSF;
        }

    public String getReferenceType() {return m_sReferenceType;}
    public String getTargetFrameName() {return m_sTargetFrameName;}
    public String getPrinterName() {return m_sPrinterName;}
    public boolean isHidden() {return m_bHidden;}
    public String getDefaultXMLFormatApp() {return m_sDefaultXMLFormatApplication;}

    /**
     * Within the directory run through, it's possible to say which file extension types should not
     * consider like '*.prn' because it's not a document.
     *
     * @return a FileFilter function
     */
    public FileFilter getFileFilter()
        {
            FileFilter aFileFilter = new FileFilter()
                {
                    public boolean accept( File pathname )
                        {
                            if (pathname.getName().endsWith(".prn"))
                            {
                                return false;
                            }
                            // This type of document no one would like to load.
                            if (pathname.getName().endsWith(".zip"))
                            {
                                return false;
                            }
                            return true;
                        }
                };
            return aFileFilter;
        }

    /**
     * @return true, if subdirectories should run through
     */
    public boolean includeSubDirectories() {return m_bIncludeSubdirectories;}

    /**
     * @return the number of pages to be print
     */
    public int getMaxPages() {return m_nMaxPages;}

    /**
     * @return as string, which pages should be print, e.g. '1-4;6' here, page 1 to 4 and page 6.
     */
    public String getOnlyPages()
    {
        if (m_sOnlyPage == null)
        {
            return "";
        }
        return m_sOnlyPage;
    }

    /**
     * @return true, if there should not print all pages at all, use getMaxPages() and or getOnlyPages() to get which pages to print
     */
    public boolean printAllPages()
        {
            if ( (getMaxPages() > 0) ||
                 (getOnlyPages().length() != 0))
            {
                return false;
            }
            return true;
        }

    /**
     * @return integer value, which contain resolution in DPI.
     */
    public int getResolutionInDPI() {return m_nResolutionInDPI;}

    /**
     * @return the INPUT_PATH out of the TestParameters
     */
    public String getInputPath()
        {
            String sInputPath;
            sInputPath = (String)m_aCurrentParams.get(PropertyName.DOC_COMPARATOR_INPUT_PATH);
            return sInputPath;
        }
    /**
     * @return the OUTPUT_PATH out of the TestParameters
     */
    public String getOutputPath()
        {
            String sOutputPath;
            sOutputPath = (String)m_aCurrentParams.get(PropertyName.DOC_COMPARATOR_OUTPUT_PATH);
            return sOutputPath;
        }
    /**
     * @return the REFERENCE_PATH out of the TestParameters
     */
    public String getReferencePath()
        {
            String sReferencePath;
            sReferencePath = (String)m_aCurrentParams.get(PropertyName.DOC_COMPARATOR_REFERENCE_PATH);
            return sReferencePath;
        }
    /**
     * @return the DIFF_PATH out of the TestParameters
     */
    public String getDiffPath()
        {
            String sDiffPath;
            sDiffPath = (String)m_aCurrentParams.get(PropertyName.DOC_COMPARATOR_DIFF_PATH);
            return sDiffPath;
        }

    public boolean getOverwrite()
        {
            boolean bOverwrite = m_aCurrentParams.getBool( PropertyName.DOC_COMPARATOR_OVERWRITE_REFERENCE);
            return bOverwrite;
        }
    public String getReferenceInputPath()
        {
            String sReferenceInputPath;
            sReferenceInputPath = (String)m_aCurrentParams.get(PropertyName.DOC_COMPARATOR_REFERENCE_INPUT_PATH);
            return sReferenceInputPath;
        }

    /**
     * Helper function to get the buildid of the current used OpenOffice.org
     * out of the AppExecutionCommand the build ID
     */
    public String getBuildID()
        {
            String sAPP = (String)m_aCurrentParams.get(util.PropertyName.APP_EXECUTION_COMMAND);
            // return getBuildID(sAPP);
//  TODO: here we need the getBuildID(string) method
            String sBuildID = convwatch.BuildID.getBuildID(sAPP);
            return sBuildID;
        }

        // Handle for Reference Build ID, is set in ConvWatch.createPostscriptStartCheck()
    private String m_sRefBuildID;

    public void setRefBuildID(String _sRef)
        {
            m_sRefBuildID = _sRef;
        }
    public String getRefBuildID()
        {
            return m_sRefBuildID;
        }

    public void disallowStore()
        {
            m_bStoreFile = false;
        }
    public void allowStore()
        {
            m_bStoreFile = true;
        }
    public boolean isStoreAllowed()
        {
            return m_bStoreFile;
        }


    // get/set for FilterName
    // get the right Filtername (internal Name) from
    // http://framework.openoffice.org/files/documents/25/897/filter_description.html

    String m_sImportFilterName = "";
    String m_sExportFilterName = "";
    public void setImportFilterName(String _sImportFilterName)
        {
            m_sImportFilterName = _sImportFilterName;
        }
    public String getImportFilterName()
        {
            return m_sImportFilterName;
        }
    public void setExportFilterName(String _sExportFilterName)
        {
            m_sExportFilterName = _sExportFilterName;
        }
    public String getExportFilterName()
        {
            return m_sExportFilterName;
        }

    String m_sOfficeProgram = "";
    public void setOfficeProgram(String _sName)
        {
            m_sOfficeProgram = _sName;
        }
    public String getOfficeProgram()
        {
            return m_sOfficeProgram;
        }

    public boolean restartOffice()
        {
            if (m_bResuseOffice == false)
            {
                return true;
            }
            return false;
        }

    String m_sHTMLOutputPrefix = "";
    public String getHTMLOutputPrefix()
        {
            return m_sHTMLOutputPrefix;
        }

    boolean m_bWithBorderMove = false;
    public boolean isBorderMove()
        {
            return m_bWithBorderMove;
        }

}


/*
public class MSGraphicalTestArguments extends GraphicalTestArguments
{
    MSGraphicalTestArguments()
        {
            setReferenceType("msoffice");
        }
}

public class OOoGraphicalTestArguments extends GraphicalTestArguments
{
    OOoGraphicalTestArguments(XMultiServiceFactory _aFactory)
        {
            setMultiServiceFactory(_aFactory);
        }
}
*/
