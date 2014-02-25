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

import java.io.File;
import java.util.ArrayList;
import java.io.FileWriter;

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
// import convwatch.FileHelper;
// import convwatch.MSOfficePrint;
// import convwatch.GraphicalTestArguments;
// import convwatch.ConvWatchCancelException;

// import helper.Parameter;

/**
 * This Object is to print a given document with OpenOffice.org / StarOffice
 * over the normal printer driver
 * or over it's pdf exporter
 */
public class OfficePrint {


//     static long m_nStartTime;
//     // static Date m_aDateCache = null;

//     /*
//       simple helper functions to start/stop a timer, to know how long a process need in milliseconds
//      */
//     public static void startTimer()
//         {
//             // if (m_aDateCache == null)
//             // {
//             //     m_aDateCache = new Date();
//             // }
//             // m_nStartTime = m_aDateCache.getTime();
//             m_nStartTime = System.currentTimeMillis();
//         }
//     public static long stopTimer()
//         {
//             // if (m_aDateCache == null)
//             // {
//             //     System.out.println("Forgotten to initialise start timer.");
//             //     return 0;
//             // }
//             // long m_nStopTime = m_aDateCache.getTime();
//             if (m_nStartTime == 0)
//             {
//                 System.out.println("Forgotten to initialise start timer.");
//                 return 0;
//             }
//             long m_nStopTime = System.currentTimeMillis();
//             return m_nStopTime - m_nStartTime;
//         }


    private static void showProperty(PropertyValue _aValue)
        {
            String sName = _aValue.Name;
            String sValue;
            try
            {
                sValue = AnyConverter.toString(_aValue.Value);
                GlobalLogWriter.get().println("Property " + sName + ":=" + sValue);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                // GlobalLogWriter.get().println("showProperty: can't convert a object to string.");
                GlobalLogWriter.get().println("Property " + sName + ":= a Object which can't convert by AnyConverter()");
            }
        }

    /**
     * shows the FilterName and MediaType from the given XComponent
     */
    static String getDocumentType( XComponent _aDoc )
        {
            XModel xModel =  UnoRuntime.queryInterface( XModel.class, _aDoc);
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

    static void showDocumentType( XComponent _aDoc )
        {
            String sNameValue = getDocumentType(_aDoc);
            GlobalLogWriter.get().println("  Property: '" + sNameValue);
        }
    /**
     * load a OpenOffice.org document from a given URL (_sInputURL)
     * the GraphicalTestArguments must contain a living MultiServiceFactory object
     * or we crash here.
     * Be aware, the ownership of the document gets to you, you have to close it.
     * @param _aGTA
     * @param _sInputURL
     * @return
     */
    public static XComponent loadFromURL(GraphicalTestArguments _aGTA,
                                         String _sInputURL)
        {
            XComponent aDoc = null;
            try
            {
                if (_aGTA.getMultiServiceFactory() == null)
                {
                    GlobalLogWriter.get().println("MultiServiceFactory in GraphicalTestArgument not set.");
                    return null;
                }
                Object oDsk = _aGTA.getMultiServiceFactory().createInstance("com.sun.star.frame.Desktop");
                XDesktop aDesktop = UnoRuntime.queryInterface(XDesktop.class, oDsk);

                if (aDesktop != null)
                {
                    GlobalLogWriter.get().println("com.sun.star.frame.Desktop created.");
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

                    GlobalLogWriter.get().println(DateHelper.getDateTimeForHumanreadableLog() + " Load document");
                    // GlobalLogWriter.get().flush();

                    XComponentLoader aCompLoader = UnoRuntime.queryInterface( XComponentLoader.class, aDesktop);

                    // XComponent aDoc = null;

                    _aGTA.getPerformance().startTime(PerformanceContainer.Load);
                    aDoc = aCompLoader.loadComponentFromURL(_sInputURL, "_blank", 0, PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList) );
                    _aGTA.getPerformance().stopTime(PerformanceContainer.Load);
                    if (aDoc != null)
                    {
                        GlobalLogWriter.get().println(DateHelper.getDateTimeForHumanreadableLog() + " Load document done.");
                        showDocumentType(aDoc);
                        _aGTA.setDocumentType(getDocumentType(aDoc));
                    }
                    else
                    {
                        GlobalLogWriter.get().println(" Load document failed.");
                        if (_aGTA.getImportFilterName() != null && _aGTA.getImportFilterName().length() > 0)
                        {
                            GlobalLogWriter.get().println(" Please check FilterName := '" + _aGTA.getImportFilterName() + "'");
                        }
                        GlobalLogWriter.get().println("");
                    }
                }
                else
                {
                    GlobalLogWriter.get().println("com.sun.star.frame.Desktop failed.");
                }
            }
            catch ( com.sun.star.uno.Exception e )
            {
                // Some exception occurs.FAILED
                GlobalLogWriter.get().println("UNO Exception caught.");
                GlobalLogWriter.get().println("Message: " + e.getMessage());
                e.printStackTrace();
                aDoc = null;
            }
            return aDoc;
        }

