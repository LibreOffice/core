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

import org.w3c.dom.Node;
import com.sun.star.xml.sax.XDocumentHandler;
import org.w3c.dom.Attr;
import org.w3c.dom.NamedNodeMap;

/*
 * this class is used to parse a document into SAX events
 */
class ParsingThread
{
    /*
     * the Node which will be handled with in the next step
     */
    private Node m_node;

    /*
     * the event to be handled in the next step.
     * true means endElement event, false otherwise.
     */
    private boolean m_bIsEndEvent;

    /*
     * the document handler which receives generated SAX events
     */
    private XDocumentHandler m_xDocumentHandler;

    /*
     * the TestTool which receives UI feedbacks
     */
    private TestTool m_testTool;


        ParsingThread(Node node, XDocumentHandler xDocumentHandler, TestTool testTool)
        {
            m_node = node;
            m_xDocumentHandler = xDocumentHandler;
            m_testTool = testTool;

            m_bIsEndEvent = false;
        }

        /*
         * changes the document handler.
         */
        protected void setHandler(XDocumentHandler xDocumentHandler)
        {
            this.m_xDocumentHandler = xDocumentHandler;
        }

        /*
         * sends the next SAX event.
         * when there is no further step, then false is returned,
         * otherwise, true returned.
         */
        protected boolean nextStep()
        {
            boolean rc = true;

            try
            {
                String message;
            int type = m_node.getNodeType();
            if (!m_bIsEndEvent)
            /*
             * the next event is not a endElement event.
             */
            {
                switch (type)
                {
                case Node.DOCUMENT_NODE: /* startDocument */
                    m_testTool.updatesCurrentSAXEventInformation("startDocument");
                    m_xDocumentHandler.startDocument();
                    m_testTool.updatesUIs();
                    break;
                case Node.ELEMENT_NODE: /* startElement */
                    String nodeName = m_node.getNodeName();
                    message = "startElement:"+nodeName;
                    NamedNodeMap attrs = m_node.getAttributes();

                    AttributeListHelper attributeListHelper = new AttributeListHelper();

                    int length = attrs.getLength();
                    for (int i=0; i<length; ++i)
                    {
                        Attr attr = (Attr)attrs.item(i);
                        attributeListHelper.setAttribute(attr.getName(), "CDATA", attr.getValue());
                        message += " "+attr.getName()+"='"+attr.getValue()+"'";
                    }

                    m_testTool.updatesCurrentSAXEventInformation(message);
                    m_xDocumentHandler.startElement(m_node.getNodeName(), attributeListHelper);

                    m_testTool.updatesUIs();
                    break;
                case Node.TEXT_NODE: /* characters */
                    message = m_node.getNodeValue();
                    if (message != null)
                    {
                        m_testTool.updatesCurrentSAXEventInformation("characters:"+message);
                        m_xDocumentHandler.characters(message);
                        m_testTool.updatesUIs();
                    }
                    break;
                case Node.COMMENT_NODE: /* comment */
                    break;
                case Node.PROCESSING_INSTRUCTION_NODE: /* PI */
                    m_testTool.updatesCurrentSAXEventInformation("processingInstruction:"+m_node.getNodeName()+" "+m_node.getNodeValue());
                    m_xDocumentHandler.processingInstruction(m_node.getNodeName(), m_node.getNodeValue());
                    m_testTool.updatesUIs();
                    break;
                }

                /*
                 * figures out the event for the next step.
                 */
                switch (type)
                {
                    case Node.DOCUMENT_NODE:
                    case Node.ELEMENT_NODE:
                        if (m_node.hasChildNodes())
                        /*
                         * for a Document node or an Element node,
                         * if the node has children, then the next event will be for its
                         * first child node.
                         */
                        {
                            m_node = m_node.getFirstChild();
                        }
                        else
                        /*
                         * otherwise, the next event will be endElement.
                         */
                        {
                            m_bIsEndEvent = true;
                        }
                        break;
                    case Node.TEXT_NODE:
                    case Node.PROCESSING_INSTRUCTION_NODE:
                    case Node.COMMENT_NODE:
                        Node nextNode = m_node.getNextSibling();
                        if (nextNode != null)
                        /*
                         * for other kinds of node,
                         * if it has a next sibling, then the next event will be for that
                         * sibling.
                         */
                        {
                            m_node = nextNode;
                        }
                        else
                        /*
                         * otherwise, the next event will be the endElement for the node's
                         * parent node.
                         */
                        {
                            m_node = m_node.getParentNode();
                            m_bIsEndEvent = true;
                        }
                        break;
                }
            }
            else
            /*
             * the next event is an endElement event.
             */
            {
                switch (type)
                {
                    case Node.DOCUMENT_NODE: /* endDocument */
                        m_testTool.updatesCurrentSAXEventInformation("endDocument");
                        m_xDocumentHandler.endDocument();
                        m_testTool.updatesUIs();

                        /*
                         * no further steps.
                         */
                        rc = false;
                        break;
                    case Node.ELEMENT_NODE: /* endElement */
                        m_testTool.updatesCurrentSAXEventInformation("endElement:"+m_node.getNodeName());
                        m_xDocumentHandler.endElement(m_node.getNodeName());
                        m_testTool.updatesUIs();

                        Node nextNode = m_node.getNextSibling();
                        if (nextNode != null)
                        /*
                         * if the node has a next sibling, then the next event will be the
                         * start event for that sibling node.
                         */
                        {
                            m_node = nextNode;
                            m_bIsEndEvent = false;
                        }
                        else
                        /*
                         * otherwise, the next event will be the endElement for the node's
                         * parent node.
                         */
                        {
                            m_node = m_node.getParentNode();
                        }
                        break;
                }
            }
        }
        catch(  com.sun.star.xml.sax.SAXException e)
        {
            e.printStackTrace();

            /*
             * forces to end.
             */
            rc = false;
        }

        return rc;
    }
}

