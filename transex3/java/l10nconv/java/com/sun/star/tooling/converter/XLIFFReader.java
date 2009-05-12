/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XLIFFReader.java,v $
 * $Revision: 1.4 $
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
/*
 * XLIFFReader.java
 *
 *
 */
package com.sun.star.tooling.converter;

import java.io.IOException;
import java.util.Hashtable;
import java.util.Map;

import com.sun.star.tooling.languageResolver.LanguageResolver;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
import org.xml.sax.helpers.AttributesImpl;
import org.xml.sax.helpers.DefaultHandler;


/**
 * Parse the given file and extract the content needed.
 * <br/>
 * This Reader understands the parts of the
 * <a href="http://www.oasis-open.org/committees/xliff/documents/cs-xliff-core-1.1-20031031.htm">xliff</a> spezification used to translate
 *  the strings in Star-Office and Open-Office.
 *  <br/>
 *  The given file is parsed and the content is stored in a HashMap with those keys:
 *  <br/>
 *  "BlockNr" originally coming from reading the sdf file, contains 'block nr in sdf file'+'-'+'hash value of the sdf id fields'.<br/>
 *   "Project"  first column in sdf file format.<br/>
 *  "SourceFile" second column in sdf file format.<br/>
 *  "Dummy" third column in sdf file format.<br/>
 *  "ResType" 4. column in sdf file format.<br/>
 *  "GID" 5. column in sdf file format. <br/>
 *  "LID" 6. column in sdf file format.<br/>
 *  "HID" 7. column in sdf file format.<br/>
 *  "Platform" 8. column in sdf file format. <br/>
 *  "Width", 9. column in sdf file format.<br/>
 *  "SourceLanguageID" 10. column in sdf file format(in the line with the source language).<br/>
 *  "SourceText"  11. column in sdf file format(in the line with the source language).<br/>
 *  "SourceHText" 12. column in sdf file format(in the line with the source language).<br/>
 *  "SourceQText" 13. column in sdf file format(in the line with the source language).<br/>
 *  "SourceTitle" 14. column in sdf file format(in the line with the source language).<br/>
 *  "TargetLanguageID" 10. column in sdf file format (in the line with the target language).<br/>
 *  "TargetText" 11. column in sdf file format (in the line with the target language).<br/>
 *  "TargetHText" 12. column in sdf file format (in the line with the target language).<br/>
 *  "TargetQText"  13. column in sdf file format (in the line with the target language).<br/>
 *  "TargetTitle", 14. column in sdf file format (in the line with the target language).<br/>
 *  "TimeStamp" 15. column in sdf file format.<br/>
 * @
 * @author Christian Schmidt 2005
 *
 */
public class XLIFFReader extends DefaultHandler {

    /**
     * A String array holding the keys used by the HashMap holding the Data
     */
    private final String[]      dataNames     = { "BlockNr", "Project",
            "SourceFile", "Dummy", "ResType", "GID", "LID", "HID", "Platform",
            "Width", "SourceLanguageID", "SourceText", "SourceHText",
            "SourceQText", "SourceTitle", "TargetLanguageID", "TargetText",
            "TargetHText", "TargetQText", "TargetTitle", "TimeStamp" };

    /**
     * Used to index in the data array
     */
    static int                  index         = 0;

    /**
     * The Map that holds the data returned by this class
     */
    private Map                 moveData      = new ExtMap();

    /**
     * A Map that holds  yet incomplete data
     * until all depending transunits are found
     */
    private Hashtable           DataStore     = new Hashtable();

    /**
     * An Elements name
     */
    private String              name          = new String("");

    /**
     *List of Attributes used by an Element
     */
    private Attributes          attrs;

//    private String              tagElement    = new String("");

    /**
     * Indicates whether the next found content string should be printed
     */
    private boolean             printThis     = false;
    /**
     * Indicates whether the next found content string should be stored
     */
    private boolean             storeIt       = false;

    /**
     * data holds the information created while parsing
     *
     */
    private String[]            data          = new String[26];

    /**
     * The handler used by this class
     */
    private final DataHandler   handler;
    /**
     * The target used by this class
     */
    private final DataWriter    target;

//    private boolean             searchForText = false;

    /**
     * counts how many dots are made
     */
    private int                 dotCount;

