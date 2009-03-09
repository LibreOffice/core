/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: postit.cxx,v $
 * $Revision: 1.12.54.11 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "postit.hxx"

#include <svtools/useroptions.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <svx/outlobj.hxx>
#include <svx/editobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include "scitems.hxx"
#include <svx/xlnstit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/xflclit.hxx>
#include <svx/sdshitm.hxx>
#include <svx/sdsxyitm.hxx>

#include "document.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "cell.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "detfunc.hxx"

// ============================================================================

namespace {

const long SC_NOTECAPTION_WIDTH             =  2900;    /// Default width of note caption textbox.
const long SC_NOTECAPTION_MAXWIDTH_TEMP     = 12000;    /// Maximum width of temporary note caption textbox.
const long SC_NOTECAPTION_HEIGHT            =  1800;    /// Default height of note caption textbox.
const long SC_NOTECAPTION_CELLDIST          =   600;    /// Default distance of note captions to border of anchor cell.
const long SC_NOTECAPTION_OFFSET_Y          = -1500;    /// Default Y offset of note captions to top border of anchor cell.
const long SC_NOTECAPTION_OFFSET_X          =  1500;    /// Default X offset of note captions to left border of anchor cell.
const long SC_NOTECAPTION_BORDERDIST_TEMP   =   100;    /// Distance of temporary note captions to visible sheet area.

// ----------------------------------------------------------------------------

class ScCaptionCreator
{
public:
    /** Create a new caption. The caption will not be inserted into the document. */
    explicit            ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, bool bShown, bool bTailFront );
    /** Manipulate an existing caption. */
    explicit            ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj& rCaption );

    /** Returns the caption drawing obejct. */
    inline SdrCaptionObj* GetCaption() { return mpCaption; }

    /** Moves the caption inside the passed rectangle. Uses page area if 0 is passed. */
    void                FitCaptionToRect( const Rectangle* pVisRect = 0 );
    /** Places the passed caption inside the passed rectangle, tries to keep the cell rectangle uncovered. Uses page area if 0 is passed. */
    void                AutoPlaceCaption( const Rectangle* pVisRect = 0 );
    /** Updates caption tail and textbox according to current cell position. Uses page area if 0 is passed. */
    void                UpdateCaptionPos( const Rectangle* pVisRect = 0 );
    /** Sets all default formatting attributes to the caption object. */
    void                SetDefaultItems();
    /** Updates caption itemset according to the passed item set while removing shadow items. */
    void                SetCaptionItems( const SfxItemSet& rItemSet );

private:
    /** Initializes all members. */
    void                Initialize();
    /** Returns the passed rectangle if existing, page rectangle otherwise. */
    inline const Rectangle& GetVisRect( const Rectangle* pVisRect ) const { return pVisRect ? *pVisRect : maPageRect; }
    /** Calculates the caption tail position according to current cell position. */
    Point               CalcTailPos( bool bTailFront );

private:
    ScDocument&         mrDoc;
    ScAddress           maPos;
    SdrCaptionObj*      mpCaption;
    Rectangle           maPageRect;
    Rectangle           maCellRect;
    bool                mbNegPage;
};

// ----------------------------------------------------------------------------

ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, bool bShown, bool bTailFront ) :
    mrDoc( rDoc ),
    maPos( rPos ),
    mpCaption( 0 )
{
    Initialize();

    // create the caption drawing object
    Rectangle aTextRect( Point( 0 , 0 ), Size( SC_NOTECAPTION_WIDTH, SC_NOTECAPTION_HEIGHT ) );
    Point aTailPos = CalcTailPos( bTailFront );
    mpCaption = new SdrCaptionObj( aTextRect, aTailPos );

    // basic settings
    ScDrawLayer::SetAnchor( mpCaption, SCA_PAGE );
    mpCaption->SetLayer( bShown ? SC_LAYER_INTERN : SC_LAYER_HIDDEN );
    mpCaption->SetFixedTail();
    mpCaption->SetSpecialTextBoxShadow();
}

ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj& rCaption ) :
    mrDoc( rDoc ),
    maPos( rPos ),
    mpCaption( &rCaption )
{
    Initialize();
}

