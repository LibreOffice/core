/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_STARMATH_INC_VISITORS_HXX
#define INCLUDED_STARMATH_INC_VISITORS_HXX

#include <sal/config.h>

#include <sal/log.hxx>

#include "node.hxx"
#include "caret.hxx"
#include <memory>

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

// SmDefaultingVisitor


/** Visitor that uses DefaultVisit for handling visits by default
 *
 * This abstract baseclass is useful for visitors where many methods share the same
 * implementation.
 */
class SmDefaultingVisitor : public SmVisitor
{
public:
    void Visit( SmTableNode* pNode ) override;
    void Visit( SmBraceNode* pNode ) override;
    void Visit( SmBracebodyNode* pNode ) override;
    void Visit( SmOperNode* pNode ) override;
    void Visit( SmAlignNode* pNode ) override;
    void Visit( SmAttributNode* pNode ) override;
    void Visit( SmFontNode* pNode ) override;
    void Visit( SmUnHorNode* pNode ) override;
    void Visit( SmBinHorNode* pNode ) override;
    void Visit( SmBinVerNode* pNode ) override;
    void Visit( SmBinDiagonalNode* pNode ) override;
    void Visit( SmSubSupNode* pNode ) override;
    void Visit( SmMatrixNode* pNode ) override;
    void Visit( SmPlaceNode* pNode ) override;
    void Visit( SmTextNode* pNode ) override;
    void Visit( SmSpecialNode* pNode ) override;
    void Visit( SmGlyphSpecialNode* pNode ) override;
    void Visit( SmMathSymbolNode* pNode ) override;
    void Visit( SmBlankNode* pNode ) override;
    void Visit( SmErrorNode* pNode ) override;
    void Visit( SmLineNode* pNode ) override;
    void Visit( SmExpressionNode* pNode ) override;
    void Visit( SmPolyLineNode* pNode ) override;
    void Visit( SmRootNode* pNode ) override;
    void Visit( SmRootSymbolNode* pNode ) override;
    void Visit( SmRectangleNode* pNode ) override;
    void Visit( SmVerticalBraceNode* pNode ) override;
protected:
    ~SmDefaultingVisitor() {}

    /** Method invoked by Visit methods by default */
    virtual void DefaultVisit( SmNode* pNode ) = 0;
};

// SmCaretDrawingVisitor

/** Visitor for drawing a caret position */
class SmCaretDrawingVisitor : public SmDefaultingVisitor
{
public:
    /** Given position and device this constructor will draw the caret */
    SmCaretDrawingVisitor( OutputDevice& rDevice, SmCaretPos position, Point offset, bool caretVisible );
    virtual ~SmCaretDrawingVisitor() {}
    void Visit( SmTextNode* pNode ) override;
    using SmDefaultingVisitor::Visit;
private:
    OutputDevice &mrDev;
    SmCaretPos maPos;
    /** Offset to draw from */
    Point maOffset;
    bool mbCaretVisible;
protected:
    /** Default method for drawing pNodes */
    void DefaultVisit( SmNode* pNode ) override;
};

// SmCaretPos2LineVisitor

/** Visitor getting a line from a caret position */
class SmCaretPos2LineVisitor : public SmDefaultingVisitor
{
public:
    /** Given position and device this constructor will compute a line for the caret */
    SmCaretPos2LineVisitor( OutputDevice *pDevice, SmCaretPos position )
        : mpDev( pDevice )
        , maPos( position )
    {
        SAL_WARN_IF( !position.IsValid(), "starmath", "Cannot draw invalid position!" );

        maPos.pSelectedNode->Accept( this );
    }
    virtual ~SmCaretPos2LineVisitor() {}
    void Visit( SmTextNode* pNode ) override;
    using SmDefaultingVisitor::Visit;
    const SmCaretLine& GetResult( ) {
        return maLine;
    }
private:
    SmCaretLine maLine;
    VclPtr<OutputDevice> mpDev;
    SmCaretPos maPos;
protected:
    /** Default method for computing lines for pNodes */
    void DefaultVisit( SmNode* pNode ) override;
};

