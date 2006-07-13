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
 *  Copyright: 2001-2002 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  version 0.3.3f (2004-09-22)
 *
 */

package writer2latex.latex;

import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentSerializer;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;

import java.io.IOException;

/**
 * <p>LaTeX implementation of <code>DocumentSerializer</code></p>
 *
 * <p>This converts an OpenOffice Writer XML files to a LaTeX file<.</p>
 *
 */
public final class DocumentSerializerImpl
            implements DocumentSerializer {

    private SxwDocument   sxwDoc;

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
    }


    /**
     *  <p>Convert the data passed into the <code>DocumentSerializerImpl</code>
     *  constructor into LaTeX format.</p>
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
        // The actual conversion is done elsewhere, this file only contains the
        // xmerge plugin logic
        ConverterPalette converter = new ConverterPalette(sxwDoc,null);
        return converter.convert();
    }

}