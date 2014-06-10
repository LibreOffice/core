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
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;

/** Create a set of validating stack automatons for a set of schemas.
 *  There is one DFA (deterministic finite automaton) for each complex type and
 *  one for the top level elements.
 */
public class ValidatingCreator
{
    public ValidatingCreator (
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




    /** Create one automaton for the top-level elements and one for each complex
     *  type.
     */
    public FiniteAutomatonContainer Create ()
    {
        final FiniteAutomatonContainer aAutomatons = new FiniteAutomatonContainer(maStateContainer);

        aAutomatons.AddAutomaton(
            null,
            CreateForTopLevelElements());

        for (final ComplexType aComplexType : maSchemaBase.ComplexTypes.GetSorted())
            aAutomatons.AddAutomaton(
                aComplexType.GetName(),
                CreateForComplexType(aComplexType));

        maLog.close();

        return aAutomatons;
    }




    private FiniteAutomaton CreateForTopLevelElements ()
    {
        final Map<State,Vector<Attribute>> aAttributes = new HashMap<>();

        final StateContext aStateContext = new StateContext(
            maStateContainer,
            "<top-level>");
        final State aEndState = aStateContext.CreateEndState();

        final INodeVisitor aVisitor = new ValidatingCreatorVisitor(
            aAttributes,
            aStateContext,
            maSchemaBase,
            maLog,
            "",
            aStateContext.GetStartState(),
            aStateContext.GetStartState(),
            aEndState);

        // top level elements
        for (final Element aElement : maSchemaBase.TopLevelElements.GetSorted())
            aElement.AcceptVisitor(aVisitor);

        return new FiniteAutomaton(aStateContext);

    }




    private FiniteAutomaton CreateForComplexType (final ComplexType aComplexType)
    {
        final Map<State,Vector<Attribute>> aAttributes = new HashMap<>();

        final StateContext aStateContext = new StateContext(
            maStateContainer,
            aComplexType.GetName().GetStateName());
        final State aEndState = aStateContext.CreateEndState();
        aComplexType.AcceptVisitor(
            new ValidatingCreatorVisitor(
                aAttributes,
                aStateContext,
                maSchemaBase,
                maLog,
                "",
                aStateContext.GetStartState(),
                aStateContext.GetStartState(),
                aEndState));
        return new FiniteAutomaton(aStateContext);
    }




    private final SchemaBase maSchemaBase;
    private final StateContainer maStateContainer;
    private final PrintStream maLog;
}
