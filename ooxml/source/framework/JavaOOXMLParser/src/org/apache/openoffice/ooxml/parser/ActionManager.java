package org.apache.openoffice.ooxml.parser;

import java.util.HashMap;
import java.util.Map;

/** Manage actions that are bound to states and XML events.
 */
public class ActionManager
{
    ActionManager (
        final NameMap aStateNameToIdMap)
    {
        maStateNameToIdMap = aStateNameToIdMap;
        maAllStatesActions = new ActionDescriptor(0,"*");
        maStateToActionsMap = new HashMap<>();
    }




    /** Add an action for an element start.
     *  @param sStateSelector
     *      The element is specified via a state name.  This allows one element
     *      that leads to different complex types to have different actions,
     *      depending on the complex type.
     *      The selector value can be a full state name (including the namespace
     *      prefix and CT prefix, e.g. w06_CT_Table) or a regular expression
     *      (e.g. .*_CT_Table to match w06_CT_Table and w12_CT_Table).
     *      The action is bound to all matching states.
     *  @param aAction
     *      The action to call on entering any of the states that match the
     *      selector.
     */
    public void AddElementStartAction (
        final String sStateSelector,
        final IAction aAction)
    {
        AddAction(sStateSelector, aAction, ActionTrigger.ElementStart);
    }




    /** Add an action for an element end.
     *  @see AddElementStartAction.
     */
    public void AddElementEndAction (
        final String sStateSelector,
        final IAction aAction)
    {
        AddAction(sStateSelector, aAction, ActionTrigger.ElementEnd);
    }




    /** Add an action for XML text events.
     *  @see AddElementStartAction.
     */
    public void AddTextAction (
        final String sStateSelector,
        final IAction aAction)
    {
        AddAction(sStateSelector, aAction, ActionTrigger.Text);
    }




    /** Return an iterable object that gives access to all actions
     *  bound to the given state and trigger.
     *  Return value can be null when there are no actions bound to the state
     *  and trigger.
     */
    public Iterable<IAction> GetActions (
        final int nStateId,
        final ActionTrigger eTrigger)
    {
        final ActionDescriptor aOneStateActionsDescriptor = maStateToActionsMap.get(nStateId);
        final Iterable<IAction> aOneStateActions = aOneStateActionsDescriptor!=null
            ? aOneStateActionsDescriptor.GetActions(eTrigger)
            : null;
        final Iterable<IAction> aAllStateActions = maAllStatesActions.GetActions(eTrigger);

        if (aOneStateActions == null)
            return aAllStateActions;
        else if (aAllStateActions == null)
            return aOneStateActions;
        else
            return new ActionIterator(aOneStateActions, aAllStateActions);
    }




    private void AddAction (
        final String sStateSelector,
        final IAction aAction,
        final ActionTrigger eTrigger)
    {
        if (sStateSelector.equals("*"))
        {
            // Simple optimization when an action is defined for all states.
            maAllStatesActions.AddAction(aAction, eTrigger);
        }
        else if (sStateSelector.contains("*") || sStateSelector.contains("?"))
        {
            // The state selector contains wildcards.  We have to iterate over
            // all state names to find the matching ones.
            for (final int nStateId : maStateNameToIdMap.GetMatchingStateIds(sStateSelector))
            {
                GetActionDescriptor(nStateId).AddAction(aAction, eTrigger);
            }
        }
        else
        {
            final int nStateId = maStateNameToIdMap.GetIdForName(sStateSelector);
            GetActionDescriptor(nStateId).AddAction(aAction, eTrigger);
        }
    }




    private ActionDescriptor GetActionDescriptor (final int nStateId)
    {
        ActionDescriptor aDescriptor = maStateToActionsMap.get(nStateId);
        if (aDescriptor == null)
        {
            aDescriptor = new ActionDescriptor(nStateId, maStateNameToIdMap.GetNameForId(nStateId));
            maStateToActionsMap.put(nStateId, aDescriptor);
        }
        return aDescriptor;
    }




    private final NameMap maStateNameToIdMap;
    private final ActionDescriptor maAllStatesActions;
    private final Map<Integer,ActionDescriptor> maStateToActionsMap;
}
