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
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.QualifiedName;
import org.apache.openoffice.ooxml.schema.model.schema.NamespaceMap;
import org.apache.openoffice.ooxml.schema.parser.FormDefault;
import org.apache.openoffice.ooxml.schema.simple.BlobNode;
import org.apache.openoffice.ooxml.schema.simple.DateTimeNode;
import org.apache.openoffice.ooxml.schema.simple.ISimpleTypeNode;
import org.apache.openoffice.ooxml.schema.simple.ISimpleTypeNodeVisitor;
import org.apache.openoffice.ooxml.schema.simple.NumberNode;
import org.apache.openoffice.ooxml.schema.simple.SimpleTypeContainer;
import org.apache.openoffice.ooxml.schema.simple.SimpleTypeDescriptor;
import org.apache.openoffice.ooxml.schema.simple.StringNode;
import org.apache.openoffice.ooxml.schema.simple.UnionNode;

public class ParserTablesGenerator
{
    public ParserTablesGenerator (
        final FiniteAutomatonContainer aAutomatons,
        final NamespaceMap aNamespaces,
        final SimpleTypeContainer aSimpleTypes,
        final Map<String,Integer> aAttributeValueToIdMap)
    {
        maAutomatons = aAutomatons;
        maSimpleTypes = aSimpleTypes;
        maNamespaces = aNamespaces;
        maNameToIdMap = new TreeMap<>();
        maPrefixToIdMap = new HashMap<>();
        maTypeNameToIdMap = new TreeMap<>();
        maAttributeValueToIdMap = aAttributeValueToIdMap;
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
            WriteAutomatonList(aOut);
            WriteSimpleTypes(aOut);
            WriteAttributeValues(aOut);
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
        maTypeNameToIdMap.clear();
        int nIndex = 0;

        // Process state names.
        final Set<QualifiedName> aSortedTypeNames = new TreeSet<>();
        for (final State aState : maAutomatons.GetStates())
            aSortedTypeNames.add(aState.GetQualifiedName());
        for (final Entry<String, SimpleTypeDescriptor> aSimpleType : maSimpleTypes.GetSimpleTypes())
            aSortedTypeNames.add(aSimpleType.getValue().GetName());

        for (final QualifiedName aName : aSortedTypeNames)
            maTypeNameToIdMap.put(aName.GetStateName(), nIndex++);
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
            maTypeNameToIdMap.get(aStartState.GetFullname()),
            aStartState.GetFullname());
        for (final State aAcceptingState : aAutomaton.GetAcceptingStates())
            aOut.printf("end-state %4d %s\n",
                maTypeNameToIdMap.get(aAcceptingState.GetFullname()),
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

        aOut.printf("\n# %s states\n",  maTypeNameToIdMap.size());
        for (final Entry<String, Integer> aEntry : maTypeNameToIdMap.entrySet())
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
            aOut.printf("# %s at %s\n", aAutomaton.GetTypeName(), aAutomaton.GetLocation());

            final State aStartState = aAutomaton.GetStartState();
            final int nStartStateId = maTypeNameToIdMap.get(aStartState.GetFullname());

            // Write start state.
            aOut.printf("start-state %d %s\n",
                nStartStateId,
                aStartState);

            // Write accepting states.
            for (final State aState : aAutomaton.GetAcceptingStates())
            {
                aOut.printf("accepting-state %d %s\n",
                    maTypeNameToIdMap.get(aState.GetFullname()),
                    aState.GetFullname());
            }

            // Write text type.
            final INode aTextType = aStartState.GetTextType();
            if (aTextType != null)
            {
                switch(aTextType.GetNodeType())
                {
                    case BuiltIn:
                        aOut.printf("text-type %d %d   %s\n",
                            nStartStateId,
                            maTypeNameToIdMap.get(aTextType.GetName().GetStateName()),
                            aTextType.GetName().GetStateName());
                        break;
                    case SimpleType:
                        aOut.printf("text-type %d %d   %s\n",
                            nStartStateId,
                            maTypeNameToIdMap.get(aTextType.GetName().GetStateName()),
                            aTextType.GetName().GetStateName());
                        break;
                    default:
                        throw new RuntimeException();
                }
            }

            WriteAttributes(
                aOut,
                aStartState,
                aAutomaton.GetAttributes());

            // Write transitions.
            for (final Transition aTransition : aAutomaton.GetTransitions())
            {
                final Integer nId = maTypeNameToIdMap.get(aTransition.GetElementTypeName());
                aOut.printf("transition %4d %4d %2d %4d %4d  %s %s %s %s\n",
                    maTypeNameToIdMap.get(aTransition.GetStartState().GetFullname()),
                    maTypeNameToIdMap.get(aTransition.GetEndState().GetFullname()),
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
                        maTypeNameToIdMap.get(aState.GetFullname()),
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
                maTypeNameToIdMap.get(aState.GetFullname()),
                maPrefixToIdMap.get(aAttribute.GetName().GetNamespacePrefix()),
                aAttribute.GetFormDefault()==FormDefault.qualified ? 'q' : 'u',
                maNameToIdMap.get(aAttribute.GetName().GetLocalPart()),
                maTypeNameToIdMap.get(aAttribute.GetTypeName().GetStateName()),
                aAttribute.GetUse()==Use.Optional ? 'o' : 'u',
                aAttribute.GetDefault()==null ? "null" : '"'+aAttribute.GetDefault()+'"',
                aState.GetFullname(),
                aAttribute.GetName().GetStateName(),
                aAttribute.GetTypeName().GetStateName());
        }
    }




