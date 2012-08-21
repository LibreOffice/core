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

import com.sun.star.lang.IndexOutOfBoundsException;
import java.util.Vector;

/** The VectorNode class is a simple container whose list of children is
    managed entirely by its owner.
*/
class VectorNode
    extends StringNode
{
    private Vector<AccessibleTreeNode> maChildren;

    public VectorNode (String sDisplayObject, AccessibleTreeNode aParent)
    {
        super (sDisplayObject, aParent);

        maChildren = new Vector<AccessibleTreeNode> ();
    }

    public void addChild (AccessibleTreeNode aChild)
    {
        maChildren.add (aChild);
    }

    public int getChildCount ()
    {
        return maChildren.size();
    }

    public AccessibleTreeNode getChild (int nIndex)
        throws IndexOutOfBoundsException
    {
        return maChildren.elementAt (nIndex);
    }

    public boolean removeChild (int nIndex)
        throws IndexOutOfBoundsException
    {
        return maChildren.remove (nIndex) != null;
    }

    public int indexOf (AccessibleTreeNode aNode)
    {
        return maChildren.indexOf (aNode);
    }

    public boolean isLeaf()
    {
        return maChildren.isEmpty();
    }
}
