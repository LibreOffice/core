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

package org.apache.openoffice.ooxml.schema.automaton;

import java.io.File;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.iterator.DereferencingNodeIterator;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.NodeType;
import org.apache.openoffice.ooxml.schema.model.complex.Any;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.schema.Schema;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;

/** Create a set of stack automatons for a given set of schemas.
 *  Creates one automaton for each complex type and one for the top level elements.
 *
 *  Input files but are not validated to conform to the schemas.
 */
public class NonValidatingCreator
    extends CreatorBase
{
    public NonValidatingCreator (
        final SchemaBase aSchemaBase,
        final File aLogFile)
    {
        super(aSchemaBase, aLogFile);
    }




    public FiniteAutomatonContainer Create (
        final Iterable<Schema> aTopLevelSchemas)
    {
        final FiniteAutomatonContainer aAutomatons = new FiniteAutomatonContainer(maStateContainer);

        // Create a single automaton for all top level elements.
        aAutomatons.AddAutomaton(
            null,
            CreateForTopLevelElements(aTopLevelSchemas));

        // Create one automaton for each complex type.
        for (final ComplexType aComplexType : maSchemaBase.ComplexTypes.GetSorted())
            aAutomatons.AddAutomaton(
                aComplexType.GetName(),
                CreateForComplexType(aComplexType));

        // Create one automaton for each simple type that is referenced by an element.
        for (final INode aSimpleType : maElementSimpleTypes)
            aAutomatons.AddAutomaton(
                aSimpleType.GetName(),
                CreateForSimpleType(aSimpleType));

        maLog.Close();

        return aAutomatons;
    }




    private FiniteAutomaton CreateForTopLevelElements (
        final Iterable<Schema> aTopLevelSchemas)
    {
        maLog.AddComment("top level elements");
        maLog.StartBlock();
        final String sTypeName = "<top-level>";
        final StateContext aStateContext = new StateContext(
            maStateContainer,
            sTypeName);
        final State aStartState = aStateContext.GetStartState();
        final State aEndState = aStateContext.CreateEndState();

        // top level elements
        for (final Schema aSchema : aTopLevelSchemas)
        {
            maLog.AddComment("schema %s", aSchema.GetShortName());
            maLog.StartBlock();
            for (final Element aElement : aSchema.TopLevelElements.GetSorted())
            {
                maLog.AddComment("Element: on '%s' go from %s to %s via %s",
                    aElement.GetElementName().GetDisplayName(),
                    aStartState.GetFullname(),
                    aEndState.GetFullname(),
                    aElement.GetTypeName().GetStateName());

                aStateContext.GetStartState().AddTransition(
                    new Transition(
                        aStartState,
                        aEndState,
                        aElement.GetElementName(),
                        aElement.GetTypeName().GetStateName()));
            }
            maLog.EndBlock();
        }
        maLog.EndBlock();

        return new FiniteAutomaton(aStateContext, null, null);
    }




    private FiniteAutomaton CreateForComplexType (final ComplexType aComplexType)
    {
        maLog.printf("\n");
        maLog.AddComment ("Complex Type %s defined in %s.",
            aComplexType.GetName().GetDisplayName(),
            aComplexType.GetLocation());
        maLog.StartBlock();

        final StateContext aStateContext = new StateContext(
            maStateContainer,
            aComplexType.GetName().GetStateName());

        for (final Element aElement : CollectElements(aComplexType))
        {
            maLog.AddComment("Element: on '%s' go from %s to %s via %s",
                aElement.GetElementName().GetDisplayName(),
                aStateContext.GetStartState().GetFullname(),
                aStateContext.GetStartState().GetFullname(),
                aElement.GetTypeName().GetStateName());

            aStateContext.GetStartState().AddTransition(
                new Transition(
                    aStateContext.GetStartState(),
                    aStateContext.GetStartState(),
                    aElement.GetElementName(),
                    aElement.GetTypeName().GetStateName()));

            // For elements whose type is a simple type we have to remember that
            // simple type for later (and then create an NFA for it.)
            final INode aSimpleType = maSchemaBase.GetSimpleTypeForName(
                aElement.GetTypeName());
            if (aSimpleType != null)
                maElementSimpleTypes.add(aSimpleType);
        }

        for (final Any aAny : CollectAnys(aComplexType))
        {
            AddSkipTransition(
                aStateContext.GetStartState(),
                new SkipData(
                    aAny.GetProcessContentsFlag(),
                    aAny.GetNamespaces()));
        }

        // Collect all attributes.
        maAttributes = new Vector<>();
        for (final INode aNode : new DereferencingNodeIterator(aComplexType, maSchemaBase, true))
            ProcessAttributes(aNode);

        aStateContext.GetStartState().SetIsAccepting();

        maLog.EndBlock();

        return new FiniteAutomaton(aStateContext, maAttributes, aComplexType.GetLocation());
    }




    /** Collect all elements inside the type tree that is rooted in the given
     *  complex type.
     */
    private Vector<Element> CollectElements (final ComplexType aType)
    {
        final Vector<Element> aElements = new Vector<>();
        for (final INode aNode : new DereferencingNodeIterator(aType, maSchemaBase, false))
        {
            if (aNode.GetNodeType() == NodeType.Element)
                aElements.add((Element)aNode);
        }
        return aElements;
    }




    private Vector<Any> CollectAnys (final ComplexType aType)
    {
        final Vector<Any> aAnys = new Vector<>();
        for (final INode aNode : new DereferencingNodeIterator(aType, maSchemaBase, false))
        {
            if (aNode.GetNodeType() == NodeType.Any)
                aAnys.add((Any)aNode);
        }
        return aAnys;
    }
}
