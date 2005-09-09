/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: literallayout.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:28:02 $
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
