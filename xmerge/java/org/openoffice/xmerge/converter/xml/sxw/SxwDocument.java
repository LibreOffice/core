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

package org.openoffice.xmerge.converter.xml.sxw;

import org.w3c.dom.Document;
import org.openoffice.xmerge.converter.xml.OfficeDocument;
import org.openoffice.xmerge.converter.xml.OfficeConstants;

/**
 *  This class is an implementation of <code>OfficeDocument</code> for
 *  the SXW format.
 */
public class SxwDocument extends OfficeDocument {


    /**
     *  Constructor with arguments to set <code>name</code>.
     *
     *  @param  name  The name of the <code>Document</code>
     */
    public SxwDocument(String name) {
        super(name);
    }


    /**
     *  Constructor with arguments to set <code>name</code>, the
     *  <code>namespaceAware</code> flag, and the <code>validating</code>
     *  flag.
     *
     *  @param  name            The name of the <code>Document</code>.
     *  @param  namespaceAware  The value of the namespaceAware flag.
     *  @param  validating      The value of the validating flag.
     */
    public SxwDocument(String name, boolean namespaceAware, boolean validating) {

        super(name, namespaceAware, validating);
    }


    /**
     *  Returns the Office file extension for the SXW format.
     *
     *  @return  The Office file extension for the SXW format.
     */
    protected String getFileExtension() {
        return OfficeConstants.SXW_FILE_EXTENSION;
    }


    /**
     *  Returns the Office attribute for the SXW format.
     *
     *  @return  The Office attribute for the SXW format.
     */
    protected String getOfficeClassAttribute() {
        return OfficeConstants.SXW_TYPE;
    }

    /**
     * Method to return the MIME type of the document.
     *
     * @return  String  The document's MIME type.
     */
    protected final String getDocumentMimeType() {
        return OfficeConstants.SXW_MIME_TYPE;
    }

}

