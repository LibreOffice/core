/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DirtyTagWrapper.java,v $
 * $Revision: 1.3 $
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
 * Created on 2005
 *  by Christian Schmidt
 */
package com.sun.star.tooling.DirtyTags;

import java.io.IOException;
import java.util.ArrayList;

/**
 * Solves the problem with translating strings from the
 * Star-Office Help. This Strings contain XML snippets
 * (that means parts of an xml dokument). I call them 'dirty'
 * because the start of a tag (<) and the and of an tag (>)
 * are quoted by a single backslash(\<.....\>). This is done
 * because the text out of th tags should not contain '<' and '>'
 * as Entity references (&lt; or &gt;) but as readable signs.
 * This is for translation purposes.
 * Because translators get mad while find out the really translatable
 * parts between all the markup information, the XLIFF Specification
 * allows to wrap parts of a String that should not be translated by
 * special tags (<ept>, <bpt>).
 * This Class has two static methods that do the wrapping and unwrapping
 * NOTE: this won't work with not 'dirty' Strings.
 *
 * @author Christian Schmidt 2005
 *
 */
public class DirtyTagWrapper {

    private static boolean doWrap=true;
    public static void setWrapping(boolean doWrap){
        DirtyTagWrapper.doWrap=doWrap;
    }

    /**
     * Unwraps the 'dirty' parts of a String from ept and bpt tags
     *
     * @param checkString The String to unwrap
     * @return the unwrapped String
     */
    public static String unwrapString(String checkString){
        //remove the ept and bpt tags
        String[] splitted =checkString.split("(<ept ([^<>])*>)|(</ept>)|(<bpt ([^<>])*>)|(</bpt>)|(<sub([^<>])*>)|(</sub>)|(<ex ([^<>])*/>)");
        StringBuffer workBuffer= new StringBuffer();
        for(int i=0;i<splitted.length;i++){
            workBuffer.append(splitted[i]);
        }
        String string = new String(workBuffer);
        //replace Entity references
        string=string.replaceAll( "&amp;","&").replaceAll( "&lt;","<").replaceAll( "&gt;",">").replaceAll( "&quot;","\"").replaceAll( "&apos;","'");

        //remove the nsub tags
        splitted =string.split("(<sub([^<>])*>)|(</sub>)");
        StringBuffer returnBuffer= new StringBuffer();
        for(int i=0;i<splitted.length;i++){
            returnBuffer.append(splitted[i]);
        }
        String returnString = new String(returnBuffer);
        return returnString;
    }


    /**
     * Wrap the dirty parts of a string
     *
     * @param checkString The String to check if there are dirty Parts to wrap
     * @return A String with wrapped dirty parts
     * @throws TagWrapperException
     * @throws IOException
     */
    public static String wrapString(String checkString) throws TagWrapperException, IOException {
        // if no wrapping should be done return the given string
        if(!doWrap) return checkString;
        // let's wrap
        String[] parts=null;
        int idx=0;
        //split the string at tag ends
        String[] parts2 = checkString.split("\\\\>");

        ArrayList tagString =new ArrayList();
        // put the while splitting lost parts to the end of the single strings
        for(int j=0;j<parts2.length-1;j++){
            parts2[j]+="\\>";
        }
        // same for the last string
        if (checkString.endsWith("\\>")){
            parts2[parts2.length-1]+="\\>";
        }
        // split the leading text from the real tag string (<...>)
        for(int j=0;j<parts2.length;j++){

            //is it just a tag
            if(parts2[j].startsWith("\\<")){
                tagString.add(parts2[j]);
             // or is it a tag with leading text?
            }else if((idx=parts2[j].indexOf("\\<"))>0&&parts2[j].indexOf("\\>")>0){
                //...then split it in two parts
                // the leading text
                tagString.add(parts2[j].substring(0,(parts2[j].indexOf("\\<"))));
                // ...and the tag
                tagString.add(parts2[j].substring(parts2[j].indexOf("\\<")));

            }else{
                //no tag...must be text only
                tagString.add(parts2[j]);
            }

        }
        ArrayList tagNames=new ArrayList();
        String item="";
        for(int i=0;i<tagString.size();i++){
            item=((String)tagString.get(i));
            int start=item.indexOf("\\<")+2;
            // check if we have an index that is ok
            if(start==1) start=-1;
            int end=item.lastIndexOf("\\>");
            if(start>=0&&end>0){
                boolean isStandalone=false;
                if(item.endsWith("/\\>")){
                    // this is a standalone tag
                    isStandalone=true;
                }
                item=item.substring(start,end);

                if(item.indexOf(" ")>0){
                   item=item.substring(0,item.indexOf(" "));
                }
                if(isStandalone){
                    item=item+"/";
                }
                tagNames.add(item);
            }else{
                tagNames.add("");
            }
        }
        ArrayList tagType=new ArrayList();
        for(int i=0;i<tagNames.size();i++){
            if(((String)tagNames.get(i)).equals("")){
                tagType.add("Text");
            }else if(((String)tagNames.get(i)).startsWith("/")){
                tagType.add("EndTag");
            }else if(((String)tagNames.get(i)).endsWith("/")){
                tagType.add("StartAndEndTag");
            }else {
                tagType.add("StartTag");
            }

        }

        ArrayList tagList=new ArrayList();
        for(int i=0;i<tagNames.size();i++){
            tagList.add(new Tag(
                    (String)tagType.get(i),
                    (String)tagNames.get(i),
                    (String)tagString.get(i)));
        }
        tagType=null;
        tagNames=null;
        tagString=null;

        TagPair start;
        StringBuffer returnBuffer=new StringBuffer();
        while(tagList.size()>0){
            try{
                start=new TagPair(tagList);
                returnBuffer.append(start.getWrapped());
            }catch(TagPair.TagPairConstructionException e){
                throw (new DirtyTagWrapper()).new TagWrapperException(e);
            }
        }
        TagPair.resetCounter();
        return new String(returnBuffer);
    }
    /**
     * @author Christian Schmidt 2005
     *
     */
    public class TagWrapperException extends Exception {

        /**
         * Create a new Instance of TagWrapperException
         *
         *
         */
        public TagWrapperException() {
            super();
            //
        }

        /**
         * Create a new Instance of TagWrapperException
         *
         * @param arg0
         */
        public TagWrapperException(String arg0) {
            super(arg0);
            //
        }

        /**
         * Create a new Instance of TagWrapperException
         *
         * @param arg0
         * @param arg1
         */
        public TagWrapperException(String arg0, Throwable arg1) {
            super(arg0, arg1);
            //
        }

        /**
         * Create a new Instance of TagWrapperException
         *
         * @param arg0
         */
        public TagWrapperException(Throwable arg0) {
            super(arg0);
            //
        }

    }
}
