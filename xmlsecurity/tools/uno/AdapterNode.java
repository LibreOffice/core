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
import org.w3c.dom.Attr;
import org.w3c.dom.NamedNodeMap;


/*
 * This class wraps a DOM node and returns the text we want to
 * display in the tree. It also returns children, index values,
 * and child counts.
 */
class AdapterNode
{
    private Node m_domNode;
    static final int ELEMENT_TYPE =   Node.ELEMENT_NODE;

    /*
     * An array of names for DOM node-types
     */
    static final String[] typeName = {
        "none",
        "Element",
        "Attr",
        "Text",
        "CDATA",
        "EntityRef",
        "Entity",
        "ProcInstr",
        "Comment",
        "Document",
        "DocType",
        "DocFragment",
        "Notation",
        };

    protected Node getNode()
    {
        return m_domNode;
    }

    /*
     * Construct an Adapter node from a DOM node
     */
    protected AdapterNode(org.w3c.dom.Node node)
    {
        m_domNode = node;
    }

    /*
     * Return children, index, and count values
     */
    protected int index(AdapterNode child)
    {
        int count = childCount();
        for (int i=0; i<count; ++i)
        {
            AdapterNode n = this.child(i);
            if (child.m_domNode == n.m_domNode) return i;
        }
        return -1;
    }

    protected AdapterNode child(int searchIndex)
    {
        if (m_domNode == null) return null;

        /*
         * Note: JTree index is zero-based.
         */
        org.w3c.dom.Node node =
            m_domNode.getChildNodes().item(searchIndex);

        return new AdapterNode(node);
    }

    protected int childCount()
    {
        int rc = 0;

        if (m_domNode != null)
        {
            rc = m_domNode.getChildNodes().getLength();
        }

        return rc;
    }

    /*
     * Return a string that identifies this node in the tree
     */
    public String toString()
    {
        String rc = null;

        if (m_domNode != null)
        {
            String s = typeName[m_domNode.getNodeType()];
            String nodeName = m_domNode.getNodeName();

            if (! nodeName.startsWith("#"))
            {
                s += ": " + nodeName;
            }

            if (m_domNode.getNodeValue() != null)
            {
                if (s.startsWith("ProcInstr"))
                {
                    s += ", ";
                }
                else
                {
                    s += ": ";
                }

                String t = m_domNode.getNodeValue();
                s += t;
            }

            if (m_domNode.getNodeType() == ELEMENT_TYPE)
            {
                NamedNodeMap attrs = m_domNode.getAttributes();

                int length = attrs.getLength();
                for (int i=0; i<length; ++i)
                {
                    Attr attr = (Attr)(attrs.item(i));
                    s += " "+ attr.getName()+"='"+attr.getValue() + "'";
                }
            }
            rc = s;
        }

        return rc;
    }
}

