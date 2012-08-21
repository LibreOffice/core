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

import java.util.Vector;
import com.sun.star.lang.IndexOutOfBoundsException;

/**
    Base class for all tree nodes.
 */
class AccessibleTreeNode
{
    /// The parent node.  It is null for the root node.
    protected AccessibleTreeNode maParent;

    /// The object to be displayed.
    private Object maDisplayObject;

    public AccessibleTreeNode (Object aDisplayObject, AccessibleTreeNode aParent)
    {
        maDisplayObject = aDisplayObject;
        maParent = aParent;
    }

    public void update ()
    {
        // Empty
    }

    public AccessibleTreeNode getParent ()
    {
        return maParent;
    }

    public Object getDisplayObject ()
    {
        return maDisplayObject;
    }

    public int getChildCount ()
    {
        return 0;
    }

    public AccessibleTreeNode getChild (int nIndex)
        throws IndexOutOfBoundsException
    {
        throw new IndexOutOfBoundsException();
    }

    public AccessibleTreeNode getChildNoCreate (int nIndex)
        throws IndexOutOfBoundsException
    {
        throw new IndexOutOfBoundsException();
    }

    public boolean removeChild (int nIndex)
        throws IndexOutOfBoundsException
    {
        throw new IndexOutOfBoundsException();
    }

    public int indexOf (AccessibleTreeNode aNode)
    {
        return -1;
    }

    /** Create a path to this node by first asking the parent for its path
        and then appending this object.
    */
    public void createPath (java.util.Vector<AccessibleTreeNode> aPath)
    {
        if (maParent != null)
            maParent.createPath (aPath);
        aPath.add (this);
    }

    public Object[] createPath ()
    {
        Vector<AccessibleTreeNode> aPath = new Vector<AccessibleTreeNode> (1);
        createPath (aPath);
        return aPath.toArray();
    }

    public boolean isLeaf()
    {
        return true;
    }

    public String toString()
    {
        return maDisplayObject.toString();
    }

    /** get names of suported actions */
    public String[] getActions ()
    {
        return new String[] {};
    }

    /** perform action */
    public void performAction (int nIndex)
    {
    }
}
