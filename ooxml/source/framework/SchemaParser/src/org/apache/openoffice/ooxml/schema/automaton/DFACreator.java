package org.apache.openoffice.ooxml.schema.automaton;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Queue;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Convert an NFA into a DFA via the powerset construction (also called subset
 *  construction).
 */
public class DFACreator
{
    /** For a given non-deterministic finite automaton create an equivalent
     *  deterministic finite automaton.
     */
    public static FiniteAutomaton CreateDFAforNFA (
        final StateContainer aDFAStateContainer,
        final StateContext aNFAStateContext,
        final Vector<Attribute> aAttributes,
        final QualifiedName aTypeName)
    {
        final DFACreator aCreator = new DFACreator(aDFAStateContainer, aNFAStateContext, aTypeName);
        aCreator.CreateDFAforNFA();
        return new FiniteAutomaton(
            aCreator.maDFAStateContext,
            aAttributes);
    }




    private DFACreator (
        final StateContainer aDFAStateContainer,
        final StateContext aNFAStateContext,
        final QualifiedName aTypeName)
    {
        maNFAStateContext = aNFAStateContext;

        // Create the set of state sets where each element corresponds to a
        // state in the DFA.
        maNFASetToDFAStateMap = new TreeMap<>();
        maDFAStateContext = new StateContext(
            aDFAStateContainer,
            aTypeName == null
                ? "<TOP-LEVEL>"
                : aTypeName.GetStateName());

        maDFATransitions = new HashSet<>();
        maAcceptingDFAStates = new Vector<>();
    }




    private void CreateDFAforNFA ()
    {
        final State aNFAStartState = maNFAStateContext.GetStartState();

        // Initialize the creation process by adding the epsilon closure of the
        // original start state to the work list.
        final StateSet aStartSet = GetEpsilonClosure(new StateSet(aNFAStartState));
        maNFASetToDFAStateMap.put(aStartSet, maDFAStateContext.GetStartState());

        PropagateStateFlags(aStartSet, maDFAStateContext.GetStartState());

        final Queue<StateSet> aWorklist = new LinkedList<>();
        aWorklist.add(aStartSet);

        while ( ! aWorklist.isEmpty())
        {
            final Collection<StateSet> aAdditionalWorkList = ProcessTransitionFront(
                aWorklist.poll());

            aWorklist.addAll(aAdditionalWorkList);
        }
    }




    private Collection<StateSet> ProcessTransitionFront (
        final StateSet aSet)
    {
        final Set<StateSet> aLocalWorklist = new TreeSet<>();

        // Find all regular transitions that start from any state in the set.
        final Map<String,Vector<Transition>> aTransitions = GetTransitionFront(aSet);

        // Create new state sets for states that are reachable via the same element and
        // the following epsilon transitions.
        for (final Entry<String,Vector<Transition>> aEntry : aTransitions.entrySet())
        {
            // Create new state sets for both the end state of the transition.
            final StateSet aEpsilonClosure = GetEpsilonClosure(GetEndStateSet(aEntry.getValue()));

            // When these are new state sets then add them to the worklist
            // and the set of sets.
            State aDFAState = maNFASetToDFAStateMap.get(aEpsilonClosure);
            if (aDFAState == null)
            {
                aLocalWorklist.add(aEpsilonClosure);
                aDFAState = aEpsilonClosure.CreateStateForStateSet(maDFAStateContext);
                PropagateStateFlags(aEpsilonClosure, aDFAState);
                maNFASetToDFAStateMap.put(aEpsilonClosure, aDFAState);
                if (aDFAState.IsAccepting())
                    maAcceptingDFAStates.add(aDFAState);
            }

            final State aStartState = maNFASetToDFAStateMap.get(aSet);
            final QualifiedName aElementName = GetElementName(aEntry.getValue());
            final String sElementTypeName = GetElementTypeName(aEntry.getValue());
            assert(aElementName != null);
            final Transition aTransition = new Transition(
                aStartState,
                aDFAState,
                aElementName,
                sElementTypeName);
            aStartState.AddTransition(aTransition);
            maDFATransitions.add(aTransition);
        }

        return aLocalWorklist;
    }




    private QualifiedName GetElementName (final Vector<Transition> aTransitions)
    {
        for (final Transition aTransition : aTransitions)
            return aTransition.GetElementName();
        return null;
    }




    private String GetElementTypeName (final Vector<Transition> aTransitions)
    {
        for (final Transition aTransition : aTransitions)
            return aTransition.GetElementTypeName();
        return null;
    }




    /** Return the epsilon closure of the given set of states.
     *  The result is the set of all states that are reachable via zero, one or
     *  more epsilon transitions from at least one state in the given set of
     *  states.
     */
    private StateSet GetEpsilonClosure ( final StateSet aSet)
    {
        final StateSet aClosure = new StateSet(aSet);

        final Queue<State> aWorkList = new LinkedList<>();
        for (final State aState : aSet.GetStates())
            aWorkList.add(aState);

        while( ! aWorkList.isEmpty())
        {
            final State aState = aWorkList.poll();
            for (final EpsilonTransition aTransition : aState.GetEpsilonTransitions())
            {
                final State aEndState = aTransition.GetEndState();
                if ( ! aClosure.ContainsState(aEndState))
                {
                    aClosure.AddState(aEndState);
                    aWorkList.add(aEndState);
                }
            }
        }

        return aClosure;
    }




    /** Return the list of regular transitions (i.e. not epsilon transitions)
     *  that start from any of the states in the given set.
     *  The returned map is a partition of the transitions according to their
     *  triggering XML element.
     */
    private Map<String, Vector<Transition>> GetTransitionFront (final StateSet aSet)
    {
        final Map<String, Vector<Transition>> aTransitions = new HashMap<>();

        for (final State aState : aSet.GetStates())
            for (final Transition aTransition : aState.GetTransitions())
            {
                final String sElementName;
                final QualifiedName aElementName = aTransition.GetElementName();
                if (aElementName != null)
                    sElementName = aElementName.GetDisplayName();
                else
                    sElementName = null; // For skip transitions.

                Vector<Transition> aElementTransitions = aTransitions.get(sElementName);
                if (aElementTransitions == null)
                {
                    aElementTransitions = new Vector<>();
                    aTransitions.put(sElementName, aElementTransitions);
                }
                aElementTransitions.add(aTransition);
            }
        return aTransitions;
    }




    /** Return a state set that contains all end states of all the given transitions.
     */
    private StateSet GetEndStateSet (final Iterable<Transition> aTransitions)
    {
        final StateSet aStateSet = new StateSet();
        for (final Transition aTransition : aTransitions)
            aStateSet.AddState(aTransition.GetEndState());
        return aStateSet;
    }




    /** Propagate accepting state flag and skip data.
     */
    private void PropagateStateFlags (
        final StateSet aNFAStateSet,
        final State aDFAState)
    {
        for (final State aNFAState : aNFAStateSet.GetStates())
        {
            if (aNFAState.IsAccepting())
                aDFAState.SetIsAccepting();

            for (final SkipData aSkipData : aNFAState.GetSkipData())
                aDFAState.AddSkipData(aSkipData.Clone(aDFAState));
        }
    }




    private final StateContext maNFAStateContext;

    private final Map<StateSet,State> maNFASetToDFAStateMap;
    private final StateContext maDFAStateContext;
    private final Set<Transition> maDFATransitions;
    private final Vector<State> maAcceptingDFAStates;
}
