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

import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;


/** Represents a DFA (deterministic FA) or a NFA (non-deterministic FA).
 *  There is one automaton for each complex type and one for the top level elements.
 *  Transitions correspond to 'element' elements in the schema or a start tag in
 *  the input file.  During parsing the current automaton is pushed on a stack
 *  and the automaton that represents the complex type associated with the
 *  starting element is made the current automaton.  An end tag pops an automaton
 *  from the stack and replaces the current automaton with it.
 */
public class FiniteAutomaton
{
    FiniteAutomaton (
        final StateContext aContext,
        final Vector<Attribute> aAttributes,
        final Location aLocation)
    {
        maStateContext = aContext;
        maAttributes = aAttributes!=null
            ? aAttributes
            : new Vector<Attribute>();
        maLocation = aLocation;
    }




    public int GetStateCount ()
    {
        return maStateContext.GetStateCount();
    }




    public Iterable<State> GetStates()
    {
        return maStateContext.GetStates();
    }




    public Iterable<State> GetStatesSorted ()
    {
        return maStateContext.GetStatesSorted();
    }




    public State GetStartState ()
    {
        return maStateContext.GetStartState();
    }




    public Iterable<State> GetAcceptingStates ()
    {
        return maStateContext.GetAcceptingStates();
    }




    public FiniteAutomaton CreateDFA (
        final StateContainer aDFAContainer,
        final QualifiedName aTypeName)
    {
        return DFACreator.CreateDFAforNFA(
            aDFAContainer,
            maStateContext,
            maAttributes,
            aTypeName,
            maLocation);
    }




    public StateContext GetStateContext()
    {
        return maStateContext;
    }




    public Iterable<Transition> GetTransitions ()
    {
        final Vector<Transition> aTransitions = new Vector<>();
        for (final State aState : maStateContext.GetStates())
            for (final Transition aTransition : aState.GetTransitions())
                aTransitions.add(aTransition);
        return aTransitions;
    }




    public int GetTransitionCount()
    {
        int nTransitionCount = 0;
        for (final State aState : maStateContext.GetStates())
            nTransitionCount += aState.GetTransitionCount();
        return nTransitionCount;
    }




    public String GetTypeName ()
    {
        return maStateContext.GetStartState().GetFullname();
    }




    public Location GetLocation ()
    {
        return maLocation;
    }




    public Vector<Attribute> GetAttributes ()
    {
        return maAttributes;
    }




    private final StateContext maStateContext;
    private final Vector<Attribute> maAttributes;
    private final Location maLocation;
}
