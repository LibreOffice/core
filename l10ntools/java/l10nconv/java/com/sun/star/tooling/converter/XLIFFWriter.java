/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


/*
 *
 /*
 * XLIFFWriter.java
 *
 *
 */

package com.sun.star.tooling.converter;

import java.io.*;
import java.util.*;

import com.sun.star.tooling.DirtyTags.DirtyTagWrapper;
import com.sun.star.tooling.languageResolver.LanguageResolver;
import com.sun.star.tooling.languageResolver.LanguageResolver.LanguageResolvingException;

/**
 * Write the Data to a wellformed XLIFF File
 *
 * @author Christian Schmidt
 */
public class XLIFFWriter extends DataWriter {

    /**
     * An array holding the keys of the HashMap containing the source language string
     */
    private final static String[] sourceLineNames = { "Project", "SourceFile",
            "Dummy", "ResType", "GID", "LID", "HID", "Platform", "Width",
            "SourceLanguageID", "SourceText", "SourceHText", "SourceQText",
            "SourceTitle", "TimeStamp" };
    /**
     * An array holding the keys of the HashMap containing the target language string
     */
    private final static String[] targetLineNames = { "Project", "SourceFile",
            "Dummy", "ResType", "GID", "LID", "HID", "Platform", "Width",
            "TargetLanguageID", "TargetText", "TargetHText", "TargetQText",
            "TargetTitle", "TimeStamp" };
    /**
     * An array holding the keys of the HashMap containing the source and target language string
     */
    private final static String[] outLineNames = { "BlockNr", "Project",
            "SourceFile", "Dummy", "ResType", "GID", "LID", "HID", "Platform",
            "Width", "SourceLanguageID", "SourceText", "SourceHText",
            "SourceQText", "SourceTitle", "TargetLanguageID", "TargetText",
            "TargetHText", "TargetQText", "TargetTitle", "TimeStamp" };
    /**
     * An Map holding the source and target content
     */
    private final Map data = new ExtMap(outLineNames, null);


    /**
     * Indicates if this is the first Transunit to write
     */
    boolean isFirst = true;

    LanguageResolver languageResolver;


    /**
     * Create a new Instance of XLIFFWriter
     *
     * @param bos the Buffered Output Stream to write to
     * @param charset the charset to use
     * @throws IOException
     */
    public XLIFFWriter(BufferedOutputStream bos, String charset)
            throws IOException {
        super(bos, charset);
        this.languageResolver =new LanguageResolver();


    }

    /* (non-Javadoc)
     * @see com.sun.star.tooling.converter.DataWriter#getDatafrom(com.sun.star.tooling.converter.DataHandler)
     */
    public void getDatafrom(DataHandler handler) throws java.io.IOException {
        handler.putDataTo(this.data);
    }

    /**
     * check if the item is an empty String
     *
     * @param item the string to check
     * @return true if it is not empty, false if it is empty
     */
    private final boolean isUsed(String item) {
        if (!"".equals(item))
            return true;
        return false;
    }

    /**
     * Replaces all characters that mustn't be in XLIFF PCdata
     *
     * @param string the string to check
     * @return the checked string with all characters replaced
     * @throws java.io.IOException
     */
    private final String xmlString( final String string) throws java.io.IOException {
        if (string == null)
            return string; // ""
        String str = string;
//      str = str.replaceAll("&", "&amp;");
//      str = str.replaceAll("<", "&lt;");
//      str = str.replaceAll(">", "&gt;");
//      str = str.replaceAll("\"", "&quot;");
//      str = str.replaceAll("'", "&apos;");
         for(int i=0;i<str.length();i++){
             if(str.charAt(i)=='&'){
                 str=str.substring(0, i)+"&amp;"+str.substring(i+1);
                 continue;
             }

             if(str.charAt(i)=='<'){
                 str=str.substring(0, i)+"&lt;"+str.substring(i+1);
                 continue;
             }

             if(str.charAt(i)=='>'){
                 str=str.substring(0, i)+"&gt;"+str.substring(i+1);
                 continue;
             }

             if(str.charAt(i)=='"'){
                 str=str.substring(0, i)+"&quot;"+str.substring(i+1);
                 continue;
             }

             if(str.charAt(i)=='\''){
                 str=str.substring(0, i)+"&apos;"+str.substring(i+1);
                 continue;
             }
         }

        return str;
    }

    /* (non-Javadoc)
     * @see java.io.Writer#close()
     */
    public void close() throws IOException {
        this.writeTrailer();

        super.close();
    }

    /* (non-Javadoc)
     * @see com.sun.star.tooling.converter.DataWriter#writeData()
     */
    public void writeData() throws IOException {
        if (isFirst) {

            writeHeader();
            isFirst = false;
        }
        try{
            writeTransUnit();
        }catch(DirtyTagWrapper.TagWrapperException e){

        }
    }

