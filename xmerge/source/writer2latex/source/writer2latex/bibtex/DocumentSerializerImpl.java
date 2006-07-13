/************************************************************************
 *
 *  DocumentSerializerImpl.java
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
 *  Copyright: 2001-2004 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  version 0.3.3g (2004-11-30)
 *
 */

package writer2latex.bibtex;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;

import java.io.IOException;

import org.w3c.dom.NodeList;

import writer2latex.office.*;
import writer2latex.util.*;

/**
 * <p>BibTeX implementation of <code>DocumentSerializer</code>
 * for use by {@link
 * writer2latex.bibtex.PluginFactoryImpl
 * PluginFactoryImpl}.</p>
 *
 * <p>This converts an OpenOffice Writer XML files to a BibTeX file<.</p>
 *
 */
public final class DocumentSerializerImpl
            implements DocumentSerializer {

    private SxwDocument   sxwDoc;
    private String sOutFileName;

    public void setOutFileName(String s) {
        sOutFileName = Misc.trimDocumentName(s,".bib");
    }

    /**
     *  <p>Initialises a new <code>DocumentSerializerImpl</code> using the.<br>
     *     supplied <code>Document</code></p>
     *
     * <p>The supplied document should be an {@link
     *    org.openoffice.xmerge.converter.xml.sxw.SxwDocument SxwDocument}
     *    object.</p>
     *
     *  @param  document  The <code>Document</code> to convert.
     */
    public DocumentSerializerImpl(Document doc) {
        sxwDoc = (SxwDocument) doc;
        sOutFileName = sxwDoc.getName();
    }


    /**
     *  <p>Convert the data passed into the <code>DocumentSerializerImpl</code>
     *  constructor into BibTeX format.</p>
     *
     *  <p>This method may or may not be thread-safe.  It is expected
     *  that the user code does not call this method in more than one
     *  thread.  And for most cases, this method is only done once.</p>
     *
     *  @return  <code>ConvertData</code> object to pass back the
     *           converted data.
     *
     *  @throws  ConvertException  If any conversion error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public ConvertData serialize() throws IOException, ConvertException {
        ConvertData convertData = new ConvertData();
        BibTeXDocument bibDoc = new BibTeXDocument(sOutFileName);

        // Collect all text:bibliography-mark elements from content.xml
        org.w3c.dom.Document doc = sxwDoc.getContentDOM();
        NodeList list;
        list = doc.getElementsByTagName(XMLString.TEXT_BIBLIOGRAPHY_MARK);
        int nLen = list.getLength();
        for (int i=0; i<nLen; i++) {
            String sIdentifier = Misc.getAttribute(list.item(i),XMLString.TEXT_IDENTIFIER);
            if (!bibDoc.containsKey(sIdentifier)) {
                bibDoc.put(new BibMark(list.item(i)));
            }
        }

        // Return the converted data
        convertData.addDocument(bibDoc);
        return convertData;
    }

}