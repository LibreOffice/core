/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "postit.hxx"

#include <rtl/ustrbuf.hxx>
#include <unotools/useroptions.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdocapt.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include "scitems.hxx"
#include <svx/xlnstit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/xflclit.hxx>
#include <svx/sdshitm.hxx>
#include <svx/sdsxyitm.hxx>
#include <tools/gen.hxx>

#include "table.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "formulacell.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "detfunc.hxx"

#include <utility>




namespace {

const long SC_NOTECAPTION_WIDTH             =  2900;    
const long SC_NOTECAPTION_MAXWIDTH_TEMP     = 12000;    
const long SC_NOTECAPTION_HEIGHT            =  1800;    
const long SC_NOTECAPTION_CELLDIST          =   600;    
const long SC_NOTECAPTION_OFFSET_Y          = -1500;    
const long SC_NOTECAPTION_OFFSET_X          =  1500;    
const long SC_NOTECAPTION_BORDERDIST_TEMP   =   100;    



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
    
    ScDrawObjData* pObjData = ScDrawLayer::GetObjData( &rCaption, true );
    OSL_ENSURE( pObjData, "ScCaptionUtil::SetCaptionUserData - missing drawing object user data" );
    pObjData->maStart = rPos;
    pObjData->meType = ScDrawObjData::CellNote;
}

void ScCaptionUtil::SetDefaultItems( SdrCaptionObj& rCaption, ScDocument& rDoc )
{
    SfxItemSet aItemSet = rCaption.GetMergedItemSet();

    
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
    aItemSet.Put( XFillStyleItem( XFILL_SOLID ) );
    aItemSet.Put( XFillColorItem( OUString(), ScDetectiveFunc::GetCommentColor() ) );
    aItemSet.Put( SdrCaptionEscDirItem( SDRCAPT_ESCBESTFIT ) );

    
    /*  SdrShadowItem has sal_False, instead the shadow is set for the
        rectangle only with SetSpecialTextBoxShadow() when the object is
        created (item must be set to adjust objects from older files). */
    aItemSet.Put( SdrShadowItem( false ) );
    aItemSet.Put( SdrShadowXDistItem( 100 ) );
    aItemSet.Put( SdrShadowYDistItem( 100 ) );

    
    aItemSet.Put( SdrTextLeftDistItem( 100 ) );
    aItemSet.Put( SdrTextRightDistItem( 100 ) );
    aItemSet.Put( SdrTextUpperDistItem( 100 ) );
    aItemSet.Put( SdrTextLowerDistItem( 100 ) );
    aItemSet.Put( SdrTextAutoGrowWidthItem( false ) );
    aItemSet.Put( SdrTextAutoGrowHeightItem( true ) );
    
    const ScPatternAttr& rDefPattern = static_cast< const ScPatternAttr& >( rDoc.GetPool()->GetDefaultItem( ATTR_PATTERN ) );
    rDefPattern.FillEditItemSet( &aItemSet );

    rCaption.SetMergedItemSet( aItemSet );
}

void ScCaptionUtil::SetCaptionItems( SdrCaptionObj& rCaption, const SfxItemSet& rItemSet )
{
    
    rCaption.SetMergedItemSet( rItemSet );
    
    rCaption.SetMergedItem( SdrShadowItem( false ) );
    rCaption.SetMergedItem( SdrShadowXDistItem( 100 ) );
    rCaption.SetMergedItem( SdrShadowYDistItem( 100 ) );
    rCaption.SetSpecialTextBoxShadow();
}



/** Helper for creation and manipulation of caption drawing objects independent
    from cell annotations. */
