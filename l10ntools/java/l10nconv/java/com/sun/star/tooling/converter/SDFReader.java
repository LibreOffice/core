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
/*
 * SDFReader.java
 *
 *
 */

package com.sun.star.tooling.converter;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.DecimalFormat;
import java.util.*;

/**
 *
 * SDFReader is a FileReader that knows about
 * the content of SDFFiles
 *
 * A SDFBlock is read of the given file. A SDFBlock
 * consists of all SDFLines that are traanslations of the
 * same String and the SDFLine containing the source string
 * itself. SDFFile lines are read and checked whether they
 * have the allowed column count and  don't contain illeagal
 * characters (like most unprintable characters below 0x00df).
 * If the given source language is not found in the first
 * block of SDFLines a ConverterException is thrown at runtime.
 * If the given target language is "" (that means not given)
 * the first language that is not the given source language
 * is taken for target language. The found values are returned in HashMaps that
 * use the following keys:
 * <br/>
 *  "BlockNr" originally coming from reading the sdf file, contains 'block nr in sdf file'+'-'+'hash value of the sdf id fields'.<br/>
 *  "Project"  first column in sdf file format.<br/>
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
 *
 * @author Christian Schmidt 2005
 *
 */

public class SDFReader extends DataReader {

    /**
     * an array of the SDF files column names
     */
    final static String[] fieldnames      = { "Project", "SourceFile", "Dummy",
            "ResType", "GID", "LID", "HID", "Platform", "Width", "LanguageID",
            "Text", "HText", "QText", "Title", "TimeStamp" };

    /**
     * an array of the SDF files column names if the source language is in
     */
    final static String[] sourceLineNames = { "Project", "SourceFile", "Dummy",
            "ResType", "GID", "LID", "HID", "Platform", "Width",
            "SourceLanguageID", "SourceText", "SourceHText", "SourceQText",
            "SourceTitle", "TimeStamp"   };

    /**
     * an array of the SDF files column names if the target language is in
     */
    final static String[] targetLineNames = { "Project", "SourceFile", "Dummy",
            "ResType", "GID", "LID", "HID", "Platform", "Width",
            "TargetLanguageID", "TargetText", "TargetHText", "TargetQText",
            "TargetTitle", "TimeStamp"   };

    final static String   EMPTY           = new String("");

    private int           dotCount        = 0;

    /**
     * a Map containing an SDF line with source language
     */
    private Map           sourceMap;

    /**
     * a Map containing an SDF line with target language
     */
    private Map           targetMap;

    /**
     * a Map containing an SDF
     */
    private Map           SDFMap;

    /**
     * a Map Array containing one SDF source language line and one SDF target
     * language line
     */
    private Map[]         data            = { sourceMap, targetMap };

    /**
     * The Id of the current SDFBlock
     */
    private String        CurrentBlockId;

    /**
     * The SDF file to read from
     */
    private File          sourceFile;

    /**
     * The language in the source file that should be handelt as source language
     */
    protected String        sourceLanguage;

    /**
     * The language in the source file that should be handelt as target language
     */
    protected String      targetLanguage;

    /**
     * A counter holding the number of blocks just read
     * from this file
     */
    private long           blockNr         = 0;// If we use Integer, more then numbers greater than 128k would be signed

    /**
     * A counter holding the number of skipped lines that means
     * lines that can not be worked with because they contain an error
     */
    private int           skippedLines    = 0;

    /**
     * This switch is set for indicating that all source file lines
     * are read and no lines remain buffered. Finding this like 'true'
     * means the source file is finished
     */
    private boolean       endIt           = false;

    /**
     * Indicates whether the targetLanguage is found in this source file so far
     */
    private boolean       foundTarget     = false;
    /**
     * Indicates whether the sourceLanguage is found in this source file so far
     */
    private boolean       foundSource     = false;

    /**
     * Counts how many lines were skipped because the language is
     * neither sourceLanguage nor targetLanguage
     */
    private int           langMiss;

    /**
     *  Indicates whether there is a line in the read buffer or not
     */
    private boolean       useBuffer       = false;

    /**
     * A buffer for SDFLines
     */
    private String        lineBuffer;

