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
package org.openoffice.xmerge.converter.xml.xslt.docbook;
import  java.lang.String;


public class literallayout{

    public static String RemoveSpaces(String test){
    System.out.println("\nWorking... Length=" +test.length());
      int i=0;
      int paraBreak=10000;
    String result="";
    char data[]={10};
    char tab[]={13};
    if(test!=null){
        String strArr[]= test.split(new String(data));
        test="";

        while (i< strArr.length ){
        //System.out.println("Trying "+ i);

        if (test.length()+strArr[i].length()> paraBreak){
            test=test.concat("</text:p><text:p text:style-name=\"Preformatted Text\" text:name=\"Preformatted Text\">");
            paraBreak+=10000;
        }
        strArr[i]=needsMask(strArr[i]).concat("<text:line-break/>");
        strArr[i]=needsSecondMask(strArr[i]);
         test=test.concat(strArr[i]);
        i++;
        }

        System.out.println(test.length());

    }
     else{
        test="";
        }
    System.out.println("\nDone");
    return test;
    }

    public static String needsMask(String origString){
        if (origString.indexOf("&")!=-1){
        origString=replaceStr(origString,"&","&amp;");
        }
         if (origString.indexOf("\"")!=-1){
        origString=replaceStr(origString,"\"","&quot;");
        }
        if (origString.indexOf("<")!=-1){
        origString=replaceStr(origString,"<","&lt;");
        }
        if (origString.indexOf(">")!=-1){
        origString=replaceStr(origString,">","&gt;");
        }
        return origString;

    }

        public static String needsSecondMask(String origString){
          char data[]={10};
    char tab[]={9};
        if (origString.indexOf("  ")!=-1){
        origString=replaceStr(origString,"  "," <text:s/>");
        }
         if (origString.indexOf(new String(tab))!=-1){
        origString=replaceStr(origString,new String(tab),"<text:tab-stop/>");
        }

        return origString;

    }

    public static String replaceStr(String origString, String origChar, String replaceChar){
           String tmp="";
           int index=origString.indexOf(origChar);
           if(index !=-1){
           while (index !=-1){
               String first =origString.substring(0,index);
               first=first.concat(replaceChar);
               tmp=tmp.concat(first);
               origString=origString.substring(index+1,origString.length());
               index=origString.indexOf(origChar);
               if(index==-1) {
               tmp=tmp.concat(origString);
               }

           }

           }
           return tmp;
    }


    private static String replace(String test){
     int i=0;
     String result="";
     if (test.indexOf("  ",i)!=-1){
        while (test.indexOf("  ",i)!=-1){
            result=result.concat(test.substring(0,test.indexOf("  ",i)));
            result=result.concat(" <text:s/>");
            i=test.indexOf("  ",i)+2;
        }
        return result;
        }
     else{
         return test;
     }
    }

}
