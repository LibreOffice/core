/*************************************************************************
 *
 *  $RCSfile: CGDocument.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:16:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */
package com.sun.star.wizards.web.data;

import com.sun.star.beans.PropertyValue;
import com.sun.star.document.*;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.DateTime;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.ui.event.Task;

import java.io.File;
import java.io.FileNotFoundException;
import java.text.DateFormat;
import java.util.Date;
import java.util.Locale;


import org.w3c.dom.Node;

/**
 * About the member fields Title, Decription and Author:
 * There is an ambiguicy in handling these fields.
 * On the one hand, the user can set those fields to a custom value,
 * relevant only for the web-wizard.
 * On the other-hand, the web-wizard can read those fields automatically
 * from SO documents.
 * In order to avoid conflict by saving, I use two fields: "cp_title", which
 * should be read from the configuratin (saved session), and "title" which is
 * the value read from the document.
 * The *cp_title* "overrides" *title*, if exists. if not, *title* is used.
 * The same is valid for *description* and *author*.
 * @author  rpiterman
 */

public class CGDocument extends ConfigSetItem implements XMLProvider {
    public static final int PAGE_TYPE_UNKNOWN = 0;
    public static final int PAGE_TYPE_PAGE = 1;
    public static final int PAGE_TYPE_SLIDE = 2;

    public String       cp_Title = "";
    public String       cp_Description = "";
    public String       cp_URL;
    public String       cp_Author = "";
    public String       cp_Exporter;

    /**
     * the destination filename to which this
     * document will be exported to.
     */
    public String       localFilename = "";
    public String       urlFilename = "";
    public String       title = "";
    public String       description = "";
    public String       author = "";
    public DateTime     createDate;
    public DateTime     updateDate;

    public int          sizeKB = -1;
    public int          pages = -1;

    /**
     * The contents subdirectory name in which the document is in.
     * This subdirectory will be created in the content's "docs"
     * subdirectory.
     */
    public String       dirName;

    /**
     * contains an identifier of the type of this document.
     */
    public String          appType;
    public PropertyValue[] mediaDescriptor;

    /**
     * Is this document an original SO Document (different versions).
     */
    public boolean      isSODocument;

    /**
     * can this document be opend through SO?
     */
    public boolean      isSOOpenable;

    /**
     * This flag is set to true after the validate method is called.
     */
    public boolean      valid = false;

    public String       targetTypeName;

    public CGDocument() {}

    public CGDocument(String url, XMultiServiceFactory xmsf,Task task) throws Exception {
        cp_URL = JavaTools.converttoURLNotation(url);
        if (task==null) task = new Task("","",5);
        validate(xmsf,task);
    }

    private static FileAccess fileAccess;

    private static FileAccess getFileAccess(XMultiServiceFactory xmsf) throws Exception {
        if (fileAccess==null)
          fileAccess = new FileAccess(xmsf);
        return fileAccess;
    }

