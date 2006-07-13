/************************************************************************
 *
 *  PluginFactoryImpl.java
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
 *  Copyright: 2002-2004 by Henrik Just
 *
 *  All Rights Reserved.
 *
 * version 0.3.3f (2004-08-28)
 *
 */


package writer2latex.latex;

import java.io.InputStream;
import java.io.IOException;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.DocumentSerializerFactory;

import org.openoffice.xmerge.converter.xml.sxw.SxwPluginFactory;

import org.openoffice.xmerge.util.registry.ConverterInfo;


/**
 * Factory class used to create converters to LaTeX 2e format.
 */
public final class PluginFactoryImpl extends SxwPluginFactory
    implements  DocumentSerializerFactory {

   /**
    *  <p>Constructor that caches the <code>ConvertInfo</code> that
    *     corresponds to the registry information for this plug-in.</p>
    *
    *  @param  ci  <code>ConvertInfo</code> object.
    */
    public PluginFactoryImpl (ConverterInfo ci) {
        super(ci);
    }

    /**
     *  <p>The <code>DocumentSerializer</code> is used to convert
     *  from the OpenOffice Writer <code>Document</code> format
     *  to LaTeX <code>Document</code> format.</p>
     *
     *  <p>The <code>Document</code> object is passed along to the
     *     created <code>DocumentSerializer</code> via its constructor.
     *     The <code>Document</code> is read and converted when the
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
     *  <p>Create a <code>Document</code> object that corresponds to
     *  the LaTeX data passed in via the <code>InputStream</code>
     *  object.
     *
     *  <p>This method will read from the given <code>InputStream</code>
     *  object.  The returned <code>Document</code> object will contain
     *  the necessary data for the other objects created by the
     *  <code>PluginFactoryImpl</code> to process, like the
     *  <code>DocumentSerializerImpl</code> object and a
     *  <code>DocumentMerger</code> object.</p>
     *  <p>Currently this method doesn't read anything, since it it not used.</p>
     *
     *  @param  name  The <code>Document</code> name.
     *  @param  is    <code>InputStream</code> object corresponding
     *                to the <code>Document</code>.
     *
     *  @return  A <code>Document</code> object representing the
     *           LaTeX format.
     *
     *  @throws   IOException   If any I/O error occurs.
     */

    public Document createDeviceDocument(String name, InputStream is)
            throws IOException {
        LaTeXDocument texDoc = new LaTeXDocument(name);
        texDoc.read(is); // this has no effect currently
        return texDoc;
    }

}