class ScCaptionCreator
{
public:
    /** Create a new caption. The caption will not be inserted into the document. */
    explicit            ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, bool bShown, bool bTailFront );
    /** Manipulate an existing caption. */
    explicit            ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj& rCaption );

    /** Returns the drawing layer page of the sheet contained in maPos. */
    SdrPage*            GetDrawPage();
    /** Returns the caption drawing object. */
    inline SdrCaptionObj* GetCaption() { return mpCaption; }

    /** Moves the caption inside the passed rectangle. Uses page area if 0 is passed. */
    void                FitCaptionToRect( const Rectangle* pVisRect = 0 );
    /** Places the caption inside the passed rectangle, tries to keep the cell rectangle uncovered. Uses page area if 0 is passed. */
    void                AutoPlaceCaption( const Rectangle* pVisRect = 0 );
    /** Updates caption tail and textbox according to current cell position. Uses page area if 0 is passed. */
    void                UpdateCaptionPos( const Rectangle* pVisRect = 0 );

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
    inline const Rectangle& GetVisRect( const Rectangle* pVisRect ) const { return pVisRect ? *pVisRect : maPageRect; }

private:
    ScDocument&         mrDoc;
    ScAddress           maPos;
    SdrCaptionObj*      mpCaption;
    Rectangle           maPageRect;
    Rectangle           maCellRect;
    bool                mbNegPage;
};



ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, bool bShown, bool bTailFront ) :
    mrDoc( rDoc ),
    maPos( rPos ),
    mpCaption( 0 )
{
    Initialize();
    CreateCaption( bShown, bTailFront );
}

ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj& rCaption ) :
    mrDoc( rDoc ),
    maPos( rPos ),
    mpCaption( &rCaption )
{
    Initialize();
}

ScCaptionCreator::ScCaptionCreator( ScDocument& rDoc, const ScAddress& rPos ) :
    mrDoc( rDoc ),
    maPos( rPos ),
    mpCaption( 0 )
{
    Initialize();
}

SdrPage* ScCaptionCreator::GetDrawPage()
{
    ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer();
    return pDrawLayer ? pDrawLayer->GetPage( static_cast< sal_uInt16 >( maPos.Tab() ) ) : 0;
}

void ScCaptionCreator::FitCaptionToRect( const Rectangle* pVisRect )
{
    const Rectangle& rVisRect = GetVisRect( pVisRect );

    
    Point aTailPos = mpCaption->GetTailPos();
    aTailPos.X() = ::std::max( ::std::min( aTailPos.X(), rVisRect.Right() ), rVisRect.Left() );
    aTailPos.Y() = ::std::max( ::std::min( aTailPos.Y(), rVisRect.Bottom() ), rVisRect.Top() );
    mpCaption->SetTailPos( aTailPos );

    
    Rectangle aCaptRect = mpCaption->GetLogicRect();
    Point aCaptPos = aCaptRect.TopLeft();
    
    aCaptPos.X() = ::std::min< long >( aCaptPos.X(), rVisRect.Right() - aCaptRect.GetWidth() );
    
    aCaptPos.X() = ::std::max< long >( aCaptPos.X(), rVisRect.Left() );
    
    aCaptPos.Y() = ::std::min< long >( aCaptPos.Y(), rVisRect.Bottom() - aCaptRect.GetHeight() );
    
    aCaptPos.Y() = ::std::max< long >( aCaptPos.Y(), rVisRect.Top() );
    
    aCaptRect.SetPos( aCaptPos );
    mpCaption->SetLogicRect( aCaptRect );
}

