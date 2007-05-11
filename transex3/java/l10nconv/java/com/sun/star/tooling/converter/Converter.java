/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Converter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 09:08:04 $
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
/*
 * Converter.java
 *
 * create Source and Target
 * for converting
 * TODO maybe a factory would be good here
 */

package com.sun.star.tooling.converter;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.EntityResolver;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.DefaultHandler;

/**
 *
 * This class handles the creating of the source to read from,
 * the target to write to and the appropriate DataHandler
 *
 * @author Christian Schmidt 2005
 */
public final class Converter {

    /**
     * the used charset f.e. UTF-8
     */
    private final static String         CHARSET        = new String("UTF-8");

    private static final String EMPTY          = new String("");

    /**
     * The DataHandler use to connect reader and writer
     */
    private static DataHandler  handler;

    /**
     * Counting the lines written by a writer
     */
    private static int          lineCounter;

    /**
     * The target to write to
     */
    private static DataWriter   theTargetWriter;


    /**
     * Overwrite existing files
     */
    private boolean             overwrite      = false;
    /**
     * The source to read from
     */
    private Source              reader;
    /**
     * The name of the source file
     */
    private String              sourceString;
    /**
     * the Type of the Source file(SDF,GSI,XLIFF)
     */
    private String              sourceType;
    /**
     * The name of the target fille
     */
    private String              targetString;
    /**
     * the Type of the Target file(SDF,GSI,XLIFF)
     */
    private String              targetType;
    /**
     * The writer that handles the output
     */
    private Target              writer;

    /**
     * The sourceLanguage
     */
    String                      sourceLanguage = "en-US";

    /**
     * The targetLanguage
     */
    String                      targetLanguage = "";

    /**
     * The name of the second source, needed for GSI to SDF merge
     */
    private String secondSourceString=EMPTY;


    /**
     * Get the line counter
     * @return Returns the lineCounter.
     */
    public static int getLineCounter() {
        return lineCounter;
    }

    /**
     * increment the lineCounter
     */
    final static void countLine() {
        lineCounter++;
    }

    /**
     * Creates a new instance of Converter
     *
     * @param sourceType        the type of the sourceFile
     * @param sourceString      the name of the sourceFile
     * @param SourceLanguage    the ISO Id of the sourceLanguage
     * @param targetType        the type of the targetFile
     * @param targetString      the name of the targetFile
     * @param TargetLanguage    the ISO Id of the targetLanguage
     * @param secondSourceString     the name of the second sourceFile (GSI merge only)
     * @param overwrite         indicates whether overwrite existing files
     * @throws IOException
     * @throws Exception
     */
    public Converter(String sourceType, String sourceString,
            String SourceLanguage, String targetType, String targetString,
            String TargetLanguage,String secondSourceString, boolean overwrite) throws IOException,
            Exception {

        this.sourceType = sourceType;
        this.sourceString = sourceString;
        this.sourceLanguage = SourceLanguage;
        this.targetType = targetType;
        this.targetString = targetString;
        this.targetLanguage = TargetLanguage;
        this.secondSourceString=secondSourceString;
        this.overwrite = overwrite;

        handler = new DataHandler();

        if ("sdf".equalsIgnoreCase(sourceType)) {
            reader = new SDFSource();
        } else if ("xliff".equalsIgnoreCase(sourceType)||"dbxliff".equalsIgnoreCase(sourceType)) {
            reader = new XLIFFSource();
        } else if ("gsi".equalsIgnoreCase(sourceType)) {
            reader = new GSISource();
        } else {
            throw new ConverterException("Unknown Source File Type: '"+sourceType+"'");
        }

        if ("sdf".equalsIgnoreCase(targetType)) {
            writer = new SDFTarget();
        } else if ("xliff".equalsIgnoreCase(targetType)) {
            writer = new XLIFFTarget();
        } else if ("gsi".equalsIgnoreCase(targetType)) {
            writer = new GSITarget();
        } else {
            throw new ConverterException("Unknown Target File Type: '"+targetType+"'");
        }

    }

