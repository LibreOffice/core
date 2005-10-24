/************************************************************************
 *
 *  PluginFactoryImpl.java
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
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


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
 * @author  Mark Murnane
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
