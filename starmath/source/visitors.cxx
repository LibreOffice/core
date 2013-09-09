/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "visitors.hxx"
#include "cursor.hxx"

///////////////////////////////////// SmVisitorTest /////////////////////////////////////

void SmVisitorTest::Visit( SmTableNode* pNode )
{
    assert( pNode->GetType( ) == NTABLE );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmBraceNode* pNode )
{
    assert( pNode->GetType( ) == NBRACE );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmBracebodyNode* pNode )
{
    assert( pNode->GetType( ) == NBRACEBODY );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmOperNode* pNode )
{
    assert( pNode->GetType( ) == NOPER );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmAlignNode* pNode )
{
    assert( pNode->GetType( ) == NALIGN );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmAttributNode* pNode )
{
    assert( pNode->GetType( ) == NATTRIBUT );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmFontNode* pNode )
{
    assert( pNode->GetType( ) == NFONT );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmUnHorNode* pNode )
{
    assert( pNode->GetType( ) == NUNHOR );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmBinHorNode* pNode )
{
    assert( pNode->GetType( ) == NBINHOR );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmBinVerNode* pNode )
{
    assert( pNode->GetType( ) == NBINVER );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmBinDiagonalNode* pNode )
{
    assert( pNode->GetType( ) == NBINDIAGONAL );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmSubSupNode* pNode )
{
    assert( pNode->GetType( ) == NSUBSUP );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmMatrixNode* pNode )
{
    assert( pNode->GetType( ) == NMATRIX );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmPlaceNode* pNode )
{
    assert( pNode->GetType( ) == NPLACE );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmTextNode* pNode )
{
    assert( pNode->GetType( ) == NTEXT );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmSpecialNode* pNode )
{
    assert( pNode->GetType( ) == NSPECIAL );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmGlyphSpecialNode* pNode )
{
    assert( pNode->GetType( ) == NGLYPH_SPECIAL );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmMathSymbolNode* pNode )
{
    assert( pNode->GetType( ) == NMATH || pNode->GetType( ) == NMATHIDENT );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmBlankNode* pNode )
{
    assert( pNode->GetType( ) == NBLANK );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmErrorNode* pNode )
{
    assert( pNode->GetType( ) == NERROR );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmLineNode* pNode )
{
    assert( pNode->GetType( ) == NLINE );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmExpressionNode* pNode )
{
    assert( pNode->GetType( ) == NEXPRESSION );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmPolyLineNode* pNode )
{
    assert( pNode->GetType( ) == NPOLYLINE );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmRootNode* pNode )
{
    assert( pNode->GetType( ) == NROOT );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmRootSymbolNode* pNode )
{
    assert( pNode->GetType( ) == NROOTSYMBOL );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmRectangleNode* pNode )
{
    assert( pNode->GetType( ) == NRECTANGLE );
    VisitChildren( pNode );
}

void SmVisitorTest::Visit( SmVerticalBraceNode* pNode )
{
    assert( pNode->GetType( ) == NVERTICAL_BRACE );
    VisitChildren( pNode );
}

void SmVisitorTest::VisitChildren( SmNode* pNode )
{
    SmNodeIterator it( pNode );
    while( it.Next( ) )
        it->Accept( this );
}

/////////////////////////////// SmDefaultingVisitor ////////////////////////////////

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

void SmDefaultingVisitor::Visit( SmRectangleNode* pNode )
{
    DefaultVisit( pNode );
}

void SmDefaultingVisitor::Visit( SmVerticalBraceNode* pNode )
{
    DefaultVisit( pNode );
}

/////////////////////////////// SmCaretDrawingVisitor ////////////////////////////////

SmCaretDrawingVisitor::SmCaretDrawingVisitor( OutputDevice& rDevice,
                                             SmCaretPos position,
                                             Point offset,
                                             bool caretVisible )
 : rDev( rDevice )
{
    pos = position;
    Offset = offset;
    isCaretVisible = caretVisible;
    SAL_WARN_IF( !position.IsValid(), "starmath", "Cannot draw invalid position!" );
    if( !position.IsValid( ) )
        return;

    //Save device state
    rDev.Push( PUSH_FONT | PUSH_MAPMODE | PUSH_LINECOLOR | PUSH_FILLCOLOR | PUSH_TEXTCOLOR );

    pos.pSelectedNode->Accept( this );
    //Restore device state
    rDev.Pop( );
}

