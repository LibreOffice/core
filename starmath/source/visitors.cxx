/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/gen.hxx>
#include <vcl/lineinfo.hxx>
#include "visitors.hxx"
#include "tmpdevice.hxx"
#include "cursor.hxx"
#include <cassert>

// SmDefaultingVisitor

void SmDefaultingVisitor::Visit( SmTableNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmBraceNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmBracebodyNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmOperNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmAlignNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmAttributNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmFontNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmUnHorNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmBinHorNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmBinVerNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmBinDiagonalNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmSubSupNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmMatrixNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmPlaceNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmTextNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmSpecialNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmGlyphSpecialNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmMathSymbolNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmBlankNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmErrorNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmLineNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmExpressionNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmPolyLineNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmRootNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmRootSymbolNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmDynIntegralNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmDynIntegralSymbolNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmRectangleNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmVerticalBraceNode* pNode )
{
    DefaultVisit( pNode );
}

// SmCaretDrawingVisitor

SmCaretDrawingVisitor::SmCaretDrawingVisitor( OutputDevice& rDevice,
                                             SmCaretPos position,
                                             Point offset,
                                             bool caretVisible )
    : mrDev( rDevice )
    , maPos( position )
    , maOffset( offset )
    , mbCaretVisible( caretVisible )
{
    SAL_WARN_IF( !position.IsValid(), "starmath", "Cannot draw invalid position!" );
    if( !position.IsValid( ) )
        return;

    //Save device state
    mrDev.Push( PushFlags::FONT | PushFlags::MAPMODE | PushFlags::LINECOLOR | PushFlags::FILLCOLOR | PushFlags::TEXTCOLOR );

    maPos.pSelectedNode->Accept( this );
    //Restore device state
    mrDev.Pop( );
}

void SmCaretDrawingVisitor::Visit( SmTextNode* pNode )
{
    long i = maPos.Index;

    mrDev.SetFont( pNode->GetFont( ) );

    //Find the line
    SmNode* pLine = SmCursor::FindTopMostNodeInLine( pNode );

    //Find coordinates
    long left = pNode->GetLeft( ) + mrDev.GetTextWidth( pNode->GetText( ), 0, i ) + maOffset.X( );
    long top = pLine->GetTop( ) + maOffset.Y( );
    long height = pLine->GetHeight( );
    long left_line = pLine->GetLeft( ) + maOffset.X( );
    long right_line = pLine->GetRight( ) + maOffset.X( );

    //Set color
    mrDev.SetLineColor( Color( COL_BLACK ) );

    if ( mbCaretVisible ) {
        //Draw vertical line
        Point p1( left, top );
        Point p2( left, top + height );
        mrDev.DrawLine( p1, p2 );
    }

    //Underline the line
    Point aLeft( left_line, top + height );
    Point aRight( right_line, top + height );
    mrDev.DrawLine( aLeft, aRight );
}

void SmCaretDrawingVisitor::DefaultVisit( SmNode* pNode )
{
    //Find the line
    SmNode* pLine = SmCursor::FindTopMostNodeInLine( pNode );

    //Find coordinates
    long left = pNode->GetLeft( ) + maOffset.X( ) + ( maPos.Index == 1 ? pNode->GetWidth( ) : 0 );
    long top = pLine->GetTop( ) + maOffset.Y( );
    long height = pLine->GetHeight( );
    long left_line = pLine->GetLeft( ) + maOffset.X( );
    long right_line = pLine->GetRight( ) + maOffset.X( );

    //Set color
    mrDev.SetLineColor( Color( COL_BLACK ) );

    if ( mbCaretVisible ) {
        //Draw vertical line
        Point p1( left, top );
        Point p2( left, top + height );
        mrDev.DrawLine( p1, p2 );
    }

    //Underline the line
    Point aLeft( left_line, top + height );
    Point aRight( right_line, top + height );
    mrDev.DrawLine( aLeft, aRight );
}

// SmCaretPos2LineVisitor

void SmCaretPos2LineVisitor::Visit( SmTextNode* pNode )
{
    //Save device state
    mpDev->Push( PushFlags::FONT | PushFlags::TEXTCOLOR );

    long i = maPos.Index;

    mpDev->SetFont( pNode->GetFont( ) );

    //Find coordinates
    long left = pNode->GetLeft( ) + mpDev->GetTextWidth( pNode->GetText( ), 0, i );
    long top = pNode->GetTop( );
    long height = pNode->GetHeight( );

    maLine = SmCaretLine( left, top, height );

    //Restore device state
    mpDev->Pop( );
}

void SmCaretPos2LineVisitor::DefaultVisit( SmNode* pNode )
{
    //Vertical line ( code from SmCaretDrawingVisitor )
    Point p1 = pNode->GetTopLeft( );
    if( maPos.Index == 1 )
        p1.Move( pNode->GetWidth( ), 0 );

    maLine = SmCaretLine( p1.X( ), p1.Y( ), pNode->GetHeight( ) );
}


// SmDrawingVisitor

void SmDrawingVisitor::Visit( SmTableNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmBraceNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmBracebodyNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmOperNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmAlignNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmAttributNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmFontNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmUnHorNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmBinHorNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmBinVerNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmBinDiagonalNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmSubSupNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmMatrixNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmPlaceNode* pNode )
{
    DrawSpecialNode( pNode );
}

void SmDrawingVisitor::Visit( SmTextNode* pNode )
{
    DrawTextNode( pNode );
}

void SmDrawingVisitor::Visit( SmSpecialNode* pNode )
{
    DrawSpecialNode( pNode );
}

void SmDrawingVisitor::Visit( SmGlyphSpecialNode* pNode )
{
    DrawSpecialNode( pNode );
}

void SmDrawingVisitor::Visit( SmMathSymbolNode* pNode )
{
    DrawSpecialNode( pNode );
}

void SmDrawingVisitor::Visit( SmBlankNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmErrorNode* pNode )
{
    DrawSpecialNode( pNode );
}

void SmDrawingVisitor::Visit( SmLineNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmExpressionNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmRootNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit(SmDynIntegralNode* pNode)
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmVerticalBraceNode* pNode )
{
    DrawChildren( pNode );
}

void SmDrawingVisitor::Visit( SmRootSymbolNode* pNode )
{
    if ( pNode->IsPhantom( ) )
        return;

    // draw root-sign itself
    DrawSpecialNode( pNode );

    SmTmpDevice aTmpDev( mrDev, true );
    aTmpDev.SetFillColor( pNode->GetFont( ).GetColor( ) );
    mrDev.SetLineColor( );
    aTmpDev.SetFont( pNode->GetFont( ) );

    // since the width is always unscaled it corresponds ot the _original_
    // _unscaled_ font height to be used, we use that to calculate the
    // bar height. Thus it is independent of the arguments height.
    // ( see display of sqrt QQQ versus sqrt stack{Q#Q#Q#Q} )
    long nBarHeight = pNode->GetWidth( ) * 7L / 100L;
    long nBarWidth = pNode->GetBodyWidth( ) + pNode->GetBorderWidth( );
    Point aBarOffset( pNode->GetWidth( ), +pNode->GetBorderWidth( ) );
    Point aBarPos( maPosition + aBarOffset );

    Rectangle  aBar( aBarPos, Size( nBarWidth, nBarHeight ) );
    //! avoid GROWING AND SHRINKING of drawn rectangle when constantly
    //! increasing zoomfactor.
    //  This is done by shifting its output-position to a point that
    //  corresponds exactly to a pixel on the output device.
    Point  aDrawPos( mrDev.PixelToLogic( mrDev.LogicToPixel( aBar.TopLeft( ) ) ) );
    aBar.SetPos( aDrawPos );

    mrDev.DrawRect( aBar );
}

void SmDrawingVisitor::Visit( SmDynIntegralSymbolNode* pNode )
{
    if ( pNode->IsPhantom( ) )
        return;

    // draw integral-sign itself
    DrawSpecialNode( pNode );

    //! the rest of this may not be needed at all

    // this should be something like:
    // instead of just drawing the node, take some information about the body.
    // This is also how SmRootSymbol does it (probably by means of SmRootNode)
    // NEXT: Check out SmRootNode
}

