/*************************************************************************
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
 ************************************************************************/

package graphical;

import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.util.XCloseable;
import helper.OfficeProvider;
import helper.OfficeWatcher;
import java.util.ArrayList;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.document.XTypeDetection;
import com.sun.star.container.XNameAccess;
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

// import helper.Parameter;
import java.io.File;

/**
 * This Object is to print a given document with OpenOffice.org / StarOffice
 * over the normal printer driver
 * or over it's pdf exporter
 */
public class OpenOfficePostscriptCreator implements IOffice
{
    private ParameterHelper m_aParameterHelper;
    private String m_sOutputURL;
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
            if (m_aParameterHelper.getReferenceType().toLowerCase().equals("ooo") ||
                m_aParameterHelper.getReferenceType().toLowerCase().equals("o3") ||
                m_aParameterHelper.getReferenceType().toLowerCase().equals("ps") )
            {
                String sPrintURL = sDocumentName + ".ps";

                impl_printToFileWithOOo(m_aParameterHelper, m_aDocument, sDocumentName, sPrintURL /*_sPrintFileURL*/);
                String sBasename = FileHelper.getBasename(sPrintURL);
                FileHelper.addBasenameToIndex(m_sOutputURL, sBasename, "OOo", "postscript", m_sDocumentName);
            }
            else if (m_aParameterHelper.getReferenceType().toLowerCase().equals("pdf"))
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
                // System.out.print("Property: '" + aValue.Name);
                // System.out.println("' := '" + aValue.Value + "'");
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
                    // String sInputURL = aCurrentParameter.sInputURL;
                    // String sOutputURL = aCurrentParameter.sOutputURL;
                    // String sPrintFileURL = aCurrentParameter.sPrintToFileURL;
                    // System.out.println(_sInputURL);


                    // set here the loadComponentFromURL() properties
                    // at the moment only 'Hidden' is set, so no window is opened at work

                    ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>();

                    // check which properties should set and count it.
                    // if (_aGTA.isHidden())
                    // {
                    //     nPropertyCount ++;
                    // }
                    // if (_aGTA.getImportFilterName() != null && _aGTA.getImportFilterName().length() > 0)
                    // {
                    //     nPropertyCount ++;
                    // }

                    // initialize the propertyvalue
                    // int nPropertyIndex = 0;
                    // aProps = new PropertyValue[ nPropertyCount ];

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
                    // GlobalLogWriter.flush();

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
                // Some exception occures.FAILED
                GlobalLogWriter.println("UNO Exception caught.");
                GlobalLogWriter.println("Message: " + e.getMessage());
                e.printStackTrace();
                aDoc = null;
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

    // -----------------------------------------------------------------------------

//    public boolean storeAsPDF(ParameterHelper _aGTA,
//                                     String _sInputURL,
//                                     String _sOutputURL)
//        {
//            boolean bBack = false;
//            XComponent aDoc = loadFromURL(_aGTA, _sInputURL);
//
//            if (aDoc == null)
//            {
//                GlobalLogWriter.println("Can't load document.");
//                return bBack;
//            }
//            bBack = storeAsPDF(_aGTA, aDoc, _sOutputURL);
//            FileHelper.createInfoFile(_sOutputURL, _aGTA, "as pdf");
//
//            GlobalLogWriter.println("Close document.");
//            aDoc.dispose();
//            return bBack;
//        }

    public boolean storeAsPDF(ParameterHelper _aGTA,
                                     XComponent _aDoc,
                                     String _sOutputURL) throws OfficeException
        {
            // try {
            boolean bBack = true;
            _aGTA.getPerformance().startTime(PerformanceContainer.StoreAsPDF);
            bBack = exportToPDF(_aDoc, _sOutputURL);
            _aGTA.getPerformance().stopTime(PerformanceContainer.StoreAsPDF);

            if (!bBack)
            {
                GlobalLogWriter.println("Can't store document as PDF.");
//                bBack = false;
                throw new OfficeException("Can't store document as PDF");
            }
            else
            {
                FileHelper.createInfoFile(_sOutputURL, _aGTA, "as pdf");
            }
            return bBack;
        }

    // -----------------------------------------------------------------------------