void ScCaptionCreator::AutoPlaceCaption( const Rectangle* pVisRect )
{
    const Rectangle& rVisRect = GetVisRect( pVisRect );

    
    Rectangle aCaptRect = mpCaption->GetLogicRect();
    long nWidth = aCaptRect.GetWidth();
    long nHeight = aCaptRect.GetHeight();

    
    long nLeftSpace = maCellRect.Left() - rVisRect.Left() + 1;
    long nRightSpace = rVisRect.Right() - maCellRect.Right() + 1;
    long nTopSpace = maCellRect.Top() - rVisRect.Top() + 1;
    long nBottomSpace = rVisRect.Bottom() - maCellRect.Bottom() + 1;

    
    long nNeededSpaceX = nWidth + SC_NOTECAPTION_CELLDIST;
    long nNeededSpaceY = nHeight + SC_NOTECAPTION_CELLDIST;

    
    bool bFitsWidthLeft = nNeededSpaceX <= nLeftSpace;      
    bool bFitsWidthRight = nNeededSpaceX <= nRightSpace;    
    bool bFitsWidth = nWidth <= rVisRect.GetWidth();        

    
    bool bFitsHeightTop = nNeededSpaceY <= nTopSpace;       
    bool bFitsHeightBottom = nNeededSpaceY <= nBottomSpace; 
    bool bFitsHeight = nHeight <= rVisRect.GetHeight();     

    
    bool bFitsLeft = bFitsWidthLeft && bFitsHeight;
    bool bFitsRight = bFitsWidthRight && bFitsHeight;
    bool bFitsTop = bFitsWidth && bFitsHeightTop;
    bool bFitsBottom = bFitsWidth && bFitsHeightBottom;

    Point aCaptPos;
    
    if( bFitsLeft || bFitsRight || (!bFitsTop && !bFitsBottom) )
    {
        
        bool bPreferLeft = bFitsLeft && (mbNegPage || !bFitsRight);
        bool bPreferRight = bFitsRight && (!mbNegPage || !bFitsLeft);
        
        if( bPreferLeft || (!bPreferRight && (nLeftSpace > nRightSpace)) )
            aCaptPos.X() = maCellRect.Left() - SC_NOTECAPTION_CELLDIST - nWidth;
        else 
            aCaptPos.X() = maCellRect.Right() + SC_NOTECAPTION_CELLDIST;
        
        aCaptPos.Y() = maCellRect.Top() + SC_NOTECAPTION_OFFSET_Y;
    }
    else    
    {
        
        aCaptPos.X() = maCellRect.Left() + SC_NOTECAPTION_OFFSET_X;
        
        if( bFitsTop )
            aCaptPos.Y() = maCellRect.Top() - SC_NOTECAPTION_CELLDIST - nHeight;
        else    
            aCaptPos.Y() = maCellRect.Bottom() + SC_NOTECAPTION_CELLDIST;
    }

    
    aCaptRect.SetPos( aCaptPos );
    mpCaption->SetLogicRect( aCaptRect );
    FitCaptionToRect( pVisRect );
}

void ScCaptionCreator::UpdateCaptionPos( const Rectangle* pVisRect )
{
    ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer();

    
    const Point& rOldTailPos = mpCaption->GetTailPos();
    Point aTailPos = CalcTailPos( false );
    if( rOldTailPos != aTailPos )
    {
        
        if( pDrawLayer && pDrawLayer->IsRecording() )
            pDrawLayer->AddCalcUndo( pDrawLayer->GetSdrUndoFactory().CreateUndoGeoObject( *mpCaption ) );
        
        Rectangle aCaptRect = mpCaption->GetLogicRect();
        long nDiffX = (rOldTailPos.X() >= 0) ? (aCaptRect.Left() - rOldTailPos.X()) : (rOldTailPos.X() - aCaptRect.Right());
        if( mbNegPage ) nDiffX = -nDiffX - aCaptRect.GetWidth();
        long nDiffY = aCaptRect.Top() - rOldTailPos.Y();
        aCaptRect.SetPos( aTailPos + Point( nDiffX, nDiffY ) );
        
        mpCaption->SetTailPos( aTailPos );
        mpCaption->SetLogicRect( aCaptRect );
        
        FitCaptionToRect( pVisRect );
    }

    
    ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( mpCaption, maPos.Tab() );
    if( pCaptData && (maPos != pCaptData->maStart) )
    {
        
        if( pDrawLayer && pDrawLayer->IsRecording() )
            pDrawLayer->AddCalcUndo( new ScUndoObjData( mpCaption, pCaptData->maStart, pCaptData->maEnd, maPos, pCaptData->maEnd ) );
        
        pCaptData->maStart = maPos;
    }
}

Point ScCaptionCreator::CalcTailPos( bool bTailFront )
{
    
    bool bTailLeft = bTailFront != mbNegPage;
    Point aTailPos = bTailLeft ? maCellRect.TopLeft() : maCellRect.TopRight();
    
    if( bTailLeft ) aTailPos.X() += 10; else aTailPos.X() -= 10;
    aTailPos.Y() += 10;
    return aTailPos;
}