    static boolean exportToPDF(XComponent _xComponent, String _sDestinationName)
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
                GlobalLogWriter.get().println("IO Exception caught.");
                GlobalLogWriter.get().println("Message: " + e.getMessage());
                bWorked = false;
            }

            return bWorked;
        }

    static String getFilterName_forPDF(XServiceInfo xServiceInfo)
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



    public static boolean storeAsPDF(GraphicalTestArguments _aGTA,
                                     String _sInputURL,
                                     String _sOutputURL)
        {
            boolean bBack = false;
            XComponent aDoc = loadFromURL(_aGTA, _sInputURL);

            if (aDoc == null)
            {
                GlobalLogWriter.get().println("Can't load document.");
                return bBack;
            }
            bBack = storeAsPDF(_aGTA, aDoc, _sOutputURL);
            createInfoFile(_sOutputURL, _aGTA, "as pdf");

            GlobalLogWriter.get().println("Close document.");
            aDoc.dispose();
            return bBack;
        }

    public static boolean storeAsPDF(GraphicalTestArguments _aGTA,
                                     XComponent _aDoc,
                                     String _sOutputURL)
        {
            // try {
            boolean bBack = true;
            _aGTA.getPerformance().startTime(PerformanceContainer.StoreAsPDF);
            bBack = exportToPDF(_aDoc, _sOutputURL);
            _aGTA.getPerformance().stopTime(PerformanceContainer.StoreAsPDF);

            if (!bBack)
            {
                GlobalLogWriter.get().println("Can't store document as PDF.");
                bBack = false;
            }
            return bBack;
        }



    /**
     * print the document found in file (_sInputURL) to as postscript to file (_sPrintFileURL)
     * Due to the fact we use a printer to convert the file to postscript, the default printer
     * to create such postscript format must be installed, this is not tested here.
     *
     * @param _aGTA
     * @param _sInputURL
     * @param _sOutputURL
     * @param _sPrintFileURL
     * @return true, if print has been done.
     *         Be careful, true means only print returns with no errors, to be sure print is really done
     *         check existance of _sPrintFileURL
     */

    public static boolean printToFileWithOOo(GraphicalTestArguments _aGTA,
                                             String _sInputURL,
                                             String _sOutputURL,
                                             String _sPrintFileURL)
        {
            // waitInSeconds(1);
            boolean bBack = false;

            XComponent aDoc = loadFromURL(_aGTA, _sInputURL);
            if (aDoc != null)
            {
                if ( _sInputURL.equals(_sOutputURL) )
                {
                    // don't store document
                    // input and output are equal OR
                    GlobalLogWriter.get().println("Warning: Inputpath and Outputpath are equal. Document will not stored again.");
                    _aGTA.disallowStore();
                }
                bBack = impl_printToFileWithOOo(_aGTA, aDoc, _sOutputURL, _sPrintFileURL);

                GlobalLogWriter.get().println("Close document.");
                aDoc.dispose();
            }
            else
            {
                GlobalLogWriter.get().println("loadDocumentFromURL() failed with document: " + _sInputURL);
            }
            return bBack;
        }


    public static void createInfoFile(String _sFile, GraphicalTestArguments _aGTA)
        {
            createInfoFile(_sFile, _aGTA, "");
        }

    public static void createInfoFile(String _sFile, GraphicalTestArguments _aGTA, String _sSpecial)
        {
            String sFilename;
            if (_sFile.startsWith("file://"))
            {
                sFilename = FileHelper.getSystemPathFromFileURL(_sFile);
                GlobalLogWriter.get().println("CreateInfoFile: '" + sFilename + "'" );
            }
            else
            {
                sFilename = _sFile;
            }
            String sFileDir = FileHelper.getPath(sFilename);
            String sBasename = FileHelper.getBasename(sFilename);
            String sNameNoSuffix = FileHelper.getNameNoSuffix(sBasename);

            String fs = System.getProperty("file.separator");
            String ls = System.getProperty("line.separator");
            String sInfoFilename = sFileDir + fs + sNameNoSuffix + ".info";
            File aInfoFile = new File(sInfoFilename);

            String sBuildID = "";

            try
            {
                FileWriter out = new FileWriter(aInfoFile.toString());
                out.write("# automatically created file by graphical compare" + ls);
                if (_aGTA != null)
                {
                    if (_sSpecial != null && _sSpecial.equals("msoffice"))
                    {
                        out.write("# buildid from wordloadfile" + ls);
                        sBuildID = _aGTA.getPerformance().getMSOfficeVersion();
                        out.write("buildid=" + sBuildID + ls);
                    }
                    else
                    {
                        out.write("# buildid is read out of the bootstrap file" + ls);
                        sBuildID = _aGTA.getBuildID();
                        out.write("buildid=" + sBuildID + ls);
                    }
                    // if (_sSpecial != null && _sSpecial.length() > 0)
                    // {
                    //    out.write("special=" + _sSpecial + ls);
                    // }
                    out.write(ls);
                    out.write("# resolution given in DPI" + ls);
                    out.write("resolution=" + _aGTA.getResolutionInDPI() + ls);
                }
                else
                {
                    out.write("buildid=" + _sSpecial + ls);
                }
                // long nTime = stopTimer();
                // if (nTime != 0)
                // {
                //     out.write("# time is given in milli seconds" + ls);
                //     out.write("time=" + nTime + ls);
                // }

                out.write(ls);
                out.write("# Values out of System.getProperty(...)" + ls);
                out.write("os.name=" + System.getProperty("os.name") + ls);
                out.write("os.arch=" + System.getProperty("os.arch") + ls);
                out.write("os.version=" + System.getProperty("os.version") + ls);

                if (_aGTA != null)
                {
                    out.write(ls);
                    out.write("# Performance output, values are given in milli sec." + ls);
                    _aGTA.getPerformance().print(out);
                }

                out.flush();
                out.close();
            }
            catch (java.io.IOException e)
            {
                GlobalLogWriter.get().println("can't create Info file.");
                e.printStackTrace();
            }

            String sExtension = FileHelper.getSuffix(_aGTA.getInputFile());
            if (sExtension.startsWith("."))
            {
                sExtension = sExtension.substring(1);
            }

            DB.writeToDB(_aGTA.getInputFile(),
                         sNameNoSuffix,
                         sExtension,
                         sBuildID,
                         _aGTA.getReferenceType(),
                         _aGTA.getResolutionInDPI()
                         );
        }




    private static boolean impl_printToFileWithOOo(GraphicalTestArguments _aGTA,
                                                   XComponent _aDoc,
                                                   String _sOutputURL,
                                                   String _sPrintFileURL)
        {
            boolean bBack = false;
            boolean bFailed = true;              // always be a pessimist,
            if (_aDoc == null)
            {
                GlobalLogWriter.get().println("No document is given.");
                return bBack;
            }

            try
            {
                if (_sOutputURL != null)
                {
                    if (_aGTA.isStoreAllowed())
                    {
                        // store the document in an other directory
                        XStorable aStorable = UnoRuntime.queryInterface( XStorable.class, _aDoc);
                        if (aStorable != null)
                        {
                            PropertyValue [] szEmptyArgs = new PropertyValue [0];

                            GlobalLogWriter.get().println(DateHelper.getDateTimeForHumanreadableLog() + " Store document.");
                            _aGTA.getPerformance().startTime(PerformanceContainer.Store);
                            aStorable.storeAsURL(_sOutputURL, szEmptyArgs);
                            _aGTA.getPerformance().stopTime(PerformanceContainer.Store);

                            GlobalLogWriter.get().println(DateHelper.getDateTimeForHumanreadableLog() + " Store document done.");
                            TimeHelper.waitInSeconds(2, "After store as URL to:" + _sOutputURL);
                            GlobalLogWriter.get().println("Reload stored file test.");
                            XComponent aDoc = loadFromURL(_aGTA, _sOutputURL);
                            if (aDoc == null)
                            {
                                GlobalLogWriter.get().println("Reload stored file test failed, can't reload file: " + _sOutputURL);
                            }
                        }
                    }
                }
            }
            catch ( com.sun.star.uno.Exception e )
            {
                // Some exception occurs.FAILED
                GlobalLogWriter.get().println("UNO Exception caught.");
                GlobalLogWriter.get().println("Message: " + e.getMessage());

                e.printStackTrace();
                bBack = false;
            }

            try
            {

                // System.out.println("Document loaded.");
                // Change Pagesettings to DIN A4

                GlobalLogWriter.get().println(DateHelper.getDateTimeForHumanreadableLog() + " Print document.");
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
                            // GlobalLogWriter.get().println("Printername is not null, so set to " + _aGTA.getPrinterName());
                            aPrintable.setPrinter(PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList));
                        }
                    }

                    // set property values for XPrintable.print()
                    // more can be found at "http://api.libreoffice.org/docs/common/ref/com/sun/star/view/PrintOptions.html"

                    // int nProperties = 1;                    // default for 'FileName' property
                    // if (_aGTA.printAllPages() == false)
                    // {
                    //     // we don't want to print all pages, build Pages string by ourself
                    //     nProperties ++;
                    // }
                    // int nPropsCount = 0;

                    // If we are a SpreadSheet (calc), we need to set PrintAllSheets property to 'true'
                    XServiceInfo xServiceInfo = UnoRuntime.queryInterface( XServiceInfo.class, _aDoc );
                    if ( xServiceInfo.supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
                    {
                        XMultiServiceFactory xMSF = _aGTA.getMultiServiceFactory();
                        Object aSettings = xMSF.createInstance( "com.sun.star.sheet.GlobalSheetSettings" );
                        if (aSettings != null)
                        {
                            XPropertySet xPropSet =  UnoRuntime.queryInterface( XPropertySet.class, aSettings );
                            xPropSet.setPropertyValue( "PrintAllSheets", new Boolean( true ) );
                            GlobalLogWriter.get().println("PrintAllSheets := true");
                        }
                    }

                    ArrayList<PropertyValue> aPrintProps = new ArrayList<PropertyValue>();
                    // GlobalLogWriter.get().println("Property FileName:=" + _sPrintFileURL);

                    // PropertyValue [] aPrintProps = new PropertyValue[nProperties];
                    PropertyValue Arg = new PropertyValue();
                    Arg.Name = "FileName";
                    Arg.Value = _sPrintFileURL;
                    // aPrintProps[nPropsCount ++] = Arg;
                    aPrintProps.add(Arg);
                    showProperty(Arg);

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

                    // GlobalLogWriter.get().println("Start printing.");

                    _aGTA.getPerformance().startTime(PerformanceContainer.Print);
                    aPrintable.print(PropertyHelper.createPropertyValueArrayFormArrayList(aPrintProps));
                    TimeHelper.waitInSeconds(1, "Start waiting for print ready.");

                    GlobalLogWriter.get().println("Wait until document is printed.");
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
                            GlobalLogWriter.get().println("ERROR: Cancel print due to too long wait.");
                            throw new com.sun.star.uno.Exception("Convwatch exception, wait too long for printing.");
                        }
                    }
                    _aGTA.getPerformance().stopTime(PerformanceContainer.Print);
                    GlobalLogWriter.get().println(DateHelper.getDateTimeForHumanreadableLog() + " Print document done.");

                    // Create a .info file near the printed '.ps' or '.prn' file.
                    createInfoFile(_sPrintFileURL, _aGTA);
                }
                else
                {
                    GlobalLogWriter.get().println("Can't get XPrintable interface.");
                }
                bFailed = false;
                bBack = true;
            }
            catch ( com.sun.star.uno.Exception e )
            {
                // Some exception occurs.FAILED
                GlobalLogWriter.get().println("UNO Exception caught.");
                GlobalLogWriter.get().println("Message: " + e.getMessage());

                e.printStackTrace();
                bBack = false;
            }

            if (bFailed == true)
            {
                GlobalLogWriter.get().println("convwatch.OfficePrint: FAILED");
            }
            else
            {
                GlobalLogWriter.get().println("convwatch.OfficePrint: OK");
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
    public static boolean isReferenceExists(GraphicalTestArguments _aGTA,
                                            String _sAbsoluteOutputPath,
                                            String _sAbsoluteInputFile)
        {
            if (! FileHelper.exists(_sAbsoluteInputFile))
            {
                // throw new ConvWatchCancelException("Input file: " + _sAbsoluteInputFile + " does not exist.");
                return false;
            }

            String fs = System.getProperty("file.separator");

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

            String sAbsolutePrintFilename = sOutputPath + fs + sPrintFilename + ".prn";
            if (FileHelper.exists(sAbsolutePrintFilename) && _aGTA.getOverwrite() == false)
            {
                GlobalLogWriter.get().println("Reference already exist, don't overwrite. Set " + PropertyName.DOC_COMPARATOR_OVERWRITE_REFERENCE + "=true to force overwrite.");
                return true;
            }
            return false;
        }


    /**
     * create a reference file
     * _sAbsoluteInputPath  contains the source file, if not exists, return with failure.
     * _sAbsoluteOutputPath contains the destination, where the file will store after load with StarOffice/OpenOffice.org
     *                      if is null, print only near the Input file path
     * _sPrintType ".prn" Print input file with StarOffice/OpenOffice.org and the default printer as PostScript
     *
     * @param _aGTA
     * @param _sAbsoluteOutputPath
     * @param _sAbsoluteInputFile
     * @return
     * @throws ConvWatchCancelException
     */
    public static boolean buildReference(GraphicalTestArguments _aGTA,
                                         String _sAbsoluteOutputPath,
                                         String _sAbsoluteInputFile)
        throws ConvWatchCancelException
        {
            if (! FileHelper.exists(_sAbsoluteInputFile))
            {
                throw new ConvWatchCancelException("buildReference(): Input file: " + _sAbsoluteInputFile + " does not exist.");
            }

            String fs = System.getProperty("file.separator");

            String sInputFileURL = URLHelper.getFileURLFromSystemPath(_sAbsoluteInputFile);

            String sInputFileBasename = FileHelper.getBasename(_sAbsoluteInputFile);
            String sOutputFileURL = null;
            String sOutputPath;
            if (_sAbsoluteOutputPath != null)
            {
                sOutputPath    = _sAbsoluteOutputPath;
                FileHelper.makeDirectories("", sOutputPath);
            }
            else
            {
                String sInputPath = FileHelper.getPath(_sAbsoluteInputFile);
                sOutputPath    = sInputPath;
            }
            // sOutputFileURL = URLHelper.getFileURLFromSystemPath(sOutputPath + fs + sInputFileBasename);
            sOutputFileURL = null;

            String sPrintFilename = FileHelper.getNameNoSuffix(sInputFileBasename);
            String sPrintFileURL;

            String sAbsolutePrintFilename = sOutputPath + fs + sPrintFilename + ".prn";
            if (FileHelper.exists(sAbsolutePrintFilename) && _aGTA.getOverwrite() == false)
            {
                GlobalLogWriter.get().println("Reference already exist, don't overwrite. Set " + PropertyName.DOC_COMPARATOR_OVERWRITE_REFERENCE + "=true to force overwrite.");
                return true;
            }

            if (_aGTA.getReferenceType().toLowerCase().equals("msoffice"))
            {
                sPrintFileURL = URLHelper.getFileURLFromSystemPath(sAbsolutePrintFilename);
            }
            else if (_aGTA.getReferenceType().toLowerCase().equals("pdf"))
            {
//  TODO: If we rename the stored file to *.pdf, we have to be sure that we use *.pdf also as a available reference
                sPrintFileURL = URLHelper.getFileURLFromSystemPath(sAbsolutePrintFilename );
            }
            else if (_aGTA.getReferenceType().toLowerCase().equals("ooo"))
            {
                sPrintFileURL = URLHelper.getFileURLFromSystemPath(sAbsolutePrintFilename );
            }
            else
            {
                GlobalLogWriter.get().println("OfficePrint.buildreference(): Unknown print type.");
                return false;
            }
            return printToFile(_aGTA, sInputFileURL, sOutputFileURL, sPrintFileURL);
        }

    public static boolean printToFile(GraphicalTestArguments _aGTA,
                                      String _sInputFileURL,
                                      String _sOutputFileURL,
                                      String _sPrintFileURL) throws ConvWatchCancelException
        {
            boolean bBack = false;
            // check if given file is a picture, then do nothing
            String sDocumentSuffix = FileHelper.getSuffix(_sInputFileURL);
            if (sDocumentSuffix.toLowerCase().endsWith(".png") ||
                sDocumentSuffix.toLowerCase().endsWith(".gif") ||
                sDocumentSuffix.toLowerCase().endsWith(".jpg") ||
                sDocumentSuffix.toLowerCase().endsWith(".bmp"))
            {
                return false;
            }


            // remember the current timer, to know how long a print process need.
            // startTimer();

            if (_aGTA.getReferenceType().toLowerCase().equals("ooo"))
            {
                bBack = printToFileWithOOo(_aGTA, _sInputFileURL, _sOutputFileURL, _sPrintFileURL);
            }
            else if (_aGTA.getReferenceType().toLowerCase().equals("pdf"))
            {
                GlobalLogWriter.get().println("USE PDF AS EXPORT FORMAT.");
                bBack = storeAsPDF(_aGTA, _sInputFileURL, _sPrintFileURL);
            }
            else if (_aGTA.getReferenceType().toLowerCase().equals("msoffice"))
            {
                if (MSOfficePrint.isMSOfficeDocumentFormat(_sInputFileURL))
                {
                    GlobalLogWriter.get().println("USE MSOFFICE AS EXPORT FORMAT.");
                    MSOfficePrint a = new MSOfficePrint();
                    try
                    {
                        a.printToFileWithMSOffice(_aGTA, FileHelper.getSystemPathFromFileURL(_sInputFileURL),
                                                  FileHelper.getSystemPathFromFileURL(_sPrintFileURL));
                    }
                    catch(ConvWatchCancelException e)
                    {
                        e.printStackTrace();
                        GlobalLogWriter.get().println(e.getMessage());
                        throw new ConvWatchCancelException("Exception caught. Problem with MSOffice printer methods.");
                    }
                    catch(java.io.IOException e)
                    {
                        GlobalLogWriter.get().println(e.getMessage());
                        throw new ConvWatchCancelException("IOException caught. Problem with MSOffice printer methods.");
                    }
                    bBack = true;
                }
                else
                {
                    GlobalLogWriter.get().println("This document type is not recognized as MSOffice format, as default fallback StarOffice/OpenOffice.org instead is used.");
                    bBack = printToFileWithOOo(_aGTA, _sInputFileURL, _sOutputFileURL, _sPrintFileURL);
                }
            }
            else
            {
                // System.out.println("");
                throw new ConvWatchCancelException("OfficePrint.printToFile(): Unknown print type.");
            }
            return bBack;
        }


    // TODO: move this away!

    static void showType(String _sInputURL, XMultiServiceFactory _xMSF)
        {
            if (_sInputURL.length() == 0)
            {
                return;
            }

            if (_xMSF == null)
            {
                GlobalLogWriter.get().println("MultiServiceFactory not set.");
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
                GlobalLogWriter.get().println("Can't get com.sun.star.document.TypeDetection.");
                return;
            }
            if (aTypeDetection != null)
            {
                String sType = aTypeDetection.queryTypeByURL(_sInputURL);
                GlobalLogWriter.get().println("Type is: " + sType);
            }
        }



    public static String getInternalFilterName(String _sFilterName, XMultiServiceFactory _xMSF)
        {
            if (_sFilterName.length() == 0)
            {
                // System.out.println("No FilterName set.");
                return null;
            }

            if (_xMSF == null)
            {
                GlobalLogWriter.get().println("MultiServiceFactory not set.");
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
                GlobalLogWriter.get().println("Can't get com.sun.star.document.FilterFactory.");
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
                        GlobalLogWriter.get().println("FilterFactory.hasByName() says there exist no '" + _sFilterName + "'" );
                        return null;
                    }

                    Object[] aElements = null;
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
                            GlobalLogWriter.get().println("There are no elements for FilterName '" + _sFilterName + "'");
                            return null;
                        }
                    }
                    catch (com.sun.star.container.NoSuchElementException e)
                    {
                        GlobalLogWriter.get().println("NoSuchElementException caught. " + e.getMessage());
                    }
                    catch (com.sun.star.lang.WrappedTargetException e)
                    {
                        GlobalLogWriter.get().println("WrappedTargetException caught. " + e.getMessage());
                    }
                }
            }
            return null;
        }



    static String getServiceNameFromFilterName(String _sFilterName, XMultiServiceFactory _xMSF)
        {
            if (_sFilterName.length() == 0)
            {
                // System.out.println("No FilterName set.");
                return null;
            }

            if (_xMSF == null)
            {
                GlobalLogWriter.get().println("MultiServiceFactory not set.");
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
                GlobalLogWriter.get().println("Can't get com.sun.star.document.FilterFactory.");
                return null;
            }
            if (aObj != null)
            {
                XNameAccess aNameAccess = UnoRuntime.queryInterface(XNameAccess.class, aObj);
                if (aNameAccess != null)
                {
                    if (! aNameAccess.hasByName(_sFilterName))
                    {
                        GlobalLogWriter.get().println("FilterFactory.hasByName() says there exist no '" + _sFilterName + "'" );
                        return null;
                    }

                    Object[] aElements = null;
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
                            GlobalLogWriter.get().println("There are no elements for FilterName '" + _sFilterName + "'");
                            return null;
                        }
                    }
                    catch (com.sun.star.container.NoSuchElementException e)
                    {
                        GlobalLogWriter.get().println("NoSuchElementException caught. " + e.getMessage());
                    }
                    catch (com.sun.star.lang.WrappedTargetException e)
                    {
                        GlobalLogWriter.get().println("WrappedTargetException caught. " + e.getMessage());
                    }
                }
            }
            return null;
        }


    public static String getFileExtension(String _sInternalFilterName, XMultiServiceFactory _xMSF)
        {
            if (_sInternalFilterName.length() == 0)
            {
                // System.out.println("No FilterName set.");
                return null;
            }

            if (_xMSF == null)
            {
                GlobalLogWriter.get().println("MultiServiceFactory not set.");
                return null;
            }
            XTypeDetection aTypeDetection = null;
            try
            {
                Object oObj = _xMSF.createInstance("com.sun.star.document.TypeDetection");
                aTypeDetection =UnoRuntime.queryInterface(XTypeDetection.class, oObj);
            }
            catch(com.sun.star.uno.Exception e)
            {
                GlobalLogWriter.get().println("Can't get com.sun.star.document.TypeDetection.");
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
                        GlobalLogWriter.get().println("TypeDetection.hasByName() says there exist no '" + _sInternalFilterName + "'" );
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
                                    GlobalLogWriter.get().println("   Possible extensions are: " + String.valueOf(aExtensions.length));
                                    if (aExtensions.length > 0)
                                    {
                                        for (int j=0;j<aExtensions.length;j++)
                                        {
                                            GlobalLogWriter.get().println(" " + aExtensions[j]);
                                        }
                                        sExtension = aExtensions[0];
                                        GlobalLogWriter.get().println("");
                                    }
                                }
                            }
                            return sExtension;
                        }
                        else
                        {
                            GlobalLogWriter.get().println("There are no elements for FilterName '" + _sInternalFilterName + "'");
                            return null;
                        }
                    }
                    catch (com.sun.star.container.NoSuchElementException e)
                    {
                        GlobalLogWriter.get().println("NoSuchElementException caught. " + e.getMessage());
                    }
                    catch (com.sun.star.lang.WrappedTargetException e)
                    {
                        GlobalLogWriter.get().println("WrappedTargetException caught. " + e.getMessage());
                    }
}
            }
            return null;
        }


    public static void convertDocument(String _sInputFile, String _sOutputPath, GraphicalTestArguments _aGTA) throws ConvWatchCancelException
        {
            XMultiServiceFactory xMSF = _aGTA.getMultiServiceFactory();
            if (xMSF == null)
            {
                GlobalLogWriter.get().println("MultiServiceFactory in GraphicalTestArgument not set.");
                return;
            }

            String sInputURL = URLHelper.getFileURLFromSystemPath(_sInputFile);
            // showType(sInputURL, xMSF);
            XComponent aDoc = loadFromURL( _aGTA, sInputURL);
            if (aDoc == null)
            {
                GlobalLogWriter.get().println("Can't load document '"+ sInputURL + "'");
                return;
            }

            if (_sOutputPath == null)
            {
                GlobalLogWriter.get().println("Outputpath not set.");
                return;
            }

            if (! _aGTA.isStoreAllowed())
            {
                GlobalLogWriter.get().println("It's not allowed to store, check Input/Output path.");
                return;
            }
//  TODO: Do we need to wait?
            TimeHelper.waitInSeconds(1, "wait after loadFromURL.");

            XServiceInfo xServiceInfo = UnoRuntime.queryInterface( XServiceInfo.class, aDoc );
            // String sFilter = getFilterName_forExcel(xServiceInfo);
            // System.out.println("Filter is " + sFilter);

            // store the document in an other directory
            XStorable xStorable = UnoRuntime.queryInterface( XStorable.class, aDoc);
            if (xStorable == null)
            {
                GlobalLogWriter.get().println("com.sun.star.frame.XStorable is null");
                return;
            }

            String sFilterName = _aGTA.getExportFilterName();

            // initialize PropertyArray
            // PropertyValue [] aStoreProps = new PropertyValue[ nPropertyCount ];
            // int nPropertyIndex = 0;
            ArrayList<PropertyValue> aPropertyList = new ArrayList<PropertyValue>();

            String sExtension = "";

            if (sFilterName != null && sFilterName.length() > 0)
            {
                String sInternalFilterName = getInternalFilterName(sFilterName, xMSF);
                String sServiceName = getServiceNameFromFilterName(sFilterName, xMSF);

                GlobalLogWriter.get().println("Filter detection:");
                // check if service name from file filter is the same as from the loaded document
                boolean bServiceFailed = false;
                if (sServiceName == null || sInternalFilterName == null)
                {
                    GlobalLogWriter.get().println("Given FilterName '" + sFilterName + "' seems to be unknown.");
                    bServiceFailed = true;
                }
                if (! xServiceInfo.supportsService(sServiceName))
                {
                    GlobalLogWriter.get().println("Service from FilterName '" + sServiceName + "' is not supported by loaded document.");
                    bServiceFailed = true;
                }
                if (bServiceFailed == true)
                {
                    GlobalLogWriter.get().println("Please check '" + PropertyName.DOC_CONVERTER_EXPORT_FILTER_NAME + "' in the property file.");
                    return;
                }

                if (sInternalFilterName != null && sInternalFilterName.length() > 0)
                {
                    // get the FileExtension, by the filter name, if we don't get a file extension
                    // we assume the is also no right filter name.
                    sExtension = getFileExtension(sInternalFilterName, xMSF);
                    if (sExtension == null)
                    {
                        GlobalLogWriter.get().println("Can't found an extension for filtername, take it from the source.");
                    }
                }

                PropertyValue Arg = new PropertyValue();
                Arg.Name = "FilterName";
                Arg.Value = sFilterName;
                // aStoreProps[nPropertyIndex ++] = Arg;
                aPropertyList.add(Arg);
                showProperty(Arg);
                GlobalLogWriter.get().println("FilterName is set to: " + sFilterName);
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
                    GlobalLogWriter.get().println("File already exist, don't overwrite. Set " + PropertyName.DOC_COMPARATOR_OVERWRITE_REFERENCE + "=true to force overwrite.");
                    return;
                }

                sOutputURL = URLHelper.getFileURLFromSystemPath(sOutputFile);

                GlobalLogWriter.get().println("Store document as '" + sOutputURL + "'");
                xStorable.storeAsURL(sOutputURL, PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList));
                GlobalLogWriter.get().println("Document stored.");
            }
            catch (com.sun.star.io.IOException e)
            {
                GlobalLogWriter.get().println("Can't store document '" + sOutputURL + "'. Message is :'" + e.getMessage() + "'");
            }
//  TODO: Do we need to wait?
            TimeHelper.waitInSeconds(1, "unknown in OfficePrint.convertDocument()");

        }

}

