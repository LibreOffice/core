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
import java.util.Vector;

import javax.xml.stream.Location;

import org.apache.openoffice.ooxml.parser.action.ActionManager;
import org.apache.openoffice.ooxml.parser.action.ActionTrigger;
import org.apache.openoffice.ooxml.parser.action.IAction;
import org.apache.openoffice.ooxml.parser.attribute.AttributeManager;
import org.apache.openoffice.ooxml.parser.attribute.AttributeProvider;
import org.apache.openoffice.ooxml.parser.attribute.AttributeValues;
import org.apache.openoffice.ooxml.parser.type.SimpleTypeManager;

/** The state machine is initialized at creation from the data tables
 *  created previously by a stack automaton.
 */
public class StateMachine
{
    public StateMachine (
        final File aParseTableFile,
        final Vector<String> aErrorsAndWarnings)
    {
        if (Log.Dbg != null)
            Log.Dbg.printf("reading parse tables from %s\n", aParseTableFile.toString());

        final ParseTableReader aReader = new ParseTableReader(aParseTableFile);
        maNamespaceMap = new NamespaceMap(aReader.GetSection("namespace"));
        maNameMap = new NameMap(aReader.GetSection("name"));
        maStateNameMap = new NameMap(aReader.GetSection("state-name"));
        maTransitions = new TransitionTable(aReader.GetSection("transition"));
        maSkipStates = new SkipStateTable(aReader.GetSection("skip"));
        maAttributeValueMap = new NameMap(aReader.GetSection("attribute-value"));
        maAcceptingStates = new AcceptingStateTable(aReader.GetSection("accepting-state"));
        maSimpleTypeManager = new SimpleTypeManager(
            aReader.GetSection("simple-type"),
            maAttributeValueMap);
        maAttributeManager = new AttributeManager(
            aReader.GetSection("attribute"),
            maNamespaceMap,
            maNameMap,
            maStateNameMap,
            maSimpleTypeManager,
            aErrorsAndWarnings);
        mnStartStateId = Integer.parseInt(aReader.GetSection("start-state").firstElement()[1]);
        mnEndStateId = Integer.parseInt(aReader.GetSection("end-state").firstElement()[1]);

        mnCurrentStateId = mnStartStateId;
        maStateStack = new Stack<>();
        maElementContextStack = new Stack<>();
        maActionManager = new ActionManager(maStateNameMap);
        maErrorsAndWarnings  = aErrorsAndWarnings;

        if (Log.Dbg != null)
        {
            Log.Dbg.printf("read %d namespace, %d names, %d states (%d skip, %d accept), %d transitions and %d attributes\n",
                maNamespaceMap.GetNamespaceCount(),
                maNameMap.GetNameCount(),
                maStateNameMap.GetNameCount(),
                maSkipStates.GetSkipStateCount(),
                maAcceptingStates.GetAcceptingStateCount(),
                maTransitions.GetTransitionCount(),
                maAttributeManager.GetAttributeCount());
            Log.Dbg.printf("starting in state _start_ (%d)\n", mnCurrentStateId);
        }
    }




    public boolean ProcessStartElement (
        final String sNamespaceURI,
        final String sElementName,
        final Location aStartLocation,
        final Location aEndLocation,
        final AttributeProvider aAttributes)
    {
        boolean bResult = false;

        try
        {
            final NamespaceMap.NamespaceDescriptor aNamespaceDescriptor = maNamespaceMap.GetDescriptorForURI(sNamespaceURI);
            final int nElementNameId = maNameMap.GetIdForName(sElementName);
            if (Log.Dbg != null)
                Log.Dbg.printf("%s:%s(%d:%d) L%dC%d\n",
                    aNamespaceDescriptor.Prefix,
                    sElementName,
                    aNamespaceDescriptor.Id,
                    nElementNameId,
                    aStartLocation.getLineNumber(),
                    aStartLocation.getColumnNumber());

            final Transition aTransition = maTransitions.GetTransition(
                mnCurrentStateId,
                aNamespaceDescriptor.Id,
                nElementNameId);
            if (aTransition == null)
            {
                final String sText = String.format(
                    "can not find transition for state %s(%d) and element %s:%s(%d:%d) at L%dC%d\n",
                    maStateNameMap.GetNameForId(mnCurrentStateId),
                    mnCurrentStateId,
                    aNamespaceDescriptor.Prefix,
                    maNameMap.GetNameForId(nElementNameId),
                    aNamespaceDescriptor.Id,
                    nElementNameId,
                    aStartLocation.getLineNumber(),
                    aStartLocation.getColumnNumber());
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

                // Follow the transition to its end state but first process its
                // content.  We do that by

                if (Log.Dbg != null)
                    Log.Dbg.IncreaseIndentation();

                // a) pushing the end state to the state stack so that on the
                // end tag that corresponds to the current start tag it will become the current state.
                maStateStack.push(aTransition.GetEndStateId());

                // b) entering the state that corresponds to start tag that
                // we are currently processing.
                mnCurrentStateId = aTransition.GetActionId();

                // c) Prepare the attributes and store them in the new element context.
                final AttributeValues aAttributeValues = maAttributeManager.ParseAttributes(
                    mnCurrentStateId,
                    aAttributes);

                // d) creating a new ElementContext for the element that just starts.
                maElementContextStack.push(maCurrentElementContext);
                final ElementContext aPreviousElementContext = maCurrentElementContext;
                maCurrentElementContext = new ElementContext(
                    sElementName,
                    maStateNameMap.GetNameForId(aTransition.GetActionId()),
                    false,
                    aAttributeValues,
                    aPreviousElementContext);

                // e) and run all actions that are bound to the the current start tag.
                ExecuteActions(
                    mnCurrentStateId,
                    maCurrentElementContext,
                    ActionTrigger.ElementStart,
                    null,
                    aStartLocation,
                    aEndLocation);

                bResult = true;
            }
        }
        catch (RuntimeException aException)
        {
            Log.Err.printf("error at line %d and column %d\n",
                aStartLocation.getLineNumber(),
                aStartLocation.getColumnNumber());
            throw aException;
        }
        return bResult;
    }




