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
#include <svx/xfillit0.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/xflclit.hxx>
#include <svx/sdshitm.hxx>
#include <svx/sdsxyitm.hxx>
#include <svx/sdtditm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtmfitm.hxx>
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
    static void         SetDefaultItems( SdrCaptionObj& rCaption, ScDocument& rDoc, const SfxItemSet* pExtraItemSet );
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

void ScCaptionUtil::SetDefaultItems( SdrCaptionObj& rCaption, ScDocument& rDoc, const SfxItemSet* pExtraItemSet )
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

    if (pExtraItemSet)
    {
        /* Updates caption item set according to the passed item set while removing shadow items. */

        aItemSet.MergeValues(*pExtraItemSet);
        // reset shadow items
        aItemSet.Put( makeSdrShadowItem( false ) );
        aItemSet.Put( makeSdrShadowXDistItem( 100 ) );
        aItemSet.Put( makeSdrShadowYDistItem( 100 ) );
    }

    rCaption.SetMergedItemSet( aItemSet );

    if (pExtraItemSet)
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
    explicit            ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj* pCaption );

    /** Returns the drawing layer page of the sheet contained in maPos. */
    SdrPage*            GetDrawPage();
    /** Returns the caption drawing object. */
    SdrCaptionObj*      GetCaption() { return mpCaption; }

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
    void                CreateCaption( bool bTailFront );

private:
    /** Initializes all members. */
    void                Initialize();
    /** Returns the passed rectangle if existing, page rectangle otherwise. */
    const tools::Rectangle& GetVisRect( const tools::Rectangle* pVisRect ) const { return pVisRect ? *pVisRect : maPageRect; }

private:
    ScDocument&         mrDoc;
    ScAddress           maPos;
    SdrCaptionObj*      mpCaption;
    tools::Rectangle    maPageRect;
    tools::Rectangle    maCellRect;
    bool                mbNegPage;
};

ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, bool bTailFront ) :
    mrDoc( rDoc ),
    maPos( rPos )
{
    Initialize();
    CreateCaption( bTailFront );
}

ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj* pCaption ) :
    mrDoc( rDoc ),
    maPos( rPos ),
    mpCaption( pCaption )
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
    Point aTailPos = mpCaption->GetTailPos();
    aTailPos.setX( ::std::max( ::std::min( aTailPos.X(), rVisRect.Right() ), rVisRect.Left() ) );
    aTailPos.setY( ::std::max( ::std::min( aTailPos.Y(), rVisRect.Bottom() ), rVisRect.Top() ) );
    mpCaption->SetTailPos( aTailPos );

    // caption rectangle
    tools::Rectangle aCaptRect = mpCaption->GetLogicRect();
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
    mpCaption->SetLogicRect( aCaptRect );
}

void ScCaptionCreator::AutoPlaceCaption( const tools::Rectangle* pVisRect )
{
    const tools::Rectangle& rVisRect = GetVisRect( pVisRect );

    // caption rectangle
    tools::Rectangle aCaptRect = mpCaption->GetLogicRect();
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
    mpCaption->SetLogicRect( aCaptRect );
    FitCaptionToRect( pVisRect );
}

