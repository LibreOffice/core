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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <sot/exchange.hxx>
#include <svx/objfac3d.hxx>
#include <svx/xtable.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svditer.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdundo.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <editeng/unolingu.hxx>
#include <svx/drawitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <svx/shapepropertynotifier.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/docfile.hxx>
#include <sot/storage.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/itempool.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include "drwlayer.hxx"
#include "drawpage.hxx"
#include "global.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include "userdat.hxx"
#include "markdata.hxx"
#include "globstr.hrc"
#include "scmod.hxx"
#include "chartarr.hxx"
#include "postit.hxx"
#include "attrib.hxx"
#include "charthelper.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"

#include <vcl/field.hxx>

#define DET_ARROW_OFFSET    1000

using namespace ::com::sun::star;



TYPEINIT1(ScTabDeletedHint, SfxHint);
TYPEINIT1(ScTabSizeChangedHint, SfxHint);

static ScDrawObjFactory* pFac = NULL;
static E3dObjFactory* pF3d = NULL;
static sal_uInt16 nInst = 0;

SfxObjectShell* ScDrawLayer::pGlobalDrawPersist = NULL;

sal_Bool bDrawIsInUndo = false;         



ScUndoObjData::ScUndoObjData( SdrObject* pObjP, const ScAddress& rOS, const ScAddress& rOE,
                                               const ScAddress& rNS, const ScAddress& rNE ) :
    SdrUndoObj( *pObjP ),
    aOldStt( rOS ),
    aOldEnd( rOE ),
    aNewStt( rNS ),
    aNewEnd( rNE )
{
}

ScUndoObjData::~ScUndoObjData()
{
}

void ScUndoObjData::Undo()
{
    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
    OSL_ENSURE(pData,"ScUndoObjData: Data missing");
    if (pData)
    {
        pData->maStart = aOldStt;
        pData->maEnd = aOldEnd;
    }
}

void ScUndoObjData::Redo()
{
    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
    OSL_ENSURE(pData,"ScUndoObjData: Data missing");
    if (pData)
    {
        pData->maStart = aNewStt;
        pData->maEnd = aNewEnd;
    }
}



ScTabDeletedHint::ScTabDeletedHint( SCTAB nTabNo ) :
    nTab( nTabNo )
{
}

ScTabDeletedHint::~ScTabDeletedHint()
{
}

ScTabSizeChangedHint::ScTabSizeChangedHint( SCTAB nTabNo ) :
    nTab( nTabNo )
{
}

ScTabSizeChangedHint::~ScTabSizeChangedHint()
{
}



#define MAXMM   10000000

inline long TwipsToHmm (long nVal)
{
    return static_cast< long >( MetricField::ConvertDoubleValue (static_cast<sal_Int64>(nVal), 0, 0,
            FUNIT_TWIP, FUNIT_100TH_MM) );
}

inline long HmmToTwips (long nVal)
{
    return static_cast< long > ( MetricField::ConvertDoubleValue (static_cast<sal_Int64>(nVal), 0, 0,
            FUNIT_100TH_MM, FUNIT_TWIP) );
}

inline void TwipsToMM( long& nVal )
{
    nVal = TwipsToHmm (nVal);
}

inline void ReverseTwipsToMM( long& nVal )
{
    nVal = HmmToTwips (nVal);
}

static void lcl_ReverseTwipsToMM( Rectangle& rRect )
{
    ReverseTwipsToMM( rRect.Left() );
    ReverseTwipsToMM( rRect.Right() );
    ReverseTwipsToMM( rRect.Top() );
    ReverseTwipsToMM( rRect.Bottom() );
}




ScDrawLayer::ScDrawLayer( ScDocument* pDocument, const OUString& rName ) :
    FmFormModel( SvtPathOptions().GetPalettePath(),
                 NULL,                          
                 pGlobalDrawPersist ?
                     pGlobalDrawPersist :
                     ( pDocument ? pDocument->GetDocumentShell() : NULL ),
                 true ),        
    aName( rName ),
    pDoc( pDocument ),
    pUndoGroup( NULL ),
    bRecording( false ),
    bAdjustEnabled( true ),
    bHyphenatorSet( false )
{
    pGlobalDrawPersist = NULL;          

    SfxObjectShell* pObjSh = pDocument ? pDocument->GetDocumentShell() : NULL;
    XColorListRef pXCol = XColorList::GetStdColorList();
    if ( pObjSh )
    {
        SetObjectShell( pObjSh );

        
        SvxColorListItem* pColItem = (SvxColorListItem*) pObjSh->GetItem( SID_COLOR_TABLE );
        if ( pColItem )
            pXCol = pColItem->GetColorList();
    }
    SetPropertyList( static_cast<XPropertyList *> (pXCol.get()) );

    SetSwapGraphics(true);

    SetScaleUnit(MAP_100TH_MM);
    SfxItemPool& rPool = GetItemPool();
    rPool.SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    SvxFrameDirectionItem aModeItem( FRMDIR_ENVIRONMENT, EE_PARA_WRITINGDIR );
    rPool.SetPoolDefaultItem( aModeItem );

    
    
    rPool.SetPoolDefaultItem(SdrShadowXDistItem(300));
    rPool.SetPoolDefaultItem(SdrShadowYDistItem(300));

    
    LanguageType eOfficeLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
    if (MsLangId::isKorean(eOfficeLanguage) || eOfficeLanguage == LANGUAGE_JAPANESE)
    {
        
        rPool.GetSecondaryPool()->SetPoolDefaultItem( SvxScriptSpaceItem( false, EE_PARA_ASIANCJKSPACING ) );
    }

    rPool.FreezeIdRanges();                         

    SdrLayerAdmin& rAdmin = GetLayerAdmin();
    rAdmin.NewLayer(OUString("vorne"),    SC_LAYER_FRONT);
    rAdmin.NewLayer(OUString("hinten"),   SC_LAYER_BACK);
    rAdmin.NewLayer(OUString("intern"),   SC_LAYER_INTERN);
    rAdmin.NewLayer(OUString("Controls"), SC_LAYER_CONTROLS);
    rAdmin.NewLayer(OUString("hidden"),   SC_LAYER_HIDDEN);
    

    
    ScModule* pScMod = SC_MOD();
    Outliner& rOutliner = GetDrawOutliner();
    rOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );

    Outliner& rHitOutliner = GetHitTestOutliner();
    rHitOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );

    
    SfxItemPool* pOutlinerPool = rOutliner.GetEditTextObjectPool();
    if ( pOutlinerPool )
    {
         pItemPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT ));           
         pItemPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT_CJK ));           
         pItemPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT_CTL ));           
    }
    SfxItemPool* pHitOutlinerPool = rHitOutliner.GetEditTextObjectPool();
    if ( pHitOutlinerPool )
    {
         pHitOutlinerPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT ));    
         pHitOutlinerPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT_CJK ));    
         pHitOutlinerPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT_CTL ));    
    }

    
    if( pDoc )
        EnableUndo( pDoc->IsUndoEnabled() );

    

    if( !nInst++ )
    {
        pFac = new ScDrawObjFactory;
        pF3d = new E3dObjFactory;
    }
}

ScDrawLayer::~ScDrawLayer()
{
    Broadcast(SdrHint(HINT_MODELCLEARED));

    ClearModel(sal_True);

    delete pUndoGroup;
    if( !--nInst )
    {
        delete pFac, pFac = NULL;
        delete pF3d, pF3d = NULL;
    }
}

void ScDrawLayer::UseHyphenator()
{
    if (!bHyphenatorSet)
    {
        com::sun::star::uno::Reference< com::sun::star::linguistic2::XHyphenator >
                                    xHyphenator = LinguMgr::GetHyphenator();

        GetDrawOutliner().SetHyphenator( xHyphenator );
        GetHitTestOutliner().SetHyphenator( xHyphenator );

        bHyphenatorSet = true;
    }
}

SdrPage* ScDrawLayer::AllocPage(bool bMasterPage)
{
    return new ScDrawPage(*this, bMasterPage);
}

bool ScDrawLayer::HasObjects() const
{
    bool bFound = false;

    sal_uInt16 nCount = GetPageCount();
    for (sal_uInt16 i=0; i<nCount && !bFound; i++)
        if (GetPage(i)->GetObjCount())
            bFound = true;

    return bFound;
}

SdrModel* ScDrawLayer::AllocModel() const
{
    
    

    return new ScDrawLayer( NULL, aName );
}

bool ScDrawLayer::ScAddPage( SCTAB nTab )
{
    if (bDrawIsInUndo)
        return false;   

    ScDrawPage* pPage = (ScDrawPage*)AllocPage( false );
    InsertPage(pPage, static_cast<sal_uInt16>(nTab));
    if (bRecording)
        AddCalcUndo(new SdrUndoNewPage(*pPage));

    ResetTab(nTab, pDoc->GetTableCount()-1);
    return true;        
}

