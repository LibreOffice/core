/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <i18npool/mslangid.hxx>
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

#include <vcl/field.hxx>

#define DET_ARROW_OFFSET    1000

using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScTabDeletedHint, SfxHint);
TYPEINIT1(ScTabSizeChangedHint, SfxHint);

static ScDrawObjFactory* pFac = NULL;
static E3dObjFactory* pF3d = NULL;
static sal_uInt16 nInst = 0;

SfxObjectShell* ScDrawLayer::pGlobalDrawPersist = NULL;

sal_Bool bDrawIsInUndo = false;         //! Member

// -----------------------------------------------------------------------

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
    OSL_ENSURE(pData,"ScUndoObjData: Daten nicht da");
    if (pData)
    {
        pData->maStart = aOldStt;
        pData->maEnd = aOldEnd;
    }
}

void ScUndoObjData::Redo()
{
    ScDrawObjData* pData = ScDrawLayer::GetObjData( pObj );
    OSL_ENSURE(pData,"ScUndoObjData: Daten nicht da");
    if (pData)
    {
        pData->maStart = aNewStt;
        pData->maEnd = aNewEnd;
    }
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------


ScDrawLayer::ScDrawLayer( ScDocument* pDocument, const String& rName ) :
    FmFormModel( SvtPathOptions().GetPalettePath(),
                 NULL,                          // SfxItemPool* Pool
                 pGlobalDrawPersist ?
                     pGlobalDrawPersist :
                     ( pDocument ? pDocument->GetDocumentShell() : NULL ),
                 sal_True ),        // bUseExtColorTable (is set below)
    aName( rName ),
    pDoc( pDocument ),
    pUndoGroup( NULL ),
    bRecording( false ),
    bAdjustEnabled( sal_True ),
    bHyphenatorSet( false )
{
    pGlobalDrawPersist = NULL;          // nur einmal benutzen

    SfxObjectShell* pObjSh = pDocument ? pDocument->GetDocumentShell() : NULL;
    XColorListRef pXCol = XColorList::GetStdColorList();
    if ( pObjSh )
    {
        SetObjectShell( pObjSh );

        // set color table
        SvxColorListItem* pColItem = (SvxColorListItem*) pObjSh->GetItem( SID_COLOR_TABLE );
        if ( pColItem )
            pXCol = pColItem->GetColorList();
    }
    SetPropertyList( static_cast<XPropertyList *> (pXCol.get()) );

    SetSwapGraphics(sal_True);

    SetScaleUnit(MAP_100TH_MM);
    SfxItemPool& rPool = GetItemPool();
    rPool.SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    SvxFrameDirectionItem aModeItem( FRMDIR_ENVIRONMENT, EE_PARA_WRITINGDIR );
    rPool.SetPoolDefaultItem( aModeItem );

    // #i33700#
    // Set shadow distance defaults as PoolDefaultItems. Details see bug.
    rPool.SetPoolDefaultItem(SdrShadowXDistItem(300));
    rPool.SetPoolDefaultItem(SdrShadowYDistItem(300));

    // default for script spacing depends on locale, see SdDrawDocument ctor in sd
    LanguageType eOfficeLanguage = Application::GetSettings().GetLanguage();
    if (MsLangId::isKorean(eOfficeLanguage) || eOfficeLanguage == LANGUAGE_JAPANESE)
    {
        // secondary is edit engine pool
        rPool.GetSecondaryPool()->SetPoolDefaultItem( SvxScriptSpaceItem( false, EE_PARA_ASIANCJKSPACING ) );
    }

    rPool.FreezeIdRanges();                         // the pool is also used directly

    SdrLayerAdmin& rAdmin = GetLayerAdmin();
    rAdmin.NewLayer(rtl::OUString("vorne"),    SC_LAYER_FRONT);
    rAdmin.NewLayer(rtl::OUString("hinten"),   SC_LAYER_BACK);
    rAdmin.NewLayer(rtl::OUString("intern"),   SC_LAYER_INTERN);
    rAdmin.NewLayer(rtl::OUString("Controls"), SC_LAYER_CONTROLS);
    rAdmin.NewLayer(rtl::OUString("hidden"),   SC_LAYER_HIDDEN);
    // "Controls" is new - must also be created when loading

    //  Link fuer URL-Fields setzen
    ScModule* pScMod = SC_MOD();
    Outliner& rOutliner = GetDrawOutliner();
    rOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );

    Outliner& rHitOutliner = GetHitTestOutliner();
    rHitOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );

    // set FontHeight pool defaults without changing static SdrEngineDefaults
    SfxItemPool* pOutlinerPool = rOutliner.GetEditTextObjectPool();
    if ( pOutlinerPool )
         pItemPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT ));           // 12Pt
    SfxItemPool* pHitOutlinerPool = rHitOutliner.GetEditTextObjectPool();
    if ( pHitOutlinerPool )
         pHitOutlinerPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT ));    // 12Pt

    //  URL-Buttons haben keinen Handler mehr, machen alles selber

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

        bHyphenatorSet = sal_True;
    }
}

