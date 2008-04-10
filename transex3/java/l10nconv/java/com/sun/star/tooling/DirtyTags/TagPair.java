/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TagPair.java,v $
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
import java.util.Iterator;


/**
 * @author Christian Schmidt 2005
 *
 */
public class TagPair {


    private Tag startTag=Tag.EMPTYTAG;
    private Tag endTag=Tag.EMPTYTAG;
    private String startingText="";
    private ArrayList enclosedTags=new ArrayList();
    private long id;
    private static int ElementCounter=1;
    private String endingText="";

    /**
     * @author Christian Schmidt 2005
     *
     */
    public class TagPairConstructionException extends Exception {

        /**
         * Create a new Instance of TagPairConstructionException
         *
         * @param string
         */
        public TagPairConstructionException(String string) {

        }

    }

    /**
     * Create a new Instance of TagPair
     *
     *
     */
    public TagPair() {

    }


    /**
     * Create a new Instance of TagPair
     *
     * Find matching tags in tagList, create a TagPair of it, create
     * tagPairs from the content in the tagPair and remove all used
     * tags from tagList. The rest of the tagList starts after the
     * endTag of this TagPair.
     *
     * @param tagList a List of the tags to check
     *
     * @throws TagPairConstructionException
     */
    public TagPair(ArrayList tagList) throws TagPairConstructionException {

        if(tagList.size()==0){
            return;
        }
        ArrayList contentList=new ArrayList();;
        Tag tag=(Tag)tagList.get(0);
        tagList.remove(0);


        if("Text".equals(tag.getTagType())){
            // is this Text the only content
            // of this Tag ?
            if(tagList.size()==0){
                //yes...then it is the starting Text of this TagPair
                this.startingText=tag.getTagString();
                return;
            }else{
                //no...the tag is normal content
                contentList.add(tag);
            }
           this.startingText=tag.getTagString();

        }else if("EndTag".equals(tag.getTagType())){
            //ERRor throw EXception
        }else if("StartTag".equals(tag.getTagType())){
            // find the matching end tag
            this.startTag=tag;
            Iterator iter=tagList.iterator();

            int equivalentTagCounter=0;
            while(iter.hasNext()){
                //is this the end tag?
                if((tag=(Tag)iter.next()).getTagName().equals('/'+this.startTag.getTagName())&&equivalentTagCounter==0){
                    //found the corresponding end tag

                    //this TagPair is complete
                    //so it needs an id
                    this.id=TagPair.ElementCounter++;
                    this.endTag=tag;
                    //...remove it from list
                    tagList.removeAll(contentList);
                    tagList.remove(tag);
                    break;
                }else{
                    // tag is not the end tag
                    // so it is between the start and the end tag
                    // and belongs to the content
                   // but first check if it has the same name as the current tag
                    if(tag.getTagName().equals(this.startTag.getTagName())){
                        // if this is a start tag like the current start tag
                        // we count it to find out the matching end tag in nested tags
                        if(tag.getTagType().equals("StartTag")){
                            equivalentTagCounter++;
                        }
                    }
                    if(tag.getTagName().equals("/"+this.startTag.getTagName())){
                        if(tag.getTagType().equals("EndTag")){
                            equivalentTagCounter--;
                        }
                    }

                    contentList.add(tag);
                }
            }
            //found the end tag ?
            //no...
            if (this.endTag.getTagType()==""){

                throw new TagPairConstructionException("ERROR: Missing end tag ("+
                        this.startTag.getTagString()+").");
            //...yes
            }else{
                //We need to check whether the content is starting or ending with text
                //...check starting with text
                if(contentList.size()>=1&&((String)((Tag)contentList.get(0)).getTagType()).equals("Text")){
                    //yes...store it as startingText
                    this.startingText=(String)((Tag)contentList.get(0)).getTagString();
                    //remove it from list
                    contentList.remove(0);
                }
                // ...check ending with text
                if(contentList.size()>=1&&((String)((Tag)contentList.get(contentList.size()-1)).getTagType()).equals("Text")){
                    //yes...store it as endingText
                    this.endingText=(String)((Tag)contentList.get(contentList.size()-1)).getTagString();
                    //remove it from list
                    contentList.remove(contentList.size()-1);
                }
                //create the list of tags enclosed by this tagPair
                createEnclosedTags(contentList);
            }
        //if stand AloneTag create own TagObject...give ID...add to List
        }else if("StartAndEndTag".equals(tag.getTagType())){
            this.startTag=tag;
            this.endTag=new Tag("EndOfStandAlone","","");
            createEnclosedTags(contentList);
        }

    }

    /**
     * @param contentList
     * @throws TagPairConstructionException
     */
    private void createEnclosedTags(ArrayList contentList) throws TagPairConstructionException {
        while(contentList.size()>0){
            //create the inner TagPairs
            this.enclosedTags.add(new TagPair(contentList));
        }

    }

    public String toString(){
        StringBuffer outString= new StringBuffer(this.startTag.toString());
        TagPair help=new TagPair();
        Iterator iter=enclosedTags.iterator();
        outString.append(this.startingText);
        while(iter.hasNext()){
            if((help=(TagPair)iter.next())==null){
                continue;
            }else{
                outString.append(help.toString());
            }
        }
        outString.append(this.endingText);
        outString.append(this.endTag.toString());
        return new String(outString);
    }

    public String getWrapped() throws IOException{
        Iterator iter=enclosedTags.iterator();
        StringBuffer returnBuffer=new StringBuffer();

            returnBuffer.append(wrap(this.startTag)+xmlString(this.startingText));
            while(iter.hasNext()){
                returnBuffer.append(((TagPair)iter.next()).getWrapped());
            }
            returnBuffer.append(xmlString(this.endingText)+wrap(this.endTag));



        return new String(returnBuffer);
    }

    private String wrap(Tag tag) throws IOException{
        String string="";
        //can be a start tag
        if(tag.getTagType().startsWith("Start")){
            return new String("<bpt id='"+this.id+"'>"+tag.getWrappedTagString()+"</bpt>");
        //...or a end tag
        }else if (tag.getTagType().startsWith("End")){
            //maybe the end tag of a Start and end tag
//            if("EndOfStandAlone".equals(tag.getTagType())){
//                return new String("<ex id='"+this.id+"'/>");
//            }else{
                string=tag.getWrappedTagString();
                return new String("<ept id='"+this.id+"'>"+string+"</ept>");
//            }

        //...or text
        }else{
            return xmlString(tag.getTagString());
        }
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

    /**
     *
     */
    public static void resetCounter() {
        TagPair.ElementCounter=1;

    }


}