    private void WriteSimpleTypes (
        final PrintStream aOut)
    {
        if (maSimpleTypes == null)
        {
            aOut.printf("\n// There is no simple type information.\n");
        }
        else
        {
            aOut.printf("\n// %d simple types.\n", maSimpleTypes.GetSimpleTypeCount());
            for (final Entry<String,SimpleTypeDescriptor> aEntry : maSimpleTypes.GetSimpleTypesSorted())
            {
                int nIndex = 0;
                for (final ISimpleTypeNode aSubType : aEntry.getValue().GetSubType())
                {
                    final int nCurrentIndex = nIndex++;

                    final StringBuffer aLine = new StringBuffer();
                    aLine.append(String.format(
                        "simple-type %5d %1d %c ",
                        maTypeNameToIdMap.get(aEntry.getKey()),
                        nCurrentIndex,
                        aSubType.IsList() ? 'L' : 'T'));

                    aSubType.AcceptVisitor(new ISimpleTypeNodeVisitor()
                    {
                        @Override public void Visit(UnionNode aType)
                        {
                            throw new RuntimeException("unexpected");
                        }
                        @Override public void Visit(StringNode aType)
                        {
                            AppendStringDescription(aLine, aType);
                        }
                        @Override public void Visit(NumberNode<?> aType)
                        {
                            AppendNumberDescription(aLine, aType);
                        }
                        @Override public void Visit(DateTimeNode aType)
                        {
                            AppendDateTimeDescription(aLine, aType);
                        }
                        @Override public void Visit(BlobNode aType)
                        {
                            AppendBlobDescription(aLine, aType);
                        }
                    });
                    aOut.printf("%s\n", aLine.toString());
                }
            }
        }
    }




    private void WriteAttributeValues (
        final PrintStream aOut)
    {
        final Map<String,Integer> aSortedMap = new TreeMap<>();
        aSortedMap.putAll(maAttributeValueToIdMap);
        aOut.printf("//  %d attribute values from enumerations.\n", maAttributeValueToIdMap.size());
        for (final Entry<String,Integer> aEntry : aSortedMap.entrySet())
            aOut.printf("attribute-value %5d %s\n", aEntry.getValue(), QuoteString(aEntry.getKey()));
    }




