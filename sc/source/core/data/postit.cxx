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

#include <table.hxx>
#include <document.hxx>
#include <docpool.hxx>
#include <patattr.hxx>
#include <formulacell.hxx>
#include <drwlayer.hxx>
#include <undocell.hxx>
#include <userdat.hxx>
#include <detfunc.hxx>
#include <editutil.hxx>
#include <o3tl/make_unique.hxx>

#include <utility>

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
    explicit            ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, const std::shared_ptr< SdrCaptionObj >& pCaption );

    /** Returns the drawing layer page of the sheet contained in maPos. */
    SdrPage*            GetDrawPage();
    /** Returns the caption drawing object. */
    const std::shared_ptr< SdrCaptionObj >& GetCaption() { return m_pCaption; }

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
    std::shared_ptr< SdrCaptionObj > m_pCaption;
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

ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, const std::shared_ptr< SdrCaptionObj >& pCaption ) :
    mrDoc( rDoc ),
    maPos( rPos ),
    m_pCaption( pCaption )
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
    Point aTailPos = m_pCaption->GetTailPos();
    aTailPos.setX( ::std::max( ::std::min( aTailPos.X(), rVisRect.Right() ), rVisRect.Left() ) );
    aTailPos.setY( ::std::max( ::std::min( aTailPos.Y(), rVisRect.Bottom() ), rVisRect.Top() ) );
    m_pCaption->SetTailPos( aTailPos );

    // caption rectangle
    tools::Rectangle aCaptRect = m_pCaption->GetLogicRect();
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
    m_pCaption->SetLogicRect( aCaptRect );
}

void ScCaptionCreator::AutoPlaceCaption( const tools::Rectangle* pVisRect )
{
    const tools::Rectangle& rVisRect = GetVisRect( pVisRect );

    // caption rectangle
    tools::Rectangle aCaptRect = m_pCaption->GetLogicRect();
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
    m_pCaption->SetLogicRect( aCaptRect );
    FitCaptionToRect( pVisRect );
}

void ScCaptionCreator::UpdateCaptionPos()
{
    ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer();

    // update caption position
    const Point& rOldTailPos = m_pCaption->GetTailPos();
    Point aTailPos = CalcTailPos( false );
    if( rOldTailPos != aTailPos )
    {
        // create drawing undo action
        if( pDrawLayer && pDrawLayer->IsRecording() )
            pDrawLayer->AddCalcUndo( new SdrUndoGeoObj( *m_pCaption ) );
        // calculate new caption rectangle (#i98141# handle LTR<->RTL switch correctly)
        tools::Rectangle aCaptRect = m_pCaption->GetLogicRect();
        long nDiffX = (rOldTailPos.X() >= 0) ? (aCaptRect.Left() - rOldTailPos.X()) : (rOldTailPos.X() - aCaptRect.Right());
        if( mbNegPage ) nDiffX = -nDiffX - aCaptRect.GetWidth();
        long nDiffY = aCaptRect.Top() - rOldTailPos.Y();
        aCaptRect.SetPos( aTailPos + Point( nDiffX, nDiffY ) );
        // set new tail position and caption rectangle
        m_pCaption->SetTailPos( aTailPos );
        m_pCaption->SetLogicRect( aCaptRect );
        // fit caption into draw page
        FitCaptionToRect();
    }

    // update cell position in caption user data
    ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( m_pCaption.get(), maPos.Tab() );
    if( pCaptData && (maPos != pCaptData->maStart) )
    {
        // create drawing undo action
        if( pDrawLayer && pDrawLayer->IsRecording() )
            pDrawLayer->AddCalcUndo( new ScUndoObjData( m_pCaption.get(), pCaptData->maStart, pCaptData->maEnd, maPos, pCaptData->maEnd ) );
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
    m_pCaption.reset(
        new SdrCaptionObj(
            *mrDoc.GetDrawLayer(), // TTTT should ret a ref?
            aTextRect,
            aTailPos),
        [](SdrCaptionObj* pCaptionObj){
            SdrPage* pDrawPage(pCaptionObj->getSdrPageFromSdrObject());
            if (pDrawPage)
            {
                pDrawPage->RemoveObject(pCaptionObj->GetOrdNum());
            }
            SdrObject* pObj = pCaptionObj;
            SdrObject::Free(pObj);
    });
    // basic caption settings
    ScCaptionUtil::SetBasicCaptionSettings( *m_pCaption, bShown );
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
    explicit            ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, const std::shared_ptr< SdrCaptionObj >& xCaption, bool bShown );
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
        rNoteData.m_pCaption = GetCaption();
        OSL_ENSURE( rNoteData.m_pCaption, "ScNoteCaptionCreator::ScNoteCaptionCreator - missing caption object" );
        if( rNoteData.m_pCaption )
        {
            // store note position in user data of caption object
            ScCaptionUtil::SetCaptionUserData( *rNoteData.m_pCaption, rPos );
            // insert object into draw page
            pDrawPage->InsertObject( rNoteData.m_pCaption.get() );
        }
    }
}

