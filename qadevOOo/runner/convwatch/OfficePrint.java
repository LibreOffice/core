/*************************************************************************
 *
 *  $RCSfile: OfficePrint.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:20:33 $
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

import java.io.File;
import java.io.FileWriter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.frame.XDesktop;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XComponent;
import com.sun.star.frame.XStorable;
import com.sun.star.view.XPrintable;
import com.sun.star.lang.XServiceInfo;

import helper.URLHelper;
import convwatch.FileHelper;
import convwatch.MSOfficePrint;
import convwatch.GraphicalTestArguments;
import convwatch.ConvWatchCancelException;

// import helper.Parameter;

/**
 * This Object is to print a given document with OpenOffice.org / StarOffice
 * over the normal printer driver
 * or over it's pdf exporter
 */
public class OfficePrint {


    static void waitInSeconds(int _nSeconds)
        {
            try {
                java.lang.Thread.sleep(_nSeconds * 1000);
            } catch (java.lang.InterruptedException e2) {}
        }

    /**
     * load a OpenOffice.org document from a given URL (_sInputURL)
     * the GraphicalTestArguments must contain a living MultiServiceFactory object
     * or we crash here.
     * Be aware, the ownership of the document gets to you, you have to close it.
     */
    public static XComponent loadFromURL(GraphicalTestArguments _aGTA,
                                         String _sInputURL)
        {
            XComponent aDoc = null;
            try {
                if (_aGTA.getMultiServiceFactory() == null)
                {
                    System.out.println("MultiServiceFactory in GraphicalTestArgument not set.");
                    return null;
                }
                Object oDsk = _aGTA.getMultiServiceFactory().createInstance("com.sun.star.frame.Desktop");
                XDesktop aDesktop = (XDesktop)UnoRuntime.queryInterface(XDesktop.class, oDsk);

                if (aDesktop != null)
                {
                    System.out.println("Desktop.");
                    // String sInputURL = aCurrentParameter.sInputURL;
                    // String sOutputURL = aCurrentParameter.sOutputURL;
                    // String sPrintFileURL = aCurrentParameter.sPrintToFileURL;
                    // System.out.println(_sInputURL);


                    // set here the loadComponentFromURL() properties
                    // at the moment only 'Hidden' is set, so no window is opened at work
                    PropertyValue [] aProps;
                    if (_aGTA.isHidden())
                    {
                        aProps = new PropertyValue[1];
                        PropertyValue Arg = new PropertyValue();
                        Arg.Name = "Hidden";
                        Arg.Value = Boolean.TRUE;
                        aProps[0] = Arg;
                    }
                    else
                    {
                        aProps = new PropertyValue [0];
                    }

                    XComponentLoader aCompLoader = (XComponentLoader) UnoRuntime.queryInterface( XComponentLoader.class, aDesktop);

                    // XComponent aDoc = null;

                    aDoc = aCompLoader.loadComponentFromURL(_sInputURL, "_blank", 0, aProps );
                    if (aDoc != null)
                    {
                        System.out.println("Document loaded.");
                    }
                }
                else
                {
                    System.out.println("Can't open Desktop.");
                }
            }
            catch ( com.sun.star.uno.Exception e )
            {
                // Some exception occures.FAILED
                e.printStackTrace();
                aDoc = null;
            }
            return aDoc;
        }

    static boolean exportToPDF(XComponent _xComponent, String _sDestinationName)
        {
            XServiceInfo xServiceInfo =
                (XServiceInfo) UnoRuntime.queryInterface(
                    XServiceInfo.class, _xComponent
                    );

            PropertyValue[] PDFArgs = new com.sun.star.beans.PropertyValue[1];
            PDFArgs[0] = new com.sun.star.beans.PropertyValue();
            PDFArgs[0].Name = "FilterName";
            PDFArgs[0].Value = getFilterName(xServiceInfo);

            boolean bWorked = true;

            try
            {
                XStorable store =
                    (XStorable) UnoRuntime.queryInterface(
                        XStorable.class, _xComponent
                        );
                store.storeToURL(_sDestinationName, PDFArgs);
            }
            catch (com.sun.star.io.IOException e)
            {
                bWorked = false;
            }

            return bWorked;
        }