    /**
     * print the document found in file (_sInputURL) to as postscript to file (_sPrintFileURL)
     * Due to the fact we use a printer to convert the file to postscript, the default printer
     * to create such postscript format must be installed, this is not tested here.
     *
     * @return true, if print has been done.
     *         Be careful, true means only print returns with no errors, to be sure print is really done
     *         check existance of _sPrintFileURL
     */

//    public boolean printToFileWithOOo(ParameterHelper _aGTA,
//                                             String _sInputURL,
//                                             String _sOutputURL,
//                                             String _sPrintFileURL)
//        {
//            // waitInSeconds(1);
//            boolean bBack = false;
//
//            XComponent aDoc = loadFromURL(_aGTA, _sInputURL);
//            if (aDoc != null)
//            {
//                if ( _sInputURL.equals(_sOutputURL) )
//                {
//                    // don't store document
//                    // input and output are equal OR
//                    GlobalLogWriter.println("Warning: Inputpath and Outputpath are equal. Document will not stored again.");
//                    disallowStore();
//                }
//                bBack = impl_printToFileWithOOo(_aGTA, aDoc, _sOutputURL, _sPrintFileURL);
//
//                GlobalLogWriter.println("Close document.");
//                aDoc.dispose();
//            }
//            else
//            {
//                GlobalLogWriter.println("loadDocumentFromURL() failed with document: " + _sInputURL);
//            }
//            return bBack;
//        }



    // -----------------------------------------------------------------------------
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
                            // TimeHelper.waitInSeconds(1, "After store as URL to:" + _sOutputURL);
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
                                // TimeHelper.waitInSeconds(1, "after close temp document");
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
                // Some exception occures.FAILED
                GlobalLogWriter.println("UNO Exception caught.");
                GlobalLogWriter.println("Message: " + e.getMessage());

