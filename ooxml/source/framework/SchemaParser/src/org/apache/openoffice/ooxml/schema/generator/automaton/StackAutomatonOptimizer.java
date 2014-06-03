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

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.HashSet;
import java.util.Set;
import java.util.TreeSet;
import java.util.Vector;

public class StackAutomatonOptimizer
{
    StackAutomatonOptimizer (
        final StateContext aStateContext)
    {
        maSourceStateContext = aStateContext;
        maTargetStateContext = new StateContext(
            maSourceStateContext.GetStartState().GetFullname(),
            maSourceStateContext.GetEndState().GetFullname());
        maTargetTransitions = new HashSet<Transition>();
    }




    void Optimize ()
    {
        try
        {
            maLog = new PrintStream(new FileOutputStream("/tmp/ooxml-parser/automaton-optimization.log"));
        }
        catch (FileNotFoundException e)
        {
            e.printStackTrace();
        }

        OptimizeStates();

        if (maLog != null)
            maLog.close();
    }




    StateContext GetOptimizedStateContext ()
    {
        return maTargetStateContext;
    }




    Set<Transition> GetOptimizedTransitions ()
    {
        return maTargetTransitions;
    }




    /** Optimize the given set of states and transitions so that the optimized set
     *  does not have epsilon transitions or unused states.
     */
    private void OptimizeStates ()
    {
        final long nStartTime = System.currentTimeMillis();

        OptimizeState(
            maSourceStateContext.GetStartState(),
            maTargetStateContext.GetStartState(),
            new HashSet<State>());

        final long nEndTime = System.currentTimeMillis();
        System.out.printf("optimized automaton in %fs, it has %d states and %d transitions\n",
            (nEndTime-nStartTime)/1000.0,
            maTargetStateContext.GetStateCount(),
            maTargetTransitions.size());
    }




    /** Process a single pair of source state and corresponding target state.
     *  First create a set of non-epsilon transitions that start at the source
     *  state or a state that is reachable by epsilon transitions from the source
     *  state.
     *  Then create a target transition for each of these source transitions
     *  that starts at the target state but are otherwise identical to the
     *  source transition.
     */
    private void OptimizeState (
        final State aSourceState,
        final State aTargetState,
        final Set<State> aVisitedSourceStates)
    {
        if (aVisitedSourceStates.contains(aSourceState))
            return;

        if (maLog != null)
            maLog.printf("optimizing state %s\n", aSourceState);

        aVisitedSourceStates.add(aSourceState);

        final Set<State> aTodo = new TreeSet<>();

        final Vector<Transition> aSourceTransitions = GetReachableStates(aSourceState);
        for (final Transition aSourceTransition : aSourceTransitions)
        {
            aTodo.add(aSourceTransition.GetEndState());
            if (aSourceTransition.GetActionState() != null)
                aTodo.add(aSourceTransition.GetActionState());

            final Transition aTargetTransition = Transition.CreateElementTransition(
                aTargetState,
                aSourceTransition.GetEndState().Clone(maTargetStateContext),
                aSourceTransition.GetElementName(),
                aSourceTransition.GetAction().Clone(maTargetStateContext));
            aTargetState.AddTransition(aTargetTransition);
            maTargetTransitions.add(aTargetTransition);
        }

        for (final State aOtherSourceState : aTodo)
        {
            // In order to not obfuscate the code that adds elements to the todo queue
            // we let it add null now and then.  Sort it out now.
            if (aOtherSourceState == null)
                continue;
            if (aVisitedSourceStates.contains(aOtherSourceState))
                continue;

            OptimizeState(
                aOtherSourceState,
                aOtherSourceState.Clone(maTargetStateContext),
                aVisitedSourceStates);
        }
    }




    private Vector<Transition> GetReachableStates (final State aSourceState)
    {
        final Set<State> aEpsilonFront = GetEpsilonClosure(aSourceState);
        if (maLog != null)
        {
            maLog.printf("    states reachable via epsilon transitions:\n");
            for (final State aState : aEpsilonFront)
                maLog.printf("        %s\n", aState);
        }

        final Vector<Transition> aSourceTransitions = new Vector<>();
        for (final State aEpsilonReachableState : aEpsilonFront)
            for (final Transition aTransition : aEpsilonReachableState.GetTransitions())
                if ( ! aTransition.IsEpsilonTransition())
                    aSourceTransitions.add(aTransition);

        if (maLog != null)
        {
            maLog.printf("    joined transitions are:\n");
            for (final Transition aTransition : aSourceTransitions)
                maLog.printf("        %s (via %s)\n",
                    aTransition.GetEndState(),
                    aTransition.GetActionState());
        }

        return aSourceTransitions;
    }




    /** Collect all states that are reachable via zero or more epsilon transitions
     *  from the given start state.
     */
    private Set<State> GetEpsilonClosure (final State aState)
    {
        final Set<State> aEpsilonFront = new TreeSet<State>();

        AddToEpsilonClosure(aState, aEpsilonFront, new HashSet<State>(), "        ");

        return aEpsilonFront;
    }




    private void AddToEpsilonClosure (
        final State aState,
        final Set<State> aEpsilonClosure,
        final Set<State> aVisitedStates,
        final String sIndentation)
    {
        if (aVisitedStates.contains(aState))
            return;
        aVisitedStates.add(aState);
        aEpsilonClosure.add(aState);

        // Determine the local epsilon front that originates at aState.
        final Set<State> aLocalFront = new HashSet<>();
        for (final Transition aTransition : aState.GetTransitions())
            if (aTransition.IsEpsilonTransition())
            {
                maLog.printf("%sadding state %s (from %s)\n",
                    sIndentation,
                    aTransition.GetEndState(),
                    aTransition.GetStartState());
                AddToEpsilonClosure(aTransition.GetEndState(), aLocalFront, aVisitedStates, sIndentation+" ");
            }

        // Process the short circuit.
        final State aShortCircuitEnd = aState.GetShortCircuitEnd();
        if (aShortCircuitEnd != null)
        {
            if (aLocalFront.contains(aShortCircuitEnd))
            {
                maLog.printf("    replacing short circuit from %s to %s with %s\n",
                    aState,
                    aShortCircuitEnd,
                    aState.GetShortCircuitReplacement());
                aLocalFront.remove(aShortCircuitEnd);
                aLocalFront.add(aState.GetShortCircuitReplacement());
            }
        }

        aEpsilonClosure.addAll(aLocalFront);
    }




    private final StateContext maSourceStateContext;
    private final StateContext maTargetStateContext;
    private final Set<Transition> maTargetTransitions;
    private PrintStream maLog;
}