void ScDrawLayer::ScRemovePage( SCTAB nTab )
{
    if (bDrawIsInUndo)
        return;

    Broadcast( ScTabDeletedHint( nTab ) );
    if (bRecording)
    {
        SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
        AddCalcUndo(new SdrUndoDelPage(*pPage));        
        RemovePage( static_cast<sal_uInt16>(nTab) );                            
    }
    else
        DeletePage( static_cast<sal_uInt16>(nTab) );                            

    ResetTab(nTab, pDoc->GetTableCount()-1);
}

void ScDrawLayer::ScRenamePage( SCTAB nTab, const OUString& rNewName )
{
    ScDrawPage* pPage = (ScDrawPage*) GetPage(static_cast<sal_uInt16>(nTab));
    if (pPage)
        pPage->SetName(rNewName);
}

void ScDrawLayer::ScMovePage( sal_uInt16 nOldPos, sal_uInt16 nNewPos )
{
    MovePage( nOldPos, nNewPos );
    sal_uInt16 nMinPos = std::min(nOldPos, nNewPos);
    ResetTab(nMinPos, pDoc->GetTableCount()-1);
}

void ScDrawLayer::ScCopyPage( sal_uInt16 nOldPos, sal_uInt16 nNewPos)
{
    if (bDrawIsInUndo)
        return;

    SdrPage* pOldPage = GetPage(nOldPos);
    SdrPage* pNewPage = GetPage(nNewPos);

    

    if (pOldPage && pNewPage)
    {
        SCTAB nOldTab = static_cast<SCTAB>(nOldPos);
        SCTAB nNewTab = static_cast<SCTAB>(nNewPos);

        SdrObjListIter aIter( *pOldPage, IM_FLAT );
        SdrObject* pOldObject = aIter.Next();
        while (pOldObject)
        {
            ScDrawObjData* pOldData = GetObjData(pOldObject);
            if (pOldData)
            {
                pOldData->maStart.SetTab(nOldTab);
                pOldData->maEnd.SetTab(nOldTab);
            }
            SdrObject* pNewObject = pOldObject->Clone();
            pNewObject->SetModel(this);
            pNewObject->SetPage(pNewPage);

            pNewObject->NbcMove(Size(0,0));
            pNewPage->InsertObject( pNewObject );
            ScDrawObjData* pNewData = GetObjData(pNewObject);
            if (pNewData)
            {
                pNewData->maStart.SetTab(nNewTab);
                pNewData->maEnd.SetTab(nNewTab);
            }

            if (bRecording)
                AddCalcUndo( new SdrUndoInsertObj( *pNewObject ) );

            pOldObject = aIter.Next();
        }
    }

    ResetTab(static_cast<SCTAB>(nNewPos), pDoc->GetTableCount()-1);
}

void ScDrawLayer::ResetTab( SCTAB nStart, SCTAB nEnd )
{
    SCTAB nPageSize = static_cast<SCTAB>(GetPageCount());
    if (nPageSize < 0)
        
        return;

    if (nEnd >= nPageSize)
        
        nEnd = nPageSize - 1;

    for (SCTAB i = nStart; i <= nEnd; ++i)
    {
        SdrPage* pPage = GetPage(static_cast<sal_uInt16>(i));
        if (!pPage)
            continue;

        SdrObjListIter aIter(*pPage, IM_FLAT);
        for (SdrObject* pObj = aIter.Next(); pObj; pObj = aIter.Next())
        {
            ScDrawObjData* pData = GetObjData(pObj);
            if (!pData)
                continue;

            pData->maStart.SetTab(i);
            pData->maEnd.SetTab(i);
        }
    }
}

inline sal_Bool IsInBlock( const ScAddress& rPos, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2 )
{
    return rPos.Col() >= nCol1 && rPos.Col() <= nCol2 &&
           rPos.Row() >= nRow1 && rPos.Row() <= nRow2;
}

void ScDrawLayer::MoveCells( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                                SCsCOL nDx,SCsROW nDy, bool bUpdateNoteCaptionPos )
{
    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page not found");
    if (!pPage)
        return;

    sal_Bool bNegativePage = pDoc && pDoc->IsNegativePage( nTab );

    sal_uLong nCount = pPage->GetObjCount();
    for ( sal_uLong i = 0; i < nCount; i++ )
    {
        SdrObject* pObj = pPage->GetObj( i );
        ScDrawObjData* pData = GetObjDataTab( pObj, nTab );
        if( pData )
        {
            const ScAddress aOldStt = pData->maStart;
            const ScAddress aOldEnd = pData->maEnd;
            sal_Bool bChange = false;
            if ( aOldStt.IsValid() && IsInBlock( aOldStt, nCol1,nRow1, nCol2,nRow2 ) )
            {
                pData->maStart.IncCol( nDx );
                pData->maStart.IncRow( nDy );
                bChange = sal_True;
            }
            if ( aOldEnd.IsValid() && IsInBlock( aOldEnd, nCol1,nRow1, nCol2,nRow2 ) )
            {
                pData->maEnd.IncCol( nDx );
                pData->maEnd.IncRow( nDy );
                bChange = sal_True;
            }
            if (bChange)
            {
                if ( pObj->ISA( SdrRectObj ) && pData->maStart.IsValid() && pData->maEnd.IsValid() )
                    pData->maStart.PutInOrder( pData->maEnd );
                AddCalcUndo( new ScUndoObjData( pObj, aOldStt, aOldEnd, pData->maStart, pData->maEnd ) );
                RecalcPos( pObj, *pData, bNegativePage, bUpdateNoteCaptionPos );
            }
        }
    }
}

void ScDrawLayer::SetPageSize( sal_uInt16 nPageNo, const Size& rSize, bool bUpdateNoteCaptionPos )
{
    SdrPage* pPage = GetPage(nPageNo);
    if (pPage)
    {
        if ( rSize != pPage->GetSize() )
        {
            pPage->SetSize( rSize );
            Broadcast( ScTabSizeChangedHint( static_cast<SCTAB>(nPageNo) ) );   
        }

        
        
        

        sal_Bool bNegativePage = pDoc && pDoc->IsNegativePage( static_cast<SCTAB>(nPageNo) );

        sal_uLong nCount = pPage->GetObjCount();
        for ( sal_uLong i = 0; i < nCount; i++ )
        {
            SdrObject* pObj = pPage->GetObj( i );
            ScDrawObjData* pData = GetObjDataTab( pObj, static_cast<SCTAB>(nPageNo) );
            if( pData )
                RecalcPos( pObj, *pData, bNegativePage, bUpdateNoteCaptionPos );
        }
    }
}

namespace
{
    
    Rectangle lcl_makeSafeRectangle(const Rectangle &rNew)
    {
        Rectangle aRect = rNew;
        if (aRect.Bottom() == aRect.Top())
            aRect.Bottom() = aRect.Top()+1;
        if (aRect.Right() == aRect.Left())
            aRect.Right() = aRect.Left()+1;
        return aRect;
    }

    Point lcl_calcAvailableDiff(ScDocument &rDoc, SCCOL nCol, SCROW nRow, SCTAB nTab, const Point &aWantedDiff)
    {
        Point aAvailableDiff(aWantedDiff);
        long nHeight = static_cast<long>(rDoc.GetRowHeight( nRow, nTab ) * HMM_PER_TWIPS);
        long nWidth  = static_cast<long>(rDoc.GetColWidth(  nCol, nTab ) * HMM_PER_TWIPS);
        if (aAvailableDiff.Y() > nHeight)
            aAvailableDiff.Y() = nHeight;
        if (aAvailableDiff.X() > nWidth)
            aAvailableDiff.X() = nWidth;
        return aAvailableDiff;
    }

