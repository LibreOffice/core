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

package org.apache.openoffice.ooxml.schema.generator;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;

import org.apache.openoffice.ooxml.schema.generator.automaton.IAction;
import org.apache.openoffice.ooxml.schema.generator.automaton.StackAutomaton;
import org.apache.openoffice.ooxml.schema.generator.automaton.State;
import org.apache.openoffice.ooxml.schema.generator.automaton.Transition;

public class ParserTablesGenerator
{
    public ParserTablesGenerator (final StackAutomaton aAutomaton)
    {
        maAutomaton = aAutomaton;
        maNameToIdMap = new TreeMap<>();
    }




    public void Generate (
        final File aOutputDirectory)
    {
        if ( ! aOutputDirectory.exists())
            throw new RuntimeException("output directory "+aOutputDirectory+" does not exist");
        if ( ! aOutputDirectory.canWrite())
            throw new RuntimeException("output directory "+aOutputDirectory+" is not writable");

        AssignNameIds();

        try
        {
            WriteNamespaceList(
                new PrintStream(
                    new FileOutputStream(
                        new File(aOutputDirectory, "namespaces.lst"))));

            WriteNameList(
                new PrintStream(
                    new FileOutputStream(
                        new File(aOutputDirectory, "names.lst"))));

            WriteTransitionList(
                new PrintStream(
                    new FileOutputStream(
                        new File(aOutputDirectory, "transitions.lst"))));
        }
        catch (final FileNotFoundException aException)
        {
            aException.printStackTrace();
        }
    }




    /** During the largest part of the parsing process, states and elements are
     *  identified not via their name but via a unique id.
     *  That allows a fast lookup.
     */
    private void AssignNameIds ()
    {
        maNameToIdMap.clear();
        int nIndex = 0;
        for (final State aState : maAutomaton.GetStatesSorted())
            maNameToIdMap.put(aState.GetFullname(), nIndex++);
        for (final Transition aTransition : maAutomaton.GetTransitions())
        {
            if (aTransition.GetElementName() == null)
                continue;
            // Element names are not necessarily unique.
            final String sElementName = aTransition.GetElementName().GetStateName();
            if ( ! maNameToIdMap.containsKey(sElementName))
                maNameToIdMap.put(sElementName, nIndex++);
        }
    }




    private void WriteNamespaceList (final PrintStream aOut)
    {
        for (final Entry<String, String> aEntry : maAutomaton.GetNamespaces())
        {
            aOut.printf("%s %s\n",
                aEntry.getKey(),
                aEntry.getValue());
        }
        aOut.close();
    }




    private void WriteNameList (final PrintStream aOut)
    {
        for (final Entry<String, Integer> aEntry : maNameToIdMap.entrySet())
        {
            aOut.printf("%d %s\n",
                aEntry.getValue(),
                aEntry.getKey());
        }
        aOut.close();
    }




    private void WriteTransitionList (final PrintStream aOut)
    {
        // Write regular transitions.
        for (final Transition aTransition : maAutomaton.GetTransitions())
        {
            final IAction aAction = aTransition.GetAction();
            final State aActionStartState = aAction.GetStartState();
            if (aTransition.GetElementName() != null)
            {
                aOut.printf("%d %d %d %s %d   %s %s \"%s\" %s\n",
                    maNameToIdMap.get(aTransition.GetStartState().GetFullname()),
                    maNameToIdMap.get(aTransition.GetEndState().GetFullname()),
                    maNameToIdMap.get(aTransition.GetElementName().GetStateName()),
                    aAction.GetActionName(),
                    aActionStartState!=null
                        ? maNameToIdMap.get(aActionStartState.GetFullname())
                        : -1,
                    aTransition.GetStartState(),
                    aTransition.GetEndState(),
                    aTransition.GetElementName().GetStateName(),
                    aActionStartState!=null
                        ? aActionStartState.GetFullname()
                        : "<none>");
            }
        }

        // Write skip transitions.
        for (final Transition aTransition : maAutomaton.GetTransitions())
        {
            final IAction aAction = aTransition.GetAction();
            final State aActionStartState = aAction.GetStartState();

            if (aTransition.GetElementName() == null)
            {
                aOut.printf("%d %d %d %s %d   %s %s %s\n",
                    maNameToIdMap.get(aTransition.GetStartState().GetFullname()),
                    maNameToIdMap.get(aTransition.GetEndState().GetFullname()),
                    -1,
                    aAction.GetActionName(),
                    aActionStartState!=null
                        ? maNameToIdMap.get(aActionStartState.GetFullname())
                        : -1,
                    aTransition.GetStartState(),
                    aTransition.GetEndState(),
                    aActionStartState!=null
                        ? aActionStartState.GetFullname()
                        : "<none>");

            }
        }
    }




    private final StackAutomaton maAutomaton;
    private final Map<String,Integer> maNameToIdMap;
}
