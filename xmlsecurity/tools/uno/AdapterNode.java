/*************************************************************************
 *
 *  $RCSfile: AdapterNode.java,v $
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

