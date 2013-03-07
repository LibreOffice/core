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
package org.libreoffice.report.util;

import org.libreoffice.report.OutputRepository;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.jfree.report.JFreeReportBoot;

import org.pentaho.reporting.libraries.xmlns.common.AttributeList;
import org.pentaho.reporting.libraries.xmlns.writer.DefaultTagDescription;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriter;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriterSupport;


/**
 * Creation-Date: 24.07.2007, 18:16:52
 *
 */
public class ManifestWriter
{
    // need this two strings other it breaks the ooo build :-(

    public static final String MANIFEST_NS = "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0";
    public static final String TAG_DEF_PREFIX = "org.libreoffice.report.pentaho.output.";
    private final Map<String,String> entries;

    public ManifestWriter()
    {
        entries = new HashMap<String,String>();
    }

    public void addEntry(final String fullPath, final String mimeType)
    {
        if (fullPath == null)
        {
            throw new NullPointerException();
        }
        if (mimeType == null)
        {
            throw new NullPointerException();
        }
        if ("META-INF/manifest.xml".equals(fullPath))
        {
            return;
        }
        entries.put(fullPath, mimeType);
    }

    public boolean isEmpty()
    {
        return entries.isEmpty();
    }

    public void write(final OutputRepository outputRepository) throws IOException
    {
        if (isEmpty())
        {
            return;
        }

        final DefaultTagDescription tagDescription = new DefaultTagDescription();
        tagDescription.configure(JFreeReportBoot.getInstance().getGlobalConfig(),
                TAG_DEF_PREFIX);

        final OutputStream manifestOutputStream =
                outputRepository.createOutputStream("META-INF/manifest.xml", "text/xml");

        final OutputStreamWriter writer = new OutputStreamWriter(manifestOutputStream, "UTF-8");
        final XmlWriter xmlWriter = new XmlWriter(writer, tagDescription);
        xmlWriter.setAlwaysAddNamespace(true);
        xmlWriter.writeXmlDeclaration("UTF-8");

        final AttributeList rootAttributes = new AttributeList();
        rootAttributes.addNamespaceDeclaration("manifest", MANIFEST_NS);
        xmlWriter.writeTag(MANIFEST_NS, "manifest", rootAttributes, XmlWriterSupport.OPEN);

        final Iterator<Map.Entry<String,String>> iterator = entries.entrySet().iterator();
        while (iterator.hasNext())
        {
            final Map.Entry<String,String> entry = iterator.next();
            final AttributeList entryAttrs = new AttributeList();
            entryAttrs.setAttribute(MANIFEST_NS, "media-type", entry.getValue());
            entryAttrs.setAttribute(MANIFEST_NS, "full-path", entry.getKey());
            xmlWriter.writeTag(MANIFEST_NS, "file-entry", entryAttrs, XmlWriterSupport.CLOSE);
        }

        xmlWriter.writeCloseTag();
        xmlWriter.close();
    }
}