ScNoteCaptionCreator::ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, const std::shared_ptr< SdrCaptionObj >& pCaption, bool bShown ) :
    ScCaptionCreator( rDoc, rPos, pCaption )
{
    SdrPage* pDrawPage = GetDrawPage();
    OSL_ENSURE( pDrawPage, "ScNoteCaptionCreator::ScNoteCaptionCreator - no drawing page" );
    OSL_ENSURE( pCaption->getSdrPageFromSdrObject() == pDrawPage, "ScNoteCaptionCreator::ScNoteCaptionCreator - wrong drawing page in caption" );
    if( pDrawPage && (pCaption->getSdrPageFromSdrObject() == pDrawPage) )
    {
        // store note position in user data of caption object
        ScCaptionUtil::SetCaptionUserData( *pCaption, rPos );
        // basic caption settings
        ScCaptionUtil::SetBasicCaptionSettings( *pCaption, bShown );
        // set correct tail position
        pCaption->SetTailPos( CalcTailPos( false ) );
    }
}

void removeFromDrawPageAndFree( const std::shared_ptr< SdrCaptionObj >& pCaption )
{
    SdrPage* pDrawPage(pCaption->getSdrPageFromSdrObject());
    SAL_WARN_IF( !pDrawPage, "sc.core", "ScCaptionPtr::removeFromDrawPageAndFree - object without drawing page");
    if (pDrawPage)
    {
        pDrawPage->RecalcObjOrdNums();
        bool bRecording = false;
        ScDrawLayer* pDrawLayer(dynamic_cast< ScDrawLayer* >(&pCaption->getSdrModelFromSdrObject()));
        SAL_WARN_IF( !pDrawLayer, "sc.core", "ScCaptionPtr::removeFromDrawPageAndFree - object without drawing layer");
        // create drawing undo action (before removing the object to have valid draw page in undo action)
        bRecording = (pDrawLayer && pDrawLayer->IsRecording());
        if (bRecording)
            pDrawLayer->AddCalcUndo( new ScUndoDelSdrCaptionObj( pCaption ));
        // remove the object from the drawing page, delete if undo is disabled
        pDrawPage->RemoveObject( pCaption->GetOrdNum() );
    }
}

} // namespace

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
    CreateCaption( rPos, nullptr );
}