void ScCaptionCreator::FitCaptionToRect( const Rectangle* pVisRect )
{
    const Rectangle& rVisRect = GetVisRect( pVisRect );

    // tail position
    Point aTailPos = mpCaption->GetTailPos();
    aTailPos.X() = ::std::max( ::std::min( aTailPos.X(), rVisRect.Right() ), rVisRect.Left() );
    aTailPos.Y() = ::std::max( ::std::min( aTailPos.Y(), rVisRect.Bottom() ), rVisRect.Top() );
    mpCaption->SetTailPos( aTailPos );

    // caption rectangle
    Rectangle aCaptRect = mpCaption->GetLogicRect();
    Point aCaptPos = aCaptRect.TopLeft();
    // move textbox inside right border of visible area
    aCaptPos.X() = ::std::min< long >( aCaptPos.X(), rVisRect.Right() - aCaptRect.GetWidth() );
    // move textbox inside left border of visible area (this may move it outside on right side again)
    aCaptPos.X() = ::std::max< long >( aCaptPos.X(), rVisRect.Left() );
    // move textbox inside bottom border of visible area
    aCaptPos.Y() = ::std::min< long >( aCaptPos.Y(), rVisRect.Bottom() - aCaptRect.GetHeight() );
    // move textbox inside top border of visible area (this may move it outside on bottom side again)
    aCaptPos.Y() = ::std::max< long >( aCaptPos.Y(), rVisRect.Top() );
    // update caption
    aCaptRect.SetPos( aCaptPos );
    mpCaption->SetLogicRect( aCaptRect );
}

void ScCaptionCreator::AutoPlaceCaption( const Rectangle* pVisRect )
{
    const Rectangle& rVisRect = GetVisRect( pVisRect );

    // caption rectangle
    Rectangle aCaptRect = mpCaption->GetLogicRect();
    long nWidth = aCaptRect.GetWidth();
    long nHeight = aCaptRect.GetHeight();

    // n***Space contains available space between border of visible area and cell
    long nLeftSpace = maCellRect.Left() - rVisRect.Left() + 1;
    long nRightSpace = rVisRect.Right() - maCellRect.Right() + 1;
    long nTopSpace = maCellRect.Top() - rVisRect.Top() + 1;
    long nBottomSpace = rVisRect.Bottom() - maCellRect.Bottom() + 1;

    // nNeeded*** contains textbox dimensions plus needed distances to cell or border of visible area
    long nNeededSpaceX = nWidth + SC_NOTECAPTION_CELLDIST;
    long nNeededSpaceY = nHeight + SC_NOTECAPTION_CELLDIST;

    // bFitsWidth*** == true means width of textbox fits into horizontal free space of visible area
    bool bFitsWidthLeft = nNeededSpaceX <= nLeftSpace;      // text box width fits into the width left of cell
    bool bFitsWidthRight = nNeededSpaceX <= nRightSpace;    // text box width fits into the width right of cell
    bool bFitsWidth = nWidth <= rVisRect.GetWidth();        // text box width fits into width of visible area

    // bFitsHeight*** == true means height of textbox fits into vertical free space of visible area
    bool bFitsHeightTop = nNeededSpaceY <= nTopSpace;       // text box height fits into the height above cell
    bool bFitsHeightBottom = nNeededSpaceY <= nBottomSpace; // text box height fits into the height below cell
    bool bFitsHeight = nHeight <= rVisRect.GetHeight();     // text box height fits into height of visible area

    // bFits*** == true means the textbox fits completely into free space of visible area
    bool bFitsLeft = bFitsWidthLeft && bFitsHeight;
    bool bFitsRight = bFitsWidthRight && bFitsHeight;
    bool bFitsTop = bFitsWidth && bFitsHeightTop;
    bool bFitsBottom = bFitsWidth && bFitsHeightBottom;

    Point aCaptPos;
    // use left/right placement if possible, or if top/bottom placement not possible
    if( bFitsLeft || bFitsRight || (!bFitsTop && !bFitsBottom) )
    {
        // prefer left in RTL sheet and right in LTR sheets
        bool bPreferLeft = bFitsLeft && (mbNegPage || !bFitsRight);
        bool bPreferRight = bFitsRight && (!mbNegPage || !bFitsLeft);
        // move to left, if left is preferred, or if neither left nor right fit and there is more space to the left
        if( bPreferLeft || (!bPreferRight && (nLeftSpace > nRightSpace)) )
            aCaptPos.X() = maCellRect.Left() - SC_NOTECAPTION_CELLDIST - nWidth;
        else // to right
            aCaptPos.X() = maCellRect.Right() + SC_NOTECAPTION_CELLDIST;
        // Y position according to top cell border
        aCaptPos.Y() = maCellRect.Top() + SC_NOTECAPTION_OFFSET_Y;
    }
    else    // top or bottom placement
    {
        // X position
        aCaptPos.X() = maCellRect.Left() + SC_NOTECAPTION_OFFSET_X;
        // top placement, if possible
        if( bFitsTop )
            aCaptPos.Y() = maCellRect.Top() - SC_NOTECAPTION_CELLDIST - nHeight;
        else    // bottom placement
            aCaptPos.Y() = maCellRect.Bottom() + SC_NOTECAPTION_CELLDIST;
    }

    // update textbox position in note caption object
    aCaptRect.SetPos( aCaptPos );
    mpCaption->SetLogicRect( aCaptRect );
    FitCaptionToRect( pVisRect );
}

