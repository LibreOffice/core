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
#ifndef SMVISITORS_H
#define SMVISITORS_H

#include "node.hxx"
#include "caret.hxx"

/** Base class for visitors that visits a tree of SmNodes
 * @remarks all methods have been left abstract to ensure that implementers
 * don't forget to implement one.
 */
class SmVisitor
{
public:
    virtual void Visit( SmTableNode* pNode ) = 0;
    virtual void Visit( SmBraceNode* pNode ) = 0;
    virtual void Visit( SmBracebodyNode* pNode ) = 0;
    virtual void Visit( SmOperNode* pNode ) = 0;
    virtual void Visit( SmAlignNode* pNode ) = 0;
    virtual void Visit( SmAttributNode* pNode ) = 0;
    virtual void Visit( SmFontNode* pNode ) = 0;
    virtual void Visit( SmUnHorNode* pNode ) = 0;
    virtual void Visit( SmBinHorNode* pNode ) = 0;
    virtual void Visit( SmBinVerNode* pNode ) = 0;
    virtual void Visit( SmBinDiagonalNode* pNode ) = 0;
    virtual void Visit( SmSubSupNode* pNode ) = 0;
    virtual void Visit( SmMatrixNode* pNode ) = 0;
    virtual void Visit( SmPlaceNode* pNode ) = 0;
    virtual void Visit( SmTextNode* pNode ) = 0;
    virtual void Visit( SmSpecialNode* pNode ) = 0;
    virtual void Visit( SmGlyphSpecialNode* pNode ) = 0;
    virtual void Visit( SmMathSymbolNode* pNode ) = 0;
    virtual void Visit( SmBlankNode* pNode ) = 0;
    virtual void Visit( SmErrorNode* pNode ) = 0;
    virtual void Visit( SmLineNode* pNode ) = 0;
    virtual void Visit( SmExpressionNode* pNode ) = 0;
    virtual void Visit( SmPolyLineNode* pNode ) = 0;
    virtual void Visit( SmRootNode* pNode ) = 0;
    virtual void Visit( SmRootSymbolNode* pNode ) = 0;
    virtual void Visit( SmRectangleNode* pNode ) = 0;
    virtual void Visit( SmVerticalBraceNode* pNode ) = 0;

protected:
    ~SmVisitor() {}
};

/** Simple visitor for testing SmVisitor */
class SmVisitorTest : public SmVisitor
{
public:
    virtual ~SmVisitorTest() {}
    void Visit( SmTableNode* pNode );
    void Visit( SmBraceNode* pNode );
    void Visit( SmBracebodyNode* pNode );
    void Visit( SmOperNode* pNode );
    void Visit( SmAlignNode* pNode );
    void Visit( SmAttributNode* pNode );
    void Visit( SmFontNode* pNode );
    void Visit( SmUnHorNode* pNode );
    void Visit( SmBinHorNode* pNode );
    void Visit( SmBinVerNode* pNode );
    void Visit( SmBinDiagonalNode* pNode );
    void Visit( SmSubSupNode* pNode );
    void Visit( SmMatrixNode* pNode );
    void Visit( SmPlaceNode* pNode );
    void Visit( SmTextNode* pNode );
    void Visit( SmSpecialNode* pNode );
    void Visit( SmGlyphSpecialNode* pNode );
    void Visit( SmMathSymbolNode* pNode );
    void Visit( SmBlankNode* pNode );
    void Visit( SmErrorNode* pNode );
    void Visit( SmLineNode* pNode );
    void Visit( SmExpressionNode* pNode );
    void Visit( SmPolyLineNode* pNode );
    void Visit( SmRootNode* pNode );
    void Visit( SmRootSymbolNode* pNode );
    void Visit( SmRectangleNode* pNode );
    void Visit( SmVerticalBraceNode* pNode );
private:
    /** Auxiliary method for visiting the children of a pNode */
    void VisitChildren( SmNode* pNode );
};

/////////////////////////////// SmDefaultingVisitor ////////////////////////////////


