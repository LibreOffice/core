/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MSOfficePrint.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 17:41:58 $
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

import convwatch.FileHelper;
import java.io.File;
import java.io.FileWriter;
import convwatch.GraphicalTestArguments;
import helper.ProcessHandler;
import convwatch.StringHelper;
import convwatch.ConvWatchException;
import java.util.ArrayList;

/**
 * This object gives all functionallity to print msoffice documents.
 * It also offers functions to check what type of document it is.
 * It handles *.doc as word documents and use word to print
 * *.xls as excel
 * *.ppt as powerpoint
 */

class ProcessHelper
{
    ArrayList m_aArray;
}

public class MSOfficePrint
{
    private String m_sPrinterName;               // within Windows the tools need a printer name;

    public void setPrinterName(String _s) {m_sPrinterName = _s;}

        // -----------------------------------------------------------------------------
    static boolean isWordDocument(String _sSuffix)
        {
            if (_sSuffix.toLowerCase().endsWith(".doc") ||
                _sSuffix.toLowerCase().endsWith(".rtf") ||
                _sSuffix.toLowerCase().endsWith(".dot"))
            {
                return true;
            }
            return false;
        }

    static boolean isExcelDocument(String _sSuffix)
        {
            // xlt templates
            // xlw
            // xla addin
            if (_sSuffix.toLowerCase().endsWith(".xls"))
            {
                return true;
            }
            /* temporal insertion by SUS
            if (_sSuffix.endsWith(".xml"))
            {
                return true;
            }*/
            return false;
        }

    static boolean isPowerPointDocument(String _sSuffix)
        {
            if (_sSuffix.toLowerCase().endsWith(".pps") ||
                _sSuffix.toLowerCase().endsWith(".ppt"))
            {
                return true;
            }
            return false;
        }

    /**
     * returns true, if the given filename has a MS Office suffix.
     */
    public static boolean isMSOfficeDocumentFormat(String _sFile)
    {
        String sDocumentSuffix = FileHelper.getSuffix(_sFile);
        if (isWordDocument(sDocumentSuffix)) return true;
        if (isExcelDocument(sDocumentSuffix)) return true;
        if (isPowerPointDocument(sDocumentSuffix)) return true;
        // if suffix is xml, return also true, but we can't decide if word or excel
        if (sDocumentSuffix.toLowerCase().endsWith(".xml")) return true;
        return false;
    }

    public void storeToFileWithMSOffice( GraphicalTestArguments _aGTA,
                                         String _sInputFile,
                                         String _sOutputFile) throws ConvWatchCancelException, java.io.IOException
        {
            String sDocumentSuffix = FileHelper.getSuffix(_sInputFile);
            String sFilterName = _aGTA.getExportFilterName();
            ArrayList aStartCommand = new ArrayList();
            if (isWordDocument(sDocumentSuffix))
            {
                aStartCommand = createWordStoreHelper();
            }
            else if (isExcelDocument(sDocumentSuffix))
            {
                aStartCommand = createExcelStoreHelper();
            }
            else if (isPowerPointDocument(sDocumentSuffix))
            {
            }
            else if (sDocumentSuffix.toLowerCase().equals(".xml"))
            {
                // special case, if xml we prefer word, but with DEFAULT_XML_FORMAT_APP=excel it's changeable.
                if (_aGTA.getDefaultXMLFormatApp().toLowerCase().equals("excel"))
                {
                    aStartCommand = createExcelStoreHelper();
                }
                // else
                // {
                // }
            }
            else
            {
                System.out.println("No MSOfficeDocument format found.");
// TODO: use a better Exception!!!
                throw new ConvWatchCancelException/*WrongSuffixException*/("No MS office document format found.");
            }
            if (aStartCommand != null)
            {
                if (sFilterName == null)
                {
// TODO: hardcoded FilterName in perl script
                    sFilterName = ""; // xlXMLSpreadsheet";
                }

                // String sCommand = sStartCommand + " " +
                //     _sInputFile + " " +
                //     StringHelper.doubleQuote(sFilterName) + " " +
                //     _sOutputFile;

                aStartCommand.add(_sInputFile);
                aStartCommand.add(sFilterName);
                aStartCommand.add(_sOutputFile);
                realStartCommand(aStartCommand);
            }
        }

