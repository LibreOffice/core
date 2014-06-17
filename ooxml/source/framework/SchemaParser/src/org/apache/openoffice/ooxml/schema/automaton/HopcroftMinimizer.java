package org.apache.openoffice.ooxml.schema.automaton;

import java.io.PrintStream;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Minimize an DFA with respect to its number of states.
 *  This is most important for the use of the 'all' element in the OOXML
 *  specification which leads to a lot of additional states and transitions.
 */
public class HopcroftMinimizer
{
    /** Create a DFA that is equivalent to a given DFA but has the minimal
     *  number of states.
     */
    public static FiniteAutomaton MinimizeDFA (
        final StateContainer aNewStateContainer,
        final StateContext aOriginalStates,
        final Vector<Attribute> aAttributes,
        final Location aLocation,
        final PrintStream aLog)
    {
        if (aLog != null)
        {
            aLog.printf("minimizing %d states and %d transitions\n",
                aOriginalStates.GetStateCount(),
                aOriginalStates.GetTransitionCount());
            DisplayStates(aOriginalStates, aLog);
        }

        TreeSet<StateSet> aT = new TreeSet<>();
        TreeSet<StateSet> aP = new TreeSet<>();
        Map<State,StateSet> aTMap = new HashMap<>();
        Map<State,StateSet> aPMap = new HashMap<>();
        InitializeMap(aT, aTMap, aOriginalStates.GetStates());

        // Split partitions until there is nothing else to do.
        while ( ! AreSetsOfStateSetsEqual(aP, aT))
        {
            if (aLog != null)
                aLog.printf("T has %d members\n", aT.size());

            aP = aT;
            aPMap = aTMap;
            aT = new TreeSet<>();
            aTMap = new HashMap<>();

            for (final StateSet aSet : aP)
            {
                final Iterable<StateSet> aParts = Split(aSet, aP, aPMap);
                if (aParts == null)
                {
                    // No split necessary.
                    assert( ! aSet.IsEmpty());
                    aT.add(aSet);
                    for (final State aState : aSet.GetStates())
                        aTMap.put(aState, aSet);
                }
                else
                {
                    for (final StateSet aPart : aParts)
                    {
                        assert( ! aPart.IsEmpty());
                        aT.add(aPart);

                        for (final State aState : aPart.GetStates())
                            aTMap.put(aState, aPart);
                    }
                }
            }
        }

        // Create new states.
        final StateContext aMinimizedStates = CreateNewStates(
            aP,
            aPMap,
            aNewStateContainer,
            aOriginalStates);

        if (aLog != null)
        {
            aLog.printf("to %d states and %d transitions\n",
                aMinimizedStates.GetStateCount(),
                aMinimizedStates.GetTransitionCount());
            DisplayStates(aMinimizedStates, aLog);
            for (final StateSet aSet : aT)
                aLog.printf("    %s\n", aSet.toString());
        }

        // Create and return the new minimized automaton.
        return new FiniteAutomaton(
            aMinimizedStates,
            aAttributes,
            aLocation);
    }




    /** We start with two sets.  One contains all start states (in our case
    *   just one), the other contains all other states.
    */
    private static void InitializeMap (
        final Set<StateSet> aSet,
        final Map<State,StateSet> aMap,
        final Iterable<State> aStates)
    {
        final StateSet aAcceptingStates = new StateSet();
        final StateSet aNonAcceptingStates = new StateSet();
        for (final State aState : aStates)
        {
            if (aState.IsAccepting())
            {
                aAcceptingStates.AddState(aState);
                aMap.put(aState, aAcceptingStates);
            }
            else
            {
                aNonAcceptingStates.AddState(aState);
                aMap.put(aState, aNonAcceptingStates);
            }
        }
        if (aAcceptingStates.IsEmpty())
            throw new RuntimeException("there should be at least one accepting state");
        aSet.add(aAcceptingStates);
        if ( ! aNonAcceptingStates.IsEmpty())
            aSet.add(aNonAcceptingStates);
    }




    private static Iterable<StateSet> Split (
        final StateSet aSet,
        final Set<StateSet> aT,
        final Map<State,StateSet> aTMap)
    {
        if (aSet.GetStateCount() == 1)
            return null;

        final Set<QualifiedName> aElements = CollectElementNames(aSet);
        for (final QualifiedName aElementName : aElements)
        {
            final Collection<StateSet> aPartitions = Split(aSet, aT, aTMap, aElementName);
            if (aPartitions == null)
                continue;
            if (aPartitions.size() > 1)
                return aPartitions;
        }
        return null;
    }




