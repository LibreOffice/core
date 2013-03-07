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
package org.libreoffice.report.pentaho.parser.draw;

import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.ObjectOleElement;
import org.libreoffice.report.pentaho.parser.ElementReadHandler;

import org.jfree.report.structure.Element;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 *
 */
public class ObjectOleReadHandler extends ElementReadHandler
{

    private final static String RPT_CHART_CLASS_ID = "80243D39-6741-46C5-926E-069164FF87BB";
    private final static String OOO_CHART_CLASS_ID = "12DCAE26-281F-416F-A234-C3086127382E";
    private final ObjectOleElement element;

    public ObjectOleReadHandler(final ObjectOleElement element)
    {
        this.element = element;
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        super.startParsing(attrs);

        final String url = attrs.getValue(OfficeNamespaces.XLINK_NS, "href");
        if (url != null)
        {
            element.setUrl(url);
        }

        String classid = attrs.getValue(OfficeNamespaces.DRAWING_NS, "class-id");
        if (classid != null)
        {
            if (classid.equalsIgnoreCase(RPT_CHART_CLASS_ID))
            {
                classid = OOO_CHART_CLASS_ID;
            }
            element.setClassId(classid);
        }
    }

    public Element getElement()
    {
        return element;
    }
}