// SmDrawingVisitor

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
        : mrDev( rDevice )
        , maPosition( position )
    {
        pTree->Accept( this );
    }
    virtual ~SmDrawingVisitor() {}
    void Visit( SmTableNode* pNode ) override;
    void Visit( SmBraceNode* pNode ) override;
    void Visit( SmBracebodyNode* pNode ) override;
    void Visit( SmOperNode* pNode ) override;
    void Visit( SmAlignNode* pNode ) override;
    void Visit( SmAttributNode* pNode ) override;
    void Visit( SmFontNode* pNode ) override;
    void Visit( SmUnHorNode* pNode ) override;
    void Visit( SmBinHorNode* pNode ) override;
    void Visit( SmBinVerNode* pNode ) override;
    void Visit( SmBinDiagonalNode* pNode ) override;
    void Visit( SmSubSupNode* pNode ) override;
    void Visit( SmMatrixNode* pNode ) override;
    void Visit( SmPlaceNode* pNode ) override;
    void Visit( SmTextNode* pNode ) override;
    void Visit( SmSpecialNode* pNode ) override;
    void Visit( SmGlyphSpecialNode* pNode ) override;
    void Visit( SmMathSymbolNode* pNode ) override;
    void Visit( SmBlankNode* pNode ) override;
    void Visit( SmErrorNode* pNode ) override;
    void Visit( SmLineNode* pNode ) override;
    void Visit( SmExpressionNode* pNode ) override;
    void Visit( SmPolyLineNode* pNode ) override;
    void Visit( SmRootNode* pNode ) override;
    void Visit( SmRootSymbolNode* pNode ) override;
    void Visit( SmRectangleNode* pNode ) override;
    void Visit( SmVerticalBraceNode* pNode ) override;
private:
    /** Draw the children of a pNode
     * This the default method, use by most pNodes
     */
    void DrawChildren( SmStructureNode* pNode );

    /** Draw an SmTextNode or a subclass of this */
    void DrawTextNode( SmTextNode* pNode );
    /** Draw an SmSpecialNode or a subclass of this  */
    void DrawSpecialNode( SmSpecialNode* pNode );
    /** OutputDevice to draw on */
    OutputDevice& mrDev;
    /** Position to draw on the mrDev
     * @remarks This variable is used to pass parameters in DrawChildren( ), this means
                that after a call to DrawChildren( ) the contents of this method is undefined
                so if needed cache it locally on the stack.
     */
    Point maPosition;
};

// SmSetSelectionVisitor

/** Set Selection Visitor
 * Sets the IsSelected( ) property on all SmNodes of the tree
 */
class SmSetSelectionVisitor : public SmDefaultingVisitor
{
public:
    SmSetSelectionVisitor( SmCaretPos startPos, SmCaretPos endPos, SmNode* pNode);
    virtual ~SmSetSelectionVisitor() {}
    void Visit( SmBinHorNode* pNode ) override;
    void Visit( SmUnHorNode* pNode ) override;
    void Visit( SmFontNode* pNode ) override;
    void Visit( SmTextNode* pNode ) override;
    void Visit( SmExpressionNode* pNode ) override;
    void Visit( SmLineNode* pNode ) override;
    void Visit( SmAlignNode* pNode ) override;
    using SmDefaultingVisitor::Visit;
    /** Set IsSelected on all pNodes of pSubTree */
    static void SetSelectedOnAll( SmNode* pSubTree, bool IsSelected = true );
private:
    /** Visit a selectable pNode
     * Can be used to handle pNodes that can be selected, that doesn't have more SmCaretPos'
     * than 0 and 1 inside them. SmTextNode should be handle separately!
     * Also note that pNodes such as SmBinVerNode cannot be selected, don't this method for
     * it.
     */
    void DefaultVisit( SmNode* pNode ) override;
    void VisitCompositionNode( SmStructureNode* pNode );
    /** Caret position where the selection starts */
    SmCaretPos maStartPos;
    /** Caret position where the selection ends */
    SmCaretPos maEndPos;
    /** The current state of this visitor
     * This property changes when the visitor meets either maStartPos
     * or maEndPos. This means that anything visited in between will be
     * selected.
     */
    bool mbSelecting;
};


