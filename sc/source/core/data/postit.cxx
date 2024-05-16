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
#include <svx/svdocapt.hxx>
#include <svx/svdpage.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <osl/diagnose.h>
#include <comphelper/lok.hxx>

#include <svx/sdsxyitm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtmfitm.hxx>
#include <tools/gen.hxx>

#include <document.hxx>
#include <stlpool.hxx>
#include <stylehelper.hxx>
#include <drwlayer.hxx>
#include <userdat.hxx>
#include <editutil.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <utility>
#include <strings.hrc>
#include <officecfg/Office/Calc.hxx>

#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <comphelper/propertyvalue.hxx>

using namespace com::sun::star;

namespace {

const tools::Long SC_NOTECAPTION_WIDTH             =  2900;    /// Default width of note caption textbox.
const tools::Long SC_NOTECAPTION_MAXWIDTH_TEMP     = 12000;    /// Maximum width of temporary note caption textbox.
const tools::Long SC_NOTECAPTION_HEIGHT            =  1800;    /// Default height of note caption textbox.
const tools::Long SC_NOTECAPTION_CELLDIST          =   600;    /// Default distance of note captions to border of anchor cell.
const tools::Long SC_NOTECAPTION_OFFSET_Y          = -1500;    /// Default Y offset of note captions to top border of anchor cell.
const tools::Long SC_NOTECAPTION_OFFSET_X          =  1500;    /// Default X offset of note captions to left border of anchor cell.
const tools::Long SC_NOTECAPTION_BORDERDIST_TEMP   =   100;    /// Distance of temporary note captions to visible sheet area.

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
    /** Sets all hard formatting attributes to the caption object. */
    static void         SetExtraItems( SdrCaptionObj& rCaption, const SfxItemSet& rExtraItemSet );
};

void ScCaptionUtil::SetCaptionLayer( SdrCaptionObj& rCaption, bool bShown )
{
    SdrLayerID nLayer = bShown ? SC_LAYER_INTERN : SC_LAYER_HIDDEN;
    if( nLayer != rCaption.GetLayer() )
        rCaption.SetLayer( nLayer );
}

void ScCaptionUtil::SetBasicCaptionSettings( SdrCaptionObj& rCaption, bool bShown )
{
    rCaption.SetFixedTail();
    rCaption.SetSpecialTextBoxShadow();
    SetCaptionLayer( rCaption, bShown );
}

void ScCaptionUtil::SetCaptionUserData( SdrCaptionObj& rCaption, const ScAddress& rPos )
{
    // pass true to ScDrawLayer::GetObjData() to create the object data entry
    ScDrawObjData* pObjData = ScDrawLayer::GetObjData( &rCaption, true );
    assert(pObjData && "ScCaptionUtil::SetCaptionUserData - missing drawing object user data");
    pObjData->maStart = rPos;
    pObjData->meType = ScDrawObjData::CellNote;
}

void ScCaptionUtil::SetExtraItems( SdrCaptionObj& rCaption, const SfxItemSet& rExtraItemSet )
{
    SfxItemSet aItemSet = rCaption.GetMergedItemSet();

    aItemSet.Put(rExtraItemSet);
    // reset shadow visibility (see also ScNoteUtil::CreateNoteFromCaption)
    aItemSet.ClearItem(SDRATTR_SHADOW);
    // ... but not distance, as that will fallback to wrong values
    // if the comment is shown and then opened in older versions:
    aItemSet.Put( makeSdrShadowXDistItem( 100 ) );
    aItemSet.Put( makeSdrShadowYDistItem( 100 ) );

    rCaption.SetMergedItemSet( aItemSet );
}

/** Helper for creation and manipulation of caption drawing objects independent
    from cell annotations. */
