/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Jonas Finnemann Jensen <jopsen@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Jonas Finnemann Jensen <jopsen@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef CARET_H
#define CARET_H

#include "node.hxx"

/** Representation of caret position with an equantion */
struct SmCaretPos{
    SmCaretPos(SmNode* selectedNode = NULL, int iIndex = 0) {
        pSelectedNode = selectedNode;
        Index = iIndex;
    }
    /** Selected node */
    SmNode* pSelectedNode;
    /** Index within the selected node
     *
     * 0: Position in front of a node
     * 1: Position after a node or after first char in SmTextNode
     * n: Position after n char in SmTextNode
     *
     * Notice how there's special cases for SmTextNode.
     */
    //TODO: Special cases for SmBlankNode is needed
    //TODO: Consider forgetting about the todo above... As it's really unpleasent.
    int Index;
    /** True, if this is a valid caret position */
    bool IsValid() const { return pSelectedNode != NULL; }
    bool operator!=(SmCaretPos pos) const {
        return pos.pSelectedNode != pSelectedNode || Index != pos.Index;
    }
    bool operator==(SmCaretPos pos) const {
        return pos.pSelectedNode == pSelectedNode && Index == pos.Index;
    }
    /** Get the caret position after pNode, regardless of pNode
     *
     * Gets the caret position following pNode, this is SmCaretPos(pNode, 1).
     * Unless pNode is an instance of SmTextNode, then the index is the text length.
     */
    static SmCaretPos GetPosAfter(SmNode* pNode) {
        if(pNode && pNode->GetType() == NTEXT)
            return SmCaretPos(pNode, ((SmTextNode*)pNode)->GetText().getLength());
        return SmCaretPos(pNode, 1);
    }
};

/** A line that represents a caret */
class SmCaretLine{
public:
    SmCaretLine(long left = 0, long top = 0, long height = 0) {
        _top = top;
        _left = left;
        _height = height;
    }
    long GetTop() const {return _top;}
    long GetLeft() const {return _left;}
    long GetHeight() const {return _height;}
    long SquaredDistanceX(SmCaretLine line) const{
        return (GetLeft() - line.GetLeft()) * (GetLeft() - line.GetLeft());
    }
    long SquaredDistanceX(Point pos) const{
        return (GetLeft() - pos.X()) * (GetLeft() - pos.X());
    }
    long SquaredDistanceY(SmCaretLine line) const{
        long d = GetTop() - line.GetTop();
        if(d < 0)
            d = (d * -1) - GetHeight();
        else
            d = d - line.GetHeight();
        if(d < 0)
            return 0;
        return d * d;
    }
    long SquaredDistanceY(Point pos) const{
        long d = GetTop() - pos.Y();
        if(d < 0)
            d = (d * -1) - GetHeight();
        if(d < 0)
            return 0;
        return d * d;
    }
private:
    long _top;
    long _left;
    long _height;
};

/////////////////////////////// SmCaretPosGraph////////////////////////////////

/** An entry in SmCaretPosGraph */
struct SmCaretPosGraphEntry{
    SmCaretPosGraphEntry(SmCaretPos pos = SmCaretPos(),
                       SmCaretPosGraphEntry* left = NULL,
                       SmCaretPosGraphEntry* right = NULL){
        CaretPos = pos;
        Left = left;
        Right = right;
    }
    /** Caret position */
    SmCaretPos CaretPos;
    /** Entry to the left visually */
    SmCaretPosGraphEntry* Left;
    /** Entry to the right visually */
    SmCaretPosGraphEntry* Right;
    void SetRight(SmCaretPosGraphEntry* right){
        Right = right;
    }
    void SetLeft(SmCaretPosGraphEntry* left){
        Left = left;
    }
};

/** Define SmCaretPosGraph to be less than one page 4096 */
#define SmCaretPosGraphSize   255

class SmCaretPosGraph;

