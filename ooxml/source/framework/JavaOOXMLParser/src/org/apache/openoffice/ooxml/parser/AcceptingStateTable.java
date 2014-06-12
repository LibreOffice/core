package org.apache.openoffice.ooxml.parser;

import java.util.HashSet;
import java.util.Set;

/** List of all accepting states.
 *
 *  The accepting status of states is important when a closing tag is seen.
 *  It denotes the end of the input stream for the state machine of the currently
 *  processed element.  It is an error when the current state is not accepting
 *  when a closing tag is processed.
 */
public class AcceptingStateTable
{
    public AcceptingStateTable (final Iterable<String[]> aData)
    {
        maAcceptingStates = new HashSet<>();

        for (final String[] aLine : aData)
        {
            // Create new transition.
            final int nStateId = Integer.parseInt(aLine[1]);

            maAcceptingStates.add(nStateId);
        }
    }




    public boolean Contains (final int nStateId)
    {
        return maAcceptingStates.contains(nStateId);
    }




    public int GetAcceptingStateCount ()
    {
        return maAcceptingStates.size();
    }




    private final Set<Integer> maAcceptingStates;
}
