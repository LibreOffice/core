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

package org.apache.openoffice.ooxml.schema.model.base;

import java.util.Vector;

/** Base class for the nodes in the type hierarchies that represent complex
 *  types, simple types, and attributes.
 */
public abstract class Node
    implements INode, Comparable<Node>
{
    abstract public NodeType GetNodeType ();



    protected Node (
        final Node aParent,
        final QualifiedName aName,
        final Location aLocation)
    {
        maParent = aParent;
        maName = aName;

        maLocation = aLocation;
        maAttributes = new Vector<>();
        maChildren = new Vector<>();
    }




    @Override
    public QualifiedName GetName ()
    {
        return maName;
    }




    public Node GetParent ()
    {
        return maParent;
    }




    /** Store the location in the schema files.  This is used for debugging or the creation of documentation.
     */
    public void SetLocation (
        final Location aLocation)
    {
        maLocation = aLocation;
    }




    public Location GetLocation ()
    {
        return maLocation;
    }




    public void AddAttribute (final INode aAttribute)
    {
        maAttributes.add(aAttribute);
    }




    @Override
    public int GetAttributeCount ()
    {
        return maAttributes.size();
    }




    @Override
    public Iterable<INode> GetAttributes ()
    {
        return maAttributes;
    }



    public void ClearChildren ()
    {
        maChildren.clear();
    }



    public void AddChild (final INode aChild)
    {
        maChildren.add(aChild);
    }




    public Iterable<INode> GetChildren ()
    {
        return maChildren;
    }




    public INode GetOnlyChild ()
    {
        assert(maChildren.size() == 1);
        return maChildren.get(0);
    }




    public int GetChildCount ()
    {
        return maChildren.size();
    }




    @Override
    public int compareTo (final Node aOther)
    {
        if (maName==null || aOther.maName==null)
        {
            throw new RuntimeException("can not compare nodes that don't have a name");
        }
        else
            return maName.compareTo(aOther.maName);
    }




    private final Node maParent;
    private final QualifiedName maName;
    private Location maLocation;
    private final Vector<INode> maAttributes;
    private final Vector<INode> maChildren;
}