    /**
     * Write the XLIFFFiles header
     *
     * @throws IOException
     */
    private void writeHeader() throws IOException {

        this.write(getHeader());
    }

    /**
     * Write the XLIFFFiles Trailer
     *
     * @throws IOException
     */
    private void writeTrailer() throws IOException {
        this.write(getTrailer());
    }

    /**
     * Write the next TransUnit
     *
     * @throws IOException
     */
    private void writeTransUnit() throws IOException, DirtyTagWrapper.TagWrapperException {
        try{
            StringBuffer writeBuffer = new StringBuffer(1000);

            StringBuffer allLinesEnd = new StringBuffer(200);
            String sRessource = "";
            int parts = 0;
            if (data == null) {
                OutputHandler.out("error");// TBD Exception
            }

            if (!(this.data.get("SourceText").equals("") || this.data.get(
                    "SourceText").equals(" "))) {
                parts++;
            }
            // if(!(this.data.get("SourceHText").equals("")||this.data.get("SourceHText").equals("
            // "))){
            // parts++;
            // }
            if (!(this.data.get("SourceQText").equals("") || this.data.get(
                    "SourceQText").equals(" "))) {
                parts++;
            }
            if (!(this.data.get("SourceTitle").equals("") || this.data.get(
                    "SourceTitle").equals(" "))) {
                parts++;
            }
            if (!(this.data.get("SourceText").equals("") || this.data.get(
                    "SourceText").equals(" "))) {
                sRessource = "res"; // normal TEXT source

                allLinesEnd
                        .append("\t\t\t\t<context-group name=\"StarOffice Attributes\">\n");

                if (isUsed((String) this.data.get("ResType")))
                    allLinesEnd
                            .append("\t\t\t\t\t<context context-type=\"DBType\">"
                                    + xmlString((String) this.data.get("ResType"))
                                    + "</context>\n");
                if (isUsed((String) this.data.get("Project")))
                    allLinesEnd
                            .append("\t\t\t\t\t<context context-type=\"Project\">"
                                    + xmlString((String) this.data.get("Project"))
                                    + "</context>\n");
                if (isUsed((String) this.data.get("SourceFile")))
                    allLinesEnd
                            .append("\t\t\t\t\t<context context-type=\"Filename\">"
                                    + xmlString((String) this.data
                                            .get("SourceFile")) + "</context>\n");
                if (isUsed((String) this.data.get("SourceHText")))
                    allLinesEnd
                            .append("\t\t\t\t\t<context context-type=\"SourceHelpText\">"
                                    + xmlString((String) this.data
                                            .get("SourceHText")) + "</context>\n");
                if (isUsed((String) this.data.get("TargetHText")))
                    allLinesEnd
                            .append("\t\t\t\t\t<context context-type=\"TargetHelpText\">"
                                    + xmlString((String) this.data
                                            .get("TargetHText")) + "</context>\n");
                if (isUsed((String) this.data.get("ResType")))
                    allLinesEnd.append("\t\t\t\t\t<context context-type=\"Type\">"
                            + xmlString((String) this.data.get("ResType"))
                            + "</context>\n");
                if (isUsed((String) this.data.get("GID")))
                    allLinesEnd.append("\t\t\t\t\t<context context-type=\"GID\">"
                            + xmlString((String) this.data.get("GID"))
                            + "</context>\n");
                if (isUsed((String) this.data.get("LID")))
                    allLinesEnd.append("\t\t\t\t\t<context context-type=\"LID\">"
                            + xmlString((String) this.data.get("LID"))
                            + "</context>\n");
                if (isUsed((String) this.data.get("HID")))
                    allLinesEnd.append("\t\t\t\t\t<context context-type=\"HID\">"
                            + xmlString((String) this.data.get("HID"))
                            + "</context>\n");
                if (isUsed((String) this.data.get("Platform")))
                    allLinesEnd
                            .append("\t\t\t\t\t<context context-type=\"Platform\">"
                                    + xmlString((String) this.data.get("Platform"))
                                    + "</context>\n");
                if (isUsed((String) this.data.get("Width")))
                    allLinesEnd.append("\t\t\t\t\t<context context-type=\"Width\">"
                            + xmlString((String) this.data.get("Width"))
                            + "</context>\n");
                allLinesEnd.append("\t\t\t\t</context-group>\n"
                        + "\t\t\t</trans-unit>\n");

                writeBuffer.append("\t\t\t<trans-unit id=\""
                        + this.data.get("BlockNr") + ":" + parts + "\" restype=\""
                        + sRessource + "\" translate=\"yes\">\n");
                if (isUsed((String) this.data.get("SourceText")))
                    writeBuffer.append("\t\t\t\t<source xml:lang=\""
                            + languageResolver.getRFCFromISO((String)this.data.get("SourceLanguageID")) + "\">"
                            + DirtyTagWrapper.wrapString((String) this.data.get("SourceText"))
                            + "</source>\n");

                if (isUsed((String) this.data.get("TargetText")))
                    writeBuffer
                            .append("\t\t\t\t<target state=\"to_translate\" xml:lang=\""
                                    + languageResolver.getRFCFromISO((String)this.data.get("TargetLanguageID"))
                                    + "\">"
                                    +DirtyTagWrapper.wrapString((String) this.data
                                            .get("TargetText")) + "</target>\n");
                writeBuffer.append(allLinesEnd);
                Converter.countLine();

            }
            // if(!(this.data.get("SourceHText").equals("")||this.data.get("SourceHText").equals("
            // "))){
            // sRessource="res-Help"; //Source is Help
            // //sLineNumber=String.valueOf(iLineNumber);//
            // writeBuffer.append("\t\t<trans-unit
            // id=\""+this.data.get("BlockNr")+":"+parts+"\"
            // restype=\""+sRessource+"\" translate=\"yes\">\n");//always translate
            // if(isUsed((String)this.data.get("SourceHText")))
            // writeBuffer.append("\t\t\t<source
            // xml:lang=\""+this.data.get("SourceLanguageID")+"\">"+xmlString((String)this.data.get("SourceHText"))+"</source>\n");
            // if(isUsed((String)this.data.get("TargetHText")))
            // writeBuffer.append("\t\t\t<target state=\"to_translate\"
            // xml:lang=\""+this.data.get("TargetLanguageID")+"\">"+xmlString((String)this.data.get("TargetHText"))+"</target>\n");
            // writeBuffer.append(allLinesEnd);
            // Converter.countLine();
            // }

            if (!(this.data.get("SourceQText").equals("") || this.data.get(
                    "SourceQText").equals(" "))) {
                sRessource = "res-QuickHelp"; // Source is OuickHelp
                // sLineNumber=String.valueOf(iLineNumber);//
                writeBuffer.append("\t\t\t<trans-unit id=\""
                        + this.data.get("BlockNr") + ":" + parts + "\" restype=\""
                        + sRessource + "\" translate=\"yes\">\n");// always translate
                if (isUsed((String) this.data.get("SourceQText")))
                    writeBuffer.append("\t\t\t\t<source xml:lang=\""
                            + languageResolver.getRFCFromISO((String)this.data.get("SourceLanguageID")) + "\">"
                            + DirtyTagWrapper.wrapString((String) this.data.get("SourceQText"))
                            + "</source>\n");
                if (isUsed((String) this.data.get("TargetQText")))
                    writeBuffer
                            .append("\t\t\t\t<target state=\"to_translate\" xml:lang=\""
                                    + languageResolver.getRFCFromISO((String)this.data.get("TargetLanguageID"))
                                    + "\">"
                                    + DirtyTagWrapper.wrapString((String) this.data
                                            .get("TargetQText")) + "</target>\n");
                writeBuffer.append(allLinesEnd);
                Converter.countLine();
            }

            if (!(this.data.get("SourceTitle").equals("") || this.data.get(
                    "SourceTitle").equals(" "))) {
                sRessource = "res-Title"; // Source is Title

                writeBuffer.append("\t\t\t<trans-unit id=\""
                        + this.data.get("BlockNr") + ":" + parts + "\" restype=\""
                        + sRessource + "\" translate=\"yes\">\n");// always translate
                if (isUsed((String) this.data.get("SourceTitle")))
                    writeBuffer.append("\t\t\t\t<source xml:lang=\""
                            + languageResolver.getRFCFromISO((String)this.data.get("SourceLanguageID")) + "\">"
                            + DirtyTagWrapper.wrapString((String) this.data.get("SourceTitle"))
                            + "</source>\n");
                if (isUsed((String) this.data.get("TargetTitle")))
                    writeBuffer
                            .append("\t\t\t\t<target state=\"to_translate\" xml:lang=\""
                                    + languageResolver.getRFCFromISO((String)this.data.get("TargetLanguageID"))
                                    + "\">"
                                    + DirtyTagWrapper.wrapString((String) this.data
                                            .get("TargetTitle")) + "</target>\n");
                writeBuffer.append(allLinesEnd);
                Converter.countLine();
            }
            this.write(writeBuffer.toString());
        }catch(Exception e){
            OutputHandler.log(e.getMessage());
        }
    }

