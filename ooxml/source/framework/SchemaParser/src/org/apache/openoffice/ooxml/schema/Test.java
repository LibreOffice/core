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

package org.apache.openoffice.ooxml.schema;

import org.apache.openoffice.ooxml.schema.automaton.HopcroftMinimizer;
import org.apache.openoffice.ooxml.schema.automaton.State;
import org.apache.openoffice.ooxml.schema.automaton.StateContainer;
import org.apache.openoffice.ooxml.schema.automaton.StateContext;
import org.apache.openoffice.ooxml.schema.automaton.Transition;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** A simple test of the minimization algorithm for DFAs.
 *
 *  May lead to the use of a testing framework in the future.
 */
public class Test
{
    public static void main (final String ... aArgumentList)
    {
        new Test("S", new String[]{"E"}, new String[][]{
            {"S", "A", "a"},
            {"A", "B", "b"},
            {"A", "C", "b"},
            {"B", "E", "c"},
            {"C", "E", "c"},
        });
    }
    private Test (
        final String sStartState,
        final String[] aAcceptingStates,
        final String[][] aTransitions)
    {
        final StateContainer aOriginalStateContainer = new StateContainer();
        final StateContext aStates = new StateContext(
            aOriginalStateContainer,
            sStartState);
        for (final String sAcceptingState : aAcceptingStates)
        {
            final State s = aStates.CreateState(sAcceptingState);
            s.SetIsAccepting();
        }
        for (final String[] aTransition : aTransitions)
        {
            final State start = aStates.GetOrCreateState(
                new QualifiedName(aTransition[0]),
                null);
            final State end = aStates.GetOrCreateState(
                new QualifiedName(aTransition[1]),
                null);
            final QualifiedName element = new QualifiedName(aTransition[2]);
            final String type = "T_"+aTransition[2];

            start.AddTransition(new Transition(start, end, element, type));
        }
        HopcroftMinimizer.MinimizeDFA (
            new StateContainer(),
            aStates,
            null,
            null,
            System.out);
    }
}
