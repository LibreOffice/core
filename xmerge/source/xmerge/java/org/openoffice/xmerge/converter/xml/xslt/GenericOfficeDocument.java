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

package org.openoffice.xmerge.converter.xml.xslt;

import org.openoffice.xmerge.converter.xml.OfficeDocument;

/**
 *  This class is an implementation of <code>OfficeDocument</code> for 
 *  the generic office format.
 */
public class GenericOfficeDocument extends OfficeDocument {

    /**
     *  Constructor with arguments to set <code>name</code>.
     *
     *  @param  name  The name of the <code>Document</code>
     */
    public GenericOfficeDocument(String name) {
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
    public GenericOfficeDocument(String name, boolean namespaceAware, boolean validating) {

        super(name, namespaceAware, validating);
    }

    /**
     *  Returns the Office file extension for the  generic format.
     *
     *  @return  The Office file extension for the generic format.
     */
    protected String getFileExtension() {
        return "";
    }

    /**
     *  Returns the Office attribute for the generic format.
     *
     *  @return  The Office attribute for the generic format.
     */
    protected String getOfficeClassAttribute() {
        
    return "";
    }
    
    /**
     * Method to return the MIME type of the document.
     *
     * @return  String  The document's MIME type.
     */
    protected String getDocumentMimeType() {
        /* TODO: Determine the MIME-type from the input. */
        return "";
    }
    
}

