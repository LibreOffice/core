/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_STARMATH_INC_CURSOR_HXX
#define INCLUDED_STARMATH_INC_CURSOR_HXX

#include "node.hxx"
#include "caret.hxx"

#include <cassert>
#include <list>
#include <memory>

/** Factor to multiple the squared horizontal distance with
 * Used for Up and Down movement.
 */
#define HORIZONTICAL_DISTANCE_FACTOR        10

/** Enum of direction for movement */
enum SmMovementDirection{
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight
};

/** Enum of elements that can inserted into a formula */
enum SmFormulaElement{
    BlankElement,
    FactorialElement,
    PlusElement,
    MinusElement,
    CDotElement,
    EqualElement,
    LessThanElement,
    GreaterThanElement,
    PercentElement
};

/** Bracket types that can be inserted */
enum class SmBracketType {
    /** Round brackets, left command "(" */
    Round,
    /**Square brackets, left command "[" */
    Square,
    /** Curly brackets, left command "lbrace" */
    Curly,
};

/** A list of nodes */
typedef std::list<SmNode*> SmNodeList;

typedef std::list<std::unique_ptr<SmNode>> SmClipboard;

class SmDocShell;

/** Formula cursor
 *
 * This class is used to represent a cursor in a formula, which can be used to manipulate
 * a formula programmatically.
 * @remarks This class is a very intimate friend of SmDocShell.
 */
class SmCursor{
public:
    SmCursor(SmNode* tree, SmDocShell* pShell)
        : mpAnchor(nullptr)
        , mpPosition(nullptr)
        , mpTree(tree)
        , mpDocShell(pShell)
        , mnEditSections(0)
        , mbIsEnabledSetModifiedSmDocShell(false)
    {
        //Build graph
        BuildGraph();
    }

    /** Get position */
    const SmCaretPos& GetPosition() const { return mpPosition->CaretPos; }

    /** True, if the cursor has a selection */
    bool HasSelection() const { return mpAnchor != mpPosition; }

    /** Move the position of this cursor */
    void Move(OutputDevice* pDev, SmMovementDirection direction, bool bMoveAnchor = true);

    /** Move to the caret position closest to a given point */
    void MoveTo(OutputDevice* pDev, const Point& pos, bool bMoveAnchor);

    /** Delete the current selection or do nothing */
    void Delete();

    /** Delete selection, previous element or merge lines
     *
     * This method implements the behaviour of backspace.
     */
    void DeletePrev(OutputDevice* pDev);

    /** Insert text at the current position */
    void InsertText(const OUString& aString);

    /** Insert an element into the formula */
    void InsertElement(SmFormulaElement element);

    /** Insert command text translated into line entries at position
     *
     * Note: This method uses the parser to translate a command text into a
     * tree, then it copies line entries from this tree into the current tree.
     * Will not work for commands such as newline or ##, if position is in a matrix.
     * This will work for stuff like "A intersection B". But stuff spanning multiple lines
     * or dependent on the context which position is placed in will not work!
     */
    void InsertCommandText(const OUString& aCommandText);

    /** Insert a special node created from aString
     *
     * Used for handling insert request from the "catalog" dialog.
     * The provided string should be formatted as the desired command: %phi
     * Note: this method ONLY supports commands defined in Math.xcu
     *
     * For more complex expressions use InsertCommandText, this method doesn't
     * use SmParser, this means that it's faster, but not as strong.
     */
    void InsertSpecial(const OUString& aString);

    /** Create sub-/super script
     *
     * If there's a selection, it will be move into the appropriate sub-/super scription
     * of the node in front of it. If there's no node in front of position (or the selection),
     * a sub-/super scription of a new SmPlaceNode will be made.
     *
     * If there's is an existing subscription of the node, the caret will be moved into it,
     * and any selection will replace it.
     */
    void InsertSubSup(SmSubSup eSubSup);

    /** Insert a new row or newline
     *
     * Inserts a new row if position is in a matrix or stack command.
     * Otherwise a newline is inserted if we're in a toplevel line.
     *
     * @returns True, if a new row/line could be inserted.
     *
     * @remarks If the caret is placed in a subline of a command that doesn't support
     *          this operator the method returns FALSE, and doesn't do anything.
     */
    bool InsertRow();

    /** Insert a fraction, use selection as numerator */
    void InsertFraction();

    /** Create brackets around current selection, or new SmPlaceNode */
    void InsertBrackets(SmBracketType eBracketType);

    /** Copy the current selection */
    void Copy();
    /** Cut the current selection */
    void Cut(){
        Copy();
        Delete();
    }
    /** Paste the clipboard */
    void Paste();