    /**
     * The buffer for the already splitted SDFLines
     */
    private String[]      splittedLineBuffer;

    /**
     * Counts how many Blocks were skipped
     * f.e. because no sourceLanguage is found
     * in it
     */
    private int           skippedBlocks;

    /**
     * Counts the blocks without targetLanguage
     */
    private int           targetLangMiss;

    /**
     * Counts the blocks without sourceLanguage
     */
    private int           sourceLangMiss;

    /**
     * Counts the lines where no targetLanguage line was found
     * and so empty lines were created
     */
    private int           targetLangCreate;


    DecimalFormat blockNrFormatter = new DecimalFormat("000000");

    /**
     * The hashcode of the current block
     */
    private int CurrentBlockHash;

    private boolean skip;

    /**
     * Create a new Instance of SDFREader
     *
     * @param source                the file to read from
     * @param sourceLanguage        the sourceLanguage (must not be empty)
     * @param targetLanguage        the targetLanguage
     * @param charset               the charset used to read source
     * @throws java.io.IOException
     * @throws Exception
     */
    public SDFReader(File source, String sourceLanguage, String targetLanguage,
            String charset) throws java.io.IOException {
        super(new InputStreamReader(new FileInputStream(source), charset));
        sourceFile = source;
        this.sourceLanguage = sourceLanguage;
        this.targetLanguage = targetLanguage;
        String line;
        String[] splitLine;
        //read first line to get the first
        //SDF block id
        mark(16000);
        if ((line = readLine()) != null) {
            if ((splitLine = split(line)) != null){
                this.CurrentBlockId = getSDFBlockId(splitLine);
                this.CurrentBlockHash=this.CurrentBlockId.hashCode();
                //found the first
                this.blockNr++;
                }
            this.splittedLineBuffer = splitLine;
        }
        reset();

    }

    /* (non-Javadoc)
     * @see com.sun.star.tooling.converter.DataReader#getData()
     */
    public Map getData()throws IOException {

        Map map=new HashMap();
//        do {
            this.skip=false;
            Map[] help=readBlock();
            if(help==null||help[1]==null||help[0]==null){
                return null;
//            }else if (help[1].get("TargetLanguageID")==null||help[0].get("SourceLanguageID")==null) {
//                OutputHandler.log("Missing Language Id in block "+blockNr+"\nthe block is skipped." );
//                this.skippedBlocks++;
//                this.skip=true;
            }else{
                map.putAll(help[1]);
                map.putAll(help[0]);

            }
//        }while(this.skip=true);
        return map;
    }


