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

import org.w3c.dom.Document;
import org.w3c.dom.DOMException;
import org.w3c.dom.Element;


/**
 * This class represents embedded object's in an OpenOffice.org document that
 * have a binary representation.
 */
public class EmbeddedBinaryObject extends EmbeddedObject {
    
    /** The object's binary representation. */
    protected byte[] objData = null;
    
    /**
     * Constructor for an embedded object stored using an XML representation.
     *
     * @param   name    The name of the object.
     * @param   type    The mime-type of the object.  See the class summary.
     */    
    public EmbeddedBinaryObject(String name, String type) {
        super(name, type);
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
    EmbeddedBinaryObject(String name, String type, OfficeZip source) {
        super(name, type, source);
    }

    
    /**
     * This method returns the data for this object.
     *
     * @return  A <code>byte</code> array containing the object's data.
     */
    public byte[] getBinaryData() {
        
        if (objData == null) {
            // See if we came from a Zip file
            if (zipFile != null) {
                objData = zipFile.getNamedBytes(objName);
            }
        }
        
        return objData;
    }
    
    
    /**
     * Sets the data for this object.
     *
     * @param   data    A <code>byte</code> array containing data for the object.
     */
    public void setBinaryData(byte[] data) {
        objData = data;
        hasChanged = true;
    }
    
    /** 
     * Package private method for writing the data of the EmbeddedObject to a
     * SX? file.
     *
     * @param   zip     An <code>OfficeZip</code> instance representing the file
     *                  the data is to be written to.
     */
    void write(OfficeZip zip) {
        if (hasChanged) {
            zip.setNamedBytes(objName, objData);
        }
    }
    
    
    /** 
     * Package private method that constructs the manifest.xml entries for this
     * embedded object.
     *
     * @return  Document    <code>Document</code> containing the manifest entries.
     */
    void writeManifestData(Document manifestDoc) throws DOMException {        
        Element objNode = manifestDoc.createElement(OfficeConstants.TAG_MANIFEST_FILE);
        
        objNode.setAttribute(OfficeConstants.ATTRIBUTE_MANIFEST_FILE_TYPE, objType);
        objNode.setAttribute(OfficeConstants.ATTRIBUTE_MANIFEST_FILE_PATH,  objName);
        
        manifestDoc.getDocumentElement().appendChild(objNode);
    }
    
}