void ScCaptionCreator::UpdateCaptionPos( const Rectangle* pVisRect )
{
    ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer();

    // update caption position
    const Point& rOldTailPos = mpCaption->GetTailPos();
    Point aTailPos = CalcTailPos( false );
    if( rOldTailPos != aTailPos )
    {
        // create drawing undo action
        if( pDrawLayer && pDrawLayer->IsRecording() )
            pDrawLayer->AddCalcUndo( pDrawLayer->GetSdrUndoFactory().CreateUndoGeoObject( *mpCaption ) );
        // calculate new caption rectangle (#i98141# handle LTR<->RTL switch correctly)
        Rectangle aCaptRect = mpCaption->GetLogicRect();
        long nDiffX = (rOldTailPos.X() >= 0) ? (aCaptRect.Left() - rOldTailPos.X()) : (rOldTailPos.X() - aCaptRect.Right());
        if( mbNegPage ) nDiffX = -nDiffX - aCaptRect.GetWidth();
        long nDiffY = aCaptRect.Top() - rOldTailPos.Y();
        aCaptRect.SetPos( aTailPos + Point( nDiffX, nDiffY ) );
        // set new tail position and caption rectangle
        mpCaption->SetTailPos( aTailPos );
        mpCaption->SetLogicRect( aCaptRect );
        // fit caption into draw page
        FitCaptionToRect( pVisRect );
    }

    // update cell position in caption user data
    ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( mpCaption, maPos.Tab() );
    if( pCaptData && (maPos != pCaptData->maStart) )
    {
        // create drawing undo action
        if( pDrawLayer && pDrawLayer->IsRecording() )
            pDrawLayer->AddCalcUndo( new ScUndoObjData( mpCaption, pCaptData->maStart, pCaptData->maEnd, maPos, pCaptData->maEnd ) );
        // set new position
        pCaptData->maStart = maPos;
    }
}

