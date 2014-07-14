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

package org.apache.openoffice.ooxml.parser.action;

import java.util.HashMap;
import java.util.Map;

import org.apache.openoffice.ooxml.parser.NameMap;

/** Manage actions that are bound to states and XML events.
 */
public class ActionManager
{
    public ActionManager (
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
