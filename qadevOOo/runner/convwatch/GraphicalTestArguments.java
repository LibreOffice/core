/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GraphicalTestArguments.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 17:41:35 $
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

import com.sun.star.lang.XMultiServiceFactory;
import lib.TestParameters;
import java.io.File;
import java.io.FileFilter;

import com.sun.star.container.XNameAccess;
import com.sun.star.uno.UnoRuntime;

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

    boolean m_bDebugMode = false;

    String m_sLeaveOutNames = null;

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

                if (sImportFilterName.toLowerCase().equals("help"))
                {
                    showInternalFilterName(sImportFilterName, getMultiServiceFactory() );
                    System.out.println("Must quit.");
                }
            }
            // ----------------------------------------
            String sExportFilterName = (String)param.get(PropertyName.DOC_CONVERTER_EXPORT_FILTER_NAME);
            if (sExportFilterName != null && sExportFilterName.length() > 0)
            {
                // System.out.println("found " + PropertyName.DOC_CONVERTER_EXPORT_FILTER_NAME + " " + sExportFilterName );
                m_sExportFilterName = sExportFilterName;
                if (sExportFilterName.toLowerCase().equals("help"))
                {
                    showInternalFilterName(sExportFilterName, getMultiServiceFactory() );
                    System.out.println("Must quit.");
                }
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

            String sLeaveOutNames = (String)param.get(PropertyName.DOC_COMPARATOR_LEAVE_OUT_FILES);
            if (sLeaveOutNames != null)
            {
                m_sLeaveOutNames = sLeaveOutNames;
            }
        }

    public boolean checkIfUsable(String _sName)
        {
            // @todo
            // check if the name is in the leave out list and then return 'false'
            if (_sName.toLowerCase().endsWith(".jpg") ||
                _sName.toLowerCase().endsWith(".png") ||
                _sName.toLowerCase().endsWith(".gif") ||
                _sName.toLowerCase().endsWith(".bmp") ||
                _sName.toLowerCase().endsWith(".prn") ||
                _sName.toLowerCase().endsWith(".ps"))
            {
                return false;
            }

            return true;
        }

    static void showInternalFilterName(String _sFilterName, XMultiServiceFactory _xMSF)
        {
            if (_sFilterName.length() == 0)
            {
                // System.out.println("No FilterName set.");
                return;
            }

            if (_xMSF == null)
            {
                System.out.println("MultiServiceFactory not set.");
                return;
            }
            // XFilterFactory aFilterFactory = null;
            Object aObj = null;
            try
            {
                aObj = _xMSF.createInstance("com.sun.star.document.FilterFactory");
            }
            catch(com.sun.star.uno.Exception e)
            {
                System.out.println("Can't get com.sun.star.document.FilterFactory.");
                return;
            }
            if (aObj != null)
            {
                XNameAccess aNameAccess = (XNameAccess)UnoRuntime.queryInterface(XNameAccess.class, aObj);
                if (aNameAccess != null)
                {

                    if (_sFilterName.toLowerCase().equals("help"))
                    {
                        System.out.println("Show all possible ElementNames from current version." );
                        String[] aElementNames = aNameAccess.getElementNames();
                        for (int i = 0; i<aElementNames.length; i++)
                        {
                            System.out.println(aElementNames[i]);
                        }
                    }
                }
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
                            // just a hack
                            if (pathname.getName().endsWith("_"))
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

    public static void checkIfMSWindowsConformPath(String _sPath)
        {
            if (_sPath != null  && _sPath.length() > 1)
            {
                if (_sPath.charAt(1) == ':')
                {
                    if (_sPath.charAt(2) != '\\')
                    {
                        System.out.println("This is not a Microsoft Windows conform path: '" + _sPath + "' please fix.");
                        System.exit(1);
                    }
                }
            }
        }


    /**
     * @return the INPUT_PATH out of the TestParameters
     */
    public String getInputPath()
        {
            String sInputPath;
            sInputPath = (String)m_aCurrentParams.get(PropertyName.DOC_COMPARATOR_INPUT_PATH);
            checkIfMSWindowsConformPath(sInputPath);
            return sInputPath;
        }
    /**
     * @return the OUTPUT_PATH out of the TestParameters
     */
    public String getOutputPath()
        {
            String sOutputPath;
            sOutputPath = (String)m_aCurrentParams.get(PropertyName.DOC_COMPARATOR_OUTPUT_PATH);
            checkIfMSWindowsConformPath(sOutputPath);
            return sOutputPath;
        }
    /**
     * @return the REFERENCE_PATH out of the TestParameters
     */
    public String getReferencePath()
        {
            String sReferencePath;
            sReferencePath = (String)m_aCurrentParams.get(PropertyName.DOC_COMPARATOR_REFERENCE_PATH);
            checkIfMSWindowsConformPath(sReferencePath);
            return sReferencePath;
        }
    /**
     * @return the DIFF_PATH out of the TestParameters
     */
    public String getDiffPath()
        {
            String sDiffPath;
            sDiffPath = (String)m_aCurrentParams.get(PropertyName.DOC_COMPARATOR_DIFF_PATH);
            checkIfMSWindowsConformPath(sDiffPath);
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

    public boolean shouldOfficeStart()
        {
            String sNoOffice = (String)m_aCurrentParams.get( "NoOffice" );
            if (sNoOffice != null)
            {
                if (sNoOffice.toLowerCase().startsWith("t") || sNoOffice.toLowerCase().startsWith("y"))
                {
                    return false;
                }
            }
            return true;
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


    /*
      helper class for performance analyser features
     */
    PerformanceContainer m_aPerformanceContainer = null;
    public PerformanceContainer getPerformance()
        {
            if (m_aPerformanceContainer == null)
            {
                m_aPerformanceContainer = new PerformanceContainer();
            }
            return m_aPerformanceContainer;
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