void SmCaretDrawingVisitor::Visit( SmTextNode* pNode )
{
    long i = pos.Index;

    rDev.SetFont( pNode->GetFont( ) );

    //Find the line
    SmNode* pLine = SmCursor::FindTopMostNodeInLine( pNode );

    //Find coordinates
    long left = pNode->GetLeft( ) + rDev.GetTextWidth( pNode->GetText( ), 0, i ) + Offset.X( );
    long top = pLine->GetTop( ) + Offset.Y( );
    long height = pLine->GetHeight( );
    long left_line = pLine->GetLeft( ) + Offset.X( );
    long right_line = pLine->GetRight( ) + Offset.X( );

    //Set color
    rDev.SetLineColor( Color( COL_BLACK ) );

    if ( isCaretVisible ) {
        //Draw vertical line
        Point p1( left, top );
        Point p2( left, top + height );
        rDev.DrawLine( p1, p2 );
    }

    //Underline the line
    Point pLeft( left_line, top + height );
    Point pRight( right_line, top + height );
    rDev.DrawLine( pLeft, pRight );
}

void SmCaretDrawingVisitor::DefaultVisit( SmNode* pNode )
{
    rDev.SetLineColor( Color( COL_BLACK ) );

    //Find the line
    SmNode* pLine = SmCursor::FindTopMostNodeInLine( pNode );

    //Find coordinates
    long left = pNode->GetLeft( ) + Offset.X( ) + ( pos.Index == 1 ? pNode->GetWidth( ) : 0 );
    long top = pLine->GetTop( ) + Offset.Y( );
    long height = pLine->GetHeight( );
    long left_line = pLine->GetLeft( ) + Offset.X( );
    long right_line = pLine->GetRight( ) + Offset.X( );

    //Set color
    rDev.SetLineColor( Color( COL_BLACK ) );

    if ( isCaretVisible ) {
        //Draw vertical line
        Point p1( left, top );
        Point p2( left, top + height );
        rDev.DrawLine( p1, p2 );
    }

    //Underline the line
    Point pLeft( left_line, top + height );
    Point pRight( right_line, top + height );
    rDev.DrawLine( pLeft, pRight );
}

/////////////////////////////// SmCaretPos2LineVisitor ////////////////////////////////

void SmCaretPos2LineVisitor::Visit( SmTextNode* pNode )
{
    //Save device state
    pDev->Push( PUSH_FONT | PUSH_TEXTCOLOR );

    long i = pos.Index;

    pDev->SetFont( pNode->GetFont( ) );

    //Find coordinates
    long left = pNode->GetLeft( ) + pDev->GetTextWidth( pNode->GetText( ), 0, i );
    long top = pNode->GetTop( );
    long height = pNode->GetHeight( );

    line = SmCaretLine( left, top, height );

    //Restore device state
    pDev->Pop( );
}

void SmCaretPos2LineVisitor::DefaultVisit( SmNode* pNode )
{
    //Vertical line ( code from SmCaretDrawingVisitor )
    Point p1 = pNode->GetTopLeft( );
    if( pos.Index == 1 )
        p1.Move( pNode->GetWidth( ), 0 );

    line = SmCaretLine( p1.X( ), p1.Y( ), pNode->GetHeight( ) );
}

/////////////////////////////// Nasty temporary device!!! ////////////////////////////////

#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include <rtl/math.hxx>
#include <tools/color.hxx>
#include <vcl/metric.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/outdev.hxx>
#include <sfx2/module.hxx>
#include "symbol.hxx"
#include "smmod.hxx"

class SmTmpDevice2
{
    OutputDevice  &rOutDev;

    // disallow use of copy-constructor and assignment-operator
    SmTmpDevice2( const SmTmpDevice2 &rTmpDev );
    SmTmpDevice2 & operator = ( const SmTmpDevice2 &rTmpDev );

    Color   Impl_GetColor( const Color& rColor );

public:
    SmTmpDevice2( OutputDevice &rTheDev, bool bUseMap100th_mm );
    ~SmTmpDevice2( )  { rOutDev.Pop( ); }

    void SetFont( const Font &rNewFont );

    void SetLineColor( const Color& rColor )    { rOutDev.SetLineColor( Impl_GetColor( rColor ) ); }
    void SetFillColor( const Color& rColor )    { rOutDev.SetFillColor( Impl_GetColor( rColor ) ); }
    void SetTextColor( const Color& rColor )    { rOutDev.SetTextColor( Impl_GetColor( rColor ) ); }

    operator OutputDevice & ( ) const { return rOutDev; }
};

SmTmpDevice2::SmTmpDevice2( OutputDevice &rTheDev, bool bUseMap100th_mm ) :
    rOutDev( rTheDev )
{
    rOutDev.Push( PUSH_FONT | PUSH_MAPMODE |
                  PUSH_LINECOLOR | PUSH_FILLCOLOR | PUSH_TEXTCOLOR );
    if ( bUseMap100th_mm  &&  MAP_100TH_MM != rOutDev.GetMapMode( ).GetMapUnit( ) )
    {
        SAL_WARN("starmath", "incorrect MapMode?");
        rOutDev.SetMapMode( MAP_100TH_MM );     //format for 100% always
    }
}

