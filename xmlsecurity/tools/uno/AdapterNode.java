/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AdapterNode.java,v $
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