    /**
     * Create the XLIFFFiles Header
     *
     * @return the header as string
     * @throws java.io.UnsupportedEncodingException
     */
    private String getHeader() throws java.io.UnsupportedEncodingException {
        return new String(
                (getProcessingInstructionTag() + getDTDLine()
                        + openVersionLine() + openFileLine() + getHeaderTag() + openBodyTag())
                        .getBytes(), "UTF8");

    }

    /**
     * Create the XLIFFFiles Trailer
     *
     * @return the trailer as string
     */
    private String getTrailer() {
        return closeBodyTag() + closeFileLine() + closeVersionLine();
    }

    /**
     * Create the Processing Instruction Tag used by this XLIFFFile
     * @return the Processing Instruction Tag used by this XLIFFFile
     */
    private String getProcessingInstructionTag() {
        String sPITagStart = "<?";
        String sPIName = "xml ";
        String sPIVersion = "version=\"1.0\" ";
        String sPIEncoding = "encoding=\"UTF-8\"";
        String sPITagEnd = "?>";
        return sPITagStart + sPIName + sPIVersion + sPIEncoding
                + /* sPIStandalone+ */sPITagEnd + '\n';
    }

    /**
     * Create the line holding the DTD referenced by this XLIFFFile
     * @return a string holding the DTD referenced by this XLIFFFile
     */
    private String getDTDLine() {
        String sDTDTagStart = "<!DOCTYPE ";
        String sDTDType = "xliff ";
        String sDTDSource = "PUBLIC \"-//XLIFF//DTD XLIFF//EN\" \"http://www.oasis-open.org/committees/xliff/documents/xliff.dtd\">";// http://www.oasis-open.org/committees/xliff/documents/
        String sDTSTagEnd = ">";
        return sDTDTagStart + sDTDType + sDTDSource + '\n';
    }

