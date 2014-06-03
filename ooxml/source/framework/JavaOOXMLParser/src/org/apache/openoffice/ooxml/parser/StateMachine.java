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

package org.apache.openoffice.ooxml.parser;

import java.io.File;
import java.util.Set;
import java.util.Stack;

import javax.xml.stream.Location;

/** The state machine is initialized at creation from the data tables
 *  created previously by a stack automaton.
 */
public class StateMachine
{
    public StateMachine (final File aDataLocation)
    {
        maNamespaceMap = new NamespaceMap(aDataLocation);
        maNameMap = new NameMap(aDataLocation);
        maTransitions = new TransitionTable(aDataLocation);
        maSkipStates = maTransitions.GetSkipStates();
        maAttributeManager = new AttributeManager(aDataLocation);

        mnStartStateId = maNameMap.GetIdForName(null, "_start_");
        mnEndStateId = maNameMap.GetIdForName(null, "_end_");
        mnCurrentStateId = mnStartStateId;
        maStateStack = new Stack<>();
        Log.Dbg.printf("starting in state _start_ (%d)\n", mnCurrentStateId);
    }




    public boolean ProcessStartElement (
        final String sNamespaceURI,
        final String sElementName,
        final Location aLocation,
        final AttributeProvider aAttributes)
    {
        final String sPrefix = maNamespaceMap.GetPrefixForURI(sNamespaceURI);
        boolean bResult = false;
        try
        {
            final int nElementId = maNameMap.GetIdForName(sPrefix, sElementName);
            Log.Dbg.printf("%s:%s(%d, aArgumentList) L%dC%d\n",
                sPrefix,
                sElementName,
                nElementId,
                aLocation.getLineNumber(),
                aLocation.getColumnNumber());

            final Transition aTransition = maTransitions.GetTransition(
                mnCurrentStateId,
                nElementId);
            if (aTransition == null)
            {
                Log.Err.printf(
                    "can not find transition for state %s and element %s at L%dC%d\n",
                    maNameMap.GetNameForId(mnCurrentStateId),
                    maNameMap.GetNameForId(nElementId),
                    aLocation.getLineNumber(),
                    aLocation.getColumnNumber());
            }
            else
            {
                Log.Dbg.printf(" %s(%d) -> %s(%d) via %s(%d)",
                    maNameMap.GetNameForId(mnCurrentStateId),
                    mnCurrentStateId,
                    maNameMap.GetNameForId(aTransition.GetEndStateId()),
                    aTransition.GetEndStateId(),
                    maNameMap.GetNameForId(aTransition.GetActionId()),
                    aTransition.GetActionId());
                Log.Dbg.printf("\n");

                final int nOldState = mnCurrentStateId;
                SetCurrentState(aTransition.GetEndStateId());

                ExecuteActions(aTransition, aAttributes, nOldState, mnCurrentStateId);

                bResult = true;
            }
        }
        catch (RuntimeException aException)
        {
            aException.printStackTrace();
        }
        return bResult;
    }




    public void ProcessEndElement (
        final String sNamespaceURI,
        final String sElementName,
        final Location aLocation)
    {
        final String sPrefix = maNamespaceMap.GetPrefixForURI(sNamespaceURI);

        final int nOldStateId = mnCurrentStateId;
        SetCurrentState(maStateStack.pop());

        Log.Dbg.DecreaseIndentation();
        Log.Dbg.printf("/%s:%s L%d%d\n",
            sPrefix,
            sElementName,
            aLocation.getLineNumber(),
            aLocation.getColumnNumber());
        Log.Dbg.printf(" %s(%d) <- %s(%d)\n",
            maNameMap.GetNameForId(nOldStateId),
            nOldStateId,
            maNameMap.GetNameForId(mnCurrentStateId),
            mnCurrentStateId);
    }




    public void ProcessCharacters (
        final String sText)
    {
    }




    public boolean IsInSkipState ()
    {
        return maSkipStates.contains(mnCurrentStateId);
    }




    private void SetCurrentState (final int nState)
    {
        if (mnCurrentStateId != nState)
        {
            if (nState == mnEndStateId)
                mnCurrentStateId = mnStartStateId;
            else
                mnCurrentStateId = nState;
        }
    }




    private void ExecuteActions (
        final Transition aTransition,
        final AttributeProvider aAttributes,
        final int nOldState,
        final int nNewState)
    {
        switch(aTransition.GetAction())
        {
            case 'p' :
                // Parse action.
                maStateStack.push(mnCurrentStateId);
                Log.Dbg.IncreaseIndentation();
                final int nActionId = aTransition.GetActionId();
                SetCurrentState(nActionId);
                maAttributeManager.ParseAttributes(nActionId, aAttributes);
                break;

            default:
                throw new RuntimeException();
        }
    }




    private final NamespaceMap maNamespaceMap;
    private final NameMap maNameMap;
    private final TransitionTable maTransitions;
    private final AttributeManager maAttributeManager;
    private int mnCurrentStateId;
    private Stack<Integer> maStateStack;
    private final int mnStartStateId;
    private final int mnEndStateId;
    private static Set<Integer> maSkipStates;
}