void ScCaptionCreator::UpdateCaptionPos()
{
    ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer();

    // update caption position
    const Point& rOldTailPos = mpCaption->GetTailPos();
    Point aTailPos = CalcTailPos( false );
    if( rOldTailPos != aTailPos )
    {
        // create drawing undo action
        if( pDrawLayer && pDrawLayer->IsRecording() )
            pDrawLayer->AddCalcUndo( std::make_unique<SdrUndoGeoObj>( *mpCaption ) );
        // calculate new caption rectangle (#i98141# handle LTR<->RTL switch correctly)
        tools::Rectangle aCaptRect = mpCaption->GetLogicRect();
        long nDiffX = (rOldTailPos.X() >= 0) ? (aCaptRect.Left() - rOldTailPos.X()) : (rOldTailPos.X() - aCaptRect.Right());
        if( mbNegPage ) nDiffX = -nDiffX - aCaptRect.GetWidth();
        long nDiffY = aCaptRect.Top() - rOldTailPos.Y();
        aCaptRect.SetPos( aTailPos + Point( nDiffX, nDiffY ) );
        // set new tail position and caption rectangle
        mpCaption->SetTailPos( aTailPos );
        mpCaption->SetLogicRect( aCaptRect );
        // fit caption into draw page
        FitCaptionToRect();
    }

    // update cell position in caption user data
    ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( mpCaption, maPos.Tab() );
    if( pCaptData && (maPos != pCaptData->maStart) )
    {
        // create drawing undo action
        if( pDrawLayer && pDrawLayer->IsRecording() )
            pDrawLayer->AddCalcUndo( std::make_unique<ScUndoObjData>( mpCaption, pCaptData->maStart, pCaptData->maEnd, maPos, pCaptData->maEnd ) );
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

void ScCaptionCreator::CreateCaption( bool bTailFront )
{
    // create the caption drawing object
    tools::Rectangle aTextRect( Point( 0 , 0 ), Size( SC_NOTECAPTION_WIDTH, SC_NOTECAPTION_HEIGHT ) );
    Point aTailPos = CalcTailPos( bTailFront );
    mpCaption =
        new SdrCaptionObj(
            *mrDoc.GetDrawLayer(), // TTTT should ret a ref?
            aTextRect,
            aTailPos);
    // basic caption settings
    ScCaptionUtil::SetBasicCaptionSettings( *mpCaption, /*bShown*/true );
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

}; // anonymous namespace

/** Helper for creation of permanent caption drawing objects for cell notes. */
class ScNoteCaptionCreator : public ScCaptionCreator
{
public:
    /** Create a new caption object and inserts it into the document. */
    explicit            ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, ScPostIt& rPostIt );
    /** Manipulate an existing caption. */
    explicit            ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj* pCaption );
};

ScNoteCaptionCreator::ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, ScPostIt& rPostIt ) :
    ScCaptionCreator( rDoc, rPos )  // use helper c'tor that does not create the caption yet
{
    assert( !rPostIt.mbShown );
    SdrPage* pDrawPage = GetDrawPage();
    // create the caption drawing object
    CreateCaption( false );
    rPostIt.maInitData.mxItemSet.reset();
    rPostIt.maInitData.mxOutlinerObj.reset();
    rPostIt.maInitData.maSimpleText.~OUString();
    rPostIt.mbShown = true;
    rPostIt.mpCaption = GetCaption();
    // store note position in user data of caption object
    ScCaptionUtil::SetCaptionUserData( *rPostIt.mpCaption, rPos );
    // insert object into draw page
    pDrawPage->InsertObject( rPostIt.mpCaption );
}

ScNoteCaptionCreator::ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj* pCaption ) :
    ScCaptionCreator( rDoc, rPos, pCaption )
{
    assert( pCaption->getSdrPageFromSdrObject() == GetDrawPage() && "ScNoteCaptionCreator::ScNoteCaptionCreator - wrong drawing page in caption" );
    // store note position in user data of caption object
    ScCaptionUtil::SetCaptionUserData( *pCaption, rPos );
    // basic caption settings
    ScCaptionUtil::SetBasicCaptionSettings( *pCaption, /*bShown*/true );
    // set correct tail position
    pCaption->SetTailPos( CalcTailPos( false ) );
}

ScNoteDataSaved::ScNoteDataSaved() = default;
ScNoteDataSaved::ScNoteDataSaved(ScNoteDataSaved&&) = default;
ScNoteDataSaved& ScNoteDataSaved::operator=(ScNoteDataSaved&&) = default;
ScNoteDataSaved::~ScNoteDataSaved() = default;

sal_uInt32 ScPostIt::mnLastPostItId = 1;

ScPostIt::ScPostIt( ScDocument& rDoc ) :
    mrDoc( rDoc ),
    mbShown( false )
{
    mnPostItId = mnLastPostItId++;
    new (&maInitData) ScInitData();
    maInitData.mbDefaultPosSize = true;
    AutoStamp();
}

