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
 * IconSet.java
 *
 * Created on 12. September 2003, 17:16
 */
package com.sun.star.wizards.web.data;

import java.util.Hashtable;
import java.util.Map;
import java.util.Vector;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameAccess;
import com.sun.star.i18n.NumberFormatIndex;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.DateTime;
import com.sun.star.wizards.common.*;
import com.sun.star.wizards.common.Helper.DateUtils;

/**
 * @author  rpiterman
 */
public class CGSettings extends ConfigGroup
{

    public String soTemplateDir;
    public String soGalleryDir;
    public String workPath = null;
    public String cp_WorkDir;
    public ConfigSet cp_Exporters = new ConfigSet(CGExporter.class);
    public ConfigSet cp_Layouts = new ConfigSet(CGLayout.class);
    public ConfigSet cp_Styles = new ConfigSet(CGStyle.class);
    public ConfigSet cp_IconSets = new ConfigSet(CGIconSet.class);
    public ConfigSet cp_BackgroundImages = new ConfigSet(CGImage.class);
    public ConfigSet cp_SavedSessions = new ConfigSet(CGSessionName.class);
    public ConfigSet cp_Filters = new ConfigSet(CGFilter.class);
    public ConfigSet savedSessions = new ConfigSet(CGSessionName.class);
    public CGSession cp_DefaultSession = new CGSession();
    public String cp_LastSavedSession;
    private Map exportersMap = new Hashtable();
    private XMultiServiceFactory xmsf;
    String[] resources;
    public Formatter formatter;
    public static final int RESOURCE_PAGES_TEMPLATE = 0;
    public static final int RESOURCE_SLIDES_TEMPLATE = 1;
    public static final int RESOURCE_CREATED_TEMPLATE = 2;
    public static final int RESOURCE_UPDATED_TEMPLATE = 3;
    public static final int RESOURCE_SIZE_TEMPLATE = 4;

    public CGSettings(XMultiServiceFactory xmsf_, String[] resources_, Object document)
    {
        xmsf = xmsf_;
        try
        {
            soTemplateDir = FileAccess.getOfficePath(xmsf, "Config", PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING);
            soGalleryDir = FileAccess.getOfficePath(xmsf, "Gallery", "share", PropertyNames.EMPTY_STRING);
            root = this;
            formatter = new Formatter(xmsf, document);
            resources = resources_;
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }
    private static final CGExporter[] EMPTY_ARRAY_1 = new CGExporter[0];

    public CGExporter[] getExporters(String mime)
    {
        CGExporter[] exps = (CGExporter[]) exportersMap.get(mime);
        if (exps == null)
        {
            exportersMap.put(mime, exps = createExporters(mime));
        }
        return exps;
    }

    private CGExporter[] createExporters(String mime)
    {
        Object[] exporters = cp_Exporters.items();
        Vector v = new Vector();
        for (int i = 0; i < exporters.length; i++)
        {
            if (((CGExporter) exporters[i]).supports(mime))
            {
                try
                {
                    v.add(exporters[i]);
                }
                catch (Exception ex)
                {
                    ex.printStackTrace();
                }
            }
        }
        return (CGExporter[]) v.toArray(EMPTY_ARRAY_1);
    }

    /**
     * call after read.
     * @param xmsf
     * @param document the background document. used for date/number formatting.
     */
    public void configure(XMultiServiceFactory xmsf) throws Exception
    {
        workPath = FileAccess.connectURLs(soTemplateDir, cp_WorkDir);
        calcExportersTargetTypeNames(xmsf);
    }

    private void calcExportersTargetTypeNames(XMultiServiceFactory xmsf) throws Exception
    {
        Object typeDetect = xmsf.createInstance("com.sun.star.document.TypeDetection");
        XNameAccess xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, typeDetect);
        for (int i = 0; i < cp_Exporters.getSize(); i++)
        {
            calcExporterTargetTypeName(xNameAccess, (CGExporter) cp_Exporters.getElementAt(i));
        }
    }

    private void calcExporterTargetTypeName(XNameAccess xNameAccess, CGExporter exporter)
            throws NoSuchElementException,
            WrappedTargetException
    {
        if (!exporter.cp_TargetType.equals(PropertyNames.EMPTY_STRING))
        {
            exporter.targetTypeName =
                    (String) Properties.getPropertyValue(
                    (PropertyValue[]) xNameAccess.getByName(exporter.cp_TargetType),
                    "UIName");
        }
    }
    FileAccess fileAccess;

    FileAccess getFileAccess() throws Exception
    {
        return getFileAccess(xmsf);
    }

    FileAccess getFileAccess(XMultiServiceFactory xmsf) throws Exception
    {
        if (fileAccess == null)
        {
            fileAccess = new FileAccess(xmsf);
        }
        return fileAccess;
    }

    public class Formatter
    {

        private long docNullTime;
        private int dateFormat,  numberFormat;
        private DateUtils dateUtils;

        public Formatter(XMultiServiceFactory xmsf, Object document) throws Exception
        {
            dateUtils = new DateUtils(xmsf, document);
            dateFormat = dateUtils.getFormat(NumberFormatIndex.DATE_SYS_DMMMYYYY);
            numberFormat = dateUtils.getFormat(NumberFormatIndex.NUMBER_1000DEC2);
        }

        public String formatCreated(DateTime date)
        {
            String sDate = dateUtils.format(dateFormat, date);
            return JavaTools.replaceSubString(resources[CGSettings.RESOURCE_CREATED_TEMPLATE], sDate, "%DATE");
        }

        public String formatUpdated(DateTime date)
        {
            String sDate = dateUtils.format(dateFormat, date);
            return JavaTools.replaceSubString(resources[CGSettings.RESOURCE_UPDATED_TEMPLATE], sDate, "%DATE");
        }

        public String formatFileSize(int size)
        {
            float sizeInKB = ((float) size) / 1024f;
            String sSize = dateUtils.getFormatter().convertNumberToString(numberFormat, sizeInKB);
            return JavaTools.replaceSubString(resources[CGSettings.RESOURCE_SIZE_TEMPLATE], sSize, "%NUMBER");
        }
    }
}