    Rectangle lcl_UpdateCalcPoly(basegfx::B2DPolygon &rCalcPoly, int nWhichPoint, const Point &rPos)
    {
        rCalcPoly.setB2DPoint(nWhichPoint, basegfx::B2DPoint(rPos.X(), rPos.Y()));
        basegfx::B2DRange aRange(basegfx::tools::getRange(rCalcPoly));
        return Rectangle(static_cast<long>(aRange.getMinX()), static_cast<long>(aRange.getMinY()),
            static_cast<long>(aRange.getMaxX()), static_cast<long>(aRange.getMaxY()));
    }
}
void ScDrawLayer::ResizeLastRectFromAnchor( SdrObject* pObj, ScDrawObjData& rData, bool bUseLogicRect, bool bNegativePage, bool bCanResize, bool bHiddenAsZero )
{
    rData.maLastRect = ( bUseLogicRect ? pObj->GetLogicRect() : pObj->GetSnapRect() );
    SCCOL nCol1 = rData.maStart.Col();
    SCROW nRow1 = rData.maStart.Row();
    SCTAB nTab1 = rData.maStart.Tab();
    SCCOL nCol2 = rData.maEnd.Col();
    SCROW nRow2 = rData.maEnd.Row();
    SCTAB nTab2 = rData.maEnd.Tab();
    Point aPos( pDoc->GetColOffset( nCol1, nTab1, bHiddenAsZero ), pDoc->GetRowOffset( nRow1, nTab1, bHiddenAsZero ) );
    TwipsToMM( aPos.X() );
    TwipsToMM( aPos.Y() );
    aPos += lcl_calcAvailableDiff(*pDoc, nCol1, nRow1, nTab1, rData.maStartOffset);

    if( bCanResize )
    {
        Point aEnd( pDoc->GetColOffset( nCol2, nTab2, bHiddenAsZero ), pDoc->GetRowOffset( nRow2, nTab2, bHiddenAsZero ) );
        TwipsToMM( aEnd.X() );
        TwipsToMM( aEnd.Y() );
        aEnd += lcl_calcAvailableDiff(*pDoc, nCol2, nRow2, nTab2, rData.maEndOffset);

        Rectangle aNew = Rectangle( aPos, aEnd );
        if ( bNegativePage )
            MirrorRectRTL( aNew );

        rData.maLastRect = lcl_makeSafeRectangle(aNew);
    }
    else
    {
        if ( bNegativePage )
            aPos.X() = -aPos.X() - rData.maLastRect.GetWidth();
        
        rData.maLastRect.SetPos( aPos );
    }
}

void ScDrawLayer::RecalcPos( SdrObject* pObj, ScDrawObjData& rData, bool bNegativePage, bool bUpdateNoteCaptionPos )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::RecalcPos - missing document" );
    if( !pDoc )
        return;

    if (rData.meType == ScDrawObjData::CellNote)
    {
        OSL_ENSURE( rData.maStart.IsValid(), "ScDrawLayer::RecalcPos - invalid position for cell note" );
        /*  #i109372# On insert/remove rows/columns/cells: Updating the caption
            position must not be done, if the cell containing the note has not
            been moved yet in the document. The calling code now passes an
            additional boolean stating if the cells are already moved. */
        if( bUpdateNoteCaptionPos )
            /*  When inside an undo action, there may be pending note captions
                where cell note is already deleted (thus document cannot find
                the note object anymore). The caption will be deleted later
                with drawing undo. */
            if( ScPostIt* pNote = pDoc->GetNote( rData.maStart ) )
                pNote->UpdateCaptionPos( rData.maStart );
        return;
    }

    bool bValid1 = rData.maStart.IsValid();
    SCCOL nCol1 = rData.maStart.Col();
    SCROW nRow1 = rData.maStart.Row();
    SCTAB nTab1 = rData.maStart.Tab();
    bool bValid2 = rData.maEnd.IsValid();
    SCCOL nCol2 = rData.maEnd.Col();
    SCROW nRow2 = rData.maEnd.Row();
    SCTAB nTab2 = rData.maEnd.Tab();

    if (rData.meType == ScDrawObjData::ValidationCircle)
    {
        
        rData.maLastRect = pObj->GetLogicRect();

        Point aPos( pDoc->GetColOffset( nCol1, nTab1 ), pDoc->GetRowOffset( nRow1, nTab1 ) );
        TwipsToMM( aPos.X() );
        TwipsToMM( aPos.Y() );

        

        Size aSize( (long)( TwipsToHmm( pDoc->GetColWidth( nCol1, nTab1) ) ),
                    (long)( TwipsToHmm( pDoc->GetRowHeight( nRow1, nTab1) ) ) );
        Rectangle aRect( aPos, aSize );
        aRect.Left()    -= 250;
        aRect.Right()   += 250;
        aRect.Top()     -= 70;
        aRect.Bottom()  += 70;
        if ( bNegativePage )
            MirrorRectRTL( aRect );

        if ( pObj->GetLogicRect() != aRect )
        {
            if (bRecording)
                AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
            rData.maLastRect = lcl_makeSafeRectangle(aRect);
            pObj->SetLogicRect(rData.maLastRect);
        }
    }
    else if (rData.meType == ScDrawObjData::DetectiveArrow)
    {
        rData.maLastRect = pObj->GetLogicRect();
        basegfx::B2DPolygon aCalcPoly;
        Point aOrigStartPos(pObj->GetPoint(0));
        Point aOrigEndPos(pObj->GetPoint(1));
        aCalcPoly.append(basegfx::B2DPoint(aOrigStartPos.X(), aOrigStartPos.Y()));
        aCalcPoly.append(basegfx::B2DPoint(aOrigEndPos.X(), aOrigEndPos.Y()));
        

        SCCOL nLastCol;
        SCROW nLastRow;
        if( bValid1 )
        {
            Point aPos( pDoc->GetColOffset( nCol1, nTab1 ), pDoc->GetRowOffset( nRow1, nTab1 ) );
            if (!pDoc->ColHidden(nCol1, nTab1, NULL, &nLastCol))
                aPos.X() += pDoc->GetColWidth( nCol1, nTab1 ) / 4;
            if (!pDoc->RowHidden(nRow1, nTab1, NULL, &nLastRow))
                aPos.Y() += pDoc->GetRowHeight( nRow1, nTab1 ) / 2;
            TwipsToMM( aPos.X() );
            TwipsToMM( aPos.Y() );
            Point aStartPos = aPos;
            if ( bNegativePage )
                aStartPos.X() = -aStartPos.X();     
            if ( pObj->GetPoint( 0 ) != aStartPos )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );

                rData.maLastRect = lcl_UpdateCalcPoly(aCalcPoly, 0, aStartPos);
                pObj->SetPoint( aStartPos, 0 );
            }

            if( !bValid2 )
            {
                Point aEndPos( aPos.X() + DET_ARROW_OFFSET, aPos.Y() - DET_ARROW_OFFSET );
                if (aEndPos.Y() < 0)
                    aEndPos.Y() += (2 * DET_ARROW_OFFSET);
                if ( bNegativePage )
                    aEndPos.X() = -aEndPos.X();
                if ( pObj->GetPoint( 1 ) != aEndPos )
                {
                    if (bRecording)
                        AddCalcUndo( new SdrUndoGeoObj( *pObj ) );

                    rData.maLastRect = lcl_UpdateCalcPoly(aCalcPoly, 1, aEndPos);
                    pObj->SetPoint( aEndPos, 1 );
                }
            }
        }
        if( bValid2 )
        {
            Point aPos( pDoc->GetColOffset( nCol2, nTab2 ), pDoc->GetRowOffset( nRow2, nTab2 ) );
            if (!pDoc->ColHidden(nCol2, nTab2, NULL, &nLastCol))
                aPos.X() += pDoc->GetColWidth( nCol2, nTab2 ) / 4;
            if (!pDoc->RowHidden(nRow2, nTab2, NULL, &nLastRow))
                aPos.Y() += pDoc->GetRowHeight( nRow2, nTab2 ) / 2;
            TwipsToMM( aPos.X() );
            TwipsToMM( aPos.Y() );
            Point aEndPos = aPos;
            if ( bNegativePage )
                aEndPos.X() = -aEndPos.X();         
            if ( pObj->GetPoint( 1 ) != aEndPos )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );

                rData.maLastRect = lcl_UpdateCalcPoly(aCalcPoly, 1, aEndPos);
                pObj->SetPoint( aEndPos, 1 );
            }

            if( !bValid1 )
            {
                Point aStartPos( aPos.X() - DET_ARROW_OFFSET, aPos.Y() - DET_ARROW_OFFSET );
                if (aStartPos.X() < 0)
                    aStartPos.X() += (2 * DET_ARROW_OFFSET);
                if (aStartPos.Y() < 0)
                    aStartPos.Y() += (2 * DET_ARROW_OFFSET);
                if ( bNegativePage )
                    aStartPos.X() = -aStartPos.X();
                if ( pObj->GetPoint( 0 ) != aStartPos )
                {
                    if (bRecording)
                        AddCalcUndo( new SdrUndoGeoObj( *pObj ) );

                    rData.maLastRect = lcl_UpdateCalcPoly(aCalcPoly, 0, aStartPos);
                    pObj->SetPoint( aStartPos, 0 );
                }
            }
        }
    }
    else
    {
        bool bCanResize = bValid2 && !pObj->IsResizeProtect();

        
        ScDrawObjData& rNoRotatedAnchor = *GetNonRotatedObjData( pObj, true );
        if (rData.maLastRect.IsEmpty())
        {
            
            
            

            
            
            
            rNoRotatedAnchor.maStart = rData.maStart;
            rNoRotatedAnchor.maEnd = rData.maEnd;
            rNoRotatedAnchor.maStartOffset = rData.maStartOffset;
            rNoRotatedAnchor.maEndOffset = rData.maEndOffset;

            Rectangle aRect = pObj->GetLogicRect();

            
            
            
            
            
            
            
            
            
            
            ResizeLastRectFromAnchor( pObj, rData, true, bNegativePage, bCanResize, false );
            
            Rectangle aFullRect = rData.maLastRect;

            
            ResizeLastRectFromAnchor( pObj, rNoRotatedAnchor, true, bNegativePage, bCanResize );

            
            pObj->SetLogicRect(aFullRect);
            
            aRect = pObj->GetSnapRect();

            
            
            
            ScDrawLayer::GetCellAnchorFromPosition( *pObj, rData, *pDoc, nTab1, false, false );
            
            pObj->SetLogicRect(rNoRotatedAnchor.maLastRect);
        }

        
        ResizeLastRectFromAnchor( pObj, rData, false, bNegativePage, bCanResize );

        if( bCanResize )
        {
            Rectangle aNew = rData.maLastRect;

            if ( pObj->GetSnapRect() != aNew )
            {
                Rectangle aOld(pObj->GetSnapRect());

                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                if (pObj->IsPolyObj())
                {
                    
                    Size aSizeMove(aNew.Left()-aOld.Left(), aNew.Top()-aOld.Top());
                    pObj->NbcMove(aSizeMove);

                    double fXFrac = static_cast<double>(aNew.GetWidth()) / static_cast<double>(aOld.GetWidth());
                    double fYFrac = static_cast<double>(aNew.GetHeight()) / static_cast<double>(aOld.GetHeight());
                    pObj->NbcResize(aNew.TopLeft(), Fraction(fXFrac), Fraction(fYFrac));
                }
                
                
                rData.maLastRect = lcl_makeSafeRectangle(rData.maLastRect);
                pObj->SetSnapRect(rData.maLastRect);
                
                
                ResizeLastRectFromAnchor( pObj, rNoRotatedAnchor, true, bNegativePage, bCanResize );
            }
        }
        else
        {
            Point aPos( rData.maLastRect.getX(), rData.maLastRect.getY() );
            if ( pObj->GetRelativePos() != aPos )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                pObj->SetRelativePos( aPos );
            }
        }
        /*
         * If we were not allowed resize the object, then the end cell anchor
         * is possibly incorrect now, and if the object has no end-cell (e.g.
         * missing in original .xml) we are also forced to generate one
        */
        bool bEndAnchorIsBad = !bValid2 || pObj->IsResizeProtect();
        if (bEndAnchorIsBad)
        {
            
            ScDrawLayer::UpdateCellAnchorFromPositionEnd(*pObj, rData, *pDoc, nTab1, false);
            
            ScDrawLayer::UpdateCellAnchorFromPositionEnd(*pObj, rNoRotatedAnchor, *pDoc, nTab1 );
        }
    }
}

