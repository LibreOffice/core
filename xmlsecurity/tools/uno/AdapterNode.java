/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AdapterNode.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:42:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

