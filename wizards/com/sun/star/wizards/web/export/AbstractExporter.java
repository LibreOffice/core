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
/*
 * AbstractExporter.java
 *
 * Created on 1. Oktober 2003, 16:12
 */
package com.sun.star.wizards.web.export;

import com.sun.star.beans.PropertyValue;
import com.sun.star.document.MacroExecMode;
import com.sun.star.document.UpdateDocMode;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XStorable;
import com.sun.star.io.IOException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.document.OfficeDocument;
import com.sun.star.wizards.text.TextDocument;
import com.sun.star.wizards.web.data.CGArgument;
import com.sun.star.wizards.web.data.CGDocument;
import com.sun.star.wizards.web.data.CGExporter;
import com.sun.star.wizards.web.data.TypeDetection;

public abstract class AbstractExporter implements Exporter
{

    protected CGExporter exporter;
    protected FileAccess fileAccess;

    protected void storeToURL(Object officeDocument, Properties props, String targetUrl, String filterName, PropertyValue[] filterData)
            throws IOException
    {

        props = new Properties();
        props.put("FilterName", filterName);

        if (filterData.length > 0)
        {
            props.put("FilterData", filterData);
        }
        XStorable xs = UnoRuntime.queryInterface(XStorable.class, officeDocument);
        PropertyValue[] o = props.getProperties();
        xs.storeToURL(targetUrl, o);
    }

    protected void storeToURL(Object officeDocument, String targetUrl, String filterName, PropertyValue[] filterData)
            throws IOException
    {

        storeToURL(officeDocument, new Properties(), targetUrl, filterName, filterData);
    }

    protected void storeToURL(Object officeDocument, String targetUrl, String filterName)
            throws IOException
    {

        storeToURL(officeDocument, new Properties(), targetUrl, filterName, new PropertyValue[0]);

    }

    protected String getArgument(String name, CGExporter p)
    {
        return ((CGArgument) p.cp_Arguments.getElement(name)).cp_Value;
    }

    protected Object openDocument(CGDocument doc, XMultiServiceFactory xmsf)
            throws com.sun.star.io.IOException
    {
        Object document = null;
        //open the document.
        try
        {
            XDesktop desktop = Desktop.getDesktop(xmsf);
            Properties props = new Properties();
            props.put("Hidden", Boolean.TRUE);
            props.put("MacroExecutionMode", new Short(MacroExecMode.NEVER_EXECUTE));
            props.put("UpdateDocMode", new Short(UpdateDocMode.NO_UPDATE));
            document = UnoRuntime.queryInterface(XComponentLoader.class, desktop).loadComponentFromURL(doc.cp_URL, "_blank", 0, props.getProperties());
        }
        catch (com.sun.star.lang.IllegalArgumentException iaex)
        {
        }
        //try to get the number of pages in the document;
        try
        {
            pageCount(doc, document);
        }
        catch (Exception ex)
        {
            //Here i do nothing since pages is not *so* important.
        }
        return document;
    }

    protected void closeDocument(Object doc, XMultiServiceFactory xmsf)
    {
        try
        {
            XCloseable xc = UnoRuntime.queryInterface(XCloseable.class, doc);
            xc.close(false);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    private void pageCount(CGDocument doc, Object document)
    {
        if (doc.appType.equals(TypeDetection.WRITER_DOC))
        {
            doc.pages = TextDocument.getPageCount(document);
        }
        else if (doc.appType.equals(TypeDetection.IMPRESS_DOC))
        {
            doc.pages = OfficeDocument.getSlideCount(document);
        }
        else if (doc.appType.equals(TypeDetection.DRAW_DOC))
        {
            doc.pages = OfficeDocument.getSlideCount(document);
        }
    }

    public void init(CGExporter exporter_)
    {
        exporter = exporter_;
    }

    protected FileAccess getFileAccess(XMultiServiceFactory xmsf)
    {
        if (fileAccess == null)
        {
            try
            {
                fileAccess = new FileAccess(xmsf);
            }
            catch (Exception ex)
            {
            }
        }
        return fileAccess;
    }

    protected void calcFileSize(CGDocument doc, String url, XMultiServiceFactory xmsf)
    {
        /*if the exporter exports to a
         * binary format, get the size of the destination.
         */
        if (exporter.cp_Binary)
        {
            doc.sizeBytes = getFileAccess(xmsf).getSize(url);
        }
    }
}
