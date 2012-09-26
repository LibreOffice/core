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

package org.openoffice.xmerge.converter.xml.sxw.pocketword;


import java.io.InputStream;
import java.io.IOException;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentDeserializer;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.DocumentDeserializerFactory;
import org.openoffice.xmerge.DocumentSerializerFactory;
import org.openoffice.xmerge.DocumentMerger;
import org.openoffice.xmerge.DocumentMergerFactory;
import org.openoffice.xmerge.ConverterCapabilities;

import org.openoffice.xmerge.util.registry.ConverterInfo;

import org.openoffice.xmerge.converter.xml.sxw.SxwPluginFactory;


/**
 * Factory class used to create converters to/from the Pocket Word format.
 *
 * @version 1.1
 */
public final class PluginFactoryImpl extends SxwPluginFactory
    implements DocumentDeserializerFactory, DocumentSerializerFactory,
               DocumentMergerFactory{

   /**
    *  <p>Constructor that caches the <code>ConvertInfo</code> that
    *     corresponds to the registry information for this plug-in.</p>
    *
    *  @param  ci  <code>ConvertInfo</code> object.
    */
    public PluginFactoryImpl (ConverterInfo ci) {
        super(ci);
    }

    /** ConverterCapabilities object for this type of conversion. */
    private final static ConverterCapabilities converterCap =
        new ConverterCapabilitiesImpl();


    /**
     *  <p>The <code>DocumentSerializer</code> is used to convert
     *  from the OpenOffice Writer <code>Document</code> format
     *  to the Pocket Word <code>Document</code> format.</p>
     *
     *  <p>The <code>ConvertData</code> object is passed along to the
     *     created <code>DocumentSerializer</code> via its constructor.
     *     The <code>ConvertData</code> is read and converted when the
     *     the <code>DocumentSerializer</code> object's
     *     <code>serialize</code> method is called.</p>
     *
     *  @param  doc  <code>Document</code> object that the created
     *               <code>DocumentSerializer</code> object uses
     *               as input.
     *
     *  @return  A <code>DocumentSerializer</code> object.
     */
    public DocumentSerializer createDocumentSerializer(Document doc) {
        return new DocumentSerializerImpl(doc);
    }


    /**
     *  The <code>DocumentDeserializer</code> is used to convert
     *  from the Pocket Word <code>Document</code> format to
     *  the OpenOffice Writer <code>Document</code> format.</p>
     *
     *  The <code>ConvertData</code> object is passed along to the
     *  created <code>DocumentDeserializer</code> via its constructor.
     *  The <code>ConvertData</code> is read and converted when the
     *  the <code>DocumentDeserializer</code> object's
     *  <code>deserialize</code> method is called.
     *  </p>
     *
     *  @param  cd  <code>ConvertData</code> object that the created
     *              <code>DocumentDeserializer</code> object uses as
     *              input.
     *
     *  @return  A <code>DocumentDeserializer</code> object.
     */
    public DocumentDeserializer createDocumentDeserializer(ConvertData cd) {
        return new DocumentDeserializerImpl(cd);
    }


    /**
     *  <p>Create a <code>Document</code> object that corresponds to
     *  the Pocket Word data passed in via the <code>InputStream</code>
     *  object.
     *
     *  <p>This method will read from the given <code>InputStream</code>
     *  object.  The returned <code>Document</code> object will contain
     *  the necessary data for the other objects created by the
     *  <code>PluginFactoryImpl</code> to process, like the
     *  <code>DocumentSerializerImpl</code> object and a
     *  <code>DocumentMerger</code> object.</p>
     *
     *  @param  name  The <code>Document</code> name.
     *  @param  is    <code>InputStream</code> object corresponding
     *                to the <code>Document</code>.
     *
     *  @return  A <code>Document</code> object representing the
     *           Pocket Word format.
     *
     *  @throws   IOException   If any I/O error occurs.
     */

    public Document createDeviceDocument(String name, InputStream is)
            throws IOException {
        PocketWordDocument pwd = new PocketWordDocument(name);
        pwd.read(is);
        return pwd;
    }

     /**
     *  Returns an instance of <code>DocumentMergerImpl</code>,
     *  which is an implementation of the <code>DocumentMerger</code>
     *  interface.
     *
     *  @param  doc  <code>Document</code> to merge.
     *
     *  @return  A DocumentMergerImpl object.
     */
    public DocumentMerger createDocumentMerger(Document doc) {
    ConverterCapabilities cc = converterCap;
        DocumentMergerImpl merger = new DocumentMergerImpl(doc, cc);
        return merger;

    }

}