void ScCaptionCreator::SetDefaultItems()
{
    SfxItemSet aItemSet = mpCaption->GetMergedItemSet();

    // caption tail arrow
    ::basegfx::B2DPolygon aTriangle;
    aTriangle.append( ::basegfx::B2DPoint( 10.0,  0.0 ) );
    aTriangle.append( ::basegfx::B2DPoint(  0.0, 30.0 ) );
    aTriangle.append( ::basegfx::B2DPoint( 20.0, 30.0 ) );
    aTriangle.setClosed( true );
    /*  #99319# Line ends are now created with an empty name. The
        checkForUniqueItem() method then finds a unique name for the item's
        value. */
    aItemSet.Put( XLineStartItem( String::EmptyString(), ::basegfx::B2DPolyPolygon( aTriangle ) ) );
    aItemSet.Put( XLineStartWidthItem( 200 ) );
    aItemSet.Put( XLineStartCenterItem( FALSE ) );
    aItemSet.Put( XFillStyleItem( XFILL_SOLID ) );
    aItemSet.Put( XFillColorItem( String::EmptyString(), ScDetectiveFunc::GetCommentColor() ) );
    aItemSet.Put( SdrCaptionEscDirItem( SDRCAPT_ESCBESTFIT ) );

    // shadow
    /*  SdrShadowItem has FALSE, instead the shadow is set for the
        rectangle only with SetSpecialTextBoxShadow when the object is
        created (item must be set to adjust objects from older files). */
    aItemSet.Put( SdrShadowItem( FALSE ) );
    aItemSet.Put( SdrShadowXDistItem( 100 ) );
    aItemSet.Put( SdrShadowYDistItem( 100 ) );

    // text attributes
    aItemSet.Put( SdrTextLeftDistItem( 100 ) );
    aItemSet.Put( SdrTextRightDistItem( 100 ) );
    aItemSet.Put( SdrTextUpperDistItem( 100 ) );
    aItemSet.Put( SdrTextLowerDistItem( 100 ) );
    aItemSet.Put( SdrTextAutoGrowWidthItem( FALSE ) );
    aItemSet.Put( SdrTextAutoGrowHeightItem( TRUE ) );
    // #78943# use the default cell style to be able to modify the caption font
    const ScPatternAttr& rDefPattern = static_cast< const ScPatternAttr& >( mrDoc.GetPool()->GetDefaultItem( ATTR_PATTERN ) );
    rDefPattern.FillEditItemSet( &aItemSet );

    mpCaption->SetMergedItemSet( aItemSet );
}

void ScCaptionCreator::SetCaptionItems( const SfxItemSet& rItemSet )
{
    // copy all items
    mpCaption->SetMergedItemSet( rItemSet );
    // reset shadow items
    mpCaption->SetMergedItem( SdrShadowItem( FALSE ) );
    mpCaption->SetMergedItem( SdrShadowXDistItem( 100 ) );
    mpCaption->SetMergedItem( SdrShadowYDistItem( 100 ) );
    mpCaption->SetSpecialTextBoxShadow();
}

void ScCaptionCreator::Initialize()
{
    maCellRect = ScDrawLayer::GetCellRect( mrDoc, maPos, true );
    mbNegPage = mrDoc.IsNegativePage( maPos.Tab() );

    if( ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer() )
    {
        if( SdrPage* pPage = pDrawLayer->GetPage( static_cast< sal_uInt16 >( maPos.Tab() ) ) )
        {
            maPageRect = Rectangle( Point( 0, 0 ), pPage->GetSize() );
            /*  #i98141# SdrPage::GetSize() returns negative width in RTL mode.
                The call to Rectangle::Adjust() orders left/right coordinate
                accordingly. */
            maPageRect.Justify();
        }
    }
}

Point ScCaptionCreator::CalcTailPos( bool bTailFront )
{
    // tail position
    bool bTailLeft = bTailFront != mbNegPage;
    Point aTailPos = bTailLeft ? maCellRect.TopLeft() : maCellRect.TopRight();
    // move caption point 1/10 mm inside cell
    if( bTailLeft ) aTailPos.X() += 10; else aTailPos.X() -= 10;
    aTailPos.Y() += 10;
    return aTailPos;
}

} // namespace

// ============================================================================

ScNoteData::ScNoteData( bool bShown ) :
    mpCaption( 0 ),
    mbShown( bShown )
{
}

// ============================================================================

ScPostIt::ScPostIt( ScDocument& rDoc, const ScAddress& rPos, bool bShown ) :
    mrDoc( rDoc ),
    maNoteData( bShown )
{
    AutoStamp();
    CreateCaption( rPos );
}