Color SmTmpDevice2::Impl_GetColor( const Color& rColor )
{
    ColorData nNewCol = rColor.GetColor( );
    if ( COL_AUTO == nNewCol )
    {
        if ( OUTDEV_PRINTER == rOutDev.GetOutDevType( ) )
            nNewCol = COL_BLACK;
        else
        {
            Color aBgCol( rOutDev.GetBackground( ).GetColor( ) );
            if ( OUTDEV_WINDOW == rOutDev.GetOutDevType( ) )
                aBgCol = ( ( Window & ) rOutDev ).GetDisplayBackground( ).GetColor( );

            nNewCol = SM_MOD( )->GetColorConfig( ).GetColorValue( svtools::FONTCOLOR ).nColor;

            Color aTmpColor( nNewCol );
            if ( aBgCol.IsDark( ) && aTmpColor.IsDark( ) )
                nNewCol = COL_WHITE;
            else if ( aBgCol.IsBright( ) && aTmpColor.IsBright( ) )
                nNewCol = COL_BLACK;
        }
    }
    return Color( nNewCol );
}

void SmTmpDevice2::SetFont( const Font &rNewFont )
{
    rOutDev.SetFont( rNewFont );
    rOutDev.SetTextColor( Impl_GetColor( rNewFont.GetColor( ) ) );
}

/////////////////////////////// SmDrawingVisitor ////////////////////////////////

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

    SmTmpDevice2  aTmpDev( ( OutputDevice & ) rDev, true );
    aTmpDev.SetFillColor( pNode->GetFont( ).GetColor( ) );
    rDev.SetLineColor( );
    aTmpDev.SetFont( pNode->GetFont( ) );

    // since the width is always unscaled it corresponds ot the _original_
    // _unscaled_ font height to be used, we use that to calculate the
    // bar height. Thus it is independent of the arguments height.
    // ( see display of sqrt QQQ versus sqrt stack{Q#Q#Q#Q} )
    long nBarHeight = pNode->GetWidth( ) * 7L / 100L;
    long nBarWidth = pNode->GetBodyWidth( ) + pNode->GetBorderWidth( );
    Point aBarOffset( pNode->GetWidth( ), +pNode->GetBorderWidth( ) );
    Point aBarPos( Position + aBarOffset );

    Rectangle  aBar( aBarPos, Size( nBarWidth, nBarHeight ) );
    //! avoid GROWING AND SHRINKING of drawn rectangle when constantly
    //! increasing zoomfactor.
    //  This is done by shifting its output-position to a point that
    //  corresponds exactly to a pixel on the output device.
    Point  aDrawPos( rDev.PixelToLogic( rDev.LogicToPixel( aBar.TopLeft( ) ) ) );
    aBar.SetPos( aDrawPos );

    rDev.DrawRect( aBar );
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
          aPos ( Position + aOffset );
    pNode->GetPolygon( ).Move( aPos.X( ), aPos.Y( ) );    //Works because Polygon wraps a pointer

    SmTmpDevice2  aTmpDev ( ( OutputDevice & ) rDev, false );
    aTmpDev.SetLineColor( pNode->GetFont( ).GetColor( ) );

    rDev.DrawPolyLine( pNode->GetPolygon( ), aInfo );
}

void SmDrawingVisitor::Visit( SmRectangleNode* pNode )
{
    if ( pNode->IsPhantom( ) )
        return;

    SmTmpDevice2  aTmpDev ( ( OutputDevice & ) rDev, false );
    aTmpDev.SetFillColor( pNode->GetFont( ).GetColor( ) );
    rDev.SetLineColor( );
    aTmpDev.SetFont( pNode->GetFont( ) );

    sal_uLong  nTmpBorderWidth = pNode->GetFont( ).GetBorderWidth( );

    // get rectangle and remove borderspace
    Rectangle  aTmp ( pNode->AsRectangle( ) + Position - pNode->GetTopLeft( ) );
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
    Point  aPos ( rDev.PixelToLogic( rDev.LogicToPixel( aTmp.TopLeft( ) ) ) );
    aTmp.SetPos( aPos );

    rDev.DrawRect( aTmp );
}

void SmDrawingVisitor::DrawTextNode( SmTextNode* pNode )
{
    if ( pNode->IsPhantom() || pNode->GetText().isEmpty() || pNode->GetText()[0] == '\0' )
        return;

    SmTmpDevice2  aTmpDev ( ( OutputDevice & ) rDev, false );
    aTmpDev.SetFont( pNode->GetFont( ) );

    Point  aPos ( Position );
    aPos.Y( ) += pNode->GetBaselineOffset( );
    // auf Pixelkoordinaten runden
    aPos = rDev.PixelToLogic( rDev.LogicToPixel( aPos ) );

    rDev.DrawStretchText( aPos, pNode->GetWidth( ), pNode->GetText( ) );
}

void SmDrawingVisitor::DrawSpecialNode( SmSpecialNode* pNode )
{
    //! since this chars might come from any font, that we may not have
    //! set to ALIGN_BASELINE yet, we do it now.
    pNode->GetFont( ).SetAlign( ALIGN_BASELINE );

    DrawTextNode( pNode );
}