    /**
     * Counts how many Trans Units are read
     */
    private int                 transUnitCounter;

    /**
     * used source Language
     */
    private String              sourceLanguage;

    /**
     * used target language
     */
    private String              targetLanguage;

    /**
     * indicates whether this is the first Transunit
     */
    private boolean             isFirst       = true;

    private static final String EMPTY         = new String("");

    /**
     * the last index in data where something is written
     */
    private int                 oldindex;

//    private boolean isBptEptTag;

//    private String innerString;
//
//    private String key;

    /**
     * Index for the BlockNr in the data array
     */
    private static final int BLOCKNR_IDX = 0;
    /**
     * Index for the Project in the data array
     */
    private static final int PROJECT_IDX = 1;
    /**
     * Index for the Sourcefile name in the data array
     */
    private static final int SOURCEFILE_IDX = 2;
    /**
     * Index for the 'dummy' in the data array
     */
    private static final int DUMMY_IDX = 3;
    /**
     * Index for the Group Id in the data array
     */
    private static final int GID_IDX = 4;
    /**
     * Index for the Local Id in the data array
     */
    private static final int LID_IDX = 5;
    /**
     * Index for the Help Id in the data array
     */
    private static final int HID_IDX = 6;
    /**
     * Index for the Platform in the data array
     */
    private static final int PLATFORM_IDX = 7;
    /**
     * Index for the 'Width' in the data array
     */
    private static final int WIDTH_IDX = 8;
    /**
     * Index for the Sourcelanguage Id in the data array
     */
    private static final int SOURCE_LANGUAGE_ID_IDX = 10;
    /**
     * Index for the Source Text in the data array
     */
    private static final int SOURCE_TEXT_IDX = 11;
    /**
     * Index for the Source Helptext in the data array
     */
    private static final int SOURCE_HELPTEXT_IDX = 12;
    /**
     * Index for the Source Quickhelp Text in the data array
     */
    private static final int SOURCE_QUICK_HELPTEXT_IDX = 13;
    /**
     * Index for the Source Titletext in the data array
     */
    private static final int SOURCE_TITLETEXT_IDX = 14;
    /**
     * Index for the Timestamp in the data array
     */
    private static final int TIMESTAMP_IDX = 15;
    /**
     * Index for the res type in the data array
     */
    private static final int RESTYPE_IDX = 16;
    /**
     * Index for the Target Language Id in the data array
     */
    private static final int TARGET_LANGUAGE_ID_IDX = 20;
    /**
     * Index for the Target Text in the data array
     */
    private static final int TARGET_TEXT_IDX = 21;
    /**
     * Index for the Target Helptext in the data array
     */
    private static final int TARGET_HELP_TEXT_IDX = 22;
    /**
     * Index for the Target Quickhelp Text in the data array
     */
    private static final int TARGET_QUICKHELP_TEXT_IDX = 23;
    /**
     * Index for the Target Titletext in the data array
     */
    private static final int TARGET_TITLE_TEXT_IDX = 24;
    /**
     * Index for the Found Parts Counter in the data array
     */
    private static final int FOUND_PARTS_COUNTER_IDX = 18;

    /**
     * used to find the matching ISO or RFC3066 language code
     */
    LanguageResolver languageResolver;

    private boolean doBlockCompleteCheck=true;



    /**
     * Create a new Instance of XLIFFReader
     *
     * @param handler the DataHandler to use
     * @param target the target used
     * @throws IOException
     */
    public XLIFFReader(DataHandler handler, DataWriter target) throws IOException {
        this.languageResolver = new LanguageResolver();
        this.handler = handler;
        this.target = target;
    }

    /**
     * Create a new Instance of XLIFFReader
     *
     * @param handler the DataHandler to use
     * @param target the target used
     * @param doBlockCompleteCheck indicates whether every single transunit should be returned or the whole block data is to be collected
     *
     * @throws IOException
     */
    public XLIFFReader(DataHandler handler, DataWriter target,boolean doBlockCompleteCheck) throws IOException {
        this(handler, target);
        this.languageResolver = new LanguageResolver();
        this.doBlockCompleteCheck=doBlockCompleteCheck;

    }