    // -----------------------------------------------------------------------------
    /**
     * print the given file (_sInputFile) to the file name (_sPrintFile)
     */
    public void printToFileWithMSOffice( GraphicalTestArguments _aGTA,
                                         String _sInputFile,
                                         String _sPrintFilename) throws ConvWatchCancelException, java.io.IOException
        {
            String sDocumentSuffix = FileHelper.getSuffix(_sInputFile);

            setPrinterName(_aGTA.getPrinterName());

            ArrayList aStartCommand = new ArrayList();
            if (isWordDocument(sDocumentSuffix))
            {
                aStartCommand = createWordPrintHelper();
            }
            else if (isExcelDocument(sDocumentSuffix))
            {
                aStartCommand = createExcelPrintHelper();
            }
            else if (isPowerPointDocument(sDocumentSuffix))
            {
                aStartCommand = createPowerPointPrintHelper();
            }
            else if (sDocumentSuffix.toLowerCase().equals(".xml"))
            {
                // special case, if xml we prefer word, but with DEFAULT_XML_FORMAT_APP=excel it's changeable.
                if (_aGTA.getDefaultXMLFormatApp().toLowerCase().equals("excel"))
                {
                    aStartCommand = createExcelPrintHelper();
                }
                else
                {
                    aStartCommand = createWordPrintHelper();
                }
            }
            else
            {
                System.out.println("No MSOfficeDocument format found.");
// TODO: use a better Exception!!!
                throw new ConvWatchCancelException/*WrongSuffixException*/("No MS office document format found.");
            }

            if (aStartCommand.isEmpty() == false)
            {
                String sPrinterName = m_sPrinterName;
                if (sPrinterName == null)
                {
                    sPrinterName = "";
                }

                // String sCommand = sStartCommand + " " +
                //     _sInputFile + " " +
                //     StringHelper.doubleQuote(m_sPrinterName) + " " +
                //     _sPrintFilename;
                aStartCommand.add(_sInputFile);
                aStartCommand.add(m_sPrinterName);
                aStartCommand.add(_sPrintFilename);

                realStartCommand(aStartCommand);
            }
            _aGTA.getPerformance().readWordValuesFromFile("c:\\temp\\wordloadtimes.txt");
            OfficePrint.createInfoFile(_sPrintFilename, _aGTA, "msoffice");
            OfficePrint.waitInSeconds(2, "Give word/excel some time to print.");
        }

    public void realStartCommand(ArrayList _aStartCommand) throws ConvWatchCancelException
        {
            if (_aStartCommand.isEmpty())
            {
                throw new ConvWatchCancelException/*WrongEnvironmentException*/("Given list is empty.");
            }

            try
            {
                // Convert the StartCommand ArrayList to a String List
                int nValues = _aStartCommand.size();
                String[] aList = new String[nValues];
                for (int i=0;i<nValues;i++)
                {
                    String aStr = (String) _aStartCommand.get(i);
                    if (aStr == null)
                    {
                        aStr = "";
                    }
                    if (aStr.length() == 0)
                    {
                        aStr = "\"\"";
                    }
                    aList[i] = new String(aStr);
                }

                // This is really the latest point where we can check if we are running within windows environment
                if (! OSHelper.isWindows())
                {
                    // TODO: use a better Exception!!!
                    throw new ConvWatchCancelException/*WrongEnvironmentException*/("We doesn't work within windows environment.");
                }


                ProcessHandler aHandler = new ProcessHandler(aList);
                boolean bBackValue = aHandler.executeSynchronously();
            }
            catch (IndexOutOfBoundsException e)
            {
                throw new ConvWatchCancelException/*WrongEnvironmentException*/("Given list is too short.");
            }

            // return aHandler.getExitCode();
        }


