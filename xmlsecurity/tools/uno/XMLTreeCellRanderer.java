/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XMLTreeCellRanderer.java,v $
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