void SmDrawingVisitor::Visit( SmPolyLineNode* pNode )
{
    if ( pNode->IsPhantom( ) )
        return;

    long nBorderwidth = pNode->GetFont( ).GetBorderWidth( );

    LineInfo  aInfo;
    aInfo.SetWidth( pNode->GetWidth( ) - 2 * nBorderwidth );

    Point aOffset ( Point( ) - pNode->GetPolygon( ).GetBoundRect( ).TopLeft( )
                   + Point( nBorderwidth, nBorderwidth ) ),
          aPos ( maPosition + aOffset );
    pNode->GetPolygon( ).Move( aPos.X( ), aPos.Y( ) );    //Works because Polygon wraps a pointer

    SmTmpDevice aTmpDev ( mrDev, false );
    aTmpDev.SetLineColor( pNode->GetFont( ).GetColor( ) );

    mrDev.DrawPolyLine( pNode->GetPolygon( ), aInfo );
}

void SmDrawingVisitor::Visit( SmRectangleNode* pNode )
{
    if ( pNode->IsPhantom( ) )
        return;

    SmTmpDevice aTmpDev ( mrDev, false );
    aTmpDev.SetFillColor( pNode->GetFont( ).GetColor( ) );
    mrDev.SetLineColor( );
    aTmpDev.SetFont( pNode->GetFont( ) );

    sal_uLong  nTmpBorderWidth = pNode->GetFont( ).GetBorderWidth( );

    // get rectangle and remove borderspace
    Rectangle  aTmp ( pNode->AsRectangle( ) + maPosition - pNode->GetTopLeft( ) );
    aTmp.Left( )   += nTmpBorderWidth;
    aTmp.Right( )  -= nTmpBorderWidth;
    aTmp.Top( )    += nTmpBorderWidth;
    aTmp.Bottom( ) -= nTmpBorderWidth;

    SAL_WARN_IF( aTmp.GetHeight() == 0 || aTmp.GetWidth() == 0,
                "starmath", "Empty rectangle" );

    //! avoid GROWING AND SHRINKING of drawn rectangle when constantly
    //! increasing zoomfactor.
    //  This is done by shifting its output-position to a point that
    //  corresponds exactly to a pixel on the output device.
    Point  aPos ( mrDev.PixelToLogic( mrDev.LogicToPixel( aTmp.TopLeft( ) ) ) );
    aTmp.SetPos( aPos );

    mrDev.DrawRect( aTmp );
}

void SmDrawingVisitor::DrawTextNode( SmTextNode* pNode )
{
    if ( pNode->IsPhantom() || pNode->GetText().isEmpty() || pNode->GetText()[0] == '\0' )
        return;

    SmTmpDevice aTmpDev ( mrDev, false );
    aTmpDev.SetFont( pNode->GetFont( ) );

    Point  aPos ( maPosition );
    aPos.Y( ) += pNode->GetBaselineOffset( );
    // auf Pixelkoordinaten runden
    aPos = mrDev.PixelToLogic( mrDev.LogicToPixel( aPos ) );

    mrDev.DrawStretchText( aPos, pNode->GetWidth( ), pNode->GetText( ) );
}

void SmDrawingVisitor::DrawSpecialNode( SmSpecialNode* pNode )
{
    //! since this chars might come from any font, that we may not have
    //! set to ALIGN_BASELINE yet, we do it now.
    pNode->GetFont( ).SetAlignment( ALIGN_BASELINE );

    DrawTextNode( pNode );
}

void SmDrawingVisitor::DrawChildren( SmNode* pNode )
{
    if ( pNode->IsPhantom( ) )
        return;

    Point rPosition = maPosition;

    SmNodeIterator it( pNode );
    while( it.Next( ) )
    {
        Point  aOffset ( it->GetTopLeft( ) - pNode->GetTopLeft( ) );
        maPosition = rPosition + aOffset;
        it->Accept( this );
    }
}

// SmSetSelectionVisitor

SmSetSelectionVisitor::SmSetSelectionVisitor( SmCaretPos startPos, SmCaretPos endPos, SmNode* pTree)
    : maStartPos(startPos)
    , maEndPos(endPos)
    , mbSelecting(false)
{
    //Assume that pTree is a SmTableNode
    SAL_WARN_IF(pTree->GetType() != NTABLE, "starmath", "pTree should be a SmTableNode!");
    //Visit root node, this is special as this node cannot be selected, but its children can!
    if(pTree->GetType() == NTABLE){
        //Change state if maStartPos is in front of this node
        if( maStartPos.pSelectedNode == pTree && maStartPos.Index == 0 )
            mbSelecting = !mbSelecting;
        //Change state if maEndPos is in front of this node
        if( maEndPos.pSelectedNode == pTree && maEndPos.Index == 0 )
            mbSelecting = !mbSelecting;
        SAL_WARN_IF(mbSelecting, "starmath", "Caret positions needed to set mbSelecting about, shouldn't be possible!");

        //Visit lines
        SmNodeIterator it( pTree );
        while( it.Next( ) ) {
            it->Accept( this );
            //If we started a selection in this line and it haven't ended, we do that now!
            if(mbSelecting) {
                mbSelecting = false;
                SetSelectedOnAll(it.Current());
                //Set maStartPos and maEndPos to invalid positions, this ensures that an unused
                //start or end (because we forced end above), doesn't start a new selection.
                maStartPos = maEndPos = SmCaretPos();
            }
        }
        //Check if pTree isn't selected
        SAL_WARN_IF(pTree->IsSelected(), "starmath", "pTree should never be selected!");
        //Discard the selection if there's a bug (it's better than crashing)
        if(pTree->IsSelected())
            SetSelectedOnAll(pTree, false);
    }else //This shouldn't happen, but I don't see any reason to die if it does
        pTree->Accept(this);
}

void SmSetSelectionVisitor::SetSelectedOnAll( SmNode* pSubTree, bool IsSelected ) {
    pSubTree->SetSelected( IsSelected );

    //Quick BFS to set all selections
    SmNodeIterator it( pSubTree );
    while( it.Next( ) )
        SetSelectedOnAll( it.Current( ), IsSelected );
}

void SmSetSelectionVisitor::DefaultVisit( SmNode* pNode ) {
    //Change state if maStartPos is in front of this node
    if( maStartPos.pSelectedNode == pNode && maStartPos.Index == 0 )
        mbSelecting = !mbSelecting;
    //Change state if maEndPos is in front of this node
    if( maEndPos.pSelectedNode == pNode && maEndPos.Index == 0 )
        mbSelecting = !mbSelecting;

    //Cache current state
    bool WasSelecting = mbSelecting;
    bool ChangedState = false;

    //Set selected
    pNode->SetSelected( mbSelecting );

    //Visit children
    SmNodeIterator it( pNode );
    while( it.Next( ) )
    {
        it->Accept( this );
        ChangedState = ( WasSelecting != mbSelecting ) || ChangedState;
    }

    //If state changed
    if( ChangedState )
    {
        //Select this node and all of its children
        //(Make exception for SmBracebodyNode)
        if( pNode->GetType() != NBRACEBODY ||
            !pNode->GetParent() ||
            pNode->GetParent()->GetType() != NBRACE )
            SetSelectedOnAll( pNode );
        else
            SetSelectedOnAll( pNode->GetParent() );
        /* If the equation is:      sqrt{2 + 4} + 5
         * And the selection is:    sqrt{2 + [4} +] 5
         *      Where [ denotes maStartPos and ] denotes maEndPos
         * Then the sqrt node should be selected, so that the
         * effective selection is:  [sqrt{2 + 4} +] 5
         * The same is the case if we swap maStartPos and maEndPos.
         */
    }

    //Change state if maStartPos is after this node
    if( maStartPos.pSelectedNode == pNode && maStartPos.Index == 1 )
    {
        mbSelecting = !mbSelecting;
    }
    //Change state if maEndPos is after of this node
    if( maEndPos.pSelectedNode == pNode && maEndPos.Index == 1 )
    {
        mbSelecting = !mbSelecting;
    }
}

void SmSetSelectionVisitor::VisitCompositionNode( SmNode* pNode ) {
    //Change state if maStartPos is in front of this node
    if( maStartPos.pSelectedNode == pNode && maStartPos.Index == 0 )
        mbSelecting = !mbSelecting;
    //Change state if maEndPos is in front of this node
    if( maEndPos.pSelectedNode == pNode && maEndPos.Index == 0 )
        mbSelecting = !mbSelecting;

    //Cache current state
    bool WasSelecting = mbSelecting;

    //Visit children
    SmNodeIterator it( pNode );
    while( it.Next( ) )
        it->Accept( this );

    //Set selected, if everything was selected
    pNode->SetSelected( WasSelecting && mbSelecting );

    //Change state if maStartPos is after this node
    if( maStartPos.pSelectedNode == pNode && maStartPos.Index == 1 )
        mbSelecting = !mbSelecting;
    //Change state if maEndPos is after of this node
    if( maEndPos.pSelectedNode == pNode && maEndPos.Index == 1 )
        mbSelecting = !mbSelecting;
}

