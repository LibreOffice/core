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

package graphical;

import com.sun.star.lang.XMultiServiceFactory;
import lib.TestParameters;

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
 *   create an ParameterHelper with a set of TestParameters
 *    ParameterHelper a = new ParameterHelper(params);
 *
 *  If you wish to use pdf export instead of normal printer output, set also the reference type to 'pdf'
 *    a.setReferenceType("pdf");
 *
 *
 * For MS Office:
 *   create a ParameterHelper and set the reference type to 'msoffice'
 *    ParameterHelper a = new ParameterHelper(params);
 *    a.setReferenceType("msoffice");
 *
 * within windows it's better to set also a printer name so it's simply possible to use for normal work the default printer
 * and for such tests with ConvWatch a extra printer.
 *    a.setPrinterName("CrossOffice Printer");
 *
 */

public class ParameterHelper
{
    /*
     TODO:
     Possible reference types are currently
     // ooo
     // pdf
     // msoffice
    */
    private String m_sReferenceType = null;

    // private String m_sTargetFrameName = "_blank";

    private String m_sPrinterName = null;

    private int m_nResolutionInDPI = 180;

    private boolean m_bIncludeSubdirectories;

    private String m_sInputPath = null;
    private String m_sOutputPath = null;
//    private String m_sReferencePath = null;

    private TestParameters m_aCurrentParams;

    // private GlobalLogWriter m_aLog;

    public ParameterHelper(TestParameters param)
        {
            m_aCurrentParams = param;
            // m_aLog = log;
            // interpretReferenceType();
            // interpretPrinterName();
        }


    protected TestParameters getTestParameters()
        {
            return m_aCurrentParams;
        }

        /**
         * return the input path, if given.
         * @return
         */
    public String getInputPath()
    {
        if (m_sInputPath == null)
        {
            String sInputPath = (String)getTestParameters().get( PropertyName.DOC_COMPARATOR_INPUT_PATH );
            if (sInputPath == null || sInputPath.length() == 0)
            {
                GlobalLogWriter.println("Please set input path (path to documents) " + PropertyName.DOC_COMPARATOR_INPUT_PATH + "=path.");
            }
            else
            {
                m_sInputPath = helper.StringHelper.removeQuoteIfExists(sInputPath);
            }
        }
        return m_sInputPath;
    }

    public String getOutputPath()
    {
        if (m_sOutputPath == null)
        {
            String sOutputPath = (String)getTestParameters().get( PropertyName.DOC_COMPARATOR_OUTPUT_PATH );
            if (sOutputPath == null || sOutputPath.length() == 0)
            {
                GlobalLogWriter.println("Please set output path (path where to store document results) " + PropertyName.DOC_COMPARATOR_OUTPUT_PATH + "=path.");
            }
            else
            {
                m_sOutputPath = helper.StringHelper.removeQuoteIfExists(sOutputPath);
            }
        }
        return m_sOutputPath;
    }

//    public String getReferencePath()
//    {
//        if (m_sReferencePath == null)
//        {
//            String sReferencePath = (String)getTestParameters().get( PropertyName.DOC_COMPARATOR_REFERENCE_PATH );
//            if (sReferencePath == null || sReferencePath.length() == 0)
//            {
//                GlobalLogWriter.println("Please set reference path (path to reference documents) " + PropertyName.DOC_COMPARATOR_REFERENCE_PATH + "=path.");
//            }
//            else
//            {
//                m_sReferencePath = sReferencePath;
//            }
//        }
//        return m_sReferencePath;
//    }


    public boolean isIncludeSubDirectories()
        {
            m_bIncludeSubdirectories = true;
            String sRECURSIVE = (String)getTestParameters().get( PropertyName.DOC_COMPARATOR_INCLUDE_SUBDIRS );
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
            return m_bIncludeSubdirectories;
        }

    public String getReferenceType()
        {
            if (m_sReferenceType == null)
            {
                // REFERENCE_TYPE ----------

                String sReferenceType = (String)getTestParameters().get( PropertyName.DOC_COMPARATOR_REFERENCE_TYPE );
                if (sReferenceType == null || sReferenceType.length() == 0)
                {
                    m_sReferenceType = "ps";
                }
                else
                {
                    // log.println("found REFERENCE_TYPE " + sReferenceType );
                    m_sReferenceType = sReferenceType;
                }
            }
            return m_sReferenceType;
        }

        public String getPrinterName()
        {
            if (m_sPrinterName == null)
            {
                // PRINTER_NAME ----------

                String sPrinterName = (String)getTestParameters().get( PropertyName.DOC_COMPARATOR_PRINTER_NAME );
                if (sPrinterName == null || sPrinterName.length() == 0)
                {
                    m_sPrinterName = "";
                }
                else
                {
                    // log.println("found PRINTER_NAME " + sPrinterName );
                    m_sPrinterName = sPrinterName;
                }
            }
            return m_sPrinterName;
        }

