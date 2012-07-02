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

import com.sun.star.lang.XMultiServiceFactory;
import lib.TestParameters;
import java.io.File;

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

    String m_sDistinct = null;

    boolean m_bCreateDefaultReference = false;

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
                    GlobalLogWriter.get().println("Must quit.");
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
                    GlobalLogWriter.get().println("Must quit.");
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
                // m_tWithBorderMove = TriState.UNSET;
                m_tWithBorderMove = TriState.FALSE;
            }
            if (sWithBorderMove.toLowerCase().equals("yes") ||
                sWithBorderMove.toLowerCase().equals("true"))
            {
                m_tWithBorderMove = TriState.TRUE;
            }
            else if (sWithBorderMove.toLowerCase().equals("no") ||
                     sWithBorderMove.toLowerCase().equals("false"))
            {
                m_tWithBorderMove = TriState.FALSE;
            }
            else
            {
                m_tWithBorderMove = TriState.FALSE;
                // m_tWithBorderMove = TriState.UNSET;
            }

            String sLeaveOutNames = (String)param.get(PropertyName.DOC_COMPARATOR_LEAVE_OUT_FILES);
            if (sLeaveOutNames != null)
            {
                m_sLeaveOutNames = sLeaveOutNames;
            }

            String sDBInfoString = (String)param.get(PropertyName.DOC_COMPARATOR_DB_INFO_STRING);
            if (sDBInfoString != null)
            {
                m_sDBInfoString = sDBInfoString;
            }

            // DISTINCT ----------
            String sDistinct = (String)param.get( "DISTINCT" );
            if (sDistinct == null || sDistinct.length() == 0)
            {
                sDistinct = "";
            }
            else
            {
                m_sDistinct = sDistinct;
            }
            // HIDDEN
            String sOfficeViewable = (String)param.get(PropertyName.OFFICE_VIEWABLE);
            if (sOfficeViewable != null)
            {
                if (sOfficeViewable.toLowerCase().equals("yes") ||
                    sOfficeViewable.toLowerCase().equals("true"))
                {
                    setViewable();
                }
                else
                {
                    setHidden();
                }
            }
            // CREATE_DEFAULT
            String sCreateDefault = (String)param.get(PropertyName.CREATE_DEFAULT);
            if (sCreateDefault != null)
            {
                if (sCreateDefault.toLowerCase().equals("yes") ||
                    sCreateDefault.toLowerCase().equals("true"))
                {
                    m_bCreateDefaultReference = true;
                }
                else
                {
                    m_bCreateDefaultReference = false;
                }
            }

        }

    public boolean checkIfUsableDocumentType(String _sName)
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
                GlobalLogWriter.get().println("MultiServiceFactory not set.");
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
                GlobalLogWriter.get().println("Can't get com.sun.star.document.FilterFactory.");
                return;
            }
            if (aObj != null)
            {
                XNameAccess aNameAccess = UnoRuntime.queryInterface(XNameAccess.class, aObj);
                if (aNameAccess != null)
                {

                    if (_sFilterName.toLowerCase().equals("help"))
                    {
                        GlobalLogWriter.get().println("Show all possible ElementNames from current version." );
                        String[] aElementNames = aNameAccess.getElementNames();
                        for (int i = 0; i<aElementNames.length; i++)
                        {
                            GlobalLogWriter.get().println(aElementNames[i]);
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
                    GlobalLogWriter.get().println("ERROR! MultiServiceFactory not given.");
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
                        GlobalLogWriter.get().println("This is not a Microsoft Windows conform path: '" + _sPath + "' please fix.");
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
    public boolean createDefaultReference()
        {
            return m_bCreateDefaultReference;
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

    TriState m_tWithBorderMove = TriState.UNSET;
    // public TriState isBorderMove()
    //     {
    //         return m_tWithBorderMove;
    //     }
    public TriState getBorderMove()
        {
            return m_tWithBorderMove;
        }
    public void setBorderMove(TriState _tBorderMove)
        {
            m_tWithBorderMove = _tBorderMove;
        }

    String m_sDocumentType = "";
    public void setDocumentType(String _sName)
        {
            m_sDocumentType = _sName;
        }
    public String getDocumentType()
        {
            return m_sDocumentType;
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

    private String m_aInputFile;
    public void setInputFile(String _sInputFile)
        {
            m_aInputFile = _sInputFile;
        }
    public String getInputFile()
        {
            return m_aInputFile;
        }

    private String m_sDBInfoString;
    public String getDBInfoString()
        {
            if (m_sDBInfoString != null)
            {
                if (m_sDBInfoString.length() == 0)
                {
                    return null;
                }
            }

            return m_sDBInfoString;
        }

    public boolean cancelRequest()
        {
            File aCancelFile = null;
            String fs;
            fs = System.getProperty("file.separator");
            String sTempPath = (String)m_aCurrentParams.get( PropertyName.TEMPPATH );
            if (sTempPath != null)
            {
                String sGDC_Dir = sTempPath;

                if (m_sDistinct.length() > 0)
                {
                    sGDC_Dir = sGDC_Dir + fs + m_sDistinct;
                }

                String sCancelFile = sGDC_Dir + fs + "cancel_compare.txt";
                aCancelFile = new File(sCancelFile);

                if (aCancelFile.exists())
                {
                    GlobalLogWriter.get().println("ATTENTION: Found file: '" + sCancelFile + "'.");
                    GlobalLogWriter.get().println("User has canceled the program flow.");
                    return true;
                }
            }
            return false;
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
