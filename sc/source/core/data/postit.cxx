/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <memory>
#include <postit.hxx>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <unotools/useroptions.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <scitems.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/xflclit.hxx>
#include <svx/sdshitm.hxx>
#include <svx/sdsxyitm.hxx>
#include <tools/gen.hxx>

#include <document.hxx>
#include <docpool.hxx>
#include <patattr.hxx>
#include <drwlayer.hxx>
#include <userdat.hxx>
#include <detfunc.hxx>
#include <editutil.hxx>

using namespace com::sun::star;

namespace {

const long SC_NOTECAPTION_WIDTH             =  2900;    /// Default width of note caption textbox.
const long SC_NOTECAPTION_MAXWIDTH_TEMP     = 12000;    /// Maximum width of temporary note caption textbox.
const long SC_NOTECAPTION_HEIGHT            =  1800;    /// Default height of note caption textbox.
const long SC_NOTECAPTION_CELLDIST          =   600;    /// Default distance of note captions to border of anchor cell.
const long SC_NOTECAPTION_OFFSET_Y          = -1500;    /// Default Y offset of note captions to top border of anchor cell.
const long SC_NOTECAPTION_OFFSET_X          =  1500;    /// Default X offset of note captions to left border of anchor cell.
const long SC_NOTECAPTION_BORDERDIST_TEMP   =   100;    /// Distance of temporary note captions to visible sheet area.

/** Static helper functions for caption objects. */
class ScCaptionUtil
{
public:
    /** Moves the caption object to the correct layer according to passed visibility. */
    static void         SetCaptionLayer( SdrCaptionObj& rCaption, bool bShown );
    /** Sets basic caption settings required for note caption objects. */
    static void         SetBasicCaptionSettings( SdrCaptionObj& rCaption, bool bShown );
    /** Stores the cell position of the note in the user data area of the caption. */
    static void         SetCaptionUserData( SdrCaptionObj& rCaption, const ScAddress& rPos );
    /** Sets all default formatting attributes to the caption object. */
    static void         SetDefaultItems( SdrCaptionObj& rCaption, ScDocument& rDoc );
    /** Updates caption item set according to the passed item set while removing shadow items. */
    static void         SetCaptionItems( SdrCaptionObj& rCaption, const SfxItemSet& rItemSet );
};

void ScCaptionUtil::SetCaptionLayer( SdrCaptionObj& rCaption, bool bShown )
{
    SdrLayerID nLayer = bShown ? SC_LAYER_INTERN : SC_LAYER_HIDDEN;
    if( nLayer != rCaption.GetLayer() )
        rCaption.SetLayer( nLayer );
}

void ScCaptionUtil::SetBasicCaptionSettings( SdrCaptionObj& rCaption, bool bShown )
{
    SetCaptionLayer( rCaption, bShown );
    rCaption.SetFixedTail();
    rCaption.SetSpecialTextBoxShadow();
}

void ScCaptionUtil::SetCaptionUserData( SdrCaptionObj& rCaption, const ScAddress& rPos )
{
    // pass true to ScDrawLayer::GetObjData() to create the object data entry
    ScDrawObjData* pObjData = ScDrawLayer::GetObjData( &rCaption, true );
    OSL_ENSURE( pObjData, "ScCaptionUtil::SetCaptionUserData - missing drawing object user data" );
    pObjData->maStart = rPos;
    pObjData->meType = ScDrawObjData::CellNote;
}

void ScCaptionUtil::SetDefaultItems( SdrCaptionObj& rCaption, ScDocument& rDoc )
{
    SfxItemSet aItemSet = rCaption.GetMergedItemSet();

    // caption tail arrow
    ::basegfx::B2DPolygon aTriangle;
    aTriangle.append( ::basegfx::B2DPoint( 10.0,  0.0 ) );
    aTriangle.append( ::basegfx::B2DPoint(  0.0, 30.0 ) );
    aTriangle.append( ::basegfx::B2DPoint( 20.0, 30.0 ) );
    aTriangle.setClosed( true );
    /*  Line ends are now created with an empty name. The
        checkForUniqueItem() method then finds a unique name for the item's
        value. */
    aItemSet.Put( XLineStartItem( OUString(), ::basegfx::B2DPolyPolygon( aTriangle ) ) );
    aItemSet.Put( XLineStartWidthItem( 200 ) );
    aItemSet.Put( XLineStartCenterItem( false ) );
    aItemSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
    aItemSet.Put( XFillColorItem( OUString(), ScDetectiveFunc::GetCommentColor() ) );
    aItemSet.Put( SdrCaptionEscDirItem( SdrCaptionEscDir::BestFit ) );

    // shadow
    /*  SdrShadowItem has sal_False, instead the shadow is set for the
        rectangle only with SetSpecialTextBoxShadow() when the object is
        created (item must be set to adjust objects from older files). */
    aItemSet.Put( makeSdrShadowItem( false ) );
    aItemSet.Put( makeSdrShadowXDistItem( 100 ) );
    aItemSet.Put( makeSdrShadowYDistItem( 100 ) );

    // text attributes
    aItemSet.Put( makeSdrTextLeftDistItem( 100 ) );
    aItemSet.Put( makeSdrTextRightDistItem( 100 ) );
    aItemSet.Put( makeSdrTextUpperDistItem( 100 ) );
    aItemSet.Put( makeSdrTextLowerDistItem( 100 ) );
    aItemSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
    aItemSet.Put( makeSdrTextAutoGrowHeightItem( true ) );
    // use the default cell style to be able to modify the caption font
    const ScPatternAttr& rDefPattern = rDoc.GetPool()->GetDefaultItem( ATTR_PATTERN );
    rDefPattern.FillEditItemSet( &aItemSet );

    rCaption.SetMergedItemSet( aItemSet );
}

void ScCaptionUtil::SetCaptionItems( SdrCaptionObj& rCaption, const SfxItemSet& rItemSet )
{
    // copy all items
    rCaption.SetMergedItemSet( rItemSet );
    // reset shadow items
    rCaption.SetMergedItem( makeSdrShadowItem( false ) );
    rCaption.SetMergedItem( makeSdrShadowXDistItem( 100 ) );
    rCaption.SetMergedItem( makeSdrShadowYDistItem( 100 ) );
    rCaption.SetSpecialTextBoxShadow();
}

/** Helper for creation and manipulation of caption drawing objects independent
    from cell annotations. */
class ScCaptionCreator
{
public:
    /** Create a new caption. The caption will not be inserted into the document. */
    explicit            ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, bool bTailFront );
    /** Manipulate an existing caption. */
    explicit            ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, const ScCaptionPtr& xCaption );

    /** Returns the drawing layer page of the sheet contained in maPos. */
    SdrPage*            GetDrawPage();
    /** Returns the caption drawing object. */
    ScCaptionPtr &      GetCaption() { return mxCaption; }

    /** Moves the caption inside the passed rectangle. Uses page area if 0 is passed. */
    void                FitCaptionToRect( const tools::Rectangle* pVisRect = nullptr );
    /** Places the caption inside the passed rectangle, tries to keep the cell rectangle uncovered. Uses page area if 0 is passed. */
    void                AutoPlaceCaption( const tools::Rectangle* pVisRect = nullptr );
    /** Updates caption tail and textbox according to current cell position. Uses page area if 0 is passed. */
    void                UpdateCaptionPos();

