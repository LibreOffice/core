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


package com.sun.star.report.pentaho.parser.rpt;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.ObjectOleElement;
import com.sun.star.report.pentaho.parser.ElementReadHandler;

import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 *
 * @author Ocke Janssen
 */
public class MasterDetailReadHandler extends ElementReadHandler
{

    private final ObjectOleElement element;
    private final boolean parseMasterDetail;

    public MasterDetailReadHandler(final ObjectOleElement element)
    {
        this.element = element;
        parseMasterDetail = false;
    }

    public MasterDetailReadHandler(final ObjectOleElement element, final boolean parseMasterDetail)
    {
        this.element = element;
        this.parseMasterDetail = parseMasterDetail;
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
        if (parseMasterDetail)
        {
            final String master = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "master");
            if (master != null && master.length() > 0)
            {
                final String detail = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "detail");
                element.addMasterDetailFields(master, detail);
            }
        }
    }

    /**
     * Returns the handler for a child element.
     *
     * @param tagName the tag name.
     * @param atts    the attributes.
     * @return the handler or null, if the tagname is invalid.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected XmlReadHandler getHandlerForChild(final String uri,
            final String tagName,
            final Attributes atts)
            throws SAXException
    {
        if (OfficeNamespaces.OOREPORT_NS.equals(uri) && "master-detail-field".equals(tagName))
        {
            // expect a report control. The control will modifiy the current
            // element (as we do not separate the elements that strictly ..)
            return new MasterDetailReadHandler(element, true);
        }

        return null;
    }

    public Element getElement()
    {
        return element;
    }
}
