/************************************************************************
 *
 *  SxiDocument.java
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
 *  Version 0.3.3g (2004-10-19)
 *
 *
 ************************************************************************/

package writer2latex.xmerge;

import org.w3c.dom.Document;
import org.openoffice.xmerge.converter.xml.OfficeDocument;

import writer2latex.office.MIMETypes;

/**
 *  This class is an implementation of <code>OfficeDocument</code> for
 *  the SXI format.
 *  (The implementation is based on the class
 *  org.openoffice.xmerge.converter.xml.sxc.SxcDocument.)
 */
public class SxiDocument extends OfficeDocument {

    /**
     *  Constructor with arguments to set <code>name</code>.
     *
     *  @param  name  The name of the <code>Document</code>
     */
    public SxiDocument(String name) {
        super(name);
    }


    /**
     *  Constructor with arguments to set <code>name</code>, the
     *  <code>namespaceAware</code> flag, and the <code>validating</code>
     *  flag.
     *
     *  @param  name            The name of the <code>Document</code>.
     *  @param  namespaceAware  The value of the <code>namespaceAware</code>
     *                          flag.
     *  @param  validating      The value of the <code>validating</code> flag.
     */
    public SxiDocument(String name, boolean namespaceAware, boolean validating) {

        super(name, namespaceAware, validating);
    }

    /**
     *  Returns the Office file extension for the SXI format.
     *
     *  @return  The Office file extension for the SXI format.
     */
    protected String getFileExtension() {
        return ".sxi";
    }

    /**
     *  Returns the Office attribute for the SXC format.
     *
     *  @return  The Office attribute for the SXC format.
     */
    protected String getOfficeClassAttribute() {
        return "presentation";
    }

    /**
     * Method to return the MIME type of the document.
     *
     * @return  String  The document's MIME type.
     */
    protected final String getDocumentMimeType() {
        return MIMETypes.IMPRESS;
    }

}