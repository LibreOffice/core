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
import java.util.Iterator;
import java.util.Stack;
import java.util.Vector;

import org.apache.openoffice.ooxml.schema.iterator.DereferencingNodeIterator;
import org.apache.openoffice.ooxml.schema.iterator.PermutationIterator;
import org.apache.openoffice.ooxml.schema.model.attribute.Attribute;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroup;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeGroupReference;
import org.apache.openoffice.ooxml.schema.model.attribute.AttributeReference;
import org.apache.openoffice.ooxml.schema.model.base.INode;
import org.apache.openoffice.ooxml.schema.model.base.INodeVisitor;
import org.apache.openoffice.ooxml.schema.model.base.Location;
import org.apache.openoffice.ooxml.schema.model.base.NodeVisitorAdapter;
import org.apache.openoffice.ooxml.schema.model.complex.All;
import org.apache.openoffice.ooxml.schema.model.complex.Any;
import org.apache.openoffice.ooxml.schema.model.complex.Choice;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexContent;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexType;
import org.apache.openoffice.ooxml.schema.model.complex.ComplexTypeReference;
import org.apache.openoffice.ooxml.schema.model.complex.Element;
import org.apache.openoffice.ooxml.schema.model.complex.ElementReference;
import org.apache.openoffice.ooxml.schema.model.complex.Extension;
import org.apache.openoffice.ooxml.schema.model.complex.Group;
import org.apache.openoffice.ooxml.schema.model.complex.GroupReference;
import org.apache.openoffice.ooxml.schema.model.complex.OccurrenceIndicator;
import org.apache.openoffice.ooxml.schema.model.complex.Sequence;
import org.apache.openoffice.ooxml.schema.model.schema.SchemaBase;
import org.apache.openoffice.ooxml.schema.model.simple.BuiltIn;
import org.apache.openoffice.ooxml.schema.model.simple.List;
import org.apache.openoffice.ooxml.schema.model.simple.Restriction;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleContent;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleType;
import org.apache.openoffice.ooxml.schema.model.simple.SimpleTypeReference;
import org.apache.openoffice.ooxml.schema.model.simple.Union;

/** Create a set of validating stack automatons for a set of schemas.
 *  There is one DFA (deterministic finite automaton) for each complex type and
 *  one for the top level elements.
 */