    ArrayList createWordPrintHelper() throws java.io.IOException
        {
            // create a program in tmp file
            String sTmpPath = util.utils.getUsersTempDir();
            String ls = System.getProperty("line.separator");
            String fs = System.getProperty("file.separator");

            String sPrintViaWord = "printViaWord.pl";

            ArrayList aList = searchLocalFile(sPrintViaWord);
            if (aList.isEmpty() == false)
            {
                return aList;
            }

            String sName = sTmpPath + fs + sPrintViaWord;
            File aFile = new File(sName);
            FileWriter out = new FileWriter(aFile.toString());


            out.write( "eval 'exec perl -wS $0 ${1+\"$@\"}'                                                          " + ls );
            out.write( "   if 0;                                                                                     " + ls );
            out.write( "use strict;                                                                                  " + ls );
            out.write( "use Time::HiRes;                                                                             " + ls );
            out.write( "if ( $^O ne \"MSWin32\")                                                                     " + ls );
            out.write( "{                                                                                            " + ls );
            out.write( "   print 'Windows only.\\n';                                                                  " + ls );
            out.write( "   print_usage();                                                                            " + ls );
            out.write( "   exit(1);                                                                                  " + ls );
            out.write( "}                                                                                            " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "use Win32::OLE;                                                                              " + ls );
            out.write( "use Win32::OLE::Const 'Microsoft Word';                                                      " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "# ------ usage ------                                                                        " + ls );
            out.write( "sub print_usage()                                                                            " + ls );
            out.write( "{                                                                                            " + ls );
            out.write( "    print STDERR \"Usage: word_print.pl  <Word file> <name of printer> <output file> .\\n     " + ls );
            out.write( "                  Please use the same string for the name of the printer as you can find \\n  " + ls );
            out.write( "                  under Start-Control Panel-Printer and Faxes  \\n                        " + ls );
            out.write( "                  The name could look like the the following line: \\n                        " + ls );
            out.write( "                  Apple LaserWriter II NT v47.0 \\n                                           " + ls );
            out.write( "                  Sample command line: \\n                                                    " + ls );
            out.write( "                  execl_print.pl  c:\\book1.doc Apple LaserWriter II NT v47.0 c:\\output\\book1.ps \\n\";  " + ls );
            out.write( "}                                                                                            " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "if ($#ARGV != 2)                                                                             " + ls );
            out.write( "{                                                                                            " + ls );
            out.write( "   print 'Too less arguments.\\n';                                                            " + ls );
            out.write( "   print_usage();                                                                            " + ls );
            out.write( "   exit(1);                                                                                  " + ls );
            out.write( "}                                                                                            " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "my $startWordTime = Time::HiRes::time(); " + ls );
            out.write( "my $Word = Win32::OLE->new('Word.Application');                                              " + ls );
            out.write( "my $stopWordTime = Time::HiRes::time() - $startWordTime; " + ls );
            out.write( "# $Word->{'Visible'} = 1;         # if you want to see what's going on                       " + ls );
            out.write( "# , ReadOnly => 1})" + ls );
            out.write(ls);
            out.write( "my $startLoadWordTime = Time::HiRes::time(); " + ls );
            out.write( "$Word->Documents->Open({Filename => $ARGV[0]})                                               " + ls );
            out.write( "    || die('Unable to open document ', Win32::OLE->LastError());                             " + ls );
            out.write( "my $stopLoadWordTime = Time::HiRes::time() - $startLoadWordTime; " + ls );
            out.write(ls);
            out.write( "my $startPrintWordTime = Time::HiRes::time(); " + ls);
            out.write( "my $oldActivePrinte = $Word->{ActivePrinter} ;                                               " + ls );
            out.write( "$Word->{ActivePrinter} = $ARGV[1];                                                           " + ls );
            out.write( "$Word->ActiveDocument->PrintOut({                                                            " + ls );
            out.write( "                                 Background => 0,                                            " + ls );
            out.write( "                                 Append     => 0,                                            " + ls );
            out.write( "                                 Range      => wdPrintAllDocument,                           " + ls );
            out.write( "                                 Item       => wdPrintDocumentContent,                       " + ls );
            out.write( "                                 Copies     => 1,                                            " + ls );
            out.write( "                                 PageType   => wdPrintAllPages,                              " + ls );
            out.write( "                                 PrintToFile => 1,                                           " + ls );
            out.write( "                                 OutputFileName => $ARGV[2]                                  " + ls );
            out.write( "  });                                                                                        " + ls );
            out.write( "$Word->{ActivePrinter} = $oldActivePrinte;                                                   " + ls );
            out.write( "my $stopPrintWordTime = Time::HiRes::time() - $startPrintWordTime;" + ls);

            out.write( "# ActiveDocument.Close(SaveChanges:=WdSaveOptions.wdDoNotSaveChanges)" + ls );
            out.write( "my $sVersion = $Word->Application->Version();"+ls);
            out.write( "$Word->ActiveDocument->Close({SaveChanges => 0});                                                           " + ls );
            out.write( "$Word->Quit();                                                                               " + ls );

            out.write( "local *FILE;" + ls);
            out.write( "if (open(FILE, \">c:/temp/wordloadtimes.txt\"))" + ls);
            out.write( "{" + ls);
            out.write( "   print FILE \"name=$ARGV[0]\\n\";" + ls);
            out.write( "   print FILE \"WordVersion=$sVersion\\n\";" + ls);
            out.write( "   print FILE \"WordStartTime=$stopWordTime\\n\";" + ls);
            out.write( "   print FILE \"WordLoadTime=$stopLoadWordTime\\n\";" + ls);
            out.write( "   print FILE \"WordPrintTime=$stopPrintWordTime\\n\";" + ls);
            out.write( "   close(FILE);" + ls);
            out.write( "}" + ls);
            out.close();

            aList.add("perl");
            aList.add(sName);
            return aList;
        }

