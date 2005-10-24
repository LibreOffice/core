/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml.xslt;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.DocumentSerializerFactory;
import org.openoffice.xmerge.DocumentDeserializer;
import org.openoffice.xmerge.DocumentDeserializerFactory;
import org.openoffice.xmerge.PluginFactory;
import org.openoffice.xmerge.converter.dom.DOMDocument;
//import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;
//import org.openoffice.xmerge.converter.xml.OfficeDocument;
import org.openoffice.xmerge.converter.xml.xslt.GenericOfficeDocument;
import org.openoffice.xmerge.util.registry.ConverterInfo;
import org.openoffice.xmerge.DocumentMerger;
import org.openoffice.xmerge.DocumentMergerFactory;
import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.util.registry.ConverterInfo;

import java.io.InputStream;
import java.util.Enumeration;
import java.io.InputStream;
import java.io.IOException;
import java.util.Properties;

/**
 *  <p>Xslt implementation of the <code>PluginFactory</code>.
 *  This encapsulates conversion of StarWriter XML format to and from
 *  a supported format.</p>
 *
 *  <p>The superclass produces a particular
 *  {@link org.openoffice.xmerge.Document Document}
 *  object, i.e. {@link
 *  org.openoffice.xmerge.converter.xml.sxw.SxwDocument
 *  SxwDocument} that the converters in this class work with.   Thus,
 *  this class only implements the methods that produces the converters,
 *  i.e. {@link
 *  org.openoffice.xmerge.DocumentSerializer
 *  DocumentSerializer} and {@link
 *  org.openoffice.xmerge.DocumentDeserializer
 *  DocumentDeserializer}</p>
 *
 *  @author   Aidan Butler
 */
public final class PluginFactoryImpl extends PluginFactory
    implements DocumentDeserializerFactory, DocumentSerializerFactory, DocumentMergerFactory
{

    public PluginFactoryImpl (ConverterInfo ci) {
           super(ci);
    }

    /** ConverterCapabilities object for this type of conversion. */
    private final static ConverterCapabilities converterCap =
        new ConverterCapabilitiesImpl();


    /**
     *  Returns an instance of <code>DocumentSerializerImpl</code>,
     *  which is an implementation of the <code>DocumentSerializer</code>
     *  interface.
     *
     *  @param  doc  <code>Document</code> object to be
     *               converted/serialized.
     *
     *  @return  A <code>DocumentSerializerImpl</code> object.
     */
    public DocumentSerializer createDocumentSerializer(Document doc) {
        return new DocumentSerializerImpl(this,doc);
    }


    /**
     *  Returns an instance of <code>DocumentDeserializerImpl</code>,
     *  which is an implementation of the <code>DocumentDeserializer</code>
     *  interface.
     *
     *  @param  is   <code>ConvertData</code> object.
     *
     *  @return  A DocumentDeserializerImpl object.
     */
    public DocumentDeserializer createDocumentDeserializer(ConvertData cd) {

        return new DocumentDeserializerImpl(this,cd);
    }

     public org.openoffice.xmerge.Document createDeviceDocument(java.lang.String str, java.io.InputStream inputStream) throws java.io.IOException {
        String ext = this.getDeviceFileExtension();
        DOMDocument domDoc = new DOMDocument(str,ext);
        domDoc.read(inputStream);
        return domDoc;
    }


     public Document createOfficeDocument(String name, InputStream is)
        throws IOException {

        // read zipped XML stream
        GenericOfficeDocument doc = new GenericOfficeDocument(name);
        doc.read(is);
        return doc;
    }

     public Document createOfficeDocument(String name, InputStream is,boolean isZip)
        throws IOException {

        // read zipped XML stream
        GenericOfficeDocument doc = new GenericOfficeDocument(name);
        doc.read(is,isZip);
        return doc;
    }

    /**
     *  Returns a <code>String</code> containing the file extension of a
     *  <code>Document</code>. This method uses a properties file to determine
     *  a mapping from the device mime in the <code>ConverterInfo</code> to a
     *  particular file extension. If a mapping is not specified, the default
     *  is ".txt".
     *
     *  @return  <code>String</code>.
     */


    public String getDeviceFileExtension(){
    Class c = this.getClass();
    InputStream is = c.getResourceAsStream("XsltPlugin.properties");
    Properties props = new Properties();
    String ext= ".txt";
    String mimeType = null;
    ConverterInfo ci = this.getConverterInfo();
    Enumeration enum = ci.getDeviceMime();
    while (enum.hasMoreElements()) {
        mimeType= (String) enum.nextElement();
    }
    try {
        props.load(is);

         String info = props.getProperty(mimeType);
         if (info != null) {
             ext = info;
         }
    } catch (Exception e) {

        // It is okay for the property file to not exist.
        //
    }
    return ext;
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











