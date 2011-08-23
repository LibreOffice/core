/*************************************************************************
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