    public void ProcessEndElement (
        final String sNamespaceURI,
        final String sElementName,
        final Location aStartLocation,
        final Location aEndLocation)
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

        // Leave the current element.

        final int nPreviousStateId = mnCurrentStateId;
        mnCurrentStateId = maStateStack.pop();
        if (mnCurrentStateId == mnEndStateId)
            mnCurrentStateId = mnStartStateId;

        final ElementContext aPreviousElementContext = maCurrentElementContext;
        maCurrentElementContext = maElementContextStack.pop();

        ExecuteActions(
            nPreviousStateId,
            aPreviousElementContext,
            ActionTrigger.ElementEnd,
            null,
            aStartLocation,
            aEndLocation);

        if (Log.Dbg != null)
        {
            Log.Dbg.DecreaseIndentation();
            Log.Dbg.printf("/%s:%s L%d%d\n",
                aDescriptor.Prefix,
                sElementName,
                aStartLocation.getLineNumber(),
                aStartLocation.getColumnNumber());
            Log.Dbg.printf(" %s(%d) <- %s(%d)\n",
                maStateNameMap.GetNameForId(nPreviousStateId),
                nPreviousStateId,
                maStateNameMap.GetNameForId(mnCurrentStateId),
                mnCurrentStateId);
        }
    }




    public void ProcessCharacters (
        final String sText,
        final Location aStartLocation,
        final Location aEndLocation)
    {
        if (Log.Dbg != null)
            Log.Dbg.printf("text [%s]\n", sText.replace("\n", "\\n"));

        ExecuteActions(
            mnCurrentStateId,
            maCurrentElementContext,
            ActionTrigger.Text,
            sText,
            aStartLocation,
            aEndLocation);

    }




    public boolean IsInSkipState ()
    {
        return maSkipStates.Contains(mnCurrentStateId);
    }




    public ActionManager GetActionManager ()
    {
        return maActionManager;
    }




    private void ExecuteActions (
        final int nStateId,
        final ElementContext aElementContext,
        final ActionTrigger eTrigger,
        final String sText,
        final Location aStartLocation,
        final Location aEndLocation)
    {
        final Iterable<IAction> aActions = maActionManager.GetActions(nStateId, eTrigger);
        if (aActions != null)
            for (final IAction aAction : aActions)
                aAction.Run(eTrigger, aElementContext, sText, aStartLocation, aEndLocation);
    }




    private final NamespaceMap maNamespaceMap;
    private final NameMap maNameMap;
    private final NameMap maStateNameMap;
    private final TransitionTable maTransitions;
    private final SimpleTypeManager maSimpleTypeManager;
    private final AttributeManager maAttributeManager;
    private final NameMap maAttributeValueMap;
    private int mnCurrentStateId;
    private Stack<Integer> maStateStack;
    private ElementContext maCurrentElementContext;
    private Stack<ElementContext> maElementContextStack;
    private final int mnStartStateId;
    private final int mnEndStateId;
    private SkipStateTable maSkipStates;
    private AcceptingStateTable maAcceptingStates;
    private final ActionManager maActionManager;
    private final Vector<String> maErrorsAndWarnings;
}
