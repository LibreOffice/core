/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DomToTreeModelAdapter.java,v $
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