bool ScDrawLayer::GetPrintArea( ScRange& rRange, bool bSetHor, bool bSetVer ) const
{
    OSL_ENSURE( pDoc, "ScDrawLayer::GetPrintArea without document" );
    if ( !pDoc )
        return false;

    SCTAB nTab = rRange.aStart.Tab();
    OSL_ENSURE( rRange.aEnd.Tab() == nTab, "GetPrintArea: Tab differ" );

    sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );

    bool bAny = false;
    long nEndX = 0;
    long nEndY = 0;
    long nStartX = LONG_MAX;
    long nStartY = LONG_MAX;

    

    if (!bSetHor)
    {
        nStartX = 0;
        SCCOL nStartCol = rRange.aStart.Col();
            SCCOL i;
        for (i=0; i<nStartCol; i++)
            nStartX +=pDoc->GetColWidth(i,nTab);
        nEndX = nStartX;
        SCCOL nEndCol = rRange.aEnd.Col();
        for (i=nStartCol; i<=nEndCol; i++)
            nEndX += pDoc->GetColWidth(i,nTab);
        nStartX = TwipsToHmm( nStartX );
        nEndX   = TwipsToHmm( nEndX );
    }
    if (!bSetVer)
    {
        nStartY = pDoc->GetRowHeight( 0, rRange.aStart.Row()-1, nTab);
        nEndY = nStartY + pDoc->GetRowHeight( rRange.aStart.Row(),
                rRange.aEnd.Row(), nTab);
        nStartY = TwipsToHmm( nStartY );
        nEndY   = TwipsToHmm( nEndY );
    }

    if ( bNegativePage )
    {
        nStartX = -nStartX;     
        nEndX   = -nEndX;
        ::std::swap( nStartX, nEndX );
    }

    const SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page not found");
    if (pPage)
    {
        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
                            

            Rectangle aObjRect = pObject->GetCurrentBoundRect();
            sal_Bool bFit = sal_True;
            if ( !bSetHor && ( aObjRect.Right() < nStartX || aObjRect.Left() > nEndX ) )
                bFit = false;
            if ( !bSetVer && ( aObjRect.Bottom() < nStartY || aObjRect.Top() > nEndY ) )
                bFit = false;
            
            if ( bFit && pObject->GetLayer() != SC_LAYER_HIDDEN )
            {
                if (bSetHor)
                {
                    if (aObjRect.Left() < nStartX) nStartX = aObjRect.Left();
                    if (aObjRect.Right()  > nEndX) nEndX = aObjRect.Right();
                }
                if (bSetVer)
                {
                    if (aObjRect.Top()  < nStartY) nStartY = aObjRect.Top();
                    if (aObjRect.Bottom() > nEndY) nEndY = aObjRect.Bottom();
                }
                bAny = true;
            }

            pObject = aIter.Next();
        }
    }

    if ( bNegativePage )
    {
        nStartX = -nStartX;     
        nEndX   = -nEndX;
        ::std::swap( nStartX, nEndX );
    }

    if (bAny)
    {
        OSL_ENSURE( nStartX<=nEndX && nStartY<=nEndY, "Start/End falsch in ScDrawLayer::GetPrintArea" );

        if (bSetHor)
        {
            nStartX = HmmToTwips( nStartX );
            nEndX = HmmToTwips( nEndX );
            long nWidth;
            SCCOL i;

            nWidth = 0;
            for (i=0; i<=MAXCOL && nWidth<=nStartX; i++)
                nWidth += pDoc->GetColWidth(i,nTab);
            rRange.aStart.SetCol( i>0 ? (i-1) : 0 );

            nWidth = 0;
            for (i=0; i<=MAXCOL && nWidth<=nEndX; i++)          
                nWidth += pDoc->GetColWidth(i,nTab);
            rRange.aEnd.SetCol( i>0 ? (i-1) : 0 );
        }

        if (bSetVer)
        {
            nStartY = HmmToTwips( nStartY );
            nEndY = HmmToTwips( nEndY );
            SCROW nRow = pDoc->GetRowForHeight( nTab, nStartY);
            rRange.aStart.SetRow( nRow>0 ? (nRow-1) : 0);
            nRow = pDoc->GetRowForHeight( nTab, nEndY);
            rRange.aEnd.SetRow( nRow == MAXROW ? MAXROW :
                    (nRow>0 ? (nRow-1) : 0));
        }
    }
    else
    {
        if (bSetHor)
        {
            rRange.aStart.SetCol(0);
            rRange.aEnd.SetCol(0);
        }
        if (bSetVer)
        {
            rRange.aStart.SetRow(0);
            rRange.aEnd.SetRow(0);
        }
    }
    return bAny;
}

void ScDrawLayer::AddCalcUndo( SdrUndoAction* pUndo )
{
    if (bRecording)
    {
        if (!pUndoGroup)
            pUndoGroup = new SdrUndoGroup(*this);

        pUndoGroup->AddAction( pUndo );
    }
    else
        delete pUndo;
}

void ScDrawLayer::BeginCalcUndo(bool bDisableTextEditUsesCommonUndoManager)
{
    SetDisableTextEditUsesCommonUndoManager(bDisableTextEditUsesCommonUndoManager);
    DELETEZ(pUndoGroup);
    bRecording = true;
}

SdrUndoGroup* ScDrawLayer::GetCalcUndo()
{
    SdrUndoGroup* pRet = pUndoGroup;
    pUndoGroup = NULL;
    bRecording = false;
    SetDisableTextEditUsesCommonUndoManager(false);
    return pRet;
}

void ScDrawLayer::MoveArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                            SCsCOL nDx,SCsROW nDy, bool bInsDel, bool bUpdateNoteCaptionPos )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::MoveArea without document" );
    if ( !pDoc )
        return;

    if (!bAdjustEnabled)
        return;

    sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );

    Rectangle aRect = pDoc->GetMMRect( nCol1, nRow1, nCol2, nRow2, nTab );
    lcl_ReverseTwipsToMM( aRect );
    

    Point aMove;

    if (nDx > 0)
        for (SCsCOL s=0; s<nDx; s++)
            aMove.X() += pDoc->GetColWidth(s+(SCsCOL)nCol1,nTab);
    else
        for (SCsCOL s=-1; s>=nDx; s--)
            aMove.X() -= pDoc->GetColWidth(s+(SCsCOL)nCol1,nTab);
    if (nDy > 0)
        aMove.Y() += pDoc->GetRowHeight( nRow1, nRow1+nDy-1, nTab);
    else
        aMove.Y() -= pDoc->GetRowHeight( nRow1+nDy, nRow1-1, nTab);

    if ( bNegativePage )
        aMove.X() = -aMove.X();

    Point aTopLeft = aRect.TopLeft();       
    if (bInsDel)
    {
        if ( aMove.X() != 0 && nDx < 0 )    
            aTopLeft.X() += aMove.X();
        if ( aMove.Y() < 0 )
            aTopLeft.Y() += aMove.Y();
    }

        //
        
        //

    MoveCells( nTab, nCol1,nRow1, nCol2,nRow2, nDx,nDy, bUpdateNoteCaptionPos );
}

