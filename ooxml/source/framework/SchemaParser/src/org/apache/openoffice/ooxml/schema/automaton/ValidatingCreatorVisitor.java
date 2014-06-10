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

import java.io.PrintStream;
import java.util.Iterator;
import java.util.Map;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.iterator.AttributeIterator;
import org.apache.openoffice.ooxml.schema.iterator.DereferencingNodeIterator;
import org.apache.openoffice.ooxml.schema.iterator.PermutationIterator;
import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.NodeVisitorAdapter;
import org.apache.openoffice.ooxml.schema.model.complex.All;
import org.apache.openoffice.ooxml.schema.model.complex.Any;
import org.apache.openoffice.ooxml.schema.model.complex.Choice;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexContent;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.complex.ElementReference;
import org.apache.openoffice.ooxml.schema.model.complex.Extension;
import org.apache.openoffice.ooxml.schema.model.complex.Group;
import org.apache.openoffice.ooxml.schema.model.complex.GroupReference;
import org.apache.openoffice.ooxml.schema.model.complex.OccurrenceIndicator;
import org.apache.openoffice.ooxml.schema.model.complex.Sequence;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleContent;

/** A helper class of the ValidatingCreator.
 *
 *  A visitor is called by the ValidatingCreator for each complex type.
 *  Further instances are created and called while iterating over all nodes
 *  of these types.
 */
