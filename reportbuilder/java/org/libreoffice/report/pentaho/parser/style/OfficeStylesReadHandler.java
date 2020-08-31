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
package org.libreoffice.report.pentaho.parser.style;

import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.OfficeStyles;
import org.libreoffice.report.pentaho.parser.ElementReadHandler;
import org.libreoffice.report.pentaho.parser.data.DataStyleReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.modules.factories.report.flow.SectionReadHandler;
import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class OfficeStylesReadHandler extends ElementReadHandler
{

    private final List<OfficeStyleReadHandler> textStyleChildren;
    private final List<DataStyleReadHandler> dataStyleChildren;
    private final List<SectionReadHandler> otherStyleChildren;
    private final List<PageLayoutReadHandler> pageLayoutChildren;
    private final OfficeStyles officeStyles;

    public OfficeStylesReadHandler(final OfficeStyles officeStyles)
    {
        this.officeStyles = officeStyles;
        this.pageLayoutChildren = new ArrayList<PageLayoutReadHandler>();
        this.dataStyleChildren = new ArrayList<DataStyleReadHandler>();
        this.textStyleChildren = new ArrayList<OfficeStyleReadHandler>();
        this.otherStyleChildren = new ArrayList<SectionReadHandler>();
    }

    /**
     * Returns the handler for a child element.
     *
     * @param tagName the tag name.
     * @param atts    the attributes.
     * @return the handler or null, if the tagname is invalid.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    @Override
    protected XmlReadHandler getHandlerForChild(final String uri, final String tagName,
            final Attributes atts)
            throws SAXException
    {
        if (OfficeNamespaces.STYLE_NS.equals(uri))
        {
            if ("style".equals(tagName))
            {
                final OfficeStyleReadHandler xrh = new OfficeStyleReadHandler();
                textStyleChildren.add(xrh);
                return xrh;
            }
            else if ("page-layout".equals(tagName))
            {
                final PageLayoutReadHandler prh = new PageLayoutReadHandler();
                pageLayoutChildren.add(prh);
                return prh;
            }
        }
        else if (OfficeNamespaces.DATASTYLE_NS.equals(uri))
        {
            final DataStyleReadHandler xrh = new DataStyleReadHandler(false);
            dataStyleChildren.add(xrh);
            return xrh;
        }

        final SectionReadHandler genericReadHandler = new SectionReadHandler();
        otherStyleChildren.add(genericReadHandler);
        return genericReadHandler;
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    @Override
    protected void doneParsing() throws SAXException
    {
        for (int i = 0; i < textStyleChildren.size(); i++)
        {
            final OfficeStyleReadHandler handler = textStyleChildren.get(i);
            officeStyles.addStyle(handler.getOfficeStyle());
        }

        for (int i = 0; i < pageLayoutChildren.size(); i++)
        {
            final PageLayoutReadHandler handler = pageLayoutChildren.get(i);
            officeStyles.addPageStyle(handler.getPageLayout());
        }

        for (int i = 0; i < dataStyleChildren.size(); i++)
        {
            final DataStyleReadHandler handler = dataStyleChildren.get(i);
            officeStyles.addDataStyle(handler.getDataStyle());
        }

        for (int i = 0; i < otherStyleChildren.size(); i++)
        {
            final SectionReadHandler handler = otherStyleChildren.get(i);
            officeStyles.addOtherNode((Element) handler.getNode());
        }
    }

    @Override
    public Element getElement()
    {
        return officeStyles;
    }
}