protected:
    /** Helper constructor for derived classes. */
    explicit            ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos );

    /** Calculates the caption tail position according to current cell position. */
    Point               CalcTailPos( bool bTailFront );
    /** Implements creation of the caption object. The caption will not be inserted into the document. */
    void                CreateCaption( bool bShown, bool bTailFront );

private:
    /** Initializes all members. */
    void                Initialize();
    /** Returns the passed rectangle if existing, page rectangle otherwise. */
    const tools::Rectangle& GetVisRect( const tools::Rectangle* pVisRect ) const { return pVisRect ? *pVisRect : maPageRect; }

private:
    ScDocument&         mrDoc;
    ScAddress           maPos;
    ScCaptionPtr        mxCaption;
    tools::Rectangle           maPageRect;
    tools::Rectangle           maCellRect;
    bool                mbNegPage;
};

ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, bool bTailFront ) :
    mrDoc( rDoc ),
    maPos( rPos )
{
    Initialize();
    CreateCaption( true/*bShown*/, bTailFront );
}

ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, const ScCaptionPtr& xCaption ) :
    mrDoc( rDoc ),
    maPos( rPos ),
    mxCaption( xCaption )
{
    Initialize();
}

ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos ) :
    mrDoc( rDoc ),
    maPos( rPos )
{
    Initialize();
}

SdrPage* ScCaptionCreator::GetDrawPage()
{
    ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer();
    return pDrawLayer ? pDrawLayer->GetPage( static_cast< sal_uInt16 >( maPos.Tab() ) ) : nullptr;
}

void ScCaptionCreator::FitCaptionToRect( const tools::Rectangle* pVisRect )
{
    const tools::Rectangle& rVisRect = GetVisRect( pVisRect );

    // tail position
    Point aTailPos = mxCaption->GetTailPos();
    aTailPos.setX( ::std::max( ::std::min( aTailPos.X(), rVisRect.Right() ), rVisRect.Left() ) );
    aTailPos.setY( ::std::max( ::std::min( aTailPos.Y(), rVisRect.Bottom() ), rVisRect.Top() ) );
    mxCaption->SetTailPos( aTailPos );

    // caption rectangle
    tools::Rectangle aCaptRect = mxCaption->GetLogicRect();
    Point aCaptPos = aCaptRect.TopLeft();
    // move textbox inside right border of visible area
    aCaptPos.setX( ::std::min< long >( aCaptPos.X(), rVisRect.Right() - aCaptRect.GetWidth() ) );
    // move textbox inside left border of visible area (this may move it outside on right side again)
    aCaptPos.setX( ::std::max< long >( aCaptPos.X(), rVisRect.Left() ) );
    // move textbox inside bottom border of visible area
    aCaptPos.setY( ::std::min< long >( aCaptPos.Y(), rVisRect.Bottom() - aCaptRect.GetHeight() ) );
    // move textbox inside top border of visible area (this may move it outside on bottom side again)
    aCaptPos.setY( ::std::max< long >( aCaptPos.Y(), rVisRect.Top() ) );
    // update caption
    aCaptRect.SetPos( aCaptPos );
    mxCaption->SetLogicRect( aCaptRect );
}

void ScCaptionCreator::AutoPlaceCaption( const tools::Rectangle* pVisRect )
{
    const tools::Rectangle& rVisRect = GetVisRect( pVisRect );

    // caption rectangle
    tools::Rectangle aCaptRect = mxCaption->GetLogicRect();
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
            aCaptPos.setX( maCellRect.Left() - SC_NOTECAPTION_CELLDIST - nWidth );
        else // to right
            aCaptPos.setX( maCellRect.Right() + SC_NOTECAPTION_CELLDIST );
        // Y position according to top cell border
        aCaptPos.setY( maCellRect.Top() + SC_NOTECAPTION_OFFSET_Y );
    }
    else    // top or bottom placement
    {
        // X position
        aCaptPos.setX( maCellRect.Left() + SC_NOTECAPTION_OFFSET_X );
        // top placement, if possible
        if( bFitsTop )
            aCaptPos.setY( maCellRect.Top() - SC_NOTECAPTION_CELLDIST - nHeight );
        else    // bottom placement
            aCaptPos.setY( maCellRect.Bottom() + SC_NOTECAPTION_CELLDIST );
    }

    // update textbox position in note caption object
    aCaptRect.SetPos( aCaptPos );
    mxCaption->SetLogicRect( aCaptRect );
    FitCaptionToRect( pVisRect );
}

