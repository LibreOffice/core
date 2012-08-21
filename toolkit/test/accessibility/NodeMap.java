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

import com.sun.star.accessibility.XAccessibleContext;

import java.util.HashMap;

abstract class NodeMapCallback
{
    public abstract void Apply (AccTreeNode aNode);
}

/** This map translates from XAccessible objects to our internal
    representations.
*/
class NodeMap
{
    public NodeMap ()
    {
        maXAccessibleToNode = new HashMap<XAccessibleContext, AccessibleTreeNode> ();
    }

    /** Clear the whole map.
    */
    public void Clear ()
    {
        maXAccessibleToNode.clear();
    }

    /** @return
            whether the new node was different from a previous one
            repspectively was the first one set.
    */
    public boolean InsertNode (XAccessibleContext xContext, AccessibleTreeNode aNode)
    {
        AccessibleTreeNode aPreviousNode = maXAccessibleToNode.put (
            xContext,
            aNode);
        return aPreviousNode != aNode;
    }

    protected void RemoveNode (AccessibleTreeNode aNode)
    {
        try
        {
            if ((aNode != null) && (aNode instanceof AccTreeNode))
            {
                maXAccessibleToNode.remove (((AccTreeNode)aNode).getContext());
            }
        }
        catch (Exception e)
        {
            System.out.println ("caught exception while removing node "
                + aNode + " : " + e);
            e.printStackTrace();
        }
    }


    public void ForEach (NodeMapCallback aFunctor)
    {
        Object[] aNodes = maXAccessibleToNode.values().toArray();
        for (int i=0; i<aNodes.length; i++)
        {
            if (aNodes[i] != null && (aNodes[i] instanceof AccTreeNode))
            {
                try
                {
                    aFunctor.Apply ((AccTreeNode)aNodes[i]);
                }
                catch (Exception e)
                {
                    System.out.println ("caught exception applying functor to "
                        + i + "th node " + aNodes[i] + " : " + e);
                    e.printStackTrace();
                }
            }
        }
    }

    AccessibleTreeNode GetNode (XAccessibleContext xContext)
    {
        return maXAccessibleToNode.get (xContext);
    }

    AccessibleTreeNode GetNode (Object aObject)
    {
        if (aObject instanceof XAccessibleContext)
            return GetNode ((XAccessibleContext)aObject);
        else
            return null;
    }

    XAccessibleContext GetAccessible (AccessibleTreeNode aNode)
    {
        if ((aNode != null) && (aNode instanceof AccTreeNode))
            return ((AccTreeNode)aNode).getContext();
        else
            return null;
    }

    boolean IsMember (XAccessibleContext xContext)
    {
        return maXAccessibleToNode.containsKey(xContext);
    }

    boolean ValueIsMember (AccessibleTreeNode aNode)
    {
        return maXAccessibleToNode.containsValue(aNode);
    }



    private HashMap<XAccessibleContext, AccessibleTreeNode> maXAccessibleToNode;
}