ScPostIt::ScPostIt( ScDocument& rDoc, const ScAddress& rPos, const ScPostIt& rNote, sal_uInt32 nPostItId ) :
    mrDoc( rDoc ),
    maNoteData( rNote.maNoteData )
{
    mnPostItId = nPostItId == 0 ? mnLastPostItId++ : nPostItId;
    maNoteData.m_pCaption.reset();
    CreateCaption( rPos, rNote.maNoteData.m_pCaption );
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

ScPostIt* ScPostIt::Clone( const ScAddress& rOwnPos, ScDocument& rDestDoc, const ScAddress& rDestPos, bool bCloneCaption ) const
{
    CreateCaptionFromInitData( rOwnPos );
    return bCloneCaption ? new ScPostIt( rDestDoc, rDestPos, *this, mnPostItId ) : new ScPostIt( rDestDoc, rDestPos, maNoteData, false, mnPostItId );
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
    if( maNoteData.m_pCaption )
        return maNoteData.m_pCaption->GetOutlinerParaObject();
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
    if( maNoteData.m_pCaption )
        maNoteData.m_pCaption->SetText( rText );
}

const std::shared_ptr< SdrCaptionObj>& ScPostIt::GetOrCreateCaption( const ScAddress& rPos ) const
{
    CreateCaptionFromInitData( rPos );
    return maNoteData.m_pCaption;
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
        maNoteData.m_pCaption.reset();
    }
    else
    {
        /*  This function is used in undo actions to give up the responsibility for
            the caption object which is handled by separate drawing undo actions. */
        maNoteData.m_pCaption.reset();
        maNoteData.mxInitData.reset();
    }
}

void ScPostIt::ShowCaption( const ScAddress& rPos, bool bShow )
{
    CreateCaptionFromInitData( rPos );
    // no separate drawing undo needed, handled completely inside ScUndoShowHideNote
    maNoteData.mbShown = bShow;
    if( maNoteData.m_pCaption )
        ScCaptionUtil::SetCaptionLayer( *maNoteData.m_pCaption, bShow );
}

void ScPostIt::ShowCaptionTemp( const ScAddress& rPos, bool bShow )
{
    CreateCaptionFromInitData( rPos );
    if( maNoteData.m_pCaption )
        ScCaptionUtil::SetCaptionLayer( *maNoteData.m_pCaption, maNoteData.mbShown || bShow );
}

void ScPostIt::UpdateCaptionPos( const ScAddress& rPos )
{
    CreateCaptionFromInitData( rPos );
    if( maNoteData.m_pCaption )
    {
        ScCaptionCreator aCreator( mrDoc, rPos, maNoteData.m_pCaption );
        aCreator.UpdateCaptionPos();
    }
}

// private --------------------------------------------------------------------

