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

import org.w3c.dom.Document;
import javax.swing.event.TreeModelListener;
import javax.swing.event.TreeModelEvent;
import javax.swing.tree.TreePath;
import java.util.Enumeration;
import java.util.Vector;

/*
 * This adapter converts the current Document (a DOM) into
 * a JTree model.
 */
class DomToTreeModelAdapter
    implements javax.swing.tree.TreeModel
{
    private Document m_document;
    private Vector m_listenerList = new Vector();

    public DomToTreeModelAdapter(Document document)
    {
        m_document = document;
    }

    /*
     * Basic TreeModel operations
     */
    public Object  getRoot()
    {
        return new AdapterNode(m_document);
    }

    public boolean isLeaf(Object aNode)
    {
        boolean rc = true;

        /*
         * Determines whether the icon shows up to the left.
         * Return true for any node with no children.
         */
        AdapterNode node = (AdapterNode) aNode;

        if (node.childCount() > 0)
        {
            rc = false;
        }

        return rc;
    }

    public int getChildCount(Object parent)
    {
        AdapterNode node = (AdapterNode) parent;
        return node.childCount();
    }

    public Object getChild(Object parent, int index)
    {
        AdapterNode node = (AdapterNode) parent;
        return node.child(index);
    }

    public int getIndexOfChild(Object parent, Object child)
    {
        AdapterNode node = (AdapterNode) parent;
        return node.index((AdapterNode) child);
    }

    public void valueForPathChanged(TreePath path, Object newValue)
    {
        /*
         * Null. We won't be making changes in the GUI
         * If we did, we would ensure the new value was really new,
         * adjust the model, and then fire a TreeNodesChanged event.
         */
    }

    public void addTreeModelListener(TreeModelListener listener)
    {
        if ( listener != null
            && ! m_listenerList.contains( listener ) )
        {
            m_listenerList.addElement( listener );
        }
    }

    public void removeTreeModelListener(TreeModelListener listener)
    {
        if ( listener != null )
        {
            m_listenerList.removeElement( listener );
        }
    }

    public void fireTreeNodesChanged( TreeModelEvent e )
    {
        Enumeration listeners = m_listenerList.elements();
        while ( listeners.hasMoreElements() )
        {
            TreeModelListener listener =
            (TreeModelListener) listeners.nextElement();
            listener.treeNodesChanged( e );
        }
    }

    public void fireTreeNodesInserted( TreeModelEvent e )
    {
        Enumeration listeners = m_listenerList.elements();
        while ( listeners.hasMoreElements() )
        {
            TreeModelListener listener =
            (TreeModelListener) listeners.nextElement();
            listener.treeNodesInserted( e );
        }
    }

    public void fireTreeNodesRemoved( TreeModelEvent e )
    {
        Enumeration listeners = m_listenerList.elements();
        while ( listeners.hasMoreElements() )
        {
            TreeModelListener listener =
            (TreeModelListener) listeners.nextElement();
            listener.treeNodesRemoved( e );
        }
    }

    public void fireTreeStructureChanged( TreeModelEvent e )
    {
        Enumeration listeners = m_listenerList.elements();
        while ( listeners.hasMoreElements() )
        {
            TreeModelListener listener =
            (TreeModelListener) listeners.nextElement();
            listener.treeStructureChanged( e );
        }
    }
}