/** Visitor that uses DefaultVisit for handling visits by default
 *
 * This abstract baseclass is useful for visitors where many methods share the same
 * implementation.
 */
class SmDefaultingVisitor : public SmVisitor
{
public:
    void Visit( SmTableNode* pNode );
    void Visit( SmBraceNode* pNode );
    void Visit( SmBracebodyNode* pNode );
    void Visit( SmOperNode* pNode );
    void Visit( SmAlignNode* pNode );
    void Visit( SmAttributNode* pNode );
    void Visit( SmFontNode* pNode );
    void Visit( SmUnHorNode* pNode );
    void Visit( SmBinHorNode* pNode );
    void Visit( SmBinVerNode* pNode );
    void Visit( SmBinDiagonalNode* pNode );
    void Visit( SmSubSupNode* pNode );
    void Visit( SmMatrixNode* pNode );
    void Visit( SmPlaceNode* pNode );
    void Visit( SmTextNode* pNode );
    void Visit( SmSpecialNode* pNode );
    void Visit( SmGlyphSpecialNode* pNode );
    void Visit( SmMathSymbolNode* pNode );
    void Visit( SmBlankNode* pNode );
    void Visit( SmErrorNode* pNode );
    void Visit( SmLineNode* pNode );
    void Visit( SmExpressionNode* pNode );
    void Visit( SmPolyLineNode* pNode );
    void Visit( SmRootNode* pNode );
    void Visit( SmRootSymbolNode* pNode );
    void Visit( SmRectangleNode* pNode );
    void Visit( SmVerticalBraceNode* pNode );
protected:
    ~SmDefaultingVisitor() {}

    /** Method invoked by Visit methods by default */
    virtual void DefaultVisit( SmNode* pNode ) = 0;
};

/////////////////////////////// SmCaretDrawingVisitor ////////////////////////////////

/** Visitor for drawing a caret position */
class SmCaretDrawingVisitor : public SmDefaultingVisitor
{
public:
    /** Given position and device this constructor will draw the caret */
    SmCaretDrawingVisitor( OutputDevice& rDevice, SmCaretPos position, Point offset, bool caretVisible );
    virtual ~SmCaretDrawingVisitor() {}
    void Visit( SmTextNode* pNode );
    using SmDefaultingVisitor::Visit;
private:
    OutputDevice &rDev;
    SmCaretPos pos;
    /** Offset to draw from */
    Point Offset;
    bool isCaretVisible;
protected:
    /** Default method for drawing pNodes */
    void DefaultVisit( SmNode* pNode );
};

/////////////////////////////// SmCaretPos2LineVisitor ////////////////////////////////

/** Visitor getting a line from a caret position */
class SmCaretPos2LineVisitor : public SmDefaultingVisitor
{
public:
    /** Given position and device this constructor will compute a line for the caret */
    SmCaretPos2LineVisitor( OutputDevice *pDevice, SmCaretPos position ) {
        pDev = pDevice;
        pos = position;
        OSL_ENSURE( position.IsValid( ), "Cannot draw invalid position!" );

        pos.pSelectedNode->Accept( this );
    }
    virtual ~SmCaretPos2LineVisitor() {}
    void Visit( SmTextNode* pNode );
    using SmDefaultingVisitor::Visit;
    SmCaretLine GetResult( ){
        return line;
    }
private:
    SmCaretLine line;
    OutputDevice *pDev;
    SmCaretPos pos;
protected:
    /** Default method for computing lines for pNodes */
    void DefaultVisit( SmNode* pNode );
};

/////////////////////////////// SmDrawingVisitor ////////////////////////////////