    /**
     * delete and initialize the data content
     */
    public void initData() {
        for (int i = BLOCKNR_IDX; i < SOURCE_LANGUAGE_ID_IDX; i++) {
            data[i] = "";
        }
        for (int i = SOURCE_TEXT_IDX; i < TIMESTAMP_IDX; i++) { // skip Time Stamp
            data[i] = "";
        }
        for (int i = RESTYPE_IDX; i < TARGET_LANGUAGE_ID_IDX; i++) { // skip Source language ID
            data[i] = "";
        }
        for (int i = TARGET_TEXT_IDX; i < 26; i++) {// skip Target language ID,
            data[i] = "";
        }

        data[DUMMY_IDX] = "0";//dummy
        data[FOUND_PARTS_COUNTER_IDX] = "1";//parts found

    }

    /** (non-Javadoc)
     * @see org.xml.sax.ContentHandler#startDocument()
     */
    public void startDocument() {
        initData();
        //System.out.print("Start");

    }

    /** (non-Javadoc)
     * @see org.xml.sax.ContentHandler#endDocument()
     */
    public void endDocument() {

        try {
            showStatistic();
        } catch (IOException e) {

            OutputHandler.log(e.getMessage());

        }
    }

    /** (non-Javadoc)
     * @throws SAXException
     * @see org.xml.sax.ContentHandler#startElement(java.lang.String, java.lang.String, java.lang.String, org.xml.sax.Attributes)
     */
    public void startElement(String namespaceURI, String sName, String qName,
            Attributes attrs) throws SAXException {
        this.name = new String(qName);
        this.attrs = new AttributesImpl(attrs);
        String resType;

        String attributeName = new String("");
        String attribute = new String("");
        String tagElement = new String("");
        int i;

        if (qName.equals("bpt")||qName.equals("ept")||qName.equals("sub")||qName.equals("ex")) {
            //ignore bpt, ept, ex  and sub tags
            // content of the tags will be stored

            storeIt=true;
            return;

        }
        if (qName.equals("target")) {
            if ((resType = data[RESTYPE_IDX]) == null) {

            } else {
                if ("res".equals(resType)) {
                    index = TARGET_TEXT_IDX;

                    storeIt = true;
                    return;
                }
                //                if("res-Help".equals(resType)){
                //                    index=TARGET_HELP_TEXT_IDX;
                //                    storeIt=true;
                //                    return;
                //                }
                if ("res-QuickHelp".equals(resType)) {
                    index = TARGET_QUICKHELP_TEXT_IDX;

                    storeIt = true;
                    return;
                }
                if ("res-Title".equals(resType)) {
                    index = TARGET_TITLE_TEXT_IDX;

                    storeIt = true;
                    return;
                }
            }

        }
        if (qName.equals("source")) {
            if ((resType = data[RESTYPE_IDX]) == null) {
                //throw new SAXException("Ressource type not found");
            } else {
                if ("res".equals(resType)) {
                    index = SOURCE_TEXT_IDX;

                    storeIt = true;
                    return;
                }
                //                if("res-Help".equals(resType)){
                //                    index=SOURCEHELPTEXT_IDX;
                //                    storeIt=true;
                //                    return;
                //                }
                if ("res-QuickHelp".equals(resType)) {
                    index = SOURCE_QUICK_HELPTEXT_IDX;
                    storeIt = true;
                    return;
                }
                if ("res-Title".equals(resType)) {
                    index = SOURCE_TITLETEXT_IDX;
                    storeIt = true;
                    return;
                }
            }
        }

        if (qName.equals("file")) {
            data[TIMESTAMP_IDX] = attrs.getValue("date");
            //data[17]=(attrs.getValue("original"));
            try{
                data[SOURCE_LANGUAGE_ID_IDX] = (languageResolver.getISOFromRFC((String)attrs.getValue("source-language")));
                if(languageResolver.getISOFromRFC((String)attrs.getValue("target-language"))!=null){
                    data[TARGET_LANGUAGE_ID_IDX] = (languageResolver.getISOFromRFC((String)attrs.getValue("target-language")));
                }
            }catch(Exception e){
                OutputHandler.log(e.getMessage());
            }
            return;
        }
        if (qName.equals("trans-unit")) {
            String id = attrs.getValue("id");
            if ((DataStore.get(id)) != null) {
                //TODO arraycopy might not be nessessary
                System.arraycopy((String[]) DataStore.get(id), 0, data, 0,
                        data.length);
                int help = (new Integer(data[FOUND_PARTS_COUNTER_IDX])).intValue(); //found one more part
                help++; // refresh the actual found parts
                data[FOUND_PARTS_COUNTER_IDX] = (new Integer(help)).toString(); // belonging to this information

                DataStore.remove(attrs.getValue("id")); // TODO this can be deleted?
            } else {

                data[BLOCKNR_IDX] = (attrs.getValue("id")); // a new part
            }
            data[RESTYPE_IDX] = (attrs.getValue("restype"));

            return;
        }

        if (qName.equals("context")) {

            String value = attrs.getValue("context-type");

            if ("SourceHelpText".equals(value)) {
                index = SOURCE_HELPTEXT_IDX;
                storeIt = true;
                return;
            }else if ("TargetHelpText".equals(value)) {
                index = TARGET_HELP_TEXT_IDX;
                storeIt = true;
                return;
            }else if ("DBType".equals(value)) {
                //index=SOURCEFILE_IDX;
                //storeIt=true;
                return;
            }else if ("Project".equals(value)) {
                index = PROJECT_IDX;
                storeIt = true;
                return;
            }else if ("Filename".equals(value)) {
                index = SOURCEFILE_IDX;
                storeIt = true;
                return;
            }else if ("Type".equals(value)) {
                index = RESTYPE_IDX;
                storeIt = true;
                return;
            }else if ("GID".equals(value)) {
                index = GID_IDX;
                storeIt = true;
                return;
            }else if ("LID".equals(value)) {
                index = LID_IDX;
                storeIt = true;
                return;
            }else if ("HID".equals(value)) {
                index = HID_IDX;
                storeIt = true;
                return;
            }else if ("Platform".equals(value)) {
                index = PLATFORM_IDX;
                storeIt = true;
                return;
            }else if ("Width".equals(value)) {
                index = WIDTH_IDX;
                storeIt = true;
                return;
            }

        }

    }

