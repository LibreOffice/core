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

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** As there is one FA for each complex type and one for the top level elements,
 *  this container represents the whole set of schemas.
 */
public class FiniteAutomatonContainer
{
    FiniteAutomatonContainer (final StateContainer aStateContainer)
    {
        maComplexTypeNameToAutomatonMap = new HashMap<>();
    }




    public void AddAutomaton (
        final QualifiedName aElementName,
        final FiniteAutomaton aAutomaton)
    {
        maComplexTypeNameToAutomatonMap.put(aElementName, aAutomaton);
    }




    public Iterable<FiniteAutomaton> GetAutomatons()
    {
        return maComplexTypeNameToAutomatonMap.values();
    }




    public int GetAutomatonCount ()
    {
        return maComplexTypeNameToAutomatonMap.size();
    }




    public Iterable<State> GetStates()
    {
        final Vector<State> aStates = new Vector<>();
        for (final FiniteAutomaton aAutomaton : maComplexTypeNameToAutomatonMap.values())
            for (final State aState : aAutomaton.GetStates())
                aStates.add(aState);
        return aStates;
    }




    public int GetStateCount()
    {
        int nStateCount = 0;
        for (final FiniteAutomaton aAutomaton : maComplexTypeNameToAutomatonMap.values())
            nStateCount += aAutomaton.GetStateCount();
        return nStateCount;
    }




    public int GetTransitionCount ()
    {
        int nTransitionCount = 0;
        for (final FiniteAutomaton aAutomaton : maComplexTypeNameToAutomatonMap.values())
            nTransitionCount += aAutomaton.GetTransitionCount();
        return nTransitionCount;
    }




    public FiniteAutomatonContainer CreateDFAs ()
    {
        final StateContainer aDFAStateContainer = new StateContainer();
        final FiniteAutomatonContainer aDFAs = new FiniteAutomatonContainer(aDFAStateContainer);
        for (final Entry<QualifiedName, FiniteAutomaton> aEntry : maComplexTypeNameToAutomatonMap.entrySet())
        {
            aDFAs.AddAutomaton(
                aEntry.getKey(),
                aEntry.getValue().CreateDFA(
                    aDFAStateContainer,
                    aEntry.getKey()));
        }
        return aDFAs;
    }




    public FiniteAutomatonContainer MinimizeDFAs ()
    {
        PrintStream aLog = null;
        try
        {
            aLog = new PrintStream(new FileOutputStream(new File("/tmp/minimization.log")));
        }
        catch(Exception e)
        {
            e.printStackTrace();
            return null;
        }

        final StateContainer aNewStateContainer = new StateContainer();
        final FiniteAutomatonContainer aDFAs = new FiniteAutomatonContainer(aNewStateContainer);
        for (final Entry<QualifiedName, FiniteAutomaton> aEntry : maComplexTypeNameToAutomatonMap.entrySet())
        {
            aDFAs.AddAutomaton(
                aEntry.getKey(),
                HopcroftMinimizer.MinimizeDFA(
                    aNewStateContainer,
                    aEntry.getValue().GetStateContext(),
                    aEntry.getValue().GetAttributes(),
                    aEntry.getValue().GetLocation(),
                    aLog));
        }
        return aDFAs;
    }




    public FiniteAutomaton GetTopLevelAutomaton ()
    {
        return maComplexTypeNameToAutomatonMap.get(null);
    }




    private final Map<QualifiedName, FiniteAutomaton> maComplexTypeNameToAutomatonMap;
}
