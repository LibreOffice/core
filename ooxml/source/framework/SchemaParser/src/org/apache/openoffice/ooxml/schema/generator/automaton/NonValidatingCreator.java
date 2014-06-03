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

package org.apache.openoffice.ooxml.schema.generator.automaton;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.iterator.AttributeIterator;
import org.apache.openoffice.ooxml.schema.iterator.DereferencingNodeIterator;
import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.complex.Any;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.schema.Schema;

/** Create a stack automaton for a given Schema object that parses
 *  its input files but does not validate them.
 */
public class NonValidatingCreator
{
    public NonValidatingCreator (
        final Schema aSchema)
    {
        maSchema = aSchema;
        maStateContext = new StateContext("_start_", "_end_");
        maTransitions = new HashSet<Transition>();
        maAttributeMap = new HashMap<>();
    }




    public StackAutomaton Create (final File aLogFile)
    {
        final Map<State,Vector<Attribute>> aAttributes = new HashMap<>();

        // namespaces

        PrintStream aLog = null;
        try
        {
            aLog = new PrintStream(new FileOutputStream(aLogFile));
        }
        catch (FileNotFoundException e)
        {
            e.printStackTrace();
        }

        // top level elements
        for (final Element aElement : maSchema.TopLevelElements.GetSorted())
            maTransitions.add(
                Transition.CreateElementTransition(
                    maStateContext.GetStartState(),
                    maStateContext.GetEndState(),
                    aElement.GetElementName(),
                    new ParseElementAction(
                        maStateContext.GetStateForTypeName(
                            aElement.GetTypeName()))));

        // Complex types.
        System.out.printf("processing %d complex types\n", maSchema.ComplexTypes.GetCount());
        for (final ComplexType aComplexType : maSchema.ComplexTypes.GetSorted())
        {
            ProcessType(
                aComplexType,
                CollectElements(aComplexType),
                CollectAnys(aComplexType),
                CollectAttributes(aComplexType));
        }

        aLog.close();

        // simple types
        // Finish

        return new StackAutomaton(
            maSchema.Namespaces,
            maStateContext,
            maTransitions,
            aAttributes);
    }




    private void ProcessType (
        final ComplexType aComplexType,
        final Iterable<Element> aElements,
        final Iterable<Any> aAnys,
        final Iterable<Attribute> aAttributes)
    {
        for (final Element aElement : aElements)
            maTransitions.add(
                Transition.CreateElementTransition(
                    maStateContext.GetStateForTypeName(aComplexType.GetName()),
                    maStateContext.GetStateForTypeName(aComplexType.GetName()),
                    aElement.GetElementName(),
                    new ParseElementAction(
                        maStateContext.GetStateForTypeName(aElement.GetTypeName()))));

        for (final Any aAny : aAnys)
            maTransitions.add(
                Transition.CreateElementTransition(
                    maStateContext.GetStateForTypeName(aComplexType.GetName()),
                    maStateContext.GetStateForTypeName(aComplexType.GetName()),
                    null,
                    new SkipElementAction()));

        if (aComplexType != null)
        {
            final Vector<Attribute> aAttributeVector = new Vector<Attribute>();
            if (aAttributes != null)
                for (final Attribute aAttribute : aAttributes)
                    aAttributeVector.add(aAttribute);
            maAttributeMap.put(
                maStateContext.GetStateForTypeName(aComplexType.GetName()),
                aAttributeVector);
        }
    }




    /** Collect all elements inside the type tree that is rooted in the given
     *  complex type.
     */
    private Vector<Element> CollectElements (final ComplexType aType)
    {
        final Vector<Element> aElements = new Vector<>();
        for (final INode aNode : new DereferencingNodeIterator(aType, maSchema, false))
        {
            if (aNode.GetNodeType() == NodeType.Element)
                aElements.add((Element)aNode);
        }
        return aElements;
    }




    private Vector<Any> CollectAnys (final ComplexType aType)
    {
        final Vector<Any> aAnys = new Vector<>();
        for (final INode aNode : new DereferencingNodeIterator(aType, maSchema, false))
        {
            if (aNode.GetNodeType() == NodeType.Any)
                aAnys.add((Any)aNode);
        }
        return aAnys;
    }




    private Vector<Attribute> CollectAttributes (final ComplexType aComplexType)
    {
        final Vector<Attribute> aAttributes = new Vector<>();
        for (final INode aNode : new DereferencingNodeIterator(aComplexType, maSchema, false))
            for (final Attribute aAttribute : new AttributeIterator(aNode, maSchema))
                aAttributes.add(aAttribute);
        return aAttributes;
    }



    private final Schema maSchema;
    private final StateContext maStateContext;
    private final Set<Transition> maTransitions;
    private final Map<State,Vector<Attribute>> maAttributeMap;
}