void ScCaptionCreator::UpdateCaptionPos()
{
    ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer();

    // update caption position
    const Point& rOldTailPos = mxCaption->GetTailPos();
    Point aTailPos = CalcTailPos( false );
    if( rOldTailPos != aTailPos )
    {
        // create drawing undo action
        if( pDrawLayer && pDrawLayer->IsRecording() )
            pDrawLayer->AddCalcUndo( std::make_unique<SdrUndoGeoObj>( *mxCaption ) );
        // calculate new caption rectangle (#i98141# handle LTR<->RTL switch correctly)
        tools::Rectangle aCaptRect = mxCaption->GetLogicRect();
        long nDiffX = (rOldTailPos.X() >= 0) ? (aCaptRect.Left() - rOldTailPos.X()) : (rOldTailPos.X() - aCaptRect.Right());
        if( mbNegPage ) nDiffX = -nDiffX - aCaptRect.GetWidth();
        long nDiffY = aCaptRect.Top() - rOldTailPos.Y();
        aCaptRect.SetPos( aTailPos + Point( nDiffX, nDiffY ) );
        // set new tail position and caption rectangle
        mxCaption->SetTailPos( aTailPos );
        mxCaption->SetLogicRect( aCaptRect );
        // fit caption into draw page
        FitCaptionToRect();
    }

    // update cell position in caption user data
    ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( mxCaption.get(), maPos.Tab() );
    if( pCaptData && (maPos != pCaptData->maStart) )
    {
        // create drawing undo action
        if( pDrawLayer && pDrawLayer->IsRecording() )
            pDrawLayer->AddCalcUndo( std::make_unique<ScUndoObjData>( mxCaption.get(), pCaptData->maStart, pCaptData->maEnd, maPos, pCaptData->maEnd ) );
        // set new position
        pCaptData->maStart = maPos;
    }
}

Point ScCaptionCreator::CalcTailPos( bool bTailFront )
{
    // tail position
    bool bTailLeft = bTailFront != mbNegPage;
    Point aTailPos = bTailLeft ? maCellRect.TopLeft() : maCellRect.TopRight();
    // move caption point 1/10 mm inside cell
    if( bTailLeft ) aTailPos.AdjustX(10 ); else aTailPos.AdjustX( -10 );
    aTailPos.AdjustY(10);
    return aTailPos;
}

void ScCaptionCreator::CreateCaption( bool bShown, bool bTailFront )
{
    // create the caption drawing object
    tools::Rectangle aTextRect( Point( 0 , 0 ), Size( SC_NOTECAPTION_WIDTH, SC_NOTECAPTION_HEIGHT ) );
    Point aTailPos = CalcTailPos( bTailFront );
    mxCaption.reset(
        new SdrCaptionObj(
            *mrDoc.GetDrawLayer(), // TTTT should ret a ref?
            aTextRect,
            aTailPos));
    // basic caption settings
    ScCaptionUtil::SetBasicCaptionSettings( *mxCaption, bShown );
}

void ScCaptionCreator::Initialize()
{
    maCellRect = ScDrawLayer::GetCellRect( mrDoc, maPos, true );
    mbNegPage = mrDoc.IsNegativePage( maPos.Tab() );
    if( SdrPage* pDrawPage = GetDrawPage() )
    {
        maPageRect = tools::Rectangle( Point( 0, 0 ), pDrawPage->GetSize() );
        /*  #i98141# SdrPage::GetSize() returns negative width in RTL mode.
            The call to Rectangle::Adjust() orders left/right coordinate
            accordingly. */
        maPageRect.Justify();
    }
}

/** Helper for creation of permanent caption drawing objects for cell notes. */
class ScNoteCaptionCreator : public ScCaptionCreator
{
public:
    /** Create a new caption object and inserts it into the document. */
    explicit            ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, ScNoteData& rNoteData );
    /** Manipulate an existing caption. */
    explicit            ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, ScCaptionPtr& xCaption, bool bShown );
};

ScNoteCaptionCreator::ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, ScNoteData& rNoteData ) :
    ScCaptionCreator( rDoc, rPos )  // use helper c'tor that does not create the caption yet
{
    SdrPage* pDrawPage = GetDrawPage();
    OSL_ENSURE( pDrawPage, "ScNoteCaptionCreator::ScNoteCaptionCreator - no drawing page" );
    if( pDrawPage )
    {
        // create the caption drawing object
        CreateCaption( rNoteData.mbShown, false );
        rNoteData.mxCaption = GetCaption();
        OSL_ENSURE( rNoteData.mxCaption, "ScNoteCaptionCreator::ScNoteCaptionCreator - missing caption object" );
        if( rNoteData.mxCaption )
        {
            // store note position in user data of caption object
            ScCaptionUtil::SetCaptionUserData( *rNoteData.mxCaption, rPos );
            // insert object into draw page
            pDrawPage->InsertObject( rNoteData.mxCaption.get() );
        }
    }
}

ScNoteCaptionCreator::ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, ScCaptionPtr& xCaption, bool bShown ) :
    ScCaptionCreator( rDoc, rPos, xCaption )
{
    SdrPage* pDrawPage = GetDrawPage();
    OSL_ENSURE( pDrawPage, "ScNoteCaptionCreator::ScNoteCaptionCreator - no drawing page" );
    OSL_ENSURE( xCaption->getSdrPageFromSdrObject() == pDrawPage, "ScNoteCaptionCreator::ScNoteCaptionCreator - wrong drawing page in caption" );
    if( pDrawPage && (xCaption->getSdrPageFromSdrObject() == pDrawPage) )
    {
        // store note position in user data of caption object
        ScCaptionUtil::SetCaptionUserData( *xCaption, rPos );
        // basic caption settings
        ScCaptionUtil::SetBasicCaptionSettings( *xCaption, bShown );
        // set correct tail position
        xCaption->SetTailPos( CalcTailPos( false ) );
    }
}

} // namespace

ScCaptionPtr::ScCaptionPtr() :
    mpHead(nullptr), mpNext(nullptr), mpCaption(nullptr), mbNotOwner(false)
{
}

ScCaptionPtr::ScCaptionPtr( SdrCaptionObj* p ) :
    mpHead(nullptr), mpNext(nullptr), mpCaption(p), mbNotOwner(false)
{
    if (p)
    {
        newHead();
     }
}

ScCaptionPtr::ScCaptionPtr( const ScCaptionPtr& r ) :
    mpHead(r.mpHead), mpCaption(r.mpCaption), mbNotOwner(false)
{
    if (r.mpCaption)
    {
        assert(r.mpHead);
        r.incRef();
        // Insert into list.
        mpNext = r.mpNext;
        r.mpNext = this;
    }
    else
    {
        assert(!r.mpHead);
        mpNext = nullptr;
    }
}

ScCaptionPtr::ScCaptionPtr( ScCaptionPtr&& r ) :
    mpHead(r.mpHead), mpNext(r.mpNext), mpCaption(r.mpCaption), mbNotOwner(false)
{
    r.replaceInList( this );
    r.mpCaption = nullptr;
    r.mbNotOwner = false;
}

