/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
     */
    void writeManifestData(Document manifestDoc) throws DOMException {
        Element objNode = manifestDoc.createElement(OfficeConstants.TAG_MANIFEST_FILE);

        objNode.setAttribute(OfficeConstants.ATTRIBUTE_MANIFEST_FILE_TYPE, objType);
        objNode.setAttribute(OfficeConstants.ATTRIBUTE_MANIFEST_FILE_PATH,  objName);

        manifestDoc.getDocumentElement().appendChild(objNode);
    }

}