void SmSetSelectionVisitor::Visit( SmTextNode* pNode ) {
    long    i1 = -1,
            i2 = -1;
    if( maStartPos.pSelectedNode == pNode )
        i1 = maStartPos.Index;
    if( maEndPos.pSelectedNode == pNode )
        i2 = maEndPos.Index;

    long start, end;
    pNode->SetSelected(true);
    if( i1 != -1 && i2 != -1 ) {
        start = i1 < i2 ? i1 : i2; //MIN
        end   = i1 > i2 ? i1 : i2; //MAX
    } else if( mbSelecting && i1 != -1 ) {
        start = 0;
        end = i1;
        mbSelecting = false;
    } else if( mbSelecting && i2 != -1 ) {
        start = 0;
        end = i2;
        mbSelecting = false;
    } else if( !mbSelecting && i1 != -1 ) {
        start = i1;
        end = pNode->GetText().getLength();
        mbSelecting = true;
    } else if( !mbSelecting && i2 != -1 ) {
        start = i2;
        end = pNode->GetText().getLength();
        mbSelecting = true;
    } else if( mbSelecting ) {
        start = 0;
        end = pNode->GetText().getLength();
    } else {
        pNode->SetSelected( false );
        start = 0;
        end = 0;
    }
    pNode->SetSelected( start != end );
    pNode->SetSelectionStart( start );
    pNode->SetSelectionEnd( end );
}

void SmSetSelectionVisitor::Visit( SmExpressionNode* pNode ) {
    VisitCompositionNode( pNode );
}

void SmSetSelectionVisitor::Visit( SmLineNode* pNode ) {
    VisitCompositionNode( pNode );
}

void SmSetSelectionVisitor::Visit( SmAlignNode* pNode ) {
    VisitCompositionNode( pNode );
}

void SmSetSelectionVisitor::Visit( SmBinHorNode* pNode ) {
    VisitCompositionNode( pNode );
}

void SmSetSelectionVisitor::Visit( SmUnHorNode* pNode ) {
    VisitCompositionNode( pNode );
}

void SmSetSelectionVisitor::Visit( SmFontNode* pNode ) {
    VisitCompositionNode( pNode );
}

// SmCaretPosGraphBuildingVisitor

SmCaretPosGraphBuildingVisitor::SmCaretPosGraphBuildingVisitor( SmNode* pRootNode )
    : mpRightMost(nullptr)
    , mpGraph(new SmCaretPosGraph)
{
    //pRootNode should always be a table
    SAL_WARN_IF( pRootNode->GetType( ) != NTABLE, "starmath", "pRootNode must be a table node");
    //Handle the special case where NTABLE is used a rootnode
    if( pRootNode->GetType( ) == NTABLE ){
        //Children are SmLineNodes
        //Or so I thought... Apparently, the children can be instances of SmExpression
        //especially if there's a error in the formula... So he we go, a simple work around.
        SmNodeIterator it( pRootNode );
        while( it.Next( ) ){
            //There's a special invariant between this method and the Visit( SmLineNode* )
            //Usually mpRightMost may not be NULL, to avoid this mpRightMost should here be
            //set to a new SmCaretPos in front of it.Current( ), however, if it.Current( ) is
            //an instance of SmLineNode we let SmLineNode create this position in front of
            //the visual line.
            //The argument for doing this is that we now don't have to worry about SmLineNode
            //being a visual line composition node. Thus, no need for yet another special case
            //in SmCursor::IsLineCompositionNode and everywhere this method is used.
            //if( it->GetType( ) != NLINE )
                mpRightMost = mpGraph->Add( SmCaretPos( it.Current( ), 0 ) );
            it->Accept( this );
        }
    }else
        pRootNode->Accept(this);
}

SmCaretPosGraphBuildingVisitor::~SmCaretPosGraphBuildingVisitor()
{
}

void SmCaretPosGraphBuildingVisitor::Visit( SmLineNode* pNode ){
    SmNodeIterator it( pNode );
    while( it.Next( ) ){
        it->Accept( this );
    }
}

