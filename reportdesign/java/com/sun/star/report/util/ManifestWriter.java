/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ManifestWriter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 09:52:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
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

package com.sun.star.report.util;

import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.io.OutputStreamWriter;
import java.io.IOException;
import java.io.OutputStream;

import org.jfree.xmlns.writer.XmlWriterSupport;
import org.jfree.xmlns.writer.DefaultTagDescription;
import org.jfree.xmlns.writer.XmlWriter;
import org.jfree.xmlns.common.AttributeList;
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
  private HashMap entries;

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