    private static void AppendStringDescription (
        final StringBuffer aLine,
        final StringNode aType)
    {
        aLine.append("S ");
        switch(aType.GetRestrictionType())
        {
            case Enumeration:
                aLine.append('E');
                for (final int nValueId : aType.GetEnumerationRestriction())
                {
                    aLine.append(' ');
                    aLine.append(nValueId);
                }
                break;
            case Pattern:
                aLine.append("P ");
                aLine.append(QuoteString(aType.GetPatternRestriction()));
                break;
            case Length:
                aLine.append("L ");
                final int[] aLengthRestriction = aType.GetLengthRestriction();
                aLine.append(aLengthRestriction[0]);
                aLine.append(' ');
                aLine.append(aLengthRestriction[1]);
                break;
            case None:
                aLine.append('N');
                break;
            default:
                throw new RuntimeException();
        }
    }




    private static void AppendNumberDescription (
        final StringBuffer aLine,
        final NumberNode<?> aType)
    {
        aLine.append("N ");
        switch(aType.GetNumberType())
        {
            case Boolean: aLine.append("u1"); break;
            case Byte: aLine.append("s8"); break;
            case UnsignedByte: aLine.append("u8"); break;
            case Short: aLine.append("s16"); break;
            case UnsignedShort: aLine.append("u16"); break;
            case Int: aLine.append("s32"); break;
            case UnsignedInt: aLine.append("u32"); break;
            case Long: aLine.append("s64"); break;
            case UnsignedLong: aLine.append("u64"); break;
            case Integer: aLine.append("s*"); break;
            case Float: aLine.append("f"); break;
            case Double: aLine.append("d"); break;
            default:
                throw new RuntimeException("unsupported numerical type "+aType.GetNumberType());
        }
        aLine.append(' ');
        switch(aType.GetRestrictionType())
        {
            case Enumeration:
                aLine.append("E ");
                for (final Object nValue : aType.GetEnumerationRestriction())
                {
                    aLine.append(" ");
                    aLine.append(nValue);
                }
                break;
            case Size:
                aLine.append("S");
                if (aType.GetMinimum() != null)
                {
                    if (aType.IsMinimumInclusive())
                        aLine.append(" >= ");
                    else
                        aLine.append(" > ");
                    aLine.append(aType.GetMinimum());
                }
                if (aType.GetMaximum() != null)
                {
                    if (aType.IsMaximumInclusive())
                        aLine.append(" <= ");
                    else
                        aLine.append(" < ");
                    aLine.append(aType.GetMaximum());
                }
                break;
            case None:
                aLine.append("N");
                break;
            default:
                throw new RuntimeException("unsupported numerical restriction "+aType.GetRestrictionType());
        }
    }




    private static void AppendDateTimeDescription (
        final StringBuffer aLine,
        final DateTimeNode aType)
    {
        aLine.append("D");
    }




    private static void AppendBlobDescription (
        final StringBuffer aLine,
        final BlobNode aType)
    {
        aLine.append("B ");
        switch(aType.GetBlobType())
        {
            case Base64Binary:
                aLine.append("B ");
                break;
            case HexBinary:
                aLine.append ("H ");
                break;
            default:
                throw new RuntimeException("unsupported blob type");
        }
        switch(aType.GetRestrictionType())
        {
            case Length:
                aLine.append("L ");
                aLine.append(aType.GetLengthRestriction());
                break;
            case None:
                aLine.append("N");
                break;
            default:
                throw new RuntimeException();
        }
    }




    private static String QuoteString(final String sText)
    {
        return "\"" + sText.replace("\"", "&quot;").replace(" ", "%20") + "\"";
    }




    private final FiniteAutomatonContainer maAutomatons;
    private final SimpleTypeContainer maSimpleTypes;
    private final NamespaceMap maNamespaces;
    private final Map<String,Integer> maNameToIdMap;
    private final Map<String,Integer> maPrefixToIdMap;
    private final Map<String,Integer> maTypeNameToIdMap;
    private final Map<String,Integer> maAttributeValueToIdMap;
}