void SmDrawingVisitor::DrawChildren( SmNode* pNode )
{
    if ( pNode->IsPhantom( ) )
        return;

    Point rPosition = Position;

    SmNodeIterator it( pNode );
    while( it.Next( ) )
    {
        Point  aOffset ( it->GetTopLeft( ) - pNode->GetTopLeft( ) );
        Position = rPosition + aOffset;
        it->Accept( this );
    }
}

/////////////////////////////// SmSetSelectionVisitor ////////////////////////////////

SmSetSelectionVisitor::SmSetSelectionVisitor( SmCaretPos startPos, SmCaretPos endPos, SmNode* pTree) {
    StartPos    = startPos;
    EndPos      = endPos;
    IsSelecting = false;

    //Assume that pTree is a SmTableNode
    SAL_WARN_IF(pTree->GetType() != NTABLE, "starmath", "pTree should be a SmTableNode!");
    //Visit root node, this is special as this node cannot be selected, but its children can!
    if(pTree->GetType() == NTABLE){
        //Change state if StartPos is in front of this node
        if( StartPos.pSelectedNode == pTree && StartPos.Index == 0 )
            IsSelecting = !IsSelecting;
        //Change state if EndPos is in front of this node
        if( EndPos.pSelectedNode == pTree && EndPos.Index == 0 )
            IsSelecting = !IsSelecting;
        SAL_WARN_IF(IsSelecting, "starmath", "Caret positions needed to set IsSelecting about, shouldn't be possible!");

        //Visit lines
        SmNodeIterator it( pTree );
        while( it.Next( ) ) {
            it->Accept( this );
            //If we started a selection in this line and it haven't ended, we do that now!
            if(IsSelecting) {
                IsSelecting = false;
                SetSelectedOnAll(it.Current(), true);
                //Set StartPos and EndPos to invalid positions, this ensures that an unused
                //start or end (because we forced end above), doesn't start a new selection.
                StartPos = EndPos = SmCaretPos();
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
    //Change state if StartPos is in front of this node
    if( StartPos.pSelectedNode == pNode && StartPos.Index == 0 )
        IsSelecting = !IsSelecting;
    //Change state if EndPos is in front of this node
    if( EndPos.pSelectedNode == pNode && EndPos.Index == 0 )
        IsSelecting = !IsSelecting;

    //Cache current state
    bool WasSelecting = IsSelecting;
    bool ChangedState = false;

    //Set selected
    pNode->SetSelected( IsSelecting );

    //Visit children
    SmNodeIterator it( pNode );
    while( it.Next( ) )
    {
        it->Accept( this );
        ChangedState = ( WasSelecting != IsSelecting ) || ChangedState;
    }

    //If state changed
    if( ChangedState )
    {
        //Select this node and all of its children
        //(Make exception for SmBracebodyNode)
        if( pNode->GetType() != NBRACEBODY ||
            !pNode->GetParent() ||
            pNode->GetParent()->GetType() != NBRACE )
            SetSelectedOnAll( pNode, true );
        else
            SetSelectedOnAll( pNode->GetParent(), true );
        /* If the equation is:      sqrt{2 + 4} + 5
         * And the selection is:    sqrt{2 + [4} +] 5
         *      Where [ denotes StartPos and ] denotes EndPos
         * Then the sqrt node should be selected, so that the
         * effective selection is:  [sqrt{2 + 4} +] 5
         * The same is the case if we swap StartPos and EndPos.
         */
    }

    //Change state if StartPos is after this node
    if( StartPos.pSelectedNode == pNode && StartPos.Index == 1 )
    {
        IsSelecting = !IsSelecting;
    }
    //Change state if EndPos is after of this node
    if( EndPos.pSelectedNode == pNode && EndPos.Index == 1 )
    {
        IsSelecting = !IsSelecting;
    }
}

void SmSetSelectionVisitor::VisitCompositionNode( SmNode* pNode ) {
    //Change state if StartPos is in front of this node
    if( StartPos.pSelectedNode == pNode && StartPos.Index == 0 )
        IsSelecting = !IsSelecting;
    //Change state if EndPos is in front of this node
    if( EndPos.pSelectedNode == pNode && EndPos.Index == 0 )
        IsSelecting = !IsSelecting;

    //Cache current state
    bool WasSelecting = IsSelecting;

    //Visit children
    SmNodeIterator it( pNode );
    while( it.Next( ) )
        it->Accept( this );

    //Set selected, if everything was selected
    pNode->SetSelected( WasSelecting && IsSelecting );

    //Change state if StartPos is after this node
    if( StartPos.pSelectedNode == pNode && StartPos.Index == 1 )
        IsSelecting = !IsSelecting;
    //Change state if EndPos is after of this node
    if( EndPos.pSelectedNode == pNode && EndPos.Index == 1 )
        IsSelecting = !IsSelecting;
}

void SmSetSelectionVisitor::Visit( SmTextNode* pNode ) {
    long    i1 = -1,
            i2 = -1;
    if( StartPos.pSelectedNode == pNode )
        i1 = StartPos.Index;
    if( EndPos.pSelectedNode == pNode )
        i2 = EndPos.Index;

    long start, end;
    pNode->SetSelected( true );
    if( i1 != -1 && i2 != -1 ) {
        start = i1 < i2 ? i1 : i2; //MIN
        end   = i1 > i2 ? i1 : i2; //MAX
    } else if( IsSelecting && i1 != -1 ) {
        start = 0;
        end = i1;
        IsSelecting = false;
    } else if( IsSelecting && i2 != -1 ) {
        start = 0;
        end = i2;
        IsSelecting = false;
    } else if( !IsSelecting && i1 != -1 ) {
        start = i1;
        end = pNode->GetText().getLength();
        IsSelecting = true;
    } else if( !IsSelecting && i2 != -1 ) {
        start = i2;
        end = pNode->GetText().getLength();
        IsSelecting = true;
    } else if( IsSelecting ) {
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

/////////////////////////////// SmCaretPosGraphBuildingVisitor ////////////////////////////////

SmCaretPosGraphBuildingVisitor::SmCaretPosGraphBuildingVisitor( SmNode* pRootNode ) {
    pRightMost  = NULL;
    pGraph = new SmCaretPosGraph( );
    //pRootNode should always be a table
    SAL_WARN_IF( pRootNode->GetType( ) != NTABLE, "starmath", "pRootNode must be a table node");
    //Handle the special case where NTABLE is used a rootnode
    if( pRootNode->GetType( ) == NTABLE ){
        //Children are SmLineNodes
        //Or so I thought... Aparently, the children can be instances of SmExpression
        //especially if there's a error in the formula... So he we go, a simple work around.
        SmNodeIterator it( pRootNode );
        while( it.Next( ) ){
            //There's a special invariant between this method and the Visit( SmLineNode* )
            //Usually pRightMost may not be NULL, to avoid this pRightMost should here be
            //set to a new SmCaretPos in front of it.Current( ), however, if it.Current( ) is
            //an instance of SmLineNode we let SmLineNode create this position in front of
            //the visual line.
            //The argument for doing this is that we now don't have to worry about SmLineNode
            //being a visual line composition node. Thus, no need for yet another special case
            //in SmCursor::IsLineCompositionNode and everywhere this method is used.
            //if( it->GetType( ) != NLINE )
                pRightMost = pGraph->Add( SmCaretPos( it.Current( ), 0 ) );
            it->Accept( this );
        }
    }else
        pRootNode->Accept(this);
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
    SmCaretPosGraphEntry *left  = pRightMost,
                         *right = pGraph->Add( SmCaretPos( pNode, 1) );
    bool bIsFirst = true;
    SmNodeIterator it( pNode );
    while( it.Next() ){
        pRightMost = pGraph->Add( SmCaretPos( it.Current(), 0 ), left);
        if(bIsFirst)
            left->SetRight(pRightMost);
        it->Accept( this );
        pRightMost->SetRight(right);
        if(bIsFirst)
            right->SetLeft(pRightMost);
        bIsFirst = false;
    }
    pRightMost = right;
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

    left = pRightMost;
    SAL_WARN_IF( !pRightMost, "starmath", "pRightMost shouldn't be NULL here!" );

    //Create bodyLeft
    SAL_WARN_IF( !pNode->GetBody(), "starmath", "SmSubSupNode Doesn't have a body!" );
    bodyLeft = pGraph->Add( SmCaretPos( pNode->GetBody( ), 0 ), left );
    left->SetRight( bodyLeft ); //TODO: Don't make this if LSUP or LSUB are NULL ( not sure??? )

    //Create right
    right = pGraph->Add( SmCaretPos( pNode, 1 ) );

    //Visit the body, to get bodyRight
    pRightMost = bodyLeft;
    pNode->GetBody( )->Accept( this );
    bodyRight = pRightMost;
    bodyRight->SetRight( right );
    right->SetLeft( bodyRight );

    SmNode* pChild;
    //If there's an LSUP
    if( ( pChild = pNode->GetSubSup( LSUP ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = pGraph->Add( SmCaretPos( pChild, 0 ), left );

        pRightMost = cLeft;
        pChild->Accept( this );

        pRightMost->SetRight( bodyLeft );
    }
    //If there's an LSUB
    if( ( pChild = pNode->GetSubSup( LSUB ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = pGraph->Add( SmCaretPos( pChild, 0 ), left );

        pRightMost = cLeft;
        pChild->Accept( this );

        pRightMost->SetRight( bodyLeft );
    }
    //If there's an CSUP
    if( ( pChild = pNode->GetSubSup( CSUP ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = pGraph->Add( SmCaretPos( pChild, 0 ), left );

        pRightMost = cLeft;
        pChild->Accept( this );

        pRightMost->SetRight( right );
    }
    //If there's an CSUB
    if( ( pChild = pNode->GetSubSup( CSUB ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = pGraph->Add( SmCaretPos( pChild, 0 ), left );

        pRightMost = cLeft;
        pChild->Accept( this );

        pRightMost->SetRight( right );
    }
    //If there's an RSUP
    if( ( pChild = pNode->GetSubSup( RSUP ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = pGraph->Add( SmCaretPos( pChild, 0 ), bodyRight );

        pRightMost = cLeft;
        pChild->Accept( this );

        pRightMost->SetRight( right );
    }
    //If there's an RSUB
    if( ( pChild = pNode->GetSubSup( RSUB ) ) ){
        SmCaretPosGraphEntry *cLeft; //Child left
        cLeft = pGraph->Add( SmCaretPos( pChild, 0 ), bodyRight );

        pRightMost = cLeft;
        pChild->Accept( this );

        pRightMost->SetRight( right );
    }

    //Set return parameters
    pRightMost = right;
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

    SmCaretPosGraphEntry *left = pRightMost,
                         *bodyLeft,
                         *bodyRight,
                         *right;
    //Create body left
    bodyLeft = pGraph->Add( SmCaretPos( pBody, 0 ), left );
    left->SetRight( bodyLeft );

    //Visit body, get bodyRight
    pRightMost = bodyLeft;
    pBody->Accept( this );
    bodyRight = pRightMost;

    //Create right
    right = pGraph->Add( SmCaretPos( pNode, 1 ), bodyRight );
    bodyRight->SetRight( right );

    //Get subsup pNode if any
    SmSubSupNode* pSubSup = pOper->GetType( ) == NSUBSUP ? ( SmSubSupNode* )pOper : NULL;

    SmNode* pChild;
    SmCaretPosGraphEntry *childLeft;
    if( pSubSup && ( pChild = pSubSup->GetSubSup( LSUP ) ) ) {
        //Create position in front of pChild
        childLeft = pGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        pRightMost = childLeft;
        pChild->Accept( this );
        //Set right on pRightMost from pChild
        pRightMost->SetRight( bodyLeft );
    }
    if( pSubSup && ( pChild = pSubSup->GetSubSup( LSUB ) ) ) {
        //Create position in front of pChild
        childLeft = pGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        pRightMost = childLeft;
        pChild->Accept( this );
        //Set right on pRightMost from pChild
        pRightMost->SetRight( bodyLeft );
    }
    if( pSubSup && ( pChild = pSubSup->GetSubSup( CSUP ) ) ) {//TO
        //Create position in front of pChild
        childLeft = pGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        pRightMost = childLeft;
        pChild->Accept( this );
        //Set right on pRightMost from pChild
        pRightMost->SetRight( bodyLeft );
    }
    if( pSubSup && ( pChild = pSubSup->GetSubSup( CSUB ) ) ) { //FROM
        //Create position in front of pChild
        childLeft = pGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        pRightMost = childLeft;
        pChild->Accept( this );
        //Set right on pRightMost from pChild
        pRightMost->SetRight( bodyLeft );
    }
    if( pSubSup && ( pChild = pSubSup->GetSubSup( RSUP ) ) ) {
        //Create position in front of pChild
        childLeft = pGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        pRightMost = childLeft;
        pChild->Accept( this );
        //Set right on pRightMost from pChild
        pRightMost->SetRight( bodyLeft );
    }
    if( pSubSup && ( pChild = pSubSup->GetSubSup( RSUB ) ) ) {
        //Create position in front of pChild
        childLeft = pGraph->Add( SmCaretPos( pChild, 0 ), left );
        //Visit pChild
        pRightMost = childLeft;
        pChild->Accept( this );
        //Set right on pRightMost from pChild
        pRightMost->SetRight( bodyLeft );
    }

    //Return right
    pRightMost = right;
}

void SmCaretPosGraphBuildingVisitor::Visit( SmMatrixNode* pNode )
{
    SmCaretPosGraphEntry *left  = pRightMost,
                         *right = pGraph->Add( SmCaretPos( pNode, 1 ) );

    for ( sal_uInt16 i = 0;  i < pNode->GetNumRows( ); i++ ) {
        SmCaretPosGraphEntry* r = left;
        for ( sal_uInt16 j = 0;  j < pNode->GetNumCols( ); j++ ){
            SmNode* pSubNode = pNode->GetSubNode( i * pNode->GetNumCols( ) + j );

            pRightMost = pGraph->Add( SmCaretPos( pSubNode, 0 ), r );
            if( j != 0 || ( pNode->GetNumRows( ) - 1 ) / 2 == i )
                r->SetRight( pRightMost );

            pSubNode->Accept( this );

            r = pRightMost;
        }
        pRightMost->SetRight( right );
        if( ( pNode->GetNumRows( ) - 1 ) / 2 == i )
            right->SetLeft( pRightMost );
    }

    pRightMost = right;
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
        SmCaretPosGraphEntry* pRight = pRightMost;
        pRightMost = pGraph->Add( SmCaretPos( pNode, i ), pRight );
        pRight->SetRight( pRightMost );
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

    //Set left
    left = pRightMost;
    SAL_WARN_IF( !pRightMost, "starmath", "There must be a position in front of this" );

    //Create right
    right = pGraph->Add( SmCaretPos( pNode, 1 ) );

    //Create numLeft
    numLeft = pGraph->Add( SmCaretPos( pNum, 0 ), left );
    left->SetRight( numLeft );

    //Visit pNum
    pRightMost = numLeft;
    pNum->Accept( this );
    pRightMost->SetRight( right );
    right->SetLeft( pRightMost );

    //Create denomLeft
    denomLeft = pGraph->Add( SmCaretPos( pDenom, 0 ), left );

    //Visit pDenom
    pRightMost = denomLeft;
    pDenom->Accept( this );
    pRightMost->SetRight( right );

    //Set return parameter
    pRightMost = right;
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

    left = pRightMost;

    //Create right
    right = pGraph->Add( SmCaretPos( pNode, 1 ) );

    //Create bodyLeft
    bodyLeft = pGraph->Add( SmCaretPos( pBody, 0 ), left );
    left->SetRight( bodyLeft );
    pRightMost = bodyLeft;
    pBody->Accept( this );
    pRightMost->SetRight( right );
    right->SetLeft( pRightMost );

    //Create script
    scriptLeft = pGraph->Add( SmCaretPos( pScript, 0 ), left );
    pRightMost = scriptLeft;
    pScript->Accept( this );
    pRightMost->SetRight( right );

    //Set return value
    pRightMost = right;
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
    left = pRightMost;

    //Create right
    right = pGraph->Add( SmCaretPos( pNode, 1 ) );

    //Create left A
    leftA = pGraph->Add( SmCaretPos( A, 0 ), left );
    left->SetRight( leftA );

    //Visit A
    pRightMost = leftA;
    A->Accept( this );
    rightA = pRightMost;

    //Create left B
    leftB = pGraph->Add( SmCaretPos( B, 0 ), rightA );
    rightA->SetRight( leftB );

    //Visit B
    pRightMost = leftB;
    B->Accept( this );
    pRightMost->SetRight( right );
    right->SetLeft( pRightMost );

    //Set return value
    pRightMost = right;
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
 * n8 [label="SmMathSymbolNode: ∣"];
 * n5 -> n9 [label="2"];
 * n9 [label="SmExpressionNode"];
 * n9 -> n10 [label="0"];
 * n10 [label="SmBinHorNode"];
 * n10 -> n11 [label="0"];
 * n11 [label="SmTextNode: i"];
 * n10 -> n12 [label="1"];
 * n12 [label="SmMathSymbolNode: ∈"];
 * n10 -> n13 [label="2"];
 * n13 [label="SmMathSymbolNode: ℤ"];
 * n3 -> n14 [label="2"];
 * n14 [label="SmMathSymbolNode: }"];
 * }
 * \enddot
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmBracebodyNode* pNode )
{
    SmNodeIterator it( pNode );
    while( it.Next( ) ) {
        SmCaretPosGraphEntry* pStart = pGraph->Add( SmCaretPos( it.Current(), 0), pRightMost );
        pRightMost->SetRight( pStart );
        pRightMost = pStart;
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
    SAL_WARN_IF( !pBody, "starmath", "pBody cannot be NULL" );

    SmCaretPosGraphEntry  *left,
                        *right,
                        *bodyLeft,
                        *bodyRight;

    //Get left and save it
    SAL_WARN_IF( !pRightMost, "starmath", "There must be a position in front of this" );
    left = pRightMost;

    //Create body left
    bodyLeft = pGraph->Add( SmCaretPos( pBody, 0 ), left );
    left->SetRight( bodyLeft );

    //Create right
    right = pGraph->Add( SmCaretPos( pNode, 1 ) );

    //Visit body
    pRightMost = bodyLeft;
    pBody->Accept( this );
    bodyRight = pRightMost;
    bodyRight->SetRight( right );
    right->SetLeft( bodyRight );

    //Visit pExtra
    if( pExtra ){
        pRightMost = pGraph->Add( SmCaretPos( pExtra, 0 ), left );
        pExtra->Accept( this );
        pRightMost->SetRight( bodyLeft );
    }

    pRightMost = right;
}

/** Build SmCaretPosGraph for SmPlaceNode
 * Consider this a single character.
 */
void SmCaretPosGraphBuildingVisitor::Visit( SmPlaceNode* pNode )
{
    SmCaretPosGraphEntry* right = pGraph->Add( SmCaretPos( pNode, 1 ), pRightMost );
    pRightMost->SetRight( right );
    pRightMost = right;
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
    SmCaretPosGraphEntry* right = pGraph->Add( SmCaretPos( pNode, 1 ), pRightMost );
    pRightMost->SetRight( right );
    pRightMost = right;
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

    SmCaretPosGraphEntry  *left = pRightMost,
                        *right = pGraph->Add( SmCaretPos( pNode, 1 ) );

    if( pBody->GetType() != NBRACEBODY ) {
        pRightMost = pGraph->Add( SmCaretPos( pBody, 0 ), left );
        left->SetRight( pRightMost );
    }else
        pRightMost = left;

    pBody->Accept( this );
    pRightMost->SetRight( right );
    right->SetLeft( pRightMost );

    pRightMost = right;
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

    SmCaretPosGraphEntry  *left = pRightMost,
                        *attrLeft,
                        *bodyLeft,
                        *bodyRight,
                        *right;

    //Creating bodyleft
    bodyLeft = pGraph->Add( SmCaretPos( pBody, 0 ), left );
    left->SetRight( bodyLeft );

    //Creating right
    right = pGraph->Add( SmCaretPos( pNode, 1 ) );

    //Visit the body
    pRightMost = bodyLeft;
    pBody->Accept( this );
    bodyRight = pRightMost;
    bodyRight->SetRight( right );
    right->SetLeft( bodyRight );

    //Create attrLeft
    attrLeft = pGraph->Add( SmCaretPos( pAttr, 0 ), left );

    //Visit attribute
    pRightMost = attrLeft;
    pAttr->Accept( this );
    pRightMost->SetRight( right );

    //Set return value
    pRightMost = right;
}

//Consider these single symboles
void SmCaretPosGraphBuildingVisitor::Visit( SmSpecialNode* pNode )
{
    SmCaretPosGraphEntry* right = pGraph->Add( SmCaretPos( pNode, 1 ), pRightMost );
    pRightMost->SetRight( right );
    pRightMost = right;
}
void SmCaretPosGraphBuildingVisitor::Visit( SmGlyphSpecialNode* pNode )
{
    SmCaretPosGraphEntry* right = pGraph->Add( SmCaretPos( pNode, 1 ), pRightMost );
    pRightMost->SetRight( right );
    pRightMost = right;
}
void SmCaretPosGraphBuildingVisitor::Visit( SmMathSymbolNode* pNode )
{
    SmCaretPosGraphEntry* right = pGraph->Add( SmCaretPos( pNode, 1 ), pRightMost );
    pRightMost->SetRight( right );
    pRightMost = right;
}

void SmCaretPosGraphBuildingVisitor::Visit( SmRootSymbolNode* )
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

/////////////////////////////// SmCloningVisitor ///////////////////////////////

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
    SmNode* pKid;
    for( sal_uInt16 i = 0; i < nSize; i++ ){
        if( NULL != ( pKid = pSource->GetSubNode( i ) ) )
            pKid->Accept( this );
        else
            pResult = NULL;
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

/////////////////////////////// SmSelectionDrawingVisitor ///////////////////////////////

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
        rDev.Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
        //Change colors
        rDev.SetLineColor( );
        rDev.SetFillColor( Color( COL_LIGHTGRAY ) );

        //Draw rectangle
        rDev.DrawRect( aSelectionArea );

        //Restore device state
        rDev.Pop( );
    }
}

void SmSelectionDrawingVisitor::ExtendSelectionArea( Rectangle aArea )
{
    if ( ! bHasSelectionArea ) {
        aSelectionArea = aArea;
        bHasSelectionArea = true;
    } else
        aSelectionArea.Union( aArea );
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
        rDev.Push( PUSH_TEXTCOLOR | PUSH_FONT );

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

/////////////////////////////// SmNodeToTextVisitor ///////////////////////////////

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
        SmSubSupNode *pSubSup = ( SmSubSupNode* )pNode->GetSubNode( 0 );
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
        if( ( pChild = pSubSup->GetSubSup( CSUB ) ) ) {
            Separate( );
            if (pSubSup->IsUseLimits())
                Append( "from { " );
            else
                Append( "csub { " );
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
                    case FNTSIZ_PLUS:
                        Append( "+" );
                        break;
                    case FNTSIZ_MINUS:
                        Append( "-" );
                        break;
                    case FNTSIZ_MULTIPLY:
                        Append( "*" );
                        break;
                    case FNTSIZ_DIVIDE:
                        Append( "/" );
                        break;
                    case FNTSIZ_ABSOLUT:
                    default:
                        break;
                }
                Append( ::rtl::math::doubleToUString(
                            static_cast<double>( pNode->GetSizeParameter( ) ),
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', sal_True ) );
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
    if( ( pChild = pNode->GetSubSup( CSUB ) ) ) {
        Separate( );
        if (pNode->IsUseLimits())
            Append( "from " );
        else
            Append( "csub " );
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
