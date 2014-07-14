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

import org.apache.openoffice.ooxml.schema.misc.Log;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;

/** Represent a single node in the node tree of a simple type.
 */
public interface ISimpleTypeNode
{
    void ApplyRestriction (
        final Restriction aNode,
        final Map<String,Integer> aValueToIdMap);
    void Print (final Log aLog);

    /** List elements are not represented by their own node (type).
     *  There is only this flag that makes this node a list of the item type
     *  which is represented by the node.
     */
    boolean IsList ();

    /** Set the IsList flag.
     */
    void SetIsList ();
    void AcceptVisitor (final ISimpleTypeNodeVisitor aVisitor);
}