    /** (non-Javadoc)
     * @see org.xml.sax.ContentHandler#endElement(java.lang.String, java.lang.String, java.lang.String)
     */
    public void endElement(String namespaceURI, String sName, String qName)
            throws SAXException {
        //we ignore bpt and ept tags
        if(!(qName.equals("bpt")||qName.equals("ept")||qName.equals("sub")||qName.equals("ex"))){
            storeIt = false;
        }
        if (qName.equals("trans-unit")) {
            showData();
        }

    }

    /** (non-Javadoc)
     * @see org.xml.sax.ContentHandler#characters(char[], int, int)
     */
    public void characters(char[] ch, int start, int length) {

        // checkContent();
        String str2 = new String(ch, start, length);

        if (storeIt) {

            String str = new String(ch, start, length);
            if (index == oldindex) {
                data[index] += str;
            } else {
                data[index] = str;
            }

        }
        oldindex = index;

    }

    /** (non-Javadoc)
     * @see org.xml.sax.ErrorHandler#error(org.xml.sax.SAXParseException)
     */
    public void error(SAXParseException e) throws SAXParseException {

        OutputHandler.log(e.getMessage());
    }

    /** (non-Javadoc)
     * @see org.xml.sax.ErrorHandler#fatalError(org.xml.sax.SAXParseException)
     */
    public void fatalError(SAXParseException e) throws SAXParseException {

        OutputHandler.log("PARSE ERROR in line " + e.getLineNumber() + ", "
                + e.getMessage() );

    }

    /** (non-Javadoc)
     * @see org.xml.sax.ErrorHandler#warning(org.xml.sax.SAXParseException)
     */
    public void warning(SAXParseException e) throws SAXParseException {
        //throw e;
        OutputHandler.log(e.getMessage());
    }