/** Build SmCaretPosGraph for SmTableNode
 * This method covers cases where SmTableNode is used in a binom or stack,
 * the special case where it is used as root node for the entire formula is
 * handled in the constructor.
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmTableNode* pNode ){
    SmCaretPosGraphEntry *left  = mpRightMost,
                         *right = mpGraph->Add( SmCaretPos( pNode, 1) );
    bool bIsFirst = true;
    SmNodeIterator it( pNode );
    while( it.Next() ){
        mpRightMost = mpGraph->Add( SmCaretPos( it.Current(), 0 ), left);
        if(bIsFirst)
            left->SetRight(mpRightMost);
        it->Accept( this );
        mpRightMost->SetRight(right);
        if(bIsFirst)
            right->SetLeft(mpRightMost);
        bIsFirst = false;
    }
    mpRightMost = right;
}

/** Build SmCaretPosGraph for SmSubSupNode
 *
 * The child positions in a SubSupNode, where H is the body:
 * \code
 *      CSUP
 *
 * LSUP H  H RSUP
 *      H  H
 *      HHHH
 *      H  H
 * LSUB H  H RSUB
 *
 *      CSUB
 * \endcode
 *
 * Graph over these, where "left" is before the SmSubSupNode and "right" is after:
 * \dot
 *  digraph Graph{
 *      left -> H;
 *      H -> right;
 *      LSUP -> H;
 *      LSUB -> H;
 *      CSUP -> right;
 *      CSUB -> right;
 *      RSUP -> right;
 *      RSUB -> right;
 *  };
 * \enddot
 *
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmSubSupNode* pNode )
{
    SmCaretPosGraphEntry *left,
                         *right,
                         *bodyLeft,
                         *bodyRight;

    assert(mpRightMost);
    left = mpRightMost;

    //Create bodyLeft
    SAL_WARN_IF( !pNode->GetBody(), "starmath", "SmSubSupNode Doesn't have a body!" );
    bodyLeft = mpGraph->Add( SmCaretPos( pNode->GetBody( ), 0 ), left );
    left->SetRight( bodyLeft ); //TODO: Don't make this if LSUP or LSUB are NULL ( not sure??? )

    //Create right
    right = mpGraph->Add( SmCaretPos( pNode, 1 ) );

    //Visit the body, to get bodyRight
    mpRightMost = bodyLeft;
    pNode->GetBody( )->Accept( this );
    bodyRight = mpRightMost;
    bodyRight->SetRight( right );
    right->SetLeft( bodyRight );

    SmNode* pChild;
    //If there's an LSUP
    if( ( pChild = pNode->GetSubSup( LSUP ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), left );

        mpRightMost = cLeft;
        pChild->Accept( this );

        mpRightMost->SetRight( bodyLeft );
    }
    //If there's an LSUB
    if( ( pChild = pNode->GetSubSup( LSUB ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), left );

        mpRightMost = cLeft;
        pChild->Accept( this );

        mpRightMost->SetRight( bodyLeft );
    }
    //If there's an CSUP
    if( ( pChild = pNode->GetSubSup( CSUP ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), left );

        mpRightMost = cLeft;
        pChild->Accept( this );

        mpRightMost->SetRight( right );
    }
    //If there's an CSUB
    if( ( pChild = pNode->GetSubSup( CSUB ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), left );

        mpRightMost = cLeft;
        pChild->Accept( this );

        mpRightMost->SetRight( right );
    }
    //If there's an RSUP
    if( ( pChild = pNode->GetSubSup( RSUP ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), bodyRight );

        mpRightMost = cLeft;
        pChild->Accept( this );

        mpRightMost->SetRight( right );
    }
    //If there's an RSUB
    if( ( pChild = pNode->GetSubSup( RSUB ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), bodyRight );

        mpRightMost = cLeft;
        pChild->Accept( this );

        mpRightMost->SetRight( right );
    }

    //Set return parameters
    mpRightMost = right;
}

/** Build caret position for SmOperNode
 *
 * If first child is an SmSubSupNode we will ignore its
 * body, as this body is a SmMathSymbol, for SUM, INT or similar
 * that shouldn't be subject to modification.
 * If first child is not a SmSubSupNode, ignore it completely
 * as it is a SmMathSymbol.
 *
 * The child positions in a SmOperNode, where H is symbol, e.g. int, sum or similar:
 * \code
 *       TO
 *
 * LSUP H  H RSUP    BBB    BB  BBB  B   B
 *      H  H         B  B  B  B B  B  B B
 *      HHHH         BBB   B  B B  B   B
 *      H  H         B  B  B  B B  B   B
 * LSUB H  H RSUB    BBB    BB  BBB    B
 *
 *      FROM
 * \endcode
 * Notice, CSUP, etc. are actually granchildren, but inorder to ignore H, these are visited
 * from here. If they are present, that is if pOper is an instance of SmSubSupNode.
 *
 * Graph over these, where "left" is before the SmOperNode and "right" is after:
 * \dot
 *  digraph Graph{
 *      left -> BODY;
 *      BODY -> right;
 *      LSUP -> BODY;
 *      LSUB -> BODY;
 *      TO   -> BODY;
 *      FROM -> BODY;
 *      RSUP -> BODY;
 *      RSUB -> BODY;
 *  };
 * \enddot
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmOperNode* pNode )
{
    SmNode *pOper = pNode->GetSubNode( 0 ),
           *pBody = pNode->GetSubNode( 1 );

    SmCaretPosGraphEntry *left = mpRightMost,
                         *bodyLeft,
                         *bodyRight,
                         *right;
    //Create body left
    bodyLeft = mpGraph->Add( SmCaretPos( pBody, 0 ), left );
    left->SetRight( bodyLeft );

    //Visit body, get bodyRight
    mpRightMost = bodyLeft;
    pBody->Accept( this );
    bodyRight = mpRightMost;

    //Create right
    right = mpGraph->Add( SmCaretPos( pNode, 1 ), bodyRight );
    bodyRight->SetRight( right );

    //Get subsup pNode if any
    SmSubSupNode* pSubSup = pOper->GetType( ) == NSUBSUP ? static_cast<SmSubSupNode*>(pOper) : nullptr;

    SmNode* pChild;
    SmCaretPosGraphEntry *childLeft;
    if( pSubSup && ( pChild = pSubSup->GetSubSup( LSUP ) ) ) {
        //Create position in front of pChild
        childLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        mpRightMost = childLeft;
        pChild->Accept( this );
        //Set right on mpRightMost from pChild
        mpRightMost->SetRight( bodyLeft );
    }
    if( pSubSup && ( pChild = pSubSup->GetSubSup( LSUB ) ) ) {
        //Create position in front of pChild
        childLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        mpRightMost = childLeft;
        pChild->Accept( this );
        //Set right on mpRightMost from pChild
        mpRightMost->SetRight( bodyLeft );
    }
    if( pSubSup && ( pChild = pSubSup->GetSubSup( CSUP ) ) ) {//TO
        //Create position in front of pChild
        childLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        mpRightMost = childLeft;
        pChild->Accept( this );
        //Set right on mpRightMost from pChild
        mpRightMost->SetRight( bodyLeft );
    }
    if( pSubSup && ( pChild = pSubSup->GetSubSup( CSUB ) ) ) { //FROM
        //Create position in front of pChild
        childLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        mpRightMost = childLeft;
        pChild->Accept( this );
        //Set right on mpRightMost from pChild
        mpRightMost->SetRight( bodyLeft );
    }
    if( pSubSup && ( pChild = pSubSup->GetSubSup( RSUP ) ) ) {
        //Create position in front of pChild
        childLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        mpRightMost = childLeft;
        pChild->Accept( this );
        //Set right on mpRightMost from pChild
        mpRightMost->SetRight( bodyLeft );
    }
    if( pSubSup && ( pChild = pSubSup->GetSubSup( RSUB ) ) ) {
        //Create position in front of pChild
        childLeft = mpGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        mpRightMost = childLeft;
        pChild->Accept( this );
        //Set right on mpRightMost from pChild
        mpRightMost->SetRight( bodyLeft );
    }

    //Return right
    mpRightMost = right;
}

void SmCaretPosGraphBuildingVisitor::Visit( SmMatrixNode* pNode )
{
    SmCaretPosGraphEntry *left  = mpRightMost,
                         *right = mpGraph->Add( SmCaretPos( pNode, 1 ) );

    for ( sal_uInt16 i = 0;  i < pNode->GetNumRows( ); i++ ) {
        SmCaretPosGraphEntry* r = left;
        for ( sal_uInt16 j = 0;  j < pNode->GetNumCols( ); j++ ){
            SmNode* pSubNode = pNode->GetSubNode( i * pNode->GetNumCols( ) + j );

            mpRightMost = mpGraph->Add( SmCaretPos( pSubNode, 0 ), r );
            if( j != 0 || ( pNode->GetNumRows( ) - 1 ) / 2 == i )
                r->SetRight( mpRightMost );

            pSubNode->Accept( this );

            r = mpRightMost;
        }
        mpRightMost->SetRight( right );
        if( ( pNode->GetNumRows( ) - 1 ) / 2 == i )
            right->SetLeft( mpRightMost );
    }

    mpRightMost = right;
}

/** Build SmCaretPosGraph for SmTextNode
 *
 * Lines in an SmTextNode:
 * \code
 * A B C
 * \endcode
 * Where A B and C are characters in the text.
 *
 * Graph over these, where "left" is before the SmTextNode and "right" is after:
 * \dot
 *  digraph Graph{
 *      left -> A;
 *      A -> B
 *      B -> right;
 *  };
 * \enddot
 * Notice that C and right is the same position here.
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmTextNode* pNode )
{
    SAL_WARN_IF( pNode->GetText().isEmpty(), "starmath", "Empty SmTextNode is bad" );

    int size = pNode->GetText().getLength();
    for( int i = 1; i <= size; i++ ){
        SmCaretPosGraphEntry* pRight = mpRightMost;
        mpRightMost = mpGraph->Add( SmCaretPos( pNode, i ), pRight );
        pRight->SetRight( mpRightMost );
    }
}

/** Build SmCaretPosGraph for SmBinVerNode
 *
 * Lines in an SmBinVerNode:
 * \code
 *    A
 *  -----
 *    B
 * \endcode
 *
 * Graph over these, where "left" is before the SmBinVerNode and "right" is after:
 * \dot
 *  digraph Graph{
 *      left -> A;
 *      A -> right;
 *      B -> right;
 *  };
 * \enddot
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmBinVerNode* pNode )
{
    //None if these children can be NULL, see SmBinVerNode::Arrange
    SmNode  *pNum   = pNode->GetSubNode( 0 ),
            *pDenom = pNode->GetSubNode( 2 );

    SmCaretPosGraphEntry *left,
                         *right,
                         *numLeft,
                         *denomLeft;

    assert(mpRightMost);
    //Set left
    left = mpRightMost;

    //Create right
    right = mpGraph->Add( SmCaretPos( pNode, 1 ) );

    //Create numLeft
    numLeft = mpGraph->Add( SmCaretPos( pNum, 0 ), left );
    left->SetRight( numLeft );

    //Visit pNum
    mpRightMost = numLeft;
    pNum->Accept( this );
    mpRightMost->SetRight( right );
    right->SetLeft( mpRightMost );

    //Create denomLeft
    denomLeft = mpGraph->Add( SmCaretPos( pDenom, 0 ), left );

    //Visit pDenom
    mpRightMost = denomLeft;
    pDenom->Accept( this );
    mpRightMost->SetRight( right );

    //Set return parameter
    mpRightMost = right;
}

/** Build SmCaretPosGraph for SmVerticalBraceNode
 *
 * Lines in an SmVerticalBraceNode:
 * \code
 *   pScript
 *  ________
 * /        \
 *   pBody
 * \endcode
 *
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmVerticalBraceNode* pNode )
{
    SmNode  *pBody   = pNode->GetSubNode( 0 ),
            *pScript = pNode->GetSubNode( 2 );
    //None of these children can be NULL

    SmCaretPosGraphEntry  *left,
                        *bodyLeft,
                        *scriptLeft,
                        *right;

    left = mpRightMost;

    //Create right
    right = mpGraph->Add( SmCaretPos( pNode, 1 ) );

    //Create bodyLeft
    bodyLeft = mpGraph->Add( SmCaretPos( pBody, 0 ), left );
    left->SetRight( bodyLeft );
    mpRightMost = bodyLeft;
    pBody->Accept( this );
    mpRightMost->SetRight( right );
    right->SetLeft( mpRightMost );

    //Create script
    scriptLeft = mpGraph->Add( SmCaretPos( pScript, 0 ), left );
    mpRightMost = scriptLeft;
    pScript->Accept( this );
    mpRightMost->SetRight( right );

    //Set return value
    mpRightMost = right;
}

/** Build SmCaretPosGraph for SmBinDiagonalNode
 *
 * Lines in an SmBinDiagonalNode:
 * \code
 *  A /
 *   /
 *  / B
 * \endcode
 * Where A and B are lines.
 *
 * Used in formulas such as "A wideslash B"
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmBinDiagonalNode* pNode )
{
    SmNode  *A = pNode->GetSubNode( 0 ),
            *B = pNode->GetSubNode( 1 );

    SmCaretPosGraphEntry  *left,
                        *leftA,
                        *rightA,
                        *leftB,
                        *right;
    left = mpRightMost;

    //Create right
    right = mpGraph->Add( SmCaretPos( pNode, 1 ) );

    //Create left A
    leftA = mpGraph->Add( SmCaretPos( A, 0 ), left );
    left->SetRight( leftA );

    //Visit A
    mpRightMost = leftA;
    A->Accept( this );
    rightA = mpRightMost;

    //Create left B
    leftB = mpGraph->Add( SmCaretPos( B, 0 ), rightA );
    rightA->SetRight( leftB );

    //Visit B
    mpRightMost = leftB;
    B->Accept( this );
    mpRightMost->SetRight( right );
    right->SetLeft( mpRightMost );

    //Set return value
    mpRightMost = right;
}

//Straigt forward ( I think )
void SmCaretPosGraphBuildingVisitor::Visit( SmBinHorNode* pNode )
{
    SmNodeIterator it( pNode );
    while( it.Next( ) )
        it->Accept( this );
}
void SmCaretPosGraphBuildingVisitor::Visit( SmUnHorNode* pNode )
{
    // Unary operator node
    SmNodeIterator it( pNode );
    while( it.Next( ) )
        it->Accept( this );

}

void SmCaretPosGraphBuildingVisitor::Visit( SmExpressionNode* pNode )
{
    SmNodeIterator it( pNode );
    while( it.Next( ) )
        it->Accept( this );
}

void SmCaretPosGraphBuildingVisitor::Visit( SmFontNode* pNode )
{
    //Has only got one child, should act as an expression if possible
    SmNodeIterator it( pNode );
    while( it.Next( ) )
        it->Accept( this );
}

/** Build SmCaretPosGraph for SmBracebodyNode
 * Acts as an SmExpressionNode
 *
 * Below is an example of a formula tree that has multiple children for SmBracebodyNode
 * \dot
 * digraph {
 * labelloc = "t";
 * label= "Equation: \"lbrace i mline i in setZ rbrace\"";
 * n0 [label="SmTableNode"];
 * n0 -> n1 [label="0"];
 * n1 [label="SmLineNode"];
 * n1 -> n2 [label="0"];
 * n2 [label="SmExpressionNode"];
 * n2 -> n3 [label="0"];
 * n3 [label="SmBraceNode"];
 * n3 -> n4 [label="0"];
 * n4 [label="SmMathSymbolNode: {"];
 * n3 -> n5 [label="1"];
 * n5 [label="SmBracebodyNode"];
 * n5 -> n6 [label="0"];
 * n6 [label="SmExpressionNode"];
 * n6 -> n7 [label="0"];
 * n7 [label="SmTextNode: i"];
 * n5 -> n8 [label="1"];
 * n8 [label="SmMathSymbolNode: &#124;"]; // Unicode "VERTICAL LINE"
 * n5 -> n9 [label="2"];
 * n9 [label="SmExpressionNode"];
 * n9 -> n10 [label="0"];
 * n10 [label="SmBinHorNode"];
 * n10 -> n11 [label="0"];
 * n11 [label="SmTextNode: i"];
 * n10 -> n12 [label="1"];
 * n12 [label="SmMathSymbolNode: &#8712;"]; // Unicode "ELEMENT OF"
 * n10 -> n13 [label="2"];
 * n13 [label="SmMathSymbolNode: &#8484;"]; // Unicode "DOUBLE-STRUCK CAPITAL Z"
 * n3 -> n14 [label="2"];
 * n14 [label="SmMathSymbolNode: }"];
 * }
 * \enddot
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmBracebodyNode* pNode )
{
    SmNodeIterator it( pNode );
    while( it.Next( ) ) {
        SmCaretPosGraphEntry* pStart = mpGraph->Add( SmCaretPos( it.Current(), 0), mpRightMost );
        mpRightMost->SetRight( pStart );
        mpRightMost = pStart;
        it->Accept( this );
    }
}

/** Build SmCaretPosGraph for SmAlignNode
 * Acts as an SmExpressionNode, as it only has one child this okay
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmAlignNode* pNode )
{
    SmNodeIterator it( pNode );
    while( it.Next( ) )
        it->Accept( this );
}

/** Build SmCaretPosGraph for SmRootNode
 *
 * Lines in an SmRootNode:
 * \code
 *    _________
 *  A/
 * \/    B
 *
 * \endcode
 * A: pExtra ( optional, can be NULL ),
 * B: pBody
 *
 * Graph over these, where "left" is before the SmRootNode and "right" is after:
 * \dot
 *  digraph Graph{
 *      left -> B;
 *      B -> right;
 *      A -> B;
 *  }
 * \enddot
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmRootNode* pNode )
{
    SmNode  *pExtra = pNode->GetSubNode( 0 ), //Argument, NULL for sqrt, and SmTextNode if cubicroot
            *pBody  = pNode->GetSubNode( 2 ); //Body of the root
    assert(pBody);

    SmCaretPosGraphEntry  *left,
                        *right,
                        *bodyLeft,
                        *bodyRight;

    //Get left and save it
    assert(mpRightMost);
    left = mpRightMost;

    //Create body left
    bodyLeft = mpGraph->Add( SmCaretPos( pBody, 0 ), left );
    left->SetRight( bodyLeft );

    //Create right
    right = mpGraph->Add( SmCaretPos( pNode, 1 ) );

    //Visit body
    mpRightMost = bodyLeft;
    pBody->Accept( this );
    bodyRight = mpRightMost;
    bodyRight->SetRight( right );
    right->SetLeft( bodyRight );

    //Visit pExtra
    if( pExtra ){
        mpRightMost = mpGraph->Add( SmCaretPos( pExtra, 0 ), left );
        pExtra->Accept( this );
        mpRightMost->SetRight( bodyLeft );
    }

    mpRightMost = right;
}


void SmCaretPosGraphBuildingVisitor::Visit( SmDynIntegralNode* pNode )
{
    //! To be changed: Integrals don't have args.
    SmNode  *pBody  = pNode->Body(); //Body of the root
    assert(pBody);

    SmCaretPosGraphEntry  *left,
                        *right,
                        *bodyLeft,
                        *bodyRight;

    //Get left and save it
    assert(mpRightMost);
    left = mpRightMost;

    //Create body left
    bodyLeft = mpGraph->Add( SmCaretPos( pBody, 0 ), left );
    left->SetRight( bodyLeft );

    //Create right
    right = mpGraph->Add( SmCaretPos( pNode, 1 ) );

    //Visit body
    mpRightMost = bodyLeft;
    pBody->Accept( this );
    bodyRight = mpRightMost;
    bodyRight->SetRight( right );
    right->SetLeft( bodyRight );

    mpRightMost = right;
}


/** Build SmCaretPosGraph for SmPlaceNode
 * Consider this a single character.
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmPlaceNode* pNode )
{
    SmCaretPosGraphEntry* right = mpGraph->Add( SmCaretPos( pNode, 1 ), mpRightMost );
    mpRightMost->SetRight( right );
    mpRightMost = right;
}

/** SmErrorNode is context dependent metadata, it can't be selected
 *
 * @remarks There's no point in deleting, copying and/or moving an instance
 * of SmErrorNode as it may not exist in an other context! Thus there are no
 * positions to select an SmErrorNode.
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmErrorNode* )
{
}

/** Build SmCaretPosGraph for SmBlankNode
 * Consider this a single character, as it is only a blank space
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmBlankNode* pNode )
{
    SmCaretPosGraphEntry* right = mpGraph->Add( SmCaretPos( pNode, 1 ), mpRightMost );
    mpRightMost->SetRight( right );
    mpRightMost = right;
}

/** Build SmCaretPosGraph for SmBraceNode
 *
 * Lines in an SmBraceNode:
 * \code
 * |     |
 * |  B  |
 * |     |
 * \endcode
 * B: Body
 *
 * Graph over these, where "left" is before the SmBraceNode and "right" is after:
 * \dot
 *  digraph Graph{
 *      left -> B;
 *      B -> right;
 *  }
 * \enddot
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmBraceNode* pNode )
{
    SmNode* pBody = pNode->GetSubNode( 1 );

    SmCaretPosGraphEntry  *left = mpRightMost,
                        *right = mpGraph->Add( SmCaretPos( pNode, 1 ) );

    if( pBody->GetType() != NBRACEBODY ) {
        mpRightMost = mpGraph->Add( SmCaretPos( pBody, 0 ), left );
        left->SetRight( mpRightMost );
    }else
        mpRightMost = left;

    pBody->Accept( this );
    mpRightMost->SetRight( right );
    right->SetLeft( mpRightMost );

    mpRightMost = right;
}

/** Build SmCaretPosGraph for SmAttributNode
 *
 * Lines in an SmAttributNode:
 * \code
 *   Attr
 *   Body
 * \endcode
 *
 * There's a body and an attribute, the construction is used for "widehat A", where "A" is the body
 * and "^" is the attribute ( note GetScaleMode( ) on SmAttributNode tells how the attribute should be
 * scaled ).
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmAttributNode* pNode )
{
    SmNode  *pAttr = pNode->GetSubNode( 0 ),
            *pBody = pNode->GetSubNode( 1 );
    //None of the children can be NULL

    SmCaretPosGraphEntry  *left = mpRightMost,
                        *attrLeft,
                        *bodyLeft,
                        *bodyRight,
                        *right;

    //Creating bodyleft
    bodyLeft = mpGraph->Add( SmCaretPos( pBody, 0 ), left );
    left->SetRight( bodyLeft );

    //Creating right
    right = mpGraph->Add( SmCaretPos( pNode, 1 ) );

    //Visit the body
    mpRightMost = bodyLeft;
    pBody->Accept( this );
    bodyRight = mpRightMost;
    bodyRight->SetRight( right );
    right->SetLeft( bodyRight );

    //Create attrLeft
    attrLeft = mpGraph->Add( SmCaretPos( pAttr, 0 ), left );

    //Visit attribute
    mpRightMost = attrLeft;
    pAttr->Accept( this );
    mpRightMost->SetRight( right );

    //Set return value
    mpRightMost = right;
}

//Consider these single symbols
void SmCaretPosGraphBuildingVisitor::Visit( SmSpecialNode* pNode )
{
    SmCaretPosGraphEntry* right = mpGraph->Add( SmCaretPos( pNode, 1 ), mpRightMost );
    mpRightMost->SetRight( right );
    mpRightMost = right;
}
void SmCaretPosGraphBuildingVisitor::Visit( SmGlyphSpecialNode* pNode )
{
    SmCaretPosGraphEntry* right = mpGraph->Add( SmCaretPos( pNode, 1 ), mpRightMost );
    mpRightMost->SetRight( right );
    mpRightMost = right;
}
void SmCaretPosGraphBuildingVisitor::Visit( SmMathSymbolNode* pNode )
{
    SmCaretPosGraphEntry* right = mpGraph->Add( SmCaretPos( pNode, 1 ), mpRightMost );
    mpRightMost->SetRight( right );
    mpRightMost = right;
}

void SmCaretPosGraphBuildingVisitor::Visit( SmRootSymbolNode* )
{
    //Do nothing
}

void SmCaretPosGraphBuildingVisitor::Visit( SmDynIntegralSymbolNode* )
{
    //Do nothing
}


void SmCaretPosGraphBuildingVisitor::Visit( SmRectangleNode* )
{
    //Do nothing
}
void SmCaretPosGraphBuildingVisitor::Visit( SmPolyLineNode* )
{
    //Do nothing
}

// SmCloningVisitor

SmNode* SmCloningVisitor::Clone( SmNode* pNode )
{
    SmNode* pCurrResult = pResult;
    pNode->Accept( this );
    SmNode* pClone = pResult;
    pResult = pCurrResult;
    return pClone;
}

void SmCloningVisitor::CloneNodeAttr( SmNode* pSource, SmNode* pTarget )
{
    pTarget->SetScaleMode( pSource->GetScaleMode( ) );
    //Other attributes are set when prepare or arrange is executed
    //and may depend on stuff not being cloned here.
}

void SmCloningVisitor::CloneKids( SmStructureNode* pSource, SmStructureNode* pTarget )
{
    //Cache current result
    SmNode* pCurrResult = pResult;

    //Create array for holding clones
    sal_uInt16 nSize = pSource->GetNumSubNodes( );
    SmNodeArray aNodes( nSize );

    //Clone children
    for( sal_uInt16 i = 0; i < nSize; i++ ){
        SmNode* pKid;
        if( nullptr != ( pKid = pSource->GetSubNode( i ) ) )
            pKid->Accept( this );
        else
            pResult = nullptr;
        aNodes[i] = pResult;
    }

    //Set subnodes of pTarget
    pTarget->SetSubNodes( aNodes );

    //Restore result as where prior to call
    pResult = pCurrResult;
}

void SmCloningVisitor::Visit( SmTableNode* pNode )
{
    SmTableNode* pClone = new SmTableNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmBraceNode* pNode )
{
    SmBraceNode* pClone = new SmBraceNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmBracebodyNode* pNode )
{
    SmBracebodyNode* pClone = new SmBracebodyNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmOperNode* pNode )
{
    SmOperNode* pClone = new SmOperNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmAlignNode* pNode )
{
    SmAlignNode* pClone = new SmAlignNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmAttributNode* pNode )
{
    SmAttributNode* pClone = new SmAttributNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmFontNode* pNode )
{
    SmFontNode* pClone = new SmFontNode( pNode->GetToken( ) );
    pClone->SetSizeParameter( pNode->GetSizeParameter( ), pNode->GetSizeType( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmUnHorNode* pNode )
{
    SmUnHorNode* pClone = new SmUnHorNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmBinHorNode* pNode )
{
    SmBinHorNode* pClone = new SmBinHorNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmBinVerNode* pNode )
{
    SmBinVerNode* pClone = new SmBinVerNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmBinDiagonalNode* pNode )
{
    SmBinDiagonalNode *pClone = new SmBinDiagonalNode( pNode->GetToken( ) );
    pClone->SetAscending( pNode->IsAscending( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmSubSupNode* pNode )
{
    SmSubSupNode *pClone = new SmSubSupNode( pNode->GetToken( ) );
    pClone->SetUseLimits( pNode->IsUseLimits( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmMatrixNode* pNode )
{
    SmMatrixNode *pClone = new SmMatrixNode( pNode->GetToken( ) );
    pClone->SetRowCol( pNode->GetNumRows( ), pNode->GetNumCols( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmPlaceNode* pNode )
{
    pResult = new SmPlaceNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pResult );
}

void SmCloningVisitor::Visit( SmTextNode* pNode )
{
    SmTextNode* pClone = new SmTextNode( pNode->GetToken( ), pNode->GetFontDesc( ) );
    pClone->ChangeText( pNode->GetText( ) );
    CloneNodeAttr( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmSpecialNode* pNode )
{
    pResult = new SmSpecialNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pResult );
}

void SmCloningVisitor::Visit( SmGlyphSpecialNode* pNode )
{
    pResult = new SmGlyphSpecialNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pResult );
}

void SmCloningVisitor::Visit( SmMathSymbolNode* pNode )
{
    pResult = new SmMathSymbolNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pResult );
}

void SmCloningVisitor::Visit( SmBlankNode* pNode )
{
    SmBlankNode* pClone = new SmBlankNode( pNode->GetToken( ) );
    pClone->SetBlankNum( pNode->GetBlankNum( ) );
    pResult = pClone;
    CloneNodeAttr( pNode, pResult );
}

void SmCloningVisitor::Visit( SmErrorNode* pNode )
{
    //PE_NONE is used the information have been discarded and isn't used
    pResult = new SmErrorNode( PE_NONE, pNode->GetToken( ) );
    CloneNodeAttr( pNode, pResult );
}

void SmCloningVisitor::Visit( SmLineNode* pNode )
{
    SmLineNode* pClone = new SmLineNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmExpressionNode* pNode )
{
    SmExpressionNode* pClone = new SmExpressionNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmPolyLineNode* pNode )
{
    pResult = new SmPolyLineNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pResult );
}

void SmCloningVisitor::Visit( SmRootNode* pNode )
{
    SmRootNode* pClone = new SmRootNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmRootSymbolNode* pNode )
{
    pResult = new SmRootSymbolNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pResult );
}

void SmCloningVisitor::Visit( SmDynIntegralNode* pNode )
{
    SmDynIntegralNode* pClone = new SmDynIntegralNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

void SmCloningVisitor::Visit( SmDynIntegralSymbolNode* pNode )
{
    pResult = new SmDynIntegralSymbolNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pResult );
}

void SmCloningVisitor::Visit( SmRectangleNode* pNode )
{
    pResult = new SmRectangleNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pResult );
}

void SmCloningVisitor::Visit( SmVerticalBraceNode* pNode )
{
    SmVerticalBraceNode* pClone = new SmVerticalBraceNode( pNode->GetToken( ) );
    CloneNodeAttr( pNode, pClone );
    CloneKids( pNode, pClone );
    pResult = pClone;
}

// SmSelectionDrawingVisitor

SmSelectionDrawingVisitor::SmSelectionDrawingVisitor( OutputDevice& rDevice, SmNode* pTree, Point Offset )
    : rDev( rDevice ) {
    bHasSelectionArea = false;

    //Visit everything
    SAL_WARN_IF( !pTree, "starmath", "pTree can't be null!" );
    if( pTree )
        pTree->Accept( this );

    //Draw selection if there's any
    if( bHasSelectionArea ){
        aSelectionArea.Move( Offset.X( ), Offset.Y( ) );

        //Save device state
        rDev.Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
        //Change colors
        rDev.SetLineColor( );
        rDev.SetFillColor( Color( COL_LIGHTGRAY ) );

        //Draw rectangle
        rDev.DrawRect( aSelectionArea );

        //Restore device state
        rDev.Pop( );
    }
}

void SmSelectionDrawingVisitor::ExtendSelectionArea(const Rectangle& rArea)
{
    if ( ! bHasSelectionArea ) {
        aSelectionArea = rArea;
        bHasSelectionArea = true;
    } else
        aSelectionArea.Union(rArea);
}

void SmSelectionDrawingVisitor::DefaultVisit( SmNode* pNode )
{
    if( pNode->IsSelected( ) )
        ExtendSelectionArea( pNode->AsRectangle( ) );
    VisitChildren( pNode );
}

void SmSelectionDrawingVisitor::VisitChildren( SmNode* pNode )
{
    SmNodeIterator it( pNode );
    while( it.Next( ) )
        it->Accept( this );
}

void SmSelectionDrawingVisitor::Visit( SmTextNode* pNode )
{
    if( pNode->IsSelected( ) ){
        rDev.Push( PushFlags::TEXTCOLOR | PushFlags::FONT );

        rDev.SetFont( pNode->GetFont( ) );
        Point Position = pNode->GetTopLeft( );
        long left   = Position.getX( ) + rDev.GetTextWidth( pNode->GetText( ), 0, pNode->GetSelectionStart( ) );
        long right  = Position.getX( ) + rDev.GetTextWidth( pNode->GetText( ), 0, pNode->GetSelectionEnd( ) );
        long top    = Position.getY( );
        long bottom = top + pNode->GetHeight( );
        Rectangle rect( left, top, right, bottom );

        ExtendSelectionArea( rect );

        rDev.Pop( );
    }
}

// SmNodeToTextVisitor

SmNodeToTextVisitor::SmNodeToTextVisitor( SmNode* pNode, OUString &rText )
{
    pNode->Accept( this );
    rText = aCmdText.makeStringAndClear();
}

void SmNodeToTextVisitor::Visit( SmTableNode* pNode )
{
    if( pNode->GetToken( ).eType == TBINOM ) {
        Append( "{ binom" );
        LineToText( pNode->GetSubNode( 0 ) );
        LineToText( pNode->GetSubNode( 1 ) );
        Append("} ");
    } else if( pNode->GetToken( ).eType == TSTACK ) {
        Append( "stack{ " );
        SmNodeIterator it( pNode );
        it.Next( );
        while( true ) {
            LineToText( it.Current( ) );
            if( it.Next( ) ) {
                Separate( );
                Append( "# " );
            }else
                break;
        }
        Separate( );
        Append( "}" );
    } else { //Assume it's a toplevel table, containing lines
        SmNodeIterator it( pNode );
        it.Next( );
        while( true ) {
            Separate( );
            it->Accept( this );
            if( it.Next( ) ) {
                Separate( );
                Append( "newline" );
            }else
                break;
        }
    }
}

void SmNodeToTextVisitor::Visit( SmBraceNode* pNode )
{
    SmNode *pLeftBrace  = pNode->GetSubNode( 0 ),
           *pBody       = pNode->GetSubNode( 1 ),
           *pRightBrace = pNode->GetSubNode( 2 );
    //Handle special case where it's absolute function
    if( pNode->GetToken( ).eType == TABS ) {
        Append( "abs" );
        LineToText( pBody );
    } else {
        if( pNode->GetScaleMode( ) == SCALE_HEIGHT )
            Append( "left " );
        pLeftBrace->Accept( this );
        Separate( );
        pBody->Accept( this );
        Separate( );
        if( pNode->GetScaleMode( ) == SCALE_HEIGHT )
            Append( "right " );
        pRightBrace->Accept( this );
    }
}

void SmNodeToTextVisitor::Visit( SmBracebodyNode* pNode )
{
    SmNodeIterator it( pNode );
    while( it.Next( ) ){
        Separate( );
        it->Accept( this );
    }
}

void SmNodeToTextVisitor::Visit( SmOperNode* pNode )
{
    Append( pNode->GetToken( ).aText );
    Separate( );
    if( pNode->GetToken( ).eType == TOPER ){
        //There's an SmGlyphSpecialNode if eType == TOPER
        if( pNode->GetSubNode( 0 )->GetType( ) == NSUBSUP )
            Append( pNode->GetSubNode( 0 )->GetSubNode( 0 )->GetToken( ).aText );
        else
            Append( pNode->GetSubNode( 0 )->GetToken( ).aText );
    }
    if( pNode->GetSubNode( 0 )->GetType( ) == NSUBSUP ) {
        SmSubSupNode *pSubSup = static_cast<SmSubSupNode*>( pNode->GetSubNode( 0 ) );
        SmNode* pChild;
        if( ( pChild = pSubSup->GetSubSup( LSUP ) ) ) {
            Separate( );
            Append( "lsup { " );
            LineToText( pChild );
            Append( "} " );
        }
        if( ( pChild = pSubSup->GetSubSup( LSUB ) ) ) {
            Separate( );
            Append( "lsub { " );
            LineToText( pChild );
            Append( "} " );
        }
        if( ( pChild = pSubSup->GetSubSup( RSUP ) ) ) {
            Separate( );
            Append( "^ { " );
            LineToText( pChild );
            Append( "} " );
        }
        if( ( pChild = pSubSup->GetSubSup( RSUB ) ) ) {
            Separate( );
            Append( "_ { " );
            LineToText( pChild );
            Append( "} " );
        }
        if( ( pChild = pSubSup->GetSubSup( CSUP ) ) ) {
            Separate( );
            if (pSubSup->IsUseLimits())
                Append( "to { " );
            else
                Append( "csup { " );
            LineToText( pChild );
            Append( "} " );
        }
        if( ( pChild = pSubSup->GetSubSup( CSUB ) ) ) {
            Separate( );
            if (pSubSup->IsUseLimits())
                Append( "from { " );
            else
                Append( "csub { " );
            LineToText( pChild );
            Append( "} " );
        }
    }
    LineToText( pNode->GetSubNode( 1 ) );
}

void SmNodeToTextVisitor::Visit( SmAlignNode* pNode )
{
    Append( pNode->GetToken( ).aText );
    LineToText( pNode->GetSubNode( 0 ) );
}

void SmNodeToTextVisitor::Visit( SmAttributNode* pNode )
{
    Append( pNode->GetToken( ).aText );
    LineToText( pNode->GetSubNode( 1 ) );
}

void SmNodeToTextVisitor::Visit( SmFontNode* pNode )
{
    switch ( pNode->GetToken( ).eType )
    {
        case TBOLD:
            Append( "bold " );
            break;
        case TNBOLD:
            Append( "nbold " );
            break;
        case TITALIC:
            Append( "italic " );
            break;
        case TNITALIC:
            Append( "nitalic " );
            break;
        case TPHANTOM:
            Append( "phantom " );
            break;
        case TSIZE:
            {
                Append( "size " );
                switch ( pNode->GetSizeType( ) )
                {
                    case FontSizeType::PLUS:
                        Append( "+" );
                        break;
                    case FontSizeType::MINUS:
                        Append( "-" );
                        break;
                    case FontSizeType::MULTIPLY:
                        Append( "*" );
                        break;
                    case FontSizeType::DIVIDE:
                        Append( "/" );
                        break;
                    case FontSizeType::ABSOLUT:
                    default:
                        break;
                }
                Append( ::rtl::math::doubleToUString(
                            static_cast<double>( pNode->GetSizeParameter( ) ),
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', true ) );
                Append( " " );
            }
            break;
        case TBLACK:
            Append( "color black " );
            break;
        case TWHITE:
            Append( "color white " );
            break;
        case TRED:
            Append( "color red " );
            break;
        case TGREEN:
            Append( "color green " );
            break;
        case TBLUE:
            Append( "color blue " );
            break;
        case TCYAN:
            Append( "color cyan " );
            break;
        case TMAGENTA:
            Append( "color magenta " );
            break;
        case TYELLOW:
            Append( "color yellow " );
            break;
        case TSANS:
            Append( "font sans " );
            break;
        case TSERIF:
            Append( "font serif " );
            break;
        case TFIXED:
            Append( "font fixed " );
            break;
        default:
            break;
    }
    LineToText( pNode->GetSubNode( 1 ) );
}

void SmNodeToTextVisitor::Visit( SmUnHorNode* pNode )
{
    SmNodeIterator it( pNode, pNode->GetSubNode( 1 )->GetToken( ).eType == TFACT );
    while( it.Next( ) ) {
        Separate( );
        it->Accept( this );
    }
}

void SmNodeToTextVisitor::Visit( SmBinHorNode* pNode )
{
    SmNode *pLeft  = pNode->GetSubNode( 0 ),
           *pOper  = pNode->GetSubNode( 1 ),
           *pRight = pNode->GetSubNode( 2 );
    Separate( );
    pLeft->Accept( this );
    Separate( );
    pOper->Accept( this );
    Separate( );
    pRight->Accept( this );
    Separate( );
}

void SmNodeToTextVisitor::Visit( SmBinVerNode* pNode )
{
    SmNode *pNum    = pNode->GetSubNode( 0 ),
           *pDenom  = pNode->GetSubNode( 2 );
    Append( "{ " );
    LineToText( pNum );
    Append( "over" );
    LineToText( pDenom );
    Append( "} " );
}

void SmNodeToTextVisitor::Visit( SmBinDiagonalNode* pNode )
{
    SmNode *pLeftOperand  = pNode->GetSubNode( 0 ),
           *pRightOperand = pNode->GetSubNode( 1 );
    Append( "{ " );
    LineToText( pLeftOperand );
    Separate( );
    Append( "wideslash " );
    LineToText( pRightOperand );
    Append( "} " );
}

void SmNodeToTextVisitor::Visit( SmSubSupNode* pNode )
{
    LineToText( pNode->GetBody( ) );
    SmNode *pChild;
    if( ( pChild = pNode->GetSubSup( LSUP ) ) ) {
        Separate( );
        Append( "lsup " );
        LineToText( pChild );
    }
    if( ( pChild = pNode->GetSubSup( LSUB ) ) ) {
        Separate( );
        Append( "lsub " );
        LineToText( pChild );
    }
    if( ( pChild = pNode->GetSubSup( RSUP ) ) ) {
        Separate( );
        Append( "^ " );
        LineToText( pChild );
    }
    if( ( pChild = pNode->GetSubSup( RSUB ) ) ) {
        Separate( );
        Append( "_ " );
        LineToText( pChild );
    }
    if( ( pChild = pNode->GetSubSup( CSUP ) ) ) {
        Separate( );
        if (pNode->IsUseLimits())
            Append( "to " );
        else
            Append( "csup " );
        LineToText( pChild );
    }
    if( ( pChild = pNode->GetSubSup( CSUB ) ) ) {
        Separate( );
        if (pNode->IsUseLimits())
            Append( "from " );
        else
            Append( "csub " );
        LineToText( pChild );
    }
}

void SmNodeToTextVisitor::Visit( SmMatrixNode* pNode )
{
    Append( "matrix{" );
    for ( sal_uInt16 i = 0; i < pNode->GetNumRows( ); i++ ) {
        for ( sal_uInt16 j = 0; j < pNode->GetNumCols( ); j++ ) {
            SmNode* pSubNode = pNode->GetSubNode( i * pNode->GetNumCols( ) + j );
            Separate( );
            pSubNode->Accept( this );
            Separate( );
            if( j != pNode->GetNumCols( ) - 1 )
                Append( "#" );
        }
        Separate( );
        if( i != pNode->GetNumRows( ) - 1 )
            Append( "##" );
    }
    Append( "} " );
}

void SmNodeToTextVisitor::Visit( SmPlaceNode* )
{
    Append( "<?>" );
}

void SmNodeToTextVisitor::Visit( SmTextNode* pNode )
{
    //TODO: This method might need improvements, see SmTextNode::CreateTextFromNode
    if( pNode->GetToken( ).eType == TTEXT )
        Append( "\"" );
    Append( pNode->GetText( ) );
    if( pNode->GetToken( ).eType == TTEXT )
        Append( "\"" );
}

void SmNodeToTextVisitor::Visit( SmSpecialNode* pNode )
{
    Append( pNode->GetToken( ).aText );
}

void SmNodeToTextVisitor::Visit( SmGlyphSpecialNode* pNode )
{
    if( pNode->GetToken( ).eType == TBOPER )
        Append( "boper " );
    else
        Append( "uoper " );
    Append( pNode->GetToken( ).aText );
}

void SmNodeToTextVisitor::Visit( SmMathSymbolNode* pNode )
{
    Append( pNode->GetToken( ).aText );
}

void SmNodeToTextVisitor::Visit( SmBlankNode* pNode )
{
    Append( pNode->GetToken( ).aText );
}

void SmNodeToTextVisitor::Visit( SmErrorNode* )
{
}

void SmNodeToTextVisitor::Visit( SmLineNode* pNode )
{
    SmNodeIterator it( pNode );
    while( it.Next( ) ){
        Separate( );
        it->Accept( this );
    }
}

void SmNodeToTextVisitor::Visit( SmExpressionNode* pNode )
{
    bool bracketsNeeded = pNode->GetNumSubNodes() != 1 || pNode->GetSubNode(0)->GetType() == NBINHOR;
    if (!bracketsNeeded)
    {
        const SmNode *pParent = pNode->GetParent();
        // nested subsups
        bracketsNeeded =
            pParent && pParent->GetType() == NSUBSUP &&
            pNode->GetNumSubNodes() == 1 &&
            pNode->GetSubNode(0)->GetType() == NSUBSUP;
    }

    if (bracketsNeeded) {
        Append( "{ " );
    }
    SmNodeIterator it( pNode );
    while( it.Next( ) ) {
        it->Accept( this );
        Separate( );
    }
    if (bracketsNeeded) {
        Append( "} " );
    }
}

void SmNodeToTextVisitor::Visit( SmPolyLineNode* )
{
}

void SmNodeToTextVisitor::Visit( SmRootNode* pNode )
{
    SmNode *pExtra   = pNode->GetSubNode( 0 ),
           *pBody    = pNode->GetSubNode( 2 );
    if( pExtra ) {
        Append( "nroot" );
        LineToText( pExtra );
    } else
        Append( "sqrt" );
    LineToText( pBody );
}

void SmNodeToTextVisitor::Visit( SmRootSymbolNode* )
{
}

void SmNodeToTextVisitor::Visit( SmDynIntegralNode* pNode )
{
    SmNode *pBody    = pNode->Body();
    Append( "intd" );
    LineToText( pBody );
}

void SmNodeToTextVisitor::Visit( SmDynIntegralSymbolNode* )
{
}

void SmNodeToTextVisitor::Visit( SmRectangleNode* )
{
}

void SmNodeToTextVisitor::Visit( SmVerticalBraceNode* pNode )
{
    SmNode *pBody   = pNode->GetSubNode( 0 ),
           *pScript = pNode->GetSubNode( 2 );
    LineToText( pBody );
    Append( pNode->GetToken( ).aText );
    LineToText( pScript );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
