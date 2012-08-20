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
     * <b>N.B.</b>The name refers to the name as found in the
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
     */
    abstract void writeManifestData(Document manifestDoc) throws DOMException;
}
