/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