    /**
     * Put the Data to the DataHandler
     * tell the Writer to write it
     *
     * @throws SAXException
     */
    public void showData() throws SAXException {
        transUnitCounter++;
        makeDot();
        if (isComplete()) {

            try {
                moveData();
                if (isFirst == true) {
                    this.sourceLanguage = (String) this.moveData
                            .get("SourceLanguageID");
                    this.targetLanguage = (String) this.moveData
                            .get("TargetLanguageID");
                    OutputHandler.out(EMPTY);
                    OutputHandler.out("Source Language is: "
                            + this.sourceLanguage);
                    OutputHandler.out("Target Language is: "
                            + this.targetLanguage);
                    OutputHandler.out(EMPTY);
                    OutputHandler.out("Start");
                    OutputHandler.out(EMPTY);
                    isFirst = false;
                }
                target.getDataFrom(handler);
                target.writeData();

            } catch (java.io.IOException e) {
                throw new SAXException(e);
            }

        } else {
            DataStore.put(data[BLOCKNR_IDX], data.clone());
            initData();

        }
        initData();
    }


    /**
     * put the data in an Map in the format that
     * DataHandler can handle it
     */
    final public void moveData() {

        moveData.put("BlockNr", data[BLOCKNR_IDX]);

        moveData.put("Project", data[PROJECT_IDX]);

        moveData.put("SourceFile", data[SOURCEFILE_IDX]);

        moveData.put("Dummy", "0");

        moveData.put("ResType", data[RESTYPE_IDX]);

        moveData.put("GID", data[GID_IDX]);

        moveData.put("LID", data[LID_IDX]);

        moveData.put("HID", data[HID_IDX]);

        moveData.put("Platform", data[PLATFORM_IDX]);

        if (EMPTY.equals(data[WIDTH_IDX]))
            data[WIDTH_IDX] = "0";
        moveData.put("Width", data[WIDTH_IDX]);

        moveData.put("SourceLanguageID", data[SOURCE_LANGUAGE_ID_IDX]);

        moveData.put("SourceText", data[SOURCE_TEXT_IDX]);

        moveData.put("SourceHText", data[SOURCE_HELPTEXT_IDX]);

        moveData.put("SourceQText", data[SOURCE_QUICK_HELPTEXT_IDX]);

        moveData.put("SourceTitle", data[SOURCE_TITLETEXT_IDX]);

        moveData.put("TargetLanguageID", data[TARGET_LANGUAGE_ID_IDX]);

        moveData.put("TargetText", data[TARGET_TEXT_IDX]);

        moveData.put("TargetHText", data[TARGET_HELP_TEXT_IDX]);

        moveData.put("TargetQText", data[TARGET_QUICKHELP_TEXT_IDX]);

        moveData.put("TargetTitle", data[TARGET_TITLE_TEXT_IDX]);

        moveData.put("TimeStamp", data[TIMESTAMP_IDX]);

        //and give it to the data handler
        this.handler.fillDataWith(moveData);
    }

    /**
     * complete means all depending parts have been found esp. all res types
     * that belong to the same SDF Line
     *
     * @return true if the data is complete
     *
     */
    final public boolean isComplete() {

        if(!doBlockCompleteCheck){
            return true;
        }

        String sParts;
        if (data[FOUND_PARTS_COUNTER_IDX] == EMPTY)
            data[FOUND_PARTS_COUNTER_IDX] = "1"; //this is the first part

        String sFoundParts = data[FOUND_PARTS_COUNTER_IDX];
        //create the new 'id'
        sParts = data[BLOCKNR_IDX].substring(data[BLOCKNR_IDX].lastIndexOf(":") + 1);

        if (sFoundParts.equals(sParts)) {
            return true;
        }
        return false;
    }

    // TODO this belongs in OutputHandler
    /**
     * show the user that it is going
     * on by printing dots on the screen
     *
     */
    private void makeDot() {
        int count = 0;
        if ((count = (int) this.transUnitCounter / 1000) > this.dotCount) {
            this.dotCount = count;
            OutputHandler.printDot();
        }
    }

    /**
     * show the statistic data found while parse this file
     *
     * @throws IOException
     */
    final void showStatistic() throws IOException {
        OutputHandler.out(EMPTY);
        OutputHandler.out("TransUnits found:  " + this.transUnitCounter);
        // every data in DataStore is
        // skipped 'cause its not complete
        // TODO count really every transunit not only the data (might consist of
        // more than one
        OutputHandler.dbg("TransUnits skip :  " + this.DataStore.size());
        //Converter.out(EMPTY);
    }
}

