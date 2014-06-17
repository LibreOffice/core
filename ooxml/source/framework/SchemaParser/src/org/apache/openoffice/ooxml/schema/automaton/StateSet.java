package org.apache.openoffice.ooxml.schema.automaton;

import java.util.Iterator;
import java.util.Set;
import java.util.TreeSet;

import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;

/** Used in the transformation of NFA to DFA and in the minimization of DFAs.
 *  References a set of regular states.
 */
public class StateSet
    implements Comparable<StateSet>
{
    public StateSet ()
    {
        maStates = new TreeSet<>();
    }




    public StateSet (final State aState)
    {
        this();
        maStates.add(aState);
    }




    public StateSet (final StateSet aSet)
    {
        this();
        maStates.addAll(aSet.maStates);
    }




    public StateSet (final Iterable<State> aStates)
    {
        this();
        for (final State aState : aStates)
            maStates.add(aState);
    }




    public void AddState (final State aState)
    {
        maStates.add(aState);
    }




    public void AddStates (final StateSet aStates)
    {
        maStates.addAll(aStates.maStates);
    }




    public boolean IsDisjoint (final StateSet aOther)
    {
        for (final State aState : aOther.maStates)
            if (maStates.contains(aState))
                return false;
        for (final State aState : maStates)
            if (aOther.maStates.contains(aState))
                return false;
        return true;
    }




    public void RemoveState (final State aState)
    {
        maStates.remove(aState);
    }




    public Iterable<State> GetStates ()
    {
        return maStates;
    }




    public boolean ContainsState (final State aState)
    {
        return maStates.contains(aState);
    }




    public int GetStateCount ()
    {
        return maStates.size();
    }




    public boolean HasStates ()
    {
        return ! maStates.isEmpty();
    }




    public State CreateStateForStateSet (final StateContext aContext)
    {
        // Find a name for the new state.  If there is type state in the given
        // set then use its name.
        QualifiedName aBaseName = null;
        String sShortestSuffix = null;
        for (final State aState : maStates)
        {
            final QualifiedName aName = aState.GetBasename();
            final String sSuffix = aState.GetSuffix();

            if (aBaseName == null)
            {
                aBaseName = aName;
                sShortestSuffix = sSuffix;
            }
            else if (aBaseName.compareTo(aName) != 0)
            {
                System.out.printf("%s != %s\n", aBaseName, aName);
                throw new RuntimeException("state set contains states with different base names: "+toString());
            }

            if (sShortestSuffix == null)
                sShortestSuffix = sSuffix;
            else if (sSuffix.length() < sShortestSuffix.length())
                sShortestSuffix = sSuffix;
        }
        if (aBaseName == null)
            throw new RuntimeException("can not create state for "+toString());

        // Disambiguate new state name.
        State aNewState = aContext.CreateState(
            aBaseName,
            aContext.GetUnambiguousSuffix(aBaseName, sShortestSuffix));
        assert(aNewState!=null);

        // Mark the new state as accepting if at least one of its original states
        // is accepting.
        for (final State aState : maStates)
        {
            if (aState.IsAccepting())
            {
                aNewState.SetIsAccepting();
                break;
            }
            for (final SkipData aData : aState.GetSkipData())
                aNewState.AddSkipData(aData.Clone(aNewState));
            final INode aTextType = aState.GetTextType();
            if (aTextType != null)
                aNewState.SetTextType(aTextType);
        }

        return aNewState;
    }




    @Override
    public int compareTo (final StateSet aOther)
    {
        final int nStateCount = maStates.size();

        if (nStateCount != aOther.maStates.size())
            return nStateCount - aOther.maStates.size();
        else
        {
            final Iterator<State> aIterator = maStates.iterator();
            final Iterator<State> aOtherIterator = aOther.maStates.iterator();
            while (aIterator.hasNext())
            {
                final State aState = aIterator.next();
                final State aOtherState = aOtherIterator.next();
                final int nResult = aState.compareTo(aOtherState);
                if (nResult != 0)
                    return nResult;
            }
            return 0;
        }
    }




    public boolean IsEmpty()
    {
        return maStates.isEmpty();
    }




    @Override
    public String toString ()
    {
        return "set of "+maStates.size()+" states "+maStates.toString();
    }




    private final Set<State> maStates;
}