                e.printStackTrace();
                bBack = false;
            }

            try
            {

                // System.out.println("Document loaded.");
                // Change Pagesettings to DIN A4

                GlobalLogWriter.println(DateHelper.getDateTimeForHumanreadableLog() + " Print document.");
                XPrintable aPrintable =  UnoRuntime.queryInterface( XPrintable.class, _aDoc);
                if (aPrintable != null)
                {
                    // System.out.println("  Set PaperFormat to DIN A4");
                    // {
                    //     PropertyValue[] aPrinterProps = aPrintable.getPrinter();
                    //     System.out.println("PrinterProps size: " + String.valueOf(aPrinterProps.length));
                    //     int nPropIndex = 0;
                    //     while (!"PaperFormat".equals(aPrinterProps[nPropIndex].Name))
                    //     {
                    //         // System.out.println(aPrinterProps[nPropIndex].Name);
                    //         nPropIndex++;
                    //     }
                    //     aPrinterProps[nPropIndex].Value = com.sun.star.view.PaperFormat.A4;
                    //     aPrintable.setPrinter(aPrinterProps);
                    // }

                    // configure Office to allow to execute macos

// TODO: We need a possiblity to set the printer name also for StarOffice/OpenOffice
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
                    // more can be found at "http://api.openoffice.org/docs/common/ref/com/sun/star/view/PrintOptions.html"

                    // int nProperties = 1;                    // default for 'FileName' property
                    // if (_aGTA.printAllPages() == false)
                    // {
                    //     // we don't want to print all pages, build Pages string by ourself
                    //     nProperties ++;
                    // }
                    // int nPropsCount = 0;

                    // If we are a SpreadSheet (calc), we need to set PrintAllSheets property to 'true'
                    XServiceInfo xServiceInfo =  UnoRuntime.queryInterface( XServiceInfo.class, _aDoc );
                    if ( xServiceInfo.supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
                    {
                        XMultiServiceFactory xMSF = _aGTA.getMultiServiceFactory();
                        Object aSettings = xMSF.createInstance( "com.sun.star.sheet.GlobalSheetSettings" );
                        if (aSettings != null)
                        {
                            XPropertySet xPropSet = UnoRuntime.queryInterface( XPropertySet.class, aSettings );
                            xPropSet.setPropertyValue( "PrintAllSheets", new Boolean( true ) );
                            GlobalLogWriter.println("PrintAllSheets := true");
                        }
                    }

                    ArrayList<PropertyValue> aPrintProps = new ArrayList<PropertyValue>();
                    // GlobalLogWriter.println("Property FileName:=" + _sPrintFileURL);

                    // PropertyValue [] aPrintProps = new PropertyValue[nProperties];
                    PropertyValue Arg = new PropertyValue();
                    Arg.Name = "FileName";
                    Arg.Value = _sPrintFileURL;
                    // aPrintProps[nPropsCount ++] = Arg;
                    aPrintProps.add(Arg);
                    showProperty(Arg);


                    // generate pages string
                    if (_aGTA.printAllPages() == false)
                    {
                        String sPages = "";
                        if (_aGTA.getMaxPages() > 0)
                        {
                            sPages = "1-" + String.valueOf(_aGTA.getMaxPages());
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

                    // GlobalLogWriter.println("Start printing.");

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
                            // System.out.println(aPrinterProps[nPropIndex].Name);
                            nPropIndex++;
                        }
                        isBusy = (aPrinterProps[nPropIndex].Value == Boolean.TRUE) ? true : false;
                        TimeHelper.waitInSeconds(1, "is print ready?");
                        nPrintCount++;
                        if (nPrintCount > 3600)
                        {
                            // we will never wait >1h until print is ready!
                            GlobalLogWriter.println("ERROR: Cancel print due to too long wait.");
                            throw new com.sun.star.uno.Exception("Convwatch exception, wait too long for printing.");
                        }
                    }
// TODO:
//                    TimeHelper.waitInSeconds(40, "Start waiting after print ready.");

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
                // Some exception occures.FAILED
                GlobalLogWriter.println("UNO Exception caught.");
                GlobalLogWriter.println("Message: " + e.getMessage());

                e.printStackTrace();
                bBack = false;
            }

            if (bFailed == true)
            {
                GlobalLogWriter.println("convwatch.OfficePrint: FAILED");
            }
            else
            {
                GlobalLogWriter.println("convwatch.OfficePrint: OK");
            }
            return bBack;
        }


    /**
     * @param _aGTA
     * @param _sAbsoluteOutputPath
     * @param _sAbsoluteInputFile
     * @return true, if the reference (*.prrn file) based on given output path and given input path exist.
     *               If OVERWRITE_REFERENCE is set, always return false.
     */
    public boolean isReferenceExists(ParameterHelper _aGTA,
                                            String _sAbsoluteOutputPath,
                                            String _sAbsoluteInputFile)
        {
            if (! FileHelper.exists(_sAbsoluteInputFile))
            {
                // throw new ConvWatchCancelException("Input file: " + _sAbsoluteInputFile + " does not exist.");
                return false;
            }

            // String fs = System.getProperty("file.separator");

            // String sInputFileURL = URLHelper.getFileURLFromSystemPath(_sAbsoluteInputFile);

            String sInputFileBasename = FileHelper.getBasename(_sAbsoluteInputFile);
            // String sOutputFileURL = null;
            String sOutputPath;
            if (_sAbsoluteOutputPath != null)
            {
                sOutputPath    = _sAbsoluteOutputPath;
                // FileHelper.makeDirectories("", sOutputPath);
            }
            else
            {
                String sInputPath = FileHelper.getPath(_sAbsoluteInputFile);
                sOutputPath    = sInputPath;
            }
            // sOutputFileURL = URLHelper.getFileURLFromSystemPath(sOutputPath + fs + sInputFileBasename);
            // sOutputFileURL = null;

            String sPrintFilename = FileHelper.getNameNoSuffix(sInputFileBasename);
            // String sPrintFileURL;

            String sAbsolutePrintFilename = FileHelper.appendPath(sOutputPath, sPrintFilename + ".prn");
            if (FileHelper.exists(sAbsolutePrintFilename) && _aGTA.getOverwrite() == false)
            {
                GlobalLogWriter.println("Reference already exist, don't overwrite. Set " + PropertyName.DOC_COMPARATOR_OVERWRITE_REFERENCE + "=true to force overwrite.");
                return true;
            }
            return false;
        }

    // -----------------------------------------------------------------------------
    /**
     * create a reference file
     * _sAbsoluteInputPath  contains the source file, if not exists, return with failure.
     * _sAbsoluteOutputPath contains the destination, where the file will store after load with StarOffice/OpenOffice.org
     *                      if is null, print only near the Input file path
     * _sPrintType ".prn" Print input file with StarOffice/OpenOffice.org and the default printer as PostScript
     *
     * @param _aGTA
     * @return
     */
//    public static boolean buildReference(ParameterHelper _aGTA,
//                                         String _sAbsoluteOutputPath,
//                                         String _sAbsoluteInputFile)
//        throws OfficeException
//        {
//            if (! FileHelper.exists(_sAbsoluteInputFile))
//            {
//                throw new OfficeException("buildReference(): Input file: " + _sAbsoluteInputFile + " does not exist.");
//            }
//
//            String fs = System.getProperty("file.separator");
//
//            String sInputFileURL = URLHelper.getFileURLFromSystemPath(_sAbsoluteInputFile);
//
//            String sInputFileBasename = FileHelper.getBasename(_sAbsoluteInputFile);
//            String sOutputFileURL = null;
//            String sOutputPath;
//            if (_sAbsoluteOutputPath != null)
//            {
//                sOutputPath    = _sAbsoluteOutputPath;
//                FileHelper.makeDirectories("", sOutputPath);
//            }
//            else
//            {
//                String sInputPath = FileHelper.getPath(_sAbsoluteInputFile);
//                sOutputPath    = sInputPath;
//            }
//            // sOutputFileURL = URLHelper.getFileURLFromSystemPath(sOutputPath + fs + sInputFileBasename);
//            sOutputFileURL = null;
//
//            String sPrintFilename = FileHelper.getNameNoSuffix(sInputFileBasename);
//            String sPrintFileURL;
//
//            String sAbsolutePrintFilename = sOutputPath + fs + sPrintFilename + ".prn";
//            if (FileHelper.exists(sAbsolutePrintFilename) && _aGTA.getOverwrite() == false)
//            {
//                GlobalLogWriter.println("Reference already exist, don't overwrite. Set " + PropertyName.DOC_COMPARATOR_OVERWRITE_REFERENCE + "=true to force overwrite.");
//                return true;
//            }
//
//            if (_aGTA.getReferenceType().toLowerCase().equals("msoffice"))
//            {
//                sPrintFileURL = URLHelper.getFileURLFromSystemPath(sAbsolutePrintFilename);
//            }
//            else if (_aGTA.getReferenceType().toLowerCase().equals("pdf"))
//            {
////  TODO: If we rename the stored file to *.pdf, we have to be sure that we use *.pdf also as a available reference
//                sPrintFileURL = URLHelper.getFileURLFromSystemPath(sAbsolutePrintFilename );
//            }
//            else if (_aGTA.getReferenceType().toLowerCase().equals("ooo"))
//            {
//                sPrintFileURL = URLHelper.getFileURLFromSystemPath(sAbsolutePrintFilename );
//            }
//            else
//            {
//                GlobalLogWriter.println("OfficePrint.buildreference(): Unknown print type.");
//                return false;
//            }
//            return printToFile(_aGTA, sInputFileURL, sOutputFileURL, sPrintFileURL);
//        }



    // TODO: Das Teil muss hier raus!


//    public static boolean printToFile(ParameterHelper _aGTA,
//                                      String _sInputFileURL,
//                                      String _sOutputFileURL,
//                                      String _sPrintFileURL) throws OfficeException
//        {
//            boolean bBack = false;
//            String sPrintFileURL = null;
//
//
//            // remember the current timer, to know how long a print process need.
//            // startTimer();
//
//            if (_aGTA.getReferenceType().toLowerCase().equals("ooo"))
//            {
//                bBack = printToFileWithOOo(_aGTA, _sInputFileURL, _sOutputFileURL, _sPrintFileURL);
//            }
//            else if (_aGTA.getReferenceType().toLowerCase().equals("pdf"))
//            {
//                GlobalLogWriter.println("USE PDF AS EXPORT FORMAT.");
//                bBack = storeAsPDF(_aGTA, _sInputFileURL, _sPrintFileURL);
//            }
//            else if (_aGTA.getReferenceType().toLowerCase().equals("msoffice"))
//            {
//                if (MSOfficePostscriptCreator.isMSOfficeDocumentFormat(_sInputFileURL))
//                {
//                    GlobalLogWriter.println("USE MSOFFICE AS EXPORT FORMAT.");
//                    MSOfficePostscriptCreator a = new MSOfficePostscriptCreator();
//                    try
//                    {
//                        a.printToFileWithMSOffice(_aGTA, FileHelper.getSystemPathFromFileURL(_sInputFileURL),
//                                                  FileHelper.getSystemPathFromFileURL(_sPrintFileURL));
//                    }
//                    catch(OfficeException e)
//                    {
//                        e.printStackTrace();
//                        GlobalLogWriter.println(e.getMessage());
//                        throw new OfficeException("Exception caught. Problem with MSOffice printer methods.");
//                    }
//                    catch(java.io.IOException e)
//                    {
//                        GlobalLogWriter.println(e.getMessage());
//                        throw new OfficeException("IOException caught. Problem with MSOffice printer methods.");
//                    }
//                    bBack = true;
//                }
//                else
//                {
//                    GlobalLogWriter.println("This document type is not recognized as MSOffice format, as default fallback StarOffice/OpenOffice.org instead is used.");
//                    bBack = printToFileWithOOo(_aGTA, _sInputFileURL, _sOutputFileURL, _sPrintFileURL);
//                }
//            }
//            else
//            {
//                // System.out.println("");
//                throw new OfficeException("OfficePrint.printToFile(): Unknown print type.");
//            }
//            return bBack;
//        }

    // -----------------------------------------------------------------------------
    // TODO: move this away!
    // -----------------------------------------------------------------------------
    void showType(String _sInputURL, XMultiServiceFactory _xMSF)
        {
            if (_sInputURL.length() == 0)
            {
                return;
            }

            if (_xMSF == null)
            {
                GlobalLogWriter.println("MultiServiceFactory not set.");
                return;
            }
            XTypeDetection aTypeDetection = null;
            try
            {
                Object oObj = _xMSF.createInstance("com.sun.star.document.TypeDetection");
                aTypeDetection = UnoRuntime.queryInterface(XTypeDetection.class, oObj);
            }
            catch(com.sun.star.uno.Exception e)
            {
                GlobalLogWriter.println("Can't get com.sun.star.document.TypeDetection.");
                return;
            }
            if (aTypeDetection != null)
            {
                String sType = aTypeDetection.queryTypeByURL(_sInputURL);
                GlobalLogWriter.println("Type is: " + sType);
            }
        }


    // -----------------------------------------------------------------------------
    public String getInternalFilterName(String _sFilterName, XMultiServiceFactory _xMSF)
        {
            if (_sFilterName.length() == 0)
            {
                // System.out.println("No FilterName set.");
                return null;
            }

            if (_xMSF == null)
            {
                GlobalLogWriter.println("MultiServiceFactory not set.");
                return null;
            }
            // XFilterFactory aFilterFactory = null;
            Object aObj = null;
            try
            {
                aObj = _xMSF.createInstance("com.sun.star.document.FilterFactory");
            }
            catch(com.sun.star.uno.Exception e)
            {
                GlobalLogWriter.println("Can't get com.sun.star.document.FilterFactory.");
                return null;
            }
            if (aObj != null)
            {
                XNameAccess aNameAccess = UnoRuntime.queryInterface(XNameAccess.class, aObj);
                if (aNameAccess != null)
                {

                    // if (_sFilterName.toLowerCase().equals("help"))
                    // {
                    //     System.out.println("Show all possible ElementNames from current version." );
                    // String[] aElementNames = aNameAccess.getElementNames();
                    // for (int i = 0; i<aElementNames.length; i++)
                    // {
                    //     System.out.println(aElementNames[i]);
                    // }
                    //     System.out.println("Must quit.");
                    //     System.out.exit(1);
                    // }

                    if (! aNameAccess.hasByName(_sFilterName))
                    {
                        GlobalLogWriter.println("FilterFactory.hasByName() says there exist no '" + _sFilterName + "'" );
                        return null;
                    }

                    Object[] aElements = null;
                    String[] aExtensions;
                    try
                    {
                        aElements = (Object[]) aNameAccess.getByName(_sFilterName);
                        if (aElements != null)
                        {
                            String sInternalFilterName = null;
                            // System.out.println("getByName().length: " + String.valueOf(aElements.length));
                            for (int i=0;i<aElements.length; i++)
                            {
                                PropertyValue aPropertyValue = (PropertyValue)aElements[i];
                                // System.out.println("PropertyValue.Name: " + aPropertyValue.Name);
                                if (aPropertyValue.Name.equals("Type"))
                                {
                                    String sValue = (String)aPropertyValue.Value;
                                    // System.out.println("Type: " + sValue);
                                    sInternalFilterName = sValue;
                                }
                            }
                            return sInternalFilterName;
                        }
                        else
                        {
                            GlobalLogWriter.println("There are no elements for FilterName '" + _sFilterName + "'");
                            return null;
                        }
                    }
                    catch (com.sun.star.container.NoSuchElementException e)
                    {
                        GlobalLogWriter.println("NoSuchElementException caught. " + e.getMessage());
                    }
                    catch (com.sun.star.lang.WrappedTargetException e)
                    {
                        GlobalLogWriter.println("WrappedTargetException caught. " + e.getMessage());
                    }
                }
            }
            return null;
        }

    // -----------------------------------------------------------------------------

    String getServiceNameFromFilterName(String _sFilterName, XMultiServiceFactory _xMSF)
        {
            if (_sFilterName.length() == 0)
            {
                // System.out.println("No FilterName set.");
                return null;
            }

            if (_xMSF == null)
            {
                GlobalLogWriter.println("MultiServiceFactory not set.");
                return null;
            }
            // XFilterFactory aFilterFactory = null;
            Object aObj = null;
            try
            {
                aObj = _xMSF.createInstance("com.sun.star.document.FilterFactory");
            }
            catch(com.sun.star.uno.Exception e)
            {
                GlobalLogWriter.println("Can't get com.sun.star.document.FilterFactory.");
                return null;
            }
            if (aObj != null)
            {
                XNameAccess aNameAccess = UnoRuntime.queryInterface(XNameAccess.class, aObj);
                if (aNameAccess != null)
                {
                    if (! aNameAccess.hasByName(_sFilterName))
                    {
                        GlobalLogWriter.println("FilterFactory.hasByName() says there exist no '" + _sFilterName + "'" );
                        return null;
                    }

                    Object[] aElements = null;
                    String[] aExtensions;
                    try
                    {
                        aElements = (Object[]) aNameAccess.getByName(_sFilterName);
                        if (aElements != null)
                        {
                            String sServiceName = null;
                            // System.out.println("getByName().length: " + String.valueOf(aElements.length));
                            for (int i=0;i<aElements.length; i++)
                            {
                                PropertyValue aPropertyValue = (PropertyValue)aElements[i];
                                if (aPropertyValue.Name.equals("DocumentService"))
                                {
                                    String sValue = (String)aPropertyValue.Value;
                                    // System.out.println("DocumentService: " + sValue);
                                    sServiceName = sValue;
                                    break;
                                }
                            }
                            return sServiceName;
                        }
                        else
                        {
                            GlobalLogWriter.println("There are no elements for FilterName '" + _sFilterName + "'");
                            return null;
                        }
                    }
                    catch (com.sun.star.container.NoSuchElementException e)
                    {
                        GlobalLogWriter.println("NoSuchElementException caught. " + e.getMessage());
                    }
                    catch (com.sun.star.lang.WrappedTargetException e)
                    {
                        GlobalLogWriter.println("WrappedTargetException caught. " + e.getMessage());
                    }
                }
            }
            return null;
        }
    // -----------------------------------------------------------------------------

    public static String getFileExtension(String _sInternalFilterName, XMultiServiceFactory _xMSF)
        {
            if (_sInternalFilterName.length() == 0)
            {
                // System.out.println("No FilterName set.");
                return null;
            }

            if (_xMSF == null)
            {
                GlobalLogWriter.println("MultiServiceFactory not set.");
                return null;
            }
            XTypeDetection aTypeDetection = null;
            try
            {
                Object oObj = _xMSF.createInstance("com.sun.star.document.TypeDetection");
                aTypeDetection = UnoRuntime.queryInterface(XTypeDetection.class, oObj);
            }
            catch(com.sun.star.uno.Exception e)
            {
                GlobalLogWriter.println("Can't get com.sun.star.document.TypeDetection.");
                return null;
            }
            if (aTypeDetection != null)
            {
                XNameAccess aNameAccess = UnoRuntime.queryInterface(XNameAccess.class, aTypeDetection);
                if (aNameAccess != null)
                {

                    // System.out.println("Show ElementNames" );
                    // String[] aElementNames = aNameAccess.getElementNames();
                    // for (int i = 0; i<aElementNames.length; i++)
                    // {
                    //     System.out.println(aElementNames[i]);
                    // }

                    if (! aNameAccess.hasByName(_sInternalFilterName))
                    {
                        GlobalLogWriter.println("TypeDetection.hasByName() says there exist no '" + _sInternalFilterName + "'" );
                        return null;
                    }

                    Object[] aElements = null;
                    String[] aExtensions;
                    try
                    {
                        aElements = (Object[]) aNameAccess.getByName(_sInternalFilterName);
                        if (aElements != null)
                        {
                            String sExtension = null;
                            // System.out.println("getByName().length: " + String.valueOf(aElements.length));
                            for (int i=0;i<aElements.length; i++)
                            {
                                PropertyValue aPropertyValue = (PropertyValue)aElements[i];
                                // System.out.println("PropertyValue.Name: " + aPropertyValue.Name);
                                if (aPropertyValue.Name.equals("Extensions"))
                                {
                                    aExtensions = (String[])aPropertyValue.Value;
                                    GlobalLogWriter.println("   Possible extensions are: " + String.valueOf(aExtensions.length));
                                    if (aExtensions.length > 0)
                                    {
                                        for (int j=0;j<aExtensions.length;j++)
                                        {
                                            GlobalLogWriter.println(" " + aExtensions[j]);
                                        }
                                        sExtension = aExtensions[0];
                                        GlobalLogWriter.println("");
                                    }
                                }
                            }
                            return sExtension;
                        }
                        else
                        {
                            GlobalLogWriter.println("There are no elements for FilterName '" + _sInternalFilterName + "'");
                            return null;
                        }
                    }
                    catch (com.sun.star.container.NoSuchElementException e)
                    {
                        GlobalLogWriter.println("NoSuchElementException caught. " + e.getMessage());
                    }
                    catch (com.sun.star.lang.WrappedTargetException e)
                    {
                        GlobalLogWriter.println("WrappedTargetException caught. " + e.getMessage());
                    }
}
            }
            return null;
        }

    // -----------------------------------------------------------------------------
    public void convertDocument(String _sInputFile, String _sOutputPath, ParameterHelper _aGTA) throws OfficeException
        {
            XMultiServiceFactory xMSF = _aGTA.getMultiServiceFactory();
            if (xMSF == null)
            {
                GlobalLogWriter.println("MultiServiceFactory in GraphicalTestArgument not set.");
                return;
            }

            String sInputURL = URLHelper.getFileURLFromSystemPath(_sInputFile);
            // showType(sInputURL, xMSF);
            XComponent aDoc = loadFromURL( _aGTA, sInputURL);
            if (aDoc == null)
            {
                GlobalLogWriter.println("Can't load document '"+ sInputURL + "'");
                return;
            }

            if (_sOutputPath == null)
            {
                GlobalLogWriter.println("Outputpath not set.");
                return;
            }

            if (! isStoreAllowed())
            {
                GlobalLogWriter.println("It's not allowed to store, check Input/Output path.");
                return;
            }
//  TODO: Do we need to wait?
//            TimeHelper.waitInSeconds(1, "wait after loadFromURL.");

            XServiceInfo xServiceInfo =  UnoRuntime.queryInterface( XServiceInfo.class, aDoc );
            // String sFilter = getFilterName_forExcel(xServiceInfo);
            // System.out.println("Filter is " + sFilter);

            // store the document in an other directory
            XStorable xStorable =  UnoRuntime.queryInterface( XStorable.class, aDoc);
            if (xStorable == null)
            {
                GlobalLogWriter.println("com.sun.star.frame.XStorable is null");
                return;
            }

            String sFilterName = _aGTA.getExportFilterName();

            // check how many Properties should initialize
            int nPropertyCount = 0;
            // if (sFilterName != null && sFilterName.length() > 0)
            // {
            //     nPropertyCount ++;
            // }

            // initialize PropertyArray
            // PropertyValue [] aStoreProps = new PropertyValue[ nPropertyCount ];
            // int nPropertyIndex = 0;
            ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>();

            String sExtension = "";

            if (sFilterName != null && sFilterName.length() > 0)
            {
                String sInternalFilterName = getInternalFilterName(sFilterName, xMSF);
                String sServiceName = getServiceNameFromFilterName(sFilterName, xMSF);

                GlobalLogWriter.println("Filter detection:");
                // check if service name from file filter is the same as from the loaded document
                boolean bServiceFailed = false;
                if (sServiceName == null || sInternalFilterName == null)
                {
                    GlobalLogWriter.println("Given FilterName '" + sFilterName + "' seems to be unknown.");
                    bServiceFailed = true;
                }
                if (! xServiceInfo.supportsService(sServiceName))
                {
                    GlobalLogWriter.println("Service from FilterName '" + sServiceName + "' is not supported by loaded document.");
                    bServiceFailed = true;
                }
                if (bServiceFailed == true)
                {
                    GlobalLogWriter.println("Please check '" + PropertyName.DOC_CONVERTER_EXPORT_FILTER_NAME + "' in the property file.");
                    return;
                }

                if (sInternalFilterName != null && sInternalFilterName.length() > 0)
                {
                    // get the FileExtension, by the filter name, if we don't get a file extension
                    // we assume the is also no right filter name.
                    sExtension = getFileExtension(sInternalFilterName, xMSF);
                    if (sExtension == null)
                    {
                        GlobalLogWriter.println("Can't found an extension for filtername, take it from the source.");
                    }
                }

                PropertyValue Arg = new PropertyValue();
                Arg.Name = "FilterName";
                Arg.Value = sFilterName;
                // aStoreProps[nPropertyIndex ++] = Arg;
                aPropertyList.add(Arg);
                showProperty(Arg);
                GlobalLogWriter.println("FilterName is set to: " + sFilterName);
            }

            String sOutputURL = "";
            try
            {
                // create the new filename with the extension, which is ok to the file format
                String sInputFileBasename = FileHelper.getBasename(_sInputFile);
                // System.out.println("InputFileBasename " + sInputFileBasename);
                String sInputFileNameNoSuffix = FileHelper.getNameNoSuffix(sInputFileBasename);
                // System.out.println("InputFilename no suffix " + sInputFileNameNoSuffix);
                String fs = System.getProperty("file.separator");
                String sOutputFile = _sOutputPath;
                if (! sOutputFile.endsWith(fs))
                {
                    sOutputFile += fs;
                }
                if (sExtension != null && sExtension.length() > 0)
                {
                    sOutputFile += sInputFileNameNoSuffix + "." + sExtension;
                }
                else
                {
                    sOutputFile += sInputFileBasename;
                }

                if (FileHelper.exists(sOutputFile) && _aGTA.getOverwrite() == false)
                {
                    GlobalLogWriter.println("File already exist, don't overwrite. Set " + PropertyName.DOC_COMPARATOR_OVERWRITE_REFERENCE + "=true to force overwrite.");
                    return;
                }

                sOutputURL = URLHelper.getFileURLFromSystemPath(sOutputFile);

                GlobalLogWriter.println("Store document as '" + sOutputURL + "'");
                xStorable.storeAsURL(sOutputURL, PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList));
                GlobalLogWriter.println("Document stored.");
            }
            catch (com.sun.star.io.IOException e)
            {
                GlobalLogWriter.println("Can't store document '" + sOutputURL + "'. Message is :'" + e.getMessage() + "'");
            }
//  TODO: Do we need to wait?
//            TimeHelper.waitInSeconds(1, "unknown in OfficePrint.convertDocument()");

        }

    /**
     *
     * @return false, if 'NoOffice=yes' is given
     */
