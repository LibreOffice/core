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

