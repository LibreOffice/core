/*************************************************************************
 *
 *  $RCSfile: DomToTreeModelAdapter.java,v $
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