SdrPage* ScDrawLayer::AllocPage(bool bMasterPage)
{
    //  don't create basic until it is needed
    StarBASIC* pBasic = NULL;
    ScDrawPage* pPage = new ScDrawPage( *this, pBasic, bMasterPage);
    return pPage;
}

sal_Bool ScDrawLayer::HasObjects() const
{
    sal_Bool bFound = false;

    sal_uInt16 nCount = GetPageCount();
    for (sal_uInt16 i=0; i<nCount && !bFound; i++)
        if (GetPage(i)->GetObjCount())
            bFound = sal_True;

    return bFound;
}

void ScDrawLayer::UpdateBasic()
{
    //  don't create basic until it is needed
    //! remove this method?
}

SdrModel* ScDrawLayer::AllocModel() const
{
    //  Allocated model (for clipboard etc) must not have a pointer
    //  to the original model's document, pass NULL as document:

    return new ScDrawLayer( NULL, aName );
}

Window* ScDrawLayer::GetCurDocViewWin()
{
    OSL_ENSURE( pDoc, "ScDrawLayer::GetCurDocViewWin without document" );
    if ( !pDoc )
        return NULL;

    SfxViewShell* pViewSh = SfxViewShell::Current();
    SfxObjectShell* pObjSh = pDoc->GetDocumentShell();

    if (pViewSh && pViewSh->GetObjectShell() == pObjSh)
        return pViewSh->GetWindow();

    return NULL;
}

sal_Bool ScDrawLayer::ScAddPage( SCTAB nTab )
{
    if (bDrawIsInUndo)
        return false;   // not inserted

    ScDrawPage* pPage = (ScDrawPage*)AllocPage( false );
    InsertPage(pPage, static_cast<sal_uInt16>(nTab));
    if (bRecording)
        AddCalcUndo(new SdrUndoNewPage(*pPage));

    ResetTab(nTab, pDoc->GetTableCount()-1);
    return true;        // inserted
}

void ScDrawLayer::ScRemovePage( SCTAB nTab )
{
    if (bDrawIsInUndo)
        return;

    Broadcast( ScTabDeletedHint( nTab ) );
    if (bRecording)
    {
        SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
        AddCalcUndo(new SdrUndoDelPage(*pPage));        // Undo-Action wird Owner der Page
        RemovePage( static_cast<sal_uInt16>(nTab) );                            // nur austragen, nicht loeschen
    }
    else
        DeletePage( static_cast<sal_uInt16>(nTab) );                            // einfach weg damit

    ResetTab(nTab, pDoc->GetTableCount()-1);
}

void ScDrawLayer::ScRenamePage( SCTAB nTab, const String& rNewName )
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

