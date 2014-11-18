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

    private String m_sPrinterName = null;

    private final int m_nResolutionInDPI = 180;

    private String m_sInputPath = null;
    private String m_sOutputPath = null;

    private final TestParameters m_aCurrentParams;

    public ParameterHelper(TestParameters param)
        {
            m_aCurrentParams = param;
        }


    protected TestParameters getTestParameters()
        {
            return m_aCurrentParams;
        }

        /**
         * @return the input path, if given.
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

    public boolean isIncludeSubDirectories()
        {
            boolean bIncludeSubdirectories = true;
            String sRECURSIVE = (String)getTestParameters().get( PropertyName.DOC_COMPARATOR_INCLUDE_SUBDIRS );
// TODO: I need to get the boolean value with get("name") because, if it is not given getBool() returns
//       with a default of 'false' which is not very helpful if the default should be 'true'
//       maybe a getBoolean("name", true) could be a better choice.
            if (sRECURSIVE == null)
            {
                sRECURSIVE = "true";
            }
            if (sRECURSIVE.equalsIgnoreCase("no") ||
                sRECURSIVE.equalsIgnoreCase("false"))
            {
                bIncludeSubdirectories = false;
            }
            return bIncludeSubdirectories;
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
                    m_sPrinterName = sPrinterName;
                }
            }
            return m_sPrinterName;
        }

    private PerformanceContainer m_aPerformanceContainer = null;
    /**
     * helper class for performance analyser features
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
     */
    public String getBuildID()
        {
            String sAPP = (String)m_aCurrentParams.get(util.PropertyName.APP_EXECUTION_COMMAND);
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
            XMultiServiceFactory xMSF = m_aCurrentParams.getMSF();

            // check if MultiServiceFactory is given
            if (getReferenceType().equalsIgnoreCase("pdf") ||
                getReferenceType().equalsIgnoreCase("ps") ||
                getReferenceType().equalsIgnoreCase("ooo") ||
                getReferenceType().equalsIgnoreCase("o3") )
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
            return !(sOfficeViewable.toLowerCase().equals("yes") ||
                     sOfficeViewable.toLowerCase().equals("true"));
        }
        return true; /* default: hidden */
    }

    // get/set for FilterName
    // get the right Filtername (internal Name) from
    // http://framework.openoffice.org/files/documents/25/897/filter_description.html

    private String m_sImportFilterName = "";
    private String m_sExportFilterName = "";
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
    private String m_sDocumentType = "";
    public void setDocumentType(String _sName)
        {
            m_sDocumentType = _sName;
        }
    public String getDocumentType()
        {
            return m_sDocumentType;
        }


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
            return !((getMaxPages() > 0) || (getOnlyPages().length() != 0));
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
            return !m_aCurrentParams.getBool(PropertyName.NO_SMALL_PICTURES);
        }

}