class ScCaptionCreator
{
public:
    /** Create a new caption. The caption will not be inserted into the document. */
    explicit            ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, bool bTailFront );
    /** Manipulate an existing caption. */
    explicit            ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, const rtl::Reference<SdrCaptionObj>& xCaption );

    /** Returns the drawing layer page of the sheet contained in maPos. */
    SdrPage*            GetDrawPage();
    /** Returns the caption drawing object. */
    rtl::Reference<SdrCaptionObj> &      GetCaption() { return mxCaption; }

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
    rtl::Reference<SdrCaptionObj> mxCaption;
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

ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, const rtl::Reference<SdrCaptionObj>& xCaption ) :
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
    aTailPos.setX( ::std::clamp( aTailPos.X(), rVisRect.Left(), rVisRect.Right() ) );
    aTailPos.setY( ::std::clamp( aTailPos.Y(), rVisRect.Top(), rVisRect.Bottom() ) );
    mxCaption->SetTailPos( aTailPos );

    // caption rectangle
    tools::Rectangle aCaptRect = mxCaption->GetLogicRect();
    Point aCaptPos = aCaptRect.TopLeft();
    // move textbox inside right border of visible area
    aCaptPos.setX( ::std::min< tools::Long >( aCaptPos.X(), rVisRect.Right() - aCaptRect.GetWidth() ) );
    // move textbox inside left border of visible area (this may move it outside on right side again)
    aCaptPos.setX( ::std::max< tools::Long >( aCaptPos.X(), rVisRect.Left() ) );
    // move textbox inside bottom border of visible area
    aCaptPos.setY( ::std::min< tools::Long >( aCaptPos.Y(), rVisRect.Bottom() - aCaptRect.GetHeight() ) );
    // move textbox inside top border of visible area (this may move it outside on bottom side again)
    aCaptPos.setY( ::std::max< tools::Long >( aCaptPos.Y(), rVisRect.Top() ) );
    // update caption
    aCaptRect.SetPos( aCaptPos );
    mxCaption->SetLogicRect( aCaptRect );
}

void ScCaptionCreator::AutoPlaceCaption( const tools::Rectangle* pVisRect )
{
    const tools::Rectangle& rVisRect = GetVisRect( pVisRect );

    // caption rectangle
    tools::Rectangle aCaptRect = mxCaption->GetLogicRect();
    tools::Long nWidth = aCaptRect.GetWidth();
    tools::Long nHeight = aCaptRect.GetHeight();

    // n***Space contains available space between border of visible area and cell
    tools::Long nLeftSpace = maCellRect.Left() - rVisRect.Left() + 1;
    tools::Long nRightSpace = rVisRect.Right() - maCellRect.Right() + 1;
    tools::Long nTopSpace = maCellRect.Top() - rVisRect.Top() + 1;
    tools::Long nBottomSpace = rVisRect.Bottom() - maCellRect.Bottom() + 1;

    // nNeeded*** contains textbox dimensions plus needed distances to cell or border of visible area
    tools::Long nNeededSpaceX = nWidth + SC_NOTECAPTION_CELLDIST;
    tools::Long nNeededSpaceY = nHeight + SC_NOTECAPTION_CELLDIST;

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
        tools::Long nDiffX = (rOldTailPos.X() >= 0) ? (aCaptRect.Left() - rOldTailPos.X()) : (rOldTailPos.X() - aCaptRect.Right());
        if( mbNegPage ) nDiffX = -nDiffX - aCaptRect.GetWidth();
        tools::Long nDiffY = aCaptRect.Top() - rOldTailPos.Y();
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
    mxCaption =
        new SdrCaptionObj(
            *mrDoc.GetDrawLayer(), // TTTT should ret a ref?
            aTextRect,
            aTailPos);
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
        maPageRect.Normalize();
    }
}

/** Helper for creation of permanent caption drawing objects for cell notes. */
class ScNoteCaptionCreator : public ScCaptionCreator
{
public:
    /** Create a new caption object and inserts it into the document. */
    explicit            ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, ScNoteData& rNoteData );
    /** Manipulate an existing caption. */
    explicit            ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, rtl::Reference<SdrCaptionObj>& xCaption, bool bShown );
};

ScNoteCaptionCreator::ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, ScNoteData& rNoteData ) :
    ScCaptionCreator( rDoc, rPos )  // use helper c'tor that does not create the caption yet
{
    SdrPage* pDrawPage = GetDrawPage();
    OSL_ENSURE( pDrawPage, "ScNoteCaptionCreator::ScNoteCaptionCreator - no drawing page" );
    if( !pDrawPage )
        return;

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

ScNoteCaptionCreator::ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, rtl::Reference<SdrCaptionObj>& xCaption, bool bShown ) :
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

