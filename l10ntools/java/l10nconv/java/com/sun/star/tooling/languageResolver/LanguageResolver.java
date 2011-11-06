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
package com.sun.star.tooling.languageResolver;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.ListIterator;

/**
 * Translate language codes into another format
 * between ISO, RFC3066 and numeric
 *
 * @author Christian Schmidt 2005
 *
 */
public class LanguageResolver {
    private final static int ISO        =2;
    private final static int LANGID     =0;
    private final static int LANGNAME   =1;
    private final static int RFC3066    =3;

    ArrayList languages=new ArrayList();

//    public static void main(String[] args){
//        try {
//            LanguageResolver lr=new LanguageResolver();
//        } catch (IOException e) {
//            //
//            e.printStackTrace();
//        }
//    }

    /**
     * Create a new Instance of LanguageResolver
     *
     * @throws IOException
     */
    public LanguageResolver() throws IOException{
        String lang = "com/sun/star/tooling/languageResolver/lang.map";
        ClassLoader cl = this.getClass().getClassLoader();
        InputStream in = cl.getResourceAsStream(lang);
        BufferedReader languageTable= new BufferedReader(new InputStreamReader(in));

        String line;

        while((line=(languageTable.readLine()))!=null){
            languages.add(line.split(","));
        }
    }
    /**
     * Get the numeric value of the given ISO Language Code
     *
     * @param isoCode the ISO Language Code to find
     * @return numeric value of the given isoCode
     * @throws LanguageResolvingException if the Language ISO Code is not known
     */
    public String getNrFromISO(String isoCode) throws LanguageResolvingException{
        if("".equals(isoCode)) return "";
            ListIterator iter=languages.listIterator();
            String[] line=new String[5];
            while(isoCode!="" && iter.hasNext()){
                line=(String[]) iter.next();
                if(line[ISO].equals(isoCode)) return line[LANGID];
            }
            throw new LanguageResolvingException("Can not find ISO Code: "+isoCode );

    }

    /**
     * Get the ISO Language Code corresponding with the given Language ID
     *
     * @param ID the numeric language id to find
     * @return the ISO Language Code corresponding with the given Language ID
     * @throws LanguageResolvingException if the Language ID is not known
     */
    public String getISOfromNr(String ID) throws LanguageResolvingException{
        if("".equals(ID)) return "";
        ListIterator iter=languages.listIterator();
        String[] line=new String[5];
        while(iter.hasNext()){
            line=(String[]) iter.next();
            if(line[LANGID].equals(ID)) return line[ISO];
        }
        throw new LanguageResolvingException("Can not find Language Id: "+ID );
    }

    /**
     * Get the RFC3066 value of the given ISO Language Code
     *
     * @param isoCode the ISO Language Code to find
     * @return RFC3066 value of the given isoCode
     * @throws LanguageResolvingException if the Language ISO Code is not known
     */
    public String getRFCFromISO(String isoCode) throws LanguageResolvingException{
        if("".equals(isoCode)) return "";
        ListIterator iter=languages.listIterator();
        String[] line=new String[5];
        while(iter.hasNext()){
            line=(String[]) iter.next();
            if(line[ISO].equals(isoCode)) return line[RFC3066];
        }
        throw new LanguageResolvingException("Can not find ISO Code: "+isoCode );
    }

    /**
     * Get the ISO Language Code corresponding with the given RFC3066 code
     *
     * @param RFC RFC3066 language id to find
     * @return the ISO Language Code corresponding with the given RFC3066 code
     * @throws LanguageResolvingException if the RFC3066 code is not known
     */
    public String getISOFromRFC(String RFC) throws LanguageResolvingException{
        if("".equals(RFC)) return "";
        ListIterator iter=languages.listIterator();
        String[] line=new String[5];
        while(iter.hasNext()){
            line=(String[]) iter.next();
            if(line[RFC3066].equals(RFC)) return line[ISO];
        }
        throw new LanguageResolvingException("Can not find Language Id: "+RFC );
    }


    /**
     * This Exception is thrown if a Language Identfier is unknown
     *
     * @author Christian Schmidt 2005
     *
     */
    public class LanguageResolvingException extends Exception {

        /**
         *
         */
        public LanguageResolvingException() {
            super();
            //
        }

        /**
         * @param arg0
         */
        public LanguageResolvingException(String arg0) {
            super(arg0);
            //
        }

        /**
         * @param arg0
         * @param arg1
         */
        public LanguageResolvingException(String arg0, Throwable arg1) {
            super(arg0, arg1);
            //
        }

        /**
         * @param arg0
         */
        public LanguageResolvingException(Throwable arg0) {
            super(arg0);
            //
        }

    }

}