    /**
     * the task will advance 5 times during validate.
     * @param xmsf
     * @param task
     * @throws Exception
     */
    public void validate(XMultiServiceFactory xmsf, Task task)
        throws  FileNotFoundException,
                IllegalArgumentException,
                Exception {

        if (!getFileAccess(xmsf).exists(cp_URL,false))
              throw new FileNotFoundException("The given URL does not point to a file");

        if (getFileAccess(xmsf).isDirectory(cp_URL))
              throw new IllegalArgumentException("The given URL points to a directory");

        //create a TypeDetection service

        mediaDescriptor = OfficeDocument.getFileMediaDecriptor(xmsf,cp_URL);

        task.advance(true); //1

        //System.out.println(Properties.getPropertyValue(mediaDescriptor,"UIName"));

        analyzeFileType(mediaDescriptor);

        task.advance(true); //2

        String path = getFileAccess(xmsf).getPath(cp_URL,"");
        localFilename = getFileAccess(xmsf).getFilename(path,File.separator);

        /* if the type is a star office convertable document
         * We try to open the document to get some properties
         */

        //first get the info object which is a DocumentInfo service.
        Object info = null;
        if (isSODocument) {//for SO documents, use StandaloneDocumentInfo service.
            info = xmsf.createInstance("com.sun.star.document.StandaloneDocumentInfo");
            ((XStandaloneDocumentInfo)UnoRuntime.queryInterface(XStandaloneDocumentInfo.class, info)).loadFromURL(cp_URL);
        }

        task.advance(true); //3

        if (!isSODocument && isSOOpenable) { //for other documents which are openable through SO, use DocumentInfo service.
            XDesktop desktop = Desktop.getDesktop(xmsf);
            PropertyValue[] props = new PropertyValue[] { new PropertyValue() };
            props[0].Name="Hidden";
            props[0].Value=Boolean.TRUE;
            XComponent component = ((XComponentLoader)UnoRuntime.queryInterface(XComponentLoader.class,desktop)).loadComponentFromURL(cp_URL,"_default",0,props);
            info = ((XDocumentInfoSupplier)UnoRuntime.queryInterface(XDocumentInfoSupplier.class, component)).getDocumentInfo();
        }

        task.advance(true); //4

        //now use the object to read some document properties.
        if (isSODocument || isSOOpenable) {
            title = (String)Helper.getUnoPropertyValue(info,"Title");
            description = (String)Helper.getUnoPropertyValue(info,"Description");
            author = (String)Helper.getUnoPropertyValue(info,"Author");
            createDate = (DateTime)Helper.getUnoPropertyValue(info,"CreationDate",DateTime.class);
            updateDate = (DateTime)Helper.getUnoPropertyValue(info,"ModifyDate",DateTime.class);
            //TODO get pages here.
        }
        else { //get some information from OS.
            title = localFilename;
            updateDate = getFileAccess(xmsf).getLastModified(cp_URL);
        }

        task.advance(true); //5

        valid = true;

        if (cp_Title.equals("")) cp_Title=title;
        if (cp_Title.equals("")) cp_Title=localFilename;
        if (cp_Description.equals("")) cp_Description=description;
        if (cp_Author.equals("")) cp_Author = author;

        if (cp_Exporter == null || cp_Exporter.equals(""))
            cp_Exporter = (String)getSettings().cp_Exporters.getKey(
                getSettings().getExporters(appType)[0] );

    }

    /**
     * Analyzes a type-detection string, returned from the TypeDetection service,
     * and sets the appType, isSOOpenable and isSODocument memebres.
     */
    private void analyzeFileType(PropertyValue[] mediaDesc) {

        if (mediaDesc == null) {
        }

        String media = (mediaDesc == null)
            ? "" :  (String) Properties.getPropertyValue(mediaDescriptor,"Name");
        appType = getDocType(media);

        //System.out.println(appType);

        isSOOpenable =
          ( appType == TypeDetection.WRITER_DOC
               || appType == TypeDetection.CALC_DOC
               || appType == TypeDetection.IMPRESS_DOC
               || appType == TypeDetection.DRAW_DOC ) || appType == TypeDetection.HTML_DOC;

        String[] parts = media.split("_");

        isSODocument = isSOOpenable  && ( parts[1].startsWith("Star") );

    }

    /**
     * @param media is the media description string returned by an UNO TypeDetection object.
     * @return one of the constants in the interface TypeDetection.
     *
     */
    private String getDocType(String media) {
        if (media.equals("")) return TypeDetection.NO_TYPE;

        if (media.startsWith("writer")) {
            if (media.startsWith("writer_web_HTML")) return TypeDetection.HTML_DOC;
            else return TypeDetection.WRITER_DOC;
        }
        else if (media.startsWith("calc")) return TypeDetection.CALC_DOC;
        else if (media.startsWith("draw")) return TypeDetection.DRAW_DOC;
        else if (media.startsWith("impress")) return TypeDetection.IMPRESS_DOC;
        else if (media.startsWith("pdf")) return TypeDetection.PDF_DOC;
        else if (media.startsWith("gif") || media.startsWith("jpg")) return TypeDetection.WEB_GRAPHICS;
        else if (media.startsWith("wav")) return TypeDetection.SOUND_FILE;
        else return TypeDetection.NO_TYPE;
    }