struct ScCaptionInitData
{
    std::optional< SfxItemSet > moItemSet;  /// Caption object formatting.
    std::optional< OutlinerParaObject > mxOutlinerObj; /// Text object with all text portion formatting.
    std::unique_ptr< GenerateNoteCaption > mxGenerator; /// Operator to generate Caption Object from import data
    OUString            maStyleName;        /// Drawing style associated with the caption object.
    OUString            maSimpleText;       /// Simple text without formatting.
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
    maNoteData.mxCaption.clear();
    CreateCaption( rPos, rNote.maNoteData.mxCaption.get() );
}

ScPostIt::ScPostIt( ScDocument& rDoc, const ScAddress& rPos, ScNoteData aNoteData, bool bAlwaysCreateCaption, sal_uInt32 nPostItId ) :
    mrDoc( rDoc ),
    maNoteData(std::move( aNoteData ))
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
    // tdf#117307: Don't clone comment, if it is in the same position
    if ( (rOwnPos == rDestPos) && !mrDoc.IsClipboard() )
        bCloneCaption = false;
    CreateCaptionFromInitData( rOwnPos );
    sal_uInt32 nPostItId = comphelper::LibreOfficeKit::isActive() ? 0 : mnPostItId;
    return bCloneCaption ? std::make_unique<ScPostIt>( rDestDoc, rDestPos, *this, nPostItId ) : std::make_unique<ScPostIt>( rDestDoc, rDestPos, maNoteData, false, mnPostItId );
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
    maNoteData.maDate = ScGlobal::getLocaleData().getDate( Date( Date::SYSTEM ) ) + " " +
        ScGlobal::getLocaleData().getTime(DateTime(DateTime::SYSTEM), false);
    const OUString aAuthor = SvtUserOptions().GetFullName();
    maNoteData.maAuthor = !aAuthor.isEmpty() ? aAuthor : ScResId(STR_CHG_UNKNOWN_AUTHOR);
}

