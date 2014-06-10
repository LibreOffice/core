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

/** Representation of the 'any' XML schema element.  It specifies that its
 *  children conform to a non-standard schema.  If it is unknown than the
 *  children are to be ignored.
 */
public class Any
    extends Node
{
    public enum ProcessContents
    {
        lax,
        skip,
        strict
    }

    public Any (
        final Node aParent,
        final Location aLocation,
        final String sProcessContents,
        final String sNamespace)
    {
        super(aParent, null, aLocation);
        meProcessContents = ProcessContents.valueOf(sProcessContents);
        maNamespaces = sNamespace.split("\\s+");
    }



    @Override
    public NodeType GetNodeType ()
    {
        return NodeType.Any;
    }




    @Override
    public void AcceptVisitor (final INodeVisitor aVisitor)
    {
        aVisitor.Visit(this);
    }


    public ProcessContents GetProcessContentsFlag ()
    {
        return meProcessContents;
    }




    public String[] GetNamespaces ()
    {
        return maNamespaces;
    }




    @Override
    public String toString ()
    {
        final StringBuffer aBuffer = new StringBuffer();
        aBuffer.append("any processContents=");
        aBuffer.append(meProcessContents.toString());
        aBuffer.append(", namespaces=");
        boolean bFirst = true;
        for (final String sNamespace : maNamespaces)
        {
            if (bFirst)
                bFirst = false;
            else
                aBuffer.append('|');
            aBuffer.append(sNamespace);
        }
        return aBuffer.toString();
    }




    private final ProcessContents meProcessContents;
    private final String[] maNamespaces;
}
