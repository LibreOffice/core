/*************************************************************************
 *
 *  $RCSfile: SAXEventCollector.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