void ScCaptionCreator::CreateCaption( bool bShown, bool bTailFront )
{
    
    Rectangle aTextRect( Point( 0 , 0 ), Size( SC_NOTECAPTION_WIDTH, SC_NOTECAPTION_HEIGHT ) );
    Point aTailPos = CalcTailPos( bTailFront );
    mpCaption = new SdrCaptionObj( aTextRect, aTailPos );
    
    ScCaptionUtil::SetBasicCaptionSettings( *mpCaption, bShown );
}

void ScCaptionCreator::Initialize()
{
    maCellRect = ScDrawLayer::GetCellRect( mrDoc, maPos, true );
    mbNegPage = mrDoc.IsNegativePage( maPos.Tab() );
    if( SdrPage* pDrawPage = GetDrawPage() )
    {
        maPageRect = Rectangle( Point( 0, 0 ), pDrawPage->GetSize() );
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
    explicit            ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj& rCaption, bool bShown );
};



ScNoteCaptionCreator::ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, ScNoteData& rNoteData ) :
    ScCaptionCreator( rDoc, rPos )  
{
    SdrPage* pDrawPage = GetDrawPage();
    OSL_ENSURE( pDrawPage, "ScNoteCaptionCreator::ScNoteCaptionCreator - no drawing page" );
    if( pDrawPage )
    {
        
        CreateCaption( rNoteData.mbShown, false );
        rNoteData.mpCaption = GetCaption();
        OSL_ENSURE( rNoteData.mpCaption, "ScNoteCaptionCreator::ScNoteCaptionCreator - missing caption object" );
        if( rNoteData.mpCaption )
        {
            
            ScCaptionUtil::SetCaptionUserData( *rNoteData.mpCaption, rPos );
            
            pDrawPage->InsertObject( rNoteData.mpCaption );
        }
    }
}

ScNoteCaptionCreator::ScNoteCaptionCreator( ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj& rCaption, bool bShown ) :
    ScCaptionCreator( rDoc, rPos, rCaption )
{
    SdrPage* pDrawPage = GetDrawPage();
    OSL_ENSURE( pDrawPage, "ScNoteCaptionCreator::ScNoteCaptionCreator - no drawing page" );
    OSL_ENSURE( rCaption.GetPage() == pDrawPage, "ScNoteCaptionCreator::ScNoteCaptionCreator - wrong drawing page in caption" );
    if( pDrawPage && (rCaption.GetPage() == pDrawPage) )
    {
        
        ScCaptionUtil::SetCaptionUserData( rCaption, rPos );
        
        ScCaptionUtil::SetBasicCaptionSettings( rCaption, bShown );
        
        rCaption.SetTailPos( CalcTailPos( false ) );
    }
}

} 



struct ScCaptionInitData
{
    typedef ::std::auto_ptr< SfxItemSet >           SfxItemSetPtr;
    typedef ::std::auto_ptr< OutlinerParaObject >   OutlinerParaObjPtr;

    SfxItemSetPtr       mxItemSet;          
    OutlinerParaObjPtr  mxOutlinerObj;      
    OUString     maSimpleText;       
    Point               maCaptionOffset;    
    Size                maCaptionSize;      
    bool                mbDefaultPosSize;   

    explicit            ScCaptionInitData();
};



ScCaptionInitData::ScCaptionInitData() :
    mbDefaultPosSize( true )
{
}



ScNoteData::ScNoteData( bool bShown ) :
    mpCaption( 0 ),
    mbShown( bShown )
{
}

ScNoteData::~ScNoteData()
{
}



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

ScPostIt::ScPostIt( ScDocument& rDoc, const ScAddress& rPos, const ScNoteData& rNoteData, bool bAlwaysCreateCaption ) :
    mrDoc( rDoc ),
    maNoteData( rNoteData )
{
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
    return bCloneCaption ? new ScPostIt( rDestDoc, rDestPos, *this ) : new ScPostIt( rDestDoc, rDestPos, maNoteData, false );
}

