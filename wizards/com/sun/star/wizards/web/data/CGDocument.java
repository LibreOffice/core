/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.wizards.web.data;

import java.io.File;
import java.io.FileNotFoundException;

import org.w3c.dom.Node;

import com.sun.star.beans.PropertyValue;
import com.sun.star.document.MacroExecMode;
import com.sun.star.document.UpdateDocMode;
import com.sun.star.document.XDocumentProperties;
import com.sun.star.document.XDocumentPropertiesSupplier;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.DateTime;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.XMLHelper;
import com.sun.star.wizards.common.XMLProvider;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.ui.event.Task;

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
 */
public class CGDocument extends ConfigSetItem implements XMLProvider
{

    public static final int PAGE_TYPE_UNKNOWN = 0;
    public static final int PAGE_TYPE_PAGE = 1;
    public static final int PAGE_TYPE_SLIDE = 2;
    public String cp_Title = PropertyNames.EMPTY_STRING;
    public String cp_Description = PropertyNames.EMPTY_STRING;
    public String cp_URL;
    public String cp_Author = PropertyNames.EMPTY_STRING;
    public String cp_Exporter;
    /**
     * the destination filename to which this
     * document will be exported to.
     */
    public String localFilename = PropertyNames.EMPTY_STRING;
    public String urlFilename = PropertyNames.EMPTY_STRING;
    public String title = PropertyNames.EMPTY_STRING;
    public String description = PropertyNames.EMPTY_STRING;
    public String author = PropertyNames.EMPTY_STRING;
    public DateTime createDate;
    public DateTime updateDate;
    public int sizeBytes = -1;
    public int pages = -1;
    /**
     * The contents subdirectory name in which the document is in.
     * This subdirectory will be created in the content's "docs"
     * subdirectory.  
     */
    public String dirName;
    /**
     * contains an identifier of the type of this document.
     */
    public String appType;
    public PropertyValue[] mediaDescriptor;
    /**
     * Is this document an original SO Document (different versions).
     */
    public boolean isSODocument;
    /**
     * can this document be opend through SO?
     */
    public boolean isSOOpenable;
    /**
     * This flag is set to true after the validate method is called.
     */
    public boolean valid = false;
    public String targetTypeName;

    public CGDocument()
    {
    }

    public CGDocument(String url, XMultiServiceFactory xmsf, Task task) throws Exception
    {
        cp_URL = getSettings().getFileAccess(xmsf).getURL(url);
        if (task == null)
        {
            task = new Task(PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING, 5);
        }
        validate(xmsf, task);
    }

    /**
     * the task will advance 5 times during validate.
     * @param xmsf
     * @param task
     * @throws Exception
     */
    public void validate(XMultiServiceFactory xmsf, Task task)
            throws FileNotFoundException,
            IllegalArgumentException,
            Exception
    {

        if (!getSettings().getFileAccess(xmsf).exists(cp_URL, false))
        {
            throw new FileNotFoundException("The given URL does not point to a file");
        }
        if (getSettings().getFileAccess(xmsf).isDirectory(cp_URL))
        {
            throw new IllegalArgumentException("The given URL points to a directory");        //create a TypeDetection service
        }
        mediaDescriptor = OfficeDocument.getFileMediaDecriptor(xmsf, cp_URL);

        task.advance(true); //1

        analyzeFileType(mediaDescriptor);

        task.advance(true); //2

        String path = getSettings().getFileAccess(xmsf).getPath(cp_URL, PropertyNames.EMPTY_STRING);
        localFilename = FileAccess.getFilename(path, File.separator);

        /* if the type is a star office convertable document
         * We try to open the document to get some properties
         */

        XDocumentProperties xProps = null;

        task.advance(true); //3

        if (isSOOpenable)
        { // for documents which are openable through SO, use DocumentProperties service.
            XDesktop desktop = Desktop.getDesktop(xmsf);
            PropertyValue[] props = new PropertyValue[3];
            props[0] = Properties.createProperty("Hidden", Boolean.TRUE);
            props[1] = Properties.createProperty("MacroExecutionMode", new Short(MacroExecMode.NEVER_EXECUTE));
            props[2] = Properties.createProperty("UpdateDocMode", new Short(UpdateDocMode.NO_UPDATE));
            XComponent component = UnoRuntime.queryInterface(XComponentLoader.class, desktop).loadComponentFromURL(cp_URL, "_default", 0, props);
            xProps = UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, component).getDocumentProperties();
        }