public class ValidatingCreator
    extends CreatorBase
    implements INodeVisitor
{
    public ValidatingCreator (
        final SchemaBase aSchemaBase,
        final File aLogFile)
    {
        super(aSchemaBase, aLogFile);
        maContextStack = new Stack<>();
        maCurrentContext = null;
    }




    /** Create one automaton for the top-level elements and one for each complex
     *  type.
     */
    public FiniteAutomatonContainer Create ()
    {
        final FiniteAutomatonContainer aAutomatons = new FiniteAutomatonContainer(maStateContainer);

        // Create the automaton for the top-level elements.
        aAutomatons.AddAutomaton(
            null,
            CreateForTopLevelElements());

        // Create one automation for each complex type.
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




    private FiniteAutomaton CreateForTopLevelElements ()
    {
        maStateContext = new StateContext(
            maStateContainer,
            "<top-level>");
        final State aEndState = maStateContext.CreateEndState();

        assert(maContextStack.isEmpty());
        msLogIndentation = "";

        // top level elements
        for (final Element aElement : maSchemaBase.TopLevelElements.GetSorted())
            ProcessType(
                aElement,
                maStateContext.GetStartState(),
                maStateContext.GetStartState(),
                aEndState);

        return new FiniteAutomaton(maStateContext, null, null);
    }




    private FiniteAutomaton CreateForComplexType (final ComplexType aComplexType)
    {
        maStateContext = new StateContext(
            maStateContainer,
            aComplexType.GetName().GetStateName());
        maAttributes = new Vector<>();
        final State aEndState = maStateContext.CreateEndState();
        ProcessType(
            aComplexType,
            maStateContext.GetStartState(),
            maStateContext.GetStartState(),
            aEndState);
        return new FiniteAutomaton(
            maStateContext,
            maAttributes,
            aComplexType.GetLocation());
    }




    @Override
    public void Visit (final All aAll)
    {
        maLog.AddComment("All");
        ProcessAttributes(aAll);

        // Make a transformation of the children into a choice of sequences that
        // can then be processed by already existing Visit() methods.
        // These sequences enumerate all permutations of the original children.
        final INode aReplacement = GetAllReplacement(aAll);

        final State aLocalStartState = maStateContext.CreateState(
            maCurrentContext.BaseState,
            "As");
        final State aLocalEndState = maStateContext.CreateState(
            maCurrentContext.BaseState,
            "Ae");

        maLog.StartBlock();
        AddEpsilonTransition(maCurrentContext.StartState, aLocalStartState);
        final long nStartTime = System.currentTimeMillis();
        ProcessType(
            aReplacement,
            maStateContext.CreateState(maCurrentContext.BaseState, "A"),
            aLocalStartState,
            aLocalEndState);
        final long nEndTime = System.currentTimeMillis();
        System.out.printf("processed 'all' children in %fs\n", (nEndTime-nStartTime)/1000.0);
        AddEpsilonTransition(aLocalEndState, maCurrentContext.EndState);
        maLog.EndBlock();
    }




    @Override
    public void Visit (final Any aAny)
    {
        assert(aAny.GetChildCount() == 0);

        maLog.AddComment("Any");
        ProcessAttributes(aAny);

        AddSkipTransition(
            maCurrentContext.StartState,
            new SkipData(
                aAny.GetProcessContentsFlag(),
                aAny.GetNamespaces()));
        AddEpsilonTransition(maCurrentContext.StartState, maCurrentContext.EndState);
    }




    @Override
    public void Visit (final ComplexContent aComplexContent)
    {
        assert(aComplexContent.GetChildCount() == 1);

        maLog.AddComment ("Complex Content.");
        ProcessAttributes(aComplexContent);

        maLog.StartBlock();
        ProcessType(
            aComplexContent.GetChildren().iterator().next(),
            maCurrentContext.BaseState,
            maCurrentContext.StartState,
            maCurrentContext.EndState);
        maLog.EndBlock();
    }




    @Override
    public void Visit (final ComplexType aComplexType)
    {
        if (maLog != null)
        {
            maLog.printf("\n");
            maLog.AddComment ("Complex Type %s defined in %s.",
                aComplexType.GetName().GetDisplayName(),
                aComplexType.GetLocation());
        }
        ProcessAttributes(aComplexType);

        maLog.StartBlock();
        maLog.printf("%sstarting at state %s\n", msLogIndentation, maCurrentContext.StartState.GetFullname());

        if (GetElementCount(aComplexType) == 0)
        {
            // There are elements. Therefore there will be no transitions.
            // The start state is accepting and the end state is not necessary.
            maCurrentContext.StartState.SetIsAccepting();
            maStateContext.RemoveState(maCurrentContext.EndState);
        }

        for (final INode aChild : aComplexType.GetChildren())
            ProcessType(aChild, maCurrentContext.BaseState, maCurrentContext.StartState, maCurrentContext.EndState);

        maLog.EndBlock();
    }




    @Override
    public void Visit (final ComplexTypeReference aNode)
    {
        throw new RuntimeException("can not handle "+aNode.toString());
    }




    @Override
    public void Visit (final Choice aChoice)
    {
        maLog.AddComment("Choice");
        ProcessAttributes(aChoice);

        final State aLocalStartState = maStateContext.CreateState(maCurrentContext.BaseState, "Cs");
        final State aLocalEndState = maStateContext.CreateState(maCurrentContext.BaseState, "Ce");
        maLog.StartBlock();
        AddEpsilonTransition(maCurrentContext.StartState, aLocalStartState);

        int nStateIndex = 0;
        for (final INode aChild : aChoice.GetChildren())
        {
            ProcessType(
                aChild,
                maStateContext.CreateState(maCurrentContext.BaseState, "C"+nStateIndex++),
                aLocalStartState,
                aLocalEndState);
        }
        AddEpsilonTransition(aLocalEndState, maCurrentContext.EndState);
        maLog.EndBlock();
    }




    @Override
    public void Visit (final Element aElement)
    {
        assert(aElement.GetChildCount()==0);

        maLog.AddComment("Element: on '%s' go from %s to %s via %s",
            aElement.GetElementName().GetDisplayName(),
            maCurrentContext.StartState.GetFullname(),
            maCurrentContext.EndState.GetFullname(),
            aElement.GetTypeName().GetStateName());
        ProcessAttributes(aElement);

        final Transition aTransition = new Transition(
            maCurrentContext.StartState,
            maCurrentContext.EndState,
            aElement.GetElementName(),
            aElement.GetTypeName().GetStateName());
        maCurrentContext.StartState.AddTransition(aTransition);

        // For elements whose type is a simple type we have to remember that
        // simple type for later (and then create an NFA for it.)
        final INode aSimpleType = maSchemaBase.GetSimpleTypeForName(
            aElement.GetTypeName());
        if (aSimpleType != null)
            maElementSimpleTypes.add(aSimpleType);
    }




    @Override
    public void Visit (final ElementReference aReference)
    {
        assert(aReference.GetChildCount() == 0);

        maLog.AddComment("Element reference to %s", aReference.GetReferencedElementName());
        ProcessAttributes(aReference);

        final Element aElement = aReference.GetReferencedElement(maSchemaBase);
        if (aElement == null)
            throw new RuntimeException("can't find referenced element "+aReference.GetReferencedElementName());
        maLog.StartBlock();
        ProcessType(aElement, maCurrentContext.BaseState, maCurrentContext.StartState, maCurrentContext.EndState);
        maLog.EndBlock();
    }




    /** Treat extension nodes like sequences (for now).
     */
    @Override
    public void Visit (final Extension aExtension)
    {
        assert(aExtension.GetChildCount() <= 1);

        maLog.AddComment("Extension of base type %s", aExtension.GetBaseTypeName());
        ProcessAttributes(aExtension);

        final Vector<INode> aNodes = aExtension.GetTypeNodes(maSchemaBase);

        maLog.StartBlock();
        int nStateIndex = 0;
        State aCurrentState = maStateContext.CreateState(maCurrentContext.BaseState, "E"+nStateIndex++);
        AddEpsilonTransition(maCurrentContext.StartState, aCurrentState);

        State aNextState = maStateContext.CreateState(maCurrentContext.BaseState, "E"+nStateIndex++);
        ProcessType(aExtension.GetReferencedNode(maSchemaBase), aCurrentState, aCurrentState, aNextState);
        aCurrentState = aNextState;

        for (final INode aChild : aNodes)
        {
            aNextState = maStateContext.CreateState(maCurrentContext.BaseState, "E"+nStateIndex++);
            ProcessType(aChild, aCurrentState, aCurrentState, aNextState);
            aCurrentState = aNextState;
        }
        AddEpsilonTransition(aCurrentState, maCurrentContext.EndState);
        maLog.EndBlock();
    }




    @Override
    public void Visit (final Group aGroup)
    {
        assert(aGroup.GetChildCount() == 1);

        maLog.AddComment("Group %s", aGroup.GetName());
        ProcessAttributes(aGroup);

        maLog.StartBlock();
        final State aGroupBaseState = maStateContext.CreateState(maCurrentContext.BaseState, "G");
        ProcessType(
            aGroup.GetOnlyChild(),
            aGroupBaseState,
            maCurrentContext.StartState,
            maCurrentContext.EndState);
        maLog.EndBlock();
    }




    @Override
    public void Visit (final GroupReference aReference)
    {
        maLog.AddComment("Group reference to %s", aReference.GetReferencedGroupName());
        ProcessAttributes(aReference);

        final Group aGroup = aReference.GetReferencedGroup(maSchemaBase);
        if (aGroup == null)
            throw new RuntimeException("can't find referenced group "+aReference.GetReferencedGroupName());

        maLog.StartBlock();
        ProcessType(aGroup, maCurrentContext.BaseState, maCurrentContext.StartState, maCurrentContext.EndState);
        maLog.EndBlock();
    }




    /** An occurrence indicator defines how many times the single child can occur.
     *  The minimum value defines the mandatory number of times.  The maximum value
     *  defines the optional number.
     */
    @Override
    public void Visit (final OccurrenceIndicator aOccurrence)
    {
        assert(aOccurrence.GetChildCount() == 1);

        maLog.AddComment("OccurrenceIndicator %s->%s",
            aOccurrence.GetDisplayMinimum(),
            aOccurrence.GetDisplayMaximum());
        ProcessAttributes(aOccurrence);

        maLog.StartBlock();

        final INode aChild = aOccurrence.GetChildren().iterator().next();

        int nIndex = 0;
        State aCurrentState = maStateContext.CreateState(maCurrentContext.BaseState, "O"+nIndex++);
        AddEpsilonTransition(maCurrentContext.StartState, aCurrentState);

        if (aOccurrence.GetMinimum() == 0)
        {
            // A zero minimum means that all occurrences are optional.
            // Add a short circuit from start to end.
            maLog.AddComment("Occurrence: make whole element optional (min==0)");
            AddEpsilonTransition(maCurrentContext.StartState, maCurrentContext.EndState);
        }
        else
        {
            // Write a row of mandatory transitions for the minimum.
            for (; nIndex<=aOccurrence.GetMinimum(); ++nIndex)
            {
                // Add transition i-1 -> i (i == nIndex).
                final State aNextState = maStateContext.CreateState(maCurrentContext.BaseState, "O"+nIndex);
                maLog.AddComment("Occurrence: move from %d -> %d (%s -> %s) (minimum)",
                    nIndex-1,
                    nIndex,
                    aCurrentState,
                    aNextState);
                maLog.StartBlock();
                ProcessType(aChild, aCurrentState, aCurrentState, aNextState);
                maLog.EndBlock();
                aCurrentState = aNextState;
            }
        }

        if (aOccurrence.GetMaximum() == OccurrenceIndicator.unbounded)
        {
            // Write loop on last state when max is unbounded.

            // last -> loop
            final State aLoopState = maStateContext.CreateState(maCurrentContext.BaseState, "OL");
            maLog.AddComment("Occurrence: forward to loop (maximum)");
            AddEpsilonTransition(aCurrentState, aLoopState);

            // loop -> loop
            maLog.AddComment("Occurrence: loop");
            maLog.StartBlock();
            ProcessType(aChild, aLoopState, aLoopState, aLoopState);
            maLog.EndBlock();

            // -> end
            maLog.AddComment("Occurrence: forward to local end");
            AddEpsilonTransition(aLoopState, maCurrentContext.EndState);
        }
        else
        {
            // Write a row of optional transitions for the maximum.
            for (; nIndex<=aOccurrence.GetMaximum(); ++nIndex)
            {
                if (nIndex > 0)
                {
                    // i-1 -> end
                    maLog.AddComment("Occurrence: make %d optional (maximum)", nIndex-1);
                    AddEpsilonTransition(aCurrentState, maCurrentContext.EndState);
                }

                // i-1 -> i
                final State aNextState = maStateContext.CreateState(maCurrentContext.BaseState, "O"+nIndex);
                maLog.AddComment("Occurrence: %d -> %d (%s -> %s) (maximum)",
                    nIndex-1,
                    nIndex,
                    aCurrentState,
                    aNextState);
                maLog.StartBlock();
                ProcessType(aChild, aCurrentState, aCurrentState, aNextState);
                maLog.EndBlock();

                aCurrentState = aNextState;
            }

            // max -> end
            maLog.AddComment("Occurrence: forward to local end");
            AddEpsilonTransition(aCurrentState, maCurrentContext.EndState);
        }
        maLog.EndBlock();
    }




    /** Ordered sequence of nodes.
     *  For n nodes create states S0 to Sn where Si and Si+1 become start and
     *  end states for the i-th child.
     */
    @Override
    public void Visit (final Sequence aSequence)
    {
        maLog.AddComment("Sequence.");
        ProcessAttributes(aSequence);

        maLog.StartBlock();
        int nStateIndex = 0;
        State aCurrentState = maStateContext.CreateState(maCurrentContext.BaseState, "S"+nStateIndex++);
        AddEpsilonTransition(maCurrentContext.StartState, aCurrentState);
        for (final INode aChild : aSequence.GetChildren())
        {
            final State aNextState = maStateContext.CreateState(maCurrentContext.BaseState, "S"+nStateIndex++);
            ProcessType(aChild, aCurrentState, aCurrentState, aNextState);
            aCurrentState = aNextState;
        }
        AddEpsilonTransition(aCurrentState, maCurrentContext.EndState);
        maLog.EndBlock();
    }




    @Override
    public void Visit (final BuiltIn aNode)
    {
        // Ignored.
        //throw new RuntimeException("can not handle "+aNode.toString());
    }




    @Override
    public void Visit (final List aNode)
    {
        throw new RuntimeException("can not handle "+aNode.toString());
    }




    @Override
    public void Visit (final Restriction aNode)
    {
        throw new RuntimeException("can not handle "+aNode.toString());
    }



    @Override
    public void Visit (final SimpleContent aNode)
    {
        maLog.AddComment("SimpleContent.");
        ProcessAttributes(aNode);

        for (final INode aChild : aNode.GetChildren())
            ProcessType(aChild, maCurrentContext.BaseState, maCurrentContext.StartState, maCurrentContext.EndState);
    }




    @Override
    public void Visit (final SimpleType aNode)
    {
        maLog.AddComment("SimpleType.");
        //for (final INode aChild : aNode.GetChildren())
            //ProcessType(aChild, maCurrentContext.BaseState, maCurrentContext.StartState, maCurrentContext.EndState);
    }




    @Override
    public void Visit (final SimpleTypeReference aNode)
    {
        throw new RuntimeException("can not handle "+aNode.toString());
    }




    @Override
    public void Visit (final Union aNode)
    {
        throw new RuntimeException("can not handle "+aNode.toString());
    }




    @Override
    public void Visit (final AttributeGroup aNode)
    {
        throw new RuntimeException("can not handle "+aNode.toString());
    }




    @Override
    public void Visit (final AttributeReference aNode)
    {
        throw new RuntimeException("can not handle "+aNode.toString());
    }




    @Override
    public void Visit (final Attribute aNode)
    {
        throw new RuntimeException("can not handle "+aNode.toString());
    }




    @Override
    public void Visit (final AttributeGroupReference aNode)
    {
        throw new RuntimeException("can not handle "+aNode.toString());
    }




    private void ProcessType (
        final INode aNode,
        final State aBaseState,
        final State aStartState,
        final State aEndState)
    {
        maContextStack.push(maCurrentContext);
        maCurrentContext = new Context(aBaseState, aStartState, aEndState);
        aNode.AcceptVisitor(this);
        maCurrentContext = maContextStack.pop();
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
        final long nStartTime = System.currentTimeMillis();

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
        final long nEndTime = System.currentTimeMillis();
        System.out.printf("created %d permutations in %fs\n",
            nCount,
            (nEndTime-nStartTime)/1000.0);

        return aChoice;
    }




    class Context
    {
        Context (
            final State aBaseState,
            final State aStartState,
            final State aEndState)
        {
            BaseState = aBaseState;
            StartState = aStartState;
            EndState = aEndState;
        }
        final State BaseState;
        final State StartState;
        final State EndState;
    }




    private StateContext maStateContext;
    private final Stack<Context> maContextStack;
    private Context maCurrentContext;
}
