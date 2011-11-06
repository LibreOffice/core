/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

