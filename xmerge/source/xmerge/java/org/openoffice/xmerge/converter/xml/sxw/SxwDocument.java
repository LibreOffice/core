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

package org.openoffice.xmerge.converter.xml.sxw;

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

