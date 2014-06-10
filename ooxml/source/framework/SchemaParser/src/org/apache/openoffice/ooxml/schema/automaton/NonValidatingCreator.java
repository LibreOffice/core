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
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
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
{
    public NonValidatingCreator (
        final SchemaBase aSchemaBase,
        final File aLogFile)
    {
        maSchemaBase = aSchemaBase;
        maStateContainer = new StateContainer();

        PrintStream aLog = null;
        try
        {
            aLog = new PrintStream(new FileOutputStream(aLogFile));
        }
        catch (FileNotFoundException e)
        {
            e.printStackTrace();
        }
        maLog = aLog;
    }




    public FiniteAutomatonContainer Create (
        final Iterable<Schema> aTopLevelSchemas)
    {
        final FiniteAutomatonContainer aAutomatons = new FiniteAutomatonContainer(maStateContainer);

        aAutomatons.AddAutomaton(
            null,
            CreateForTopLevelElements(aTopLevelSchemas));

        for (final ComplexType aComplexType : maSchemaBase.ComplexTypes.GetSorted())
            aAutomatons.AddAutomaton(
                aComplexType.GetName(),
                CreateForComplexType(aComplexType));

        maLog.close();

        return aAutomatons;
    }



    private FiniteAutomaton CreateForTopLevelElements (
        final Iterable<Schema> aTopLevelSchemas)
    {
        final String sTypeName = "<top-level>";
        final StateContext aStateContext = new StateContext(
            maStateContainer,
            sTypeName);
        final State aEndState = aStateContext.CreateEndState();

        // top level elements
        for (final Schema aSchema : aTopLevelSchemas)
            for (final Element aElement : aSchema.TopLevelElements.GetSorted())
                aStateContext.GetStartState().AddTransition(
                    new Transition(
                        aStateContext.GetStartState(),
                        aEndState,
                        aElement.GetElementName(),
                        aElement.GetTypeName().GetStateName()));

        return new FiniteAutomaton(aStateContext);
    }




    private FiniteAutomaton CreateForComplexType (final ComplexType aComplexType)
    {
        final StateContext aStateContext = new StateContext(
            maStateContainer,
            aComplexType.GetName().GetStateName());

        for (final Element aElement : CollectElements(aComplexType))
        {
            aStateContext.GetStartState().AddTransition(
                new Transition(
                    aStateContext.GetStartState(),
                    aStateContext.GetStartState(),
                    aElement.GetElementName(),
                    aElement.GetTypeName().GetStateName()));
        }

        for (final Any aAny : CollectAnys(aComplexType))
        {
            aStateContext.GetStartState().AddSkipData(
                new SkipData(
                    aAny.GetProcessContentsFlag(),
                    aAny.GetNamespaces()));
        }

        aStateContext.GetStartState().SetIsAccepting();

        return new FiniteAutomaton(aStateContext);
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




    private final SchemaBase maSchemaBase;
    private final StateContainer maStateContainer;
    private final PrintStream maLog;
}
