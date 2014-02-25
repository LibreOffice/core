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

package org.openoffice.xmerge.converter.xml.sxc;

import java.io.InputStream;
import java.io.IOException;

import org.openoffice.xmerge.util.registry.ConverterInfo;
import org.openoffice.xmerge.PluginFactory;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentMergerFactory;


/**
 *  General implementation of the <code>PluginFactory</code> interface
 *  for SXC <code>Document</code> objects.
 *
 *  @see  org.openoffice.xmerge.DocumentDeserializer
 *  @see  org.openoffice.xmerge.DocumentMerger
 *  @see  org.openoffice.xmerge.DocumentSerializer
 */
public abstract class SxcPluginFactory
    extends PluginFactory implements DocumentMergerFactory {


   /**
    *  Constructor that caches the <code>ConvertInfo</code> that
    *  corresponds to the registry information for this plug-in.
    *
    *  @param  ci  <code>ConvertInfo</code> object.
    */
    public SxcPluginFactory(ConverterInfo ci) {
        super(ci);
    }


    public Document createOfficeDocument(String name, InputStream is)
        throws IOException {

        // read zipped XML stream

        SxcDocument doc = new SxcDocument(name);
        doc.read(is);
        return doc;
    }

     public Document createOfficeDocument(String name, InputStream is,boolean isZip)
        throws IOException {

        // read zipped XML stream

        SxcDocument doc = new SxcDocument(name);
        doc.read(is,isZip);
        return doc;
    }
}

