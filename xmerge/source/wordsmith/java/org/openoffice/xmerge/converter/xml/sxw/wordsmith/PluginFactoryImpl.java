/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PluginFactoryImpl.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:14:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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

package org.openoffice.xmerge.converter.xml.sxw.wordsmith;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.DocumentMerger;
import org.openoffice.xmerge.DocumentMergerFactory;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.DocumentSerializerFactory;
import org.openoffice.xmerge.DocumentDeserializer;
import org.openoffice.xmerge.DocumentDeserializerFactory;
import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.sxw.SxwPluginFactory;
import org.openoffice.xmerge.converter.palm.PalmDocument;
import org.openoffice.xmerge.util.registry.ConverterInfo;

import java.io.InputStream;
import java.io.IOException;


/**
 *  <p>WordSmith implementation of a <code>PluginFactory</code> that
 *  encapsulates conversion of StarWriter XML format to and from
 *  WordSmith format.</p>
 *
 *  The superclass produces a particular
 *  {@link org.openoffice.xmerge.Document Document}
 *  object, i.e.
 *  {@link org.openoffice.xmerge.converter.xml.sxw.SxwDocument
 *  SxwDocument} that the converters in this class works with.  Thus,
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
 *  org.openoffice.xmerge.converter.xml.sxw.wordsmith.DocumentMergerImpl
 *  DocumentMergerImpl} which this class derives the functionality.</p>
 *
 *  @author   Herbie Ong, Dave Proulx
 */
public final class PluginFactoryImpl extends SxwPluginFactory
    implements DocumentDeserializerFactory, DocumentSerializerFactory,
    DocumentMergerFactory {

    public PluginFactoryImpl(ConverterInfo ci) {
        super(ci);
    }

    /** ConverterCapabilities object for this type of conversion. */
    private final static ConverterCapabilities converterCap =
        new ConverterCapabilitiesImpl();


    /**
     *  Returns an instance of <code>DocumentSerializerImpl</code>, which is
     *  an implementation of <code>DocumentSerializer</code> interface.
     *
     *  @param  doc  <code>Document</code> object to be converted/serialized.
     *
     *  @return  A <code>DocumentSerializerImpl</code> object.
     */
    public DocumentSerializer createDocumentSerializer(Document doc) {

        return new DocumentSerializerImpl(doc);
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

        return new DocumentDeserializerImpl(cd);
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

    /**
     *  Returns an instance of the DeviceDocument
     *  which is an implementation of the <code>DocumentMerger</code>
     *  interface.
     *
     *  @param  doc  <code>Document</code> to merge.
     *
     *  @return  A Device Document object
     */
    public Document createDeviceDocument(String name, InputStream is)
    throws IOException {

        PalmDocument palmDoc = new PalmDocument(is);
        return palmDoc;
    }
}

