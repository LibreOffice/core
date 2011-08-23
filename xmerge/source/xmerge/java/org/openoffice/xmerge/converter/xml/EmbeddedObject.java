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

package org.openoffice.xmerge.converter.xml;

import java.io.IOException;

import org.w3c.dom.Document;
import org.w3c.dom.DOMException;


public abstract class EmbeddedObject {
    protected String objName;
    protected String objType;
    
    /** Representation of the file from which this object was read. */
    protected OfficeZip zipFile = null;
    
    /** Flag indicating if this document has changed since reading or is new. */
    protected boolean hasChanged = false;
    
    /**
     * Constructor for an embedded object stored using an XML representation.
     *
     * @param   name    The name of the object.
     * @param   type    The mime-type of the object.  See the class summary.
     */
    public EmbeddedObject(String name, String type) {
        objName = name;
        objType = type;
        
        hasChanged = true;
    }

    
    /**
     * Package private constructor for use when reading an object from a 
     * compressed SX? file.
     *
     * @param   name    The name of the object.
     * @param   type    The mime-type of the object.  See the class summary.
     * @param   source  The OfficeZip representation of the SX? file that stores
     *                  the object.
     */
    EmbeddedObject(String name, String type, OfficeZip source) {
        this(name, type);
        zipFile = source;
    }

    
    /**
     * Retrieves the name of the embedded object represented by an instance of
     * this class.
     *
     * <b>N.B.</b>The name referes to the name as found in the 
     * <code>META-INF/manifest.xml</code> file.
     *
     * @return  The name of the object.
     */
    public final String getName() {
        return objName;
    }
    
    
    /** 
     * Retrieves the type of the embedded object represented by an instance of
     * this class.
     *
     * The <code>META-INF/manifest.xml</code> file currently represents the 
     * type of an object using MIME types.
     */
    public final String getType() {
        return objType;
    }
    
    /**
     * Package private method for writing the data of the EmbeddedObject to a
     * SX? file.
     *
     * @param   zip     An <code>OfficeZip</code> instance representing the file
     *                  the data is to be written to.
     */
    abstract void write(OfficeZip zip) throws IOException;
    
    /**
     * Package private method that constructs the manifest.xml entries for this
     * embedded object.
     *
     * @return  Document    <code>Document</code> containing the manifest entries.
     */
    abstract void writeManifestData(Document manifestDoc) throws DOMException;
}