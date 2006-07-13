/************************************************************************
 *
 *  Application.java
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
 *  Copyright: 2002-2005 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3j (2005-07-01)
 *
 */

package writer2latex;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.URL;
import java.util.Enumeration;
import javax.xml.parsers.ParserConfigurationException;
import org.xml.sax.SAXException;

import org.openoffice.xmerge.Convert;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.converter.xml.OfficeDocument;
import org.openoffice.xmerge.converter.xml.sxc.SxcDocument;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;

import writer2latex.util.Misc;
import writer2latex.util.Config;
import writer2latex.office.*;
import writer2latex.xmerge.SxiDocument;
//import writer2latex.xhtml.XhtmlDocument;
import writer2latex.latex.*;

/**
 * <p>Commandline utility to convert an OpenOffice.org Writer XML file to a LaTeX file<.</p>
 * <p>The code is based on org.openoffice.xmerge.test.Driver</p>
 */
public final class Application {
    // Version number and date of Writer2LaTeX
    private static final String VERSION = "0.4";
    private static final String DATE = "2005-07-01";

    public static String getVersion() { return VERSION; }
    public static String getDate() { return DATE; }

    /**  Based on command-line parameters. */
    private String sOutputFormat = null;
    private String toMime = null;
    private String sFileName = null;
    private String sOutPathName = null;
    private String sOutFileName = null;
    private String sConfigFileName = null;

    Config config;

    /**
     *  Main method
     *
     *  @param  args  The argument passed on the command line.
     */

    public static final void main (String[] args){
        try {
            Application app = new Application();
            app.parseCommandLine(args);
            System.out.println("Starting conversion...");
            app.doConversion();
            System.out.println("Done!");
        } catch (IllegalArgumentException ex) {
            String msg = ex.getMessage();
            showUsage(msg);
        } catch (Exception ex) {
            String msg = ex.getMessage();
            if (msg != null) System.out.println("\n" + msg);
            ex.printStackTrace();
        }

    }

    /**
     *  Converts the document
     *
     *  @throws  IllegalArgumentException  If an argument is invalid.
     */
    private void doConversion() {

        // If filename ends with .xml, we suppose that it's flat xml (and is Writer)
        boolean bFlat = sFileName.toLowerCase().endsWith(".xml");
        // Determine the type of document by the extension:
        boolean bCalc = sFileName.toLowerCase().endsWith(".sxc");
        boolean bImpress = sFileName.toLowerCase().endsWith(".sxi");

        ConvertData dataOut = null;

        // Create and use the converter
        // No longer uses the XMerge interface
        // Somewhat messy, should be changed if XMerge is discarded
        OfficeDocument oooDoc;
        if (bCalc) {
            oooDoc = new SxcDocument(sFileName);
        }
        else if (bImpress) {
            oooDoc = new SxiDocument(sFileName);
        }
        else {
            oooDoc = new SxwDocument(sFileName);
        }
        File f = new File(sFileName);
        // Make sure the input file actually exists before using it
        try {
            if (!f.exists()) {
                System.out.println("I'm sorry, I can't find "+sFileName);
                System.exit(0);
            }
            FileInputStream fis = new FileInputStream(f);
            oooDoc.read(fis,!bFlat);
        }
        catch (IOException e) {
               System.out.println("Oops, there was an error reading "+sFileName);
               e.printStackTrace();
        }

        try {
            if (toMime.equals(MIMETypes.LATEX)) {
                writer2latex.latex.ConverterPalette converter
                    = new writer2latex.latex.ConverterPalette((SxwDocument)oooDoc,sConfigFileName);
                converter.setOutFileName(sOutFileName);
                dataOut = converter.convert();
            }
            else if (toMime.equals(MIMETypes.BIBTEX)) {
                writer2latex.bibtex.DocumentSerializerImpl converter
                    = new writer2latex.bibtex.DocumentSerializerImpl((SxwDocument)oooDoc);
                converter.setOutFileName(sOutFileName);
                dataOut = converter.serialize();
            }
/* disabled XHTML filter part
            else if (toMime.equals(MIMETypes.XHTML)) {
                writer2latex.xhtml.DocumentSerializerImpl converter
                    = new writer2latex.xhtml.DocumentSerializerImpl(oooDoc,
                            XhtmlDocument.XHTML10, sConfigFileName);
                converter.setOutFileName(sOutFileName);
                dataOut = converter.serialize();
            }
            else if (toMime.equals(MIMETypes.XHTML_MATHML)) {
                writer2latex.xhtml.DocumentSerializerImpl converter
                    = new writer2latex.xhtml.DocumentSerializerImpl(oooDoc,
                            XhtmlDocument.XHTML_MATHML, sConfigFileName);
                converter.setOutFileName(sOutFileName);
                dataOut = converter.serialize();
            }
            else if (toMime.equals(MIMETypes.XHTML_MATHML_XSL)) {
                writer2latex.xhtml.DocumentSerializerImpl converter
                    = new writer2latex.xhtml.DocumentSerializerImpl(oooDoc,
                            XhtmlDocument.XHTML_MATHML_XSL, sConfigFileName);
                converter.setOutFileName(sOutFileName);
                dataOut = converter.serialize();
            }
        */
        }
        catch (Exception e) {
            System.out.println("Conversion failed");
            e.printStackTrace();
        }

        // TODO: Should do some further checking on the feasability of writing
        // the directory and the files.
        File dir = new File(sOutPathName);
        if (!dir.exists()) { dir.mkdirs(); }

        Enumeration docEnum = dataOut.getDocumentEnumeration();
        while (docEnum.hasMoreElements()) {
            Document docOut      = (Document)docEnum.nextElement();
            String fileName      = sOutPathName+docOut.getFileName();
            try {
                FileOutputStream fos = new FileOutputStream(fileName);
                docOut.write(fos);
                fos.flush();
                fos.close();
            } catch (Exception writeExcept) {
                System.out.println("\nThere was an error writing out file <" +
                    fileName + ">");
                writeExcept.printStackTrace();
            }
        }
    }

