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

package org.apache.openoffice.ooxml.schema.generator.automaton;

import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

public class Transition
{
    public static Transition CreateElementTransition (
        final State aStartState,
        final State aEndState,
        final QualifiedName aElementName,
        final IAction aAction)
    {
        return new Transition(aStartState, aEndState, aElementName, aAction);
    }




    public static Transition CreateEpsilonTransition (
        final State aStartState,
        final State aEndState)
    {
        return new Transition(aStartState, aEndState, null, null);
    }




    public static Transition CreateSkipTransition (
        final State aStartState,
        final State aEndState)
    {
        return new Transition(aStartState, aEndState, null, new SkipElementAction());
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




    public IAction GetAction()
    {
        return maAction;
    }




    public State GetActionState ()
    {
        if (maAction == null)
            return null;
        else
            return maAction.GetStartState();
    }




    private Transition (
        final State aStartState,
        final State aEndState,
        final QualifiedName aElementName,
        final IAction aAction)
    {
        maStartState = aStartState;
        maEndState = aEndState;
        maElementName = aElementName;
        maAction = aAction;
    }




    public boolean IsEpsilonTransition ()
    {
        return maElementName==null && maAction==null;
    }



    @Override
    public String toString ()
    {
        if (maElementName != null)
            return String.format("%s --'%s'-> %s (%s)",
                maStartState.GetFullname(),
                maElementName.GetDisplayName(),
                maEndState.GetFullname(),
                maAction.toString());
        else if (maAction != null)
            return String.format("%s -s-> %s (%s)",
                maStartState.GetFullname(),
                maEndState.GetFullname(),
                maAction.toString());
        else
            return String.format("%s -> %s",
            maStartState.GetFullname(),
            maEndState.GetFullname());
    }

    private final State maStartState;
    private final State maEndState;
    private final QualifiedName maElementName;
    private final IAction maAction;
}
