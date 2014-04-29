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
//                    // we can't match this gsi block to the current sdf block
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
