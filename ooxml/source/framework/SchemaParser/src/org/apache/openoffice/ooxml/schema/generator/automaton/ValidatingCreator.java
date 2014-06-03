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

import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.schema.Schema;

/** Create a stack automaton for a given Schema object that parses and validates
 *  its input files.
 */
public class ValidatingCreator
{
    public ValidatingCreator (
        final Schema aSchema)
    {
        maSchema = aSchema;
        maStateContext = new StateContext("_start_", "_end_");
        maTransitions = new HashSet<Transition>();
    }




    public StackAutomaton Create ()
    {
        final Map<State,Vector<Attribute>> aAttributes = new HashMap<>();

        // namespaces

        PrintStream aLog = null;
        try
        {
            aLog = new PrintStream(new FileOutputStream(new File("/tmp/schema.log")));
        }
        catch (FileNotFoundException e)
        {
            e.printStackTrace();
        }
        final INodeVisitor aVisitor = new ValidatingCreatorVisitor(
            aAttributes,
            maStateContext,
            maTransitions,
            maSchema,
            aLog,
            "",
            maStateContext.GetStartState(),
            maStateContext.GetStartState(),
            maStateContext.GetEndState());

        // top level elements
        for (final Element aElement : maSchema.TopLevelElements.GetSorted())
            aElement.AcceptVisitor(aVisitor);

        // Complex types.
        System.out.printf("processing %d complex types\n", maSchema.ComplexTypes.GetCount());
        for (final ComplexType aComplexType : maSchema.ComplexTypes.GetSorted())
        {
            final State aComplexTypeState = maStateContext.GetStateForTypeName(aComplexType.GetName());
            aComplexType.AcceptVisitor(
                new ValidatingCreatorVisitor(
                    aAttributes,
                    maStateContext,
                    maTransitions,
                    maSchema,
                    aLog,
                    "",
                    aComplexTypeState,
                    aComplexTypeState,
                    maStateContext.CreateState(aComplexTypeState, "end")));
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




    private final Schema maSchema;
    private final StateContext maStateContext;
    private final Set<Transition> maTransitions;
}