        task.advance(true); //4    

        //now use the object to read some document properties.
        if (xProps != null)
        {
            title = xProps.getTitle();
            description = xProps.getDescription();
            author = xProps.getAuthor();
            createDate = xProps.getCreationDate();
            updateDate = xProps.getModificationDate();
        }
        else
        { //get some information from OS.
            title = localFilename;
            updateDate = getSettings().getFileAccess(xmsf).getLastModified(cp_URL);
        }

        task.advance(true); //5

        valid = true;

        if (cp_Title.equals(PropertyNames.EMPTY_STRING))
        {
            cp_Title = title;
        }
        if (cp_Title.equals(PropertyNames.EMPTY_STRING))
        {
            cp_Title = localFilename;
        }
        if (cp_Description.equals(PropertyNames.EMPTY_STRING))
        {
            cp_Description = description;
        }
        if (cp_Author.equals(PropertyNames.EMPTY_STRING))
        {
            cp_Author = author;
        }
        if (cp_Exporter == null || cp_Exporter.equals(PropertyNames.EMPTY_STRING))
        {
            cp_Exporter = (String) getSettings().cp_Exporters.getKey(
                    getSettings().getExporters(appType)[0]);
        }
    }

    /**
     * Analyzes a type-detection string, returned from the TypeDetection service,
     * and sets the appType, isSOOpenable and isSODocument members.
     */
    private void analyzeFileType(PropertyValue[] mediaDesc)
    {

        if (mediaDesc == null)
        {
        }

        String media = (mediaDesc == null)
                ? PropertyNames.EMPTY_STRING : (String) Properties.getPropertyValue(mediaDescriptor, PropertyNames.PROPERTY_NAME);
        appType = getDocType(media);

        isSOOpenable =
                (appType.equals(TypeDetection.WRITER_DOC) || appType.equals(TypeDetection.CALC_DOC) || appType.equals(TypeDetection.IMPRESS_DOC) || appType.equals(TypeDetection.DRAW_DOC)) || appType.equals(TypeDetection.HTML_DOC);

        String[] parts = JavaTools.ArrayoutofString(media, "_");


        isSODocument = parts.length >= 2 && isSOOpenable && (parts[1].startsWith("Star"));

    }

    /**
     * @param media is the media description string returned by an UNO TypeDetection object.
     * @return one of the constants in the interface TypeDetection.
     * 
     */
    private String getDocType(String media)
    {
        if (media.equals(PropertyNames.EMPTY_STRING))
        {
            return TypeDetection.NO_TYPE;
        }
        if (media.startsWith("generic_HTML"))
        {
            return TypeDetection.HTML_DOC;
        }
        if (media.startsWith("writer"))
        {
            return TypeDetection.WRITER_DOC;
        }
        else if (media.startsWith("calc"))
        {
            return TypeDetection.CALC_DOC;
        }
        else if (media.startsWith("draw"))
        {
            return TypeDetection.DRAW_DOC;
        }
        else if (media.startsWith("impress"))
        {
            return TypeDetection.IMPRESS_DOC;
        }
        else if (media.startsWith("pdf"))
        {
            return TypeDetection.PDF_DOC;
        }
        else if (media.startsWith("gif") || media.startsWith("jpg"))
        {
            return TypeDetection.WEB_GRAPHICS;
        }
        else if (media.startsWith("wav"))
        {
            return TypeDetection.SOUND_FILE;
        }
        else
        {
            return TypeDetection.NO_TYPE;
        }
    }

    public Node createDOM(Node parent)
    {
        CGDesign d = getSettings().cp_DefaultSession.cp_Design;
        CGExporter exp = (CGExporter) getSettings().cp_Exporters.getElement(cp_Exporter);

        return XMLHelper.addElement(parent, "document",
                new String[]
                {
                    "title", "description", "author", "format", "filename", "create-date", "update-date", "pages", "size", "icon", "dir", "fn"
                },
                new String[]
                {
                    d.cp_DisplayTitle ? cp_Title : PropertyNames.EMPTY_STRING,
                    d.cp_DisplayDescription ? cp_Description : PropertyNames.EMPTY_STRING,
                    d.cp_DisplayAuthor ? cp_Author : PropertyNames.EMPTY_STRING,
                    d.cp_DisplayFileFormat ? getTargetTypeName(exp) : PropertyNames.EMPTY_STRING,
                    d.cp_DisplayFilename ? localFilename : PropertyNames.EMPTY_STRING,
                    d.cp_DisplayCreateDate ? createDate() : PropertyNames.EMPTY_STRING,
                    d.cp_DisplayUpdateDate ? updateDate() : PropertyNames.EMPTY_STRING,
                    d.cp_DisplayPages && (pages > -1) ? PropertyNames.EMPTY_STRING + pages() : PropertyNames.EMPTY_STRING, //TODO when do i calculate pages?
                    d.cp_DisplaySize ? sizeKB() : PropertyNames.EMPTY_STRING,//TODO when do i calculate size?
                    d.cp_DisplayFormatIcon ? getIcon(exp) : PropertyNames.EMPTY_STRING,
                    dirName, urlFilename
                });
    }

    private String updateDate()
    {
        if (this.updateDate == null)
        {
            return PropertyNames.EMPTY_STRING;
        }
        return getSettings().formatter.formatCreated(this.updateDate);
    }

    private String createDate()
    {
        if (this.createDate == null)
        {
            return PropertyNames.EMPTY_STRING;
        }
        return getSettings().formatter.formatCreated(this.createDate);
    }

    private String sizeKB()
    {
        if (sizeBytes == -1)
        {
            return PropertyNames.EMPTY_STRING;
        }
        else
        {
            return getSettings().formatter.formatFileSize(sizeBytes);
        }
    }

    private String pages()
    {
        return pages == -1 ? PropertyNames.EMPTY_STRING
                : JavaTools.replaceSubString(pagesTemplate(), PropertyNames.EMPTY_STRING + pages, "%NUMBER");
    }

    private String pagesTemplate()
    {
        int pagesType = ((CGExporter) getSettings().cp_Exporters.getElement(cp_Exporter)).cp_PageType;
        switch (pagesType)
        {
            case PAGE_TYPE_UNKNOWN:
                return PropertyNames.EMPTY_STRING;
            case PAGE_TYPE_PAGE:
                return getSettings().resources[CGSettings.RESOURCE_PAGES_TEMPLATE];
            case PAGE_TYPE_SLIDE:
                return getSettings().resources[CGSettings.RESOURCE_SLIDES_TEMPLATE];
            default:
                return PropertyNames.EMPTY_STRING;
        }
    }

    private String getTargetTypeName(CGExporter exp)
    {
        return (exp.targetTypeName.equals(PropertyNames.EMPTY_STRING))
                ? (String) Properties.getPropertyValue(mediaDescriptor, "UIName")
                : exp.targetTypeName;
    }

    private String getIcon(CGExporter exporter)
    {
        return exporter.cp_Icon.equals(PropertyNames.EMPTY_STRING) ? getIcon(this.appType) : exporter.cp_Icon;
    }

    private String getIcon(String appType)
    {
        return appType + ".gif";
    }

    /**
     * This method is used by the DataAware model
     * and returns the index of the exporter of
     * this document, <b>in the list of the
     * exporters available for this type of document</b>
     * @return
     */
    public short[] getExporter()
    {
        return new short[]
                {
                    getExporterIndex()
                };
    }

    /**
     * @see #getExporter()
     */
    public void setExporter(short[] exporter_)
    {
        Object exp = getSettings().getExporters(appType)[exporter_[0]];
        cp_Exporter = (String) getSettings().cp_Exporters.getKey(exp);
    }

    /**
     * @see #getExporter()
     * @return
     */
    private short getExporterIndex()
    {
        if (cp_Exporter == null)
        {
            return 0;
        }
        Object exporter = getSettings().cp_Exporters.getElement(cp_Exporter);
        Object[] exporters = getSettings().getExporters(appType);
        for (short i = 0; i < exporters.length; i++)
        {
            if (exporters[i] == exporter)
            {
                return i;
            }
        }
        return -1;
    }

    public String toString()
    {
        return localFilename;
    }
}
