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

