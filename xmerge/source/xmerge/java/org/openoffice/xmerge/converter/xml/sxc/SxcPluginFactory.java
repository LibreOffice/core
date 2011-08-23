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
        //
        SxcDocument doc = new SxcDocument(name);
        doc.read(is);
        return doc;
    }

     public Document createOfficeDocument(String name, InputStream is,boolean isZip)
        throws IOException {

        // read zipped XML stream
        //
        SxcDocument doc = new SxcDocument(name);
        doc.read(is,isZip);
        return doc;
    }
}

