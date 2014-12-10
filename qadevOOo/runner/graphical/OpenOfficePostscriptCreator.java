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

import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.util.XCloseable;
import helper.OfficeProvider;
import helper.OfficeWatcher;
import java.util.ArrayList;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.frame.XDesktop;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XStorable;
import com.sun.star.view.XPrintable;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.frame.XModel;
import com.sun.star.uno.AnyConverter;

import helper.URLHelper;
import helper.PropertyHelper;
import helper.OSHelper;

import java.io.File;

/**
 * This Object is to print a given document with OpenOffice.org / StarOffice
 * over the normal printer driver
 * or over it's pdf exporter
 */
public class OpenOfficePostscriptCreator implements IOffice
{
    private final ParameterHelper m_aParameterHelper;
    private final String m_sOutputURL;
    private String m_sBasename;
    private String m_sDocumentName;
    private XComponent m_aDocument;

    public OpenOfficePostscriptCreator(ParameterHelper _aParam, String _sResult)
    {
        m_aParameterHelper = _aParam;
        String sOutputURL = _sResult;
        if (! sOutputURL.startsWith("file:"))
        {
            sOutputURL = URLHelper.getFileURLFromSystemPath(_sResult);
        }
        m_sOutputURL = sOutputURL;
        m_aDocument = null;
    }


    public void load(String _sDocumentName) throws OfficeException
    {
        m_sDocumentName = _sDocumentName;

        String sInputFileURL = URLHelper.getFileURLFromSystemPath(m_sDocumentName);
        m_aDocument = loadFromURL(m_aParameterHelper, sInputFileURL);
        if (m_aDocument == null)
        {
            GlobalLogWriter.println("loadDocumentFromURL() failed with document: " + sInputFileURL);
            throw new OfficeException("load(): failed with document" + sInputFileURL);
        }

        m_sBasename = FileHelper.getBasename(m_sDocumentName);
    }

    public void storeAsPostscript() throws OfficeException
    {
        if (m_aDocument != null)
        {
            String sDocumentName = FileHelper.appendPath(m_sOutputURL, m_sBasename);
            if (m_aParameterHelper.getReferenceType().equalsIgnoreCase("ooo") ||
                m_aParameterHelper.getReferenceType().equalsIgnoreCase("o3") ||
                m_aParameterHelper.getReferenceType().equalsIgnoreCase("ps") )
            {
                String sPrintURL = sDocumentName + ".ps";

                impl_printToFileWithOOo(m_aParameterHelper, m_aDocument, sDocumentName, sPrintURL /*_sPrintFileURL*/);
                String sBasename = FileHelper.getBasename(sPrintURL);
                FileHelper.addBasenameToIndex(m_sOutputURL, sBasename, "OOo", "postscript", m_sDocumentName);
            }
            else if (m_aParameterHelper.getReferenceType().equalsIgnoreCase("pdf"))
            {
                String sPDFURL = sDocumentName + ".pdf";
                storeAsPDF(m_aParameterHelper, m_aDocument, sPDFURL);

                String sBasename = FileHelper.getBasename(sPDFURL);
                FileHelper.addBasenameToIndex(m_sOutputURL, sBasename, "pdf", "pdf-export", m_sDocumentName);
            }
            else
            {
                throw new OfficeException("unknown reference type");
            }
            GlobalLogWriter.println("Close document.");
            m_aDocument.dispose();
        }
    }

    public void start() throws OfficeException
    {
        startOffice();
    }

    public void close() throws OfficeException
    {
        stopOffice();
    }





    private void showProperty(PropertyValue _aValue)
        {
            String sName = _aValue.Name;
            String sValue;
            try
            {
                sValue = AnyConverter.toString(_aValue.Value);
                GlobalLogWriter.println("Property " + sName + ":=" + sValue);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                GlobalLogWriter.println("showProperty: can't convert a object to string. " + e.getMessage());
            }
        }