    /** Returns true if more than one node is selected
     *
     * This method is used for implementing backspace and delete.
     * If one of these causes a complex selection, e.g. a node with
     * subnodes or similar, this should not be deleted immediately.
     */
    bool HasComplexSelection();

    /** Finds the topmost node in a visual line
     *
     * If MoveUpIfSelected is true, this will move up to the parent line
     * if the parent of the current line is selected.
     */
    static SmNode* FindTopMostNodeInLine(SmNode* pSNode, bool MoveUpIfSelected = false);

    /** Draw the caret */
    void Draw(OutputDevice& pDev, Point Offset, bool isCaretVisible);

    bool IsAtTailOfBracket(SmBracketType eBracketType, SmBraceNode** ppBraceNode) const;
    void MoveAfterBracket(SmBraceNode* pBraceNode);

private:
    friend class SmDocShell;

    SmCaretPosGraphEntry    *mpAnchor,
                            *mpPosition;
    /** Formula tree */
    SmNode* mpTree;
    /** Owner of the formula tree */
    SmDocShell* mpDocShell;
    /** Graph over caret position in the current tree */
    std::unique_ptr<SmCaretPosGraph> mpGraph;
    /** Clipboard holder */
    SmClipboard maClipboard;

    /** Returns a node that is selected, if any could be found */
    SmNode* FindSelectedNode(SmNode* pNode);

    /** Is this one of the nodes used to compose a line
     *
     * These are SmExpression, SmBinHorNode, SmUnHorNode etc.
     */
    static bool IsLineCompositionNode(SmNode const * pNode);

    /** Count number of selected nodes, excluding line composition nodes
     *
     * Note this function doesn't count line composition nodes and it
     * does count all subnodes as well as the owner nodes.
     *
     * Used by SmCursor::HasComplexSelection()
     */
    int CountSelectedNodes(SmNode* pNode);

    /** Convert a visual line to a list
     *
     * Note this method will delete all the nodes that will no longer be needed.
     * that includes pLine!
     * This method also deletes SmErrorNode's as they're just meta info in the line.
     */
    static void LineToList(SmStructureNode* pLine, SmNodeList& rList);

    /** Auxiliary function for calling LineToList on a node
     *
     * This method sets pNode = NULL and remove it from its parent.
     * (Assuming it has a parent, and is a child of it).
     */
    static void NodeToList(SmNode*& rpNode, SmNodeList& rList){
        //Remove from parent and NULL rpNode
        SmNode* pNode = rpNode;
        if(rpNode && rpNode->GetParent()){    //Don't remove this, correctness relies on it
            int index = rpNode->GetParent()->IndexOfSubNode(rpNode);
            assert(index >= 0);
            rpNode->GetParent()->SetSubNode(index, nullptr);
        }
        rpNode = nullptr;
        //Create line from node
        if(pNode && IsLineCompositionNode(pNode)){
            LineToList(static_cast<SmStructureNode*>(pNode), rList);
            return;
        }
        if(pNode)
            rList.push_front(pNode);
    }

    /** Clone a visual line to a clipboard
     *
     * ... but the selected part only.
     * Doesn't clone SmErrorNodes, which are ignored as they are context dependent metadata.
     */
    static void CloneLineToClipboard(SmStructureNode* pLine, SmClipboard* pClipboard);

    /** Build pGraph over caret positions */
    void BuildGraph();

    /** Insert new nodes in the tree after position */
    void InsertNodes(std::unique_ptr<SmNodeList> pNewNodes);

    /** tries to set position to a specific SmCaretPos
     *
     * @returns false on failure to find the position in pGraph.
     */
    bool SetCaretPosition(SmCaretPos pos);

    /** Set selected on nodes of the tree */
    void AnnotateSelection();

    /** Clone list of nodes in a clipboard (creates a deep clone) */
    static std::unique_ptr<SmNodeList> CloneList(SmClipboard &rClipboard);

    /** Find an iterator pointing to the node in pLineList following rCaretPos
     *
     * If rCaretPos.pSelectedNode cannot be found it is assumed that it's in front of pLineList,
     * thus not an element in pLineList. In this case this method returns an iterator to the
     * first element in pLineList.
     *
     * If the current position is inside an SmTextNode, this node will be split in two, for this
     * reason you should beaware that iterators to elements in pLineList may be invalidated, and
     * that you should call PatchLineList() with this iterator if no action is taken.
     */
    static SmNodeList::iterator FindPositionInLineList(SmNodeList* pLineList,
                                                       const SmCaretPos& rCaretPos);

    /** Patch a line list after modification, merge SmTextNode, remove SmPlaceNode etc.
     *
     * @param pLineList The line list to patch
     * @param aIter     Iterator pointing to the element that needs to be patched with its previous.
     *
     * When the list is patched text nodes before and after aIter will be merged.
     * If there's an, in the context, inappropriate SmPlaceNode before or after aIter it will also be
     * removed.
     *
     * @returns A caret position equivalent to one selecting the node before aIter, the method returns
     *          an invalid SmCaretPos to indicate placement in front of the line.
     */
     static SmCaretPos PatchLineList(SmNodeList* pLineList, SmNodeList::iterator aIter);