ScPostIt::ScPostIt( ScDocument& rDoc, const ScPostIt& rPostIt ) :
    mrDoc( rDoc ),
    maDate( rPostIt.maDate ),
    maAuthor( rPostIt.maAuthor ),
    mbShown( false )
{
    mnPostItId = mnLastPostItId++;
    new (&maInitData) ScInitData();
    maInitData.mxItemSet.reset(rPostIt.maInitData.mxItemSet ? new SfxItemSet(*rPostIt.maInitData.mxItemSet) : nullptr);
    maInitData.mxOutlinerObj.reset(rPostIt.maInitData.mxOutlinerObj ? new OutlinerParaObject(*rPostIt.maInitData.mxOutlinerObj) : nullptr);
    maInitData.maSimpleText = rPostIt.maInitData.maSimpleText;
    maInitData.maCaptionOffset = rPostIt.maInitData.maCaptionOffset;
    maInitData.maCaptionSize = rPostIt.maInitData.maCaptionSize;
    maInitData.mbDefaultPosSize = rPostIt.maInitData.mbDefaultPosSize;
}

ScPostIt::ScPostIt( ScDocument& rDoc,
                    std::unique_ptr<SfxItemSet> pItemSet,
                    OutlinerParaObject* pOutlinerObj,
                    Point aCaptionOffset, Size aCaptionSize, bool bDefaultPosSize)
  : mrDoc( rDoc ),
    mbShown( false )
{
    mnPostItId = mnLastPostItId++;
    new (&maInitData) ScInitData();
    maInitData.mxItemSet = std::move(pItemSet);
    maInitData.mxOutlinerObj.reset(pOutlinerObj);
    maInitData.maCaptionOffset = aCaptionOffset;
    maInitData.maCaptionSize = aCaptionSize;
    maInitData.mbDefaultPosSize = bDefaultPosSize;
}

ScPostIt::ScPostIt( ScDocument& rDoc, const OUString& rNoteText)
  : mrDoc( rDoc ),
    mbShown( false )
{
    mnPostItId = mnLastPostItId++;
    new (&maInitData) ScInitData();
    maInitData.maSimpleText = rNoteText;
    maInitData.mbDefaultPosSize = true;
}

ScPostIt::ScPostIt( ScDocument& rDoc, const ScNoteDataSaved& rSaved)
  : mrDoc( rDoc ),
    mbShown( false )
{
    mnPostItId = rSaved.mnPostItId;
    new (&maInitData) ScInitData();
    // todo
    assert(false);
}

ScPostIt::ScPostIt( ScDocument& rDoc, SdrCaptionObj* pCaption)
  : mrDoc( rDoc ),
    mbShown( true )
{
    mnPostItId = mnLastPostItId++;
    mpCaption = pCaption;
}

ScPostIt::~ScPostIt()
{
    if (mbShown)
        HideCaption();
    maInitData.~ScInitData();
}

std::unique_ptr<ScPostIt> ScPostIt::Clone( ScDocument& rDestDoc ) const
{
    return std::make_unique<ScPostIt>( rDestDoc, *this );
}

void ScPostIt::SetDate( const OUString& rDate )
{
    maDate = rDate;
}

void ScPostIt::SetAuthor( const OUString& rAuthor )
{
    maAuthor = rAuthor;
}

void ScPostIt::AutoStamp()
{
    maDate = ScGlobal::pLocaleData->getDate( Date( Date::SYSTEM ) );
    maAuthor = SvtUserOptions().GetID();
}

const OutlinerParaObject* ScPostIt::GetOutlinerObject() const
{
    if ( mbShown )
       return mpCaption->GetOutlinerParaObject();
    else
        return maInitData.mxOutlinerObj.get();
}

const EditTextObject* ScPostIt::GetEditTextObject() const
{
    const OutlinerParaObject* pOPO = GetOutlinerObject();
    return pOPO ? &pOPO->GetTextObject() : nullptr;
}