/** Visitor for drawing SmNodes to OutputDevice */
class SmDrawingVisitor : public SmVisitor
{
public:
    /** Create an instance of SmDrawingVisitor, and use it to draw a formula
     * @param rDevice   Device to draw on
     * @param position  Offset on device to draw the formula
     * @param pTree     Formula tree to draw
     * @remarks This constructor will do the drawing, no need to anything more.
     */
    SmDrawingVisitor( OutputDevice &rDevice, Point position, SmNode* pTree )
        : rDev( rDevice ) {
        this->Position = position;
        pTree->Accept( this );
    }
    virtual ~SmDrawingVisitor() {}
    void Visit( SmTableNode* pNode );
    void Visit( SmBraceNode* pNode );
    void Visit( SmBracebodyNode* pNode );
    void Visit( SmOperNode* pNode );
    void Visit( SmAlignNode* pNode );
    void Visit( SmAttributNode* pNode );
    void Visit( SmFontNode* pNode );
    void Visit( SmUnHorNode* pNode );
    void Visit( SmBinHorNode* pNode );
    void Visit( SmBinVerNode* pNode );
    void Visit( SmBinDiagonalNode* pNode );
    void Visit( SmSubSupNode* pNode );
    void Visit( SmMatrixNode* pNode );
    void Visit( SmPlaceNode* pNode );
    void Visit( SmTextNode* pNode );
    void Visit( SmSpecialNode* pNode );
    void Visit( SmGlyphSpecialNode* pNode );
    void Visit( SmMathSymbolNode* pNode );
    void Visit( SmBlankNode* pNode );
    void Visit( SmErrorNode* pNode );
    void Visit( SmLineNode* pNode );
    void Visit( SmExpressionNode* pNode );
    void Visit( SmPolyLineNode* pNode );
    void Visit( SmRootNode* pNode );
    void Visit( SmRootSymbolNode* pNode );
    void Visit( SmRectangleNode* pNode );
    void Visit( SmVerticalBraceNode* pNode );
private:
    /** Draw the children of a pNode
     * This the default method, use by most pNodes
     */
    void DrawChildren( SmNode* pNode );

    /** Draw an SmTextNode or a subclass of this */
    void DrawTextNode( SmTextNode* pNode );
    /** Draw an SmSpecialNode or a subclass of this  */
    void DrawSpecialNode( SmSpecialNode* pNode );
    /** OutputDevice to draw on */
    OutputDevice& rDev;
    /** Position to draw on the rDev
     * @remarks This variable is used to pass parameters in DrawChildren( ), this means
                that after a call to DrawChildren( ) the contents of this method is undefined
                so if needed cache it locally on the stack.
     */
    Point Position;
};

/////////////////////////////// SmSetSelectionVisitor ////////////////////////////////

/** Set Selection Visitor
 * Sets the IsSelected( ) property on all SmNodes of the tree
 */
class SmSetSelectionVisitor : public SmDefaultingVisitor
{
public:
    SmSetSelectionVisitor( SmCaretPos startPos, SmCaretPos endPos, SmNode* pNode);
    virtual ~SmSetSelectionVisitor() {}
    void Visit( SmBinHorNode* pNode );
    void Visit( SmUnHorNode* pNode );
    void Visit( SmFontNode* pNode );
    void Visit( SmTextNode* pNode );
    void Visit( SmExpressionNode* pNode );
    void Visit( SmLineNode* pNode );
    void Visit( SmAlignNode* pNode );
    using SmDefaultingVisitor::Visit;
    /** Set IsSelected on all pNodes of pSubTree */
    static void SetSelectedOnAll( SmNode* pSubTree, bool IsSelected = true );
private:
    /** Visit a selectable pNode
     * Can be used to handle pNodes that can be selected, that doesn't have more SmCaretPos'
     * than 0 and 1 inside them. SmTextNode should be handle seperately!
     * Also note that pNodes such as SmBinVerNode cannot be selected, don't this method for
     * it.
     */
    void DefaultVisit( SmNode* pNode );
    void VisitCompositionNode( SmNode* pNode );
    /** Caret position where the selection starts */
    SmCaretPos  StartPos;
    /** Caret position where the selection ends */
    SmCaretPos  EndPos;
    /** The current state of this visitor
     * This property changes when the visitor meets either StartPos
     * or EndPos. This means that anything visited in between will be
     * selected.
     */
    bool IsSelecting;
};


