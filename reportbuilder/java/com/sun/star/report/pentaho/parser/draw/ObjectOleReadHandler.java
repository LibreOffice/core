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


package com.sun.star.report.pentaho.parser.draw;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.ObjectOleElement;
import com.sun.star.report.pentaho.parser.ElementReadHandler;

import org.jfree.report.structure.Element;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 *
 * @author Ocke Janssen
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