const ScNoteData& ScPostIt::GetNoteData() const
{
    return maNoteData;
}

const OUString& ScPostIt::GetDate() const
{
    return maNoteData.maDate;
}

void ScPostIt::SetDate( const OUString& rDate )
{
    maNoteData.maDate = rDate;
}

const OUString& ScPostIt::GetAuthor() const
{
    return maNoteData.maAuthor;
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
    if( maNoteData.mpCaption )
        return maNoteData.mpCaption->GetOutlinerParaObject();
    if( maNoteData.mxInitData.get() )
        return maNoteData.mxInitData->mxOutlinerObj.get();
    return 0;
}

const EditTextObject* ScPostIt::GetEditTextObject() const
{
    const OutlinerParaObject* pOPO = GetOutlinerObject();
    return pOPO ? &pOPO->GetTextObject() : 0;
}

OUString ScPostIt::GetText() const
{
    if( const EditTextObject* pEditObj = GetEditTextObject() )
    {
        OUStringBuffer aBuffer;
        for( sal_Int32 nPara = 0, nParaCount = pEditObj->GetParagraphCount(); nPara < nParaCount; ++nPara )
        {
            if( nPara > 0 )
                aBuffer.append( '\n' );
            aBuffer.append( pEditObj->GetText( nPara ) );
        }
        return aBuffer.makeStringAndClear();
    }
    if( maNoteData.mxInitData.get() )
        return maNoteData.mxInitData->maSimpleText;
    return OUString();
}

void ScPostIt::SetText( const ScAddress& rPos, const OUString& rText )
{
    CreateCaptionFromInitData( rPos );
    if( maNoteData.mpCaption )
        maNoteData.mpCaption->SetText( rText );
}

SdrCaptionObj* ScPostIt::GetCaption() const
{
    return maNoteData.mpCaption;
}

SdrCaptionObj* ScPostIt::GetOrCreateCaption( const ScAddress& rPos ) const
{
    CreateCaptionFromInitData( rPos );
    return maNoteData.mpCaption;
}

void ScPostIt::ForgetCaption()
{
    /*  This function is used in undo actions to give up the responsibility for
        the caption object which is handled by separate drawing undo actions. */
    maNoteData.mpCaption = 0;
    maNoteData.mxInitData.reset();
}

void ScPostIt::ShowCaption( const ScAddress& rPos, bool bShow )
{
    CreateCaptionFromInitData( rPos );
    
    maNoteData.mbShown = bShow;
    if( maNoteData.mpCaption )
        ScCaptionUtil::SetCaptionLayer( *maNoteData.mpCaption, bShow );
}

bool ScPostIt::IsCaptionShown() const
{
    return maNoteData.mbShown;
}

void ScPostIt::ShowCaptionTemp( const ScAddress& rPos, bool bShow )
{
    CreateCaptionFromInitData( rPos );
    if( maNoteData.mpCaption )
        ScCaptionUtil::SetCaptionLayer( *maNoteData.mpCaption, maNoteData.mbShown || bShow );
}

void ScPostIt::UpdateCaptionPos( const ScAddress& rPos )
{
    CreateCaptionFromInitData( rPos );
    if( maNoteData.mpCaption )
    {
        ScCaptionCreator aCreator( mrDoc, rPos, *maNoteData.mpCaption );
        aCreator.UpdateCaptionPos();
    }
}