// SmCaretPosGraphBuildingVisitor


/** A visitor for building a SmCaretPosGraph
 *
 * Visit invariant:
 * Each pNode, except SmExpressionNode, SmBinHorNode and a few others, constitutes an entry
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
    explicit SmCaretPosGraphBuildingVisitor( SmNode* pRootNode );
    virtual ~SmCaretPosGraphBuildingVisitor();
    void Visit( SmTableNode* pNode ) override;
    void Visit( SmBraceNode* pNode ) override;
    void Visit( SmBracebodyNode* pNode ) override;
    void Visit( SmOperNode* pNode ) override;
    void Visit( SmAlignNode* pNode ) override;
    void Visit( SmAttributNode* pNode ) override;
    void Visit( SmFontNode* pNode ) override;
    void Visit( SmUnHorNode* pNode ) override;
    void Visit( SmBinHorNode* pNode ) override;
    void Visit( SmBinVerNode* pNode ) override;
    void Visit( SmBinDiagonalNode* pNode ) override;
    void Visit( SmSubSupNode* pNode ) override;
    void Visit( SmMatrixNode* pNode ) override;
    void Visit( SmPlaceNode* pNode ) override;
    void Visit( SmTextNode* pNode ) override;
    void Visit( SmSpecialNode* pNode ) override;
    void Visit( SmGlyphSpecialNode* pNode ) override;
    void Visit( SmMathSymbolNode* pNode ) override;
    void Visit( SmBlankNode* pNode ) override;
    void Visit( SmErrorNode* pNode ) override;
    void Visit( SmLineNode* pNode ) override;
    void Visit( SmExpressionNode* pNode ) override;
    void Visit( SmPolyLineNode* pNode ) override;
    void Visit( SmRootNode* pNode ) override;
    void Visit( SmRootSymbolNode* pNode ) override;
    void Visit( SmRectangleNode* pNode ) override;
    void Visit( SmVerticalBraceNode* pNode ) override;
    SmCaretPosGraph* takeGraph()
    {
        return mpGraph.release();
    }
private:
    SmCaretPosGraphEntry* mpRightMost;
    std::unique_ptr<SmCaretPosGraph> mpGraph;
};

// SmCloningVisitor

/** Visitor for cloning a pNode
 *
 * This visitor creates deep clones.
 */
class SmCloningVisitor : public SmVisitor
{
public:
    SmCloningVisitor()
        : mpResult(nullptr)
    {}
    virtual ~SmCloningVisitor() {}
    void Visit( SmTableNode* pNode ) override;
    void Visit( SmBraceNode* pNode ) override;
    void Visit( SmBracebodyNode* pNode ) override;
    void Visit( SmOperNode* pNode ) override;
    void Visit( SmAlignNode* pNode ) override;
    void Visit( SmAttributNode* pNode ) override;
    void Visit( SmFontNode* pNode ) override;
    void Visit( SmUnHorNode* pNode ) override;
    void Visit( SmBinHorNode* pNode ) override;
    void Visit( SmBinVerNode* pNode ) override;
    void Visit( SmBinDiagonalNode* pNode ) override;
    void Visit( SmSubSupNode* pNode ) override;
    void Visit( SmMatrixNode* pNode ) override;
    void Visit( SmPlaceNode* pNode ) override;
    void Visit( SmTextNode* pNode ) override;
    void Visit( SmSpecialNode* pNode ) override;
    void Visit( SmGlyphSpecialNode* pNode ) override;
    void Visit( SmMathSymbolNode* pNode ) override;
    void Visit( SmBlankNode* pNode ) override;
    void Visit( SmErrorNode* pNode ) override;
    void Visit( SmLineNode* pNode ) override;
    void Visit( SmExpressionNode* pNode ) override;
    void Visit( SmPolyLineNode* pNode ) override;
    void Visit( SmRootNode* pNode ) override;
    void Visit( SmRootSymbolNode* pNode ) override;
    void Visit( SmRectangleNode* pNode ) override;
    void Visit( SmVerticalBraceNode* pNode ) override;
    /** Clone a pNode */
    SmNode* Clone( SmNode* pNode );
private:
    SmNode* mpResult;
    /** Clone children of pSource and give them to pTarget */
    void CloneKids( SmStructureNode* pSource, SmStructureNode* pTarget );
    /** Clone attributes on a pNode */
    static void CloneNodeAttr( SmNode* pSource, SmNode* pTarget );
};


