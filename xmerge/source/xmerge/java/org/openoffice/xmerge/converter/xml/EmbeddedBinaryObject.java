/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EmbeddedBinaryObject.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:23:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package org.openoffice.xmerge.converter.xml;

import org.w3c.dom.Document;
import org.w3c.dom.DOMException;
import org.w3c.dom.Element;
import org.w3c.dom.Node;


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