void ScDrawLayer::ScCopyPage( sal_uInt16 nOldPos, sal_uInt16 nNewPos, sal_Bool bAlloc )
{
    //! remove argument bAlloc (always sal_False)

    if (bDrawIsInUndo)
        return;

    SdrPage* pOldPage = GetPage(nOldPos);
    SdrPage* pNewPage = bAlloc ? AllocPage(false) : GetPage(nNewPos);

    // kopieren

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

    if (bAlloc)
        InsertPage(pNewPage, nNewPos);

    ResetTab(static_cast<SCTAB>(nNewPos), pDoc->GetTableCount()-1);
}

void ScDrawLayer::ResetTab( SCTAB nStart, SCTAB nEnd )
{
    SCTAB nPageSize = static_cast<SCTAB>(GetPageCount());
    if (nPageSize < 0)
        // No drawing pages exist.
        return;

    if (nEnd >= nPageSize)
        // Avoid iterating beyond the last existing page.
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
    OSL_ENSURE(pPage,"Page nicht gefunden");
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
            Broadcast( ScTabSizeChangedHint( static_cast<SCTAB>(nPageNo) ) );   // SetWorkArea() an den Views
        }

        // Detektivlinien umsetzen (an neue Hoehen/Breiten anpassen)
        //  auch wenn Groesse gleich geblieben ist
        //  (einzelne Zeilen/Spalten koennen geaendert sein)

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
    //Can't have a zero width dimension
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
            if( ScPostIt* pNote = pDoc->GetNotes( rData.maStart.Tab() )->findByAddress( rData.maStart ) )
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
        // Validation circle for detective.
        rData.maLastRect = pObj->GetLogicRect();

        Point aPos( pDoc->GetColOffset( nCol1, nTab1 ), pDoc->GetRowOffset( nRow1, nTab1 ) );
        TwipsToMM( aPos.X() );
        TwipsToMM( aPos.Y() );

        //  Berechnung und Werte wie in detfunc.cxx

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
        //! nicht mehrere Undos fuer ein Objekt erzeugen (hinteres kann dann weggelassen werden)

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
                aStartPos.X() = -aStartPos.X();     // don't modify aPos - used below
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
                aEndPos.X() = -aEndPos.X();         // don't modify aPos - used below
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

        //First time positioning, must be able to at least move it
        if (rData.maLastRect.IsEmpty())
            rData.maLastRect = pObj->GetLogicRect();

        OSL_ENSURE( bValid1, "ScDrawLayer::RecalcPos - invalid start position" );
        Point aPos( pDoc->GetColOffset( nCol1, nTab1 ), pDoc->GetRowOffset( nRow1, nTab1 ) );
        TwipsToMM( aPos.X() );
        TwipsToMM( aPos.Y() );
        aPos += lcl_calcAvailableDiff(*pDoc, nCol1, nRow1, nTab1, rData.maStartOffset);

        if( bCanResize )
        {
            Point aEnd( pDoc->GetColOffset( nCol2, nTab2 ), pDoc->GetRowOffset( nRow2, nTab2 ) );
            TwipsToMM( aEnd.X() );
            TwipsToMM( aEnd.Y() );
            aEnd += lcl_calcAvailableDiff(*pDoc, nCol2, nRow2, nTab2, rData.maEndOffset);

            Rectangle aNew( aPos, aEnd );
            if ( bNegativePage )
                MirrorRectRTL( aNew );
            if ( pObj->GetLogicRect() != aNew )
            {
                Rectangle aOld(pObj->GetLogicRect());

                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                rData.maLastRect = lcl_makeSafeRectangle(aNew);
                if (pObj->IsPolyObj())
                {
                    // Polyline objects need special treatment.
                    Size aSizeMove(aNew.Left()-aOld.Left(), aNew.Top()-aOld.Top());
                    pObj->NbcMove(aSizeMove);

                    double fXFrac = static_cast<double>(aNew.GetWidth()) / static_cast<double>(aOld.GetWidth());
                    double fYFrac = static_cast<double>(aNew.GetHeight()) / static_cast<double>(aOld.GetHeight());
                    pObj->NbcResize(aNew.TopLeft(), Fraction(fXFrac), Fraction(fYFrac));
                }

                pObj->SetLogicRect(rData.maLastRect);
            }
        }
        else
        {
            if ( bNegativePage )
                aPos.X() = -aPos.X() - rData.maLastRect.GetWidth();
            if ( pObj->GetRelativePos() != aPos )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                rData.maLastRect.SetPos( aPos );
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
            ScDrawLayer::UpdateCellAnchorFromPositionEnd(*pObj, *pDoc, nTab1);
    }
}