const OutlinerParaObject* ScPostIt::GetOutlinerObject() const
{
    if( maNoteData.mxCaption )
        return maNoteData.mxCaption->GetOutlinerParaObject();
    if( maNoteData.mxInitData && maNoteData.mxInitData->mxOutlinerObj )
        return &*maNoteData.mxInitData->mxOutlinerObj;
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
        rEngine.SetTextCurrentDefaults(*pEditObj);
        sal_Int32 nParaCount = rEngine.GetParagraphCount();
        for( sal_Int32 nPara = 0; nPara < nParaCount; ++nPara )
        {
            if( nPara > 0 )
                aBuffer.append( '\n' );
            aBuffer.append(rEngine.GetText(nPara));
        }
        return aBuffer.makeStringAndClear();
    }
    if( maNoteData.mxInitData )
        return maNoteData.mxInitData->maSimpleText;
    return OUString();
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
            pInitData->mxOutlinerObj = *pOPO;
        pInitData->maSimpleText = GetText();

        maNoteData.mxInitData.reset(pInitData);
        maNoteData.mxCaption.clear();
    }
    else
    {
        /*  This function is used in undo actions to give up the responsibility for
            the caption object which is handled by separate drawing undo actions. */
        maNoteData.mxCaption.clear();
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
    assert((maNoteData.mxCaption || maNoteData.mxInitData) || mrDoc.IsUndo() || mrDoc.IsClipboard());
    if( !maNoteData.mxInitData )
        return;


    /*  This function is called from ScPostIt::Clone() when copying cells
        to the clipboard/undo document, and when copying cells from the
        clipboard/undo document. The former should always be called first,
        so if called in a clipboard/undo document, the caption should have
        been created already. However, for clipboard in case the
        originating document was destructed a new caption has to be
        created. */
    OSL_ENSURE( !mrDoc.IsUndo() && (!mrDoc.IsClipboard() || !maNoteData.mxCaption),
            "ScPostIt::CreateCaptionFromInitData - note caption should not be created in undo/clip documents" );

    // going to forget the initial caption data struct when this method returns
    auto xInitData = std::move(maNoteData.mxInitData);

    /*  #i104915# Never try to create notes in Undo document, leads to
        crash due to missing document members (e.g. row height array). */
    if( maNoteData.mxCaption || mrDoc.IsUndo() )
        return;

    if (mrDoc.IsClipboard())
        mrDoc.InitDrawLayer();  // ensure there is a drawing layer

    // ScNoteCaptionCreator c'tor creates the caption and inserts it into the document and maNoteData
    ScNoteCaptionCreator aCreator( mrDoc, rPos, maNoteData );
    if( !maNoteData.mxCaption )
        return;

    // Prevent triple change broadcasts of the same object.
    bool bWasLocked = maNoteData.mxCaption->getSdrModelFromSdrObject().isLocked();
    maNoteData.mxCaption->getSdrModelFromSdrObject().setLock(true);

    if (xInitData->mxGenerator)
        xInitData->mxGenerator->Generate(*maNoteData.mxCaption);
    else
    {
        // transfer ownership of outliner object to caption, or set simple text
        OSL_ENSURE( xInitData->mxOutlinerObj || !xInitData->maSimpleText.isEmpty(),
            "ScPostIt::CreateCaptionFromInitData - need either outliner para object or simple text" );
        if (xInitData->mxOutlinerObj)
            maNoteData.mxCaption->SetOutlinerParaObject( std::move(xInitData->mxOutlinerObj) );
        else
            maNoteData.mxCaption->SetText( xInitData->maSimpleText );
    }

    if (!xInitData->maStyleName.isEmpty())
    {
        if (auto pStyleSheet = mrDoc.GetStyleSheetPool()->Find(xInitData->maStyleName, SfxStyleFamily::Frame))
            maNoteData.mxCaption->SetStyleSheet(static_cast<SfxStyleSheet*>(pStyleSheet), true);

        if (xInitData->moItemSet)
            maNoteData.mxCaption->SetMergedItemSet(*xInitData->moItemSet);
    }
    else
    {
        if (auto pStyleSheet = mrDoc.GetStyleSheetPool()->Find(ScResId(STR_STYLENAME_NOTE), SfxStyleFamily::Frame))
            maNoteData.mxCaption->SetStyleSheet(static_cast<SfxStyleSheet*>(pStyleSheet), true);

        // copy all items and reset shadow items
        if (xInitData->moItemSet)
            ScCaptionUtil::SetExtraItems(*maNoteData.mxCaption, *xInitData->moItemSet);
    }

    // set position and size of the caption object
    if( xInitData->mbDefaultPosSize )
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
        tools::Long nPosX = bNegPage ? (aCellRect.Left() - xInitData->maCaptionOffset.X()) : (aCellRect.Right() + xInitData->maCaptionOffset.X());
        tools::Long nPosY = aCellRect.Top() + xInitData->maCaptionOffset.Y();
        tools::Rectangle aCaptRect( Point( nPosX, nPosY ), xInitData->maCaptionSize );
        maNoteData.mxCaption->SetLogicRect( aCaptRect );
        aCreator.FitCaptionToRect();
    }

    // End prevent triple change broadcasts of the same object.
    maNoteData.mxCaption->getSdrModelFromSdrObject().setLock(bWasLocked);
    maNoteData.mxCaption->BroadcastObjectChange();
}