bool ScDrawLayer::HasObjectsInRows( SCTAB nTab, SCROW nStartRow, SCROW nEndRow )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::HasObjectsInRows without document" );
    if ( !pDoc )
        return false;

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page not found");
    if (!pPage)
        return false;

    
    if (!pPage->GetObjCount())
        return false;

    Rectangle aTestRect;

    aTestRect.Top() += pDoc->GetRowHeight( 0, nStartRow-1, nTab);

    if (nEndRow==MAXROW)
        aTestRect.Bottom() = MAXMM;
    else
    {
        aTestRect.Bottom() = aTestRect.Top();
        aTestRect.Bottom() += pDoc->GetRowHeight( nStartRow, nEndRow, nTab);
        TwipsToMM( aTestRect.Bottom() );
    }

    TwipsToMM( aTestRect.Top() );

    aTestRect.Left()  = 0;
    aTestRect.Right() = MAXMM;

    sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );
    if ( bNegativePage )
        MirrorRectRTL( aTestRect );

    bool bFound = false;

    Rectangle aObjRect;
    SdrObjListIter aIter( *pPage );
    SdrObject* pObject = aIter.Next();
    while ( pObject && !bFound )
    {
        aObjRect = pObject->GetSnapRect();  
        if (aTestRect.IsInside(aObjRect.TopLeft()) || aTestRect.IsInside(aObjRect.BottomLeft()))
            bFound = true;

        pObject = aIter.Next();
    }

    return bFound;
}

void ScDrawLayer::DeleteObjectsInArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1,
                                            SCCOL nCol2,SCROW nRow2 )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::DeleteObjectsInArea without document" );
    if ( !pDoc )
        return;

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page ?");
    if (!pPage)
        return;

    pPage->RecalcObjOrdNums();

    sal_uLong   nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        long nDelCount = 0;
        Rectangle aDelRect = pDoc->GetMMRect( nCol1, nRow1, nCol2, nRow2, nTab );

        SdrObject** ppObj = new SdrObject*[nObjCount];

        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            
            
            if (!IsNoteCaption( pObject ))
            {
                Rectangle aObjRect = pObject->GetCurrentBoundRect();
                if ( aDelRect.IsInside( aObjRect ) )
                    ppObj[nDelCount++] = pObject;
            }

            pObject = aIter.Next();
        }

        long i;
        if (bRecording)
            for (i=1; i<=nDelCount; i++)
                AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );

        for (i=1; i<=nDelCount; i++)
            pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );

        delete[] ppObj;
    }
}

void ScDrawLayer::DeleteObjectsInSelection( const ScMarkData& rMark )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::DeleteObjectsInSelection without document" );
    if ( !pDoc )
        return;

    if ( !rMark.IsMultiMarked() )
        return;

    ScRange aMarkRange;
    rMark.GetMultiMarkArea( aMarkRange );

    SCTAB nTabCount = pDoc->GetTableCount();
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nTabCount; ++itr)
    {
        SCTAB nTab = *itr;
        SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
        if (pPage)
        {
            pPage->RecalcObjOrdNums();
            sal_uLong   nObjCount = pPage->GetObjCount();
            if (nObjCount)
            {
                long nDelCount = 0;
                
                Rectangle aMarkBound = pDoc->GetMMRect(
                            aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                            aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(), nTab );

                SdrObject** ppObj = new SdrObject*[nObjCount];

                SdrObjListIter aIter( *pPage, IM_FLAT );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    
                    
                    if (!IsNoteCaption( pObject ))
                    {
                        Rectangle aObjRect = pObject->GetCurrentBoundRect();
                        if ( aMarkBound.IsInside( aObjRect ) )
                        {
                            ScRange aRange = pDoc->GetRange( nTab, aObjRect );
                            if (rMark.IsAllMarked(aRange))
                                ppObj[nDelCount++] = pObject;
                        }
                    }

                    pObject = aIter.Next();
                }

                

                long i;
                if (bRecording)
                    for (i=1; i<=nDelCount; i++)
                        AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );

                for (i=1; i<=nDelCount; i++)
                    pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );

                delete[] ppObj;
            }
        }
        else
        {
            OSL_FAIL("pPage?");
        }
    }
}

void ScDrawLayer::CopyToClip( ScDocument* pClipDoc, SCTAB nTab, const Rectangle& rRange )
{
    

    SdrPage* pSrcPage = GetPage(static_cast<sal_uInt16>(nTab));
    if (pSrcPage)
    {
        ScDrawLayer* pDestModel = NULL;
        SdrPage* pDestPage = NULL;

        SdrObjListIter aIter( *pSrcPage, IM_FLAT );
        SdrObject* pOldObject = aIter.Next();
        while (pOldObject)
        {
            Rectangle aObjRect = pOldObject->GetCurrentBoundRect();
            
            if ( rRange.IsInside( aObjRect ) && (pOldObject->GetLayer() != SC_LAYER_INTERN) && !IsNoteCaption( pOldObject ) )
            {
                if ( !pDestModel )
                {
                    pDestModel = pClipDoc->GetDrawLayer();      
                    if ( !pDestModel )
                    {
                        

                        pClipDoc->InitDrawLayer();                  
                        pDestModel = pClipDoc->GetDrawLayer();
                    }
                    if (pDestModel)
                        pDestPage = pDestModel->GetPage( static_cast<sal_uInt16>(nTab) );
                }

                OSL_ENSURE( pDestPage, "no page" );
                if (pDestPage)
                {
                    SdrObject* pNewObject = pOldObject->Clone();
                    pNewObject->SetModel(pDestModel);
                    pNewObject->SetPage(pDestPage);

                    uno::Reference< chart2::XChartDocument > xOldChart( ScChartHelper::GetChartFromSdrObject( pOldObject ) );
                    if(!xOldChart.is())
                        pNewObject->NbcMove(Size(0,0));
                    pDestPage->InsertObject( pNewObject );

                    
                    
                }
            }

            pOldObject = aIter.Next();
        }
    }
}

static sal_Bool lcl_IsAllInRange( const ::std::vector< ScRangeList >& rRangesVector, const ScRange& rClipRange )
{
    

    ::std::vector< ScRangeList >::const_iterator aIt = rRangesVector.begin();
    for( ;aIt!=rRangesVector.end(); ++aIt )
    {
        const ScRangeList& rRanges = *aIt;
        for ( size_t i = 0, nCount = rRanges.size(); i < nCount; i++ )
        {
            ScRange aRange = *rRanges[ i ];
            if ( !rClipRange.In( aRange ) )
            {
                return false;   
            }
        }
    }

    return sal_True;            
}

static sal_Bool lcl_MoveRanges( ::std::vector< ScRangeList >& rRangesVector, const ScRange& rSourceRange, const ScAddress& rDestPos )
{
    sal_Bool bChanged = false;

    ::std::vector< ScRangeList >::iterator aIt = rRangesVector.begin();
    for( ;aIt!=rRangesVector.end(); ++aIt )
    {
        ScRangeList& rRanges = *aIt;
        for ( size_t i = 0, nCount = rRanges.size(); i < nCount; i++ )
        {
            ScRange* pRange = rRanges[ i ];
            if ( rSourceRange.In( *pRange ) )
            {
                SCsCOL nDiffX = rDestPos.Col() - (SCsCOL)rSourceRange.aStart.Col();
                SCsROW nDiffY = rDestPos.Row() - (SCsROW)rSourceRange.aStart.Row();
                SCsTAB nDiffZ = rDestPos.Tab() - (SCsTAB)rSourceRange.aStart.Tab();
                pRange->Move( nDiffX, nDiffY, nDiffZ );
                bChanged = sal_True;
            }
        }
    }

    return bChanged;
}