ScPostIt::ScPostIt( ScDocument& rDoc, const ScAddress& rPos, const ScPostIt& rNote ) :
    mrDoc( rDoc ),
    maNoteData( rNote.maNoteData )
{
    maNoteData.mpCaption = 0;
    CreateCaption( rPos, rNote.maNoteData.mpCaption );
}

ScPostIt::ScPostIt( ScDocument& rDoc, const ScNoteData& rNoteData ) :
    mrDoc( rDoc ),
    maNoteData( rNoteData )
{
}

ScPostIt::~ScPostIt()
{
    RemoveCaption();
}

void ScPostIt::AutoStamp()
{
    maNoteData.maDate = ScGlobal::pLocaleData->getDate( Date() );
    maNoteData.maAuthor = SvtUserOptions().GetID();
}

const EditTextObject* ScPostIt::GetEditTextObject() const
{
    if( maNoteData.mpCaption )
        if( const OutlinerParaObject* pOPO = maNoteData.mpCaption->GetOutlinerParaObject() )
            return &pOPO->GetTextObject();
    return 0;
}

String ScPostIt::GetText() const
{
    String aText;
    if( const EditTextObject* pEditObj = GetEditTextObject() )
    {
        for( USHORT nPara = 0, nParaCount = pEditObj->GetParagraphCount(); nPara < nParaCount; ++nPara )
        {
            if( nPara > 0 )
                aText.Append( '\n' );
            aText.Append( pEditObj->GetText( nPara ) );
        }
    }
    return aText;
}

bool ScPostIt::HasMultiLineText() const
{
    const EditTextObject* pEditObj = GetEditTextObject();
    return pEditObj && (pEditObj->GetParagraphCount() > 1);
}

void ScPostIt::SetText( const String& rText )
{
    if( maNoteData.mpCaption )
        maNoteData.mpCaption->SetText( rText );
}

void ScPostIt::ShowCaption( bool bShow )
{
    maNoteData.mbShown = bShow;
    UpdateCaptionLayer( maNoteData.mbShown );
}

void ScPostIt::ShowCaptionTemp( bool bShow )
{
    UpdateCaptionLayer( maNoteData.mbShown || bShow );
}

void ScPostIt::UpdateCaptionPos( const ScAddress& rPos )
{
    if( maNoteData.mpCaption )
    {
        ScCaptionCreator aCreator( mrDoc, rPos, *maNoteData.mpCaption );
        aCreator.UpdateCaptionPos();
    }
}

void ScPostIt::SetCaptionDefaultItems()
{
    if( maNoteData.mpCaption )
    {
        ScCaptionCreator aCreator( mrDoc, ScAddress(), *maNoteData.mpCaption );
        aCreator.SetDefaultItems();
    }
}

void ScPostIt::SetCaptionItems( const SfxItemSet& rItemSet )
{
    if( maNoteData.mpCaption )
    {
        ScCaptionCreator aCreator( mrDoc, ScAddress(), *maNoteData.mpCaption );
        aCreator.SetCaptionItems( rItemSet );
    }
}

// private --------------------------------------------------------------------

