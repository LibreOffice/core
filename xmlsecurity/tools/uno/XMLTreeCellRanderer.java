/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTreeCellRanderer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:45:52 $
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

import javax.swing.tree.DefaultTreeCellRenderer;
import org.w3c.dom.Node;
import javax.swing.ImageIcon;
import java.awt.Component;
import javax.swing.JTree;

/*
 * a TreeCellRender which can show a graph on the current
 * tree node.
 */
class XMLTreeCellRanderer extends DefaultTreeCellRenderer
{
    /*
     * the icon for the current Node
     */
    private ImageIcon m_currentIcon;

    /*
     * the current Node
     */
    private Node m_currentNode;

    XMLTreeCellRanderer(Node currentNode)
    {
        m_currentNode = currentNode;
        m_currentIcon = new ImageIcon("current.gif");
    }

    public Component getTreeCellRendererComponent(
                    JTree tree,
                    Object value,
                    boolean sel,
                    boolean expanded,
                    boolean leaf,
                    int row,
                    boolean hasFocus)
    {
        super.getTreeCellRendererComponent(
                        tree, value, sel,
                        expanded, leaf, row,
                        hasFocus);

        if (((AdapterNode)value).getNode() == m_currentNode)
        {
            setIcon(m_currentIcon);
            setToolTipText("This is the current element.");
        }
        else
        {
            setToolTipText(null); /* no tool tip */
        }

        return this;
    }
}