sal_Bool ScDrawLayer::GetPrintArea( ScRange& rRange, sal_Bool bSetHor, sal_Bool bSetVer ) const
{
    OSL_ENSURE( pDoc, "ScDrawLayer::GetPrintArea without document" );
    if ( !pDoc )
        return false;

    SCTAB nTab = rRange.aStart.Tab();
    OSL_ENSURE( rRange.aEnd.Tab() == nTab, "GetPrintArea: Tab unterschiedlich" );

    sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );

    sal_Bool bAny = false;
    long nEndX = 0;
    long nEndY = 0;
    long nStartX = LONG_MAX;
    long nStartY = LONG_MAX;

    // Grenzen ausrechnen

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
        nStartX = -nStartX;     // positions are negative, swap start/end so the same comparisons work
        nEndX   = -nEndX;
        ::std::swap( nStartX, nEndX );
    }

    const SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page nicht gefunden");
    if (pPage)
    {
        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
                            //! Flags (ausgeblendet?) testen

            Rectangle aObjRect = pObject->GetCurrentBoundRect();
            sal_Bool bFit = sal_True;
            if ( !bSetHor && ( aObjRect.Right() < nStartX || aObjRect.Left() > nEndX ) )
                bFit = false;
            if ( !bSetVer && ( aObjRect.Bottom() < nStartY || aObjRect.Top() > nEndY ) )
                bFit = false;
            // #i104716# don't include hidden note objects
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
                bAny = sal_True;
            }

            pObject = aIter.Next();
        }
    }

    if ( bNegativePage )
    {
        nStartX = -nStartX;     // reverse transformation, so the same cell address calculation works
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
            for (i=0; i<=MAXCOL && nWidth<=nEndX; i++)          //! bei Start anfangen
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

void ScDrawLayer::BeginCalcUndo()
{
    DELETEZ(pUndoGroup);
    bRecording = sal_True;
}

SdrUndoGroup* ScDrawLayer::GetCalcUndo()
{
    SdrUndoGroup* pRet = pUndoGroup;
    pUndoGroup = NULL;
    bRecording = false;
    return pRet;
}

void ScDrawLayer::MoveArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                            SCsCOL nDx,SCsROW nDy, sal_Bool bInsDel, bool bUpdateNoteCaptionPos )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::MoveArea without document" );
    if ( !pDoc )
        return;

    if (!bAdjustEnabled)
        return;

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page not found");
    if (!pPage)
        return;

    // for an empty page, there's no need to calculate the row heights
    if (!pPage->GetObjCount())
        return;

    sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );

    Rectangle aRect = pDoc->GetMMRect( nCol1, nRow1, nCol2, nRow2, nTab );
    lcl_ReverseTwipsToMM( aRect );
    //! use twips directly?

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

    Point aTopLeft = aRect.TopLeft();       // Anfang beim Verkleinern
    if (bInsDel)
    {
        if ( aMove.X() != 0 && nDx < 0 )    // nDx counts cells, sign is independent of RTL
            aTopLeft.X() += aMove.X();
        if ( aMove.Y() < 0 )
            aTopLeft.Y() += aMove.Y();
    }

        //
        //      Detektiv-Pfeile: Zellpositionen anpassen
        //

    MoveCells( nTab, nCol1,nRow1, nCol2,nRow2, nDx,nDy, bUpdateNoteCaptionPos );
}

