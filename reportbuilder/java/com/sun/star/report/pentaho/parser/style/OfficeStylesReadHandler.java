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


package com.sun.star.report.pentaho.parser.style;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.OfficeStyles;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.data.DataStyleReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.modules.factories.report.flow.SectionReadHandler;
import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class OfficeStylesReadHandler extends ElementReadHandler
{

    private final List textStyleChilds;
    private final List dataStyleChilds;
    private final List otherStyleChilds;
    private final List pageLayoutChilds;
    private final OfficeStyles officeStyles;

    public OfficeStylesReadHandler(final OfficeStyles officeStyles)
    {
        this.officeStyles = officeStyles;
        this.pageLayoutChilds = new ArrayList();
        this.dataStyleChilds = new ArrayList();
        this.textStyleChilds = new ArrayList();
        this.otherStyleChilds = new ArrayList();
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
    protected XmlReadHandler getHandlerForChild(final String uri, final String tagName,
            final Attributes atts)
            throws SAXException
    {
        if (OfficeNamespaces.STYLE_NS.equals(uri))
        {
            if ("style".equals(tagName))
            {
                final OfficeStyleReadHandler xrh = new OfficeStyleReadHandler();
                textStyleChilds.add(xrh);
                return xrh;
            }
            else if ("page-layout".equals(tagName))
            {
                final PageLayoutReadHandler prh = new PageLayoutReadHandler();
                pageLayoutChilds.add(prh);
                return prh;
            }
        }
        else if (OfficeNamespaces.DATASTYLE_NS.equals(uri))
        {
            final DataStyleReadHandler xrh = new DataStyleReadHandler(false);
            dataStyleChilds.add(xrh);
            return xrh;
        }

        final SectionReadHandler genericReadHander = new SectionReadHandler();
        otherStyleChilds.add(genericReadHander);
        return genericReadHander;
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void doneParsing() throws SAXException
    {
        for (int i = 0; i < textStyleChilds.size(); i++)
        {
            final OfficeStyleReadHandler handler =
                    (OfficeStyleReadHandler) textStyleChilds.get(i);
            officeStyles.addStyle(handler.getOfficeStyle());
        }

        for (int i = 0; i < pageLayoutChilds.size(); i++)
        {
            final PageLayoutReadHandler handler =
                    (PageLayoutReadHandler) pageLayoutChilds.get(i);
            officeStyles.addPageStyle(handler.getPageLayout());
        }

        for (int i = 0; i < dataStyleChilds.size(); i++)
        {
            final DataStyleReadHandler handler =
                    (DataStyleReadHandler) dataStyleChilds.get(i);
            officeStyles.addDataStyle(handler.getDataStyle());
        }

        for (int i = 0; i < otherStyleChilds.size(); i++)
        {
            final SectionReadHandler handler =
                    (SectionReadHandler) otherStyleChilds.get(i);
            officeStyles.addOtherNode((Element) handler.getNode());
        }
    }

    public Element getElement()
    {
        return officeStyles;
    }
}