/** Iterator for SmCaretPosGraph */
class SmCaretPosGraphIterator{
public:
    SmCaretPosGraphIterator(SmCaretPosGraph* graph){
        pGraph = graph;
        nOffset = 0;
        pEntry = NULL;
    }
    /** Get the next entry, NULL if none */
    SmCaretPosGraphEntry* Next();
    /** Get the current entry, NULL if none */
    SmCaretPosGraphEntry* Current(){
        return pEntry;
    }
    /** Get the current entry, NULL if none */
    SmCaretPosGraphEntry* operator->(){
        return pEntry;
    }
private:
    /** Next entry to return */
    int nOffset;
    /** Current graph */
    SmCaretPosGraph* pGraph;
    /** Current entry */
    SmCaretPosGraphEntry* pEntry;
};


/** A graph over all caret positions
 * @remarks Graphs can only grow, entries cannot be removed!
 */
class SmCaretPosGraph{
public:
    SmCaretPosGraph(){
        pNext = NULL;
        nOffset = 0;
    }
    ~SmCaretPosGraph();
    /** Add a caret position
     *  @remarks If Left and/or Right are set NULL, they will point back to the entry.
     */
    SmCaretPosGraphEntry* Add(SmCaretPosGraphEntry entry);
    /** Add a caret position
     *  @remarks If left and/or right are set NULL, they will point back to the entry.
     */
    SmCaretPosGraphEntry* Add(SmCaretPos pos,
                            SmCaretPosGraphEntry* left = NULL,
                            SmCaretPosGraphEntry* right = NULL){
        OSL_ENSURE(pos.Index >= 0, "Index shouldn't be -1!");
        return Add(SmCaretPosGraphEntry(pos, left, right));
    }
    /** Get an iterator for this graph */
    SmCaretPosGraphIterator GetIterator(){
        return SmCaretPosGraphIterator(this);
    }
    friend class SmCaretPosGraphIterator;
private:
    /** Next graph, to be used when this graph is full */
    SmCaretPosGraph* pNext;
    /** Next free entry in graph */
    int nOffset;
    /** Entries in this graph segment */
    SmCaretPosGraphEntry Graph[SmCaretPosGraphSize];
};

