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

import com.sun.star.lang.IndexOutOfBoundsException;
import java.util.Vector;

/** The VectorNode class is a simple container whose list of children is
    managed entirely by its owner.
*/
class VectorNode
    extends StringNode
{
    private Vector maChildren;

    public VectorNode (String sDisplayObject, AccessibleTreeNode aParent)
    {
        super (sDisplayObject, aParent);

        maChildren = new Vector ();
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
        return (AccessibleTreeNode)maChildren.elementAt (nIndex);
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
