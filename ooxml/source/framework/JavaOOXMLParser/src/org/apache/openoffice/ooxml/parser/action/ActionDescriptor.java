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

import java.util.Vector;

/** Container of all actions that are associated with a single state.
 */
public class ActionDescriptor
{
    public ActionDescriptor (
        final int nStateId,
        final String sName)
    {
        msStateName = sName;

        maElementStartActions = null;
        maElementEndActions = null;
        maTextActions = null;
    }




    public void AddAction (
        final IAction aAction,
        final ActionTrigger eTrigger)
    {
        GetActionsForTrigger(eTrigger, true).add(aAction);
    }




    public Iterable<IAction> GetActions (
        final ActionTrigger eTrigger)
    {
        return GetActionsForTrigger(eTrigger, false);
    }




    @Override
    public String toString ()
    {
        return "actions for state "+msStateName;
    }




    private Vector<IAction> GetActionsForTrigger (
        final ActionTrigger eTrigger,
        final boolean bCreateWhenMissing)
    {
        Vector<IAction> aActions = null;
        switch(eTrigger)
        {
            case ElementStart:
                aActions = maElementStartActions;
                if (bCreateWhenMissing && aActions==null)
                {
                    aActions = new Vector<>();
                    maElementStartActions = aActions;
                }
                break;
            case ElementEnd:
                aActions = maElementEndActions;
                if (bCreateWhenMissing && aActions==null)
                {
                    aActions = new Vector<>();
                    maElementEndActions = aActions;
                }
                break;
            case Text:
                aActions = maTextActions;
                if (bCreateWhenMissing && aActions==null)
                {
                    aActions = new Vector<>();
                    maTextActions = aActions;
                }
                break;
        }
        return aActions;
    }




    private final String msStateName;
    private Vector<IAction> maElementStartActions;
    private Vector<IAction> maElementEndActions;
    private Vector<IAction> maTextActions;
}
