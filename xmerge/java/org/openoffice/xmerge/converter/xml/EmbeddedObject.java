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