ScCaptionPtr& ScCaptionPtr::operator=( ScCaptionPtr&& r )
{
    assert(this != &r);

    mpHead = r.mpHead;
    mpNext = r.mpNext;
    mpCaption = r.mpCaption;
    mbNotOwner = r.mbNotOwner;

    r.replaceInList( this );
    r.mpCaption = nullptr;
    r.mbNotOwner = false;

    return *this;
}

ScCaptionPtr& ScCaptionPtr::operator=( const ScCaptionPtr& r )
{
    if (this == &r)
        return *this;

    if (mpCaption == r.mpCaption)
    {
        // Two lists for the same caption is bad.
        assert(!mpCaption || mpHead == r.mpHead);
        assert(!mpCaption);     // assigning same caption pointer within same list is weird
        // Nullptr captions are not inserted to the list, so nothing to do here
        // if both are.
        return *this;
    }

    // Let's find some weird usage.
    // Assigning without head doesn't make sense unless it is a nullptr caption.
    assert(r.mpHead || !r.mpCaption);
    // A nullptr caption must not be in a list and thus not have a head.
    assert(!r.mpHead || r.mpCaption);
    // Same captions were caught above, so here different heads must be present.
    assert(r.mpHead != mpHead);

    r.incRef();
    decRefAndDestroy();
    removeFromList();

    mpCaption = r.mpCaption;
    mbNotOwner = r.mbNotOwner;
    // That head is this' master.
    mpHead = r.mpHead;
    // Insert into list.
    mpNext = r.mpNext;
    r.mpNext = this;

    return *this;
}

void ScCaptionPtr::setNotOwner()
{
    mbNotOwner = true;
}

ScCaptionPtr::Head::Head( ScCaptionPtr* p ) :
    mpFirst(p), mnRefs(1)
{
}

void ScCaptionPtr::newHead()
{
    assert(!mpHead);
    mpHead = new Head(this);
}

void ScCaptionPtr::replaceInList( ScCaptionPtr* pNew )
{
    if (!mpHead && !mpNext)
        return;

    assert(mpHead);
    assert(mpCaption == pNew->mpCaption);

    ScCaptionPtr* pThat = mpHead->mpFirst;
    while (pThat && pThat != this && pThat->mpNext != this)
    {
        pThat = pThat->mpNext;
    }
    if (pThat && pThat != this)
    {
        assert(pThat->mpNext == this);
        pThat->mpNext = pNew;
    }
    pNew->mpNext = mpNext;
    if (mpHead->mpFirst == this)
        mpHead->mpFirst = pNew;

    mpHead = nullptr;
    mpNext = nullptr;
}

void ScCaptionPtr::removeFromList()
{
    if (!mpHead && !mpNext && !mpCaption)
        return;

#if OSL_DEBUG_LEVEL > 0
    oslInterlockedCount nCount = 0;
#endif
    ScCaptionPtr* pThat = (mpHead ? mpHead->mpFirst : nullptr);
    while (pThat && pThat != this && pThat->mpNext != this)
    {
        // Use the walk to check consistency on the fly.
        assert(pThat->mpHead == mpHead);            // all belong to the same
        assert(pThat->mpHead || !pThat->mpNext);    // next without head is bad
        assert(pThat->mpCaption == mpCaption);
        pThat = pThat->mpNext;
#if OSL_DEBUG_LEVEL > 0
        ++nCount;
#endif
    }
    assert(pThat || !mpHead);   // not found only if this was standalone
    if (pThat)
    {
        if (pThat != this)
        {
#if OSL_DEBUG_LEVEL > 0
            // The while loop above was not executed, and for this
            // (pThat->mpNext) the loop below won't either.
            ++nCount;
#endif
            pThat->mpNext = mpNext;
        }
#if OSL_DEBUG_LEVEL > 0
        do
        {
            assert(pThat->mpHead == mpHead);            // all belong to the same
            assert(pThat->mpHead || !pThat->mpNext);    // next without head is bad
            assert(pThat->mpCaption == mpCaption);
            ++nCount;
        }
        while ((pThat = pThat->mpNext) != nullptr);
#endif
    }
#if OSL_DEBUG_LEVEL > 0
    // If part of a list then refs were already decremented.
    assert(nCount == (mpHead ? mpHead->mnRefs + 1 : 0));
#endif
    if (mpHead && mpHead->mpFirst == this)
    {
        if (mpNext)
            mpHead->mpFirst = mpNext;
        else
        {
            // The only one destroys also head.
            assert(mpHead->mnRefs == 0);    // cough
            delete mpHead;                  // DEAD now
        }
    }
    mpHead = nullptr;
    mpNext = nullptr;
}

void ScCaptionPtr::reset( SdrCaptionObj* p )
{
    assert(!p || p != mpCaption);
#if OSL_DEBUG_LEVEL > 0
    if (p)
    {
        // Check if we end up with a duplicated management in this list.
        ScCaptionPtr* pThat = (mpHead ? mpHead->mpFirst : nullptr);
        while (pThat)
        {
            assert(pThat->mpCaption != p);
            pThat = pThat->mpNext;
        }
    }
#endif
    decRefAndDestroy();
    removeFromList();
    mpCaption = p;
    mbNotOwner = false;
    if (p)
    {
        newHead();
    }
}

ScCaptionPtr::~ScCaptionPtr()
{
    decRefAndDestroy();
    removeFromList();
}

oslInterlockedCount ScCaptionPtr::getRefs() const
{
    return mpHead ? mpHead->mnRefs : 0;
}

void ScCaptionPtr::incRef() const
{
    if (mpHead)
        osl_atomic_increment(&mpHead->mnRefs);
}

bool ScCaptionPtr::decRef() const
{
    return mpHead && mpHead->mnRefs > 0 && !osl_atomic_decrement(&mpHead->mnRefs);
}

void ScCaptionPtr::decRefAndDestroy()
{
    if (decRef())
    {
        assert(mpHead->mpFirst == this);    // this must be one and only one
        assert(!mpNext);                    // this must be one and only one
        assert(mpCaption);

#if 0
        // Quick workaround for when there are still cases where the caption
        // pointer is dangling
        mpCaption = nullptr;
        mbNotOwner = false;
#else
        // Destroying Draw Undo and some other delete the SdrObject, don't
        // attempt that twice.
        if (mbNotOwner)
        {
            mpCaption = nullptr;
            mbNotOwner = false;
        }
        else
        {
            removeFromDrawPageAndFree( true );  // ignoring Undo
            if (mpCaption)
            {
                // There's no draw page associated so removeFromDrawPageAndFree()
                // didn't do anything, but still we want to delete the caption
                // object. release()/dissolve() also resets mpCaption.
                SdrObject* pObj = release();
                SdrObject::Free( pObj );
            }
        }
#endif
        delete mpHead;
        mpHead = nullptr;
    }
}

