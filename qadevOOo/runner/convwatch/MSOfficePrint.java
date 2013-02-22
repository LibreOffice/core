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

import convwatch.FileHelper;
import java.io.File;
import java.io.FileWriter;
import java.io.RandomAccessFile;
import convwatch.GraphicalTestArguments;
import helper.ProcessHandler;
import java.util.ArrayList;
import helper.OSHelper;

/**
 * This object gives all functionallity to print msoffice documents.
 * It also offers functions to check what type of document it is.
 * It handles *.doc as word documents and use word to print
 * *.xls as excel
 * *.ppt as powerpoint
 */

class ProcessHelper
{
    ArrayList<String> m_aArray;
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
            ArrayList<String> aStartCommand = new ArrayList<String>();
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
                GlobalLogWriter.get().println("No Microsoft Office document format found.");
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

            ArrayList<String> aStartCommand = new ArrayList<String>();
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
// TODO: Open XML File and check if we need excel or word
                String sOfficeType = getOfficeType(_sInputFile);

                // special case, if xml we prefer word, but with DEFAULT_XML_FORMAT_APP=excel it's changeable.
                // if (_aGTA.getDefaultXMLFormatApp().toLowerCase().equals("excel"))
                if (sOfficeType.equals("excel"))
                {
                    aStartCommand = createExcelPrintHelper();
                }
                else if (sOfficeType.equals("word"))
                {
                    aStartCommand = createWordPrintHelper();
                }
                else
                {
                    return;
                }
            }
            else
            {
                GlobalLogWriter.get().println("No Microsoft Office document format found.");
// TODO: use a better Exception!!!
                throw new ConvWatchCancelException/*WrongSuffixException*/("No Mircosoft Office document format found.");
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
            String sUserDir = System.getProperty("user.home");
            String fs = System.getProperty("file.separator");
            if (! sUserDir.endsWith(fs))
            {
                sUserDir = sUserDir + fs;
            }
            _aGTA.getPerformance().readWordValuesFromFile(sUserDir + "msofficeloadtimes.txt");
            OfficePrint.createInfoFile(_sPrintFilename, _aGTA, "msoffice");
            TimeHelper.waitInSeconds(2, "Give Microsoft Office some time to print.");
        }

    public void realStartCommand(ArrayList<String> _aStartCommand) throws ConvWatchCancelException
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
                    String aStr = _aStartCommand.get(i);
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
                aHandler.executeSynchronously();
            }
            catch (IndexOutOfBoundsException e)
            {
                throw new ConvWatchCancelException/*WrongEnvironmentException*/("Given list is too short.");
            }

            // return aHandler.getExitCode();
        }


    ArrayList<String> createWordPrintHelper() throws java.io.IOException
        {
            // create a program in tmp file
            String sTmpPath = util.utils.getUsersTempDir();
            String ls = System.getProperty("line.separator");
            String fs = System.getProperty("file.separator");

            String sPrintViaWord = "printViaWord.pl";

            ArrayList<String> aList = searchLocalFile(sPrintViaWord);
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
            out.write( "                  The name could look like the following line: \\n                        " + ls );
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
            out.write( "if (open(FILE, \">$ENV{HOME}/msofficeloadtimes.txt\"))" + ls);
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

    ArrayList<String> searchLocalFile(String _sScriptName)
        {
            String userdir = System.getProperty("user.dir");
            String fs = System.getProperty("file.separator");

            ArrayList<String> aList = new ArrayList<String>();
            File aPerlScript = new File(userdir + fs + _sScriptName);
            if (FileHelper.isDebugEnabled())
            {
                GlobalLogWriter.get().println("Search for local existance of " + aPerlScript.getAbsolutePath());
            }

            if (aPerlScript.exists())
            {
                if (FileHelper.isDebugEnabled())
                {
                    GlobalLogWriter.get().println("OK, found it, use this instead the internal one.");
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

    ArrayList<String> createWordStoreHelper() throws java.io.IOException
        {
            // create a program in tmp file
            String sTmpPath = util.utils.getUsersTempDir();
            String ls = System.getProperty("line.separator");
            String fs = System.getProperty("file.separator");

            // ArrayList aList = new ArrayList();
            String sSaveViaWord = "saveViaWord.pl";

            ArrayList<String> aList = searchLocalFile(sSaveViaWord);
            if (aList.isEmpty() == false)
            {
                return aList;
            }

            String sName = sTmpPath + fs + sSaveViaWord;
            if (FileHelper.isDebugEnabled())
            {
                GlobalLogWriter.get().println("No local found, create a perl script: " + sName);
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


    ArrayList<String> createExcelPrintHelper() throws java.io.IOException
        {
            // create a program in tmp file
            String sTmpPath = util.utils.getUsersTempDir();
            String ls = System.getProperty("line.separator");
            String fs = System.getProperty("file.separator");

            String sPrintViaExcel = "printViaExcel.pl";

            ArrayList<String> aList = searchLocalFile(sPrintViaExcel);
            if (aList.isEmpty() == false)
            {
                return aList;
            }
            String sName = sTmpPath + fs + sPrintViaExcel;
            if (FileHelper.isDebugEnabled())
            {
                GlobalLogWriter.get().println("No local found, create a perl script: " + sName);
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
            out.write( "                  The name could look like the following line: \\n                                            " + ls );
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
            out.write( "my $sVersion = $Excel->Application->Version();"+ls);
            out.write( "$Excel->Quit();                                                                                                     " + ls );
            out.write( "local *FILE;" + ls);
            out.write( "if (open(FILE, \">$ENV{HOME}/msofficeloadtimes.txt\"))" + ls);
            out.write( "{" + ls);
            out.write( "   print FILE \"name=$ARGV[0]\\n\";" + ls);
            out.write( "   print FILE \"ExcelVersion=$sVersion\\n\";" + ls);
//            out.write( "   print FILE \"WordStartTime=$stopWordTime\\n\";" + ls);
//            out.write( "   print FILE \"WordLoadTime=$stopLoadWordTime\\n\";" + ls);
//            out.write( "   print FILE \"WordPrintTime=$stopPrintWordTime\\n\";" + ls);
            out.write( "   close(FILE);" + ls);
            out.write( "}" + ls);
            out.close();

            aList.add("perl");
            aList.add(sName);
            return aList;
        }

    ArrayList<String> createExcelStoreHelper() throws java.io.IOException
        {
            // create a program in tmp file
            String sTmpPath = util.utils.getUsersTempDir();
            String ls = System.getProperty("line.separator");
            String fs = System.getProperty("file.separator");

            String sSaveViaExcel = "saveViaExcel.pl";

            ArrayList<String> aList = searchLocalFile(sSaveViaExcel);
            if (aList.isEmpty() == false)
            {
                return aList;
            }
            String sName = sTmpPath + fs + sSaveViaExcel;
            if (FileHelper.isDebugEnabled())
            {
                GlobalLogWriter.get().println("No local found, create a script: " + sName);
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
            out.write( "$Excel->Quit();                                                                                                     " + ls );
            out.close();

            aList.add("perl");
            aList.add(sName);
            return aList;
        }

    ArrayList<String> createPowerPointPrintHelper() throws java.io.IOException
        {
            // create a program in tmp file
            String sTmpPath = util.utils.getUsersTempDir();
            String ls = System.getProperty("line.separator");
            String fs = System.getProperty("file.separator");

            String sPrintViaPowerPoint = "printViaPowerPoint.pl";

            ArrayList<String> aList = searchLocalFile(sPrintViaPowerPoint);
            if (aList.isEmpty() == false)
            {
                return aList;
            }
            String sName = sTmpPath + fs + sPrintViaPowerPoint;
            if (FileHelper.isDebugEnabled())
            {
                GlobalLogWriter.get().println("No local found, create a script: " + sName);
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
            out.write( "                  The name could look like the following line: \\n                                                  " + ls );
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
            out.write( "# we can't change active printer in powerpoint                                                            " + ls );
            out.write( "#   $Presentation->PrintOptions->{ActivePrinter} = $ARGV[1]; " + ls );
            out.write( "   print \"Active printer is: \" . $Presentation->PrintOptions->{ActivePrinter} . \"\\n\"; " + ls );
            out.write( "   $Presentation->PrintOptions->{PrintInBackground} = 0;                                                               " + ls );
            out.write( "   # PrintColorType = 1 means print in color and PrintColorType = 2 means print in gray                                " + ls );
            out.write( "   $Presentation->PrintOptions->{PrintColorType} = 1;                                                                  " + ls );
            out.write( "                                                                                                                       " + ls );
            out.write( "   $Presentation->PrintOut({PrintToFile => $ARGV[2]});                                                                 " + ls );
            out.write( "   sleep 5;                                                                                                            " + ls );
            out.write( "   print \"Presentation has been printed\\n\";                                                                            " + ls );
            out.write( "my $sVersion = $Presentation->Application->Version();"+ls);
            out.write( "   $PowerPoint->Quit(); " + ls );

            out.write( "local *FILE;" + ls);
            out.write( "if (open(FILE, \">$ENV{HOME}/msofficeloadtimes.txt\"))" + ls);
            out.write( "{" + ls);
            out.write( "   print FILE \"name=$ARGV[0]\\n\";" + ls);
            out.write( "   print FILE \"PowerPointVersion=$sVersion\\n\";" + ls);
//            out.write( "   print FILE \"WordStartTime=$stopWordTime\\n\";" + ls);
//            out.write( "   print FILE \"WordLoadTime=$stopLoadWordTime\\n\";" + ls);
//            out.write( "   print FILE \"WordPrintTime=$stopPrintWordTime\\n\";" + ls);
            out.write( "   close(FILE);" + ls);
            out.write( "}" + ls);
            out.close();

            aList.add("perl");
            aList.add(sName);
            return aList;
        }

    /**
       @param _sFilename a name to a ms office xml file
       @return 'word' or 'excel' or '' if type not known
    */
    public String getOfficeType(String _sFilename)
        {
            File aFile = new File(_sFilename);
            if (! aFile.exists())
            {
                GlobalLogWriter.get().println("couldn't find file " + _sFilename);
                return "";
            }
            RandomAccessFile aReader = null;
            String sOfficeType = "";
            try
            {
                aReader = new RandomAccessFile(aFile,"r");
                String aLine = "";
                while (aLine != null)
                {
                    aLine = aReader.readLine();
                    if (aLine != null)
                    {
                        aLine = aLine.trim();
                        if ( (! (aLine.length() < 2) ) &&
                             (! aLine.startsWith("#")) &&
                             (! aLine.startsWith(";")) )
                        {
                            int nIdx = aLine.indexOf("mso-application");
                            if (nIdx > 0)
                            {
                                if (aLine.indexOf("Word.Document") > 0)
                                {
                                    sOfficeType = "word";
                                }
                                else if (aLine.indexOf("Excel") > 0)
                                {
                                    sOfficeType = "excel";
                                }
                                else
                                {
                                    GlobalLogWriter.get().println("Unknown/unsupported data file: " + aLine);
                                }
                            }
                        }
                    }
                }
            }
            catch (java.io.FileNotFoundException fne)
            {
                System.out.println("couldn't open file " + _sFilename);
                System.out.println("Message: " + fne.getMessage());
            }
            catch (java.io.IOException ie)
            {
                System.out.println("Exception while reading file " + _sFilename);
                System.out.println("Message: " + ie.getMessage());
            }
            try
            {
                aReader.close();
            }
            catch (java.io.IOException ie)
            {
                System.out.println("Couldn't close file " + _sFilename);
                System.out.println("Message: " + ie.getMessage());
            }
            return sOfficeType;
        }

}