public class ValidatingCreatorVisitor
    extends NodeVisitorAdapter
{
    public ValidatingCreatorVisitor (
        final Map<State,Vector<Attribute>> aAttributes,
        final StateContext aStateContext,
        final SchemaBase aSchemaBase,
        final PrintStream aLog,
        final String sLogIndentation,
        final State aBaseState,
        final State aStartState,
        final State aEndState)
    {
        maAttributes = aAttributes;
        maStateContext = aStateContext;
        maSchemaBase = aSchemaBase;
        maLog = aLog;
        msLogIndentation = sLogIndentation;
        maBaseState = aBaseState;
        maStartState = aStartState;
        maEndState = aEndState;
    }




    @Override
    public void Visit (final All aAll)
    {
        AddComment("All");

        // Make a transformation of the children into a choice of sequences that
        // can then be processed by already existing Visit() methods.
        // These sequences enumerate all permutations of the original children.
        final INode aReplacement = GetAllReplacement(aAll);

        final State aLocalStartState = maStateContext.CreateState(maBaseState, "As");
        final State aLocalEndState = maStateContext.CreateState(maBaseState, "Ae");

        StartBlock();
        AddEpsilonTransition(maStartState, aLocalStartState);
        ProcessType(
            aReplacement,
            maStateContext.CreateState(maBaseState, "A"),
            aLocalStartState,
            aLocalEndState);
        AddEpsilonTransition(aLocalEndState, maEndState);
        EndBlock();
    }




    @Override
    public void Visit (final Any aAny)
    {
        assert(aAny.GetChildCount() == 0);

        AddComment("Any");
        AddSkipTransition(
            maStartState,
            new SkipData(
                aAny.GetProcessContentsFlag(),
                aAny.GetNamespaces()));
        AddEpsilonTransition(maStartState, maEndState);
    }




    @Override
    public void Visit (final Choice aChoice)
    {
        AddComment("Choice");

        final State aLocalStartState = maStateContext.CreateState(maBaseState, "Cs");
        final State aLocalEndState = maStateContext.CreateState(maBaseState, "Ce");
        StartBlock();
        AddEpsilonTransition(maStartState, aLocalStartState);

        int nStateIndex = 0;
        for (final INode aChild : aChoice.GetChildren())
        {
            ProcessType(
                aChild,
                maStateContext.CreateState(maBaseState, "C"+nStateIndex++),
                aLocalStartState,
                aLocalEndState);
        }
        AddEpsilonTransition(aLocalEndState, maEndState);
        EndBlock();
    }




    @Override
    public void Visit (final ComplexContent aComplexContent)
    {
        assert(aComplexContent.GetChildCount() == 1);

        if (maLog != null)
            AddComment ("Complex Content.");

        StartBlock();
        ProcessType(
            aComplexContent.GetChildren().iterator().next(),
            maBaseState,
            maStartState,
            maEndState);
        EndBlock();
    }




    @Override
    public void Visit (final ComplexType aComplexType)
    {
        if (maLog != null)
        {
            maLog.print("\n");
            AddComment ("Complex Type %s defined in %s.",
                aComplexType.GetName().GetDisplayName(),
                aComplexType.GetLocation());
        }

        StartBlock();
        maLog.printf("%sstarting at state %s\n", msLogIndentation, maStartState.GetFullname());
        for (final Attribute aAttribute : new AttributeIterator(aComplexType, maSchemaBase))
            ProcessAttribute(aAttribute);

        if (GetElementCount(aComplexType) == 0)
        {
            // There are elements. Therefore there will be no transitions.
            // The start state is accepting and the end state is not necessary.
            maStartState.SetIsAccepting();
            maStateContext.RemoveState(maEndState);
        }

        for (final INode aChild : aComplexType.GetChildren())
            ProcessType(aChild, maBaseState, maStartState, maEndState);

        EndBlock();
    }




    @Override
    public void Visit (final Element aElement)
    {
        assert(aElement.GetChildCount()==0);

        AddComment("Element: on '%s' go from %s to %s via %s",
            aElement.GetElementName().GetDisplayName(),
            maStartState.GetFullname(),
            maEndState.GetFullname(),
            aElement.GetTypeName().GetStateName());

        final Transition aTransition = new Transition(
            maStartState,
            maEndState,
            aElement.GetElementName(),
            aElement.GetTypeName().GetStateName());
        maStartState.AddTransition(aTransition);
    }




    @Override
    public void Visit (final ElementReference aReference)
    {
        assert(aReference.GetChildCount() == 0);

        AddComment("Element reference to %s", aReference.GetReferencedElementName());

        final Element aElement = aReference.GetReferencedElement(maSchemaBase);
        if (aElement == null)
            throw new RuntimeException("can't find referenced element "+aReference.GetReferencedElementName());
        StartBlock();
        ProcessType(aElement, maBaseState, maStartState, maEndState);
        EndBlock();
    }




    /** Treat extension nodes like sequences (for now).
     */
    @Override
    public void Visit (final Extension aExtension)
    {
        assert(aExtension.GetChildCount() <= 1);

        AddComment("Extension of base type %s", aExtension.GetBaseTypeName());

        final Vector<INode> aNodes = aExtension.GetTypeNodes(maSchemaBase);

        StartBlock();
        int nStateIndex = 0;
        State aCurrentState = maStateContext.CreateState(maBaseState, "E"+nStateIndex++);
        AddEpsilonTransition(maStartState, aCurrentState);
        for (final INode aChild : aNodes)
        {
            final State aNextState = maStateContext.CreateState(maBaseState, "E"+nStateIndex++);
            ProcessType(aChild, aCurrentState, aCurrentState, aNextState);
            aCurrentState = aNextState;
        }
        AddEpsilonTransition(aCurrentState, maEndState);
        EndBlock();
    }




    @Override
    public void Visit (final Group aGroup)
    {
        assert(aGroup.GetChildCount() == 1);

        AddComment("Group %s", aGroup.GetName());

        StartBlock();
        final State aGroupBaseState = maStateContext.CreateState(maBaseState, "G");
        ProcessType(
            aGroup.GetOnlyChild(),
            aGroupBaseState,
            maStartState,
            maEndState);
        EndBlock();
    }




    @Override
    public void Visit (final GroupReference aReference)
    {
        AddComment("Group reference to %s", aReference.GetReferencedGroupName());
        final Group aGroup = aReference.GetReferencedGroup(maSchemaBase);
        if (aGroup == null)
            throw new RuntimeException("can't find referenced group "+aReference.GetReferencedGroupName());
        StartBlock();
        ProcessType(aGroup, maBaseState, maStartState, maEndState);
        EndBlock();
    }




    /** An occurrence indicator defines how many times the single child can occur.
     *  The minimum value defines the mandatory number of times.  The maximum value
     *  defines the optional number.
     */
    @Override
    public void Visit (final OccurrenceIndicator aOccurrence)
    {
        assert(aOccurrence.GetChildCount() == 1);

        AddComment("OccurrenceIndicator %s->%s",
            aOccurrence.GetDisplayMinimum(),
            aOccurrence.GetDisplayMaximum());
        StartBlock();

        final INode aChild = aOccurrence.GetChildren().iterator().next();

        int nIndex = 0;
        State aCurrentState = maStateContext.CreateState(maBaseState, "O"+nIndex++);
        AddEpsilonTransition(maStartState, aCurrentState);

        if (aOccurrence.GetMinimum() == 0)
        {
            // A zero minimum means that all occurrences are optional.
            // Add a short circuit from start to end.
            AddComment("Occurrence: make whole element optional (min==0)");
            AddEpsilonTransition(maStartState, maEndState);
        }
        else
        {
            // Write a row of mandatory transitions for the minimum.
            for (; nIndex<=aOccurrence.GetMinimum(); ++nIndex)
            {
                // Add transition i-1 -> i (i == nIndex).
                final State aNextState = maStateContext.CreateState(maBaseState, "O"+nIndex);
                AddComment("Occurrence: move from %d -> %d (%s -> %s) (minimum)",
                    nIndex-1,
                    nIndex,
                    aCurrentState,
                    aNextState);
                StartBlock();
                ProcessType(aChild, aCurrentState, aCurrentState, aNextState);
                EndBlock();
                aCurrentState = aNextState;
            }
        }

        if (aOccurrence.GetMaximum() == OccurrenceIndicator.unbounded)
        {
            // Write loop on last state when max is unbounded.

            // last -> loop
            final State aLoopState = maStateContext.CreateState(maBaseState, "OL");
            AddComment("Occurrence: forward to loop (maximum)");
            AddEpsilonTransition(aCurrentState, aLoopState);

            // loop -> loop
            AddComment("Occurrence: loop");
            StartBlock();
            ProcessType(aChild, aLoopState, aLoopState, aLoopState);
            EndBlock();

            // -> end
            AddComment("Occurrence: forward to local end");
            AddEpsilonTransition(aLoopState, maEndState);
        }
        else
        {
            // Write a row of optional transitions for the maximum.
            for (; nIndex<=aOccurrence.GetMaximum(); ++nIndex)
            {
                if (nIndex > 0)
                {
                    // i-1 -> end
                    AddComment("Occurrence: make %d optional (maximum)", nIndex-1);
                    AddEpsilonTransition(aCurrentState, maEndState);
                }

                // i-1 -> i
                final State aNextState = maStateContext.CreateState(maBaseState, "O"+nIndex);
                AddComment("Occurrence: %d -> %d (%s -> %s) (maximum)",
                    nIndex-1,
                    nIndex,
                    aCurrentState,
                    aNextState);
                StartBlock();
                ProcessType(aChild, aCurrentState, aCurrentState, aNextState);
                EndBlock();

                aCurrentState = aNextState;
            }

            // max -> end
            AddComment("Occurrence: forward to local end");
            AddEpsilonTransition(aCurrentState, maEndState);
        }
        EndBlock();
    }




    /** Ordered sequence of nodes.
     *  For n nodes create states S0 to Sn where Si and Si+1 become start and
     *  end states for the i-th child.
     */
    @Override
    public void Visit (final Sequence aSequence)
    {
        AddComment("Sequence.");

        StartBlock();
        int nStateIndex = 0;
        State aCurrentState = maStateContext.CreateState(maBaseState, "S"+nStateIndex++);
        AddEpsilonTransition(maStartState, aCurrentState);
        for (final INode aChild : aSequence.GetChildren())
        {
            final State aNextState = maStateContext.CreateState(maBaseState, "S"+nStateIndex++);
            ProcessType(aChild, aCurrentState, aCurrentState, aNextState);
            aCurrentState = aNextState;
        }
        AddEpsilonTransition(aCurrentState, maEndState);
        EndBlock();
    }




    @Override
    public void Visit (final SimpleContent aSimpleContent)
    {
        AddComment("SimpleContent.");
    }




    @Override
    public void Default (final INode aNode)
    {
        throw new RuntimeException("can not handle "+aNode.toString());
    }




    private void ProcessType (
        final INode aNode,
        final State aBaseState,
        final State aStartState,
        final State aEndState)
    {
        aNode.AcceptVisitor(
            new ValidatingCreatorVisitor(
                maAttributes,
                maStateContext,
                maSchemaBase,
                maLog,
                msLogIndentation,
                aBaseState,
                aStartState,
                aEndState));
    }




    private void ProcessAttribute (
        final Attribute aAttribute)
    {
        Vector<Attribute> aAttributes = maAttributes.get(maStartState);
        if (aAttributes == null)
        {
            aAttributes = new Vector<>();
            maAttributes.put(maStartState, aAttributes);
        }
        aAttributes.add(aAttribute);

        if (maLog != null)
            maLog.printf("%sattribute %s\n", msLogIndentation, aAttribute.toString());
    }




    private void AddEpsilonTransition (
        final State aStartState,
        final State aEndState)
    {
        // Silently ignore epsilon transitions from a state to itself.
        // They may indicate a problem but usually are just artifacts
        // that can be safely ignored.
        if (aStartState == aEndState)
            return;
        else
        {
            final EpsilonTransition aTransition = new EpsilonTransition(
                aStartState,
                aEndState);
            aStartState.AddEpsilonTransition(aTransition);

            if (maLog != null)
            {
                maLog.printf("%sepsilon transition from %s to %s\n",
                    msLogIndentation,
                    aStartState.GetFullname(),
                    aEndState.GetFullname());
            }
        }
    }




    private void AddSkipTransition (
        final State aState,
        final SkipData aSkipData)
    {
        aState.AddSkipData(aSkipData);

        if (maLog != null)
        {
            maLog.printf("%sskip state %s\n",
                msLogIndentation,
                aState.GetFullname());
        }
    }




    private void AddComment (
        final String sFormat,
        final Object ... aArgumentList)
    {
        if (maLog != null)
        {
            maLog.print(msLogIndentation);
            maLog.print("// ");
            maLog.printf(sFormat, aArgumentList);
            maLog.print("\n");
        }
    }




    private void StartBlock ()
    {
        if (maLog != null)
            msLogIndentation += "    ";
    }




    private void EndBlock ()
    {
        if (maLog != null)
            msLogIndentation = msLogIndentation.substring(4);
    }




    private int GetElementCount (final INode aNode)
    {

        class Visitor extends NodeVisitorAdapter
        {
            int nElementCount = 0;
            @Override public void Visit (final Element aElement)
            {
                ++nElementCount;
            }
            int GetElementCount ()
            {
                return nElementCount;
            }
        };
        final Visitor aVisitor = new Visitor();
        for (final INode aChildNode : new DereferencingNodeIterator(aNode, maSchemaBase, false))
        {
            aChildNode.AcceptVisitor(aVisitor);
        }
        return aVisitor.GetElementCount();
    }




    private INode GetAllReplacement (final All aAll)
    {
        // By default each child of this node can appear exactly once, however
        // the order is undefined.  This corresponds to an enumeration of all
        // permutations of the children.

        // Set up an array of all children.  This array will be modified to contain
        // all permutations.
        final INode[] aNodes = new INode[aAll.GetChildCount()];
        final Iterator<INode> aChildren = aAll.GetChildren().iterator();
        for (int nIndex=0; aChildren.hasNext(); ++nIndex)
            aNodes[nIndex] = aChildren.next();

        final Location aLocation = aAll.GetLocation();
        final Choice aChoice = new Choice(aAll, aLocation);

        // Treat every permutation as sequence so that the whole set of permutations
        // is equivalent to a choice of sequences.
        int nCount = 0;
        for (final PermutationIterator<INode> aIterator = new PermutationIterator<>(aNodes); aIterator.HasMore(); aIterator.Next())
        {
            // Create a Sequence node for the current permutation and add it as
            // choice to the Choice node.
            final Sequence aSequence = new Sequence(aChoice, null, aLocation);
            aChoice.AddChild(aSequence);

            for (final INode aNode : aNodes)
                aSequence.AddChild(aNode);

            ++nCount;
        }
        System.out.printf("there are %d permutations\n", nCount);

        return aChoice;
    }




    private final Map<State,Vector<Attribute>> maAttributes;
    private final StateContext maStateContext;
    private final SchemaBase maSchemaBase;
    private final State maBaseState;
    private final State maStartState;
    private final State maEndState;
    private final PrintStream maLog;
    private String msLogIndentation;
}