/** \page visual_formula_editing Visual Formula Editing
 * A visual formula editor allows users to easily edit formulas without having to learn and
 * use complicated commands. A visual formula editor is a WYSIWYG editor. For OpenOffice Math
 * this essentially means that you can click on the formula image, to get a caret, which you
 * can move with arrow keys, and use to modify the formula by entering text, clicking buttons
 * or using shortcuts.
 *
 * \subsection formula_trees Formula Trees
 * A formula in OpenOffice Math is a tree of nodes, take for instance the formula
 * "A + {B cdot C} over D", it looks like this
 * \f$ \mbox{A} + \frac{\mbox{B} \cdot \mbox{C}}{\mbox{D}} \f$. The tree for this formula
 * looks like this:
 *
 * \dot
 * digraph {
 * labelloc = "t";
 * label= "Equation: \"A  + {B  cdot C} over D\"";
 * size = "9,9";
 * n0 [label="SmTableNode (1)"];
 * n0 -> n1 [label="0"];
 * n1 [label="SmLineNode (2)"];
 * n1 -> n2 [label="0"];
 * n2 [label="SmExpressionNode (3)"];
 * n2 -> n3 [label="0"];
 * n3 [label="SmBinHorNode (4)"];
 * n3 -> n4 [label="0"];
 * n4 [label="SmTextNode: A (5)"];
 * n3 -> n5 [label="1"];
 * n5 [label="SmMathSymbolNode:  (6)"];
 * n3 -> n6 [label="2"];
 * n6 [label="SmBinVerNode (7)"];
 * n6 -> n7 [label="0"];
 * n7 [label="SmExpressionNode (8)"];
 * n7 -> n8 [label="0"];
 * n8 [label="SmBinHorNode (9)"];
 * n8 -> n9 [label="0"];
 * n9 [label="SmTextNode: B (10)"];
 * n8 -> n10 [label="1"];
 * n10 [label="SmMathSymbolNode: ⋅ (11)"];
 * n8 -> n11 [label="2"];
 * n11 [label="SmTextNode: C (12)"];
 * n6 -> n12 [label="1"];
 * n12 [label="SmRectangleNode (13)"];
 * n6 -> n13 [label="2"];
 * n13 [label="SmTextNode: D (14)"];
 * }
 * \enddot
 *
 * The vertices are nodes, their label says what kind of node and the number in parentheses is
 *  the identifier of the node (In practices a pointer is used instead of the id). The direction
 *  of the edges tells which node is parent and which is child. The label of the edges are the
 *  child node index number, given to SmNode::GetSubNode() of the parent to get the child node.
 *
 *
 * \subsection visual_lines Visual Lines
 *
 * Inorder to do caret movement in visual lines, we need a definition of caret position and
 * visual line. In a tree such as the above there are three visual lines. There's the outer most
 * line, with entries such as
 * \f$\mbox{A}\f$, \f$ + \f$ and \f$ \frac{\mbox{B} \cdot \mbox{C}}{\mbox{D}} \f$. Then there's
 *  the numerator line of the fraction it has entries \f$ \mbox{B} \f$, \f$ \cdot \f$ and \f$ \mbox{C} \f$.
 *  And last by not least there's the denominator line of the fraction it's only entry is \f$ \mbox{D} \f$.
 *
 * For visual editing it should be possible to place a caret on both sides of any line entry,
 * consider a line entry a character or construction that in a line is treated as a character.
 *  Imagine the caret is placed to the right of the plus sign (id: 6), now if user presses
 * backspace this should delete the plus sign (id: 6), and if the user presses delete this
 * should delete the entire fraction (id: 7). This is because the caret is in the outer most
 * line where the fraction is considered a line entry.
 *
 * However, inorder to prevent users from accidentally deleting large subtrees, just because
 * they logically placed there caret a in the wrong line, require that complex constructions
 * such as a fraction is selected before it is deleted. Thus in this case it wouldn't be
 * deleted, but only selected and then deleted if the user hit delete again. Anyway, this is
 * slightly off topic for now.
 *
 * Important about visual lines is that they don't always have an SmExpressionNode as root
 * and the entries in a visual line is all the nodes of a subtree ordered left to right that
 *  isn't either an SmExpressionNode, SmBinHorNode or SmUnHorNode.
 *
 *
 * \subsection caret_positions Caret Positions
 *
 * A caret position in OpenOffice Math is representated by an instance of SmCaretPos.
 * That is a caret position is a node and an index related to this node. For most nodes the
 * index 0, means caret is in front of this node, the index 1 means caret is after this node.
 * For SmTextNode the index is the caret position after the specified number of characters,
 * imagine an SmTextNode with the number 1337. The index 3 in such SmTextNode would mean a
 * caret placed right before 7, e.g. "133|7".
 *
 * For SmExpressionNode, SmBinHorNode and SmUnHorNode the only legal index is 0, which means
 * in front of the node. Actually the index 0 may only because for the first caret position
 * in a visual line. From the example above, consider the following subtree that constitutes
 * a visual line:
 *
 * \dot
 * digraph {
 * labelloc = "t";
 * label= "Subtree that constitutes a visual line";
 * size = "7,5";
 * n7 [label="SmExpressionNode (8)"];
 * n7 -> n8 [label="0"];
 * n8 [label="SmBinHorNode (9)"];
 * n8 -> n9 [label="0"];
 * n9 [label="SmTextNode: B (10)"];
 * n8 -> n10 [label="1"];
 * n10 [label="SmMathSymbolNode: ⋅ (11)"];
 * n8 -> n11 [label="2"];
 * n11 [label="SmTextNode: C (12)"];
 * }
 * \enddot
 * Here the caret positions are:
 *
 * <TABLE>
 * <TR><TD><B>Caret position:</B></TD><TD><B>Example:</B></TD>
 * </TR><TR>
 *     <TD>{id: 8, index: 0}</TD>
 *     <TD>\f$ \mid \mbox{C} \cdot \mbox{C} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 10, index: 1}</TD>
 *     <TD>\f$ \mbox{C} \mid \cdot \mbox{C} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 11, index: 1}</TD>
 *     <TD>\f$ \mbox{C} \cdot \mid \mbox{C} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 12, index: 1}</TD>
 *     <TD>\f$ \mbox{C} \cdot \mbox{C} \mid \f$</TD>
 * </TR><TR>
 * </TABLE>
 *
 * Where \f$ \mid \f$ is used to denote caret position.
 *
 * With these exceptions included in the definition the id and index: {id: 11, index: 0} does
 * \b not constitute a caret position in the given context. Note the method
 * SmCaretPos::IsValid() does not check if this invariant holds true, but code in SmCaret,
 * SmSetSelectionVisitor and other places depends on this invariant to hold.
 *
 *
 * \subsection caret_movement Caret Movement
 *
 * As the placement of caret positions depends very much on the context within which a node
 * appears it is not trivial to find all caret positions and determine which follows which.
 * In OpenOffice Math this is done by the SmCaretPosGraphBuildingVisitor. This visitor builds
 * graph (an instnce of SmCaretPosGraph) over the caret positions. For details on how this
 * graph is build, and how new methods should be implemented see SmCaretPosGraphBuildingVisitor.
 *
 * The result of the SmCaretPosGraphBuildingVisitor is a graph over the caret positions in a
 * formula, representated by an instance of SmCaretPosGraph. Each entry (instances of SmCaretPosGraphEntry)
 * has a pointer to the entry to the left and right of itself. This way we can easily find
 * the caret position to a right or left of a given caret position. Note each caret position
 * only appears once in this graph.
 *
 * When searching for a caret position after a left click on the formula this map is also used.
 * We simply iterate over all entries, uses the SmCaretPos2LineVisitor to find a line for each
 * caret position. Then the distance from the click to the line is computed and we choose the
 * caret position closest to the click.
 *
 * For up and down movement, we also iterator over all caret positions and use SmCaretPos2LineVisitor
 * to find a line for each caret position. Then we compute the distance from the current
 * caret position to every other caret position and chooses the one closest that is either
 * above or below the current caret position, depending on whether we're doing up or down movement.
 *
 * This result of this approach to caret movement is that we have logically predictable
 * movement for left and right, whilst leftclick, up and down movement depends on the sizes
 * and placement of all node and may be less logically predictable. This solution also means
 * that we only have one complex visitor generating the graph, imagine the nightmare if we
 * had a visitor for movement in each direction.
 *
 * Making up and down movement independent of node sizes and placement wouldn't necessarily
 * be a good thing either. Consider the formula \f$ \frac{1+2+3+4+5}{6} \f$, if the caret is
 * placed as displayed here: \f$ \frac{1+2+3+4+5}{6 \mid} \f$, up movement should move to right
 * after "3": \f$ \frac{1+2+3|+4+5}{6} \f$. However, such a move depends on the sizes and placement
 * of all nodes in the fraction.
 *
 *
 * \subsubsection caretpos_graph_example Example of Caret Position Graph
 *
 * If we consider the formula
 * \f$ \mbox{A} + \frac{\mbox{B} \cdot \mbox{C}}{\mbox{D}} \f$ from \ref formula_trees.
 * It has the following caret positions:
 *
 * <TABLE>
 * <TR>
 *     <TD><B>Caret position:</B></TD>
 *     <TD><B>Example:</B></TD>
 * </TR><TR>
 *     <TD>{id: 3, index: 0}</TD>
 *     <TD>\f$ \mid\mbox{A} + \frac{\mbox{B} \cdot \mbox{C}}{\mbox{D}} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 5, index: 1}</TD>
 *     <TD>\f$ \mbox{A}\mid + \frac{\mbox{B} \cdot \mbox{C}}{\mbox{D}} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 6, index: 1}</TD>
 *     <TD>\f$ \mbox{A} + \mid \frac{\mbox{B} \cdot \mbox{C}}{\mbox{D}} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 8, index: 0}</TD>
 *     <TD>\f$ \mbox{A} + \frac{ \mid \mbox{B} \cdot \mbox{C}}{\mbox{D}} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 10, index: 1}</TD>
 *     <TD>\f$ \mbox{A} + \frac{\mbox{B} \mid \cdot \mbox{C}}{\mbox{D}} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 11, index: 1}</TD>
 *     <TD>\f$ \mbox{A} + \frac{\mbox{B} \cdot \mid \mbox{C}}{\mbox{D}} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 12, index: 1}</TD>
 *     <TD>\f$ \mbox{A} + \frac{\mbox{B} \cdot \mbox{C} \mid}{\mbox{D}} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 14, index: 0}</TD>
 *     <TD>\f$ \mbox{A} + \frac{\mbox{B} \cdot \mbox{C}}{\mid \mbox{D}} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 14, index: 1}</TD>
 *     <TD>\f$ \mbox{A} + \frac{\mbox{B} \cdot \mbox{C}}{\mbox{D} \mid} \f$</TD>
 * </TR><TR>
 *     <TD>{id: 7, index: 1}</TD>
 *     <TD>\f$ \mbox{A} + \frac{\mbox{B} \cdot \mbox{C}}{\mbox{D}} \mid \f$</TD>
 * </TR>
 * </TABLE>
 *
 * Below is a directed graph over the caret postions and how you can move between them.
 * \dot
 * digraph {
 *     labelloc = "t";
 *     label= "Caret Position Graph";
 *     size = "4,6";
 *     p0 [label = "{id: 3, index: 0}"];
 *     p0 -> p1 [fontsize = 10.0, label = "right"];
 *     p1 [label = "{id: 5, index: 1}"];
 *     p1 -> p0 [fontsize = 10.0, label = "left"];
 *     p1 -> p2 [fontsize = 10.0, label = "right"];
 *     p2 [label = "{id: 6, index: 1}"];
 *     p2 -> p1 [fontsize = 10.0, label = "left"];
 *     p2 -> p3 [fontsize = 10.0, label = "right"];
 *     p3 [label = "{id: 8, index: 0}"];
 *     p3 -> p2 [fontsize = 10.0, label = "left"];
 *     p3 -> p4 [fontsize = 10.0, label = "right"];
 *     p4 [label = "{id: 10, index: 1}"];
 *     p4 -> p3 [fontsize = 10.0, label = "left"];
 *     p4 -> p5 [fontsize = 10.0, label = "right"];
 *     p5 [label = "{id: 11, index: 1}"];
 *     p5 -> p4 [fontsize = 10.0, label = "left"];
 *     p5 -> p6 [fontsize = 10.0, label = "right"];
 *     p6 [label = "{id: 12, index: 1}"];
 *     p6 -> p5 [fontsize = 10.0, label = "left"];
 *     p6 -> p9 [fontsize = 10.0, label = "right"];
 *     p7 [label = "{id: 14, index: 0}"];
 *     p7 -> p2 [fontsize = 10.0, label = "left"];
 *     p7 -> p8 [fontsize = 10.0, label = "right"];
 *     p8 [label = "{id: 14, index: 1}"];
 *     p8 -> p7 [fontsize = 10.0, label = "left"];
 *     p8 -> p9 [fontsize = 10.0, label = "right"];
 *     p9 [label = "{id: 7, index: 1}"];
 *     p9 -> p6 [fontsize = 10.0, label = "left"];
 * }
 * \enddot
 */

/* TODO: Write documentation about the following keywords:
 *
 * Visual Selections:
 *  - Show images
 *  - Talk about how the visitor does this
 *
 * Modifying a Visual Line:
 *  - Find top most non-compo of the line (e.g. The subtree that constitutes a line)
 *  - Make the line into a list
 *  - Edit the list, add/remove/modify nodes
 *  - Parse the list back into a subtree
 *  - Insert the new subtree where the old was taken
 */

#endif /* CARET_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
