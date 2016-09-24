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

package org.openoffice.xmerge.converter.xml.xslt;

import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;
import java.util.Properties;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentDeserializer;
import org.openoffice.xmerge.DocumentDeserializerFactory;
import org.openoffice.xmerge.DocumentMerger;
import org.openoffice.xmerge.DocumentMergerFactory;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.DocumentSerializerFactory;
import org.openoffice.xmerge.PluginFactory;
import org.openoffice.xmerge.converter.dom.DOMDocument;
import org.openoffice.xmerge.util.registry.ConverterInfo;

/**
 * Xslt implementation of the {@code PluginFactory}.
 *
 * <p>This encapsulates conversion of StarWriter XML format to and from a
 * supported format.</p>
 *
 * <p>The superclass produces a particular {@link org.openoffice.xmerge.Document
 * Document} object, i.e. {@link
 * org.openoffice.xmerge.converter.xml.sxw.SxwDocument xwDocument} that the
 * converters in this class work with. Thus, this class only implements the
 * methods that produces the converters, i.e. {@link
 * org.openoffice.xmerge.DocumentSerializer DocumentSerializer} and {@link
 * org.openoffice.xmerge.DocumentDeserializer DocumentDeserializer}</p>
 */
public final class PluginFactoryImpl extends PluginFactory
    implements DocumentDeserializerFactory, DocumentSerializerFactory, DocumentMergerFactory
{

    public PluginFactoryImpl (ConverterInfo ci) {
           super(ci);
    }

    /** ConverterCapabilities object for this type of conversion. */
    private static final ConverterCapabilities converterCap =
        new ConverterCapabilitiesImpl();

    /**
     * Returns an instance of {@code DocumentSerializerImpl}, which is an
     * implementation of the {@code DocumentSerializer} interface.
     *
     * @param   doc  {@code Document} object to be converted/serialized.
     *
     * @return  A {@code DocumentSerializerImpl} object.
     */
    public DocumentSerializer createDocumentSerializer(Document doc) {
        return new DocumentSerializerImpl(this,doc);
    }

    /**
     * Returns an instance of <code>DocumentDeserializerImpl</code>,
     * which is an implementation of the <code>DocumentDeserializer</code>
     * interface.
     *
     * @param   cd  {@code ConvertData} object.
     *
     * @return  A {@code DocumentDeserializerImpl} object.
     */
    public DocumentDeserializer createDocumentDeserializer(ConvertData cd) {
        return new DocumentDeserializerImpl(this,cd);
    }

    @Override
    public org.openoffice.xmerge.Document createDeviceDocument(String str,
            java.io.InputStream inputStream) throws java.io.IOException {
        String ext = this.getDeviceFileExtension();
        DOMDocument domDoc = new DOMDocument(str,ext);
        domDoc.read(inputStream);
        return domDoc;
    }

    @Override
    public Document createOfficeDocument(String name, InputStream is)
        throws IOException {

        // read zipped XML stream
        GenericOfficeDocument doc = new GenericOfficeDocument(name);
        doc.read(is);
        return doc;
    }

    @Override
    public Document createOfficeDocument(String name, InputStream is, boolean isZip)
        throws IOException {

        // read zipped XML stream
        GenericOfficeDocument doc = new GenericOfficeDocument(name);
        doc.read(is,isZip);
        return doc;
    }

    /**
     * Returns a {@code String} containing the file extension of a
     * {@code Document}.
     *
     * <p>This method uses a properties file to determine a mapping from the
     * device mime in the {@code ConverterInfo} to a particular file extension.
     * If a mapping is not specified, the default is ".txt".</p>
     *
     * @return  The file extension of a {@code Document}.
     */
    private String getDeviceFileExtension() {
        Class<? extends PluginFactoryImpl> c = this.getClass();
        InputStream is = c.getResourceAsStream("XsltPlugin.properties");
        Properties props = new Properties();
        String ext = ".txt";
        String mimeType = null;
        ConverterInfo ci = this.getConverterInfo();
        Iterator<String> enumerate = ci.getDeviceMime();
        while (enumerate.hasNext()) {
            mimeType = enumerate.next();
        }
        try {
            props.load(is);

            String info = mimeType != null ? props.getProperty(mimeType) : null;
            if (info != null) {
                ext = info;
            }
        } catch (Exception e) {

        // It is okay for the property file to not exist.
        } finally {
            try {
                if (is != null) {
                    is.close();
                }
            } catch (IOException ex) {
            }
        }
        return ext;
    }

    /**
     * Returns an instance of {@code DocumentMergerImpl}, which is an
     * implementation of the {@code DocumentMerger} interface.
     *
     * @param   doc  {@code Document} to merge.
     *
     * @return  A {@code DocumentMergerImpl} object.
     */
    public DocumentMerger createDocumentMerger(Document doc) {
        ConverterCapabilities cc = converterCap;
        DocumentMergerImpl merger = new DocumentMergerImpl(doc, cc);
        return merger;
    }
}