void ScPostIt::CreateCaption( const ScAddress& rPos, const SdrCaptionObj* pCaption )
{
    DBG_ASSERT( !maNoteData.mpCaption, "ScPostIt::CreateCaption - unexpected caption object found" );
    maNoteData.mpCaption = 0;

    // drawing layer may be missing, if a note is copied into a clipboard document
    DBG_ASSERT( !mrDoc.IsUndo(), "ScPostIt::CreateCaption - note caption should not be created in undo documents" );
    if( mrDoc.IsClipboard() )
        mrDoc.InitDrawLayer();

    if( ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer() )
    {
        SdrPage* pDrawPage = pDrawLayer->GetPage( static_cast< sal_uInt16 >( rPos.Tab() ) );
        DBG_ASSERT( pDrawPage, "ScPostIt::CreateCaption - no drawing page" );
        if( pDrawPage )
        {
            // create the caption drawing object
            ScCaptionCreator aCreator( mrDoc, rPos, maNoteData.mbShown, false );
            maNoteData.mpCaption = aCreator.GetCaption();

            // additional user data (pass true to create the object data entry)
            ScDrawObjData* pData = ScDrawLayer::GetObjData( maNoteData.mpCaption, true );
            pData->maStart = rPos;
            pData->mbNote = true;

            // insert object into draw page
            pDrawPage->InsertObject( maNoteData.mpCaption );

            // clone settings of passed caption
            if( pCaption )
            {
                // copy edit text object (object must be inserted into page already)
                if( OutlinerParaObject* pOPO = pCaption->GetOutlinerParaObject() )
                    maNoteData.mpCaption->SetOutlinerParaObject( pOPO );
                // copy formatting items (after text has been copied to apply font formatting)
                maNoteData.mpCaption->SetMergedItemSetAndBroadcast( pCaption->GetMergedItemSet() );
                // move textbox position relative to new cell, copy textbox size
                Rectangle aCaptRect = pCaption->GetLogicRect();
                Point aDist = maNoteData.mpCaption->GetTailPos() - pCaption->GetTailPos();
                aCaptRect.Move( aDist.X(), aDist.Y() );
                maNoteData.mpCaption->SetLogicRect( aCaptRect );
                aCreator.FitCaptionToRect();
            }
            else
            {
                // set default formatting and default position
                aCreator.SetDefaultItems();
                aCreator.AutoPlaceCaption();
            }

            // create undo action
            if( pDrawLayer->IsRecording() )
                pDrawLayer->AddCalcUndo( pDrawLayer->GetSdrUndoFactory().CreateUndoNewObject( *maNoteData.mpCaption ) );
        }
    }
}

void ScPostIt::RemoveCaption()
{
    /*  Remove caption object only, if this note is its owner (e.g. notes in
        undo documents refer to captions in original document, do not remove
        them from drawing layer here). */
    if( maNoteData.mpCaption && (mrDoc.GetDrawLayer() == maNoteData.mpCaption->GetModel()) )
    {
        SdrPage* pDrawPage = maNoteData.mpCaption->GetPage();
        DBG_ASSERT( pDrawPage, "ScPostIt::RemoveCaption - object without drawing page" );
        if( pDrawPage )
        {
               pDrawPage->RecalcObjOrdNums();

            ScDrawLayer* pDrawLayer = static_cast< ScDrawLayer* >( maNoteData.mpCaption->GetModel() );
            DBG_ASSERT( pDrawLayer, "ScPostIt::RemoveCaption - object without drawing layer" );

            // create drawing undo action (before removing the object to have valid draw page in undo action)
            if( pDrawLayer && pDrawLayer->IsRecording() )
                pDrawLayer->AddCalcUndo( pDrawLayer->GetSdrUndoFactory().CreateUndoDeleteObject( *maNoteData.mpCaption ) );

            // remove the object from the drawing page, delete if undo is disabled
            pDrawPage->RemoveObject( maNoteData.mpCaption->GetOrdNum() );
        }
    }
    maNoteData.mpCaption = 0;
}

void ScPostIt::UpdateCaptionLayer( bool bShow )
{
    // no separate drawing undo needed, handled completely inside ScUndoShowHideNote
    SdrLayerID nLayer = bShow ? SC_LAYER_INTERN : SC_LAYER_HIDDEN;
    if( maNoteData.mpCaption && (nLayer != maNoteData.mpCaption->GetLayer()) )
        maNoteData.mpCaption->SetLayer( nLayer );
}

// ============================================================================

ScPostIt* ScNoteUtil::CloneNote( ScDocument& rDoc, const ScAddress& rPos, const ScPostIt& rNote, bool bCloneCaption )
{
    return bCloneCaption ? new ScPostIt( rDoc, rPos, rNote ) : new ScPostIt( rDoc, rNote.GetNoteData() );
}

