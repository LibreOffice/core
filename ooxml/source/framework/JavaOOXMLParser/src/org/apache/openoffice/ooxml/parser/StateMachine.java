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
import java.util.Stack;

import javax.xml.stream.Location;

/** The state machine is initialized at creation from the data tables
 *  created previously by a stack automaton.
 */
public class StateMachine
{
    public StateMachine (final File aParseTableFile)
    {
        if (Log.Dbg != null)
            Log.Dbg.printf("reading parse tables from %s\n", aParseTableFile.toString());

        final ParseTableReader aReader = new ParseTableReader(aParseTableFile);
        maNamespaceMap = new NamespaceMap(aReader.GetSection("namespace"));
        maNameMap = new NameMap(aReader.GetSection("name"));
        maStateNameMap = new NameMap(aReader.GetSection("state-name"));
        maTransitions = new TransitionTable(aReader.GetSection("transition"));
        maSkipStates = new SkipStateTable(aReader.GetSection("skip"));
        maAcceptingStates = new AcceptingStateTable(aReader.GetSection("accepting-state"));
        maAttributeManager = new AttributeManager(
            aReader.GetSection("attribute"),
            maNamespaceMap,
            maNameMap);

        System.out.printf("read %d namespace, %d names, %d states (%d skip, %d accept), %d transitions and %d attributes\n",
                maNamespaceMap.GetNamespaceCount(),
                maNameMap.GetNameCount(),
                maStateNameMap.GetNameCount(),
                maSkipStates.GetSkipStateCount(),
                maAcceptingStates.GetAcceptingStateCount(),
                maTransitions.GetTransitionCount(),
                maAttributeManager.GetAttributeCount());

        mnStartStateId = Integer.parseInt(aReader.GetSection("start-state").firstElement()[1]);
        mnEndStateId = Integer.parseInt(aReader.GetSection("end-state").firstElement()[1]);
        mnCurrentStateId = mnStartStateId;
        maStateStack = new Stack<>();

        if (Log.Dbg != null)
            Log.Dbg.printf("starting in state _start_ (%d)\n", mnCurrentStateId);
    }




    public boolean ProcessStartElement (
        final String sNamespaceURI,
        final String sElementName,
        final Location aLocation,
        final AttributeProvider aAttributes)
    {
        boolean bResult = false;

        try
        {
            final NamespaceMap.NamespaceDescriptor aDescriptor = maNamespaceMap.GetDescriptorForURI(sNamespaceURI);
            final int nElementNameId = maNameMap.GetIdForName(sElementName);
            if (Log.Dbg != null)
                Log.Dbg.printf("%s:%s(%d:%d) L%dC%d\n",
                    aDescriptor.Prefix,
                    sElementName,
                    aDescriptor.Id,
                    nElementNameId,
                    aLocation.getLineNumber(),
                    aLocation.getColumnNumber());

            final Transition aTransition = maTransitions.GetTransition(
                mnCurrentStateId,
                aDescriptor.Id,
                nElementNameId);
            if (aTransition == null)
            {
                final String sText = String.format(
                    "can not find transition for state %s(%d) and element %s(%d:%d) at L%dC%d\n",
                    maStateNameMap.GetNameForId(mnCurrentStateId),
                    mnCurrentStateId,
                    aDescriptor.Id,
                    maNameMap.GetNameForId(nElementNameId),
                    nElementNameId,
                    aLocation.getLineNumber(),
                    aLocation.getColumnNumber());
                Log.Err.printf(sText);
                if (Log.Dbg != null)
                    Log.Dbg.printf(sText);
            }
            else
            {
                if (Log.Dbg != null)
                {
                    Log.Dbg.printf(" %s(%d) -> %s(%d) via %s(%d)",
                        maStateNameMap.GetNameForId(mnCurrentStateId),
                        mnCurrentStateId,
                        maStateNameMap.GetNameForId(aTransition.GetEndStateId()),
                        aTransition.GetEndStateId(),
                        maStateNameMap.GetNameForId(aTransition.GetActionId()),
                        aTransition.GetActionId());
                    Log.Dbg.printf("\n");
                }

                final int nOldState = mnCurrentStateId;
                SetCurrentState(aTransition.GetEndStateId());

                ExecuteActions(aTransition, aAttributes, nOldState, mnCurrentStateId);

                bResult = true;
            }
        }
        catch (RuntimeException aException)
        {
            Log.Err.printf("error at line %d and column %d\n",
                aLocation.getLineNumber(),
                aLocation.getColumnNumber());
            throw aException;
        }
        return bResult;
    }




    public void ProcessEndElement (
        final String sNamespaceURI,
        final String sElementName,
        final Location aLocation)
    {
        if ( ! maAcceptingStates.Contains(mnCurrentStateId)
            && mnCurrentStateId!=-1)
        {
            if (Log.Dbg != null)
                Log.Dbg.printf("current state %s(%d) is not an accepting state\n",
                    maStateNameMap.GetNameForId(mnCurrentStateId),
                    mnCurrentStateId);
            throw new RuntimeException("not expecting end element "+sElementName);
        }

        final NamespaceMap.NamespaceDescriptor aDescriptor = maNamespaceMap.GetDescriptorForURI(sNamespaceURI);

        final int nOldStateId = mnCurrentStateId;
        SetCurrentState(maStateStack.pop());

        if (Log.Dbg != null)
        {
            Log.Dbg.DecreaseIndentation();
            Log.Dbg.printf("/%s:%s L%d%d\n",
                aDescriptor.Prefix,
                sElementName,
                aLocation.getLineNumber(),
                aLocation.getColumnNumber());
            Log.Dbg.printf(" %s(%d) <- %s(%d)\n",
                maStateNameMap.GetNameForId(nOldStateId),
                nOldStateId,
                maStateNameMap.GetNameForId(mnCurrentStateId),
                mnCurrentStateId);
        }
    }




    public void ProcessCharacters (
        final String sText)
    {
    }




    public boolean IsInSkipState ()
    {
        return maSkipStates.Contains(mnCurrentStateId);
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
        maStateStack.push(mnCurrentStateId);
        if (Log.Dbg != null)
            Log.Dbg.IncreaseIndentation();
        final int nActionId = aTransition.GetActionId();
        SetCurrentState(nActionId);
        maAttributeManager.ParseAttributes(
            nActionId,
            aAttributes);
    }




    private final NamespaceMap maNamespaceMap;
    private final NameMap maNameMap;
    private final NameMap maStateNameMap;
    private final TransitionTable maTransitions;
    private final AttributeManager maAttributeManager;
    private int mnCurrentStateId;
    private Stack<Integer> maStateStack;
    private final int mnStartStateId;
    private final int mnEndStateId;
    private SkipStateTable maSkipStates;
    private AcceptingStateTable maAcceptingStates;
}
