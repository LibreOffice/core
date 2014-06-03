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

import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.schema.NamespaceMap;

/** The model of the parser generators is a (nested) stack automaton.
 *  States correspond to complex types, transitions to elements.
 */
public class StackAutomaton
{
    StackAutomaton (
        final NamespaceMap aNamespaces,
        final StateContext aContext,
        final Set<Transition> aTransitions,
        final Map<State,Vector<Attribute>> aAttributes)
    {
        maNamespaces = aNamespaces;
        maStateContext = aContext;
        maTransitions = aTransitions;
        maAttributes = aAttributes;
    }




    public Iterable<Entry<String,String>> GetNamespaces ()
    {
        return maNamespaces;
    }




    public int GetStateCount ()
    {
        return maStateContext.GetStateCount();
    }




    public Iterable<State> GetStatesSorted ()
    {
        return maStateContext.GetStatesSorted();
    }




    public int GetTransitionCount ()
    {
        return maTransitions.size();
    }




    public Iterable<Transition> GetTransitions()
    {
        return maTransitions;
    }




    public StackAutomaton Optimize ()
    {
        final StackAutomatonOptimizer aOptimizer = new StackAutomatonOptimizer(maStateContext);
        aOptimizer.Optimize();
        return new StackAutomaton(
            maNamespaces,
            aOptimizer.GetOptimizedStateContext(),
            aOptimizer.GetOptimizedTransitions(),
            maAttributes);
    }




    private final NamespaceMap maNamespaces;
    private final StateContext maStateContext;
    private final Set<Transition> maTransitions;
    private final Map<State,Vector<Attribute>> maAttributes;
}
