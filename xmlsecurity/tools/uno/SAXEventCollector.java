/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SAXEventCollector.java,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package com.sun.star.xml.security.uno;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.Text;
import org.w3c.dom.ProcessingInstruction;

/* uno classes */
import com.sun.star.xml.sax.XDocumentHandler;
import com.sun.star.xml.sax.XAttributeList;

/*
 * this class is used to collect all received SAX events
 * into a DOM document.
 */
class SAXEventCollector implements XDocumentHandler
{
    /*
     * the document which keeps received SAX events
     */
    private Document m_document;

    /*
     * the current Element to which the next received
     * SAX event will be added.
     */
    private Node m_currentElement;

    /*
     * the TestTool which receives UI feedbacks
     */
    private TestTool m_testTool;

    /*
     * whether displays information on console.
     */
    private boolean m_systemDisplay;

    SAXEventCollector(TestTool testTool)
    {
        this(testTool, false);
    }

    SAXEventCollector(TestTool testTool, boolean sysDis)
    {
        m_systemDisplay = sysDis;
        m_testTool = testTool;

        DocumentBuilderFactory factory =
            DocumentBuilderFactory.newInstance();

        try
        {
            DocumentBuilder builder = factory.newDocumentBuilder();
            m_document = builder.newDocument();

            m_currentElement = m_document;
        }
        catch (ParserConfigurationException pce) {
            pce.printStackTrace();
        }
    }

    protected Document getDocument()
    {
        return m_document;
    }

    protected Node getCurrentElement()
    {
        return m_currentElement;
    }

        /*
         * XDocumentHandler
         */
    public void  startDocument ()
    {
    }

    public void endDocument()
    {
    }

    public void startElement (String str, com.sun.star.xml.sax.XAttributeList xattribs)
    {
        Element newElement = m_document.createElement(str);

        if (xattribs !=null)
        {
            int length = xattribs.getLength();
            for (short i=0; i<length; ++i)
            {
                newElement.setAttribute(
                    xattribs.getNameByIndex(i),
                    xattribs.getValueByIndex(i));
            }
        }

        if (m_systemDisplay)
        {
            System.out.println("startElement:"+m_currentElement.toString());
        }

        m_currentElement.appendChild(newElement);
        m_currentElement = newElement;

        if (m_testTool != null)
        {
            m_testTool.updatesUIs();
        }
    }

    public void endElement(String str)
    {
        if (m_systemDisplay)
        {
            System.out.println("endElement:"+str+" "+m_currentElement.toString());
        }

            m_currentElement = m_currentElement.getParentNode();
        if (m_systemDisplay)
        {
            System.out.println("----> "+m_currentElement.toString());
        }

        if (m_testTool != null)
        {
            m_testTool.updatesUIs();
        }
    }

    public void characters(String str)
    {
            Text newText = m_document.createTextNode(str);
            m_currentElement.appendChild(newText);
        if (m_testTool != null)
        {
            m_testTool.updatesUIs();
        }
    }

    public void ignorableWhitespace(String str)
    {
    }

    public void processingInstruction(String aTarget, String aData)
    {
        ProcessingInstruction newPI
            = m_document.createProcessingInstruction(aTarget, aData);
        m_currentElement.appendChild(newPI);
        if (m_testTool != null)
        {
            m_testTool.updatesUIs();
        }
    }

    public void setDocumentLocator (com.sun.star.xml.sax.XLocator xLocator )
        throws com.sun.star.xml.sax.SAXException
    {
    }
}