    /**
     *  Display usage.
     */
    private static void showUsage(String msg) {
        System.out.println();
        System.out.println("This is Writer2LaTeX, Version " + VERSION
                           + " (" + DATE + ")");
        System.out.println();
        if (msg != null) System.out.println(msg);
        System.out.println();
        System.out.println("Usage:");
        System.out.println("   java -jar <path>/writer2latex.jar <options> <document> [<output path/file name>]");
        System.out.println("where <options> is one of the following:");
        System.out.println("   [-latex] [-config <configuration file>]");
        System.out.println("   -bibtex");
/*      System.out.println("   -xhtml [-config <configuration file>]");
        System.out.println("   -xhtml+mathml [-config <configuration file>]");
        System.out.println("   -xhtml+mathml+xsl [-config <configuration file>]");*/
    }


    /**
     *  Parse command-line arguments.
     *
     *  @param  args  Array of command line arguments.
     *
     *  @throws  IllegalArgumentException  If an argument is invalid.
     */
    private void parseCommandLine(String args[])
        throws IllegalArgumentException {

        if (args.length == 0) {
            throw new IllegalArgumentException();
        }

        int i = 0;
        String sArg;


        sArg = getArg(i,args);
    /*if ("-xhtml".equals(sArg)) {
            sOutputFormat = "xhtml";
            toMime = MIMETypes.XHTML;
            i++;
        }
        else if ("-xhtml+mathml".equals(sArg)) {
            sOutputFormat = "xhtml";
            toMime = MIMETypes.XHTML_MATHML;
            i++;
        }
        else if ("-xhtml+mathml+xsl".equals(sArg)) {
            sOutputFormat = "xhtml";
            toMime = MIMETypes.XHTML_MATHML_XSL;
            i++;
        }else
     */ if ("-bibtex".equals(sArg)) {
            sOutputFormat = "BibTeX";
            toMime = MIMETypes.BIBTEX;
            i++;
        }
        else if ("-latex".equals(sArg)) {
            sOutputFormat = "LaTeX";
            toMime = MIMETypes.LATEX;
            i++;
        }
        else { // assume LaTeX as default
            sOutputFormat = "LaTeX";
            toMime = MIMETypes.LATEX;
        }

        sArg = getArg(i,args);
        if ("-config".equals(sArg)) {
            i++;
            sConfigFileName = getArg(i++,args);
        }

        sFileName = getArg(i++,args);

        String sFullOutFileName;
        if (i<args.length) { sFullOutFileName = getArg(i,args); }
        else { sFullOutFileName = Misc.removeExtension(sFileName); }
        if (sFullOutFileName.endsWith(File.separator)) { // directory only
            sOutPathName=sFullOutFileName;
            sOutFileName = (new File(sFileName)).getName();
        }
        else { // directory and filename
            File f = new File(sFullOutFileName);
            sOutPathName = f.getParent();
            if (sOutPathName==null) { sOutPathName=""; } else { sOutPathName+=File.separator; }
            sOutFileName = f.getName();
        }
        sOutFileName = Misc.removeExtension(sOutFileName);

        System.out.println();
        System.out.println("This is Writer2" + sOutputFormat
            + ", Version " + VERSION + " (" + DATE + ")");
        System.out.println();

    }


    /**
     *  Extract the next argument from the array, while checking to see
     *  that the array size is not exceeded.  Throw a friendly error
     *  message in case the arg is missing.
     *
     *  @param  i     Argument index.
     *  @param  args  Array of command line arguments.
     *
     *  @return  The argument with the specified index.
     *
     *  @throws  IllegalArgumentException  If an argument is invalid.
     */
    private String getArg(int i, String args[])
        throws IllegalArgumentException {

        if (i < args.length)
            return args[i];
        else throw new
            IllegalArgumentException("I'm sorry, the commandline ended abnormally");
    }



}
