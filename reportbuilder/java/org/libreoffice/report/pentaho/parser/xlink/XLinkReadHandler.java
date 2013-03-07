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
package org.libreoffice.report.pentaho.parser.xlink;

import org.libreoffice.report.pentaho.OfficeNamespaces;

import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * An image reference.
 *
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
