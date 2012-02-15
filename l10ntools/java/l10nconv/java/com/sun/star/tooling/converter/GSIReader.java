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
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;

import com.sun.star.tooling.languageResolver.LanguageResolver;
import com.sun.star.tooling.languageResolver.LanguageResolver.LanguageResolvingException;

/**
 * @author Christian Schmidt 2005
 *
 */
public class GSIReader extends DataReader {
    /**
     * A Map holding an empty GSIBlock
     */
    private Map                 EmptyGSIBlock;

    /**
     * @see LanguageResolver
     */
    LanguageResolver            languageResolver;

    /**
     * The source language identifier
     */
    protected String            sourceLanguage;

    /**
     * The target language identifier
     */
    protected String            targetLanguage;

    /**
     * The number of the last block
     */
    private String              oldBlockNr;

    /**
     * A buffer holding one GSILine
     */
    private Map                 GSILineBuffer;

    /**
     * Indicates whether to use the buffered line
     */
    private boolean             useBuffer        = false;

    private static final String EMPTY            = new String("");

    /**
     * An empty Map to fill with language depending data
     */
    private ExtMap              EmptyLanguageMap;

    /**
     * Indicates whether the first block is read
     */
    private boolean             isFirst          = true;

    private int                 lineCounter;

    private int                 blockCounter;
    /**
     * Indicates whether the last line is read
     */
    private boolean             lastLineFound = false;

    /**
     * Create a new Instance of GSIReader
     *
     * @param source
     *            the file to read from
     * @param sourceLanguage
     *            the sourceLanguage (must not be empty)
     * @param targetLanguage
     *            the targetLanguage
     * @param charset
     *            the charset used to read source
     * @throws java.io.IOException
     * @throws Exception
     */
    public GSIReader(File source, String sourceLanguage, String targetLanguage,
            String charset) throws java.io.IOException {
        super(new InputStreamReader(new FileInputStream(source), charset));
    this.languageResolver = new LanguageResolver();
        this.sourceLanguage = sourceLanguage;
        this.targetLanguage = targetLanguage;

        EmptyLanguageMap = new ExtMap(new String[0], new String[0]);

    }

    /**
     * Read the next GSIBlock and return the data
     *
     * @return A Map containing the data of the read GSIBlock the keys for the language depending data are the language id (numeric) the
     * single language  are acessible with the keys "BlockNr", "resType", "languageNr", "status","content".
     *
     * @throws IOException
     */
    public Map getGSIData() throws IOException {
        String help;
        Map helpmap;
        Map GSIBlock = new HashMap();
        GSIBlock.put(sourceLanguage, EmptyLanguageMap.clone());
        GSIBlock.put(targetLanguage, EmptyLanguageMap.clone());
        String line = EMPTY;
        String[] splitLine;
        Map GSILine;
        String[] GSINames = { "BlockNr", "resType", "languageNr", "status",
                "content" };

        while (useBuffer || (line = readLine()) != null) {



            if (useBuffer) {
                GSILine = GSILineBuffer;
                GSIBlock.put(sourceLanguage, EmptyLanguageMap.clone());
                GSIBlock.put(targetLanguage, EmptyLanguageMap.clone());
                GSIBlock.put("BlockNr", GSILine.get("BlockNr"));
                useBuffer = false;
            } else {
                this.lineCounter++;
                if ((splitLine = split(line)) == null) {
                    continue;
                }
                GSILine = new ExtMap(GSINames, splitLine);
                if (isFirst) {
                    GSIBlock.put("BlockNr", GSILine.get("BlockNr"));
                    oldBlockNr = (String) GSILine.get("BlockNr");
                    isFirst = false;
                }
            }
            if (oldBlockNr == null) {
                oldBlockNr = (String) GSILine.get("BlockNr");
            }
            if (!oldBlockNr.equals((String) GSILine.get("BlockNr"))) {
                GSILineBuffer = GSILine;
                oldBlockNr = (String) GSILine.get("BlockNr");
                useBuffer = true;
                break;
            }
            String lang;
            try {
                // Is there the source language in this line?
                if ((lang = languageResolver.getISOfromNr((String) GSILine
                        .get("languageNr"))).equals(this.sourceLanguage)) {
                    // ok..store it as Source String under the depending
                    // ressource type

                    ((Map) GSIBlock.get(sourceLanguage)).put("Source"
                            + ResTypeResolver.getInternKey((String) GSILine
                                    .get("resType")), GSILine.get("content"));
                    // ..maybe the target language?
                } else {
                    if (targetLanguage.equals(EMPTY)) {
                        // if no target language is given at command line
                        targetLanguage = lang;
                        GSIBlock.put(targetLanguage, EmptyLanguageMap.clone());
                    }
                    if (lang.equals(this.targetLanguage)) {

                        // ok..store it as target String under the depending
                        // ressource type
                        ((Map) GSIBlock.get(targetLanguage)).put("Target"
                                + ResTypeResolver.getInternKey((String) GSILine
                                        .get("resType")), GSILine
                                .get("content"));
                    }
                }
            } catch (LanguageResolvingException e) {
                OutputHandler.out("Can not resolve the language "+e.getMessage());
            }

        }
        if (line == null) {
            if (lastLineFound){
                OutputHandler.out("\n\n");
                OutputHandler.out("GSI Blocks    :         " + this.blockCounter);
                OutputHandler.out("GSI Lines     :         " + this.lineCounter);

                return null;
            }else{
                lastLineFound = true;
                this.blockCounter++;
                return GSIBlock;
            }
        } else {
            this.blockCounter++;
            return GSIBlock;
        }

    }

    /**
     * Split a GSILine to single fields
     *
     * @param line
     *            The line to split
     * @return An array containing the contents of the columns in the given line
     */
    private String[] split(String line) {
        String[] splitLine = (line.substring(0, line.length() - 1))
                .split("\\(\\$\\$\\)");
        if (splitLine.length != 5)
            return null;
        else
            return splitLine;
    }

}
