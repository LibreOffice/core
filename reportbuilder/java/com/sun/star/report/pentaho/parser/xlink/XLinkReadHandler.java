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


package com.sun.star.report.pentaho.parser.xlink;

import com.sun.star.report.pentaho.OfficeNamespaces;

import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * An image reference.
 *
 * @author Thomas Morgner
 */
public class XLinkReadHandler extends AbstractXmlReadHandler
{

    private String uri;
    private String type;
    private String show;
    private String actuate;

    public XLinkReadHandler()
    {
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        uri = attrs.getValue(OfficeNamespaces.XLINK_NS, "uri");
        type = attrs.getValue(OfficeNamespaces.XLINK_NS, "type");
        show = attrs.getValue(OfficeNamespaces.XLINK_NS, "show");
        actuate = attrs.getValue(OfficeNamespaces.XLINK_NS, "actuate");
    }

    /**
     * Returns the object for this element or null, if this element does not
     * create an object.
     *
     * @return the object.
     */
    public Object getObject() throws SAXException
    {
        return uri;
    }

    public String getUri()
    {
        return uri;
    }

    public String getType()
    {
        return type;
    }

    public String getShow()
    {
        return show;
    }

    public String getActuate()
    {
        return actuate;
    }
}