void ScPostIt::CreateCaptionFromInitData( const ScAddress& rPos ) const
{
    OSL_ENSURE( maNoteData.mpCaption || maNoteData.mxInitData.get(), "ScPostIt::CreateCaptionFromInitData - need caption object or initial caption data" );
    if( maNoteData.mxInitData.get() )
    {
        /*  This function is called from ScPostIt::Clone() when copying cells
            to the clipboard/undo document, and when copying cells from the
            clipboard/undo document. The former should always be called first,
            so if called in an clipboard/undo document, the caption should have
            been created already. */
        OSL_ENSURE( !mrDoc.IsUndo() && !mrDoc.IsClipboard(), "ScPostIt::CreateCaptionFromInitData - note caption should not be created in undo/clip documents" );

        /*  #i104915# Never try to create notes in Undo document, leads to
            crash due to missing document members (e.g. row height array). */
        if( !maNoteData.mpCaption && !mrDoc.IsUndo() )
        {
            
            ScNoteCaptionCreator aCreator( mrDoc, rPos, maNoteData );
            if( maNoteData.mpCaption )
            {
                ScCaptionInitData& rInitData = *maNoteData.mxInitData;

                
                OSL_ENSURE( rInitData.mxOutlinerObj.get() || !rInitData.maSimpleText.isEmpty(),
                    "ScPostIt::CreateCaptionFromInitData - need either outliner para object or simple text" );
                if( rInitData.mxOutlinerObj.get() )
                    maNoteData.mpCaption->SetOutlinerParaObject( rInitData.mxOutlinerObj.release() );
                else
                    maNoteData.mpCaption->SetText( rInitData.maSimpleText );

                
                ScCaptionUtil::SetDefaultItems( *maNoteData.mpCaption, mrDoc );
                if( rInitData.mxItemSet.get() )
                    ScCaptionUtil::SetCaptionItems( *maNoteData.mpCaption, *rInitData.mxItemSet );

                
                if( rInitData.mbDefaultPosSize )
                {
                    
                    maNoteData.mpCaption->SetMergedItem( SdrTextMinFrameWidthItem( SC_NOTECAPTION_WIDTH ) );
                    maNoteData.mpCaption->SetMergedItem( SdrTextMaxFrameWidthItem( SC_NOTECAPTION_MAXWIDTH_TEMP ) );
                    maNoteData.mpCaption->AdjustTextFrameWidthAndHeight();
                    aCreator.AutoPlaceCaption();
                }
                else
                {
                    Rectangle aCellRect = ScDrawLayer::GetCellRect( mrDoc, rPos, true );
                    bool bNegPage = mrDoc.IsNegativePage( rPos.Tab() );
                    long nPosX = bNegPage ? (aCellRect.Left() - rInitData.maCaptionOffset.X()) : (aCellRect.Right() + rInitData.maCaptionOffset.X());
                    long nPosY = aCellRect.Top() + rInitData.maCaptionOffset.Y();
                    Rectangle aCaptRect( Point( nPosX, nPosY ), rInitData.maCaptionSize );
                    maNoteData.mpCaption->SetLogicRect( aCaptRect );
                    aCreator.FitCaptionToRect();
                }
            }
        }
        
        maNoteData.mxInitData.reset();
    }
}

void ScPostIt::CreateCaption( const ScAddress& rPos, const SdrCaptionObj* pCaption )
{
    OSL_ENSURE( !maNoteData.mpCaption, "ScPostIt::CreateCaption - unexpected caption object found" );
    maNoteData.mpCaption = 0;

    /*  #i104915# Never try to create notes in Undo document, leads to
        crash due to missing document members (e.g. row height array). */
    OSL_ENSURE( !mrDoc.IsUndo(), "ScPostIt::CreateCaption - note caption should not be created in undo documents" );
    if( mrDoc.IsUndo() )
        return;

    
    if( mrDoc.IsClipboard() )
        mrDoc.InitDrawLayer();

    
    ScNoteCaptionCreator aCreator( mrDoc, rPos, maNoteData );
    if( maNoteData.mpCaption )
    {
        
        if( pCaption )
        {
            
            if( OutlinerParaObject* pOPO = pCaption->GetOutlinerParaObject() )
                maNoteData.mpCaption->SetOutlinerParaObject( new OutlinerParaObject( *pOPO ) );
            
            maNoteData.mpCaption->SetMergedItemSetAndBroadcast( pCaption->GetMergedItemSet() );
            
            Rectangle aCaptRect = pCaption->GetLogicRect();
            Point aDist = maNoteData.mpCaption->GetTailPos() - pCaption->GetTailPos();
            aCaptRect.Move( aDist.X(), aDist.Y() );
            maNoteData.mpCaption->SetLogicRect( aCaptRect );
            aCreator.FitCaptionToRect();
        }
        else
        {
            
            ScCaptionUtil::SetDefaultItems( *maNoteData.mpCaption, mrDoc );
            aCreator.AutoPlaceCaption();
        }

        
        if( ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer() )
            if( pDrawLayer->IsRecording() )
                pDrawLayer->AddCalcUndo( pDrawLayer->GetSdrUndoFactory().CreateUndoNewObject( *maNoteData.mpCaption ) );
    }
}