/////////////////////////////// SmCaretPosGraphBuildingVisitor ////////////////////////////////


/** A visitor for building a SmCaretPosGraph
 *
 * Visit invariant:
 * Each pNode, except SmExpressionNode, SmBinHorNode and a few others, constitues an entry
 * in a line. Consider the line entry "H", this entry creates one carat position, here
 * denoted by | in "H|".
 *
 * Parameter variables:
 *  The following variables are used to transfer parameters in to calls and results out
 *  of calls.
 *      pRightMost : SmCaretPosGraphEntry*
 *
 * Prior to a Visit call:
 *  pRightMost: A pointer to right most position in front of the current line entry.
 *
 * After a Visit call:
 *  pRightMost: A pointer to the right most position in the called line entry, if no there's
 *              no caret positions in called line entry don't change this variable.
 */
class SmCaretPosGraphBuildingVisitor : public SmVisitor
{
public:
    /** Builds a caret position graph for pRootNode */
    SmCaretPosGraphBuildingVisitor( SmNode* pRootNode );
    virtual ~SmCaretPosGraphBuildingVisitor() {}
    void Visit( SmTableNode* pNode );
    void Visit( SmBraceNode* pNode );
    void Visit( SmBracebodyNode* pNode );
    void Visit( SmOperNode* pNode );
    void Visit( SmAlignNode* pNode );
    void Visit( SmAttributNode* pNode );
    void Visit( SmFontNode* pNode );
    void Visit( SmUnHorNode* pNode );
    void Visit( SmBinHorNode* pNode );
    void Visit( SmBinVerNode* pNode );
    void Visit( SmBinDiagonalNode* pNode );
    void Visit( SmSubSupNode* pNode );
    void Visit( SmMatrixNode* pNode );
    void Visit( SmPlaceNode* pNode );
    void Visit( SmTextNode* pNode );
    void Visit( SmSpecialNode* pNode );
    void Visit( SmGlyphSpecialNode* pNode );
    void Visit( SmMathSymbolNode* pNode );
    void Visit( SmBlankNode* pNode );
    void Visit( SmErrorNode* pNode );
    void Visit( SmLineNode* pNode );
    void Visit( SmExpressionNode* pNode );
    void Visit( SmPolyLineNode* pNode );
    void Visit( SmRootNode* pNode );
    void Visit( SmRootSymbolNode* pNode );
    void Visit( SmRectangleNode* pNode );
    void Visit( SmVerticalBraceNode* pNode );
    SmCaretPosGraph* Graph( ){
        return pGraph;
    }
private:
    SmCaretPosGraphEntry* pRightMost;
    SmCaretPosGraph*      pGraph;
};

/////////////////////////////// SmCloningVisitor ///////////////////////////////

/** Visitor for cloning a pNode
 *
 * This visitor creates deep clones.
 */
class SmCloningVisitor : public SmVisitor
{
public:
    SmCloningVisitor( ){ pResult = NULL; }
    virtual ~SmCloningVisitor() {}
    void Visit( SmTableNode* pNode );
    void Visit( SmBraceNode* pNode );
    void Visit( SmBracebodyNode* pNode );
    void Visit( SmOperNode* pNode );
    void Visit( SmAlignNode* pNode );
    void Visit( SmAttributNode* pNode );
    void Visit( SmFontNode* pNode );
    void Visit( SmUnHorNode* pNode );
    void Visit( SmBinHorNode* pNode );
    void Visit( SmBinVerNode* pNode );
    void Visit( SmBinDiagonalNode* pNode );
    void Visit( SmSubSupNode* pNode );
    void Visit( SmMatrixNode* pNode );
    void Visit( SmPlaceNode* pNode );
    void Visit( SmTextNode* pNode );
    void Visit( SmSpecialNode* pNode );
    void Visit( SmGlyphSpecialNode* pNode );
    void Visit( SmMathSymbolNode* pNode );
    void Visit( SmBlankNode* pNode );
    void Visit( SmErrorNode* pNode );
    void Visit( SmLineNode* pNode );
    void Visit( SmExpressionNode* pNode );
    void Visit( SmPolyLineNode* pNode );
    void Visit( SmRootNode* pNode );
    void Visit( SmRootSymbolNode* pNode );
    void Visit( SmRectangleNode* pNode );
    void Visit( SmVerticalBraceNode* pNode );
    /** Clone a pNode */
    SmNode* Clone( SmNode* pNode );
private:
    SmNode* pResult;
    /** Clone children of pSource and give them to pTarget */
    void CloneKids( SmStructureNode* pSource, SmStructureNode* pTarget );
    /** Clone attributes on a pNode */
    void CloneNodeAttr( SmNode* pSource, SmNode* pTarget );
};