    /**
     * shows the FilterName and MediaType from the given XComponent
     */
    private String getDocumentType( XComponent _aDoc )
        {
            XModel xModel = UnoRuntime.queryInterface( XModel.class, _aDoc);
            PropertyValue[] aArgs = xModel.getArgs();
            for (int i=0;i<aArgs.length;i++)
            {
                PropertyValue aValue = aArgs[i];
                if (aValue.Name.equals("FilterName") ||
                    aValue.Name.equals("MediaType"))
                {
                    String sNameValue = "'" + aValue.Name + "' := '" + aValue.Value + "'";
                    return sNameValue;
                }
            }
            return "";
        }

    private void showDocumentType( XComponent _aDoc )
        {
            String sNameValue = getDocumentType(_aDoc);
            GlobalLogWriter.println("  Property: '" + sNameValue);
        }
    /**
     * load a OpenOffice.org document from a given URL (_sInputURL)
     * the ParameterHelper must contain a living MultiServiceFactory object
     * or we crash here.
     * Be aware, the ownership of the document gets to you, you have to close it.
     */
    private XComponent loadFromURL(ParameterHelper _aGTA,
                                         String _sInputURL)
        {
            XComponent aDoc = null;
            try
            {
                if (_aGTA.getMultiServiceFactory() == null)
                {
                    GlobalLogWriter.println("MultiServiceFactory in GraphicalTestArgument not set.");
                    return null;
                }
                Object oDsk = _aGTA.getMultiServiceFactory().createInstance("com.sun.star.frame.Desktop");
                XDesktop aDesktop = UnoRuntime.queryInterface(XDesktop.class, oDsk);

                if (aDesktop != null)
                {
                    GlobalLogWriter.println("com.sun.star.frame.Desktop created.");
                    // set here the loadComponentFromURL() properties
                    // at the moment only 'Hidden' is set, so no window is opened at work

                    ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>();

                    // set all property values
                    if (_aGTA.isHidden())
                    {
                        PropertyValue Arg = new PropertyValue();
                        Arg.Name = "Hidden";
                        Arg.Value = Boolean.TRUE;
                        aPropertyList.add(Arg);
                        showProperty(Arg);
                    }
                    if (_aGTA.getImportFilterName() != null && _aGTA.getImportFilterName().length() > 0)
                    {
                        PropertyValue Arg = new PropertyValue();
                        Arg.Name = "FilterName";
                        Arg.Value = _aGTA.getImportFilterName();
                        aPropertyList.add(Arg);
                        showProperty(Arg);
                    }
                    PropertyValue ReadOnly = new PropertyValue();
                    ReadOnly.Name = "ReadOnly";
                    ReadOnly.Value = Boolean.TRUE;
                    aPropertyList.add(ReadOnly);
                    showProperty(ReadOnly);

                    GlobalLogWriter.println(DateHelper.getDateTimeForHumanreadableLog() + " Load document");

                    XComponentLoader aCompLoader = UnoRuntime.queryInterface( XComponentLoader.class, aDesktop);

                    // XComponent aDoc = null;

                    _aGTA.getPerformance().startTime(PerformanceContainer.Load);
                    aDoc = aCompLoader.loadComponentFromURL(_sInputURL, "_blank", FrameSearchFlag.ALL, PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList) );
                    _aGTA.getPerformance().stopTime(PerformanceContainer.Load);
                    if (aDoc != null)
                    {
                        GlobalLogWriter.println(DateHelper.getDateTimeForHumanreadableLog() + " Load document done.");
                        showDocumentType(aDoc);
                        _aGTA.setDocumentType(getDocumentType(aDoc));
// TODO:                        TimeHelper.waitInSeconds(20, "Wait after load document. Maybe helps due to layouting problems.");
                    }
                    else
                    {
                        GlobalLogWriter.println(" Load document failed.");
                        if (_aGTA.getImportFilterName() != null && _aGTA.getImportFilterName().length() > 0)
                        {
                            GlobalLogWriter.println(" Please check FilterName := '" + _aGTA.getImportFilterName() + "'");
                        }
                        GlobalLogWriter.println("");
                    }
                }
                else
                {
                    GlobalLogWriter.println("com.sun.star.frame.Desktop failed.");
                }
            }
            catch ( com.sun.star.uno.Exception e )
            {
                // Some exception occurs.FAILED
                GlobalLogWriter.println("UNO Exception caught.");
                GlobalLogWriter.println("Message: " + e.getMessage());
                e.printStackTrace();
            }
            return aDoc;
        }

    private boolean exportToPDF(XComponent _xComponent, String _sDestinationName)
        {
            XServiceInfo xServiceInfo =
                 UnoRuntime.queryInterface(
                    XServiceInfo.class, _xComponent
                    );

            ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>();
            PropertyValue aFiltername = new PropertyValue();
            aFiltername.Name = "FilterName";
            aFiltername.Value = getFilterName_forPDF(xServiceInfo);
            aPropertyList.add(aFiltername);
            showProperty(aFiltername);
            boolean bWorked = true;

// TODO:             TimeHelper.waitInSeconds(20, "Wait before storeToURL. Maybe helps due to layouting problems.");
            try
            {
                XStorable store =
                     UnoRuntime.queryInterface(
                        XStorable.class, _xComponent
                        );
                store.storeToURL(_sDestinationName, PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList));
            }
            catch (com.sun.star.io.IOException e)
            {
                GlobalLogWriter.println("IO Exception caught.");
                GlobalLogWriter.println("Message: " + e.getMessage());
                bWorked = false;
            }

            return bWorked;
        }


    private String getFilterName_forPDF(XServiceInfo xServiceInfo)
        {
            String filterName = "";

            if (xServiceInfo.supportsService("com.sun.star.text.TextDocument"))
            {
                //writer
                filterName = "writer_pdf_Export";
            }
            else if ( xServiceInfo.supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
            {
                //calc
                filterName = "calc_pdf_Export";
            }
            else if ( xServiceInfo.supportsService( "com.sun.star.drawing.DrawingDocument" ) )
            {
                //draw
                filterName = "draw_pdf_Export";
            }
            else if ( xServiceInfo.supportsService( "com.sun.star.presentation.PresentationDocument" ) )
            {
                //impress
                filterName = "impress_pdf_Export";
            }
            else if (xServiceInfo.supportsService("com.sun.star.text.WebDocument"))
            {
                //html document
                filterName = "writer_web_pdf_Export";
            }
            else if ( xServiceInfo.supportsService("com.sun.star.text.GlobalDocument") )
            {
                //master document
                filterName = "writer_globaldocument_pdf_Export";
            }
            else if ( xServiceInfo.supportsService( "com.sun.star.formulaFormulaProperties" ) )
            {
                //math document
                filterName = "math_pdf_Export";
            }

            return filterName;
        }



    private boolean storeAsPDF(ParameterHelper _aGTA,
                                     XComponent _aDoc,
                                     String _sOutputURL) throws OfficeException
        {
            boolean bBack = true;
            _aGTA.getPerformance().startTime(PerformanceContainer.StoreAsPDF);
            bBack = exportToPDF(_aDoc, _sOutputURL);
            _aGTA.getPerformance().stopTime(PerformanceContainer.StoreAsPDF);

            if (!bBack)
            {
                GlobalLogWriter.println("Can't store document as PDF.");
                throw new OfficeException("Can't store document as PDF");
            }
            else
            {
                FileHelper.createInfoFile(_sOutputURL, _aGTA, "as pdf");
            }
            return bBack;
        }


    private boolean impl_printToFileWithOOo(ParameterHelper _aGTA,
                                                   XComponent _aDoc,
                                                   String _sOutputURL,
                                                   String _sPrintFileURL)
        {
            boolean bBack = false;
            boolean bFailed = true;              // always be a pessimist,
            if (_aDoc == null)
            {
                GlobalLogWriter.println("No document is given.");
                return bBack;
            }

            try
            {
                if (_sOutputURL != null)
                {
                    if (isStoreAllowed())
                    {
                        // store the document in an other directory
                        XStorable aStorable = UnoRuntime.queryInterface( XStorable.class, _aDoc);
                        if (aStorable != null)
                        {
                            PropertyValue [] szEmptyArgs = new PropertyValue [0];

                            GlobalLogWriter.println(DateHelper.getDateTimeForHumanreadableLog() + " Store document.");
                            _aGTA.getPerformance().startTime(PerformanceContainer.Store);
                            aStorable.storeAsURL(_sOutputURL, szEmptyArgs);
                            _aGTA.getPerformance().stopTime(PerformanceContainer.Store);

                            GlobalLogWriter.println(DateHelper.getDateTimeForHumanreadableLog() + " Store document done.");
                            GlobalLogWriter.println("Reload stored file test.");
                            XComponent aDoc = loadFromURL(_aGTA, _sOutputURL);
                            if (aDoc == null)
                            {
                                GlobalLogWriter.println("Reload stored file test failed, can't reload file: " + _sOutputURL);
                            }
                            else
                            {
                                XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, aDoc);
                                if (xClose != null)
                                {
                                    xClose.close(true);
                                }
                                else
                                {
                                    aDoc.dispose();
                                }
                            }
                        }
                    }
                    else
                    {
                        // make sure to create the directory in
                        String sOutputFilename = FileHelper.getSystemPathFromFileURL(_sOutputURL);
                        String sOutputPath = FileHelper.getPath(sOutputFilename);
                        File aFile = new File(sOutputPath);
                        aFile.mkdirs();
                    }
                }
            }
            catch ( com.sun.star.uno.Exception e )
            {
                // Some exception occurs.FAILED
                GlobalLogWriter.println("UNO Exception caught.");
                GlobalLogWriter.println("Message: " + e.getMessage());

                e.printStackTrace();
                bBack = false;
            }

            try
            {

                // Change Pagesettings to DIN A4

                GlobalLogWriter.println(DateHelper.getDateTimeForHumanreadableLog() + " Print document.");
                XPrintable aPrintable =  UnoRuntime.queryInterface( XPrintable.class, _aDoc);
                if (aPrintable != null)
                {
                    // configure Office to allow to execute macos

// TODO: We need a possibility to set the printer name also for StarOffice/OpenOffice
                    if (OSHelper.isWindows())
                    {
                        if (_aGTA.getPrinterName() != null)
                        {
                            ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>();
                            // PropertyValue [] aPrintProps = new PropertyValue[1];
                            PropertyValue Arg = new PropertyValue();
                            Arg.Name = "Name";
                            Arg.Value = _aGTA.getPrinterName();
                            aPropertyList.add(Arg);
                            showProperty(Arg);
                            // GlobalLogWriter.println("Printername is not null, so set to " + _aGTA.getPrinterName());
                            aPrintable.setPrinter(PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList));
                        }
                    }

                    // set property values for XPrintable.print()
                    // more can be found at "http://api.libreoffice.org/docs/common/ref/com/sun/star/view/PrintOptions.html"

                    // If we are a SpreadSheet (calc), we need to set PrintAllSheets property to 'true'
                    XServiceInfo xServiceInfo =  UnoRuntime.queryInterface( XServiceInfo.class, _aDoc );
                    if ( xServiceInfo.supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
                    {
                        XMultiServiceFactory xMSF = _aGTA.getMultiServiceFactory();
                        Object aSettings = xMSF.createInstance( "com.sun.star.sheet.GlobalSheetSettings" );
                        if (aSettings != null)
                        {
                            XPropertySet xPropSet = UnoRuntime.queryInterface( XPropertySet.class, aSettings );
                            xPropSet.setPropertyValue( "PrintAllSheets", Boolean.TRUE );
                            GlobalLogWriter.println("PrintAllSheets := true");
                        }
                    }

                    ArrayList<PropertyValue> aPrintProps = new ArrayList<PropertyValue>();

                    PropertyValue Arg = new PropertyValue();
                    Arg.Name = "FileName";
                    Arg.Value = _sPrintFileURL;
                    aPrintProps.add(Arg);
                    showProperty(Arg);


                    // generate pages string
                    if (!_aGTA.printAllPages())
                    {
                        String sPages = "";
                        if (_aGTA.getMaxPages() > 0)
                        {
                            sPages = "1-" + _aGTA.getMaxPages();
                        }
                        if (_aGTA.getOnlyPages().length() != 0)
                        {
                            if (sPages.length() != 0)
                            {
                                sPages += ";";
                            }
                            sPages += String.valueOf(_aGTA.getOnlyPages());
                        }

                        Arg = new PropertyValue();
                        Arg.Name = "Pages";
                        Arg.Value = sPages;
                        aPrintProps.add(Arg);
                        showProperty(Arg);
                    }

                    _aGTA.getPerformance().startTime(PerformanceContainer.Print);
                    aPrintable.print(PropertyHelper.createPropertyValueArrayFormArrayList(aPrintProps));
                    TimeHelper.waitInSeconds(1, "Start waiting for print ready.");

                    GlobalLogWriter.println("Wait until document is printed.");
                    boolean isBusy = true;
                    int nPrintCount = 0;
                    while (isBusy)
                    {
                        PropertyValue[] aPrinterProps = aPrintable.getPrinter();
                        int nPropIndex = 0;
                        while (!"IsBusy".equals(aPrinterProps[nPropIndex].Name))
                        {
                            nPropIndex++;
                        }
                        isBusy = aPrinterProps[nPropIndex].Value.equals(Boolean.TRUE);
                        TimeHelper.waitInSeconds(1, "is print ready?");
                        nPrintCount++;
                        if (nPrintCount > 3600)
                        {
                            // we will never wait >1h until print is ready!
                            GlobalLogWriter.println("ERROR: Cancel print due to too long wait.");
                            throw new com.sun.star.uno.Exception("Convwatch exception, wait too long for printing.");
                        }
                    }
                    _aGTA.getPerformance().stopTime(PerformanceContainer.Print);
                    GlobalLogWriter.println(DateHelper.getDateTimeForHumanreadableLog() + " Print document done.");

                    // Create a .info file near the printed '.ps' or '.prn' file.
                    FileHelper.createInfoFile(_sPrintFileURL, _aGTA);
                }
                else
                {
                    GlobalLogWriter.println("Can't get XPrintable interface.");
                }
                bFailed = false;
                bBack = true;
            }
            catch ( com.sun.star.uno.Exception e )
            {
                // Some exception occurs.FAILED
                GlobalLogWriter.println("UNO Exception caught.");
                GlobalLogWriter.println("Message: " + e.getMessage());

                e.printStackTrace();
                bBack = false;
            }

            if (bFailed)
            {
                GlobalLogWriter.println("convwatch.OfficePrint: FAILED");
            }
            else
            {
                GlobalLogWriter.println("convwatch.OfficePrint: OK");
            }
            return bBack;
        }





    // TODO: move this away!





    private OfficeProvider m_aProvider = null;
    private void startOffice()
    {
            m_aParameterHelper.getTestParameters().put(util.PropertyName.DONT_BACKUP_USERLAYER, Boolean.TRUE);

            m_aParameterHelper.getPerformance().startTime(PerformanceContainer.OfficeStart);
            m_aProvider = new OfficeProvider();
            XMultiServiceFactory xMSF = (XMultiServiceFactory) m_aProvider.getManager(m_aParameterHelper.getTestParameters());
            m_aParameterHelper.getTestParameters().put("ServiceFactory", xMSF);
            m_aParameterHelper.getPerformance().stopTime(PerformanceContainer.OfficeStart);

            long nStartTime = m_aParameterHelper.getPerformance().getTime(PerformanceContainer.OfficeStart);
            m_aParameterHelper.getPerformance().setTime(PerformanceContainer.OfficeStart, nStartTime);

        // Watcher Object is need in log object to give a simple way to say if a running office is alive.
        // As long as a log comes, it pings the Watcher and says the office is alive, if not an
        // internal counter increase and at a given point (300 seconds) the office is killed.
        if (GlobalLogWriter.get().getWatcher() == null)
        {
            GlobalLogWriter.println("Set office watcher");
            OfficeWatcher aWatcher = (OfficeWatcher)m_aParameterHelper.getTestParameters().get("Watcher");
            GlobalLogWriter.get().setWatcher(aWatcher);
        }
    }

    private void stopOffice()
    {
        // Office shutdown
        if (m_aProvider != null)
        {
            String sAppExecCmd = (String)m_aParameterHelper.getTestParameters().get("AppExecutionCommand");
            if (sAppExecCmd != null && sAppExecCmd.length() > 0)
            {
                m_aProvider.closeExistingOffice(m_aParameterHelper.getTestParameters(), true);
            }
        }
    }



    private boolean isStoreAllowed()
        {
        return false;
        }

}

