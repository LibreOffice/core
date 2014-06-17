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

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Represents the set of states of a single complex type.
 *
 *  Because states have to be unique, the state container is an object shared
 *  by all StateContext objects.
 *
 *  There is a single start state but there can be more than one accepting state.
 */
public class StateContext
{
    public StateContext (
        final StateContainer aStateContainer,
        final String sBaseStateName)
    {
        maStateContainer = aStateContainer;
        maStates = new HashSet<>();
        maStartState = GetOrCreateState(new QualifiedName(null, null, sBaseStateName), null);
        maDisambiguateCounters = new HashMap<>();
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



    public State CreateState (final String sBasename)
    {
        return CreateState(new QualifiedName(sBasename), null);
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
        return maStateContainer.GetStateForFullname(State.GetStateName(aBasename, sSuffix));
    }




    public State GetOrCreateState (
        final QualifiedName aBasename,
        final String sSuffix)
    {
        State aState = GetState(aBasename, sSuffix);
        if (aState == null)
        {
            aState = CreateState(aBasename, sSuffix);
            AddState(aState);
        }
        return aState;
    }




    public State GetStartStateForTypeName (final QualifiedName aName)
    {
        return GetOrCreateState(aName, null);
    }




    public State CreateEndState ()
    {
        final State aEndState = CreateState(
            maStartState.GetBasename(),
            "end");
        aEndState.SetIsAccepting();
        return aEndState;
    }



    /** Some algorithms can not easily produce unique suffixes.
     *  Append an integer to the given suffix so that it becomes unique.
     */
    public String GetUnambiguousSuffix (final QualifiedName aBasename, final String sSuffix)
    {
        String sStateName = State.GetStateName(aBasename, sSuffix);
        if ( ! HasState(sStateName))
        {
            // The given suffix can be used without modification.
            return sSuffix;
        }
        else
        {
            int nIndex = 2;
            final Integer nDisambiguateCounter = maDisambiguateCounters.get(sStateName);
            if (nDisambiguateCounter != null)
                nIndex = nDisambiguateCounter+1;
            maDisambiguateCounters.put(sStateName, nIndex);

            return sSuffix + "_" + nIndex;
        }
    }




    public boolean HasState (
        final QualifiedName aBasename,
        final String sSuffix)
    {
        return maStateContainer.HasState(State.GetStateName(aBasename, sSuffix));
    }




    /** Return whether a state with the given name already belongs to the state
     *  context.
     */
    public boolean HasState (final String sFullname)
    {
        return maStateContainer.HasState(sFullname);
    }




    /** The start state is the state a parser is in initially.
     */
    public State GetStartState ()
    {
        return maStartState;
    }




    public Iterable<State> GetAcceptingStates ()
    {
        final Vector<State> aAcceptingStates = new Vector<>();
        for (final State aState : maStates)
            if (aState.IsAccepting())
                aAcceptingStates.add(aState);
        return aAcceptingStates;
    }




    /** Add the given state to the state context.
     */
    public void AddState (final State aState)
    {
        maStateContainer.AddState(aState);
        maStates.add(aState);
    }




    public void RemoveState (final State aState)
    {
        maStateContainer.RemoveState(aState);
        maStates.remove(aState);
    }




    public int GetStateCount ()
    {
        return maStates.size();
    }




    public Iterable<State> GetStatesSorted()
    {
        final Set<State> aSortedStates = new TreeSet<>();
        aSortedStates.addAll(maStates);
        return aSortedStates;
    }




    public Iterable<State> GetStates()
    {
        return maStates;
    }




    public int GetTransitionCount ()
    {
        int nStateCount = 0;
        for (final State aState : maStates)
            nStateCount += aState.GetTransitionCount();
        return nStateCount;
    }




    private final StateContainer maStateContainer;
    private final Set<State> maStates;
    private final State maStartState;
    private final Map<String,Integer> maDisambiguateCounters;
}