/////////////////////////////// SmSelectionDrawingVisitor ///////////////////////////////

class SmSelectionDrawingVisitor : public SmDefaultingVisitor
{
public:
    /** Draws a selection on rDevice for the selection on pTree */
    SmSelectionDrawingVisitor( OutputDevice& rDevice, SmNode* pTree, Point Offset );
    virtual ~SmSelectionDrawingVisitor() {}
    void Visit( SmTextNode* pNode );
    using SmDefaultingVisitor::Visit;
private:
    /** Reference to drawing device */
    OutputDevice& rDev;
    /** True if  aSelectionArea have been initialized */
    bool bHasSelectionArea;
    /** The current area that is selected */
    Rectangle aSelectionArea;
    /** Extend the area that must be selected  */
    void ExtendSelectionArea( Rectangle aArea );
    /** Default visiting method */
    void DefaultVisit( SmNode* pNode );
    /** Visit the children of a given pNode */
    void VisitChildren( SmNode* pNode );
};

/////////////////////////////// SmNodeToTextVisitor ///////////////////////////////

/** Extract command text from pNodes */
class SmNodeToTextVisitor : public SmVisitor
{
public:
    SmNodeToTextVisitor( SmNode* pNode, OUString &rText );
    virtual ~SmNodeToTextVisitor() {}

    void Visit( SmTableNode* pNode );
    void Visit( SmBraceNode* pNode );
    void Visit( SmBracebodyNode* pNode );
    void Visit( SmOperNode* pNode );
    void Visit( SmAlignNode* pNode );
    void Visit( SmAttributNode* pNode );
    void Visit( SmFontNode* pNode );
    void Visit( SmUnHorNode* pNode );
    void Visit( SmBinHorNode* pNode );
    void Visit( SmBinVerNode* pNode );
    void Visit( SmBinDiagonalNode* pNode );
    void Visit( SmSubSupNode* pNode );
    void Visit( SmMatrixNode* pNode );
    void Visit( SmPlaceNode* pNode );
    void Visit( SmTextNode* pNode );
    void Visit( SmSpecialNode* pNode );
    void Visit( SmGlyphSpecialNode* pNode );
    void Visit( SmMathSymbolNode* pNode );
    void Visit( SmBlankNode* pNode );
    void Visit( SmErrorNode* pNode );
    void Visit( SmLineNode* pNode );
    void Visit( SmExpressionNode* pNode );
    void Visit( SmPolyLineNode* pNode );
    void Visit( SmRootNode* pNode );
    void Visit( SmRootSymbolNode* pNode );
    void Visit( SmRectangleNode* pNode );
    void Visit( SmVerticalBraceNode* pNode );
private:
    /** Extract text from a pNode that constitues a line */
    void LineToText( SmNode* pNode ) {
        Separate( );
        if( pNode )
            pNode->Accept( this );
        Separate( );
    }
    void Append( const OUString &rText ) {
        aCmdText.append( rText );
    }
    /** Append a blank for separation, if needed */
    inline void Separate( ){
        if( !aCmdText.getLength() || aCmdText[ aCmdText.getLength() - 1 ] != ' ' )
            aCmdText.append(' ');
    }
    /** Output text generated from the pNodes */
    OUStringBuffer aCmdText;
};

#endif /* SMVISITORS_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