    // TODO: Maybe give a possibility to say where search the script from outside

    ArrayList searchLocalFile(String _sScriptName)
        {
            String userdir = System.getProperty("user.dir");
            String fs = System.getProperty("file.separator");

            ArrayList aList = new ArrayList();
            File aPerlScript = new File(userdir + fs + _sScriptName);
            if (FileHelper.isDebugEnabled())
            {
                System.out.println("Search for local existance of " + aPerlScript.getAbsolutePath());
            }

            if (aPerlScript.exists())
            {
                if (FileHelper.isDebugEnabled())
                {
                    System.out.println("OK, found it, use this instead the internal one.");
                }

                String sName = aPerlScript.getAbsolutePath();
                // String sCommand = "perl " + sName;
                // System.out.println(sCommand);
                aList.add("perl");
                aList.add(sName);
                return aList;
            }
            return aList;
        }

    ArrayList createWordStoreHelper() throws java.io.IOException
        {
            // create a program in tmp file
            String sTmpPath = util.utils.getUsersTempDir();
            String ls = System.getProperty("line.separator");
            String fs = System.getProperty("file.separator");

            // ArrayList aList = new ArrayList();
            String sSaveViaWord = "saveViaWord.pl";

            ArrayList aList = searchLocalFile(sSaveViaWord);
            if (aList.isEmpty() == false)
            {
                return aList;
            }

            String sName = sTmpPath + fs + sSaveViaWord;
            if (FileHelper.isDebugEnabled())
            {
                System.out.println("No local found, create a perl script: " + sName);
            }

            File aFile = new File(sName);
            FileWriter out = new FileWriter(aFile.toString());

            out.write( "eval 'exec perl -wS $0 ${1+\"$@\"}'                                                          " + ls );
            out.write( "   if 0;                                                                                     " + ls );
            out.write( "use strict;                                                                                  " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "if ( $^O ne \"MSWin32\")                                                                     " + ls );
            out.write( "{                                                                                            " + ls );
            out.write( "   print 'Windows only.\\n';                                                                  " + ls );
            out.write( "   print_usage();                                                                            " + ls );
            out.write( "   exit(1);                                                                                  " + ls );
            out.write( "}                                                                                            " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "use Win32::OLE;                                                                              " + ls );
            out.write( "use Win32::OLE::Const 'Microsoft Word';                                                      " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "# ------ usage ------                                                                        " + ls );
            out.write( "sub print_usage()                                                                            " + ls );
            out.write( "{                                                                                            " + ls );
            out.write( "    print STDERR \"Usage: storeViaWord.pl  <Word file> <output filer> <output file> \\n\"     " + ls );
            out.write( "}                                                                                            " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "if ($#ARGV != 2)                                                                             " + ls );
            out.write( "{                                                                                            " + ls );
            out.write( "   print 'Too less arguments.\\n';                                                            " + ls );
            out.write( "   print_usage();                                                                            " + ls );
            out.write( "   exit(1);                                                                                  " + ls );
            out.write( "}                                                                                            " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "                                                                                             " + ls );
            out.write( "my $Word = Win32::OLE->new('Word.Application');                                              " + ls );
            out.write( "# $Word->{'Visible'} = 1;         # if you want to see what's going on                       " + ls );
            out.write( "my $Book = $Word->Documents->Open($ARGV[0])                                                             " + ls );
            out.write( "    || die('Unable to open document ', Win32::OLE->LastError());                             " + ls );
            out.write( "# my $oldActivePrinte = $Word->{ActivePrinter} ;                                               " + ls );
            out.write( "# $Word->{ActivePrinter} = $ARGV[1];                                                           " + ls );
            out.write( "# $Word->ActiveDocument->PrintOut({                                                            " + ls );
            out.write( "#                                  Background => 0,                                            " + ls );
            out.write( "#                                  Append     => 0,                                            " + ls );
            out.write( "#                                  Range      => wdPrintAllDocument,                           " + ls );
            out.write( "#                                  Item       => wdPrintDocumentContent,                       " + ls );
            out.write( "#                                  Copies     => 1,                                            " + ls );
            out.write( "#                                  PageType   => wdPrintAllPages,                              " + ls );
            out.write( "#                                  PrintToFile => 1,                                           " + ls );
            out.write( "#                                  OutputFileName => $ARGV[2]                                  " + ls );
            out.write( "#   });                                                                                        " + ls );
            out.write( "# $Word->{ActivePrinter} = $oldActivePrinte;                                                   " + ls );
            out.write( "$Book->savaAs($ARGV[2], $ARGV[1]);                                                             " + ls );
            out.write( "# ActiveDocument.Close(SaveChanges:=WdSaveOptions.wdDoNotSaveChanges)" + ls );
            out.write( "$Book->Close({SaveChanges => 0});                                                           " + ls );
            out.write( "$Word->Quit();                                                                               " + ls );
            out.close();

            aList.add("perl");
            aList.add(sName);
            return aList;
        }


