package org.apache.openoffice.ooxml.schema.automaton;

/** Transition from one state to another that does not consume an input token.
 *
 *  Use in the process of creating a validating parser.
 */
public class EpsilonTransition
{
    EpsilonTransition (
        final State aStartState,
        final State aEndState)
    {
        maStartState = aStartState;
        maEndState = aEndState;
    }




    public State GetStartState ()
    {
        return maStartState;
    }




    public State GetEndState ()
    {
        return maEndState;
    }




    @Override
    public String toString ()
    {
        return String.format("%s -> %s",
            maStartState.GetFullname(),
            maEndState.GetFullname());
    }




    private final State maStartState;
    private final State maEndState;
}
