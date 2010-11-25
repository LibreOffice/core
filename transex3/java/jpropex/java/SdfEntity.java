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


public class SdfEntity implements Cloneable{
    private String project      = new String("");
    private String source_file  = new String("");
    private String dummy1       = new String("");
    private String resource_type= new String("");
    private String gid          = new String("");
    private String lid          = new String("");
    private String helpid       = new String("");
    private String platform     = new String("");
    private String dummy2       = new String("");
    private String langid       = new String("");
    private String text         = new String("");
    private String helptext     = new String("");
    private String quickhelptext= new String("");
    private String title        = new String("");
    private String date         = new String("");

    public static int PROJECT_POS       = 0;
    public static int SOURCE_FILE_POS   = 1;
    public static int DUMMY1_POS        = 2;
    public static int RESOURCE_TYPE_POS = 3;
    public static int GID_POS           = 4;
    public static int LID_POS           = 5;
    public static int HELPID_POS        = 6;
    public static int PLATFORM_POS      = 7;
    public static int DUMMY2_POS        = 8;
    public static int LANGID_POS        = 9;
    public static int TEXT_POS          = 10;
    public static int HELPTEXT_POS      = 11;
    public static int QUICKHELPTEXT_POS = 12;
    public static int TITLE_POS         = 13;
    public static int DATE_POS          = 14;

    public Object clone()
    {
        try
        {
            return super.clone();
        }
        catch( CloneNotSupportedException e )
        {
            System.out.println("ERROR: Can not clone, soemthing is broken here ....");
            System.exit( -1 );
        }
        return null; // dummy
    }

    public SdfEntity( String line ){
        // isValid?
        setProperties( line ) ;
    }
    public SdfEntity(String project, String source_file, String dummy1, String resource_type, String gid, String lid, String helpid, String platform, String dummy2, String langid, String text, String helptext, String quickhelptext, String title , String date) {
        super();
        this.project        = project;
        this.source_file    = source_file;
        this.dummy1         = dummy1;
        this.resource_type  = resource_type;
        this.gid            = gid;
        this.lid            = lid;
        this.helpid         = helpid;
        this.platform       = platform;
        this.dummy2         = dummy2;
        this.langid         = langid;
        this.text           = text;
        this.helptext       = helptext;
        this.quickhelptext  = quickhelptext;
        this.title          = title;
        this.date           = date;
    }

    public void setProperties( String line ){

        String[] splitted       = line.split("\t");

        setProject(         splitted[ SdfEntity.PROJECT_POS ]       );
        setSource_file(     splitted[ SdfEntity.SOURCE_FILE_POS ]   );
        setDummy1(          splitted[ SdfEntity.DUMMY1_POS ]        );
        setResource_type(   splitted[ SdfEntity.RESOURCE_TYPE_POS ] );
        setGid(             splitted[ SdfEntity.GID_POS ]           );
        setLid(             splitted[ SdfEntity.LID_POS ]           );
        setHelpid(          splitted[ SdfEntity.HELPID_POS ]        );
        setPlatform(        splitted[ SdfEntity.PLATFORM_POS ]      );
        setDummy2(          splitted[ SdfEntity.DUMMY2_POS ]        );
        setLangid(          splitted[ SdfEntity.LANGID_POS ]        );
        setText(            splitted[ SdfEntity.TEXT_POS ]          );
        setHelptext(        splitted[ SdfEntity.HELPTEXT_POS ]      );
        setQuickhelptext(   splitted[ SdfEntity.QUICKHELPTEXT_POS ] );
        setTitle(           splitted[ SdfEntity.TITLE_POS ]         );
        setDate(            splitted[ SdfEntity.DATE_POS ]          );
    }

    public String getFileId(){
        return project+"\\"+source_file;
    }
    public String getResourcePath(){
        return source_file.substring(0 , source_file.lastIndexOf( "\\" )-1 );
    }
    public String toString(){
        return project+"\t"+source_file+"\t"+dummy1+"\t"+resource_type+"\t"+gid+"\t"
              +lid+"\t"+helpid+"\t"+platform+"\t"+dummy2+"\t"+langid+"\t"
              +text+"\t"+helptext+"\t"+quickhelptext+"\t"+title+"\t"+date;
    }
    public String getId(){
        return project+gid+lid+source_file+resource_type+platform+helpid;
    }

    public String getDummy1() {
        return dummy1;
    }

    public void setDummy1(String dummy1) {
        this.dummy1 = dummy1;
    }

    public String getPlatform() {
        return platform;
    }

    public void setPlatform(String platform) {
        this.platform = platform;
    }

    public String getDummy2() {
        return dummy2;
    }

    public void setDummy2(String dummy2) {
        this.dummy2 = dummy2;
    }

    public String getGid() {
        return gid;
    }

    public void setGid(String gid) {
        this.gid = gid;
    }

    public String getHelpid() {
        return helpid;
    }

    public void setHelpid(String helpid) {
        this.helpid = helpid;
    }

    public String getHelptext() {
        return helptext;
    }

    public void setHelptext(String helptext) {
        this.helptext = helptext;
    }

    public String getLangid() {
        return langid;
    }

    public void setLangid(String langid) {
        this.langid = langid;
    }

    public String getLid() {
        return lid;
    }

    public void setLid(String lid) {
        this.lid = lid;
    }

    public String getProject() {
        return project;
    }

    public void setProject(String project) {
        this.project = project;
    }

    public String getQuickhelptext() {
        return quickhelptext;
    }

    public void setQuickhelptext(String quickhelptext) {
        this.quickhelptext = quickhelptext;
    }

    public String getResource_type() {
        return resource_type;
    }

    public void setResource_type(String resource_type) {
        this.resource_type = resource_type;
    }

    public String getSource_file() {
        return source_file;
    }

    public void setSource_file(String source_file) {
        this.source_file = source_file;
    }

    public String getText() {
        return text;
    }

    public void setText(String text) {
        this.text = text;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }
    public String getDate() {
        return date;
    }
    public void setDate(String date) {
        this.date = date;
    }


}