sal_Bool ScDrawLayer::HasObjectsInRows( SCTAB nTab, SCROW nStartRow, SCROW nEndRow )
{
    OSL_ENSURE( pDoc, "ScDrawLayer::HasObjectsInRows without document" );
    if ( !pDoc )
        return false;

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    OSL_ENSURE(pPage,"Page not found");
    if (!pPage)
        return sal_False;

    // for an empty page, there's no need to calculate the row heights
    if (!pPage->GetObjCount())
        return sal_False;

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

    sal_Bool bFound = false;

    Rectangle aObjRect;
    SdrObjListIter aIter( *pPage );
    SdrObject* pObject = aIter.Next();
    while ( pObject && !bFound )
    {
        aObjRect = pObject->GetSnapRect();  //! GetLogicRect ?
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
            // do not delete note caption, they are always handled by the cell note
            // TODO: detective objects are still deleted, is this desired?
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
                //  Rechteck um die ganze Selektion
                Rectangle aMarkBound = pDoc->GetMMRect(
                            aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                            aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(), nTab );

                SdrObject** ppObj = new SdrObject*[nObjCount];

                SdrObjListIter aIter( *pPage, IM_FLAT );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    // do not delete note caption, they are always handled by the cell note
                    // TODO: detective objects are still deleted, is this desired?
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

                //  Objekte loeschen (rueckwaerts)

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
    //  copy everything in the specified range into the same page (sheet) in the clipboard doc

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
            // do not copy internal objects (detective) and note captions
            if ( rRange.IsInside( aObjRect ) && (pOldObject->GetLayer() != SC_LAYER_INTERN) && !IsNoteCaption( pOldObject ) )
            {
                if ( !pDestModel )
                {
                    pDestModel = pClipDoc->GetDrawLayer();      // does the document already have a drawing layer?
                    if ( !pDestModel )
                    {
                        //  allocate drawing layer in clipboard document only if there are objects to copy

                        pClipDoc->InitDrawLayer();                  //! create contiguous pages
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
                    if(!xOldChart.is())//#i110034# do not move charts as they loose all their data references otherwise
                        pNewObject->NbcMove(Size(0,0));
                    pDestPage->InsertObject( pNewObject );

                    //  no undo needed in clipboard document
                    //  charts are not updated
                }
            }

            pOldObject = aIter.Next();
        }
    }
}

