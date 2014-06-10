package org.apache.openoffice.ooxml.schema.automaton;

import java.util.HashMap;
import java.util.Map;

/** A container of states that spans all StateContext objects that represent each
 *  a single complex type.
 */
public class StateContainer
{
    public StateContainer ()
    {
        maNameToStateMap = new HashMap<>();
    }




    boolean HasState (final String sFullname)
    {
        return maNameToStateMap.containsKey(sFullname);
    }




    State GetStateForFullname (final String sFullname)
    {
        return maNameToStateMap.get(sFullname);
    }




    public void AddState (final State aState)
    {
        maNameToStateMap.put(aState.GetFullname(), aState);
    }




    public void RemoveState (final State aState)
    {
        maNameToStateMap.remove(aState);
    }




    private final Map<String,State> maNameToStateMap;
}