void ScDrawLayer::CopyFromClip( ScDrawLayer* pClipModel, SCTAB nSourceTab, const Rectangle& rSourceRange,
                                    const ScAddress& rDestPos, const Rectangle& rDestRange )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::CopyFromClip without document" );
    if ( !pDoc )
        return;

    if (!pClipModel)
        return;

    if (bDrawIsInUndo)      
    {
        OSL_FAIL("CopyFromClip, bDrawIsInUndo");
        return;
    }

    sal_Bool bMirrorObj = ( rSourceRange.Left() < 0 && rSourceRange.Right() < 0 &&
                        rDestRange.Left()   > 0 && rDestRange.Right()   > 0 ) ||
                      ( rSourceRange.Left() > 0 && rSourceRange.Right() > 0 &&
                        rDestRange.Left()   < 0 && rDestRange.Right()   < 0 );
    Rectangle aMirroredSource = rSourceRange;
    if ( bMirrorObj )
        MirrorRectRTL( aMirroredSource );

    SCTAB nDestTab = rDestPos.Tab();

    SdrPage* pSrcPage = pClipModel->GetPage(static_cast<sal_uInt16>(nSourceTab));
    SdrPage* pDestPage = GetPage(static_cast<sal_uInt16>(nDestTab));
    OSL_ENSURE( pSrcPage && pDestPage, "draw page missing" );
    if ( !pSrcPage || !pDestPage )
        return;

    SdrObjListIter aIter( *pSrcPage, IM_FLAT );
    SdrObject* pOldObject = aIter.Next();

    ScDocument* pClipDoc = pClipModel->GetDocument();
    
    
    
    sal_Bool bSameDoc = pDoc && pClipDoc && pDoc->GetPool() == pClipDoc->GetPool();
    sal_Bool bDestClip = pDoc && pDoc->IsClipboard();

    
    
    OUString aDestTabName;
    sal_Bool bRestoreDestTabName = false;
    if( pOldObject && !bSameDoc && !bDestClip )
    {
        if( pDoc && pClipDoc )
        {
            OUString aSourceTabName;
            if( pClipDoc->GetName( nSourceTab, aSourceTabName )
                && pDoc->GetName( nDestTab, aDestTabName ) )
            {
                if( !aSourceTabName.equals(aDestTabName) &&
                    pDoc->ValidNewTabName(aSourceTabName) )
                {
                    bRestoreDestTabName = pDoc->RenameTab( nDestTab, aSourceTabName ); 
                }
            }
        }
    }

    
    Size aMove( rDestRange.Left() - aMirroredSource.Left(), rDestRange.Top() - aMirroredSource.Top() );

    long nDestWidth = rDestRange.GetWidth();
    long nDestHeight = rDestRange.GetHeight();
    long nSourceWidth = rSourceRange.GetWidth();
    long nSourceHeight = rSourceRange.GetHeight();

    long nWidthDiff = nDestWidth - nSourceWidth;
    long nHeightDiff = nDestHeight - nSourceHeight;

    Fraction aHorFract(1,1);
    Fraction aVerFract(1,1);
    sal_Bool bResize = false;
    
    
    if ( std::abs(nWidthDiff) > 1 && nDestWidth > 1 && nSourceWidth > 1 )
    {
        aHorFract = Fraction( nDestWidth, nSourceWidth );
        bResize = sal_True;
    }
    if ( std::abs(nHeightDiff) > 1 && nDestHeight > 1 && nSourceHeight > 1 )
    {
        aVerFract = Fraction( nDestHeight, nSourceHeight );
        bResize = sal_True;
    }
    Point aRefPos = rDestRange.TopLeft();       

    while (pOldObject)
    {
        Rectangle aObjRect = pOldObject->GetCurrentBoundRect();
        
        if ( rSourceRange.IsInside( aObjRect ) && (pOldObject->GetLayer() != SC_LAYER_INTERN) && !IsNoteCaption( pOldObject ) )
        {
            SdrObject* pNewObject = pOldObject->Clone();
            pNewObject->SetModel(this);
            pNewObject->SetPage(pDestPage);

            if ( bMirrorObj )
                MirrorRTL( pNewObject );        

            pNewObject->NbcMove( aMove );
            if ( bResize )
                pNewObject->NbcResize( aRefPos, aHorFract, aVerFract );

            pDestPage->InsertObject( pNewObject );
            if (bRecording)
                AddCalcUndo( new SdrUndoInsertObj( *pNewObject ) );

            

            if ( pNewObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                uno::Reference< embed::XEmbeddedObject > xIPObj = ((SdrOle2Obj*)pNewObject)->GetObjRef();
                uno::Reference< embed::XClassifiedObject > xClassified( xIPObj, uno::UNO_QUERY );
                SvGlobalName aObjectClassName;
                if ( xClassified.is() )
                {
                    try {
                        aObjectClassName = SvGlobalName( xClassified->getClassID() );
                    } catch( uno::Exception& )
                    {
                        
                    }
                }

                if ( xIPObj.is() && SotExchange::IsChart( aObjectClassName ) )
                {
                    uno::Reference< chart2::XChartDocument > xNewChart( ScChartHelper::GetChartFromSdrObject( pNewObject ) );
                    if( xNewChart.is() && !xNewChart->hasInternalDataProvider() )
                    {
                        OUString aChartName = ((SdrOle2Obj*)pNewObject)->GetPersistName();
                        ::std::vector< ScRangeList > aRangesVector;
                        pDoc->GetChartRanges( aChartName, aRangesVector, pDoc );
                        if( !aRangesVector.empty() )
                        {
                            sal_Bool bInSourceRange = false;
                            ScRange aClipRange;
                            if ( pClipDoc )
                            {
                                SCCOL nClipStartX;
                                SCROW nClipStartY;
                                SCCOL nClipEndX;
                                SCROW nClipEndY;
                                pClipDoc->GetClipStart( nClipStartX, nClipStartY );
                                pClipDoc->GetClipArea( nClipEndX, nClipEndY, true );
                                nClipEndX = nClipEndX + nClipStartX;
                                nClipEndY += nClipStartY;   

                                SCTAB nClipTab = bRestoreDestTabName ? nDestTab : nSourceTab;
                                aClipRange = ScRange( nClipStartX, nClipStartY, nClipTab,
                                                        nClipEndX, nClipEndY, nClipTab );

                                bInSourceRange = lcl_IsAllInRange( aRangesVector, aClipRange );
                            }

                            
                            if ( ( bInSourceRange || bSameDoc ) && !bDestClip )
                            {
                                if ( bInSourceRange )
                                {
                                    if ( rDestPos != aClipRange.aStart )
                                    {
                                        
                                        if ( lcl_MoveRanges( aRangesVector, aClipRange, rDestPos ) )
                                            pDoc->SetChartRanges( aChartName, aRangesVector );
                                    }
                                }
                                else
                                {
                                    
                                }
                            }
                            else
                            {
                                
                                

                                uno::Reference< chart::XChartDocument > xOldChartDoc( ScChartHelper::GetChartFromSdrObject( pOldObject ), uno::UNO_QUERY );
                                uno::Reference< chart::XChartDocument > xNewChartDoc( xNewChart, uno::UNO_QUERY );
                                if( xOldChartDoc.is() && xNewChartDoc.is() )
                                    xNewChartDoc->attachData( xOldChartDoc->getData() );

                                
                            }
                        }
                    }
                }
            }
        }

        pOldObject = aIter.Next();
    }

    if( bRestoreDestTabName )
        pDoc->RenameTab( nDestTab, aDestTabName );
}

void ScDrawLayer::MirrorRTL( SdrObject* pObj )
{
    sal_uInt16 nIdent = pObj->GetObjIdentifier();

    
    
    sal_Bool bCanMirror = ( nIdent != OBJ_GRAF && nIdent != OBJ_OLE2 );
    if (bCanMirror)
    {
        SdrObjTransformInfoRec aInfo;
        pObj->TakeObjInfo( aInfo );
        bCanMirror = aInfo.bMirror90Allowed;
    }

    if (bCanMirror)
    {
        Point aRef1( 0, 0 );
        Point aRef2( 0, 1 );
        if (bRecording)
            AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
        pObj->Mirror( aRef1, aRef2 );
    }
    else
    {
        
        
        
        Rectangle aObjRect = pObj->GetLogicRect();
        Size aMoveSize( -(aObjRect.Left() + aObjRect.Right()), 0 );
        if (bRecording)
            AddCalcUndo( new SdrUndoMoveObj( *pObj, aMoveSize ) );
        pObj->Move( aMoveSize );
    }
}

void ScDrawLayer::MirrorRectRTL( Rectangle& rRect )
{
    
    long nTemp = rRect.Left();
    rRect.Left() = -rRect.Right();
    rRect.Right() = -nTemp;
}