void ScPostIt::RemoveCaption()
{

    /*  Remove caption object only, if this note is its owner (e.g. notes in
        undo documents refer to captions in original document, do not remove
        them from drawing layer here). */
    ScDrawLayer* pDrawLayer = mrDoc.GetDrawLayer();
    if( maNoteData.mpCaption && (pDrawLayer == maNoteData.mpCaption->GetModel()) )
    {
        OSL_ENSURE( pDrawLayer, "ScPostIt::RemoveCaption - object without drawing layer" );
        SdrPage* pDrawPage = maNoteData.mpCaption->GetPage();
        OSL_ENSURE( pDrawPage, "ScPostIt::RemoveCaption - object without drawing page" );
        if( pDrawPage )
        {
            pDrawPage->RecalcObjOrdNums();
            
            bool bRecording = ( pDrawLayer && pDrawLayer->IsRecording() );
            if( bRecording )
                pDrawLayer->AddCalcUndo( pDrawLayer->GetSdrUndoFactory().CreateUndoDeleteObject( *maNoteData.mpCaption ) );
            
            SdrObject* pObj = pDrawPage->RemoveObject( maNoteData.mpCaption->GetOrdNum() );
            if( !bRecording )
                SdrObject::Free( pObj );
        }
    }
    maNoteData.mpCaption = 0;
}



SdrCaptionObj* ScNoteUtil::CreateTempCaption(
        ScDocument& rDoc, const ScAddress& rPos, SdrPage& rDrawPage,
        const OUString& rUserText, const Rectangle& rVisRect, bool bTailFront )
{
    OUStringBuffer aBuffer( rUserText );
    
    SdrCaptionObj* pNoteCaption = 0;
    const ScPostIt* pNote = rDoc.GetNote( rPos );
    if( pNote && !pNote->IsCaptionShown() )
    {
        if( !aBuffer.isEmpty() )
            aBuffer.append( "\n--------\n" ).append( pNote->GetText() );
        pNoteCaption = pNote->GetOrCreateCaption( rPos );
    }

    
    if( !pNoteCaption && aBuffer.isEmpty() )
        return 0;

    
    Rectangle aVisRect(
        rVisRect.Left() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Top() + SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Right() - SC_NOTECAPTION_BORDERDIST_TEMP,
        rVisRect.Bottom() - SC_NOTECAPTION_BORDERDIST_TEMP );

    
    ScCaptionCreator aCreator( rDoc, rPos, true, bTailFront );
    SdrCaptionObj* pCaption = aCreator.GetCaption();

    
    rDrawPage.InsertObject( pCaption );

    
    if( pNoteCaption && rUserText.isEmpty() )
    {
        if( OutlinerParaObject* pOPO = pNoteCaption->GetOutlinerParaObject() )
            pCaption->SetOutlinerParaObject( new OutlinerParaObject( *pOPO ) );
        
        pCaption->SetMergedItemSetAndBroadcast( pNoteCaption->GetMergedItemSet() );
        Rectangle aCaptRect( pCaption->GetLogicRect().TopLeft(), pNoteCaption->GetLogicRect().GetSize() );
        pCaption->SetLogicRect( aCaptRect );
    }
    else
    {
        
        pCaption->SetText( aBuffer.makeStringAndClear() );
        ScCaptionUtil::SetDefaultItems( *pCaption, rDoc );
        
        long nMaxWidth = ::std::min< long >( aVisRect.GetWidth() * 2 / 3, SC_NOTECAPTION_MAXWIDTH_TEMP );
        pCaption->SetMergedItem( SdrTextAutoGrowWidthItem( sal_True ) );
        pCaption->SetMergedItem( SdrTextMinFrameWidthItem( SC_NOTECAPTION_WIDTH ) );
        pCaption->SetMergedItem( SdrTextMaxFrameWidthItem( nMaxWidth ) );
        pCaption->SetMergedItem( SdrTextAutoGrowHeightItem( sal_True ) );
        pCaption->AdjustTextFrameWidthAndHeight();
    }

    
    aCreator.AutoPlaceCaption( &aVisRect );
    return pCaption;
}