void ScCaptionPtr::insertToDrawPage( SdrPage& rDrawPage )
{
    assert(mpHead && mpCaption);

    rDrawPage.InsertObject( mpCaption );
}

void ScCaptionPtr::removeFromDrawPage( SdrPage& rDrawPage )
{
    assert(mpHead && mpCaption);
    SdrObject* pObj = rDrawPage.RemoveObject( mpCaption->GetOrdNum() );
    assert(pObj == mpCaption); (void)pObj;
}

void ScCaptionPtr::removeFromDrawPageAndFree( bool bIgnoreUndo )
{
    assert(mpHead && mpCaption);
    SdrPage* pDrawPage(mpCaption->getSdrPageFromSdrObject());
    SAL_WARN_IF( !pDrawPage, "sc.core", "ScCaptionPtr::removeFromDrawPageAndFree - object without drawing page");
    if (pDrawPage)
    {
        pDrawPage->RecalcObjOrdNums();
        bool bRecording = false;
        if(!bIgnoreUndo)
        {
            ScDrawLayer* pDrawLayer(dynamic_cast< ScDrawLayer* >(&mpCaption->getSdrModelFromSdrObject()));
            SAL_WARN_IF( !pDrawLayer, "sc.core", "ScCaptionPtr::removeFromDrawPageAndFree - object without drawing layer");
            // create drawing undo action (before removing the object to have valid draw page in undo action)
            bRecording = (pDrawLayer && pDrawLayer->IsRecording());
            if (bRecording)
                pDrawLayer->AddCalcUndo( std::make_unique<SdrUndoDelObj>( *mpCaption ));
        }
        // remove the object from the drawing page, delete if undo is disabled
        removeFromDrawPage( *pDrawPage );
        // If called from outside mnRefs must be 1 to delete. If called from
        // decRefAndDestroy() mnRefs is already 0.
        if (!bRecording && getRefs() <= 1)
        {
            SdrObject* pObj = release();
            SdrObject::Free( pObj );
        }
    }
}

SdrCaptionObj* ScCaptionPtr::release()
{
    SdrCaptionObj* pTmp = mpCaption;
    dissolve();
    return pTmp;
}

void ScCaptionPtr::forget()
{
    decRef();
    removeFromList();
    mpCaption = nullptr;
    mbNotOwner = false;
}

void ScCaptionPtr::dissolve()
{
    ScCaptionPtr::Head* pHead = mpHead;
    ScCaptionPtr* pThat = (mpHead ? mpHead->mpFirst : this);
    while (pThat)
    {
        assert(!pThat->mpNext || pThat->mpHead);    // next without head is bad
        assert(pThat->mpHead == pHead);             // same head required within one list
        ScCaptionPtr* p = pThat->mpNext;
        pThat->clear();
        pThat = p;
     }
    assert(!mpHead && !mpNext && !mpCaption);       // should had been cleared during list walk
    delete pHead;
}

void ScCaptionPtr::clear()
{
    mpHead = nullptr;
    mpNext = nullptr;
    mpCaption = nullptr;
    mbNotOwner = false;
}

struct ScCaptionInitData
{
    std::unique_ptr< SfxItemSet >       mxItemSet;          /// Caption object formatting.
    std::unique_ptr< OutlinerParaObject >  mxOutlinerObj;      /// Text object with all text portion formatting.
    OUString     maSimpleText;       /// Simple text without formatting.
    Point               maCaptionOffset;    /// Caption position relative to cell corner.
    Size                maCaptionSize;      /// Size of the caption object.
    bool                mbDefaultPosSize;   /// True = use default position and size for caption.

    explicit            ScCaptionInitData();
};

ScCaptionInitData::ScCaptionInitData() :
    mbDefaultPosSize( true )
{
}

ScNoteData::ScNoteData( bool bShown ) :
    mbShown( bShown )
{
}

sal_uInt32 ScPostIt::mnLastPostItId = 1;

ScPostIt::ScPostIt( ScDocument& rDoc, const ScAddress& rPos, sal_uInt32 nPostItId ) :
    mrDoc( rDoc ),
    maNoteData( false )
{
    mnPostItId = nPostItId == 0 ? mnLastPostItId++ : nPostItId;
    AutoStamp();
    CreateCaption( rPos );
}

ScPostIt::ScPostIt( ScDocument& rDoc, const ScAddress& rPos, const ScPostIt& rNote, sal_uInt32 nPostItId ) :
    mrDoc( rDoc ),
    maNoteData( rNote.maNoteData )
{
    mnPostItId = nPostItId == 0 ? mnLastPostItId++ : nPostItId;
    maNoteData.mxCaption.reset(nullptr);
    CreateCaption( rPos, rNote.maNoteData.mxCaption.get() );
}

ScPostIt::ScPostIt( ScDocument& rDoc, const ScAddress& rPos, const ScNoteData& rNoteData, bool bAlwaysCreateCaption, sal_uInt32 nPostItId ) :
    mrDoc( rDoc ),
    maNoteData( rNoteData )
{
    mnPostItId = nPostItId == 0 ? mnLastPostItId++ : nPostItId;
    if( bAlwaysCreateCaption || maNoteData.mbShown )
        CreateCaptionFromInitData( rPos );
}

ScPostIt::~ScPostIt()
{
    RemoveCaption();
}

std::unique_ptr<ScPostIt> ScPostIt::Clone( const ScAddress& rOwnPos, ScDocument& rDestDoc, const ScAddress& rDestPos, bool bCloneCaption ) const
{
    CreateCaptionFromInitData( rOwnPos );
    return bCloneCaption ? std::make_unique<ScPostIt>( rDestDoc, rDestPos, *this, mnPostItId ) : std::make_unique<ScPostIt>( rDestDoc, rDestPos, maNoteData, false, mnPostItId );
}

void ScPostIt::SetDate( const OUString& rDate )
{
    maNoteData.maDate = rDate;
}

void ScPostIt::SetAuthor( const OUString& rAuthor )
{
    maNoteData.maAuthor = rAuthor;
}

