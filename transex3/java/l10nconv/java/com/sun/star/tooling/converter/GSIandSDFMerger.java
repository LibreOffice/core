/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GSIandSDFMerger.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 09:09:33 $
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
 * Created on 2005
 *  by Christian Schmidt
 */
package com.sun.star.tooling.converter;

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

/**
 * Merge GSIFiles back to to the original (!) SDFFile
 *
 *
 * @author Christian Schmidt 2005
 *
 */
public class GSIandSDFMerger extends SDFReader {

    int lineCounter=0;

    GSIReader gsiReader;

    private Map temp=new HashMap();
    private int j;
    private boolean skip=true;
    Map gsiBlock=null;
    Map sdfBlock=null;

    private boolean dontLoadGSI=false;

    private int count;
    /**
     * Merge the GSIFile back to the original(!) SDFFile
     *
     * @param source the file to read from
     * @param sourceLanguage the source language in the source file
     * @param targetLanguage the target language in the source file
     * @param charset   the charset of the files
     * @throws java.io.IOException
     * @throws Exception
     */
    public GSIandSDFMerger(File source, File secondSource,String sourceLanguage,
            String targetLanguage, String charset) throws IOException {
        // merging GSI and SDF requieres two Sources
        //this. is the SDF source
        super(secondSource, sourceLanguage, targetLanguage, charset);
        //create the GSI Source

        gsiReader=new GSIReader(source,sourceLanguage,targetLanguage,charset);

    }

    /* (non-Javadoc)
     * @see com.sun.star.tooling.converter.DataReader#getData()
     */
    public Map getData()throws java.io.IOException{
        do{
            skip=false;
            this.temp=matchGSI();
        }while(skip);
        if(temp==null){
            OutputHandler.out("Blocks merged :         "+this.lineCounter);
        }
        return temp;

    }

    /**
     * Read each block of the GSIFile and check whether there is a matching
     * block in the SDFFile. Match depends on the BlockNr and BlockId.
     *
     * @return A Map that contains the source language content
     *          and  the target language content.
     * @throws IOException
     * @throws ConverterException
     */
    public Map matchGSI() throws IOException{


        try {
            //System.out.println("Start...");

            if (dontLoadGSI||(gsiBlock=gsiReader.getGSIData())!=null){
                dontLoadGSI=false;
                //check if we must update this block
                //if so its BlockNr is in the gsi file
                if((sdfBlock = super.getData())!=null){

                    if(((String)sdfBlock.get("BlockNr")).equals((String)gsiBlock.get("BlockNr"))){

                        gsiBlock.remove(EMPTY);
                         //if the target language string is empty this may be caused by an error in the source sdf File
                         //I don't want to overwrite a possibly correct translation with an empty string
                         // so remove the target part from the gsiBlock
                        Map mp=(Map)gsiBlock.get(gsiReader.targetLanguage);
                        if (mp.size()!=0&&!((String)mp.get("TargetText")).equals("")){

                             // target language part in this gsiBlock
//                             if(((String)mp.get("TargetText")).equals("")){
//                                 gsiBlock.remove(targetLanguage);
//                             }
                            // count the merged blocks
                             lineCounter++;
                             Map helpMap = (Map)gsiBlock.get(super.targetLanguage);//"ja"
                             sdfBlock.putAll(helpMap);
                             skip=false;
                        }else{
                            //no target language part in this gsiBlock
                             skip=true;
//
//                             return null;
                         }
                    }else{
//                        skip=true;
//
//                    // we cant match this gsi block to the current sdf block
                      // try matching the next sdf block with this gsi line
                       dontLoadGSI=true;
                    }
                }
                return sdfBlock;
            }

        } catch (IOException e) {

            e.printStackTrace();
        }
        return null;
    }






}