    /**
     * Create the beginning of the line holding the version of this XIFFFile
     *
     * @return a string  with the beginning of the line holding the version of this XIFFFile
     */
    private String openVersionLine() {
        return "<xliff version=\"1.0\">\n";
    }
    /**
     * Create the ending of the line holding the version of this XIFFFile
     *
     * @return a string  with the ending of the line holding the version of this XIFFFile
     */
    private String closeVersionLine() {
        return "</xliff>";
    }
    /**
     * Create the beginning of the line holding the file tag of this XIFFFile
     *
     * @return a string  with the beginning of the file tag of this XIFFFile
     */
    private String openFileLine() {

            String FileTagStart = "\t<file";
            String FileDataType = " datatype=\"STAROFFICE\"";
            String FileDate = " date=\"" + this.data.get("TimeStamp") + "\"";
            String FileOriginal = " original=\"" + this.data.get("SourceFile")
                    + "\"";
            String FileSourceLanguage="";
            String FileTargetLanguage="";
            try {
                FileSourceLanguage = " source-language=\""
                        + languageResolver.getRFCFromISO((String)this.data.get("SourceLanguageID")) + "\" ";
                FileTargetLanguage = " target-language=\""
                        + languageResolver.getRFCFromISO((String)this.data.get("TargetLanguageID")) + "\" ";
            } catch (LanguageResolvingException e) {
                OutputHandler.out(e.getMessage());
            }
            String FileTagEnd = ">";
            return FileTagStart + FileDataType + FileDate + FileOriginal
                + FileSourceLanguage + FileTargetLanguage + FileTagEnd;

    }
    /**
     * Create the ending of the line holding the file tag of this XIFFFile
     *
     * @return a string  with the ending of the file tag of this XIFFFile
     */
    private String closeFileLine() {
        return "\t</file>";
    }
    /**
     * Create a String  containing the header tag
     * @return the String  containing the header tag
     */
    private String getHeaderTag() {
        return "<header></header>\n";
    }
    /**
     * Create the beginning of the line holding the body tag of this XIFFFile
     *
     * @return a string  with the beginning of the body tag of this XIFFFile
     */
    private String openBodyTag() {
        return "\t\t<body>\n";
    }
    /**
     * Create the ending of the line holding the body tag of this XIFFFile
     *
     * @return a string  with the ending of the body tag of this XIFFFile
     */
    private String closeBodyTag() {
        return "\t\t</body>";
    }

    /*
     * (non-Javadoc)
     *
     * @see com.sun.star.tooling.converter.DataWriter#writeData(java.util.Map[])
     */
    protected void writeData(Map[] data) throws IOException {
        // TODO Auto-generated method stub

    }

    /*
     * (non-Javadoc)
     *
     * @see com.sun.star.tooling.converter.DataWriter#getDataFrom(com.sun.star.tooling.converter.DataHandler)
     */
    protected void getDataFrom(DataHandler handler) {   }

}