void ScPostIt::CreateCaption( const ScAddress& rPos, const SdrCaptionObj* pCaption )
{
    OSL_ENSURE( !maNoteData.mxCaption, "ScPostIt::CreateCaption - unexpected caption object found" );
    maNoteData.mxCaption.clear();

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
    if( !maNoteData.mxCaption )
        return;

    // clone settings of passed caption
    if( pCaption )
    {
        // copy edit text object (object must be inserted into page already)
        if( OutlinerParaObject* pOPO = pCaption->GetOutlinerParaObject() )
            maNoteData.mxCaption->SetOutlinerParaObject( *pOPO );
        // copy formatting items (after text has been copied to apply font formatting)
        if (auto pStyleSheet = pCaption->GetStyleSheet())
        {
            auto pPool = mrDoc.GetStyleSheetPool();
            pPool->CopyStyleFrom(pStyleSheet->GetPool(), pStyleSheet->GetName(), pStyleSheet->GetFamily(), true);

            if (auto pDestStyleSheet = pPool->Find(pStyleSheet->GetName(), pStyleSheet->GetFamily()))
                maNoteData.mxCaption->SetStyleSheet(static_cast<SfxStyleSheet*>(pDestStyleSheet), true);
        }
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
        if (auto pStyleSheet = mrDoc.GetStyleSheetPool()->Find(ScResId(STR_STYLENAME_NOTE), SfxStyleFamily::Frame))
            maNoteData.mxCaption->SetStyleSheet(static_cast<SfxStyleSheet*>(pStyleSheet), true);
        // set default size, undoing sdr::TextProperties::SetStyleSheet's
        // adjustment that use a wrong min height.
        tools::Rectangle aCaptRect = maNoteData.mxCaption->GetLogicRect();
        aCaptRect.SetSize({ SC_NOTECAPTION_WIDTH, SC_NOTECAPTION_HEIGHT });
        maNoteData.mxCaption->SetLogicRect(aCaptRect);
        // set default position
        aCreator.AutoPlaceCaption();
    }

    // create undo action
    if( ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer() )
        if( pDrawLayer->IsRecording() )
            pDrawLayer->AddCalcUndo( std::make_unique<SdrUndoNewObj>( *maNoteData.mxCaption ) );
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
    {
        SdrPage* pDrawPage(maNoteData.mxCaption->getSdrPageFromSdrObject());
        SAL_WARN_IF( !pDrawPage, "sc.core", "ScCaptionPtr::removeFromDrawPageAndFree - object without drawing page");
        if (pDrawPage)
        {
            pDrawPage->RecalcObjOrdNums();
            // create drawing undo action (before removing the object to have valid draw page in undo action)
            bool bRecording = (pDrawLayer && pDrawLayer->IsRecording());
            if (bRecording)
                pDrawLayer->AddCalcUndo( std::make_unique<SdrUndoDelObj>( *maNoteData.mxCaption ));
            // remove the object from the drawing page
            rtl::Reference<SdrObject> pRemovedObj = pDrawPage->RemoveObject( maNoteData.mxCaption->GetOrdNum() );
            assert(pRemovedObj.get() == maNoteData.mxCaption.get()); (void)pRemovedObj;
        }
    }

    SAL_INFO("sc.core","ScPostIt::RemoveCaption -"
            "  IsUndo: " << mrDoc.IsUndo() << "  IsClip: " << mrDoc.IsClipboard() <<
            "  Dtor: " << mrDoc.IsInDtorClear());

    // Forget the caption object if removeFromDrawPageAndFree() did not free it.
    if (maNoteData.mxCaption)
    {
        SAL_INFO("sc.core","ScPostIt::RemoveCaption - forgetting one ref");
        maNoteData.mxCaption.clear();
    }
}

static void lcl_FormatAndInsertAuthorAndDatepara(SdrCaptionObj* pCaption, OUStringBuffer& aUserData, bool bUserWithTrackText)
{
    uno::Reference<drawing::XShape> xShape = pCaption->getUnoShape();
    uno::Reference<text::XText> xText(xShape, uno::UNO_QUERY);
    uno::Reference<text::XTextAppend> xBodyTextAppend(xText, uno::UNO_QUERY);

    if (xBodyTextAppend.is())
    {
        uno::Sequence< beans::PropertyValue > aArgs;
        if (bUserWithTrackText)
        {
            xBodyTextAppend->insertTextPortion(aUserData.makeStringAndClear(), aArgs, xText->getStart());
        }
        else
        {
            xBodyTextAppend->insertTextPortion(u"\n--------\n"_ustr, aArgs, xText->getStart());
            aArgs = {
                comphelper::makePropertyValue(u"CharWeight"_ustr, uno::Any(awt::FontWeight::BOLD)),
            };
            xBodyTextAppend->insertTextPortion(aUserData.makeStringAndClear(), aArgs, xText->getStart());
        }
    }
}

