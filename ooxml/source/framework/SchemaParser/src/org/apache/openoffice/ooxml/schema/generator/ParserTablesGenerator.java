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
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

import org.apache.openoffice.ooxml.schema.automaton.FiniteAutomaton;
import org.apache.openoffice.ooxml.schema.automaton.FiniteAutomatonContainer;
import org.apache.openoffice.ooxml.schema.automaton.SkipData;
import org.apache.openoffice.ooxml.schema.automaton.State;
import org.apache.openoffice.ooxml.schema.automaton.Transition;
import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeBase.Use;
import org.apache.openoffice.ooxml.schema.model.schema.NamespaceMap;
import org.apache.openoffice.ooxml.schema.parser.FormDefault;

public class ParserTablesGenerator
{
    public ParserTablesGenerator (
        final FiniteAutomatonContainer aAutomatons,
        final NamespaceMap aNamespaces)
    {
        maAutomatons = aAutomatons;
        maNamespaces = aNamespaces;
        maNameToIdMap = new TreeMap<>();
        maPrefixToIdMap = new HashMap<>();
        maStateNameToIdMap = new TreeMap<>();
    }




    public void Generate (
        final File aParseTableFile)
    {
        final long nStartTime = System.currentTimeMillis();

        SetupNameList();
        AssignNameIds();

        try
        {
            final PrintStream aOut = new PrintStream(new FileOutputStream(aParseTableFile));

            WriteNamespaceList(aOut);
            WriteNameList(aOut);
            WriteGlobalStartEndStates(aOut);
            WriteNameList(aOut);
            WriteAutomatonList(aOut);
            aOut.close();
        }
        catch (final FileNotFoundException aException)
        {
            aException.printStackTrace();
        }

        final long nEndTime = System.currentTimeMillis();
        System.out.printf("wrote parse tables to %s in %fs\n",
            aParseTableFile.toString(),
            (nEndTime-nStartTime)/1000.0);
    }




    private void SetupNameList ()
    {
        final Set<String> aNames = new TreeSet<>();

        // Add the element names.
        for (final FiniteAutomaton aAutomaton : maAutomatons.GetAutomatons())
            for (final Transition aTransition : aAutomaton.GetTransitions())
            {
                if (aTransition.GetElementName() == null)
                    throw new RuntimeException();
                aNames.add(aTransition.GetElementName().GetLocalPart());
            }

        // Add the attribute names.
        for (final FiniteAutomaton aAutomaton : maAutomatons.GetAutomatons())
            for (final Attribute aAttribute : aAutomaton.GetAttributes())
                aNames.add(aAttribute.GetName().GetLocalPart());

        // Create unique ids for the names.
        int nIndex = 1;
        maNameToIdMap.clear();
        for (final String sName : aNames)
            maNameToIdMap.put(sName, nIndex++);

        // Create unique ids for namespace prefixes.
        nIndex = 1;
        maPrefixToIdMap.clear();
        for (final Entry<String, String> aEntry : maNamespaces)
        {
            maPrefixToIdMap.put(aEntry.getValue(), nIndex++);
        }
    }




    /** During the largest part of the parsing process, states and elements are
     *  identified not via their name but via a unique id.
     *  That allows a fast lookup.
     */
    private void AssignNameIds ()
    {
        maStateNameToIdMap.clear();
        int nIndex = 0;

        // Process state names.
        final Set<State> aSortedStates = new TreeSet<>();
        for (final State aState : maAutomatons.GetStates())
            aSortedStates.add(aState);
        for (final State aState : aSortedStates)
            maStateNameToIdMap.put(aState.GetFullname(), nIndex++);
    }




    private void WriteNamespaceList (final PrintStream aOut)
    {
        aOut.printf("# namespaces\n");
        for (final Entry<String, String> aEntry : maNamespaces)
        {
            aOut.printf("namespace %-8s %2d %s\n",
                aEntry.getValue(),
                maPrefixToIdMap.get(aEntry.getValue()),
                aEntry.getKey());
        }
    }




    private void WriteGlobalStartEndStates (final PrintStream aOut)
    {
        aOut.printf("\n# start and end states\n");

        final FiniteAutomaton aAutomaton = maAutomatons.GetTopLevelAutomaton();
        final State aStartState = aAutomaton.GetStartState();
        aOut.printf("start-state %4d %s\n",
            maStateNameToIdMap.get(aStartState.GetFullname()),
            aStartState.GetFullname());
        for (final State aAcceptingState : aAutomaton.GetAcceptingStates())
            aOut.printf("end-state %4d %s\n",
                maStateNameToIdMap.get(aAcceptingState.GetFullname()),
                aAcceptingState.GetFullname());
    }