ScPostIt* ScNoteUtil::CreateNoteFromCaption(
        ScDocument& rDoc, const ScAddress& rPos, SdrCaptionObj& rCaption, bool bShown )
{
    ScNoteData aNoteData( bShown );
    aNoteData.mpCaption = &rCaption;
    ScPostIt* pNote = new ScPostIt( rDoc, rPos, aNoteData, false );
    pNote->AutoStamp();

    rDoc.SetNote(rPos, pNote);

    
    ScNoteCaptionCreator aCreator( rDoc, rPos, rCaption, bShown );

    return pNote;
}

ScPostIt* ScNoteUtil::CreateNoteFromObjectData(
        ScDocument& rDoc, const ScAddress& rPos, SfxItemSet* pItemSet,
        OutlinerParaObject* pOutlinerObj, const Rectangle& rCaptionRect,
        bool bShown, bool bAlwaysCreateCaption )
{
    OSL_ENSURE( pItemSet && pOutlinerObj, "ScNoteUtil::CreateNoteFromObjectData - item set and outliner object expected" );
    ScNoteData aNoteData( bShown );
    aNoteData.mxInitData.reset( new ScCaptionInitData );
    ScCaptionInitData& rInitData = *aNoteData.mxInitData;
    rInitData.mxItemSet.reset( pItemSet );
    rInitData.mxOutlinerObj.reset( pOutlinerObj );

    
    rInitData.mbDefaultPosSize = rCaptionRect.IsEmpty();
    if( !rInitData.mbDefaultPosSize )
    {
        Rectangle aCellRect = ScDrawLayer::GetCellRect( rDoc, rPos, true );
        bool bNegPage = rDoc.IsNegativePage( rPos.Tab() );
        rInitData.maCaptionOffset.X() = bNegPage ? (aCellRect.Left() - rCaptionRect.Right()) : (rCaptionRect.Left() - aCellRect.Right());
        rInitData.maCaptionOffset.Y() = rCaptionRect.Top() - aCellRect.Top();
        rInitData.maCaptionSize = rCaptionRect.GetSize();
    }

    /*  Create the note and insert it into the document. If the note is
        visible, the caption object will be created automatically. */
    ScPostIt* pNote = new ScPostIt( rDoc, rPos, aNoteData, bAlwaysCreateCaption );
    pNote->AutoStamp();

    rDoc.SetNote(rPos, pNote);

    return pNote;
}

ScPostIt* ScNoteUtil::CreateNoteFromString(
        ScDocument& rDoc, const ScAddress& rPos, const OUString& rNoteText,
        bool bShown, bool bAlwaysCreateCaption )
{
    ScPostIt* pNote = 0;
    if( !rNoteText.isEmpty() )
    {
        ScNoteData aNoteData( bShown );
        aNoteData.mxInitData.reset( new ScCaptionInitData );
        ScCaptionInitData& rInitData = *aNoteData.mxInitData;
        rInitData.maSimpleText = rNoteText;
        rInitData.mbDefaultPosSize = true;

        /*  Create the note and insert it into the document. If the note is
            visible, the caption object will be created automatically. */
        pNote = new ScPostIt( rDoc, rPos, aNoteData, bAlwaysCreateCaption );
        pNote->AutoStamp();
        
        rDoc.SetNote(rPos, pNote);
    }
    return pNote;
}

namespace sc {

NoteEntry::NoteEntry( const ScAddress& rPos, const ScPostIt* pNote ) :
    maPos(rPos), mpNote(pNote) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
