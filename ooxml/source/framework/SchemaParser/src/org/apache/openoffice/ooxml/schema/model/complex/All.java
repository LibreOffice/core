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

package org.apache.openoffice.ooxml.schema.model.complex;

import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.Node;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;

/** Representation of the 'all' XML schema element.  All its children are
 *  expected to occur in any order.  By default each child is optional but with
 *  a min=1 attribute the children can be made mandatory.
 */
public class All
    extends Node
{
    public All (
        final Node aParent,
        final Location aLocation)
    {
        super(aParent, null, aLocation);

        assert(CheckParent(aParent));
    }




    /** Occurrence values of an 'all' node must be
     *  min=(0,1)
     *  max=1.
     */
    private static boolean CheckParent (final Node aParent)
    {
        if (aParent == null)
            return false;
        if (aParent.GetNodeType() != NodeType.OccurrenceIndicator)
            // Missing occurrence parent means that min and max have the default
            // values of 0 and 1, which is valid.
            return true;

        final OccurrenceIndicator aIndicator = (OccurrenceIndicator)aParent;
        if (aIndicator.GetMinimum() > 1)
            return false;
        else if (aIndicator.GetMaximum() != 1)
            return false;
        else
            return true;
    }




    @Override
    public void AcceptVisitor (final INodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }




    @Override
    public NodeType GetNodeType()
    {
        return NodeType.All;
    }




    @Override
    public String toString ()
    {
        return "all";
    }
}
