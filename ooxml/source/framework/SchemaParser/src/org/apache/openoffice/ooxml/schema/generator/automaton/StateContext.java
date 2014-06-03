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

import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;

import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Represents a set of state.
 *  There is a single start state.
 */
public class StateContext
{
    public StateContext (
        final String sStartStateName,
        final String sEndStateName)
    {
        maStates = new HashMap<>();
        maStartState = CreateState(new QualifiedName(null, null, sStartStateName), null);
        maEndState = CreateState(new QualifiedName(null, null, sEndStateName), null);
    }



    public State CreateState (
        final QualifiedName aBasename,
        final String sSuffix)
    {
        final String sFullname = State.GetStateName(aBasename, sSuffix);
        if (HasState(sFullname))
            throw new RuntimeException("state with name '"+sFullname+"' can not be created because it already exists");
        final State aState = new State(aBasename, sSuffix);
        AddState(aState);
        return aState;
    }




    public State CreateState (
        final State aState,
        final String sSuffix)
    {
        if (sSuffix==null && aState.GetSuffix()==null)
            return CreateState(aState.GetBasename(), null);
        else if (sSuffix!=null && aState.GetSuffix()!=null)
            return CreateState(aState.GetBasename(), aState.GetSuffix()+"_"+sSuffix);
        else if (sSuffix != null)
            return CreateState(aState.GetBasename(), sSuffix);
        else
            return CreateState(aState.GetBasename(), aState.GetSuffix());
    }




    public State GetState (
        final QualifiedName aBasename,
        final String sSuffix)
    {
        return maStates.get(State.GetStateName(aBasename, sSuffix));
    }




    public State GetOrCreateState (
        final QualifiedName aBasename,
        final String sSuffix)
    {
        State aState = maStates.get(State.GetStateName(aBasename, sSuffix));
        if (aState == null)
        {
            aState = new State(aBasename, sSuffix);
            AddState(aState);
        }
        return aState;
    }




    public State GetStateForTypeName (final QualifiedName aName)
    {
        State aState = maStates.get(aName.GetStateName());
        if (aState == null)
        {
            aState = new State(aName, null);
            AddState(aState);
        }
        return aState;
    }




    /** The start state is the state a parser is in initially.
     */
    public State GetStartState ()
    {
        return maStartState;
    }




    public State GetEndState ()
    {
        return maEndState;
    }




    /** Return whether a state with the given name already belongs to the state
     *  context.
     */
    public boolean HasState (final String sFullname)
    {
        return maStates.containsKey(sFullname);
    }




    /** Add the given state to the state context.
     */
    public void AddState (final State aState)
    {
        maStates.put(aState.GetFullname(), aState);
    }




    public int GetStateCount ()
    {
        return maStates.size();
    }




    public Iterable<State> GetStatesSorted()
    {
        final Set<State> aSortedStates = new TreeSet<>();
        aSortedStates.addAll(maStates.values());
        return aSortedStates;
    }




    private final Map<String,State> maStates;
    private final State maStartState;
    private final State maEndState;
}