rtl::Reference<SdrCaptionObj> ScNoteUtil::CreateTempCaption(
        ScDocument& rDoc, const ScAddress& rPos, SdrPage& rDrawPage,
        std::u16string_view rUserText, const tools::Rectangle& rVisRect, bool bTailFront )
{
    bool bUserWithTrackText = false;
    OUStringBuffer aBuffer( rUserText );
    // add plain text of invisible (!) cell note (no formatting etc.)
    SdrCaptionObj* pNoteCaption = nullptr;
    const ScPostIt* pNote = rDoc.GetNote( rPos );
    if( pNote && !pNote->IsCaptionShown() )
    {
        if (!aBuffer.isEmpty())
        {
            bUserWithTrackText = true;
            aBuffer.append("\n--------\n");
        }
        else
        {
            aBuffer.append(pNote->GetAuthor()
                + ", "
                + pNote->GetDate());
        }
        pNoteCaption = pNote->GetOrCreateCaption( rPos );
    }

    // prepare visible rectangle (add default distance to all borders)
    tools::Rectangle aVisRect(
        rVisRect.Left() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Top() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Right() - SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Bottom() - SC_NOTECAPTION_BORDERDIST_TEMP );

    // create the caption object
    ScCaptionCreator aCreator( rDoc, rPos, bTailFront );

    // insert caption into page (needed to set caption text)
    rtl::Reference<SdrCaptionObj> pCaption = aCreator.GetCaption();  // just for ease of use
    rDrawPage.InsertObject( pCaption.get() );

    // clone the edit text object, then seta and format the Author and date text
    if (pNoteCaption)
    {
        if( OutlinerParaObject* pOPO = pNoteCaption->GetOutlinerParaObject() )
            pCaption->SetOutlinerParaObject( *pOPO );
        // Setting and formatting rUserText: Author name and date time
        if (officecfg::Office::Calc::Content::Display::NoteAuthor::get())
           lcl_FormatAndInsertAuthorAndDatepara(pCaption.get(), aBuffer, bUserWithTrackText);
        // set formatting (must be done after setting text) and resize the box to fit the text
        if (auto pStyleSheet = pNoteCaption->GetStyleSheet())
            pCaption->SetStyleSheet(pStyleSheet, true);
        pCaption->SetMergedItemSetAndBroadcast(pNoteCaption->GetMergedItemSet());
    }
    else
    {
        pCaption->SetText(aBuffer.makeStringAndClear());
        if (auto pStyleSheet = rDoc.GetStyleSheetPool()->Find(ScResId(STR_STYLENAME_NOTE), SfxStyleFamily::Frame))
            pCaption->SetStyleSheet(static_cast<SfxStyleSheet*>(pStyleSheet), true);
    }

    // adjust caption size to text size
    tools::Long nMaxWidth = ::std::min< tools::Long >( aVisRect.GetWidth() * 2 / 3, SC_NOTECAPTION_MAXWIDTH_TEMP );
    pCaption->SetMergedItem( makeSdrTextAutoGrowWidthItem( true ) );
    pCaption->SetMergedItem( makeSdrTextMinFrameWidthItem( SC_NOTECAPTION_WIDTH ) );
    pCaption->SetMergedItem( makeSdrTextMaxFrameWidthItem( nMaxWidth ) );
    pCaption->SetMergedItem( makeSdrTextAutoGrowHeightItem( true ) );
    pCaption->AdjustTextFrameWidthAndHeight();

    // move caption into visible area
    aCreator.AutoPlaceCaption( &aVisRect );

    // XXX Note it is already inserted to the draw page.
    return aCreator.GetCaption();
}