    /** Create a partition of the given set of states according to their
     *  transitions.
     *  All states whose transitions point to the same state set go in the same
     *  partition.
     */
    private static Collection<StateSet> Split (
        final StateSet aSet,
        final Set<StateSet> aT,
        final Map<State,StateSet> aTMap,
        final QualifiedName aElementName)
    {
        // Set up a forward map that does two steps:
        // from s via transition regarding aElementName to s'
        // from s' to a state set under aTMap(s).
        final Map<State,StateSet> aForwardMap = new HashMap<>();
        for (final State aState : aSet.GetStates())
        {
            final Transition aTransition = GetTransition(aState, aElementName);
            if (aTransition == null)
                aForwardMap.put(aState, null);
            else
                aForwardMap.put(aState, aTMap.get(aTransition.GetEndState()));
        }

        // Create the partion of aSet according to aForwardMap.  All states that map
        // to the same element go into the same state set.
        if (aForwardMap.size() == 1)
        {
            // No split necessary.
            return null;
        }
        else
        {
            // Set up a reverse map that maps that maps the values in aForwardMap to
            // new state sets whose contents are the keys in aForwardMap.
            final Map<StateSet,StateSet> aReverseMap = new HashMap<>();
            for (final Entry<State,StateSet> aEntry : aForwardMap.entrySet())
            {
                StateSet aPartitionSet = aReverseMap.get(aEntry.getValue());
                if (aPartitionSet == null)
                {
                    aPartitionSet = new StateSet();
                    aReverseMap.put(aEntry.getValue(), aPartitionSet);
                }
                aPartitionSet.AddState(aEntry.getKey());
            }
            return aReverseMap.values();
        }
    }




    private static Transition GetTransition (
        final State aState,
        final QualifiedName aElementName)
    {
        Transition aTransition = null;
        for (final Transition aCandidate : aState.GetTransitions())
            if (aCandidate.GetElementName().compareTo(aElementName) == 0)
            {
                assert(aTransition==null);
                aTransition = aCandidate;
                // break;
            }
        return aTransition;
    }




    private static Set<QualifiedName> CollectElementNames (final StateSet aSet)
    {
        final Set<QualifiedName> aNames = new TreeSet<>();
        for (final State aState : aSet.GetStates())
            for (final Transition aTransition : aState.GetTransitions())
                aNames.add(aTransition.GetElementName());

        return aNames;
    }




    private static boolean AreSetsOfStateSetsEqual (
        final TreeSet<StateSet> aSetOfSetsA,
        final TreeSet<StateSet> aSetOfSetsB)
    {
        if (aSetOfSetsA.size() != aSetOfSetsB.size())
            return false;
        else
        {
            final Iterator<StateSet> aSetIteratorA = aSetOfSetsA.iterator();
            final Iterator<StateSet> aSetIteratorB = aSetOfSetsB.iterator();
            while (aSetIteratorA.hasNext() && aSetIteratorB.hasNext())
            {
                if (aSetIteratorA.next().compareTo(aSetIteratorB.next()) != 0)
                    return false;
            }
            return true;
        }
    }




    private static StateContext CreateNewStates (
        final TreeSet<StateSet> aP,
        final Map<State,StateSet> aPMap,
        final StateContainer aNewStateContainer,
        final StateContext aOriginalStates)
    {
        final StateContext aMinimizedStates = new StateContext(
            aNewStateContainer,
            aOriginalStates.GetStartState().GetFullname());

        // Create the new states.
        final Map<State,State> aOldStateToNewStateMap = new TreeMap<>();
        for (final StateSet aSet : aP)
        {
            State aNewState = null;
            for (final State aOldState : aSet.GetStates())
            {
                if (aNewState == null)
                    aNewState = aOldState.Clone(aMinimizedStates);
                aOldStateToNewStateMap.put(aOldState, aNewState);
            }
        }

        // Create the new transitions.
        for (final StateSet aSet : aP)
        {
            final State aOldStartState = aSet.GetStates().iterator().next();
            final State aNewStartState = aOldStateToNewStateMap.get(aOldStartState);

            for (final Transition aTransition : aOldStartState.GetTransitions())
            {
                final State aOldEndState = aTransition.GetEndState();
                final State aNewEndState = aOldStateToNewStateMap.get(aOldEndState);

                // Check if the transition already exists.
                if (HasTransition(aNewStartState, aTransition.GetElementName()))
                    continue;

                aNewStartState.AddTransition(
                    new Transition(
                        aNewStartState,
                        aNewEndState,
                        aTransition.GetElementName(),
                        aTransition.GetElementTypeName()));
            }
        }

        // Transfer skip data and accepting flags.
        for (final State aOldState : aOriginalStates.GetStates())
        {
            final State aNewState = aOldStateToNewStateMap.get(aOldState);
            aNewState.CopyFrom(aOldState);
        }
        return aMinimizedStates;
    }




    private static boolean HasTransition (
        final State aState,
        final QualifiedName aElementName)
    {
        for (final Transition aTransition : aState.GetTransitions())
            if (aTransition.GetElementName().compareTo(aElementName) == 0)
                return true;
        return false;
    }




    private static void DisplayStates (
        final StateContext aStates,
        final PrintStream aLog)
    {
        for (final State aState : aStates.GetStates())
        {
            aLog.printf("  %s %s\n", aState.GetFullname(),
                aState.IsAccepting() ? "is accepting" : "");
            for (final Transition aTransition : aState.GetTransitions())
                aLog.printf("    -> %s via %s\n",
                    aTransition.GetEndState().GetFullname(),
                    aTransition.GetElementName().GetStateName());
        }
    }
}