    ArrayList createExcelPrintHelper() throws java.io.IOException
        {
            // create a program in tmp file
            String sTmpPath = util.utils.getUsersTempDir();
            String ls = System.getProperty("line.separator");
            String fs = System.getProperty("file.separator");

            String sPrintViaExcel = "printViaExcel.pl";

            ArrayList aList = searchLocalFile(sPrintViaExcel);
            if (aList.isEmpty() == false)
            {
                return aList;
            }
            String sName = sTmpPath + fs + sPrintViaExcel;
            if (FileHelper.isDebugEnabled())
            {
                System.out.println("No local found, create a perl script: " + sName);
            }

            File aFile = new File(sName);
            FileWriter out = new FileWriter(aFile.toString());

            out.write( "eval 'exec perl -wS $0 ${1+\"$@\"}'                                                                                " + ls );
            out.write( "   if 0;                                                                                                         " + ls );
            out.write( "use strict;                                                                                                      " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "if ( $^O ne \"MSWin32\")                                                                                         " + ls );
            out.write( "{                                                                                                                " + ls );
            out.write( "   print \"Windows only.\\n\";                                                                                    " + ls );
            out.write( "   print_usage();                                                                                                " + ls );
            out.write( "   exit(1);                                                                                                      " + ls );
            out.write( "}                                                                                                                " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "use Win32::OLE qw(in with);                                                                                      " + ls );
            out.write( "use Win32::OLE::Const 'Microsoft Excel';                                                                         " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "# ------ usage ------                                                                                            " + ls );
            out.write( "sub print_usage()                                                                                                " + ls );
            out.write( "{                                                                                                                " + ls );
            out.write( "    print STDERR \"Usage: printViaExcel.pl  <Excel file> <name of printer> <output file> .\\n                       " + ls );
            out.write( "                  Please use the same string for the name of the printer as you can find \\n                      " + ls );
            out.write( "                  under Start-Control Panel-Printer and Faxes  \\n                                            " + ls );
            out.write( "                  The name could look like the the following line: \\n                                            " + ls );
            out.write( "                  Apple LaserWriter II NT v47.0 \\n                                                               " + ls );
            out.write( "                  Sample command line: \\n                                                                        " + ls );
            out.write( "                  execl_print.pl  c:\\book1.xls Apple LaserWriter II NT v47.0 c:\\output\\book1.ps \\n\";     " + ls );
            out.write( "}                                                                                                                " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "$Win32::OLE::Warn = 3;                                # die on errors...                                         " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "if ($#ARGV != 2)                                                                                                 " + ls );
            out.write( "{                                                                                                                " + ls );
            out.write( "   print STDERR \"Too less arguments.\\n\";                                                                      " + ls );
            out.write( "   print STDERR \"ARGV[0] $ARGV[0]\\n\";                                                                         " + ls );
            out.write( "   print STDERR \"ARGV[1] $ARGV[1]\\n\";                                                                         " + ls );
            out.write( "   print STDERR \"ARGV[2] $ARGV[2]\\n\";                                                                         " + ls );
            out.write( "   print_usage();                                                                                                " + ls );
            out.write( "   exit(1);                                                                                                      " + ls );
            out.write( "}                                                                                                                " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "my $Excel = Win32::OLE->GetActiveObject('Excel.Application')                                                     " + ls );
            out.write( "    || Win32::OLE->new('Excel.Application', 'Quit');  # get already active Excel                                 " + ls );
            out.write( "                                                      # application or open new                                  " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "my $Book = $Excel->Workbooks->Open( $ARGV[0] );                                                                  " + ls );
            out.write( "   $Book->PrintOut({Copies => 1,                                                                                 " + ls );
            out.write( "                    ActivePrinter => $ARGV[1],                                                                   " + ls );
            out.write( "                    PrToFileName => $ARGV[2],                                                                    " + ls );
            out.write( "                    Collate => 1                                                                                 " + ls );
            out.write( "                    });                                                                                          " + ls );
            out.write( "# Close worksheets without store changes" + ls );
            out.write( "# $Book->Close({SaveChanges => 0});                                                           " + ls );
            out.write( "$Excel->Quit                                                                                                     " + ls );
            out.close();

            aList.add("perl");
            aList.add(sName);
            return aList;
        }