    /**
     * Read a Block from the sdf file and return
     * @return a Map[] where [0] holds the source and [1] the target language data.
     *
     * @throws java.io.IOException
     */
    public Map[] readBlock() throws java.io.IOException {
        String line = EMPTY;
        String blockId = EMPTY;
        String[] splittedLine = null;
        data[0]=new ExtMap();
        data[1]=new ExtMap();
        String help;
        String c = null;
        //read next line or use buffered line
        while (useBuffer || (line = readLine()) != null) { //works because '||' is shortcut

            try {
                //              buffer used?
                if (useBuffer) {
                    line = this.lineBuffer;
                    splittedLine = this.splittedLineBuffer;
                    this.SDFMap = new ExtMap(SDFReader.fieldnames, splittedLine);

                    try {
                        checkLanguage(splittedLine);
                    } catch (ConverterException e) {
                        throw e;
                    }finally{

                        useBuffer = false;
                    }
                } else {
                    //...are there wrong characters?
                    if ((check(line)).length() < line.length()) {
                        throw new LineErrorException(getLineNumber()
                                + " : Line contains wrong character "
                                //+ Integer.toHexString(Integer.parseInt(c))
                                + " \n" + line);
                    }
                    //...is the number of columns ok?
                    if ((splittedLine = split(line)) == null) {

                        throw new LineErrorException(super.getLineNumber()
                                + " : Line has wrong column number \n" + line);
                        //continue;
                    } else {
                        // TODO makeDot is better in Data Handler
                        makeDot();
                        // ...is this line in a new SDF block ?
                        if ((blockId = getSDFBlockId(splittedLine))
                                .equals(CurrentBlockId)) {

                            this.SDFMap = new ExtMap(SDFReader.fieldnames,
                                    splittedLine);
                            //what language is in it ?
                            checkLanguage(splittedLine);

                        } else {

                            /*
                             * we found the next block , but do we have the
                             * target text?
                             */

                            if (!foundTarget) {
                                createTargetLine();
                            }

                            blockNr++;
                            splittedLineBuffer = splittedLine;//read one line
                            // too much so
                            // buffer it
                            lineBuffer = line;
                            useBuffer = true;//reset();

                            this.CurrentBlockId = blockId;
                            this.CurrentBlockHash=this.CurrentBlockId.hashCode();

                            /* ...and what about the source text ? */
                            if (!foundSource) {

                                OutputHandler
                                        .log("Error in Line:"
                                                + getLineNumber()
                                                + "Source Language is missing maybe "
                                                + "previous block has an error.\nBlock "
                                                + (blockNr - 1)
                                                + " is skipped. before line: \n"
                                                + line);

                                foundTarget = false;//no target without source
                                skippedBlocks++;
                                skippedLines++;
                                sourceLangMiss++;
                                continue;// skip output of this block if no
                                         // source language is found

                            }

                            break;

                        }
                    }

                }

            } catch (LineErrorException e) {

                OutputHandler.log(e.getMessage());
                this.skippedLines++;
            } catch (ConverterException e) {
                OutputHandler.log(e.getMessage());
            }

        }
        // did we read the whole stuff?
        if (null != line) {
            // no
            foundSource = false;
            foundTarget = false;

            return this.data;

        } else {
            // ok , its the end but is everything written now?
            if (!endIt) {
                // there is something to write
                // but next time we can end it
                endIt = true;
                if(!foundTarget){
                    createTargetLine();
                }
                // write
                return this.data;//last lines
            } else {

                showStat();
                return null;
            }
        }
        //        }catch(ConverterException e) {
        //            Converter.log(e.getMessage());
        //            return null;
        //        }
    }

    /**
     *
     */
    private void createTargetLine() {
        targetLangMiss++;
        // if not, create one ...
        data[1] = new ExtMap(SDFReader.targetLineNames,
                splittedLineBuffer);
        data[1].put("TargetLanguageID",
                this.targetLanguage);
        if ((String) data[1].get("TargetText") != EMPTY)
            data[1].put("TargetText", EMPTY);
        if ((String) data[1].get("TargetHText") != EMPTY)
            data[1].put("TargetHText", EMPTY);
        if ((String) data[1].get("TargetQText") != EMPTY)
            data[1].put("TargetQText", EMPTY);
        if ((String) data[1].get("TargetTitle") != EMPTY)
            data[1].put("TargetTitle", EMPTY);
        this.data[1].put("BlockNr", blockNrFormatter.format(blockNr)+'-'+Integer.toString(this.CurrentBlockHash));
        targetLangCreate++;
    }

    /**
     * Show the statistic information got while
     * reading the file
     *
     * @throws IOException
     */
    private void showStat() throws IOException {
        OutputHandler.out(EMPTY);OutputHandler.out(EMPTY);
 //       OutputHandler.out("Hashes:              " + (theHashes.size()) + " ");
        OutputHandler.out("Blocks found:           " + blockNr + " ");
        OutputHandler.out(EMPTY);
        OutputHandler.out("Lines read:             " + (getLineNumber()) + " ");
        OutputHandler
                .dbg("Lines created           " + (targetLangCreate) + " ");
        OutputHandler.dbg("                        -------");
        OutputHandler.dbg("Lines total:            "
                + (getLineNumber() + targetLangCreate) + " ");
        OutputHandler.dbg("Lines skipped:          " + skippedLines + " ");

        OutputHandler.dbg("Source Language misses: " + sourceLangMiss + " ");
        OutputHandler.dbg("Target Language misses: " + targetLangMiss + " ");
        OutputHandler.dbg("Blocks found:           " + blockNr + " ");
        OutputHandler.dbg("Blocks skipped:         " + skippedBlocks + " ");
        if ((sourceLangMiss + skippedBlocks + skippedLines) > 0)
            OutputHandler.out("\n---! Errors found !--- view Logfile.\n\n"
                    + "To enable logfile use -l option at command line.\n"
                    + "For help type 'convert -h {Enter}'.\n");
    }