    PerformanceContainer m_aPerformanceContainer = null;
    /**
     * helper class for performance analyser features
     * @return
     */
    public PerformanceContainer getPerformance()
        {
            if (m_aPerformanceContainer == null)
            {
                m_aPerformanceContainer = new PerformanceContainer();
            }
            return m_aPerformanceContainer;
        }

    /**
     * Helper function to get the buildid of the current used OpenOffice.org
     * out of the AppExecutionCommand the build ID
     * @return
     */
    public String getBuildID()
        {
            String sAPP = (String)m_aCurrentParams.get(util.PropertyName.APP_EXECUTION_COMMAND);
            // return getBuildID(sAPP);
//  TODO: here we need the getBuildID(string) method
            String sBuildID = BuildID.getBuildID(sAPP);
            return sBuildID;
        }

    /**
     * @return integer value, which contain resolution in DPI.
     */
    public int getResolutionInDPI()
    {
        return m_nResolutionInDPI;
    }
    // get methods
    public XMultiServiceFactory getMultiServiceFactory()
        {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)m_aCurrentParams.getMSF();

            // check if MultiServiceFactory is given
            if (getReferenceType().toLowerCase().equals("pdf") ||
                getReferenceType().toLowerCase().equals("ps") ||
                getReferenceType().toLowerCase().equals("ooo") ||
                getReferenceType().toLowerCase().equals("o3") )
            {
                if (xMSF == null)
                {
                    GlobalLogWriter.println("ERROR! MultiServiceFactory not given.");
                }
            }
            return xMSF;
        }

    // Hidden = true hiddes a used OpenOffice.org, all code is executed in the background
    // This parameter is not used for RefType: msoffice
    // boolean m_bHidden = true;


    public boolean isHidden()
    {
        // HIDDEN

        String sOfficeViewable = (String)m_aCurrentParams.get(PropertyName.OFFICE_VIEWABLE);
        if (sOfficeViewable != null)
        {
            if (sOfficeViewable.toLowerCase().equals("yes") ||
                sOfficeViewable.toLowerCase().equals("true"))
            {
                return false; // setViewable();
            }
            else
            {
                return true; // setHidden();
            }
        }
        return true; /* default: hidden */
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
    String m_sDocumentType = "";
    public void setDocumentType(String _sName)
        {
            m_sDocumentType = _sName;
        }
    public String getDocumentType()
        {
            return m_sDocumentType;
        }


//    String m_sDefaultXMLFormatApplication = null;
//    public String getDefaultXMLFormatApp()
//    {
//        if (m_sDefaultXMLFormatApplication == null)
//        {
//            // DEFAULT_XML_FORMAT_APP ------
//
//            String sDefaultXMLFormatApp = (String)m_aCurrentParams.get( PropertyName.DOC_COMPARATOR_DEFAULT_XML_FORMAT_APP );
//            if (sDefaultXMLFormatApp == null || sDefaultXMLFormatApp.length() == 0)
//            {
//                m_sDefaultXMLFormatApplication = "word";
//            }
//            else
//            {
//                m_sDefaultXMLFormatApplication = sDefaultXMLFormatApp;
//            }
//        }
//        return m_sDefaultXMLFormatApplication;
//    }


    // Pages -------------------------------------------------------------------

    /**
     * @return the number of pages to be print
     */
    public int getMaxPages()
    {
        // default is 0 (print all pages)
        int nMaxPages = m_aCurrentParams.getInt( PropertyName.DOC_COMPARATOR_PRINT_MAX_PAGE );
        return nMaxPages;
    }

    /**
     * @return as string, which pages should be print, e.g. '1-4;6' here, page 1 to 4 and page 6.
     */
    public String getOnlyPages()
    {
        // default is null, there is no page which we want to print only.
        String sOnlyPage = (String)m_aCurrentParams.get(PropertyName.DOC_COMPARATOR_PRINT_ONLY_PAGE);
        if (sOnlyPage == null)
        {
            sOnlyPage = "";
        }
        return sOnlyPage;
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

    public boolean getOverwrite()
        {
            boolean bOverwrite = m_aCurrentParams.getBool( PropertyName.DOC_COMPARATOR_OVERWRITE_REFERENCE);
            return bOverwrite;
        }

    private String m_sHTMLPrefix = null;
    public String getHTMLPrefix()
    {
        if (m_sHTMLPrefix == null)
        {
            String sPrefix = (String)getTestParameters().get( PropertyName.DOC_COMPARATOR_HTML_OUTPUT_PREFIX );
            if (sPrefix == null || sPrefix.length() == 0)
            {
                GlobalLogWriter.println("Please set html prefix " + PropertyName.DOC_COMPARATOR_HTML_OUTPUT_PREFIX + "=prefix.");
            }
            else
            {
                m_sHTMLPrefix = sPrefix;
            }
        }
        return m_sHTMLPrefix;
    }

    public boolean createSmallPictures()
        {
            // boolean bCreateSmallPictures = true;
            boolean bNoSmallPictures = m_aCurrentParams.getBool( PropertyName.NO_SMALL_PICTURES);
            if (bNoSmallPictures == true)
            {
                return false;
            }
            return true;
        }

}