void ScPostIt::CreateCaptionFromInitData( const ScAddress& rPos ) const
{
    // Captions are not created in Undo documents and only rarely in Clipboard,
    // but otherwise we need caption or initial data.
    assert((maNoteData.m_pCaption || maNoteData.mxInitData.get()) || mrDoc.IsUndo() || mrDoc.IsClipboard());
    if( maNoteData.mxInitData.get() )
    {
        /*  This function is called from ScPostIt::Clone() when copying cells
            to the clipboard/undo document, and when copying cells from the
            clipboard/undo document. The former should always be called first,
            so if called in an clipboard/undo document, the caption should have
            been created already. However, for clipboard in case the
            originating document was destructed a new caption has to be
            created. */
        OSL_ENSURE( !mrDoc.IsUndo() && (!mrDoc.IsClipboard() || !maNoteData.m_pCaption),
                "ScPostIt::CreateCaptionFromInitData - note caption should not be created in undo/clip documents" );

        /*  #i104915# Never try to create notes in Undo document, leads to
            crash due to missing document members (e.g. row height array). */
        if( !maNoteData.m_pCaption && !mrDoc.IsUndo() )
        {
            if (mrDoc.IsClipboard())
                mrDoc.InitDrawLayer();  // ensure there is a drawing layer

            // ScNoteCaptionCreator c'tor creates the caption and inserts it into the document and maNoteData
            ScNoteCaptionCreator aCreator( mrDoc, rPos, maNoteData );
            if( maNoteData.m_pCaption )
            {
                // Prevent triple change broadcasts of the same object.
                SdrDelayBroadcastObjectChange aDelayChange( *maNoteData.m_pCaption);

                ScCaptionInitData& rInitData = *maNoteData.mxInitData;

                // transfer ownership of outliner object to caption, or set simple text
                OSL_ENSURE( rInitData.mxOutlinerObj.get() || !rInitData.maSimpleText.isEmpty(),
                    "ScPostIt::CreateCaptionFromInitData - need either outliner para object or simple text" );
                if( rInitData.mxOutlinerObj.get() )
                    maNoteData.m_pCaption->SetOutlinerParaObject( std::move(rInitData.mxOutlinerObj) );
                else
                    maNoteData.m_pCaption->SetText( rInitData.maSimpleText );

                // copy all items or set default items; reset shadow items
                ScCaptionUtil::SetDefaultItems( *maNoteData.m_pCaption, mrDoc );
                if( rInitData.mxItemSet.get() )
                    ScCaptionUtil::SetCaptionItems( *maNoteData.m_pCaption, *rInitData.mxItemSet );

                // set position and size of the caption object
                if( rInitData.mbDefaultPosSize )
                {
                    // set other items and fit caption size to text
                    maNoteData.m_pCaption->SetMergedItem( makeSdrTextMinFrameWidthItem( SC_NOTECAPTION_WIDTH ) );
                    maNoteData.m_pCaption->SetMergedItem( makeSdrTextMaxFrameWidthItem( SC_NOTECAPTION_MAXWIDTH_TEMP ) );
                    maNoteData.m_pCaption->AdjustTextFrameWidthAndHeight();
                    aCreator.AutoPlaceCaption();
                }
                else
                {
                    tools::Rectangle aCellRect = ScDrawLayer::GetCellRect( mrDoc, rPos, true );
                    bool bNegPage = mrDoc.IsNegativePage( rPos.Tab() );
                    long nPosX = bNegPage ? (aCellRect.Left() - rInitData.maCaptionOffset.X()) : (aCellRect.Right() + rInitData.maCaptionOffset.X());
                    long nPosY = aCellRect.Top() + rInitData.maCaptionOffset.Y();
                    tools::Rectangle aCaptRect( Point( nPosX, nPosY ), rInitData.maCaptionSize );
                    maNoteData.m_pCaption->SetLogicRect( aCaptRect );
                    aCreator.FitCaptionToRect();
                }
            }
        }
        // forget the initial caption data struct
        maNoteData.mxInitData.reset();
    }
}

void ScPostIt::CreateCaption( const ScAddress& rPos, const std::shared_ptr< SdrCaptionObj >& pCaption )
{
    OSL_ENSURE( !maNoteData.m_pCaption, "ScPostIt::CreateCaption - unexpected caption object found" );
    maNoteData.m_pCaption.reset();

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
    if( maNoteData.m_pCaption )
    {
        // clone settings of passed caption
        if( pCaption )
        {
            // copy edit text object (object must be inserted into page already)
            if( OutlinerParaObject* pOPO = pCaption->GetOutlinerParaObject() )
                maNoteData.m_pCaption->SetOutlinerParaObject( o3tl::make_unique<OutlinerParaObject>( *pOPO ) );
            // copy formatting items (after text has been copied to apply font formatting)
            maNoteData.m_pCaption->SetMergedItemSetAndBroadcast( pCaption->GetMergedItemSet() );
            // move textbox position relative to new cell, copy textbox size
            tools::Rectangle aCaptRect = pCaption->GetLogicRect();
            Point aDist = maNoteData.m_pCaption->GetTailPos() - pCaption->GetTailPos();
            aCaptRect.Move( aDist.X(), aDist.Y() );
            maNoteData.m_pCaption->SetLogicRect( aCaptRect );
            aCreator.FitCaptionToRect();
        }
        else
        {
            // set default formatting and default position
            ScCaptionUtil::SetDefaultItems( *maNoteData.m_pCaption, mrDoc );
            aCreator.AutoPlaceCaption();
        }

        // create undo action
        if( ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer() )
            if( pDrawLayer->IsRecording() )
                pDrawLayer->AddCalcUndo( new ScUndoNewSdrCaptionObj( maNoteData.m_pCaption ) );
    }
}