    /**
     * Check the current line  whether the source language
     * or target language is in it
     *
     * @throws ConverterException if a not needed language or no target language is found
     *              in this block
     * @throws IOException
     *
     */
    final private void checkLanguage(String[] splittedLine)
            throws ConverterException, IOException {
        String langID = (String) SDFMap.get("LanguageID");

        //maybe the source language is in this line
        if (!foundSource && this.sourceLanguage.equals(langID)) {
//          found the next source language line
            this.data[0] = new ExtMap(SDFReader.sourceLineNames, splittedLine);

//            this.data[0].put("BlockNr", Integer.toHexString(blockNr));
//            this.data[0].put("BlockHash", Integer.toHexString(this.CurrentBlockHash));
            this.data[0].put("BlockNr", blockNrFormatter.format(blockNr)+'-'+Integer.toHexString(this.CurrentBlockHash));
//            this.data[0].put("BlockHash", blockHashFormatter.format(this.CurrentBlockHash));
            foundSource = true;
            return;

        } else {
            // or the target language is in this line
            if (!foundTarget) {
                //no target language is given at command line
                if (this.targetLanguage.equals(EMPTY)) {
                    //try if we can use the current lines language for target
                    // language
                    if (!langID.equals(this.sourceLanguage)) {
                        //yes , we can use this lines laanguage as target
                        this.targetLanguage = langID;

                        //source and target language both are known: show it
                        OutputHandler.out("Source Language is: "
                                + this.sourceLanguage + " ");
                        OutputHandler.out("Target Language is: "
                                + this.targetLanguage + " ");
                        OutputHandler.out(EMPTY);
                        System.out.println("Start");

                    } else {
                        throw new ConverterException("(" + getLineNumber()
                                + ") No target language found: "
                                + this.targetLanguage);
                    }
                }
                if (this.targetLanguage.equals(langID)) {
                    this.data[1] = new ExtMap(SDFReader.targetLineNames,
                            splittedLine);// found the next target language line
                    this.data[1].put("BlockNr", blockNrFormatter.format(blockNr)+'-'+Integer.toHexString(this.CurrentBlockHash));
                    foundTarget = true;

                    return;
                }
            }//end !foundTarget
        }
        //if we go here we dont need the found language...
        throw new ConverterException("found not needed language '"
                + this.SDFMap.get("LanguageID") + "' in Line: "
                + getLineNumber());

    }

    /**
     *  Make a dot on the screen to show the user that it is going on
     */
    private void makeDot() {
        int count = 0;
        if ((count = (int) super.getLineNumber() / 1000) > this.dotCount) {
            this.dotCount = count;
            OutputHandler.printDot();
        }

    }

    /**
     * split the SDFLine in its columns
     *
     * @param   line the current SDFLine
     * @return  the splitted SDFLine as array of String
     *          or null if an error occours
     * @throws  IOException
     */
    private String[] split(String line) throws IOException {
        check(line);
        String[] splitLine;
        if ((splitLine = line.split("\t")).length == 15)
            return splitLine;
        else
            //an error occurred
            return null;
    }

    /**
     * create a block Id from a splitted SDFLine
     * the blockId consists of the column one to eight of an SDF File
     *
     * @param splitLine    the line to create a block id from
     * @return the blockId as String
     */
    private String getSDFBlockId(String[] splitLine) {
        StringBuffer BlockId = new StringBuffer("");
        for (int i = 0; i < 8; i++) {
            BlockId.append(splitLine[i]);
        }
        return BlockId.toString();
    }

//    public final boolean canRead() {
//        return this.sourceFile.canRead();
//    }

    /**
     * Check if there are not allowed characters in this line
     *
     * @param line      the SDFLine to check
     * @return          if everything, ok the original
     *                   else the wrong character as String
     *
     * @throws java.io.IOException
     */
    private String check(String line) throws java.io.IOException {
        char c = ' ';
        for (int i = 0; i < line.length(); i++) {
            c = line.charAt(i);
            if (c < 30 && c != 9) {
                return (new Character(c)).toString();
            }

        }
        return line;

    }
}