    ArrayList createExcelStoreHelper() throws java.io.IOException
        {
            // create a program in tmp file
            String sTmpPath = util.utils.getUsersTempDir();
            String ls = System.getProperty("line.separator");
            String fs = System.getProperty("file.separator");

            String sSaveViaExcel = "saveViaExcel.pl";

            ArrayList aList = searchLocalFile(sSaveViaExcel);
            if (aList.isEmpty() == false)
            {
                return aList;
            }
            String sName = sTmpPath + fs + sSaveViaExcel;
            if (FileHelper.isDebugEnabled())
            {
                System.out.println("No local found, create a script: " + sName);
            }

            File aFile = new File(sName);
            FileWriter out = new FileWriter(aFile.toString());

            out.write( "eval 'exec perl -wS $0 ${1+\"$@\"}'                                                                                " + ls );
            out.write( "   if 0;                                                                                                         " + ls );
            out.write( "use strict;                                                                                                      " + ls );
            out.write( "# This script is automatically created.                                                                          " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "use Win32::OLE qw(in with);                                                                                      " + ls );
            out.write( "use Win32::OLE::Const 'Microsoft Excel';                                                                         " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "# ------ usage ------                                                                                            " + ls );
            out.write( "sub print_usage()                                                                                                " + ls );
            out.write( "{                                                                                                                " + ls );
            out.write( "    print STDERR \"Usage: savaViaExcel.pl  <Excel file> <filefilter> <output file> .\\n                       " + ls );
            out.write( "                  execl_print.pl  c:\\book1.xls Apple LaserWriter II NT v47.0 c:\\output\\book1.ps \\n\";     " + ls );
            out.write( "}                                                                                                                " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "$Win32::OLE::Warn = 3;                                # die on errors...                                         " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "if ($#ARGV != 2)                                                                                                 " + ls );
            out.write( "{                                                                                                                " + ls );
            out.write( "   print \"Too less arguments.\\n\";                                                                              " + ls );
            out.write( "   print_usage();                                                                                                " + ls );
            out.write( "   exit(1);                                                                                                      " + ls );
            out.write( "}                                                                                                                " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "my $Excel = Win32::OLE->GetActiveObject('Excel.Application')                                                     " + ls );
            out.write( "    || Win32::OLE->new('Excel.Application', 'Quit');  # get already active Excel                                 " + ls );
            out.write( "                                                      # application or open new                                  " + ls );
            out.write( "my $sFilterParameter = $ARGV[1];                                                                                                                 " + ls );
            out.write( "my $sFilterName = xlHTML;                                                                                                                 " + ls );
            out.write( "if ($sFilterParameter eq 'xlXMLSpreadsheet')                                                                                                                 " + ls );
            out.write( "{                                                                                                                 " + ls );
            out.write( "    $sFilterName = xlXMLSpreadsheet;                                                                                                                " + ls );
            out.write( "}                                                                                                                 " + ls );
            out.write( "elsif ($sFilterParameter eq 'xlHTML')                                                                                                                 " + ls );
            out.write( "{                                                                                                                 " + ls );
            out.write( "    $sFilterName = xlHTML;                                                                                                                 " + ls );
            out.write( "}                                                                                                                 " + ls );
            out.write( "else                                                                                                                 " + ls );
            out.write( "{                                                                                                                 " + ls );
            out.write( "    my $undefined;                                                                                                " + ls);
            out.write( "    $sFilterName = $undefined;                                                                                                              " + ls );
            out.write( "}                                                                                                                 " + ls );
            out.write( "                                                                                                                 " + ls );
            out.write( "my $Book = $Excel->Workbooks->Open( $ARGV[0] );                                                                  " + ls );
            out.write( "$Excel->{DisplayAlerts} = 0;                                                                                     " + ls );
            out.write( "$Book->saveAs($ARGV[2],                                                                                          " + ls );
            out.write( "              $sFilterName,                                                                                   " + ls );
            out.write( "              '',                                                                                                " + ls );
            out.write( "              '',                                                                                                " + ls );
            out.write( "              0,                                                                                                 " + ls );
            out.write( "              0,                                                                                                 " + ls );
            out.write( "              xlNoChange,                                                                                        " + ls );
            out.write( "              xlLocalSessionChanges,                                                                             " + ls );
            out.write( "              1);                                                                                                " + ls );
            out.write( "# Close worksheets without store changes" + ls );
            out.write( "# $Book->Close({SaveChanges => 0}); " + ls );
            out.write( "$Excel->Quit                                                                                                     " + ls );
            out.close();

            aList.add("perl");
            aList.add(sName);
            return aList;
        }