void ScPostIt::AutoStamp()
{
    maNoteData.maDate = ScGlobal::pLocaleData->getDate( Date( Date::SYSTEM ) );
    maNoteData.maAuthor = SvtUserOptions().GetID();
}

const OutlinerParaObject* ScPostIt::GetOutlinerObject() const
{
    if( maNoteData.mxCaption )
        return maNoteData.mxCaption->GetOutlinerParaObject();
    if( maNoteData.mxInitData.get() )
        return maNoteData.mxInitData->mxOutlinerObj.get();
    return nullptr;
}

const EditTextObject* ScPostIt::GetEditTextObject() const
{
    const OutlinerParaObject* pOPO = GetOutlinerObject();
    return pOPO ? &pOPO->GetTextObject() : nullptr;
}

OUString ScPostIt::GetText() const
{
    if( const EditTextObject* pEditObj = GetEditTextObject() )
    {
        OUStringBuffer aBuffer;
        ScNoteEditEngine& rEngine = mrDoc.GetNoteEngine();
        rEngine.SetText(*pEditObj);
        sal_Int32 nParaCount = rEngine.GetParagraphCount();
        for( sal_Int32 nPara = 0; nPara < nParaCount; ++nPara )
        {
            if( nPara > 0 )
                aBuffer.append( '\n' );
            aBuffer.append(rEngine.GetText(nPara));
        }
        return aBuffer.makeStringAndClear();
    }
    if( maNoteData.mxInitData.get() )
        return maNoteData.mxInitData->maSimpleText;
    return OUString();
}

bool ScPostIt::HasMultiLineText() const
{
    if( const EditTextObject* pEditObj = GetEditTextObject() )
        return pEditObj->GetParagraphCount() > 1;
    if( maNoteData.mxInitData.get() )
        return maNoteData.mxInitData->maSimpleText.indexOf( '\n' ) >= 0;
    return false;
}

void ScPostIt::SetText( const ScAddress& rPos, const OUString& rText )
{
    CreateCaptionFromInitData( rPos );
    if( maNoteData.mxCaption )
        maNoteData.mxCaption->SetText( rText );
}

SdrCaptionObj* ScPostIt::GetOrCreateCaption( const ScAddress& rPos ) const
{
    CreateCaptionFromInitData( rPos );
    return maNoteData.mxCaption.get();
}

void ScPostIt::ForgetCaption( bool bPreserveData )
{
    if (bPreserveData)
    {
        // Used in clipboard when the originating document is destructed to be
        // able to paste into another document. Caption size and relative
        // position are not preserved but default created when pasted. Also the
        // MergedItemSet can not be carried over or it had to be adapted to
        // defaults and pool. At least preserve the text and outline object if
        // possible.
        ScCaptionInitData* pInitData = new ScCaptionInitData;
        const OutlinerParaObject* pOPO = GetOutlinerObject();
        if (pOPO)
            pInitData->mxOutlinerObj.reset( new OutlinerParaObject(*pOPO));
        pInitData->maSimpleText = GetText();

        maNoteData.mxInitData.reset(pInitData);
        maNoteData.mxCaption.forget();
    }
    else
    {
        /*  This function is used in undo actions to give up the responsibility for
            the caption object which is handled by separate drawing undo actions. */
        maNoteData.mxCaption.forget();
        maNoteData.mxInitData.reset();
    }
}

void ScPostIt::ShowCaption( const ScAddress& rPos, bool bShow )
{
    CreateCaptionFromInitData( rPos );
    // no separate drawing undo needed, handled completely inside ScUndoShowHideNote
    maNoteData.mbShown = bShow;
    if( maNoteData.mxCaption )
        ScCaptionUtil::SetCaptionLayer( *maNoteData.mxCaption, bShow );
}

void ScPostIt::ShowCaptionTemp( const ScAddress& rPos, bool bShow )
{
    CreateCaptionFromInitData( rPos );
    if( maNoteData.mxCaption )
        ScCaptionUtil::SetCaptionLayer( *maNoteData.mxCaption, maNoteData.mbShown || bShow );
}

void ScPostIt::UpdateCaptionPos( const ScAddress& rPos )
{
    CreateCaptionFromInitData( rPos );
    if( maNoteData.mxCaption )
    {
        ScCaptionCreator aCreator( mrDoc, rPos, maNoteData.mxCaption );
        aCreator.UpdateCaptionPos();
    }
}

// private --------------------------------------------------------------------

