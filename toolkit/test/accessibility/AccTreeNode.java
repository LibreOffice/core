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

import java.util.ArrayList;
import java.util.Arrays;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEditableText;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.accessibility.XAccessibleTable;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.uno.UnoRuntime;

/**
 * The node type for the AccessibleTreeModel.
 * This implements all the child-handling based on the appropriate
 * NodeHandlers. Trivial nodes can be implemented by any Object
 * type.
 */
class AccTreeNode
    extends AccessibleTreeNode
{
    private class HandlerDescriptor
    {
        private HandlerDescriptor (NodeHandler aHandler)
        {
            maHandler = aHandler;
            mnChildCount = -1;
        }
        private NodeHandler maHandler;
        private int mnChildCount;
    }
    /// NodeHandlers for this node
    private ArrayList<HandlerDescriptor> maHandlers;

    // The accessible context of this node.
    private XAccessibleContext mxContext;
    private XAccessibleComponent mxComponent;
    private XAccessibleText mxText;
    private XAccessibleTable mxTable;

    public AccTreeNode (XAccessible xAccessible, XAccessibleContext xContext, Object aDisplay, AccessibleTreeNode aParent)
    {
        super (aDisplay, aParent);

        maHandlers = new ArrayList<HandlerDescriptor>(5);
        mxContext = xContext;
    }

    /** Update the internal data extracted from the corresponding accessible
        object.  This is done by replacing every handler by a new one.  An
        update method at each handler would be better of course.
    */
    @Override
    public void update ()
    {
        for (int i=0; i<maHandlers.size(); i++)
        {
            System.out.println ("replacing handler " + i);
            HandlerDescriptor aDescriptor = maHandlers.get(i);
            aDescriptor.maHandler = aDescriptor.maHandler.createHandler (mxContext);
            aDescriptor.mnChildCount =
                    aDescriptor.maHandler.getChildCount ();
        }
    }

    public XAccessibleContext getContext ()
    {
        return mxContext;
    }

    public XAccessibleComponent getComponent ()
    {
        if (mxComponent == null && mxContext != null)
            mxComponent = UnoRuntime.queryInterface(
                XAccessibleComponent.class, mxContext);
        return mxComponent;
    }

    public XAccessibleText getText ()
    {
        if (mxText == null && mxContext != null)
            mxText = UnoRuntime.queryInterface(
                XAccessibleText.class, mxContext);
        return mxText;
    }

    public XAccessibleEditableText getEditText ()
    {
        return UnoRuntime.queryInterface(
                XAccessibleEditableText.class, mxContext);
    }

    public XAccessibleTable getTable ()
    {
        if (mxTable == null && mxContext != null)
            mxTable = UnoRuntime.queryInterface(
                XAccessibleTable.class, mxContext);
        return mxTable;
    }


    public XAccessibleSelection getSelection ()
    {
        return UnoRuntime.queryInterface(
                XAccessibleSelection.class, mxContext);
    }

    public void addHandler( NodeHandler aHandler )
    {
        if (aHandler != null)
            maHandlers.add (new HandlerDescriptor (aHandler));
    }


    /** iterate over handlers and return child sum */
    private HandlerDescriptor getHandlerDescriptor (int i)
    {
        HandlerDescriptor aDescriptor = maHandlers.get(i);
        if (aDescriptor.mnChildCount < 0)
            aDescriptor.mnChildCount =
                    aDescriptor.maHandler.getChildCount ();
        return aDescriptor;
    }

    @Override
    public int getChildCount()
    {
        int nChildCount = 0;
        for (int i = 0; i < maHandlers.size(); i++)
        {
            HandlerDescriptor aDescriptor = getHandlerDescriptor (i);
            nChildCount += aDescriptor.mnChildCount;
        }
        return nChildCount;
    }

    /** iterate over handlers until the child is found */
    @Override
    public AccessibleTreeNode getChild (int nIndex)
        throws IndexOutOfBoundsException
    {
        if( nIndex >= 0 )
        {
            for(int i = 0; i < maHandlers.size(); i++)
            {
                // check if this handler has the child, and if not
                // search with next handler
                HandlerDescriptor aDescriptor = getHandlerDescriptor (i);
                if (nIndex < aDescriptor.mnChildCount)
                    return aDescriptor.maHandler.getChild (this, nIndex);
                else
                    nIndex -= aDescriptor.mnChildCount;
            }
        }
        else
            throw new IndexOutOfBoundsException();

        // nothing found?
        return null;
    }

    @Override
    public AccessibleTreeNode getChildNoCreate (int nIndex)
        throws IndexOutOfBoundsException
    {
        if( nIndex >= 0 )
        {
            for(int i = 0; i < maHandlers.size(); i++)
            {
                // check if this handler has the child, and if not
                // search with next handler
                HandlerDescriptor aDescriptor = getHandlerDescriptor (i);
                if (nIndex < aDescriptor.mnChildCount)
                    return aDescriptor.maHandler.getChildNoCreate (nIndex);
                else
                    nIndex -= aDescriptor.mnChildCount;
            }
        }
        else
            throw new IndexOutOfBoundsException();

        // nothing found?
        return null;
    }

    @Override
    public boolean removeChild (int nIndex)
        throws IndexOutOfBoundsException
    {
        boolean bStatus = false;
        if (nIndex >= 0)
        {
            for (int i=0; i<maHandlers.size(); i++)
            {
                // check if this handler has the child, and if not
                // search with next handler
                HandlerDescriptor aDescriptor = getHandlerDescriptor (i);
                if (nIndex < aDescriptor.mnChildCount)
                {
                    bStatus = aDescriptor.maHandler.removeChild (nIndex);
                    aDescriptor.mnChildCount = aDescriptor.maHandler.getChildCount ();
                    break;
                }
                else
                    nIndex -= aDescriptor.mnChildCount;
            }
        }
        else
            throw new IndexOutOfBoundsException();

        return bStatus;
    }


    @Override
    public int indexOf (AccessibleTreeNode aNode)
    {
        int nBaseIndex = 0;
        if (aNode != null)
        {
            for (int i=0; i<maHandlers.size(); i++)
            {
                HandlerDescriptor aDescriptor = getHandlerDescriptor (i);
                int nIndex = aDescriptor.maHandler.indexOf (aNode);
                if (nIndex >= 0)
                    return nBaseIndex + nIndex;
                else
                    nBaseIndex += aDescriptor.mnChildCount;
            }
        }

        return -1;
    }

    /** this node is a leaf if have no handlers, or is those
            handlers show no children */
    @Override
    public boolean isLeaf()
    {
        return maHandlers.isEmpty();
    }

    @Override
    public boolean equals (Object aOther)
    {
        return (this == aOther) || (aOther!=null && aOther.equals(mxContext));
    }


    /** iterate over handlers until the child is found */
    public void getActions(java.util.List<String> aActions)
    {
        for(int i = 0; i < maHandlers.size(); i++)
        {
            HandlerDescriptor aDescriptor = getHandlerDescriptor (i);
            NodeHandler aHandler = aDescriptor.maHandler;
            String[] aHandlerActions = aHandler.getActions (this);
            aActions.addAll(Arrays.asList(aHandlerActions));
        }
    }

    @Override
    public void performAction( int nIndex )
    {
        if( nIndex >= 0 )
        {
            for(int i = 0; i < maHandlers.size(); i++)
            {
                // check if this handler has the child, and if not
                // search with next handler
                HandlerDescriptor aDescriptor = getHandlerDescriptor (i);
                NodeHandler aHandler = aDescriptor.maHandler;
                int nCount = aHandler.getActions(this).length;
                if( nCount > nIndex )
                {
                    aHandler.performAction(this, nIndex );
                    return;
                }
                else
                    nIndex -= nCount;
            }
        }
    }

    /** Try to add the specified accessible object as new accessible child of the
        AccessibleTreeHandler.
        Note that child is used in another context than
        it is used in the other methods of this class.
    */
    public AccessibleTreeNode addAccessibleChild (XAccessible xChild)
    {
        for(int i = 0; i < maHandlers.size(); i++)
        {
            HandlerDescriptor aDescriptor = getHandlerDescriptor (i);
            if (aDescriptor.maHandler instanceof AccessibleTreeHandler)
            {
                AccessibleTreeHandler aHandler = (AccessibleTreeHandler)aDescriptor.maHandler;
                AccessibleTreeNode aNode = aHandler.addAccessibleChild (this, xChild);
                aDescriptor.mnChildCount = aHandler.getChildCount ();
                return aNode;
            }
        }
        return null;
    }

    public java.util.List<Integer> updateChildren (java.lang.Class class1, java.lang.Class<AccessibleExtendedComponentHandler> class2)
    {
        ArrayList<Integer> aChildIndices = new ArrayList<Integer>();
        int nOffset = 0;
        for(int i=0; i < maHandlers.size(); i++)
        {
            HandlerDescriptor aDescriptor = getHandlerDescriptor (i);
            if ((class1.isInstance(aDescriptor.maHandler))
                || (class2 !=null && class2.isInstance(aDescriptor.maHandler)))
            {
                aDescriptor.maHandler.update(this);
                // Get updated number of children.
                int nChildCount = aDescriptor.maHandler.getChildCount ();
                aDescriptor.mnChildCount = nChildCount;
                // Fill in the indices of the updated children.
                for (int j=0; j<nChildCount; j++)
                    aChildIndices.add(j+nOffset);
            }
            nOffset += aDescriptor.mnChildCount;
        }
        return aChildIndices;
    }
}