//    private boolean shouldOfficeStart()
//    {
//        String sNoOffice = (String)m_aParameterHelper.getTestParameters().get( "NoOffice" );
//        if (sNoOffice != null)
//        {
//            if (sNoOffice.toLowerCase().startsWith("t") || sNoOffice.toLowerCase().startsWith("y"))
//            {
//                return false;
//            }
//        }
//        return true;
//    }

    OfficeProvider m_aProvider = null;
    private void startOffice()
    {
        // SimpleFileSemaphore aSemaphore = new SimpleFileSemaphore();
//        if (shouldOfficeStart())
//        {
            // if (OSHelper.isWindows())
            // {
            //     aSemaphore.P(aSemaphore.getSemaphoreFile());
            // }
            m_aParameterHelper.getTestParameters().put(util.PropertyName.DONT_BACKUP_USERLAYER, Boolean.TRUE);

            m_aParameterHelper.getPerformance().startTime(PerformanceContainer.OfficeStart);
            m_aProvider = new OfficeProvider();
            XMultiServiceFactory xMSF = (XMultiServiceFactory) m_aProvider.getManager(m_aParameterHelper.getTestParameters());
            m_aParameterHelper.getTestParameters().put("ServiceFactory", xMSF);
            m_aParameterHelper.getPerformance().stopTime(PerformanceContainer.OfficeStart);

            long nStartTime = m_aParameterHelper.getPerformance().getTime(PerformanceContainer.OfficeStart);
            // aGTA = getParameterHelper(); // get new TestArguments
            m_aParameterHelper.getPerformance().setTime(PerformanceContainer.OfficeStart, nStartTime);
//        }

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
            // if (OSHelper.isWindows())
            // {
            //     aSemaphore.V(aSemaphore.getSemaphoreFile());
            //     aSemaphore.sleep(2);
            //     // wait some time maybe an other process will take the semaphore
            //     // I know, this is absolutly dirty, but the whole convwatch is dirty and need a big cleanup.
            // }
        }
    }

    private boolean m_bStoreFile;
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
        return false;
        // return m_bStoreFile;
        }

}

