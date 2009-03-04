/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ManifestWriter.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.report.util;

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.io.OutputStreamWriter;
import java.io.IOException;
import java.io.OutputStream;

import org.pentaho.reporting.libraries.xmlns.writer.XmlWriterSupport;
import org.pentaho.reporting.libraries.xmlns.writer.DefaultTagDescription;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriter;
import org.pentaho.reporting.libraries.xmlns.common.AttributeList;
import org.jfree.report.JFreeReportBoot;
import com.sun.star.report.OutputRepository;

/**
 * Creation-Date: 24.07.2007, 18:16:52
 *
 * @author Thomas Morgner
 */
public class ManifestWriter
{
    // need this two strings other it breaks the ooo build :-(
  public static final String MANIFEST_NS = "urn:oasis:names:tc:opendocument:xmlns:manifest:1.0";
  public static final String TAG_DEF_PREFIX = "com.sun.star.report.pentaho.output.";
  private final Map entries;

  public ManifestWriter()
  {
    entries = new HashMap();
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

    final Iterator iterator = entries.entrySet().iterator();
    while (iterator.hasNext())
    {
      final Map.Entry entry = (Map.Entry) iterator.next();
      final AttributeList entryAttrs = new AttributeList();
      entryAttrs.setAttribute(MANIFEST_NS, "media-type", (String) entry.getValue());
      entryAttrs.setAttribute(MANIFEST_NS, "full-path", (String) entry.getKey());
      xmlWriter.writeTag(MANIFEST_NS, "file-entry", entryAttrs, XmlWriterSupport.CLOSE);
    }

    xmlWriter.writeCloseTag();
    xmlWriter.close();
  }
}