void ScNoteUtil::UpdateCaptionPositions( ScDocument& rDoc, const ScRange& rRange )
{
    // do not use ScCellIterator, it skips filtered and subtotal cells
    for( ScAddress aPos( rRange.aStart ); aPos.Tab() <= rRange.aEnd.Tab(); aPos.IncTab() )
        for( aPos.SetCol( rRange.aStart.Col() ); aPos.Col() <= rRange.aEnd.Col(); aPos.IncCol() )
            for( aPos.SetRow( rRange.aStart.Row() ); aPos.Row() <= rRange.aEnd.Row(); aPos.IncRow() )
                if( ScPostIt* pNote = rDoc.GetNote( aPos ) )
                    pNote->UpdateCaptionPos( aPos );
}

SdrCaptionObj* ScNoteUtil::CreateTempCaption( ScDocument& rDoc, const ScAddress& rPos,
        SdrPage& rPage, const String& rUserText, const Rectangle& rVisRect, bool bTailFront )
{
    String aFinalText = rUserText;
    // add plain text of invisible (!) cell note (no formatting etc.)
    SdrCaptionObj* pNoteCaption = 0;
    if( ScPostIt* pNote = rDoc.GetNote( rPos ) )
    {
        if( !pNote->IsCaptionShown() )
        {
            if( aFinalText.Len() > 0 )
                   aFinalText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\n--------\n" ) );
            aFinalText.Append( pNote->GetText() );
            pNoteCaption = pNote->GetCaption();
        }
    }

    // create a caption if any text exists
    if( aFinalText.Len() == 0 )
        return 0;

    // prepare visible rectangle (add default distance to all borders)
    Rectangle aVisRect(
        rVisRect.Left() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Top() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Right() - SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Bottom() - SC_NOTECAPTION_BORDERDIST_TEMP );

    // create the caption object
    ScCaptionCreator aCreator( rDoc, rPos, true, bTailFront );
    SdrCaptionObj* pCaption = aCreator.GetCaption();
    // insert caption into page (needed to set caption text)
    rPage.InsertObject( pCaption );
    // set the text to the object
    pCaption->SetText( aFinalText );

    // set formatting (must be done after setting text) and resize the box to fit the text
    if( pNoteCaption && (rUserText.Len() == 0) )
    {
        pCaption->SetMergedItemSetAndBroadcast( pNoteCaption->GetMergedItemSet() );
        Rectangle aCaptRect( pCaption->GetLogicRect().TopLeft(), pNoteCaption->GetLogicRect().GetSize() );
        pCaption->SetLogicRect( aCaptRect );
    }
    else
    {
        aCreator.SetDefaultItems();
        // adjust caption size to text size
        long nMaxWidth = ::std::min< long >( aVisRect.GetWidth() * 2 / 3, SC_NOTECAPTION_MAXWIDTH_TEMP );
        pCaption->SetMergedItem( SdrTextAutoGrowWidthItem( TRUE ) );
        pCaption->SetMergedItem( SdrTextMinFrameWidthItem( SC_NOTECAPTION_WIDTH ) );
        pCaption->SetMergedItem( SdrTextMaxFrameWidthItem( nMaxWidth ) );
        pCaption->SetMergedItem( SdrTextAutoGrowHeightItem( TRUE ) );
        pCaption->AdjustTextFrameWidthAndHeight();
    }

    // move caption into visible area
    aCreator.AutoPlaceCaption( &aVisRect );
    return pCaption;
}

ScPostIt* ScNoteUtil::CreateNoteFromString( ScDocument& rDoc, const ScAddress& rPos, const String& rNoteText, bool bShown )
{
    if( rNoteText.Len() == 0 )
        return 0;
    ScPostIt* pNote = new ScPostIt( rDoc, rPos, bShown );
    rDoc.TakeNote( rPos, pNote );
    if( SdrCaptionObj* pCaption = pNote->GetCaption() )
    {
        pCaption->SetText( rNoteText );
        pNote->SetCaptionDefaultItems();    // reformat text with default font
        pCaption->SetMergedItem( SdrTextMinFrameWidthItem( SC_NOTECAPTION_WIDTH ) );
        pCaption->SetMergedItem( SdrTextMaxFrameWidthItem( SC_NOTECAPTION_MAXWIDTH_TEMP ) );
        pCaption->AdjustTextFrameWidthAndHeight();
    }
    return pNote;
}

// ============================================================================
