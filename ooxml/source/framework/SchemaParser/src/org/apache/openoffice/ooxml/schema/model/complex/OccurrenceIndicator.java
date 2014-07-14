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

/** An occurrence indicator is based on a minimum and a maximum value.
 *  The minimum value is 0 or larger.  If it is 0 then the child node is optional.
 *  The maximum value is at least as large as the minimum value.
 *  It can be 'unbounded' in which case the child node can appear any number of times.
 *  'Unbounded' is internally stored as -1.
 *
 *  An OccurrenceIndicator represents the minOccur and maxOccur attributes of
 *  XML schema elements.
 *
 *  There is usually exactly one child.
 */
public class OccurrenceIndicator
    extends Node
{
    public static int unbounded = -1;

    public OccurrenceIndicator (
        final Node aParent,
        final String sMinimum,
        final String sMaximum,
        final Location aLocation)
    {
        super(aParent, null, aLocation);

        mnMinimum = ParseValue(sMinimum);
        mnMaximum = ParseValue(sMaximum);

        assert(mnMinimum>=0);
        assert(mnMaximum==unbounded || mnMaximum>=mnMinimum);
    }




    @Override
    public NodeType GetNodeType ()
    {
        return NodeType.OccurrenceIndicator;
    }




    /** Return a string version of the minimum value for textual display.
     */
    public String GetDisplayMinimum ()
    {
        return Integer.toString(mnMinimum);
    }




    /** Return a string version of the maximum value for textual display.
     */
    public String GetDisplayMaximum ()
    {
        if (mnMaximum == unbounded)
            return "unbounded";
        else
            return Integer.toString(mnMaximum);
    }




    public int GetMinimum ()
    {
        return mnMinimum;
    }




    public int GetMaximum ()
    {
        return mnMaximum;
    }




    @Override
    public void AcceptVisitor (final INodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }




    @Override
    public String toString ()
    {
        return String.format("occurrence %s to %s",
            GetDisplayMinimum(),
            GetDisplayMaximum());
    }




    private static int ParseValue (final String sValue)
    {
        if (sValue == null)
        {
            // Missing values default to 1.
            return 1;
        }
        else
            switch (sValue)
            {
                case "0" : return 0;
                case "1" : return 1;
                case "unbounded" : return unbounded;
                default: return Integer.parseInt(sValue);
            }
    }



    private final int mnMinimum;
    private final int mnMaximum;
}