void ScPostIt::CreateCaptionFromInitData( const ScAddress& rPos ) const
{
    // Captions are not created in Undo documents and only rarely in Clipboard,
    // but otherwise we need caption or initial data.
    assert((maNoteData.mxCaption || maNoteData.mxInitData.get()) || mrDoc.IsUndo() || mrDoc.IsClipboard());
    if( maNoteData.mxInitData.get() )
    {
        /*  This function is called from ScPostIt::Clone() when copying cells
            to the clipboard/undo document, and when copying cells from the
            clipboard/undo document. The former should always be called first,
            so if called in an clipboard/undo document, the caption should have
            been created already. However, for clipboard in case the
            originating document was destructed a new caption has to be
            created. */
        OSL_ENSURE( !mrDoc.IsUndo() && (!mrDoc.IsClipboard() || !maNoteData.mxCaption),
                "ScPostIt::CreateCaptionFromInitData - note caption should not be created in undo/clip documents" );

        /*  #i104915# Never try to create notes in Undo document, leads to
            crash due to missing document members (e.g. row height array). */
        if( !maNoteData.mxCaption && !mrDoc.IsUndo() )
        {
            if (mrDoc.IsClipboard())
                mrDoc.InitDrawLayer();  // ensure there is a drawing layer

            // ScNoteCaptionCreator c'tor creates the caption and inserts it into the document and maNoteData
            ScNoteCaptionCreator aCreator( mrDoc, rPos, maNoteData );
            if( maNoteData.mxCaption )
            {
                // Prevent triple change broadcasts of the same object.
                maNoteData.mxCaption->getSdrModelFromSdrObject().setLock(true);
                ScCaptionInitData& rInitData = *maNoteData.mxInitData;

                // transfer ownership of outliner object to caption, or set simple text
                OSL_ENSURE( rInitData.mxOutlinerObj.get() || !rInitData.maSimpleText.isEmpty(),
                    "ScPostIt::CreateCaptionFromInitData - need either outliner para object or simple text" );
                if (rInitData.mxOutlinerObj)
                    maNoteData.mxCaption->SetOutlinerParaObject( std::move(rInitData.mxOutlinerObj) );
                else
                    maNoteData.mxCaption->SetText( rInitData.maSimpleText );

                // copy all items or set default items; reset shadow items
                ScCaptionUtil::SetDefaultItems( *maNoteData.mxCaption, mrDoc );
                if (rInitData.mxItemSet)
                    ScCaptionUtil::SetCaptionItems( *maNoteData.mxCaption, *rInitData.mxItemSet );

                // set position and size of the caption object
                if( rInitData.mbDefaultPosSize )
                {
                    // set other items and fit caption size to text
                    maNoteData.mxCaption->SetMergedItem( makeSdrTextMinFrameWidthItem( SC_NOTECAPTION_WIDTH ) );
                    maNoteData.mxCaption->SetMergedItem( makeSdrTextMaxFrameWidthItem( SC_NOTECAPTION_MAXWIDTH_TEMP ) );
                    maNoteData.mxCaption->AdjustTextFrameWidthAndHeight();
                    aCreator.AutoPlaceCaption();
                }
                else
                {
                    tools::Rectangle aCellRect = ScDrawLayer::GetCellRect( mrDoc, rPos, true );
                    bool bNegPage = mrDoc.IsNegativePage( rPos.Tab() );
                    long nPosX = bNegPage ? (aCellRect.Left() - rInitData.maCaptionOffset.X()) : (aCellRect.Right() + rInitData.maCaptionOffset.X());
                    long nPosY = aCellRect.Top() + rInitData.maCaptionOffset.Y();
                    tools::Rectangle aCaptRect( Point( nPosX, nPosY ), rInitData.maCaptionSize );
                    maNoteData.mxCaption->SetLogicRect( aCaptRect );
                    aCreator.FitCaptionToRect();
                }

                // End prevent triple change broadcasts of the same object.
                maNoteData.mxCaption->getSdrModelFromSdrObject().setLock(false);
                maNoteData.mxCaption->BroadcastObjectChange();
            }
        }
        // forget the initial caption data struct
        maNoteData.mxInitData.reset();
    }
}

void ScPostIt::CreateCaption( const ScAddress& rPos, const SdrCaptionObj* pCaption )
{
    OSL_ENSURE( !maNoteData.mxCaption, "ScPostIt::CreateCaption - unexpected caption object found" );
    maNoteData.mxCaption.reset(nullptr);

    /*  #i104915# Never try to create notes in Undo document, leads to
        crash due to missing document members (e.g. row height array). */
    OSL_ENSURE( !mrDoc.IsUndo(), "ScPostIt::CreateCaption - note caption should not be created in undo documents" );
    if( mrDoc.IsUndo() )
        return;

    // drawing layer may be missing, if a note is copied into a clipboard document
    if( mrDoc.IsClipboard() )
        mrDoc.InitDrawLayer();

    // ScNoteCaptionCreator c'tor creates the caption and inserts it into the document and maNoteData
    ScNoteCaptionCreator aCreator( mrDoc, rPos, maNoteData );
    if( maNoteData.mxCaption )
    {
        // clone settings of passed caption
        if( pCaption )
        {
            // copy edit text object (object must be inserted into page already)
            if( OutlinerParaObject* pOPO = pCaption->GetOutlinerParaObject() )
                maNoteData.mxCaption->SetOutlinerParaObject( std::make_unique<OutlinerParaObject>( *pOPO ) );
            // copy formatting items (after text has been copied to apply font formatting)
            maNoteData.mxCaption->SetMergedItemSetAndBroadcast( pCaption->GetMergedItemSet() );
            // move textbox position relative to new cell, copy textbox size
            tools::Rectangle aCaptRect = pCaption->GetLogicRect();
            Point aDist = maNoteData.mxCaption->GetTailPos() - pCaption->GetTailPos();
            aCaptRect.Move( aDist.X(), aDist.Y() );
            maNoteData.mxCaption->SetLogicRect( aCaptRect );
            aCreator.FitCaptionToRect();
        }
        else
        {
            // set default formatting and default position
            ScCaptionUtil::SetDefaultItems( *maNoteData.mxCaption, mrDoc );
            aCreator.AutoPlaceCaption();
        }

        // create undo action
        if( ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer() )
            if( pDrawLayer->IsRecording() )
                pDrawLayer->AddCalcUndo( std::make_unique<SdrUndoNewObj>( *maNoteData.mxCaption ) );
    }
}

void ScPostIt::RemoveCaption()
{
    if (!maNoteData.mxCaption)
        return;

    /*  Remove caption object only, if this note is its owner (e.g. notes in
        undo documents refer to captions in original document, do not remove
        them from drawing layer here). */
    // TTTT maybe no longer needed - can that still happen?
    ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer();
    if (pDrawLayer == &maNoteData.mxCaption->getSdrModelFromSdrObject())
        maNoteData.mxCaption.removeFromDrawPageAndFree();

    SAL_INFO("sc.core","ScPostIt::RemoveCaption - refs: " << maNoteData.mxCaption.getRefs() <<
            "  IsUndo: " << mrDoc.IsUndo() << "  IsClip: " << mrDoc.IsClipboard() <<
            "  Dtor: " << mrDoc.IsInDtorClear());

    // Forget the caption object if removeFromDrawPageAndFree() did not free it.
    if (maNoteData.mxCaption)
    {
        SAL_INFO("sc.core","ScPostIt::RemoveCaption - forgetting one ref");
        maNoteData.mxCaption.forget();
    }
}