    ArrayList createPowerPointPrintHelper() throws java.io.IOException
        {
            // create a program in tmp file
            String sTmpPath = util.utils.getUsersTempDir();
            String ls = System.getProperty("line.separator");
            String fs = System.getProperty("file.separator");

            String sPrintViaPowerPoint = "printViaPowerPoint.pl";

            ArrayList aList = searchLocalFile(sPrintViaPowerPoint);
            if (aList.isEmpty() == false)
            {
                return aList;
            }
            String sName = sTmpPath + fs + sPrintViaPowerPoint;
            if (FileHelper.isDebugEnabled())
            {
                System.out.println("No local found, create a script: " + sName);
            }

            File aFile = new File(sName);
            FileWriter out = new FileWriter(aFile.toString());


            out.write( "eval 'exec perl -wS $0 $1 $2 '                                                                                         " + ls );
            out.write( "   if 0;                                                                                                               " + ls );
            out.write( "use strict;                                                                                                            " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "if ( $^O ne \"MSWin32\")                                                                                                 " + ls );
            out.write( "{                                                                                                                      " + ls );
            out.write( "   print \"Windows only.\\n\";                                                                                            " + ls );
            out.write( "   print_usage();                                                                                                      " + ls );
            out.write( "   exit(1);                                                                                                            " + ls );
            out.write( "}                                                                                                                      " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "use Win32::OLE qw(in with);                                                                                            " + ls );
            out.write( "use Win32::OLE::Const 'Microsoft PowerPoint';                                                                          " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "# ------ usage ------                                                                                                  " + ls );
            out.write( "sub print_usage()                                                                                                      " + ls );
            out.write( "{                                                                                                                      " + ls );
            out.write( "    print STDERR \"Usage: powerpoint_print.pl  <PowerPoint file> <name of printer> <output file> .\\n                    " + ls );
            out.write( "                  Please use the same string for the name of the printer as you can find \\n                            " + ls );
            out.write( "                  under Start-Control Panel-Printer and Faxes  \\n                                                  " + ls );
            out.write( "                  The name could look like the the following line: \\n                                                  " + ls );
            out.write( "                  Apple LaserWriter II NT v47.0 \\n                                                                     " + ls );
            out.write( "                  Sample command line: \\n                                                                              " + ls );
            out.write( "                  powerpoint_print.pl  c:\\book.ppt Apple LaserWriter II NT v47.0 c:\\output\\book.ps \\n\";         " + ls );
            out.write( "}                                                                                                                      " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "$Win32::OLE::Warn = 3;                                # die on errors...                                               " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "if ($#ARGV < 2)                                                                                                        " + ls );
            out.write( "{                                                                                                                      " + ls );
            out.write( "   print \"Too less arguments.\\n\";                                                                                      " + ls );
            out.write( "   print_usage();                                                                                                      " + ls );
            out.write( "   exit(1);                                                                                                            " + ls );
            out.write( "}                                                                                                                      " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "my $PowerPoint = Win32::OLE->GetActiveObject('PowerPoint.Application')                                                 " + ls );
            out.write( "    || Win32::OLE->new('PowerPoint.Application', 'Quit');  # get already active Excel                                  " + ls );
            out.write( "                                                      # application or open new                                        " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "   $PowerPoint->{'Visible'} = 1;                                                                                       " + ls );
            out.write( "   my $Presentation = $PowerPoint->Presentations->Add;                                                                 " + ls );
            out.write( "   my $Presentation = $PowerPoint->Presentations->Open( $ARGV[0] );                                                    " + ls );
            out.write( "   $Presentation->PrintOptions->{ActivePrinter} = $ARGV[1];                                                            " + ls );
            out.write( "   $Presentation->PrintOptions->{PrintInBackground} = 0;                                                               " + ls );
            out.write( "   # PrintColorType = 1 means print in color and PrintColorType = 2 means print in gray                                " + ls );
            out.write( "   $Presentation->PrintOptions->{PrintColorType} = 1;                                                                  " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "   $Presentation->PrintOut({PrintToFile => $ARGV[2]});                                                                 " + ls );
            out.write( "   sleep 5;                                                                                                            " + ls );
            out.write( "   print \"Presentation has been printed\\n\";                                                                            " + ls );
            out.write( "$PowerPoint->Quit                                                                                                      " + ls );
            out.close();

            aList.add("perl");
            aList.add(sName);
            return aList;
        }

}
