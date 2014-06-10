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

package org.apache.openoffice.ooxml.schema.automaton;

import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Each transition corresponds to an 'element' schema element.
 *  It moves from the start state to the end state when a certain start tag is
 *  processed.  The element corresponds to another complex type which will be
 *  parsed during the transition.
 */
public class Transition
{
    public Transition (
        final State aStartState,
        final State aEndState,
        final QualifiedName aElementName,
        final String sElementTypeName)
    {
        maStartState = aStartState;
        maEndState = aEndState;
        maElementName = aElementName;
        msElementTypeName = sElementTypeName;
    }




    public State GetStartState ()
    {
        return maStartState;
    }




    public State GetEndState ()
    {
        return maEndState;
    }




    public QualifiedName GetElementName()
    {
        return maElementName;
    }




    public String GetElementTypeName()
    {
        return msElementTypeName;
    }




    @Override
    public String toString ()
    {
        return String.format("%s --'%s'-> %s (via %s)\n",
            maStartState.GetFullname(),
            maElementName.GetDisplayName(),
            maEndState.GetFullname(),
            msElementTypeName);
    }




    private final State maStartState;
    private final State maEndState;
    private final QualifiedName maElementName;
    private final String msElementTypeName;
}