// SmSelectionDrawingVisitor

class SmSelectionDrawingVisitor : public SmDefaultingVisitor
{
public:
    /** Draws a selection on rDevice for the selection on pTree */
    SmSelectionDrawingVisitor( OutputDevice& rDevice, SmNode* pTree, const Point& rOffset );
    virtual ~SmSelectionDrawingVisitor() {}
    void Visit( SmTextNode* pNode ) override;
    using SmDefaultingVisitor::Visit;
private:
    /** Reference to drawing device */
    OutputDevice& mrDev;
    /** True if  aSelectionArea have been initialized */
    bool mbHasSelectionArea;
    /** The current area that is selected */
    Rectangle maSelectionArea;
    /** Extend the area that must be selected  */
    void ExtendSelectionArea(const Rectangle& rArea);
    /** Default visiting method */
    void DefaultVisit( SmNode* pNode ) override;
    /** Visit the children of a given pNode */
    void VisitChildren( SmNode* pNode );
};

// SmNodeToTextVisitor

/** Extract command text from pNodes */
class SmNodeToTextVisitor : public SmVisitor
{
public:
    SmNodeToTextVisitor( SmNode* pNode, OUString &rText );
    virtual ~SmNodeToTextVisitor() {}

    void Visit( SmTableNode* pNode ) override;
    void Visit( SmBraceNode* pNode ) override;
    void Visit( SmBracebodyNode* pNode ) override;
    void Visit( SmOperNode* pNode ) override;
    void Visit( SmAlignNode* pNode ) override;
    void Visit( SmAttributNode* pNode ) override;
    void Visit( SmFontNode* pNode ) override;
    void Visit( SmUnHorNode* pNode ) override;
    void Visit( SmBinHorNode* pNode ) override;
    void Visit( SmBinVerNode* pNode ) override;
    void Visit( SmBinDiagonalNode* pNode ) override;
    void Visit( SmSubSupNode* pNode ) override;
    void Visit( SmMatrixNode* pNode ) override;
    void Visit( SmPlaceNode* pNode ) override;
    void Visit( SmTextNode* pNode ) override;
    void Visit( SmSpecialNode* pNode ) override;
    void Visit( SmGlyphSpecialNode* pNode ) override;
    void Visit( SmMathSymbolNode* pNode ) override;
    void Visit( SmBlankNode* pNode ) override;
    void Visit( SmErrorNode* pNode ) override;
    void Visit( SmLineNode* pNode ) override;
    void Visit( SmExpressionNode* pNode ) override;
    void Visit( SmPolyLineNode* pNode ) override;
    void Visit( SmRootNode* pNode ) override;
    void Visit( SmRootSymbolNode* pNode ) override;
    void Visit( SmRectangleNode* pNode ) override;
    void Visit( SmVerticalBraceNode* pNode ) override;
private:
    /** Extract text from a pNode that constitutes a line */
    void LineToText( SmNode* pNode ) {
        Separate( );
        if( pNode )
            pNode->Accept( this );
        Separate( );
    }
    void Append( const OUString &rText ) {
        maCmdText.append( rText );
    }
    /** Append a blank for separation, if needed */
    inline void Separate( ){
        if( maCmdText.isEmpty() || maCmdText[ maCmdText.getLength() - 1 ] != ' ' )
            maCmdText.append(' ');
    }
    /** Output text generated from the pNodes */
    OUStringBuffer maCmdText;
};

#endif // INCLUDED_STARMATH_INC_VISITORS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