ScPostIt* ScNoteUtil::CreateNoteFromCaption(
        ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj* pCaption, bool bHasStyle )
{
    ScNoteData aNoteData( true/*bShown*/ );
    aNoteData.mxCaption = pCaption;
    ScPostIt* pNote = new ScPostIt( rDoc, rPos, aNoteData, false );
    pNote->AutoStamp();

    rDoc.SetNote(rPos, std::unique_ptr<ScPostIt>(pNote));

    // ScNoteCaptionCreator c'tor updates the caption object to be part of a note
    ScNoteCaptionCreator aCreator( rDoc, rPos, aNoteData.mxCaption, true/*bShown*/ );

    if (!bHasStyle)
    {
        if (auto pStyleSheet = rDoc.GetStyleSheetPool()->Find(ScResId(STR_STYLENAME_NOTE), SfxStyleFamily::Frame))
            aNoteData.mxCaption->SetStyleSheet(static_cast<SfxStyleSheet*>(pStyleSheet), true);

        /* We used to show a shadow despite of the shadow item being set to false.
           Clear the existing item, so it inherits the true setting from the style.
           Setting explicitly to true would corrupt the shadow when opened in older versions. */
        aNoteData.mxCaption->ClearMergedItem(SDRATTR_SHADOW);
    }

    return pNote;
}

ScNoteData ScNoteUtil::CreateNoteData(ScDocument& rDoc, const ScAddress& rPos,
                                      const tools::Rectangle& rCaptionRect, bool bShown)
{
    ScNoteData aNoteData( bShown );
    aNoteData.mxInitData = std::make_shared<ScCaptionInitData>();
    ScCaptionInitData& rInitData = *aNoteData.mxInitData;

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

    return aNoteData;
}

ScPostIt* ScNoteUtil::CreateNoteFromObjectData(
        ScDocument& rDoc, const ScAddress& rPos, const SfxItemSet& rItemSet, const OUString& rStyleName,
        const OutlinerParaObject& rOutlinerObj, const tools::Rectangle& rCaptionRect,
        bool bShown )
{
    ScNoteData aNoteData(CreateNoteData(rDoc, rPos, rCaptionRect, bShown));
    ScCaptionInitData& rInitData = *aNoteData.mxInitData;
    rInitData.mxOutlinerObj = rOutlinerObj;
    rInitData.moItemSet.emplace(rItemSet);
    rInitData.maStyleName = ScStyleNameConversion::ProgrammaticToDisplayName(rStyleName, SfxStyleFamily::Frame);

    return InsertNote(rDoc, rPos, std::move(aNoteData), /*bAlwaysCreateCaption*/false, 0/*nPostItId*/);
}

ScPostIt* ScNoteUtil::CreateNoteFromGenerator(
        ScDocument& rDoc, const ScAddress& rPos,
        std::unique_ptr<GenerateNoteCaption> xGenerator,
        const tools::Rectangle& rCaptionRect,
        bool bShown )
{
    ScNoteData aNoteData(CreateNoteData(rDoc, rPos, rCaptionRect, bShown));
    ScCaptionInitData& rInitData = *aNoteData.mxInitData;
    rInitData.mxGenerator = std::move(xGenerator);
    // because the Caption is generated on demand, we will need to create the
    // simple text now to supply any queries for that which don't require
    // creation of a full Caption
    rInitData.maSimpleText = rInitData.mxGenerator->GetSimpleText();

    return InsertNote(rDoc, rPos, std::move(aNoteData), /*bAlwaysCreateCaption*/false, 0/*nPostItId*/);
}

ScPostIt* ScNoteUtil::InsertNote(ScDocument& rDoc, const ScAddress& rPos, ScNoteData&& rNoteData,
                                 bool bAlwaysCreateCaption, sal_uInt32 nPostItId)
{
    /*  Create the note and insert it into the document. If the note is
        visible, the caption object will be created automatically. */
    ScPostIt* pNote = new ScPostIt( rDoc, rPos, std::move(rNoteData), bAlwaysCreateCaption, nPostItId );
    pNote->AutoStamp();
    //insert takes ownership
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
        aNoteData.mxInitData = std::make_shared<ScCaptionInitData>();
        ScCaptionInitData& rInitData = *aNoteData.mxInitData;
        rInitData.maSimpleText = rNoteText;
        rInitData.maStyleName = ScResId(STR_STYLENAME_NOTE);
        rInitData.mbDefaultPosSize = true;

        pNote = InsertNote(rDoc, rPos, std::move(aNoteData), bAlwaysCreateCaption, nPostItId);
    }
    return pNote;
}

namespace sc {

NoteEntry::NoteEntry( const ScAddress& rPos, const ScPostIt* pNote ) :
    maPos(rPos), mpNote(pNote) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
