/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.xmerge.converter.xml.sxw;

import java.io.InputStream;
import java.io.IOException;

import org.openoffice.xmerge.PluginFactory;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.util.registry.ConverterInfo;

/**
 *  General implementation of the <code>PluginFactory</code> interface
 *  for SXW documents.
 *
 *  @see  org.openoffice.xmerge.DocumentDeserializer
 *  @see  org.openoffice.xmerge.DocumentMerger
 *  @see  org.openoffice.xmerge.DocumentSerializer
 */
public abstract class SxwPluginFactory extends PluginFactory {

   /**
    *  Constructor that caches the <code>ConvertInfo</code> that
    *  corresponds to the registry information for this plug-in.
    *
    *  @param  ci  <code>ConvertInfo</code> object.
    */
    public SxwPluginFactory (ConverterInfo ci) {
        super(ci);
    }


    public Document createOfficeDocument(String name, InputStream is)
        throws IOException {

        // read zipped XML stream
        SxwDocument doc = new SxwDocument(name);
        doc.read(is);
        return doc;
    }

     public Document createOfficeDocument(String name, InputStream is,boolean isZip)
        throws IOException {

        // read XML stream
        SxwDocument doc = new SxwDocument(name);
        doc.read(is,isZip);
        return doc;
    }
}