    private String dateString(DateTime dateTime) {
        if (dateTime == null) return "";
        Date date = new Date(JavaTools.getMillis(dateTime));
        DateFormat dateFormatter = DateFormat.getDateInstance(DateFormat.LONG,Locale.getDefault());
        return dateFormatter.format(date);
    }

    public Node createDOM(Node parent) {
        CGDesign d = getSettings().cp_DefaultSession.cp_Design;
        CGExporter exp = (CGExporter)getSettings().cp_Exporters.getElement(cp_Exporter);

        return XMLHelper.addElement(parent,"document",
          new String[] {"title","description","author","format","filename","create-date","update-date","pages","size","icon","dir","fn"},
          new String[] {
              d.cp_DisplayTitle ? cp_Title : "" ,
              d.cp_DisplayDescription ? cp_Description : "",
              d.cp_DisplayAuthor ? cp_Author : "",
              d.cp_DisplayFileFormat ? getTargetTypeName(exp) : "",
              d.cp_DisplayFilename ? localFilename : "",
              d.cp_DisplayCreateDate ? createDate(dateString(createDate)) : "",
              d.cp_DisplayUpdateDate ? updateDate(dateString(updateDate)) : "",
              d.cp_DisplayPages && (pages > -1) ? "" + pages : "", //TODO when do i calculate pages?
              d.cp_DisplaySize && (sizeKB > -1) ? "" + sizeKB: "", //TODO when do i calculate size?
              d.cp_DisplayFormatIcon ? getIcon(exp) : "",
              dirName, urlFilename}
        );
    }


    private String updateDate(String s) {
        return JavaTools.replaceSubString(getSettings().resUpdated,s,"%DATE");
    }

    private String createDate(String s) {
            return JavaTools.replaceSubString(getSettings().resCreated,s,"%DATE");
    }

    private String pages() {
        return pages == -1 ?  ""
         : JavaTools.replaceSubString(pagesTemplate() , "" + pages , "%NUMBER" );
    }

    private String pagesTemplate() {
        int pagesType = ((CGExporter)getSettings().cp_Exporters.getElement(cp_Exporter)).cp_PageType;
        switch (pagesType) {
            case PAGE_TYPE_UNKNOWN :
                return "";
            case PAGE_TYPE_PAGE :
                return getSettings().resPages;
            case PAGE_TYPE_SLIDE :
                return getSettings().resSlides;
            default : return "";
        }
    }

    private String getTargetTypeName(CGExporter exp) {
        return (exp.targetTypeName.equals(""))
            ? (String) Properties.getPropertyValue(mediaDescriptor,"UIName")
            : exp.targetTypeName;
    }

    private String getIcon(CGExporter exporter) {
        return exporter.cp_Icon.equals("") ?
            getIcon(this.appType) : exporter.cp_Icon;
    }

    private String getIcon(String appType) {
        //System.out.println("Getting Icon for: " + appType);
        return appType+".gif";
    }

    /**
     * This method is used by the DataAware model
     * and returns the index of the exporter of
     * this document, <b>in the list of the
     * exporters available for this type of document</b>
     * @return
     */
    public short[] getExporter() {
        return new short[] {getExporterIndex()};
    }

    /**
     * @see getExporter()
     */
    public void setExporter(short[] exporter_) {
        Object exp = getSettings().getExporters(appType)[exporter_[0]];
        cp_Exporter = (String)getSettings().cp_Exporters.getKey(exp);
    }

    /**
     * @see getExporter()
     * @return
     */
    private short getExporterIndex() {
        if (cp_Exporter==null)
          return 0;
        Object exporter = getSettings().cp_Exporters.getElement(cp_Exporter);
        Object[] exporters = getSettings().getExporters(appType);
        for (short i = 0; i<exporters.length; i++)
          if (exporters[i] == exporter)
            return i;
        return -1;
    }

    public String toString() {
        return localFilename;
    }
}