    private void WriteNameList (final PrintStream aOut)
    {
        aOut.printf("\n# %d names\n", maNameToIdMap.size());
        for (final Entry<String, Integer> aEntry : maNameToIdMap.entrySet())
        {
            aOut.printf("name %4d %s\n",
                aEntry.getValue(),
                aEntry.getKey());
        }

        aOut.printf("\n# %s states\n",  maStateNameToIdMap.size());
        for (final Entry<String, Integer> aEntry : maStateNameToIdMap.entrySet())
        {
            aOut.printf("state-name %4d %s\n",
                aEntry.getValue(),
                aEntry.getKey());
        }
    }




    private void WriteAutomatonList (final PrintStream aOut)
    {
        for (final FiniteAutomaton aAutomaton : maAutomatons.GetAutomatons())
        {
            aOut.printf("# %s\n", aAutomaton.GetTypeName());

            final State aStartState = aAutomaton.GetStartState();
            final int nStartStateId = maStateNameToIdMap.get(aStartState.GetFullname());

            // Write start state.
            aOut.printf("start-state %d %s\n",
                nStartStateId,
                aStartState);

            // Write accepting states.
            for (final State aState : aAutomaton.GetAcceptingStates())
            {
                aOut.printf("accepting-state %d %s\n",
                    maStateNameToIdMap.get(aState.GetFullname()),
                    aState.GetFullname());
            }

            WriteAttributes(
                aOut,
                aStartState,
                aAutomaton.GetAttributes());

            // Write transitions.
            for (final Transition aTransition : aAutomaton.GetTransitions())
            {
                final Integer nId = maStateNameToIdMap.get(aTransition.GetElementTypeName());
                aOut.printf("transition %4d %4d %2d %4d %4d  %s %s \"%s\" %s\n",
                    maStateNameToIdMap.get(aTransition.GetStartState().GetFullname()),
                    maStateNameToIdMap.get(aTransition.GetEndState().GetFullname()),
                    maPrefixToIdMap.get(aTransition.GetElementName().GetNamespacePrefix()),
                    maNameToIdMap.get(aTransition.GetElementName().GetLocalPart()),
                    nId!=null ? nId : -1,
                    aTransition.GetStartState().GetFullname(),
                    aTransition.GetEndState().GetFullname(),
                    aTransition.GetElementName().GetStateName(),
                    aTransition.GetElementTypeName());
            }
            // Write skip data.
            for (final State aState : aAutomaton.GetStates())
            {
                for (@SuppressWarnings("unused") final SkipData aSkipData : aState.GetSkipData())
                    aOut.printf("skip %4d   %s\n",
                        maStateNameToIdMap.get(aState.GetFullname()),
                        aState.GetFullname());
            }
        }
    }




    private void WriteAttributes (
        final PrintStream aOut,
        final State aState,
        final Iterable<Attribute> aAttributes)
    {
        // Write attributes.
        for (final Attribute aAttribute : aAttributes)
        {
            aOut.printf("attribute %4d %2d %c %4d %4d %s %s  %s %s %s\n",
                maStateNameToIdMap.get(aState.GetFullname()),
                maPrefixToIdMap.get(aAttribute.GetName().GetNamespacePrefix()),
                aAttribute.GetFormDefault()==FormDefault.qualified ? 'q' : 'u',
                maNameToIdMap.get(aAttribute.GetName().GetLocalPart()),
                maStateNameToIdMap.get(aAttribute.GetTypeName().GetStateName()),
                aAttribute.GetUse()==Use.Optional ? 'o' : 'u',
                aAttribute.GetDefault()==null ? "null" : '"'+aAttribute.GetDefault()+'"',
                aState.GetFullname(),
                aAttribute.GetName().GetStateName(),
                aAttribute.GetTypeName().GetStateName());
        }
    }




    private final FiniteAutomatonContainer maAutomatons;
    private final NamespaceMap maNamespaces;
    private final Map<String,Integer> maNameToIdMap;
    private final Map<String,Integer> maPrefixToIdMap;
    private final Map<String,Integer> maStateNameToIdMap;
}