    /** Take selected nodes from a list
     *
     * Puts the selected nodes into pSelectedNodes, or if pSelectedNodes is NULL deletes
     * the selected nodes.
     * Note: If there's a selection inside an SmTextNode this node will be split, and it
     * will not be merged when the selection have been taken. Use PatchLineList on the
     * iterator returns to fix this.
     *
     * @returns An iterator pointing to the element following the selection taken.
     */
    static SmNodeList::iterator TakeSelectedNodesFromList(SmNodeList *pLineList,
                                                         SmNodeList *pSelectedNodes = nullptr);

    /** Create an instance of SmMathSymbolNode usable for brackets */
    static SmNode *CreateBracket(SmBracketType eBracketType, bool bIsLeft);

    /** The number of times BeginEdit have been called
     * Used to allow nesting of BeginEdit() and EndEdit() sections
     */
    int mnEditSections;
    /** Holds data for BeginEdit() and EndEdit() */
    bool mbIsEnabledSetModifiedSmDocShell;
    /** Begin edit section where the tree will be modified */
    void BeginEdit();
    /** End edit section where the tree will be modified */
    void EndEdit();
    /** Finish editing
     *
     * Finishes editing by parsing pLineList and inserting back into pParent at nParentIndex.
     * This method also rebuilds the graph, annotates the selection, sets caret position and
     * Calls EndEdit.
     *
     * @remarks Please note that this method will delete pLineList, as the elements are taken.
     *
     * @param pLineList     List the constitutes the edited line.
     * @param pParent       Parent to which the line should be inserted.
     * @param nParentIndex  Index in parent where the line should be inserted.
     * @param PosAfterEdit  Caret position to look for after rebuilding graph.
     * @param pStartLine    Line to take first position in, if PosAfterEdit cannot be found,
     *                      leave it NULL for pLineList.
     */
    void FinishEdit(std::unique_ptr<SmNodeList> pLineList,
                    SmStructureNode* pParent,
                    int nParentIndex,
                    SmCaretPos PosAfterEdit,
                    SmNode* pStartLine = nullptr);
    /** Request the formula is repainted */
    void RequestRepaint();
};

/** Minimalistic recursive decent SmNodeList parser
 *
 * This parser is used to take a list of nodes that constitutes a line
 * and parse them to a tree of SmBinHorNode, SmUnHorNode and SmExpression.
 *
 * Please note, this will not handle all kinds of nodes, only nodes that
 * constitutes and entry in a line.
 *
 * Below is an EBNF representation of the grammar used for this parser:
 * \code
 * Expression   -> Relation*
 * Relation     -> Sum [(=|<|>|...) Sum]*
 * Sum          -> Product [(+|-) Product]*
 * Product      -> Factor [(*|/) Factor]*
 * Factor       -> [+|-|-+|...]* Factor | Postfix
 * Postfix      -> node [!]*
 * \endcode
 */
class SmNodeListParser{
public:
    /** Create an instance of SmNodeListParser */
    SmNodeListParser(){
        pList = nullptr;
    }
    /** Parse a list of nodes to an expression.
     *
     * Old error nodes will be deleted.
     */
    SmNode* Parse(SmNodeList* list);
    /** True, if the token is an operator */
    static bool IsOperator(const SmToken &token);
    /** True, if the token is a relation operator */
    static bool IsRelationOperator(const SmToken &token);
    /** True, if the token is a sum operator */
    static bool IsSumOperator(const SmToken &token);
    /** True, if the token is a product operator */
    static bool IsProductOperator(const SmToken &token);
    /** True, if the token is a unary operator */
    static bool IsUnaryOperator(const SmToken &token);
    /** True, if the token is a postfix operator */
    static bool IsPostfixOperator(const SmToken &token);
private:
    SmNodeList* pList;
    /** Get the current terminal */
    SmNode* Terminal(){
        if (!pList->empty())
            return pList->front();
        return nullptr;
    }
    /** Move to next terminal */
    SmNode* Next(){
        pList->pop_front();
        return Terminal();
    }
    /** Take the current terminal */
    SmNode* Take(){
        SmNode* pRetVal = Terminal();
        Next();
        return pRetVal;
    }
    SmNode* Expression();
    SmNode* Relation();
    SmNode* Sum();
    SmNode* Product();
    SmNode* Factor();
    SmNode* Postfix();
    static SmNode* Error();
};


#endif // INCLUDED_STARMATH_INC_CURSOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
