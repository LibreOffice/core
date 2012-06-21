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

package org.openoffice.xmerge.converter.xml.sxc.pexcel;

import java.io.IOException;
import java.io.InputStream;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.DocumentSerializerFactory;
import org.openoffice.xmerge.DocumentDeserializer;
import org.openoffice.xmerge.DocumentDeserializerFactory;
import org.openoffice.xmerge.DocumentMerger;
import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.sxc.DocumentMergerImpl;
import org.openoffice.xmerge.converter.xml.sxc.SxcPluginFactory;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Workbook;
import org.openoffice.xmerge.util.registry.ConverterInfo;

/**
 *  <p>Pocket Excel implementation of the <code>PluginFactory</code>.
 *  This encapsulates conversion of StarCalc XML format to and from
 *  Pocket Excel format.</p>
 *
 *  <p>The superclass produces a particular
 *  {@link org.openoffice.xmerge.Document Document}
 *  object, i.e. {@link
 *  org.openoffice.xmerge.converter.xml.sxc.SxcDocument
 *  SxcDocument} that the converters in this class works with.  Thus,
 *  this class only implements the methods that produces the converters,
 *  i.e. {@link
 *  org.openoffice.xmerge.DocumentSerializer
 *  DocumentSerializer} and {@link
 *  org.openoffice.xmerge.DocumentDeserializer
 *  DocumentDeserializer};
 *  as well as the {@link
 *  org.openoffice.xmerge.ConverterCapabilities
 *  ConverterCapabilities} object that is specific to this format
 *  conversion.  That superclass also produces a {@link
 *  org.openoffice.xmerge.DocumentMerger DocumentMerger}
 *  object, i.e. {@link
 *  org.openoffice.xmerge.converter.xml.sxc.DocumentMergerImpl
 *  DocumentMergerImpl} which this class derives the functionality.</p>
 */
public final class PluginFactoryImpl extends SxcPluginFactory
    implements DocumentDeserializerFactory, DocumentSerializerFactory {

    /** ConverterCapabilities object for this type of conversion. */
    private final static ConverterCapabilities converterCap =
        new ConverterCapabilitiesImpl();


    public PluginFactoryImpl(ConverterInfo ci) {
        super(ci);
    }


    /**
     *  Returns an instance of <code>DocumentSerializerImpl</code>,
     *  which is an implementation of <code>DocumentSerializer</code>
     *  interface.
     *
     *  @param  doc  <code>Document</code> object to be
     *               converted/serialized.
     *
     *  @return  A <code>DocumentSerializerImpl</code> object.
     */
    public DocumentSerializer createDocumentSerializer(Document doc) {

        return new SxcDocumentSerializerImpl(doc);
    }


    /**
     *  Returns an instance of <code>DocumentDeserializerImpl</code>,
     *  which is an implementation of <code>DocumentDeserializer</code>
     *  interface.
     *
     *  @param  cd  <code>ConvertData</code> object for reading data
     *              which will be converted back to a
     *              <code>Document</code> object.
     *
     *  @return  A <code>DocumentDeserializerImpl</code> object.
     */
    public DocumentDeserializer createDocumentDeserializer(ConvertData cd) {

        return new SxcDocumentDeserializerImpl(cd);
    }


    public Document createDeviceDocument(String name, InputStream is)
    throws IOException {

        Workbook wb = new Workbook(name, is);
        return wb;
    }

    public DocumentMerger createDocumentMerger(Document doc) {

        DocumentMergerImpl merger = new DocumentMergerImpl(doc, converterCap);
        return merger;
    }
}