void ScPostIt::RemoveCaption()
{
    if (!maNoteData.m_pCaption)
        return;

    /*  Remove caption object only, if this note is its owner (e.g. notes in
        undo documents refer to captions in original document, do not remove
        them from drawing layer here). */
    // TTTT maybe no longer needed - can that still happen?
    ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer();
    if (pDrawLayer == &maNoteData.m_pCaption->getSdrModelFromSdrObject())
        removeFromDrawPageAndFree(maNoteData.m_pCaption);

    maNoteData.m_pCaption.reset();
}

std::shared_ptr< SdrCaptionObj > ScNoteUtil::CreateTempCaption(
        ScDocument& rDoc, const ScAddress& rPos, SdrPage& rDrawPage,
        const OUString& rUserText, const tools::Rectangle& rVisRect, bool bTailFront )
{
    OUStringBuffer aBuffer( rUserText );
    // add plain text of invisible (!) cell note (no formatting etc.)
    std::shared_ptr< SdrCaptionObj > pNoteCaption = nullptr;
    const ScPostIt* pNote = rDoc.GetNote( rPos );
    if( pNote && !pNote->IsCaptionShown() )
    {
        if( !aBuffer.isEmpty() )
            aBuffer.append( "\n--------\n" ).append( pNote->GetText() );
        pNoteCaption = pNote->GetOrCreateCaption( rPos );
    }

    // create a caption if any text exists
    if( !pNoteCaption && aBuffer.isEmpty() )
        return std::shared_ptr< SdrCaptionObj >();

    // prepare visible rectangle (add default distance to all borders)
    tools::Rectangle aVisRect(
        rVisRect.Left() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Top() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Right() - SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Bottom() - SC_NOTECAPTION_BORDERDIST_TEMP );

    // create the caption object
    ScCaptionCreator aCreator( rDoc, rPos, bTailFront );

    // insert caption into page (needed to set caption text)
    rDrawPage.InsertObject( aCreator.GetCaption().get() );

    SdrCaptionObj* pCaption = aCreator.GetCaption().get();  // just for ease of use

    // clone the edit text object, unless user text is present, then set this text
    if( pNoteCaption && rUserText.isEmpty() )
    {
        if( OutlinerParaObject* pOPO = pNoteCaption->GetOutlinerParaObject() )
            pCaption->SetOutlinerParaObject( o3tl::make_unique<OutlinerParaObject>( *pOPO ) );
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
    aNoteData.m_pCaption.reset(pCaption, [](SdrCaptionObj* pCaptionObj) {
        SdrPage* pDrawPage(pCaptionObj->getSdrPageFromSdrObject());
        if (pDrawPage)
        {
            pDrawPage->RemoveObject(pCaptionObj->GetOrdNum());
        }
        SdrObject* pObj = pCaptionObj;
        SdrObject::Free(pObj);
    });
    ScPostIt* pNote = new ScPostIt( rDoc, rPos, aNoteData, false );
    pNote->AutoStamp();

    rDoc.SetNote(rPos, pNote);

    // ScNoteCaptionCreator c'tor updates the caption object to be part of a note
    ScNoteCaptionCreator aCreator( rDoc, rPos, aNoteData.m_pCaption, true/*bShown*/ );

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

    rDoc.SetNote(rPos, pNote);

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
        rDoc.SetNote(rPos, pNote);
    }
    return pNote;
}

namespace sc {

NoteEntry::NoteEntry( const ScAddress& rPos, const ScPostIt* pNote ) :
    maPos(rPos), mpNote(pNote) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
