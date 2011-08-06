/*
 ************************************************************************
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

/**
 *
 * @author  rpiterman
 */
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
        /*OfficeDocument.dispose(
        xmsf,
        (XComponent) UnoRuntime.queryInterface(XComponent.class, doc));*/
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