    static String getFilterName(XServiceInfo xServiceInfo)
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

    public static boolean storeAsPDF(GraphicalTestArguments _aGTA,
                                     String _sInputURL,
                                     String _sOutputURL)
        {
            boolean bBack = false;
            XComponent aDoc = loadFromURL(_aGTA, _sInputURL);

            if (aDoc == null)
            {
                System.out.println("Can't load document.");
                return bBack;
            }
            bBack = storeAsPDF(_aGTA, aDoc, _sOutputURL);

            System.out.println("close document.");
            aDoc.dispose();
            return bBack;
        }

    public static boolean storeAsPDF(GraphicalTestArguments _aGTA,
                                     XComponent _aDoc,
                                     String _sOutputURL)
        {
            // try {
            boolean bBack = true;
            if (!exportToPDF(_aDoc, _sOutputURL))
            {
                System.out.println("Can't store document as PDF.");
                bBack = false;
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
                    System.out.println("Inputpath and Outputpath are equal.");
                }
                else
                {
                    bBack = printToFileWithOOo(_aGTA, aDoc, _sOutputURL, _sPrintFileURL);
                }

                System.out.println("close document.");
                aDoc.dispose();
            }

            return bBack;
        }

    // -----------------------------------------------------------------------------
    public static void createInfoFile(String _sFile, GraphicalTestArguments _aGTA)
        {
            String sFilename = _sFile;
            if (_sFile.startsWith("file:///"))
            {
                sFilename = FileHelper.getSystemPathFromFileURL(_sFile);
                System.out.println("CreateInfoFile: '" + sFilename + "'" );
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
            try
            {
                FileWriter out = new FileWriter(aInfoFile.toString());
                out.write("# automatically created file by graphical compare" + ls);
                out.write("buildid=" + _aGTA.getBuildID() + ls);
                out.flush();
                out.close();
            }
            catch (java.io.IOException e)
            {
                System.out.println("can't create Info file.");
                e.printStackTrace();
            }
        }
    // -----------------------------------------------------------------------------
    public static boolean printToFileWithOOo(GraphicalTestArguments _aGTA,
                                             XComponent _aDoc,
                                             String _sOutputURL,
                                             String _sPrintFileURL)
        {
            boolean bBack = false;
            boolean bFailed = true;              // always be a pessimist,
            if (_aDoc == null)
            {
                System.out.println("No document is given.");
                return bBack;
            }

            try {

                // System.out.println("Document loaded.");
                // Change Pagesettings to DIN A4

                System.out.println("Document print.");
                XPrintable aPrintable = (XPrintable) UnoRuntime.queryInterface( XPrintable.class, _aDoc);
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
                            PropertyValue [] aPrintProps = new PropertyValue[1];
                            PropertyValue Arg = new PropertyValue();
                            Arg.Name = "Name";
                            Arg.Value = _aGTA.getPrinterName();
                            aPrintProps[0] = Arg;

                            System.out.println("Printername is not null, so set to " + _aGTA.getPrinterName());
                            aPrintable.setPrinter(aPrintProps);
                        }
                    }

                    // set property values for XPrintable.print()
                    // more can be found at "http://api.openoffice.org/docs/common/ref/com/sun/star/view/PrintOptions.html"

                    int nProperties = 1;                    // default for 'FileName' property
                    if (_aGTA.printAllPages() == false)
                    {
                        // we don't want to print all pages, build Pages string by ourself
                        nProperties ++;
                    }
                    int nPropsCount = 0;

                    PropertyValue [] aPrintProps = new PropertyValue[nProperties];
                    PropertyValue Arg = new PropertyValue();
                    Arg.Name = "FileName";
                    Arg.Value = _sPrintFileURL;
                    aPrintProps[nPropsCount ++] = Arg;

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
                        aPrintProps[nPropsCount ++] = Arg;
                    }

                    System.out.println("start printing.");
                    aPrintable.print(aPrintProps);
                    waitInSeconds(1);

                    if (_sOutputURL != null)
                    {
                        // store the document in an other directory
                        XStorable aStorable = (XStorable) UnoRuntime.queryInterface( XStorable.class, _aDoc);
                        if (aStorable != null)
                        {
                            PropertyValue [] szEmptyArgs = new PropertyValue [0];

                            System.out.println("Document stored.");
                            aStorable.storeAsURL(_sOutputURL, szEmptyArgs);
                        }
                    }

                    System.out.println("Wait until document is printed.");
                    boolean isBusy = true;
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
                        waitInSeconds(1);
                    }

                    // Create a .info file near the printed '.ps' or '.prn' file.
                    createInfoFile(_sPrintFileURL, _aGTA);
                }
                else
                {
                    System.out.println("Can't get XPrintable.");
                }
                bFailed = false;
                bBack = true;
            }
            catch ( com.sun.star.uno.Exception e )
            {
                // Some exception occures.FAILED
                e.printStackTrace();
                bBack = false;
            }

            if (bFailed == true)
            {
                System.out.println("convwatch.OfficePrint: FAILED");
            }
            else
            {
                System.out.println("convwatch.OfficePrint: OK");
            }
            return bBack;
        }


    // -----------------------------------------------------------------------------
    /**
     * create a reference file
     * _sAbsoluteInputPath  contains the source file, if not exists, return with failure.
     * _sAbsoluteOutputPath contains the destination, where the file will store after load with StarOffice/OpenOffice.org
     *                      if is null, print only near the Input file path
     * _sPrintType ".prn" Print input file with StarOffice/OpenOffice.org and the default printer as PostScript
     *
     */
    public static boolean buildReference(GraphicalTestArguments _aGTA,
                                         String _sAbsoluteOutputPath,
                                         String _sAbsoluteInputFile)
        throws ConvWatchCancelException
        {
            if (! FileHelper.exists(_sAbsoluteInputFile))
            {
                throw new ConvWatchCancelException("Input file: " + _sAbsoluteInputFile + " does not exist.");
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
                System.out.println("Reference already exist, don't overwrite. Set " + PropertyName.OVERWRITE_REFERENCE + "=true to force overwrite.");
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
                System.out.println("OfficePrint.buildreference(): Unknown print type.");
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
            String sPrintFileURL = null;
            if (_aGTA.getReferenceType().toLowerCase().equals("ooo"))
            {
                bBack = printToFileWithOOo(_aGTA, _sInputFileURL, _sOutputFileURL, _sPrintFileURL);
            }
            else if (_aGTA.getReferenceType().toLowerCase().equals("pdf"))
            {
                System.out.println("USE PDF AS EXPORT FORMAT.");
                bBack = storeAsPDF(_aGTA, _sInputFileURL, _sPrintFileURL);
            }
            else if (_aGTA.getReferenceType().toLowerCase().equals("msoffice"))
            {
                if (MSOfficePrint.isMSOfficeDocumentFormat(_sInputFileURL))
                {
                    System.out.println("USE MSOFFICE AS EXPORT FORMAT.");
                    MSOfficePrint a = new MSOfficePrint();
                    try
                    {
                        a.printToFileWithMSOffice(_aGTA, FileHelper.getSystemPathFromFileURL(_sInputFileURL),
                                                  FileHelper.getSystemPathFromFileURL(_sPrintFileURL));
                    }
                    catch(ConvWatchCancelException e)
                    {
                        e.printStackTrace();
                        System.out.println(e.getMessage());
                        throw new ConvWatchCancelException("Exception caught. Problem with MSOffice printer methods.");
                    }
                    catch(java.io.IOException e)
                    {
                        System.out.println(e.getMessage());
                        throw new ConvWatchCancelException("IOException caught. Problem with MSOffice printer methods.");
                    }
                    bBack = true;
                }
                else
                {
                    System.out.println("This document type is not recognized as MSOffice format, as default fallback StarOffice/OpenOffice.org instead is used.");
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
}