    /**
     * Do the converting from the source file format to the target file format
     *
     * @throws IOException
     */
    public final void convert() throws IOException {

        try {

            reader.convertTo(writer);

            //TODO this belongs in the Target Class
            theTargetWriter.flush();
            theTargetWriter.close();
        } catch (Exception e) {
            OutputHandler.out(e.getMessage());
        }

    }


    /**
     *
     * Encapsulate the reading from an GSI file
     *
     * @author Christian Schmidt 2005
     *
     */
    private class GSISource implements Source {
        DataReader theSourceReader;
        /**
         * Create a new Instance of GSISource
         *
         * @throws IOException
         * @throws Exception
         */
        public GSISource() throws IOException {

            theSourceReader = new GSIandSDFMerger(new File(sourceString),new File(secondSourceString), sourceLanguage,
                    targetLanguage, CHARSET);
        }

        public void convertTo(Target t) {

            try {
                theTargetWriter = t.getWriter();
                while (handler.fillDataFrom(theSourceReader)) {

                    theTargetWriter.getDatafrom(handler);
                    theTargetWriter.writeData();
                }
            } catch (IOException e) {
                OutputHandler.out(e.getMessage());

            } catch (Exception e) {
                e.printStackTrace();
            }

        }

    }
    /**
     *
     * Encapsulate to write to a GSI file
     *
     * @author Christian Schmidt 2005
     *
     */
    private class GSITarget implements Target {

        File target;

        /**
         * Create a new Instance of GSITarget
         *
         * @throws FileNotFoundException
         * @throws IOException
         */
        public GSITarget() throws FileNotFoundException, IOException {

            target = FileMaker.newFile(targetString, overwrite);
            theTargetWriter = new GSIWriter(new BufferedOutputStream(
                    new FileOutputStream(target)), CHARSET);
        }

        public DataWriter getWriter() {

            return theTargetWriter;
        }

    }
    /**
     *
     * Encapsulate the reading from an SDF file
     *
     * @author Christian Schmidt 2005
     *
     */
    private final class SDFSource implements Source {

        DataReader Source;

        /**
         * Create a new Instance of SDFSource
         * @throws IOException
         * @throws Exception
         */
        public SDFSource() throws IOException, Exception {

            Source = new SDFReader(new File(sourceString), sourceLanguage,
                    targetLanguage, CHARSET);
        }

        public void convertTo(Target t) {
            try {
                theTargetWriter = t.getWriter();
                while (handler.fillDataFrom(Source)) {

                    theTargetWriter.getDatafrom(handler);
                    theTargetWriter.writeData();
                }

            } catch (IOException e) {
                OutputHandler.out(e.getMessage());

            } catch (Exception e) {
                e.printStackTrace();
            }

        }

    }
    /**
     * Encapsulate writing to a SDF file
     *
     * @author Christian Schmidt 2005
     *
     */
    private class SDFTarget implements Target {

        /**
         * Create a new Instance of SDFTarget
         *
         * @throws IOException
         */
        public SDFTarget() throws IOException {
            File target = FileMaker.newFile(targetString, overwrite);
            theTargetWriter = new SDFWriter(new BufferedOutputStream(
                    new FileOutputStream(target)), CHARSET);
        }

        /* (non-Javadoc)
         * @see com.sun.star.tooling.converter.Converter.Target#getWriter()
         */
        public DataWriter getWriter() {

            return theTargetWriter;
        }

    }

    /**
     * The interface for all convertable sources
     *
     * @author Christian Schmidt 2005
     *
     */
    private interface Source {

        DataReader Source=null;
        /**
         * Convert this. to the designated target
         * @param target the target of the converting
         * @throws IOException
         */
        abstract void convertTo(Target target) throws IOException;
    }

