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

package org.apache.openoffice.ooxml.schema.simple;

import java.util.Map;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;

public class UnionNode
    implements ISimpleTypeNode
{
    UnionNode ()
    {
        maTypes = new Vector<>();
        mbIsList = false;
    }




    @Override
    public void ApplyRestriction (
        final Restriction aNode,
        final Map<String,Integer> aValueToIdMap)
    {
        throw new RuntimeException("can not handle restriction on union");
    }





    public void AddNode (final ISimpleTypeNode aType)
    {
        if (aType instanceof UnionNode)
        {
            // Integrate union of child into this union.
            final UnionNode aChildUnion = (UnionNode)aType;
            for (final ISimpleTypeNode aChildChild : aChildUnion.maTypes)
                maTypes.add(aChildChild);
        }
        else if (aType instanceof StringNode)
        {
            // Is there already a string child?
            for (int nIndex=0; nIndex<maTypes.size(); ++nIndex)
            {
                final ISimpleTypeNode aChild = maTypes.get(nIndex);

                if (aChild instanceof StringNode)
                {
                    // Yes.  Can it be joined with the new child?
                    final ISimpleTypeNode aNewChild = ((StringNode)aChild).Join((StringNode)aType);
                    if (aNewChild != null)
                    {
                        // Yes. Replace the child with the joined child.
                        maTypes.set(nIndex, aNewChild);
                        return;
                    }
                }
            }
            // When we reach this point then there was no join possible.
            // Just add the new type.
            maTypes.add(aType);

        }
        else
            maTypes.add(aType);
    }




    @Override
    public void Print (final Log aLog)
    {
        aLog.printf("union of %d sub types\n", maTypes.size());
        aLog.StartBlock();
        for (final ISimpleTypeNode aType : maTypes)
            aType.Print(aLog);
        aLog.EndBlock();
    }




    @Override
    public boolean IsList ()
    {
        return mbIsList;
    }




    @Override
    public void SetIsList ()
    {
        mbIsList = true;
    }




    @Override
    public String toString ()
    {
        return "union";
    }


    ISimpleTypeNode[] GetChildren ()
    {
        return maTypes.toArray(new ISimpleTypeNode[maTypes.size()]);
    }




    @Override
    public void AcceptVisitor (final ISimpleTypeNodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }




    private final Vector<ISimpleTypeNode> maTypes;
    private boolean mbIsList;
}