Rectangle ScDrawLayer::GetCellRect( ScDocument& rDoc, const ScAddress& rPos, bool bMergedCell )
{
    Rectangle aCellRect;
    OSL_ENSURE( ValidColRowTab( rPos.Col(), rPos.Row(), rPos.Tab() ), "ScDrawLayer::GetCellRect - invalid cell address" );
    if( ValidColRowTab( rPos.Col(), rPos.Row(), rPos.Tab() ) )
    {
        
        Point aTopLeft;
        for( SCCOL nCol = 0; nCol < rPos.Col(); ++nCol )
            aTopLeft.X() += rDoc.GetColWidth( nCol, rPos.Tab() );
        if( rPos.Row() > 0 )
            aTopLeft.Y() += rDoc.GetRowHeight( 0, rPos.Row() - 1, rPos.Tab() );

        
        ScAddress aEndPos = rPos;
        if( bMergedCell )
        {
            const ScMergeAttr* pMerge = static_cast< const ScMergeAttr* >( rDoc.GetAttr( rPos.Col(), rPos.Row(), rPos.Tab(), ATTR_MERGE ) );
            if( pMerge->GetColMerge() > 1 )
                aEndPos.IncCol( pMerge->GetColMerge() - 1 );
            if( pMerge->GetRowMerge() > 1 )
                aEndPos.IncRow( pMerge->GetRowMerge() - 1 );
        }
        Point aBotRight = aTopLeft;
        for( SCCOL nCol = rPos.Col(); nCol <= aEndPos.Col(); ++nCol )
            aBotRight.X() += rDoc.GetColWidth( nCol, rPos.Tab() );
        aBotRight.Y() += rDoc.GetRowHeight( rPos.Row(), aEndPos.Row(), rPos.Tab() );

        
        aTopLeft.X() = static_cast< long >( aTopLeft.X() * HMM_PER_TWIPS );
        aTopLeft.Y() = static_cast< long >( aTopLeft.Y() * HMM_PER_TWIPS );
        aBotRight.X() = static_cast< long >( aBotRight.X() * HMM_PER_TWIPS );
        aBotRight.Y() = static_cast< long >( aBotRight.Y() * HMM_PER_TWIPS );

        aCellRect = Rectangle( aTopLeft, aBotRight );
        if( rDoc.IsNegativePage( rPos.Tab() ) )
            MirrorRectRTL( aCellRect );
    }
    return aCellRect;
}

OUString ScDrawLayer::GetVisibleName( SdrObject* pObj )
{
    OUString aName = pObj->GetName();
    if ( pObj->GetObjIdentifier() == OBJ_OLE2 )
    {
        
        
        
        

        if ( aName.isEmpty() )
            aName = static_cast<SdrOle2Obj*>(pObj)->GetPersistName();
    }
    return aName;
}

inline sal_Bool IsNamedObject( SdrObject* pObj, const OUString& rName )
{
    
    

    return ( pObj->GetName().equals(rName) ||
            ( pObj->GetObjIdentifier() == OBJ_OLE2 &&
              static_cast<SdrOle2Obj*>(pObj)->GetPersistName() == rName ) );
}

SdrObject* ScDrawLayer::GetNamedObject( const OUString& rName, sal_uInt16 nId, SCTAB& rFoundTab ) const
{
    sal_uInt16 nTabCount = GetPageCount();
    for (sal_uInt16 nTab=0; nTab<nTabCount; nTab++)
    {
        const SdrPage* pPage = GetPage(nTab);
        OSL_ENSURE(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
            SdrObject* pObject = aIter.Next();
            while (pObject)
            {
                if ( nId == 0 || pObject->GetObjIdentifier() == nId )
                    if ( IsNamedObject( pObject, rName ) )
                    {
                        rFoundTab = static_cast<SCTAB>(nTab);
                        return pObject;
                    }

                pObject = aIter.Next();
            }
        }
    }

    return NULL;
}

OUString ScDrawLayer::GetNewGraphicName( long* pnCounter ) const
{
    OUString aBase = ScGlobal::GetRscString(STR_GRAPHICNAME);
    aBase += " ";

    bool bThere = true;
    OUString aGraphicName;
    SCTAB nDummy;
    long nId = pnCounter ? *pnCounter : 0;
    while (bThere)
    {
        ++nId;
        aGraphicName = aBase;
        aGraphicName += OUString::number( nId );
        bThere = ( GetNamedObject( aGraphicName, 0, nDummy ) != NULL );
    }

    if ( pnCounter )
        *pnCounter = nId;

    return aGraphicName;
}

void ScDrawLayer::EnsureGraphicNames()
{
    

    sal_uInt16 nTabCount = GetPageCount();
    for (sal_uInt16 nTab=0; nTab<nTabCount; nTab++)
    {
        SdrPage* pPage = GetPage(nTab);
        OSL_ENSURE(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
            SdrObject* pObject = aIter.Next();

            /* The index passed to GetNewGraphicName() will be set to
                the used index in each call. This prevents the repeated search
                for all names from 1 to current index. */
            long nCounter = 0;

            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_GRAF && pObject->GetName().isEmpty())
                    pObject->SetName( GetNewGraphicName( &nCounter ) );

                pObject = aIter.Next();
            }
        }
    }
}

namespace
{
    SdrObjUserData* GetFirstUserDataOfType(const SdrObject *pObj, sal_uInt16 nId)
    {
        sal_uInt16 nCount = pObj ? pObj->GetUserDataCount() : 0;
        for( sal_uInt16 i = 0; i < nCount; i++ )
        {
            SdrObjUserData* pData = pObj->GetUserData( i );
            if( pData && pData->GetInventor() == SC_DRAWLAYER && pData->GetId() == nId )
                return pData;
        }
        return NULL;
    }

    void DeleteFirstUserDataOfType(SdrObject *pObj, sal_uInt16 nId)
    {
        sal_uInt16 nCount = pObj ? pObj->GetUserDataCount() : 0;
        for( sal_uInt16 i = nCount; i > 0; i-- )
        {
            SdrObjUserData* pData = pObj->GetUserData( i-1 );
            if( pData && pData->GetInventor() == SC_DRAWLAYER && pData->GetId() == nId )
                pObj->DeleteUserData(i-1);
        }
    }
}

void ScDrawLayer::SetVisualCellAnchored( SdrObject &rObj, const ScDrawObjData &rAnchor )
{
    ScDrawObjData* pAnchor = GetNonRotatedObjData( &rObj, true );
    pAnchor->maStart = rAnchor.maStart;
    pAnchor->maEnd = rAnchor.maEnd;
    pAnchor->maStartOffset = rAnchor.maStartOffset;
    pAnchor->maEndOffset = rAnchor.maEndOffset;
}

void ScDrawLayer::SetCellAnchored( SdrObject &rObj, const ScDrawObjData &rAnchor )
{
    ScDrawObjData* pAnchor = GetObjData( &rObj, true );
    pAnchor->maStart = rAnchor.maStart;
    pAnchor->maEnd = rAnchor.maEnd;
    pAnchor->maStartOffset = rAnchor.maStartOffset;
    pAnchor->maEndOffset = rAnchor.maEndOffset;
}


void ScDrawLayer::SetCellAnchoredFromPosition( SdrObject &rObj, const ScDocument &rDoc, SCTAB nTab )
{
    ScDrawObjData aAnchor;
    
    
    GetCellAnchorFromPosition( rObj, aAnchor, rDoc, nTab, false );
    SetCellAnchored( rObj, aAnchor );
    
    
    ScDrawObjData aVisAnchor;
    GetCellAnchorFromPosition( rObj, aVisAnchor, rDoc, nTab );
    SetVisualCellAnchored( rObj, aVisAnchor );
    
    
    if ( ScDrawObjData* pAnchor = GetObjData( &rObj ) )
    {
        pAnchor->maLastRect = rObj.GetSnapRect();
    }
}

void ScDrawLayer::GetCellAnchorFromPosition( SdrObject &rObj, ScDrawObjData &rAnchor, const ScDocument &rDoc, SCTAB nTab, bool bUseLogicRect, bool bHiddenAsZero )
{
    Rectangle aObjRect( bUseLogicRect ? rObj.GetLogicRect() : rObj.GetSnapRect() );
    ScRange aRange = rDoc.GetRange( nTab, aObjRect, bHiddenAsZero );

    Rectangle aCellRect;

    rAnchor.maStart = aRange.aStart;
    aCellRect = rDoc.GetMMRect( aRange.aStart.Col(), aRange.aStart.Row(),
      aRange.aStart.Col(), aRange.aStart.Row(), aRange.aStart.Tab(), bHiddenAsZero );
    rAnchor.maStartOffset.Y() = aObjRect.Top()-aCellRect.Top();
    if (!rDoc.IsNegativePage(nTab))
        rAnchor.maStartOffset.X() = aObjRect.Left()-aCellRect.Left();
    else
        rAnchor.maStartOffset.X() = aCellRect.Right()-aObjRect.Right();

    rAnchor.maEnd = aRange.aEnd;
    aCellRect = rDoc.GetMMRect( aRange.aEnd.Col(), aRange.aEnd.Row(),
      aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aEnd.Tab(), bHiddenAsZero );
    rAnchor.maEndOffset.Y() = aObjRect.Bottom()-aCellRect.Top();
    if (!rDoc.IsNegativePage(nTab))
        rAnchor.maEndOffset.X() = aObjRect.Right()-aCellRect.Left();
    else
        rAnchor.maEndOffset.X() = aCellRect.Right()-aObjRect.Left();

}