    /**
     * The interface for all creatable targets
     *
     * @author Christian Schmidt 2005
     *
     */
    private interface Target {
        /**
         * The writer to use
         */
        public OutputStream writer = null;

        /**
         * Get the writer
         * this target uses to write the
         * data in the correct format.
         *
         * @return the used DataWriter
         */
        abstract DataWriter getWriter();

    }
    /**
     * Encapsulate the reading from an XLIFF file
     *
     * @author Christian Schmidt 2005
     *
     */
    private class XLIFFSource implements Source {
        File source;

        /**
         * Create a new Instance of XLIFFSource
         *
         *
         */
        public XLIFFSource() {

            source = new File(sourceString);
        }

        public void convertTo(Target t) throws IOException {
            try {
                System.setProperty("entityExpansionLimit", "1000000");
                boolean laden = source.canRead();
                if (laden) {
                    DefaultHandler contentHandler=null;
                    if("dbxliff".equalsIgnoreCase(sourceType)){
                        contentHandler = new XLIFFReader(handler, t
                                .getWriter(),false);
                    }else{
                        contentHandler = new XLIFFReader(handler, t
                            .getWriter());
                    }
                    SAXParserFactory factory = SAXParserFactory.newInstance();
                    factory.setNamespaceAware( true );
                    factory.setValidating( true );

                    SAXParser parser=factory.newSAXParser();
                    XMLReader xliffreader=parser.getXMLReader();



//                    XMLReader xliffreader = XMLReaderFactory
//                            .createXMLReader("org.apache.crimson.parser.XMLReaderImpl");//
                    xliffreader.setErrorHandler(contentHandler);
                    xliffreader.setContentHandler(contentHandler);
                    /* one possibility to resolve an extern entity (DTD) */
                    EntityResolver res = new Resolver();
                    xliffreader.setEntityResolver(res);
                    /*
                     * a second possibility to resolve an extern entity (DTD)
                     *
                     * xliffreader.setFeature("xml.org/sax/features/validation",true);
                     * xliffreader.setEntityResolver(new EntityResolver() {
                     * public InputSource resolveEntity(java.lang.String
                     * publicId, java.lang.String systemId) throws SAXException,
                     * java.io.IOException { if (publicId.equals("-//XLIFF//DTD
                     * XLIFF//EN")) // this deactivates the open office DTD
                     * return new InputSource(new ByteArrayInputStream( " <?xml
                     * version='1.0' encoding='UTF-8'?>" .getBytes())); else
                     * return null; } });
                     *
                     */

                    xliffreader.parse(sourceString);

                } else {
                    System.out.println("Datei existiert nicht");
                }

            } catch (SAXParseException e) {
                try {
                    theTargetWriter.flush();
                } catch (IOException e1) {

                    e1.printStackTrace();
                }
                OutputHandler.out("PARSE ERROR Zeile " + e.getLineNumber()
                        + ", " + e.getMessage());

            }catch (SAXException e){
                try {
                    theTargetWriter.flush();
                } catch (IOException e1) {

                    e1.printStackTrace();
                }
                OutputHandler.out("PARSE EXCEPTION " +  e.getMessage());
            } catch (ParserConfigurationException e) {
                OutputHandler.out("PARSER Configuration failed\n " +  e.getMessage());
            }
        }

    }
    /**
     * Encapsulate writing to a XLIFF file
     *
     * @author Christian Schmidt 2005
     *
     */
    private class XLIFFTarget implements Target {
        File target;

        /**
         * Create a new Instance of XLIFFTarget
         *
         * @throws FileNotFoundException
         * @throws IOException
         */
        public XLIFFTarget() throws FileNotFoundException, IOException {
            target = FileMaker.newFile(targetString, overwrite);
            theTargetWriter = new XLIFFWriter(new BufferedOutputStream(
                    new FileOutputStream(target)), CHARSET);

        }

        public DataWriter getWriter() {

            return theTargetWriter;
        }
    }


}