ScCaptionPtr ScNoteUtil::CreateTempCaption(
        ScDocument& rDoc, const ScAddress& rPos, SdrPage& rDrawPage,
        const OUString& rUserText, const tools::Rectangle& rVisRect, bool bTailFront )
{
    OUStringBuffer aBuffer( rUserText );
    // add plain text of invisible (!) cell note (no formatting etc.)
    SdrCaptionObj* pNoteCaption = nullptr;
    const ScPostIt* pNote = rDoc.GetNote( rPos );
    if( pNote && !pNote->IsCaptionShown() )
    {
        if( !aBuffer.isEmpty() )
            aBuffer.append( "\n--------\n" ).append( pNote->GetText() );
        pNoteCaption = pNote->GetOrCreateCaption( rPos );
    }

    // create a caption if any text exists
    if( !pNoteCaption && aBuffer.isEmpty() )
        return ScCaptionPtr();

    // prepare visible rectangle (add default distance to all borders)
    tools::Rectangle aVisRect(
        rVisRect.Left() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Top() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Right() - SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Bottom() - SC_NOTECAPTION_BORDERDIST_TEMP );

    // create the caption object
    ScCaptionCreator aCreator( rDoc, rPos, bTailFront );

    // insert caption into page (needed to set caption text)
    aCreator.GetCaption().insertToDrawPage( rDrawPage );

    SdrCaptionObj* pCaption = aCreator.GetCaption().get();  // just for ease of use

    // clone the edit text object, unless user text is present, then set this text
    if( pNoteCaption && rUserText.isEmpty() )
    {
        if( OutlinerParaObject* pOPO = pNoteCaption->GetOutlinerParaObject() )
            pCaption->SetOutlinerParaObject( std::make_unique<OutlinerParaObject>( *pOPO ) );
        // set formatting (must be done after setting text) and resize the box to fit the text
        pCaption->SetMergedItemSetAndBroadcast( pNoteCaption->GetMergedItemSet() );
        tools::Rectangle aCaptRect( pCaption->GetLogicRect().TopLeft(), pNoteCaption->GetLogicRect().GetSize() );
        pCaption->SetLogicRect( aCaptRect );
    }
    else
    {
        // if pNoteCaption is null, then aBuffer contains some text
        pCaption->SetText( aBuffer.makeStringAndClear() );
        ScCaptionUtil::SetDefaultItems( *pCaption, rDoc );
        // adjust caption size to text size
        long nMaxWidth = ::std::min< long >( aVisRect.GetWidth() * 2 / 3, SC_NOTECAPTION_MAXWIDTH_TEMP );
        pCaption->SetMergedItem( makeSdrTextAutoGrowWidthItem( true ) );
        pCaption->SetMergedItem( makeSdrTextMinFrameWidthItem( SC_NOTECAPTION_WIDTH ) );
        pCaption->SetMergedItem( makeSdrTextMaxFrameWidthItem( nMaxWidth ) );
        pCaption->SetMergedItem( makeSdrTextAutoGrowHeightItem( true ) );
        pCaption->AdjustTextFrameWidthAndHeight();
    }

    // move caption into visible area
    aCreator.AutoPlaceCaption( &aVisRect );

    // XXX Note it is already inserted to the draw page.
    return aCreator.GetCaption();
}

ScPostIt* ScNoteUtil::CreateNoteFromCaption(
        ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj* pCaption )
{
    ScNoteData aNoteData( true/*bShown*/ );
    aNoteData.mxCaption.reset( pCaption );
    ScPostIt* pNote = new ScPostIt( rDoc, rPos, aNoteData, false );
    pNote->AutoStamp();

    rDoc.SetNote(rPos, std::unique_ptr<ScPostIt>(pNote));

    // ScNoteCaptionCreator c'tor updates the caption object to be part of a note
    ScNoteCaptionCreator aCreator( rDoc, rPos, aNoteData.mxCaption, true/*bShown*/ );

    return pNote;
}

ScPostIt* ScNoteUtil::CreateNoteFromObjectData(
        ScDocument& rDoc, const ScAddress& rPos, std::unique_ptr<SfxItemSet> pItemSet,
        OutlinerParaObject* pOutlinerObj, const tools::Rectangle& rCaptionRect,
        bool bShown )
{
    OSL_ENSURE( pItemSet && pOutlinerObj, "ScNoteUtil::CreateNoteFromObjectData - item set and outliner object expected" );
    ScNoteData aNoteData( bShown );
    aNoteData.mxInitData.reset( new ScCaptionInitData );
    ScCaptionInitData& rInitData = *aNoteData.mxInitData;
    rInitData.mxItemSet = std::move(pItemSet);
    rInitData.mxOutlinerObj.reset( pOutlinerObj );

    // convert absolute caption position to relative position
    rInitData.mbDefaultPosSize = rCaptionRect.IsEmpty();
    if( !rInitData.mbDefaultPosSize )
    {
        tools::Rectangle aCellRect = ScDrawLayer::GetCellRect( rDoc, rPos, true );
        bool bNegPage = rDoc.IsNegativePage( rPos.Tab() );
        rInitData.maCaptionOffset.setX( bNegPage ? (aCellRect.Left() - rCaptionRect.Right()) : (rCaptionRect.Left() - aCellRect.Right()) );
        rInitData.maCaptionOffset.setY( rCaptionRect.Top() - aCellRect.Top() );
        rInitData.maCaptionSize = rCaptionRect.GetSize();
    }

    /*  Create the note and insert it into the document. If the note is
        visible, the caption object will be created automatically. */
    ScPostIt* pNote = new ScPostIt( rDoc, rPos, aNoteData, /*bAlwaysCreateCaption*/false, 0/*nPostItId*/ );
    pNote->AutoStamp();

    rDoc.SetNote(rPos, std::unique_ptr<ScPostIt>(pNote));

    return pNote;
}

ScPostIt* ScNoteUtil::CreateNoteFromString(
        ScDocument& rDoc, const ScAddress& rPos, const OUString& rNoteText,
        bool bShown, bool bAlwaysCreateCaption, sal_uInt32 nPostItId )
{
    ScPostIt* pNote = nullptr;
    if( !rNoteText.isEmpty() )
    {
        ScNoteData aNoteData( bShown );
        aNoteData.mxInitData.reset( new ScCaptionInitData );
        ScCaptionInitData& rInitData = *aNoteData.mxInitData;
        rInitData.maSimpleText = rNoteText;
        rInitData.mbDefaultPosSize = true;

        /*  Create the note and insert it into the document. If the note is
            visible, the caption object will be created automatically. */
        pNote = new ScPostIt( rDoc, rPos, aNoteData, bAlwaysCreateCaption, nPostItId );
        pNote->AutoStamp();
        //insert takes ownership
        rDoc.SetNote(rPos, std::unique_ptr<ScPostIt>(pNote));
    }
    return pNote;
}

namespace sc {

NoteEntry::NoteEntry( const ScAddress& rPos, const ScPostIt* pNote ) :
    maPos(rPos), mpNote(pNote) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
