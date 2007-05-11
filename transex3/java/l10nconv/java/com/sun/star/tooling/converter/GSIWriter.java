/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GSIWriter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 09:09:23 $
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

package com.sun.star.tooling.converter;
import java.io.*;
import java.util.Map;

import com.sun.star.tooling.languageResolver.LanguageResolver;
import com.sun.star.tooling.languageResolver.LanguageResolver.LanguageResolvingException;
/**
 *
 * @author cs156069
 */
public class GSIWriter  extends DataWriter {
    /**
     * The seperator used to seperate GSI columns
     */
    final String seperator=new String("($$)");

    /**
     * Holding the keys used by a map holding the  content of an GSI Line contianing the source language
     */
    final static String[] sourceLineNames= {"Project","SourceFile","Dummy","ResType","GID","LID","HID","Platform","Width","SourceLanguageID","SourceText","SourceHText","SourceQText","SourceTitle","TimeStamp"};
    /**
     * Holding the keys used by a map holding the  content of an GSI Line contianing the target language
     */
    final static String[] targetLineNames= {"Project","SourceFile","Dummy","ResType","GID","LID","HID","Platform","Width","TargetLanguageID","TargetText","TargetHText","TargetQText","TargetTitle","TimeStamp"};
    /**
     * Holding the keys used by a map holding the  content of an GSI Line contianing the source and the target language
     */
    final static String[]    outLineNames= {"BlockNr","Project","SourceFile","Dummy","ResType","GID","LID","HID","Platform","Width","SourceLanguageID","SourceText","SourceHText","SourceQText","SourceTitle","TargetLanguageID","TargetText","TargetHText","TargetQText","TargetTitle","TimeStamp"};

    /**
     * A map holding the  content of an GSI Line contianing the source language
     */
    private ExtMap    sourceLine=new ExtMap(sourceLineNames,null);
    /**
     * A map holding the  content of an GSI Line contianing the target language
     */
    private ExtMap      targetLine=new ExtMap(targetLineNames,null);
    /**
     * A map holding the  content of an GSI Line contianing the source and the target language
     */
    private ExtMap outData=new ExtMap(outLineNames, null);

    private static final String EMPTY          = new String("");
    /**
     * The sourceLanguage to use
     */
    private String sourceLanguage;
    /**
     * The sourceLanguage to use
     */
    private String targetLanguage;

    /**
     * GSILines have a special Line End
     */
    private final static String lineEnd="!"+'\r'+'\n';
    //private boolean SourceIsFirst=false;

    /**
     * The blockNr of the current line
     */
    private String blockNr;




    /**
     * Create a new Instance of GSIWriter
     *
     * @param bos       the Buffered Output Stream to write to
     * @param charset   the used charset
     * @throws java.io.UnsupportedEncodingException
     */
    public GSIWriter(BufferedOutputStream bos,String charset) throws java.io.UnsupportedEncodingException {
        super(bos,charset);



    }

    /* (non-Javadoc)
     * @see com.sun.star.tooling.converter.DataWriter#writeData()
     */
    public final void writeData() throws java.io.IOException {

        StringBuffer buffer=new StringBuffer("");
        if(this.sourceLanguage==null&&this.targetLanguage==null){
            LanguageResolver lang =new LanguageResolver();
            try {
                this.sourceLanguage=lang.getNrFromISO((String)outData.get("SourceLanguageID"));
                this.targetLanguage=lang.getNrFromISO((String)outData.get("TargetLanguageID"));
            } catch (LanguageResolvingException e) {

                OutputHandler.out(e.getMessage());
                System.exit(0);
            }
        }


        this.blockNr=(String)outData.get("BlockNr");
        // get the values of the found fields
        //create the gsi lines
        //
        //at first the source language line
        buffer.append(getSourceLine("Text"));
        buffer.append(getSourceLine("HText"));
        buffer.append(getSourceLine("QText"));
        buffer.append(getSourceLine("Title"));

        //now the target language line
        // put them together for output
        buffer.append(getTargetLine("Text"));
        buffer.append(getTargetLine("HText"));
        buffer.append(getTargetLine("QText"));
        buffer.append(getTargetLine("Title"));
        //ok...put all to disk;
        this.write(buffer.toString());

    }

    /**
     * Create a line containing the source string from the data
     * @param resType
     * @return The StringBuffer containing the line
     */
    private StringBuffer getSourceLine(String resType){
        StringBuffer buffer =new StringBuffer(200);
        String resString = "Source"+resType;

        String help;
        if(EMPTY.equals((String)outData.get(resString))||" ".equals(outData.get(resString))) return new StringBuffer(EMPTY);
        else {
            // put them together for output
            buffer.append(this.blockNr);
            // seperate the fields with ($$)
            buffer.append(this.seperator);
            buffer.append(ResTypeResolver.getExternKey(resType));
            // seperate the fields with ($$)
            buffer.append(this.seperator);
            buffer.append(this.sourceLanguage);
            // seperate the fields with ($$)
            buffer.append(this.seperator);
            buffer.append("int");
            // seperate the fields with ($$)
            buffer.append(this.seperator);
            buffer.append(outData.get(resString));

            // this line is full
            // so close it with '! cr lf'
            buffer.append(GSIWriter.lineEnd);
            Converter.countLine();
            return buffer;
        }

    }
    /**
     * Create a line containing the target string from the data
     * @param resType
     * @return The StringBuffer containing the line
     */
    private StringBuffer getTargetLine(String resType){
        StringBuffer buffer =new StringBuffer(200);
        String resString = "Target"+resType;

        if(EMPTY.equals((String)outData.get(resString))||" ".equals(outData.get(resString))) return new StringBuffer(EMPTY);
        else {
            // put them together for output
            buffer.append(this.blockNr);
            // seperate the fields with ($$)
            buffer.append(this.seperator);
            buffer.append(ResTypeResolver.getExternKey(resType));
            // seperate the fields with ($$)
            buffer.append(this.seperator);
            buffer.append(this.targetLanguage);
            // seperate the fields with ($$)
            buffer.append(this.seperator);
            buffer.append("ext");
            // seperate the fields with ($$)
            buffer.append(this.seperator);
            buffer.append(outData.get(resString));

            // this line is full
            // so close it with '! cr lf'
            buffer.append(GSIWriter.lineEnd);
            Converter.countLine();
            return buffer;
        }

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
