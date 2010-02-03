/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SDFWriter.java,v $
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
 * SDFWriter.java
 *
 *
 */

package com.sun.star.tooling.converter;
import java.io.*;
import java.util.Map;

/**
 * Write data to a SDFFile
 *
 * @author Christian Schmidt 2005
 *
 */
public class SDFWriter  extends DataWriter {
    /**
     * the seperator the seperate columns
     */
    final String seperator=new String("\t");
    /**
     * an array of the SDF files column names if the source language is in
     */
    final static String[] sourceLineNames= {"Project","SourceFile","Dummy","ResType","GID","LID","HID","Platform","Width","SourceLanguageID","SourceText","SourceHText","SourceQText","SourceTitle","TimeStamp"};
    /**
     * an array of the SDF files column names if the target language is in
     */
    final static String[] targetLineNames= {"Project","SourceFile","Dummy","ResType","GID","LID","HID","Platform","Width","TargetLanguageID","TargetText","TargetHText","TargetQText","TargetTitle","TimeStamp"};
    /**
     * an array of the SDF files column names if the source and the target language is in
     */
    final static String[]    outLineNames= {"BlockNr","Project","SourceFile","Dummy","ResType","GID","LID","HID","Platform","Width","SourceLanguageID","SourceText","SourceHText","SourceQText","SourceTitle","TargetLanguageID","TargetText","TargetHText","TargetQText","TargetTitle","TimeStamp"};

    /**
     * A Map holding the source language line content
     */
    private ExtMap    sourceLine=new ExtMap(sourceLineNames,null);
    /**
     * A Map holding the target language line content
     */
    private ExtMap    targetLine=new ExtMap(targetLineNames,null);
    /**
     * A Map holding the whole content for output
     */
    private ExtMap outData=new ExtMap(outLineNames, null);
//    private ExtMap SDFLine;
//    private InputStreamReader isr;
    /**
     * The language to translate from
     */
    private String sourceLanguage;
    /**
     * The language to translate to
     */
    private String targetLanguage;

//    private boolean SourceIsFirst=false;




    /**
     * Create a new Instance of SDFWriter
     *
     * @param bos BufferedWriter to write to
     * @param charset the charset to use to write
     * @throws java.io.UnsupportedEncodingException
     */
    public SDFWriter(BufferedOutputStream bos,String charset) throws java.io.UnsupportedEncodingException {
        super(bos,charset);

    }

    /* (non-Javadoc)
     * @see com.sun.star.tooling.converter.DataWriter#writeData()
     */
    public final void writeData() throws java.io.IOException {

        StringBuffer buffer=new StringBuffer("");

        // get the values of the found fields
        //create the two sdf lines

        //at first the source language line
        for(int i=0;i<sourceLineNames.length;i++){
            // put them together for output
            buffer.append(outData.get(sourceLineNames[i]));
            if(i!=sourceLineNames.length-1) {
                // seperate the fields with tab
                buffer.append(seperator);
            }else{
                // this line is full
                // so close it with lf
                buffer.append(lineEnd);
                Converter.countLine();
            }
        }
        // is there a target line with anything in the strings?
        if (!(outData.get("TargetLanguageID")==null||((outData.get("TargetTitle").equals("")&&outData.get("TargetText").equals("")&&outData.get("TargetHText").equals("")&&outData.get("TargetQText").equals(""))))){
            //now the target language line
            for(int i=0;i<targetLineNames.length;i++){
                // put them together for output
                buffer.append(outData.get(targetLineNames[i]));
                if(i!=targetLineNames.length-1) {
                    // seperate the fields with tab
                    buffer.append(seperator);
                }else{
                    // this line is full
                    //so close it with lf
                    buffer.append(lineEnd);
                    Converter.countLine();
                }
            }
        }
        this.write(buffer.toString());

    }



    /* (non-Javadoc)
     * @see com.sun.star.tooling.converter.DataWriter#writeData(java.util.Map[])
     */
    protected void writeData(Map[] data) throws IOException {
        // TODO redesign DataHandler in the way that this is not nessesary any more

    }

    /* (non-Javadoc)
     * @see com.sun.star.tooling.converter.DataWriter#getDataFrom(com.sun.star.tooling.converter.DataHandler)
     */
    protected void getDataFrom(DataHandler handler) throws IOException {

        handler.putDataTo(this.outData);
    }

    /* (non-Javadoc)
     * @see com.sun.star.tooling.converter.DataWriter#getDatafrom(com.sun.star.tooling.converter.DataHandler)
     */
    protected void getDatafrom(DataHandler handler) throws IOException {

        handler.putDataTo(this.outData);

    }
}