OUString ScPostIt::GetText() const
{
    if (mbShown)
    {
        const EditTextObject* pEditObj = GetEditTextObject();
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
    else
        return maInitData.maSimpleText;
}

bool ScPostIt::HasMultiLineText() const
{
    if (mbShown)
    {
        const EditTextObject* pEditObj = GetEditTextObject();
        return pEditObj->GetParagraphCount() > 1;
    }
    else
        return maInitData.maSimpleText.indexOf( '\n' ) >= 0;
}

void ScPostIt::SetText( const OUString& rText )
{
    if (mbShown)
        mpCaption->SetText( rText );
    else
        maInitData.maSimpleText = rText;
}

SdrCaptionObj* ScPostIt::GetOrCreateCaption( const ScAddress& rPos ) const
{
    if (!mbShown)
        const_cast<ScPostIt*>(this)->ShowCaption( rPos );
    return mpCaption;
}

void ScPostIt::ShowCaption( const ScAddress& rPos, bool bShow )
{
    if (mbShown == bShow)
        return;
    if (bShow)
        ShowCaption(rPos);
    else
        HideCaption();
}

void ScPostIt::ShowCaption( const ScAddress& rPos )
{
    if (mbShown)
        return;

    // Captions are not created in Undo documents and only rarely in Clipboard,
    // but otherwise we need caption or initial data.
    assert(!mrDoc.IsUndo() && !mrDoc.IsClipboard());

    // move the data from the union, since we're about to overwrite
    std::unique_ptr< SfxItemSet > xInitItemSet = std::move(maInitData.mxItemSet);
    std::unique_ptr< OutlinerParaObject > xInitOutlinerObj = std::move(maInitData.mxOutlinerObj);
    OUString aInitSimpleText = std::move(maInitData.maSimpleText);
    Point aInitCaptionOffset = std::move(maInitData.maCaptionOffset);
    Size aInitCaptionSize = std::move(maInitData.maCaptionSize);
    bool aInitDefaultPosSize = maInitData.mbDefaultPosSize;
    maInitData.~ScInitData();

    // ScNoteCaptionCreator c'tor creates the caption and inserts it into the document and maNoteData
    ScNoteCaptionCreator aCreator( mrDoc, rPos, *this );

    // Prevent triple change broadcasts of the same object.
    bool bWasLocked = mpCaption->getSdrModelFromSdrObject().isLocked();
    mpCaption->getSdrModelFromSdrObject().setLock(true);

    // transfer ownership of outliner object to caption, or set simple text
    assert( (xInitOutlinerObj || !aInitSimpleText.isEmpty()) &&
        "ScPostIt::CreateCaptionFromInitData - need either outliner para object or simple text" );
    if (xInitOutlinerObj)
        mpCaption->SetOutlinerParaObject( std::move(xInitOutlinerObj) );
    else
        mpCaption->SetText( aInitSimpleText );

    // copy all items or set default items; reset shadow items
    ScCaptionUtil::SetDefaultItems( *mpCaption, mrDoc, xInitItemSet.get() );

    // set position and size of the caption object
    if( aInitDefaultPosSize )
    {
        // set other items and fit caption size to text
        mpCaption->SetMergedItem( makeSdrTextMinFrameWidthItem( SC_NOTECAPTION_WIDTH ) );
        mpCaption->SetMergedItem( makeSdrTextMaxFrameWidthItem( SC_NOTECAPTION_MAXWIDTH_TEMP ) );
        mpCaption->AdjustTextFrameWidthAndHeight();
        aCreator.AutoPlaceCaption();
    }
    else
    {
        tools::Rectangle aCellRect = ScDrawLayer::GetCellRect( mrDoc, rPos, true );
        bool bNegPage = mrDoc.IsNegativePage( rPos.Tab() );
        long nPosX = bNegPage ? (aCellRect.Left() - aInitCaptionOffset.X()) : (aCellRect.Right() + aInitCaptionOffset.X());
        long nPosY = aCellRect.Top() + aInitCaptionOffset.Y();
        tools::Rectangle aCaptRect( Point( nPosX, nPosY ), aInitCaptionSize );
        mpCaption->SetLogicRect( aCaptRect );
        aCreator.FitCaptionToRect();
    }

    // End prevent triple change broadcasts of the same object.
    mpCaption->getSdrModelFromSdrObject().setLock(bWasLocked);
    mpCaption->BroadcastObjectChange();

    ScCaptionUtil::SetCaptionLayer( *mpCaption, /*bShow*/true );
}

void ScPostIt::UpdateCaptionPos( const ScAddress& rPos )
{
    assert(bShown);
    ScCaptionCreator aCreator( mrDoc, rPos, mpCaption );
    aCreator.UpdateCaptionPos();
}

void ScPostIt::HideCaption()
{
    assert(!mrDoc.IsUndo() && !mrDoc.IsClipboard());
    if (!mbShown)
        return;

    std::unique_ptr< SfxItemSet >       xItemSet;
    std::unique_ptr< OutlinerParaObject >  xOutlinerObj;
    Point        aCaptionOffset;
    Size         aCaptionSize;
    bool         bDefaultPosSize;

    if( OutlinerParaObject* pOPO = mpCaption->GetOutlinerParaObject() )
        xOutlinerObj = std::make_unique<OutlinerParaObject>( *pOPO );
    xItemSet = std::make_unique<SfxItemSet>(mpCaption->GetMergedItemSet());

    ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer();
    assert(pDrawLayer == &mpCaption->getSdrModelFromSdrObject());

    SdrPage* pDrawPage = mpCaption->getSdrPageFromSdrObject();

    pDrawPage->RecalcObjOrdNums();
    // create drawing undo action (before removing the object to have valid draw page in undo action)
    bool bRecording = pDrawLayer->IsRecording();
    if (bRecording)
        pDrawLayer->AddCalcUndo( std::make_unique<SdrUndoDelObj>( *mpCaption ));

    // remove the object from the drawing page
    SdrObject* pObj = pDrawPage->RemoveObject( mpCaption->GetOrdNum() );
    assert(pObj == mpCaption);
    SdrObject::Free( pObj );
    mpCaption = nullptr;

// Noel - TODO, still have to get the rest of this data
    mbShown = false;
    new (&maInitData) ScInitData();
    maInitData.mxItemSet = std::move(xItemSet);
    maInitData.mxOutlinerObj = std::move(xOutlinerObj);
    maInitData.maCaptionOffset = aCaptionOffset;
    maInitData.maCaptionSize = aCaptionSize;
    maInitData.mbDefaultPosSize = bDefaultPosSize;
}

ScNoteDataSaved ScPostIt::GetNoteData() const
{
    if (mbShown)
    {
        std::unique_ptr< SfxItemSet >       xItemSet;
        std::unique_ptr< OutlinerParaObject >  xOutlinerObj;
//        Point        aCaptionOffset;
//        Size         aCaptionSize;
//        bool         bDefaultPosSize;

    // Noel - TODO, still have to get the rest of this data
        if( OutlinerParaObject* pOPO = mpCaption->GetOutlinerParaObject() )
            xOutlinerObj = std::make_unique<OutlinerParaObject>( *pOPO );
        xItemSet = std::make_unique<SfxItemSet>(mpCaption->GetMergedItemSet());

        ScNoteDataSaved aSaved;
        aSaved.mxItemSet = std::move(xItemSet);
        aSaved.mxOutlinerObj = std::move(xOutlinerObj);
        return aSaved;
    }
    else
    {
        ScNoteDataSaved aSaved;
        if (maInitData.mxItemSet)
            aSaved.mxItemSet = std::make_unique<SfxItemSet>(*maInitData.mxItemSet);
        if (maInitData.mxOutlinerObj)
            aSaved.mxOutlinerObj = std::make_unique<OutlinerParaObject>(*maInitData.mxOutlinerObj);
        aSaved.maCaptionOffset = maInitData.maCaptionOffset;
        aSaved.maCaptionSize = maInitData.maCaptionSize;
        aSaved.mbDefaultPosSize = maInitData.mbDefaultPosSize;
        return aSaved;
    }
}

void ScPostIt::Update(const ScNoteDataSaved& rNoteData)
{
    assert(!mbShown);

    if (rNoteData.mxOutlinerObj)
        maInitData.mxOutlinerObj = std::make_unique<OutlinerParaObject>( *rNoteData.mxOutlinerObj );
    else
        maInitData.mxOutlinerObj.reset();
    maInitData.mxItemSet = std::make_unique<SfxItemSet>(*rNoteData.mxItemSet);
// Noel - TODO restore other stuff
}

SdrCaptionObj* ScPostIt::GetShownCaption() const
{
    assert(mbShown);
    return mpCaption;
}

SdrCaptionObj* ScNoteUtil::CreateTempCaption(
        ScDocument& rDoc, const ScAddress& rPos, SdrPage& rDrawPage,
        const OUString& rUserText, const tools::Rectangle& rVisRect, bool bTailFront )
{
    OUStringBuffer aBuffer( rUserText );
    // add plain text of invisible (!) cell note (no formatting etc.)
    SdrCaptionObj* pNoteCaption = nullptr;
    ScPostIt* pNote = rDoc.GetNote( rPos );
    if( pNote && !pNote->IsCaptionShown() )
    {
        if( !aBuffer.isEmpty() )
            aBuffer.append( "\n--------\n" ).append( pNote->GetText() );
        pNoteCaption = pNote->GetOrCreateCaption( rPos );
    }

    // create a caption if any text exists
    if( !pNoteCaption && aBuffer.isEmpty() )
        return nullptr;

    // prepare visible rectangle (add default distance to all borders)
    tools::Rectangle aVisRect(
        rVisRect.Left() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Top() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Right() - SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Bottom() - SC_NOTECAPTION_BORDERDIST_TEMP );

    // create the caption object
    ScCaptionCreator aCreator( rDoc, rPos, bTailFront );

    SdrCaptionObj* pCaption = aCreator.GetCaption();  // just for ease of use

    // insert caption into page (needed to set caption text)
    rDrawPage.InsertObject( pCaption );

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
        ScCaptionUtil::SetDefaultItems( *pCaption, rDoc, nullptr );
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
    ScPostIt* pNote = new ScPostIt( rDoc, pCaption );
    pNote->AutoStamp();

    rDoc.SetNote(rPos, std::unique_ptr<ScPostIt>(pNote));

    // ScNoteCaptionCreator c'tor updates the caption object to be part of a note
    ScNoteCaptionCreator aCreator( rDoc, rPos, pCaption );

    return pNote;
}

ScPostIt* ScNoteUtil::CreateNoteFromObjectData(
        ScDocument& rDoc, const ScAddress& rPos, std::unique_ptr<SfxItemSet> pItemSet,
        OutlinerParaObject* pOutlinerObj, const tools::Rectangle& rCaptionRect,
        bool bShown )
{
    assert( pItemSet && pOutlinerObj && "ScNoteUtil::CreateNoteFromObjectData - item set and outliner object expected" );

    // convert absolute caption position to relative position
    bool bDefaultPosSize = rCaptionRect.IsEmpty();
    Point aCaptionOffset;
    Size aCaptionSize;
    if( !bDefaultPosSize )
    {
        tools::Rectangle aCellRect = ScDrawLayer::GetCellRect( rDoc, rPos, true );
        bool bNegPage = rDoc.IsNegativePage( rPos.Tab() );
        aCaptionOffset.setX( bNegPage ? (aCellRect.Left() - rCaptionRect.Right()) : (rCaptionRect.Left() - aCellRect.Right()) );
        aCaptionOffset.setY( rCaptionRect.Top() - aCellRect.Top() );
        aCaptionSize = rCaptionRect.GetSize();
    }

    /*  Create the note and insert it into the document. */
    ScPostIt* pNote = new ScPostIt( rDoc, std::move(pItemSet), pOutlinerObj,
        aCaptionOffset, aCaptionSize, bDefaultPosSize );
    pNote->AutoStamp();
    rDoc.SetNote(rPos, std::unique_ptr<ScPostIt>(pNote));
    if (bShown)
        pNote->ShowCaption(rPos);

    return pNote;
}

ScPostIt* ScNoteUtil::CreateNoteFromString(
        ScDocument& rDoc, const ScAddress& rPos, const OUString& rNoteText,
        bool bShown )
{
    ScPostIt* pNote = nullptr;
    if( !rNoteText.isEmpty() )
    {
        /*  Create the note and insert it into the document. */
        pNote = new ScPostIt( rDoc, rNoteText );
        pNote->AutoStamp();
        //insert takes ownership
        rDoc.SetNote(rPos, std::unique_ptr<ScPostIt>(pNote));
        if (bShown)
            pNote->ShowCaption(rPos);
    }
    return pNote;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