void ScDrawLayer::UpdateCellAnchorFromPositionEnd( SdrObject &rObj, ScDrawObjData &rAnchor, const ScDocument &rDoc, SCTAB nTab, bool bUseLogicRect )
{
    Rectangle aObjRect(bUseLogicRect ? rObj.GetLogicRect() : rObj.GetSnapRect());
    ScRange aRange = rDoc.GetRange( nTab, aObjRect );

    ScDrawObjData* pAnchor = &rAnchor;
    pAnchor->maEnd = aRange.aEnd;

    Rectangle aCellRect;
    aCellRect = rDoc.GetMMRect( aRange.aEnd.Col(), aRange.aEnd.Row(),
      aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aEnd.Tab() );
    pAnchor->maEndOffset.Y() = aObjRect.Bottom()-aCellRect.Top();
    if (!rDoc.IsNegativePage(nTab))
        pAnchor->maEndOffset.X() = aObjRect.Right()-aCellRect.Left();
    else
        pAnchor->maEndOffset.X() = aCellRect.Right()-aObjRect.Left();
}

bool ScDrawLayer::IsCellAnchored( const SdrObject& rObj )
{
    
    
    return GetFirstUserDataOfType(&rObj, SC_UD_OBJDATA) != NULL;
}

void ScDrawLayer::SetPageAnchored( SdrObject &rObj )
{
    DeleteFirstUserDataOfType(&rObj, SC_UD_OBJDATA);
    DeleteFirstUserDataOfType(&rObj, SC_UD_OBJDATA);
}

ScAnchorType ScDrawLayer::GetAnchorType( const SdrObject &rObj )
{
    
    
    return ScDrawLayer::GetObjData(const_cast<SdrObject*>(&rObj)) ? SCA_CELL : SCA_PAGE;
}

ScDrawObjData* ScDrawLayer::GetNonRotatedObjData( SdrObject* pObj, bool bCreate )
{
    sal_uInt16 nCount = pObj ? pObj->GetUserDataCount() : 0;
    sal_uInt16 nFound = 0;
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        SdrObjUserData* pData = pObj->GetUserData( i );
        if( pData && pData->GetInventor() == SC_DRAWLAYER && pData->GetId() == SC_UD_OBJDATA && ++nFound == 2 )
            return (ScDrawObjData*)pData;
    }
    if( pObj && bCreate )
    {
        ScDrawObjData* pData = new ScDrawObjData;
        pObj->AppendUserData(pData);
        return pData;
    }
    return 0;
}

ScDrawObjData* ScDrawLayer::GetObjData( SdrObject* pObj, bool bCreate )
{
    if (SdrObjUserData *pData = GetFirstUserDataOfType(pObj, SC_UD_OBJDATA))
        return (ScDrawObjData*) pData;

    if( pObj && bCreate )
    {
        ScDrawObjData* pData = new ScDrawObjData;
        pObj->AppendUserData(pData);
        return pData;
    }
    return 0;
}

ScDrawObjData* ScDrawLayer::GetObjDataTab( SdrObject* pObj, SCTAB nTab )
{
    ScDrawObjData* pData = GetObjData( pObj );
    if ( pData )
    {
        if ( pData->maStart.IsValid() )
            pData->maStart.SetTab( nTab );
        if ( pData->maEnd.IsValid() )
            pData->maEnd.SetTab( nTab );
    }
    return pData;
}

bool ScDrawLayer::IsNoteCaption( SdrObject* pObj )
{
    ScDrawObjData* pData = pObj ? GetObjData( pObj ) : 0;
    return pData && pData->meType == ScDrawObjData::CellNote;
}

ScDrawObjData* ScDrawLayer::GetNoteCaptionData( SdrObject* pObj, SCTAB nTab )
{
    ScDrawObjData* pData = pObj ? GetObjDataTab( pObj, nTab ) : 0;
    return (pData && pData->meType == ScDrawObjData::CellNote) ? pData : 0;
}

ScIMapInfo* ScDrawLayer::GetIMapInfo( SdrObject* pObj )
{
    return (ScIMapInfo*)GetFirstUserDataOfType(pObj, SC_UD_IMAPDATA);
}

IMapObject* ScDrawLayer::GetHitIMapObject( SdrObject* pObj,
                                          const Point& rWinPoint, const Window& rCmpWnd )
{
    const MapMode       aMap100( MAP_100TH_MM );
    MapMode             aWndMode = rCmpWnd.GetMapMode();
    Point               aRelPoint( rCmpWnd.LogicToLogic( rWinPoint, &aWndMode, &aMap100 ) );
    Rectangle           aLogRect = rCmpWnd.LogicToLogic( pObj->GetLogicRect(), &aWndMode, &aMap100 );
    ScIMapInfo*         pIMapInfo = GetIMapInfo( pObj );
    IMapObject*         pIMapObj = NULL;

    if ( pIMapInfo )
    {
        Size        aGraphSize;
        ImageMap&   rImageMap = (ImageMap&) pIMapInfo->GetImageMap();
        Graphic     aGraphic;
        sal_Bool        bObjSupported = false;

        if ( pObj->ISA( SdrGrafObj )  ) 
        {
            const SdrGrafObj*   pGrafObj = (const SdrGrafObj*) pObj;
            const GeoStat&      rGeo = pGrafObj->GetGeoStat();
            const Graphic&      rGraphic = pGrafObj->GetGraphic();

            
            if ( rGeo.nDrehWink )
                RotatePoint( aRelPoint, aLogRect.TopLeft(), -rGeo.nSin, rGeo.nCos );

            
            if ( ( (const SdrGrafObjGeoData*) pGrafObj->GetGeoData() )->bMirrored )
                aRelPoint.X() = aLogRect.Right() + aLogRect.Left() - aRelPoint.X();

            
            if ( rGeo.nShearWink )
                ShearPoint( aRelPoint, aLogRect.TopLeft(), -rGeo.nTan );


            if ( rGraphic.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
                aGraphSize = rCmpWnd.PixelToLogic( rGraphic.GetPrefSize(),
                                                         aMap100 );
            else
                aGraphSize = OutputDevice::LogicToLogic( rGraphic.GetPrefSize(),
                                                         rGraphic.GetPrefMapMode(),
                                                         aMap100 );

            bObjSupported = sal_True;
        }
        else if ( pObj->ISA( SdrOle2Obj ) ) 
        {
            
            aGraphSize = ((const SdrOle2Obj*)pObj)->GetOrigObjSize();
            bObjSupported = true;
        }

        
        if ( bObjSupported )
        {
            
            aRelPoint -= aLogRect.TopLeft();
            pIMapObj = rImageMap.GetHitIMapObject( aGraphSize, aLogRect.GetSize(), aRelPoint );
        }
    }

    return pIMapObj;
}

ScMacroInfo* ScDrawLayer::GetMacroInfo( SdrObject* pObj, bool bCreate )
{
    if (SdrObjUserData *pData = GetFirstUserDataOfType(pObj, SC_UD_MACRODATA))
        return (ScMacroInfo*) pData;

    if ( bCreate )
    {
        ScMacroInfo* pData = new ScMacroInfo;
        pObj->AppendUserData(pData);
        return pData;
    }
    return 0;
}

ImageMap* ScDrawLayer::GetImageMapForObject(SdrObject* pObj)
{
    ScIMapInfo* pIMapInfo = const_cast<ScIMapInfo*>( GetIMapInfo( pObj ) );
    if ( pIMapInfo )
    {
        return const_cast<ImageMap*>( &(pIMapInfo->GetImageMap()) );
    }
    return NULL;
}

sal_Int32 ScDrawLayer::GetHyperlinkCount(SdrObject* pObj)
{
    sal_Int32 nHLCount = 0;
    ScMacroInfo* pMacroInfo = GetMacroInfo(pObj, false);
    if (pMacroInfo)
        
        nHLCount = 0; 
    return nHLCount;
}

void ScDrawLayer::SetGlobalDrawPersist(SfxObjectShell* pPersist)
{
    OSL_ENSURE(!pGlobalDrawPersist,"Multiple SetGlobalDrawPersist");
    pGlobalDrawPersist = pPersist;
}

void ScDrawLayer::SetChanged( bool bFlg /* = true */ )
{
    if ( bFlg && pDoc )
        pDoc->SetChartListenerCollectionNeedsUpdate( true );
    FmFormModel::SetChanged( bFlg );
}

SdrLayerID ScDrawLayer::GetControlExportLayerId( const SdrObject & ) const
{
    
    return SC_LAYER_FRONT;
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ScDrawLayer::createUnoModel()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xRet;
    if( pDoc && pDoc->GetDocumentShell() )
        xRet = pDoc->GetDocumentShell()->GetModel();

    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