static sal_Bool lcl_IsAllInRange( const ::std::vector< ScRangeList >& rRangesVector, const ScRange& rClipRange )
{
    //  check if every range of rRangesVector is completely in rClipRange

    ::std::vector< ScRangeList >::const_iterator aIt = rRangesVector.begin();
    for( ;aIt!=rRangesVector.end(); ++aIt )
    {
        const ScRangeList& rRanges = *aIt;
        for ( size_t i = 0, nCount = rRanges.size(); i < nCount; i++ )
        {
            ScRange aRange = *rRanges[ i ];
            if ( !rClipRange.In( aRange ) )
            {
                return false;   // at least one range is not valid
            }
        }
    }

    return sal_True;            // everything is fine
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

    if (bDrawIsInUndo)      //! can this happen?
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
    //  a clipboard document and its source share the same document item pool,
    //  so the pointers can be compared to see if this is copy&paste within
    //  the same document
    sal_Bool bSameDoc = pDoc && pClipDoc && pDoc->GetPool() == pClipDoc->GetPool();
    sal_Bool bDestClip = pDoc && pDoc->IsClipboard();

    //#i110034# charts need correct sheet names for xml range conversion during load
    //so the target sheet name is temporarily renamed (if we have any SdrObjects)
    rtl::OUString aDestTabName;
    sal_Bool bRestoreDestTabName = false;
    if( pOldObject && !bSameDoc && !bDestClip )
    {
        if( pDoc && pClipDoc )
        {
            rtl::OUString aSourceTabName;
            if( pClipDoc->GetName( nSourceTab, aSourceTabName )
                && pDoc->GetName( nDestTab, aDestTabName ) )
            {
                if( !aSourceTabName.equals(aDestTabName) &&
                    pDoc->ValidNewTabName(aSourceTabName) )
                {
                    bRestoreDestTabName = pDoc->RenameTab( nDestTab, aSourceTabName ); //sal_Bool bUpdateRef = sal_True, sal_Bool bExternalDocument = sal_False
                }
            }
        }
    }

    // first mirror, then move
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
    // sizes can differ by 1 from twips->1/100mm conversion for equal cell sizes,
    // don't resize to empty size when pasting into hidden columns or rows
    if ( Abs(nWidthDiff) > 1 && nDestWidth > 1 && nSourceWidth > 1 )
    {
        aHorFract = Fraction( nDestWidth, nSourceWidth );
        bResize = sal_True;
    }
    if ( Abs(nHeightDiff) > 1 && nDestHeight > 1 && nSourceHeight > 1 )
    {
        aVerFract = Fraction( nDestHeight, nSourceHeight );
        bResize = sal_True;
    }
    Point aRefPos = rDestRange.TopLeft();       // for resizing (after moving)

    while (pOldObject)
    {
        Rectangle aObjRect = pOldObject->GetCurrentBoundRect();
        // do not copy internal objects (detective) and note captions
        if ( rSourceRange.IsInside( aObjRect ) && (pOldObject->GetLayer() != SC_LAYER_INTERN) && !IsNoteCaption( pOldObject ) )
        {
            SdrObject* pNewObject = pOldObject->Clone();
            pNewObject->SetModel(this);
            pNewObject->SetPage(pDestPage);

            if ( bMirrorObj )
                MirrorRTL( pNewObject );        // first mirror, then move

            pNewObject->NbcMove( aMove );
            if ( bResize )
                pNewObject->NbcResize( aRefPos, aHorFract, aVerFract );

            pDestPage->InsertObject( pNewObject );
            if (bRecording)
                AddCalcUndo( new SdrUndoInsertObj( *pNewObject ) );

            //#i110034# handle chart data references (after InsertObject)

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
                        // TODO: handle error?
                    }
                }

                if ( xIPObj.is() && SotExchange::IsChart( aObjectClassName ) )
                {
                    uno::Reference< chart2::XChartDocument > xNewChart( ScChartHelper::GetChartFromSdrObject( pNewObject ) );
                    if( xNewChart.is() && !xNewChart->hasInternalDataProvider() )
                    {
                        String aChartName = ((SdrOle2Obj*)pNewObject)->GetPersistName();
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
                                pClipDoc->GetClipArea( nClipEndX, nClipEndY, sal_True );
                                nClipEndX = nClipEndX + nClipStartX;
                                nClipEndY += nClipStartY;   // GetClipArea returns the difference

                                SCTAB nClipTab = bRestoreDestTabName ? nDestTab : nSourceTab;
                                aClipRange = ScRange( nClipStartX, nClipStartY, nClipTab,
                                                        nClipEndX, nClipEndY, nClipTab );

                                bInSourceRange = lcl_IsAllInRange( aRangesVector, aClipRange );
                            }

                            // always lose references when pasting into a clipboard document (transpose)
                            if ( ( bInSourceRange || bSameDoc ) && !bDestClip )
                            {
                                if ( bInSourceRange )
                                {
                                    if ( rDestPos != aClipRange.aStart )
                                    {
                                        //  update the data ranges to the new (copied) position
                                        if ( lcl_MoveRanges( aRangesVector, aClipRange, rDestPos ) )
                                            pDoc->SetChartRanges( aChartName, aRangesVector );
                                    }
                                }
                                else
                                {
                                    //  leave the ranges unchanged
                                }
                            }
                            else
                            {
                                //  pasting into a new document without the complete source data
                                //  -> break connection to source data and switch to own data

                                uno::Reference< chart::XChartDocument > xOldChartDoc( ScChartHelper::GetChartFromSdrObject( pOldObject ), uno::UNO_QUERY );
                                uno::Reference< chart::XChartDocument > xNewChartDoc( xNewChart, uno::UNO_QUERY );
                                if( xOldChartDoc.is() && xNewChartDoc.is() )
                                    xNewChartDoc->attachData( xOldChartDoc->getData() );

                                //  (see ScDocument::UpdateChartListenerCollection, PastingDrawFromOtherDoc)
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

    //  don't mirror OLE or graphics, otherwise ask the object
    //  if it can be mirrored
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
        //  Move instead of mirroring:
        //  New start position is negative of old end position
        //  -> move by sum of start and end position
        Rectangle aObjRect = pObj->GetLogicRect();
        Size aMoveSize( -(aObjRect.Left() + aObjRect.Right()), 0 );
        if (bRecording)
            AddCalcUndo( new SdrUndoMoveObj( *pObj, aMoveSize ) );
        pObj->Move( aMoveSize );
    }
}

void ScDrawLayer::MirrorRectRTL( Rectangle& rRect )
{
    //  mirror and swap left/right
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
        // find top left position of passed cell address
        Point aTopLeft;
        for( SCCOL nCol = 0; nCol < rPos.Col(); ++nCol )
            aTopLeft.X() += rDoc.GetColWidth( nCol, rPos.Tab() );
        if( rPos.Row() > 0 )
            aTopLeft.Y() += rDoc.GetRowHeight( 0, rPos.Row() - 1, rPos.Tab() );

        // find bottom-right position of passed cell address
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

        // twips -> 1/100 mm
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

String ScDrawLayer::GetVisibleName( SdrObject* pObj )
{
    String aName = pObj->GetName();
    if ( pObj->GetObjIdentifier() == OBJ_OLE2 )
    {
        //  For OLE, the user defined name (GetName) is used
        //  if it's not empty (accepting possibly duplicate names),
        //  otherwise the persist name is used so every object appears
        //  in the Navigator at all.

        if ( !aName.Len() )
            aName = static_cast<SdrOle2Obj*>(pObj)->GetPersistName();
    }
    return aName;
}

inline sal_Bool IsNamedObject( SdrObject* pObj, const String& rName )
{
    //  sal_True if rName is the object's Name or PersistName
    //  (used to find a named object)

    return ( pObj->GetName().equals(rName) ||
            ( pObj->GetObjIdentifier() == OBJ_OLE2 &&
              static_cast<SdrOle2Obj*>(pObj)->GetPersistName() == rName ) );
}

SdrObject* ScDrawLayer::GetNamedObject( const String& rName, sal_uInt16 nId, SCTAB& rFoundTab ) const
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

String ScDrawLayer::GetNewGraphicName( long* pnCounter ) const
{
    String aBase = ScGlobal::GetRscString(STR_GRAPHICNAME);
    aBase += ' ';

    sal_Bool bThere = sal_True;
    String aGraphicName;
    SCTAB nDummy;
    long nId = pnCounter ? *pnCounter : 0;
    while (bThere)
    {
        ++nId;
        aGraphicName = aBase;
        aGraphicName += String::CreateFromInt32( nId );
        bThere = ( GetNamedObject( aGraphicName, 0, nDummy ) != NULL );
    }

    if ( pnCounter )
        *pnCounter = nId;

    return aGraphicName;
}

void ScDrawLayer::EnsureGraphicNames()
{
    //  make sure all graphic objects have names (after Excel import etc.)

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
    Rectangle aObjRect(rObj.GetLogicRect());
    ScRange aRange = rDoc.GetRange( nTab, aObjRect );

    Rectangle aCellRect;

    ScDrawObjData aAnchor;
    aAnchor.maStart = aRange.aStart;
    aCellRect = rDoc.GetMMRect( aRange.aStart.Col(), aRange.aStart.Row(),
      aRange.aStart.Col(), aRange.aStart.Row(), aRange.aStart.Tab() );
    aAnchor.maStartOffset.Y() = aObjRect.Top()-aCellRect.Top();
    if (!rDoc.IsNegativePage(nTab))
        aAnchor.maStartOffset.X() = aObjRect.Left()-aCellRect.Left();
    else
        aAnchor.maStartOffset.X() = aCellRect.Right()-aObjRect.Right();

    aAnchor.maEnd = aRange.aEnd;
    aCellRect = rDoc.GetMMRect( aRange.aEnd.Col(), aRange.aEnd.Row(),
      aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aEnd.Tab() );
    aAnchor.maEndOffset.Y() = aObjRect.Bottom()-aCellRect.Top();
    if (!rDoc.IsNegativePage(nTab))
        aAnchor.maEndOffset.X() = aObjRect.Right()-aCellRect.Left();
    else
        aAnchor.maEndOffset.X() = aCellRect.Right()-aObjRect.Left();

    SetCellAnchored( rObj, aAnchor );
}

void ScDrawLayer::UpdateCellAnchorFromPositionEnd( SdrObject &rObj, const ScDocument &rDoc, SCTAB nTab )
{
    Rectangle aObjRect(rObj.GetLogicRect());
    ScRange aRange = rDoc.GetRange( nTab, aObjRect );

    ScDrawObjData* pAnchor = GetObjData( &rObj, true );
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

void ScDrawLayer::SetPageAnchored( SdrObject &rObj )
{
    DeleteFirstUserDataOfType(&rObj, SC_UD_OBJDATA);
}

ScAnchorType ScDrawLayer::GetAnchorType( const SdrObject &rObj )
{
    //If this object has a cell anchor associated with it
    //then its cell-anchored, otherwise its page-anchored
    return ScDrawLayer::GetObjData(const_cast<SdrObject*>(&rObj)) ? SCA_CELL : SCA_PAGE;
}

ScDrawObjData* ScDrawLayer::GetObjData( SdrObject* pObj, sal_Bool bCreate )
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

        if ( pObj->ISA( SdrGrafObj )  ) // einfaches Grafik-Objekt
        {
            const SdrGrafObj*   pGrafObj = (const SdrGrafObj*) pObj;
            const GeoStat&      rGeo = pGrafObj->GetGeoStat();
            const Graphic&      rGraphic = pGrafObj->GetGraphic();

            // Drehung rueckgaengig
            if ( rGeo.nDrehWink )
                RotatePoint( aRelPoint, aLogRect.TopLeft(), -rGeo.nSin, rGeo.nCos );

            // Spiegelung rueckgaengig
            if ( ( (const SdrGrafObjGeoData*) pGrafObj->GetGeoData() )->bMirrored )
                aRelPoint.X() = aLogRect.Right() + aLogRect.Left() - aRelPoint.X();

            // ggf. Unshear:
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
        else if ( pObj->ISA( SdrOle2Obj ) ) // OLE-Objekt
        {
            // TODO/LEAN: working with visual area needs running state
            aGraphSize = ((const SdrOle2Obj*)pObj)->GetOrigObjSize();
            bObjSupported = true;
        }

        // hat alles geklappt, dann HitTest ausfuehren
        if ( bObjSupported )
        {
            // relativen Mauspunkt berechnen
            aRelPoint -= aLogRect.TopLeft();
            pIMapObj = rImageMap.GetHitIMapObject( aGraphSize, aLogRect.GetSize(), aRelPoint );
        }
    }

    return pIMapObj;
}

ScMacroInfo* ScDrawLayer::GetMacroInfo( SdrObject* pObj, sal_Bool bCreate )
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

void ScDrawLayer::SetGlobalDrawPersist(SfxObjectShell* pPersist)
{
    OSL_ENSURE(!pGlobalDrawPersist,"SetGlobalDrawPersist mehrfach");
    pGlobalDrawPersist = pPersist;
}

void ScDrawLayer::SetChanged( sal_Bool bFlg /* = sal_True */ )
{
    if ( bFlg && pDoc )
        pDoc->SetChartListenerCollectionNeedsUpdate( sal_True );
    FmFormModel::SetChanged( bFlg );
}

SdrLayerID ScDrawLayer::GetControlExportLayerId( const SdrObject & ) const
{
    //  Layer fuer Export von Form-Controls in Versionen vor 5.0 - immer SC_LAYER_FRONT
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
