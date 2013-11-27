/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
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

// INCLUDE ---------------------------------------------------------------

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
#include <svx/svdlegacy.hxx>

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

#define DET_ARROW_OFFSET    1000

//  Abstand zur naechsten Zelle beim Loeschen (bShrink), damit der Anker
//  immer an der richtigen Zelle angezeigt wird
//#define SHRINK_DIST       3
//  und noch etwas mehr, damit das Objekt auch sichtbar in der Zelle liegt
#define SHRINK_DIST     25

#define SHRINK_DIST_TWIPS   15

using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

static ScDrawObjFactory* pFac = NULL;
static E3dObjFactory* pF3d = NULL;
static sal_uInt16 nInst = 0;

SfxObjectShell* ScDrawLayer::pGlobalDrawPersist = NULL;
//REMOVE    SvPersist* ScDrawLayer::pGlobalDrawPersist = NULL;

sal_Bool bDrawIsInUndo = sal_False;         //! Member

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

__EXPORT ScUndoObjData::~ScUndoObjData()
{
}

void ScUndoObjData::Undo()
{
    ScDrawObjData* pData = ScDrawLayer::GetObjData( mrSdrObject );
    DBG_ASSERT(pData,"ScUndoObjData: Daten nicht da");
    if (pData)
    {
        pData->maStart = aOldStt;
        pData->maEnd = aOldEnd;
    }
}

void __EXPORT ScUndoObjData::Redo()
{
    ScDrawObjData* pData = ScDrawLayer::GetObjData( mrSdrObject );
    DBG_ASSERT(pData,"ScUndoObjData: Daten nicht da");
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

__EXPORT ScTabDeletedHint::~ScTabDeletedHint()
{
}

ScTabSizeChangedHint::ScTabSizeChangedHint( SCTAB nTabNo ) :
    nTab( nTabNo )
{
}

__EXPORT ScTabSizeChangedHint::~ScTabSizeChangedHint()
{
}

// -----------------------------------------------------------------------

#define MAXMM   10000000

inline void TwipsToMM( long& nVal )
{
    nVal = (long) ( nVal * HMM_PER_TWIPS );
}

inline void ReverseTwipsToMM( long& nVal )
{
    //  reverse the effect of TwipsToMM - round up here (add 1)

    nVal = ((long) ( nVal / HMM_PER_TWIPS )) + 1;
}

void lcl_TwipsToMM( Point& rPoint )
{
    TwipsToMM( rPoint.X() );
    TwipsToMM( rPoint.Y() );
}

void lcl_ReverseTwipsToMM( Point& rPoint )
{
    ReverseTwipsToMM( rPoint.X() );
    ReverseTwipsToMM( rPoint.Y() );
}

void lcl_ReverseTwipsToMM( Rectangle& rRect )
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
                     ( pDocument ? pDocument->GetDocumentShell() : NULL )),
    aName( rName ),
    pDoc( pDocument ),
    pUndoGroup( NULL ),
    bRecording( sal_False ),
    bAdjustEnabled( sal_True ),
    bHyphenatorSet( sal_False ),
        mbUndoAllowed( sal_True )
{
    pGlobalDrawPersist = NULL;          // nur einmal benutzen

    SfxObjectShell* pObjSh = pDocument ? pDocument->GetDocumentShell() : NULL;
    if ( pObjSh )
    {
        SetObjectShell( pObjSh );

        // set color table
        const SvxColorTableItem* pColItem = static_cast< const SvxColorTableItem* >(pObjSh->GetItem( SID_COLOR_TABLE ));
        XColorListSharedPtr aXCol = pColItem ? pColItem->GetColorTable() : XColorList::GetStdColorList();
        SetColorTableAtSdrModel( aXCol );
    }
    else
        SetColorTableAtSdrModel( XColorList::GetStdColorList() );

    SetSwapGraphics(sal_True);
//  SetSwapAsynchron(sal_True);     // an der View

    SetExchangeObjectUnit(MAP_100TH_MM);
    SfxItemPool& rPool = GetItemPool();
    rPool.SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    SvxFrameDirectionItem aModeItem( FRMDIR_ENVIRONMENT, EE_PARA_WRITINGDIR );
    rPool.SetPoolDefaultItem( aModeItem );

    // #i33700#
    // Set shadow distance defaults as PoolDefaultItems. Details see bug.
    rPool.SetPoolDefaultItem(SdrMetricItem(SDRATTR_SHADOWXDIST, 300));
    rPool.SetPoolDefaultItem(SdrMetricItem(SDRATTR_SHADOWYDIST, 300));

    // #111216# default for script spacing depends on locale, see SdDrawDocument ctor in sd
    LanguageType eOfficeLanguage = Application::GetSettings().GetLanguage();
    if ( eOfficeLanguage == LANGUAGE_KOREAN || eOfficeLanguage == LANGUAGE_KOREAN_JOHAB ||
         eOfficeLanguage == LANGUAGE_JAPANESE )
    {
        // secondary is edit engine pool
        rPool.GetSecondaryPool()->SetPoolDefaultItem( SvxScriptSpaceItem( sal_False, EE_PARA_ASIANCJKSPACING ) );
    }

    rPool.FreezeIdRanges();                         // the pool is also used directly

    SdrLayerAdmin& rAdmin = GetModelLayerAdmin();
    rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("vorne")),    SC_LAYER_FRONT);
    rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("hinten")),   SC_LAYER_BACK);
    rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("intern")),   SC_LAYER_INTERN);
    rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Controls")), SC_LAYER_CONTROLS);
    rAdmin.NewLayer(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("hidden")),   SC_LAYER_HIDDEN);
    // "Controls" is new - must also be created when loading

    //  Link fuer URL-Fields setzen
    ScModule* pScMod = SC_MOD();
    Outliner& rOutliner = GetDrawOutliner();
    rOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );

    // #95129# SJ: set FontHeight pool defaults without changing static SdrEngineDefaults
    SfxItemPool* pOutlinerPool = rOutliner.GetEditTextObjectPool();
    if ( pOutlinerPool )
         mpItemPool->SetPoolDefaultItem(SvxFontHeightItem( 423, 100, EE_CHAR_FONTHEIGHT ));           // 12Pt

    // initial undo mode as in Calc document
    if( pDoc )
        EnableUndo( pDoc->IsUndoEnabled() );

    //  URL-Buttons haben keinen Handler mehr, machen alles selber

    if( !nInst++ )
    {
        pFac = new ScDrawObjFactory;
        pF3d = new E3dObjFactory;
    }
}

__EXPORT ScDrawLayer::~ScDrawLayer()
{
    Broadcast(SdrBaseHint(HINT_MODELCLEARED));

    // #116168#
    //Clear();
    ClearModel(true);

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
        bHyphenatorSet = sal_True;
    }
}

SdrPage* __EXPORT ScDrawLayer::AllocPage(bool bMasterPage)
{
    //  don't create basic until it is needed
    StarBASIC* pBasic = NULL;
    ScDrawPage* pPage = new ScDrawPage( *this, pBasic, sal::static_int_cast<sal_Bool>(bMasterPage) );
    return pPage;
}

sal_Bool ScDrawLayer::HasObjects() const
{
    sal_Bool bFound = sal_False;

    const sal_uInt32 nCount(GetPageCount());
    for (sal_uInt32 i=0; i<nCount && !bFound; i++)
        if (GetPage(i)->GetObjCount())
            bFound = sal_True;

    return bFound;
}

void ScDrawLayer::UpdateBasic()
{
    //  don't create basic until it is needed
    //! remove this method?
}

SdrModel* __EXPORT ScDrawLayer::AllocModel() const
{
    //  #103849# Allocated model (for clipboard etc) must not have a pointer
    //  to the original model's document, pass NULL as document:

    return new ScDrawLayer( NULL, aName );
}

Window* __EXPORT ScDrawLayer::GetCurDocViewWin()
{
    DBG_ASSERT( pDoc, "ScDrawLayer::GetCurDocViewWin without document" );
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
        return sal_False;   // not inserted

    ScDrawPage* pPage = (ScDrawPage*)AllocPage( sal_False );
    InsertPage(pPage, static_cast<sal_uInt16>(nTab));
    if (bRecording)
            AddCalcUndo< SdrUndoNewPage >(*pPage);

    return sal_True;        // inserted
}

void ScDrawLayer::ScRemovePage( SCTAB nTab )
{
    if (bDrawIsInUndo)
        return;

    Broadcast( ScTabDeletedHint( nTab ) );
    if (bRecording)
    {
        SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
        AddCalcUndo< SdrUndoDelPage >(*pPage);      // Undo-Action wird Owner der Page
        RemovePage( static_cast<sal_uInt16>(nTab) );                            // nur austragen, nicht loeschen
    }
    else
        DeletePage( static_cast<sal_uInt16>(nTab) );                            // einfach weg damit
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
}

void ScDrawLayer::ScCopyPage( sal_uInt16 nOldPos, sal_uInt16 nNewPos, sal_Bool bAlloc )
{
    //! remove argument bAlloc (always sal_False)

    if (bDrawIsInUndo)
        return;

    SdrPage* pOldPage = GetPage(nOldPos);
    SdrPage* pNewPage = bAlloc ? AllocPage(sal_False) : GetPage(nNewPos);

    // kopieren

    if (pOldPage && pNewPage)
    {
        SdrObjListIter aIter( *pOldPage, IM_FLAT );
        SdrObject* pOldObject = aIter.Next();
        while (pOldObject)
        {
            // #i112034# do not copy internal objects (detective) and note captions
            if ( pOldObject->GetLayer() != SC_LAYER_INTERN && !IsNoteCaption( *pOldObject ) )
            {
                // #116235#
                SdrObject* pNewObject = pOldObject->CloneSdrObject();

                pNewPage->InsertObjectToSdrObjList(*pNewObject);
                if (bRecording)
                    AddCalcUndo< SdrUndoInsertObj >( *pNewObject );
            }

            pOldObject = aIter.Next();
        }
    }

    if (bAlloc)
        InsertPage(pNewPage, nNewPos);
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
    DBG_ASSERT(pPage,"Page nicht gefunden");
    if (!pPage)
        return;

    sal_Bool bNegativePage = pDoc && pDoc->IsNegativePage( nTab );

    sal_uInt32 nCount = pPage->GetObjCount();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        SdrObject* pObj = pPage->GetObj( i );
        ScDrawObjData* pData = pObj ? GetObjDataTab( *pObj, nTab ) : 0;
        if( pData )
        {
            const ScAddress aOldStt = pData->maStart;
            const ScAddress aOldEnd = pData->maEnd;
            sal_Bool bChange = sal_False;
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
                if ( dynamic_cast< SdrRectObj* >(pObj) && pData->maStart.IsValid() && pData->maEnd.IsValid() )
                    pData->maStart.PutInOrder( pData->maEnd );
                AddCalcUndo< ScUndoObjData >( pObj, aOldStt, aOldEnd, pData->maStart, pData->maEnd );
                RecalcPos( pObj, *pData, bNegativePage, bUpdateNoteCaptionPos );
            }
        }
    }
}

void ScDrawLayer::SetPageSize( sal_uInt32 nPageNo, const basegfx::B2DVector& rSize, bool bUpdateNoteCaptionPos )
{
    SdrPage* pPage = GetPage(nPageNo);
    if (pPage)
    {
        if(!rSize.equal(pPage->GetPageScale()))
        {
            pPage->SetPageScale( rSize );
            Broadcast( ScTabSizeChangedHint( static_cast<SCTAB>(nPageNo) ) );   // SetWorkArea() an den Views
        }

        // Detektivlinien umsetzen (an neue Hoehen/Breiten anpassen)
        //  auch wenn Groesse gleich geblieben ist
        //  (einzelne Zeilen/Spalten koennen geaendert sein)

        sal_Bool bNegativePage = pDoc && pDoc->IsNegativePage( static_cast<SCTAB>(nPageNo) );

        sal_uInt32 nCount = pPage->GetObjCount();
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            SdrObject* pObj = pPage->GetObj( i );
            ScDrawObjData* pData = pObj ? GetObjDataTab( *pObj, static_cast<SCTAB>(nPageNo) ) : 0;
            if( pData )
                RecalcPos( pObj, *pData, bNegativePage, bUpdateNoteCaptionPos );
        }
    }
}

void ScDrawLayer::RecalcPos( SdrObject* pObj, const ScDrawObjData& rData, bool bNegativePage, bool bUpdateNoteCaptionPos )
{
    DBG_ASSERT( pDoc, "ScDrawLayer::RecalcPos - missing document" );
    if( !pDoc )
        return;

    if( rData.mbNote )
    {
        DBG_ASSERT( rData.maStart.IsValid(), "ScDrawLayer::RecalcPos - invalid position for cell note" );
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

    // validation circle
    bool bCircle = dynamic_cast< SdrCircObj* >(pObj);
    // detective arrow
    bool bArrow = pObj->IsPolygonObject() && (2 == pObj->GetObjectPointCount());

    if( bCircle )
    {
        //  Berechnung und Werte wie in detfunc.cxx
        const basegfx::B2DPoint aPos(pDoc->GetColOffset(nCol1, nTab1) * HMM_PER_TWIPS, pDoc->GetRowOffset(nRow1, nTab1) * HMM_PER_TWIPS);
        const basegfx::B2DVector aSize(pDoc->GetColWidth(nCol1, nTab1) * HMM_PER_TWIPS, pDoc->GetRowHeight(nRow1, nTab1) * HMM_PER_TWIPS);
        basegfx::B2DRange aRange(aPos.getX() - 250.0, aPos.getY() - 70.0, aPos.getX() + aSize.getX() + 250.0, aPos.getY() + aSize.getY() + 70.0);

        if ( bNegativePage )
        {
            aRange = basegfx::B2DRange(-aRange.getMaxX(), aRange.getMinY(), -aRange.getMinX(), aRange.getMaxY());
        }

        if(!sdr::legacy::GetLogicRange(*pObj).equal(aRange))
        {
            if (bRecording)
                        AddCalcUndo<SdrUndoGeoObj>( *pObj );
            sdr::legacy::SetLogicRange(*pObj, aRange);
        }
    }
    else if( bArrow )
    {
        //! nicht mehrere Undos fuer ein Objekt erzeugen (hinteres kann dann weggelassen werden)

        SCCOL nLastCol;
        SCROW nLastRow;
        if( bValid1 )
        {
            basegfx::B2DPoint aPos(pDoc->GetColOffset(nCol1, nTab1), pDoc->GetRowOffset(nRow1, nTab1));

            if (!pDoc->ColHidden(nCol1, nTab1, nLastCol))
            {
                aPos.setX(aPos.getX() + pDoc->GetColWidth(nCol1, nTab1) / 4.0);
            }

            if (!pDoc->RowHidden(nRow1, nTab1, nLastRow))
            {
                aPos.setY(aPos.getY() + pDoc->GetRowHeight(nRow1, nTab1) / 2.0);
            }

            aPos *= HMM_PER_TWIPS;

            basegfx::B2DPoint aStartPos(aPos);

            if ( bNegativePage )
            {
                aStartPos.setX(-aStartPos.getX());      // don't modify aPos - used below
            }

            if(!pObj->GetObjectPoint(0).equal(aStartPos))
            {
                if (bRecording)
                            AddCalcUndo< SdrUndoGeoObj> ( *pObj );

                pObj->SetObjectPoint( aStartPos, 0 );
            }

            if( !bValid2 )
            {
                basegfx::B2DPoint aEndPos(aPos.getX() + DET_ARROW_OFFSET, aPos.getY() - DET_ARROW_OFFSET);

                if(aEndPos.getY() < 0.0)
                {
                    aEndPos.setY(aEndPos.getY() + (2 * DET_ARROW_OFFSET));
                }

                if ( bNegativePage )
                {
                    aEndPos.setX(-aEndPos.getX());
                }

                if(!pObj->GetObjectPoint(1).equal(aEndPos))
                {
                    if (bRecording)
                                AddCalcUndo< SdrUndoGeoObj >( *pObj );

                    pObj->SetObjectPoint( aEndPos, 1 );
                }
            }
        }
        if( bValid2 )
        {
            basegfx::B2DPoint aPos(pDoc->GetColOffset(nCol2, nTab2), pDoc->GetRowOffset(nRow2, nTab2));

            if (!pDoc->ColHidden(nCol2, nTab2, nLastCol))
            {
                aPos.setX(aPos.getX() + pDoc->GetColWidth(nCol2, nTab2) / 4.0);
            }

            if (!pDoc->RowHidden(nRow2, nTab2, nLastRow))
            {
                aPos.setY(aPos.getY() + pDoc->GetRowHeight(nRow2, nTab2) / 2);
            }

            aPos *= HMM_PER_TWIPS;

            basegfx::B2DPoint aEndPos(aPos);

            if ( bNegativePage )
            {
                aEndPos.setX(-aEndPos.getX());          // don't modify aPos - used below
            }

            if(!pObj->GetObjectPoint(1).equal(aEndPos))
            {
                if (bRecording)
                            AddCalcUndo< SdrUndoGeoObj> ( *pObj  );

                pObj->SetObjectPoint( aEndPos, 1 );
            }

            if( !bValid1 )
            {
                basegfx::B2DPoint aStartPos(aPos.getX() - DET_ARROW_OFFSET, aPos.getY() - DET_ARROW_OFFSET);

                if(aStartPos.getX() < 0.0)
                {
                    aStartPos.setX(aStartPos.getX() + (2 * DET_ARROW_OFFSET));
                }

                if(aStartPos.getY() < 0.0)
                {
                    aStartPos.setY(aStartPos.getY() + (2 * DET_ARROW_OFFSET));
                }

                if ( bNegativePage )
                {
                    aStartPos.setX(-aStartPos.getX());
                }

                if(!pObj->GetObjectPoint(0).equal(aStartPos))
                {
                    if (bRecording)
                                    AddCalcUndo< SdrUndoGeoObj >( *pObj );

                    pObj->SetObjectPoint( aStartPos, 0 );
                }
            }
        }
    }
    else                                // Referenz-Rahmen
    {
        DBG_ASSERT( bValid1, "ScDrawLayer::RecalcPos - invalid start position" );
        basegfx::B2DPoint aPos(pDoc->GetColOffset(nCol1, nTab1) * HMM_PER_TWIPS, pDoc->GetRowOffset(nRow1, nTab1) * HMM_PER_TWIPS);

        if( bValid2 )
        {
            const basegfx::B2DPoint aEnd(pDoc->GetColOffset(nCol2 + 1, nTab2) + HMM_PER_TWIPS, pDoc->GetRowOffset(nRow2 + 1, nTab2) * HMM_PER_TWIPS);
            basegfx::B2DRange aNew(aPos, aPos + aEnd);

            if ( bNegativePage )
            {
                aNew = basegfx::B2DRange(-aNew.getMaxX(), aNew.getMinY(), -aNew.getMinX(), aNew.getMaxY());
            }

            if(!sdr::legacy::GetLogicRange(*pObj).equal(aNew))
            {
                if (bRecording)
                            AddCalcUndo< SdrUndoGeoObj >( *pObj );

                sdr::legacy::SetLogicRange(*pObj, aNew);
            }
        }
        else
        {
            if ( bNegativePage )
            {
                aPos.setX(-aPos.getX());
            }

            // #i108739# SC never used anchor, thus i simplified this to just TopLeft
            const basegfx::B2DPoint aRelativePos(pObj->getSdrObjectTranslate());

            if(!aRelativePos.equal(aPos))
            {
                if (bRecording)
                            AddCalcUndo< SdrUndoGeoObj >( *pObj );

                sdr::legacy::transformSdrObject(*pObj, basegfx::tools::createTranslateB2DHomMatrix(aPos - aRelativePos));
            }
        }
    }
}

sal_Bool ScDrawLayer::GetPrintArea( ScRange& rRange, sal_Bool bSetHor, sal_Bool bSetVer ) const
{
    DBG_ASSERT( pDoc, "ScDrawLayer::GetPrintArea without document" );
    if ( !pDoc )
        return sal_False;

    SCTAB nTab = rRange.aStart.Tab();
    DBG_ASSERT( rRange.aEnd.Tab() == nTab, "GetPrintArea: Tab unterschiedlich" );

    sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );

    sal_Bool bAny = sal_False;
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
        nStartX = (long)(nStartX * HMM_PER_TWIPS);
        nEndX   = (long)(nEndX   * HMM_PER_TWIPS);
    }
    if (!bSetVer)
    {
        nStartY = pDoc->GetRowHeight( 0, rRange.aStart.Row()-1, nTab);
        nEndY = nStartY + pDoc->GetRowHeight( rRange.aStart.Row(),
                rRange.aEnd.Row(), nTab);
        nStartY = (long)(nStartY * HMM_PER_TWIPS);
        nEndY   = (long)(nEndY   * HMM_PER_TWIPS);
    }

    if ( bNegativePage )
    {
        nStartX = -nStartX;     // positions are negative, swap start/end so the same comparisons work
        nEndX   = -nEndX;
        ::std::swap( nStartX, nEndX );
    }

    const SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page nicht gefunden");
    if (pPage)
    {
        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
                            //! Flags (ausgeblendet?) testen

            Rectangle aObjRect = sdr::legacy::GetBoundRect(*pObject);
            sal_Bool bFit = sal_True;
            if ( !bSetHor && ( aObjRect.Right() < nStartX || aObjRect.Left() > nEndX ) )
                bFit = sal_False;
            if ( !bSetVer && ( aObjRect.Bottom() < nStartY || aObjRect.Top() > nEndY ) )
                bFit = sal_False;
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
        DBG_ASSERT( nStartX<=nEndX && nStartY<=nEndY, "Start/End falsch in ScDrawLayer::GetPrintArea" );

        if (bSetHor)
        {
            nStartX = (long) (nStartX / HMM_PER_TWIPS);
            nEndX = (long) (nEndX / HMM_PER_TWIPS);
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
            nStartY = (long) (nStartY / HMM_PER_TWIPS);
            nEndY = (long) (nEndY / HMM_PER_TWIPS);
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
//! DBG_ASSERT( !bRecording, "BeginCalcUndo ohne GetCalcUndo" );
    SetDisableTextEditUsesCommonUndoManager(bDisableTextEditUsesCommonUndoManager);
    DELETEZ(pUndoGroup);
    bRecording = sal_True;
}

SdrUndoGroup* ScDrawLayer::GetCalcUndo()
{
//! DBG_ASSERT( bRecording, "GetCalcUndo ohne BeginCalcUndo" );

    SdrUndoGroup* pRet = pUndoGroup;
    pUndoGroup = NULL;
    bRecording = sal_False;
    SetDisableTextEditUsesCommonUndoManager(false);
    return pRet;
}

//  MoveAreaTwips: all measures are kept in twips
void ScDrawLayer::MoveAreaTwips( SCTAB nTab, const Rectangle& rArea,
        const Point& rMove, const Point& rTopLeft )
{
    if (!rMove.X() && !rMove.Y())
        return;                                     // nix

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page nicht gefunden");
    if (!pPage)
        return;

    sal_Bool bNegativePage = pDoc && pDoc->IsNegativePage( nTab );

    // fuer Shrinking!
    Rectangle aNew( rArea );
    sal_Bool bShrink = sal_False;
    if ( rMove.X() < 0 || rMove.Y() < 0 )       // verkleinern
    {
        if ( rTopLeft != rArea.TopLeft() )      // sind gleich beim Verschieben von Zellen
        {
            bShrink = sal_True;
            aNew.Left() = rTopLeft.X();
            aNew.Top() = rTopLeft.Y();
        }
    }
    SdrObjListIter aIter( *pPage, IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject)
    {
        if( GetAnchor( pObject ) == SCA_CELL )
        {
            if ( GetObjData( *pObject ) )                   // Detektiv-Pfeil ?
            {
                // hier nichts
            }
            else if (dynamic_cast< SdrEdgeObj* >(pObject))          // Verbinder?
            {
                //  hier auch nichts
                //! nicht verbundene Enden wie bei Linien (s.u.) behandeln?
            }
            else if ( pObject->IsPolygonObject() && 2 == pObject->GetObjectPointCount() )
            {
                for (sal_uInt16 i=0; i<2; i++)
                {
                    sal_Bool bMoved = sal_False;
                    const basegfx::B2DPoint aB2DPoint(pObject->GetObjectPoint(i));
                    Point aPoint(basegfx::fround(aB2DPoint.getX()), basegfx::fround(aB2DPoint.getY()));
                    lcl_ReverseTwipsToMM( aPoint );
                    if (rArea.IsInside(aPoint))
                    {
                        aPoint += rMove; bMoved = sal_True;
                    }
                    else if (bShrink && aNew.IsInside(aPoint))
                    {
                        //  Punkt ist in betroffener Zelle - Test auf geloeschten Bereich
                        if ( rMove.X() && aPoint.X() >= rArea.Left() + rMove.X() )
                        {
                            aPoint.X() = rArea.Left() + rMove.X() - SHRINK_DIST_TWIPS;
                            if ( aPoint.X() < 0 ) aPoint.X() = 0;
                            bMoved = sal_True;
                        }
                        if ( rMove.Y() && aPoint.Y() >= rArea.Top() + rMove.Y() )
                        {
                            aPoint.Y() = rArea.Top() + rMove.Y() - SHRINK_DIST_TWIPS;
                            if ( aPoint.Y() < 0 ) aPoint.Y() = 0;
                            bMoved = sal_True;
                        }
                    }
                    if( bMoved )
                    {
                                    AddCalcUndo< SdrUndoGeoObj >( *pObject );
                        lcl_TwipsToMM( aPoint );
                        pObject->SetObjectPoint( basegfx::B2DPoint(aPoint.X(), aPoint.Y()), i );
                    }
                }
            }
            else
            {
                Rectangle aObjRect(sdr::legacy::GetLogicRect(*pObject));
                // aOldMMPos: not converted, millimeters
                Point aOldMMPos = bNegativePage ? aObjRect.TopRight() : aObjRect.TopLeft();
                lcl_ReverseTwipsToMM( aObjRect );
                Point aTopLeft = bNegativePage ? aObjRect.TopRight() : aObjRect.TopLeft();  // logical left
                Size aMoveSize;
                sal_Bool bDoMove = sal_False;
                if (rArea.IsInside(aTopLeft))
                {
                    aMoveSize = Size(rMove.X(),rMove.Y());
                    bDoMove = sal_True;
                }
                else if (bShrink && aNew.IsInside(aTopLeft))
                {
                    //  Position ist in betroffener Zelle - Test auf geloeschten Bereich
                    if ( rMove.X() && aTopLeft.X() >= rArea.Left() + rMove.X() )
                    {
                        aMoveSize.Width() = rArea.Left() + rMove.X() - SHRINK_DIST - aTopLeft.X();
                        bDoMove = sal_True;
                    }
                    if ( rMove.Y() && aTopLeft.Y() >= rArea.Top() + rMove.Y() )
                    {
                        aMoveSize.Height() = rArea.Top() + rMove.Y() - SHRINK_DIST - aTopLeft.Y();
                        bDoMove = sal_True;
                    }
                }
                if ( bDoMove )
                {
                    if ( bNegativePage )
                    {
                        if ( aTopLeft.X() + aMoveSize.Width() > 0 )
                            aMoveSize.Width() = -aTopLeft.X();
                    }
                    else
                    {
                        if ( aTopLeft.X() + aMoveSize.Width() < 0 )
                            aMoveSize.Width() = -aTopLeft.X();
                    }
                    if ( aTopLeft.Y() + aMoveSize.Height() < 0 )
                        aMoveSize.Height() = -aTopLeft.Y();

                    //  get corresponding move size in millimeters:
                    Point aNewPos( aTopLeft.X() + aMoveSize.Width(), aTopLeft.Y() + aMoveSize.Height() );
                    lcl_TwipsToMM( aNewPos );
                    aMoveSize = Size( aNewPos.X() - aOldMMPos.X(), aNewPos.Y() - aOldMMPos.Y() );   // millimeters

                    AddCalcUndo(new SdrUndoGeoObj(*pObject));
                    sdr::legacy::transformSdrObject(*pObject, basegfx::tools::createTranslateB2DHomMatrix(aMoveSize.Width(), aMoveSize.Height()));
                }
                else if ( rArea.IsInside( bNegativePage ? aObjRect.BottomLeft() : aObjRect.BottomRight() ) &&
                            !pObject->IsResizeProtect() )
                {
                    //  geschuetzte Groessen werden nicht veraendert
                    //  (Positionen schon, weil sie ja an der Zelle "verankert" sind)
                            AddCalcUndo< SdrUndoGeoObj >( *pObject );
                    long nOldSizeX = aObjRect.Right() - aObjRect.Left() + 1;
                    long nOldSizeY = aObjRect.Bottom() - aObjRect.Top() + 1;
                    long nLogMoveX = rMove.X() * ( bNegativePage ? -1 : 1 );    // logical direction

                    sdr::legacy::ResizeSdrObject(*pObject, aOldMMPos, Fraction( nOldSizeX+nLogMoveX, nOldSizeX ),
                                                Fraction( nOldSizeY+rMove.Y(), nOldSizeY ) );
                }
            }
        }
        pObject = aIter.Next();
    }
}

void ScDrawLayer::MoveArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                            SCsCOL nDx,SCsROW nDy, sal_Bool bInsDel, bool bUpdateNoteCaptionPos )
{
    DBG_ASSERT( pDoc, "ScDrawLayer::MoveArea without document" );
    if ( !pDoc )
        return;

    if (!bAdjustEnabled)
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

    //  drawing objects are now directly included in cut&paste
    //  -> only update references when inserting/deleting (or changing widths or heights)
    if ( bInsDel )
        MoveAreaTwips( nTab, aRect, aMove, aTopLeft );

        //
        //      Detektiv-Pfeile: Zellpositionen anpassen
        //

    MoveCells( nTab, nCol1,nRow1, nCol2,nRow2, nDx,nDy, bUpdateNoteCaptionPos );
}

void ScDrawLayer::WidthChanged( SCTAB nTab, SCCOL nCol, long nDifTwips )
{
    DBG_ASSERT( pDoc, "ScDrawLayer::WidthChanged without document" );
    if ( !pDoc )
        return;

    if (!bAdjustEnabled)
        return;

    Rectangle aRect;
    Point aTopLeft;

    for (SCCOL i=0; i<nCol; i++)
        aRect.Left() += pDoc->GetColWidth(i,nTab);
    aTopLeft.X() = aRect.Left();
    aRect.Left() += pDoc->GetColWidth(nCol,nTab);

    aRect.Right() = MAXMM;
    aRect.Top() = 0;
    aRect.Bottom() = MAXMM;

    //! aTopLeft ist falsch, wenn mehrere Spalten auf einmal ausgeblendet werden

    sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );
    if ( bNegativePage )
    {
        MirrorRectRTL( aRect );
        aTopLeft.X() = -aTopLeft.X();
        nDifTwips = -nDifTwips;
    }

    MoveAreaTwips( nTab, aRect, Point( nDifTwips,0 ), aTopLeft );
}

void ScDrawLayer::HeightChanged( SCTAB nTab, SCROW nRow, long nDifTwips )
{
    DBG_ASSERT( pDoc, "ScDrawLayer::HeightChanged without document" );
    if ( !pDoc )
        return;

    if (!bAdjustEnabled)
        return;

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page not found");
    if (!pPage)
        return;

    // for an empty page, there's no need to calculate the row heights
    if (!pPage->GetObjCount())
        return;

    Rectangle aRect;
    Point aTopLeft;

    aRect.Top() += pDoc->GetRowHeight( 0, nRow-1, nTab);
    aTopLeft.Y() = aRect.Top();
    aRect.Top() += pDoc->GetRowHeight(nRow, nTab);

    aRect.Bottom() = MAXMM;
    aRect.Left() = 0;
    aRect.Right() = MAXMM;

    //! aTopLeft ist falsch, wenn mehrere Zeilen auf einmal ausgeblendet werden

    sal_Bool bNegativePage = pDoc->IsNegativePage( nTab );
    if ( bNegativePage )
    {
        MirrorRectRTL( aRect );
        aTopLeft.X() = -aTopLeft.X();
    }

    MoveAreaTwips( nTab, aRect, Point( 0,nDifTwips ), aTopLeft );
}

sal_Bool ScDrawLayer::HasObjectsInRows( SCTAB nTab, SCROW nStartRow, SCROW nEndRow, bool bIncludeNotes )
{
    DBG_ASSERT( pDoc, "ScDrawLayer::HasObjectsInRows without document" );
    if ( !pDoc )
        return sal_False;

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page not found");
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

    sal_Bool bFound = sal_False;

    Rectangle aObjRect;
    SdrObjListIter aIter( *pPage );
    SdrObject* pObject = aIter.Next();
    while ( pObject && !bFound )
    {
        aObjRect = sdr::legacy::GetSnapRect(*pObject);  //! GetLogicRect ?
        // #i116164# note captions are handled separately, don't have to be included for each single row height change
        if ( (aTestRect.IsInside(aObjRect.TopLeft()) || aTestRect.IsInside(aObjRect.BottomLeft())) &&
             (bIncludeNotes || !IsNoteCaption(*pObject)) )
        {
            bFound = sal_True;
        }

        pObject = aIter.Next();
    }

    return bFound;
}

#if 0
void ScDrawLayer::DeleteObjects( SCTAB nTab )
{
    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return;

    long    nDelCount = 0;
    sal_uInt32 nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        SdrObject** ppObj = new SdrObject*[nObjCount];

        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            //  alle loeschen
            ppObj[nDelCount++] = pObject;
            pObject = aIter.Next();
        }

        long i;
        if (bRecording)
            for (i=1; i<=nDelCount; i++)
                        AddCalcUndo< SdrUndoRemoveObj >( *ppObj[nDelCount-i] );

        for (i=1; i<=nDelCount; i++)
            pPage->RemoveObjectFromSdrObjList( ppObj[nDelCount-i]->GetNavigationPosition() );

        delete[] ppObj;
    }
}
#endif

void ScDrawLayer::DeleteObjectsInArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1,
                                            SCCOL nCol2,SCROW nRow2 )
{
    DBG_ASSERT( pDoc, "ScDrawLayer::DeleteObjectsInArea without document" );
    if ( !pDoc )
        return;

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return;

    long    nDelCount = 0;
    sal_uInt32 nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        Rectangle aDelRect = pDoc->GetMMRect( nCol1, nRow1, nCol2, nRow2, nTab );

        SdrObject** ppObj = new SdrObject*[nObjCount];

        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            // do not delete note caption, they are always handled by the cell note
            // TODO: detective objects are still deleted, is this desired?
            if (!IsNoteCaption( *pObject ))
            {
                Rectangle aObjRect = sdr::legacy::GetBoundRect(*pObject);
                if ( aDelRect.IsInside( aObjRect ) )
                    ppObj[nDelCount++] = pObject;
            }

            pObject = aIter.Next();
        }

        long i;
        if (bRecording)
            for (i=1; i<=nDelCount; i++)
                        AddCalcUndo< SdrUndoRemoveObj >( *ppObj[nDelCount-i] );

        for (i=1; i<=nDelCount; i++)
            pPage->RemoveObjectFromSdrObjList( ppObj[nDelCount-i]->GetNavigationPosition() );

        delete[] ppObj;
    }
}

void ScDrawLayer::DeleteObjectsInSelection( const ScMarkData& rMark )
{
    DBG_ASSERT( pDoc, "ScDrawLayer::DeleteObjectsInSelection without document" );
    if ( !pDoc )
        return;

    if ( !rMark.IsMultiMarked() )
        return;

    ScRange aMarkRange;
    rMark.GetMultiMarkArea( aMarkRange );

    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<=nTabCount; nTab++)
        if ( rMark.GetTableSelect( nTab ) )
        {
            SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
            if (pPage)
            {
                long    nDelCount = 0;
                sal_uInt32 nObjCount = pPage->GetObjCount();
                if (nObjCount)
                {
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
                        if (!IsNoteCaption( *pObject ))
                        {
                            Rectangle aObjRect = sdr::legacy::GetBoundRect(*pObject);
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
                                        AddCalcUndo< SdrUndoRemoveObj >( *ppObj[nDelCount-i] );

                    for (i=1; i<=nDelCount; i++)
                        pPage->RemoveObjectFromSdrObjList( ppObj[nDelCount-i]->GetNavigationPosition() );

                    delete[] ppObj;
                }
            }
            else
            {
                DBG_ERROR("pPage?");
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
            Rectangle aObjRect = sdr::legacy::GetBoundRect(*pOldObject);
            // do not copy internal objects (detective) and note captions
            if ( rRange.IsInside( aObjRect ) && (pOldObject->GetLayer() != SC_LAYER_INTERN) && !IsNoteCaption( *pOldObject ) )
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

                DBG_ASSERT( pDestPage, "no page" );
                if (pDestPage)
                {
                    SdrObject* pNewObject = pOldObject->CloneSdrObject();
                    uno::Reference< chart2::XChartDocument > xOldChart( ScChartHelper::GetChartFromSdrObject( pOldObject ) );
                    pDestPage->InsertObjectToSdrObjList(*pNewObject);
                    //  no undo needed in clipboard document
                    //  charts are not updated
                }
            }

            pOldObject = aIter.Next();
        }
    }
}

sal_Bool lcl_IsAllInRange( const ::std::vector< ScRangeList >& rRangesVector, const ScRange& rClipRange )
{
    //  check if every range of rRangesVector is completely in rClipRange

    ::std::vector< ScRangeList >::const_iterator aIt = rRangesVector.begin();
    for( ;aIt!=rRangesVector.end(); ++aIt )
    {
        const ScRangeList& rRanges = *aIt;
        sal_uLong nCount = rRanges.Count();
        for (sal_uLong i=0; i<nCount; i++)
        {
            ScRange aRange = *rRanges.GetObject(i);
            if ( !rClipRange.In( aRange ) )
            {
                return sal_False;   // at least one range is not valid
            }
        }
    }

    return sal_True;            // everything is fine
}

sal_Bool lcl_MoveRanges( ::std::vector< ScRangeList >& rRangesVector, const ScRange& rSourceRange, const ScAddress& rDestPos )
{
    sal_Bool bChanged = sal_False;

    ::std::vector< ScRangeList >::iterator aIt = rRangesVector.begin();
    for( ;aIt!=rRangesVector.end(); ++aIt )
    {
        ScRangeList& rRanges = *aIt;
        sal_uLong nCount = rRanges.Count();
        for (sal_uLong i=0; i<nCount; i++)
        {
            ScRange* pRange = rRanges.GetObject(i);
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
    DBG_ASSERT( pDoc, "ScDrawLayer::CopyFromClip without document" );
    if ( !pDoc )
        return;

    if (!pClipModel)
        return;

    if (bDrawIsInUndo)      //! can this happen?
    {
        DBG_ERROR("CopyFromClip, bDrawIsInUndo");
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
    DBG_ASSERT( pSrcPage && pDestPage, "draw page missing" );
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
    String aDestTabName;
    sal_Bool bRestoreDestTabName = sal_False;
    if( pOldObject && !bSameDoc && !bDestClip )
    {
        if( pDoc && pClipDoc )
        {
            String aSourceTabName;
            if( pClipDoc->GetName( nSourceTab, aSourceTabName )
                && pDoc->GetName( nDestTab, aDestTabName ) )
            {
                if( !(aSourceTabName==aDestTabName) &&
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
    sal_Bool bResize = sal_False;
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
        Rectangle aObjRect = sdr::legacy::GetBoundRect(*pOldObject);
        // do not copy internal objects (detective) and note captions
        if ( rSourceRange.IsInside( aObjRect ) && (pOldObject->GetLayer() != SC_LAYER_INTERN) && !IsNoteCaption( *pOldObject ) )
        {
            SdrObject* pNewObject = pOldObject->CloneSdrObject();
            if ( bMirrorObj )
                MirrorRTL( pNewObject );        // first mirror, then move

            sdr::legacy::MoveSdrObject(*pNewObject, aMove );
            if ( bResize )
                sdr::legacy::ResizeSdrObject(*pNewObject, aRefPos, aHorFract, aVerFract );

            pDestPage->InsertObjectToSdrObjList(*pNewObject);
            if (bRecording)
                        AddCalcUndo< SdrUndoInsertObj >( *pNewObject );

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
                            sal_Bool bInSourceRange = sal_False;
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
        bCanMirror = aInfo.mbMirror90Allowed;
    }

    if (bCanMirror)
    {
        Point aRef1( 0, 0 );
        Point aRef2( 0, 1 );
        if (bRecording)
                    AddCalcUndo< SdrUndoGeoObj >( *pObj );
        sdr::legacy::MirrorSdrObject(*pObj, aRef1, aRef2 );
    }
    else
    {
        //  Move instead of mirroring:
        //  New start position is negative of old end position
        //  -> move by sum of start and end position
        const basegfx::B2DRange aObjRange(sdr::legacy::GetLogicRange(*pObj));
        const double aDeltaX(-aObjRange.getMinX() - aObjRange.getMaxX());

        if (bRecording)
        {
            AddCalcUndo(new SdrUndoGeoObj(*pObj));
        }

        sdr::legacy::transformSdrObject(*pObj, basegfx::tools::createTranslateB2DHomMatrix(aDeltaX, 0.0));
    }
}

// static
void ScDrawLayer::MirrorRectRTL( Rectangle& rRect )
{
    //  mirror and swap left/right
    long nTemp = rRect.Left();
    rRect.Left() = -rRect.Right();
    rRect.Right() = -nTemp;
}

basegfx::B2DRange ScDrawLayer::GetCellRange(ScDocument& rDoc, const ScAddress& rPos, bool bMergedCell)
{
    const Rectangle aCellRect(GetCellRect(rDoc, rPos, bMergedCell));
    return basegfx::B2DRange(aCellRect.Left(), aCellRect.Top(), aCellRect.Right(), aCellRect.Bottom());
}

Rectangle ScDrawLayer::GetCellRect( ScDocument& rDoc, const ScAddress& rPos, bool bMergedCell )
{
    Rectangle aCellRect;
    DBG_ASSERT( ValidColRowTab( rPos.Col(), rPos.Row(), rPos.Tab() ), "ScDrawLayer::GetCellRect - invalid cell address" );
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

// static
String ScDrawLayer::GetVisibleName( SdrObject* pObj )
{
    String aName = pObj->GetName();
    if ( pObj->GetObjIdentifier() == OBJ_OLE2 )
    {
        //  #95575# For OLE, the user defined name (GetName) is used
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

    return ( pObj->GetName() == rName ||
            ( pObj->GetObjIdentifier() == OBJ_OLE2 &&
              static_cast<SdrOle2Obj*>(pObj)->GetPersistName() == rName ) );
}

SdrObject* ScDrawLayer::GetNamedObject( const String& rName, sal_uInt16 nId, SCTAB& rFoundTab ) const
{
    const sal_uInt32 nTabCount(GetPageCount());
    for (sal_uInt32 nTab=0; nTab<nTabCount; nTab++)
    {
        const SdrPage* pPage = GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");
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

    const sal_uInt32 nTabCount(GetPageCount());
    for (sal_uInt32 nTab=0; nTab<nTabCount; nTab++)
    {
        SdrPage* pPage = GetPage(nTab);
        DBG_ASSERT(pPage,"Page ?");
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
            SdrObject* pObject = aIter.Next();

            /* #101799# The index passed to GetNewGraphicName() will be set to
                the used index in each call. This prevents the repeated search
                for all names from 1 to current index. */
            long nCounter = 0;

            while (pObject)
            {
                if ( pObject->GetObjIdentifier() == OBJ_GRAF && pObject->GetName().Len() == 0 )
                    pObject->SetName( GetNewGraphicName( &nCounter ) );

                pObject = aIter.Next();
            }
        }
    }
}

void ScDrawLayer::SetAnchor( SdrObject* pObj, ScAnchorType eType )
{
    ScAnchorType eOldAnchorType = GetAnchor( pObj );

    // Ein an der Seite verankertes Objekt zeichnet sich durch eine Anker-Pos
    // von (0,1) aus. Das ist ein shabby Trick, der aber funktioniert!

    // #i108739# get rid of that SC anchor hack, replace with bool flag
    pObj->setUniversalApplicationFlag01(SCA_PAGE == eType);

    if ( eOldAnchorType != eType )
        pObj->notifyShapePropertyChange( ::svx::eSpreadsheetAnchor );
}

ScAnchorType ScDrawLayer::GetAnchor( const SdrObject* pObj )
{
    // #i108739# get rid of that SC anchor hack, replace with bool flag
    return pObj->getUniversalApplicationFlag01() ? SCA_PAGE : SCA_CELL;
}

ScDrawObjData* ScDrawLayer::GetObjData( const SdrObject& rObj, sal_Bool bCreate )       // static
{
    const sal_uInt32 nCount(rObj.GetUserDataCount());

    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        SdrObjUserData* pData = rObj.GetUserData( i );
        if( pData && pData->GetInventor() == SC_DRAWLAYER
                    && pData->GetId() == SC_UD_OBJDATA )
            return (ScDrawObjData*) pData;
    }
    if( bCreate )
    {
        ScDrawObjData* pData = new ScDrawObjData;
        const_cast< SdrObject& >(rObj).InsertUserData( pData, 0 );
        return pData;
    }
    return 0;
}

ScDrawObjData* ScDrawLayer::GetObjDataTab( const SdrObject& rObj, SCTAB nTab )    // static
{
    ScDrawObjData* pData = GetObjData( rObj );
    if ( pData )
    {
        if ( pData->maStart.IsValid() )
            pData->maStart.SetTab( nTab );
        if ( pData->maEnd.IsValid() )
            pData->maEnd.SetTab( nTab );
    }
    return pData;
}

bool ScDrawLayer::IsNoteCaption( const SdrObject& rObj )
{
    ScDrawObjData* pData = GetObjData( rObj );

    return pData && pData->mbNote;
}

ScDrawObjData* ScDrawLayer::GetNoteCaptionData( const SdrObject& rObj, SCTAB nTab )
{
    ScDrawObjData* pData = GetObjDataTab( rObj, nTab );

    return (pData && pData->mbNote) ? pData : 0;
}

ScIMapInfo* ScDrawLayer::GetIMapInfo( SdrObject* pObj )             // static
{
    const sal_uInt32 nCount(pObj->GetUserDataCount());

    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        SdrObjUserData* pData = pObj->GetUserData( i );
        if( pData && pData->GetInventor() == SC_DRAWLAYER
                    && pData->GetId() == SC_UD_IMAPDATA )
            return (ScIMapInfo*) pData;
    }
    return NULL;
}

// static:
IMapObject* ScDrawLayer::GetHitIMapObject(SdrObject& rObj,  const basegfx::B2DPoint& rWinPoint, const Window& rCmpWnd)
{
    const basegfx::B2DHomMatrix aTransWinTo100thmm(rCmpWnd.GetTransformLogicToLogic(rCmpWnd.GetMapMode(), MapMode(MAP_100TH_MM)));
    basegfx::B2DPoint aRelPoint(aTransWinTo100thmm * rWinPoint);
    basegfx::B2DRange aLogRange(sdr::legacy::GetLogicRange(rObj));
    aLogRange.transform(aTransWinTo100thmm);
    ScIMapInfo* pIMapInfo = GetIMapInfo(&rObj);
    IMapObject*         pIMapObj = NULL;

    if ( pIMapInfo )
    {
        Size        aGraphSize;
        ImageMap&   rImageMap = (ImageMap&) pIMapInfo->GetImageMap();
        bool bObjSupported(false);
        const SdrGrafObj* pGrafObj = dynamic_cast< const SdrGrafObj* >(&rObj);

        if ( pGrafObj  ) // einfaches Grafik-Objekt
        {
            // remove shear, mirror and rotation; so just absolute scale and translation get applied
            basegfx::B2DHomMatrix aJustAbsScaleTranslate(pGrafObj->getSdrObjectTransformation());

            aJustAbsScaleTranslate.invert();
            aJustAbsScaleTranslate.scale(basegfx::absolute(pGrafObj->getSdrObjectScale()));
            aJustAbsScaleTranslate.translate(pGrafObj->getSdrObjectTranslate());
            aRelPoint = aJustAbsScaleTranslate * aRelPoint;

            const Graphic& rGraphic = pGrafObj->GetGraphic();

            if ( rGraphic.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
            {
                aGraphSize = rCmpWnd.PixelToLogic( rGraphic.GetPrefSize(), MapMode( MAP_100TH_MM ) );
            }
            else
            {
                const MapMode       aMap100( MAP_100TH_MM );
                aGraphSize = OutputDevice::LogicToLogic( rGraphic.GetPrefSize(),
                    rGraphic.GetPrefMapMode(), MapMode( MAP_100TH_MM ) );
            }

            bObjSupported = true;
        }
        else
        {
            const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(&rObj);

            if ( pSdrOle2Obj ) // OLE-Objekt
        {
            // TODO/LEAN: working with visual area needs running state
                aGraphSize = pSdrOle2Obj->GetOrigObjSize();
                bObjSupported = true;
            }
        }

        // hat alles geklappt, dann HitTest ausfuehren
        if ( bObjSupported )
        {
            // relativen Mauspunkt berechnen
            aRelPoint -= aLogRange.getMinimum();
            const Size aDisplaySize(basegfx::fround(aLogRange.getWidth()), basegfx::fround(aLogRange.getHeight()));
            const Point aRelHitPoint(basegfx::fround(aRelPoint.getX()), basegfx::fround(aRelPoint.getY()));
            pIMapObj = rImageMap.GetHitIMapObject(aGraphSize, aDisplaySize, aRelHitPoint);
        }
    }

    return pIMapObj;
}

ScMacroInfo* ScDrawLayer::GetMacroInfo( SdrObject* pObj, sal_Bool bCreate )             // static
{
    const sal_uInt32 nCount(pObj->GetUserDataCount());

    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        SdrObjUserData* pData = pObj->GetUserData( i );
        if( pData && pData->GetInventor() == SC_DRAWLAYER
                    && pData->GetId() == SC_UD_MACRODATA )
            return (ScMacroInfo*) pData;
    }
    if ( bCreate )
    {
        ScMacroInfo* pData = new ScMacroInfo;
        pObj->InsertUserData( pData, 0 );
        return pData;
    }
    return 0;
}
//IAccessibility2 Implementation 2009-----
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
    ScMacroInfo* pMacroInfo = GetMacroInfo(pObj, sal_False);
    if (pMacroInfo)
        // MT IA2: GetHlink*( doesn|t exist in DEV300 anymore...
        nHLCount = 0; // pMacroInfo->GetHlink().getLength() > 0 ? 1 : 0;
    return nHLCount;
}
//-----IAccessibility2 Implementation 2009
void ScDrawLayer::SetGlobalDrawPersist(SfxObjectShell* pPersist)            // static
{
    DBG_ASSERT(!pGlobalDrawPersist,"SetGlobalDrawPersist mehrfach");
    pGlobalDrawPersist = pPersist;
}

void __EXPORT ScDrawLayer::SetChanged( bool bFlg /* = true */ )
{
    if ( bFlg && pDoc )
        pDoc->SetChartListenerCollectionNeedsUpdate( sal_True );
    FmFormModel::SetChanged( bFlg );
}

SvStream* __EXPORT ScDrawLayer::GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const
{
    DBG_ASSERT( pDoc, "ScDrawLayer::GetDocumentStream without document" );
    if ( !pDoc )
        return NULL;

    uno::Reference< embed::XStorage > xStorage = pDoc->GetDocumentShell() ?
                                                        pDoc->GetDocumentShell()->GetStorage() :
                                                        NULL;
    SvStream*   pRet = NULL;

    if( xStorage.is() )
    {
        if( rStreamInfo.maUserData.Len() &&
            ( rStreamInfo.maUserData.GetToken( 0, ':' ) ==
              String( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package" ) ) ) )
        {
            const String aPicturePath( rStreamInfo.maUserData.GetToken( 1, ':' ) );

            // graphic from picture stream in picture storage in XML package
            if( aPicturePath.GetTokenCount( '/' ) == 2 )
            {
                const String aPictureStreamName( aPicturePath.GetToken( 1, '/' ) );
                const String aPictureStorageName( aPicturePath.GetToken( 0, '/' ) );

                try {
                    if ( xStorage->isStorageElement( aPictureStorageName ) )
                    {
                        uno::Reference< embed::XStorage > xPictureStorage =
                                    xStorage->openStorageElement( aPictureStorageName, embed::ElementModes::READ );

                        if( xPictureStorage.is() &&
                            xPictureStorage->isStreamElement( aPictureStreamName ) )
                        {
                            uno::Reference< io::XStream > xStream =
                                xPictureStorage->openStreamElement( aPictureStreamName, embed::ElementModes::READ );
                            if ( xStream.is() )
                                pRet = ::utl::UcbStreamHelper::CreateStream( xStream );
                        }
                    }
                }
                catch( uno::Exception& )
                {
                    // TODO: error handling
                }
            }
        }
        // the following code seems to be related to binary format
//REMOVE            else
//REMOVE            {
//REMOVE                pRet = pStor->OpenStream( String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(STRING_SCSTREAM)),
//REMOVE                                          STREAM_READ | STREAM_WRITE | STREAM_TRUNC );
//REMOVE
//REMOVE                if( pRet )
//REMOVE                {
//REMOVE                    pRet->SetVersion( pStor->GetVersion() );
//REMOVE                    pRet->SetKey( pStor->GetKey() );
//REMOVE                }
//REMOVE            }

        rStreamInfo.mbDeleteAfterUse = ( pRet != NULL );
    }

    return pRet;
}

//REMOVE    void ScDrawLayer::ReleasePictureStorage()
//REMOVE    {
//REMOVE        xPictureStorage.Clear();
//REMOVE    }

SdrLayerID __EXPORT ScDrawLayer::GetControlExportLayerId( const SdrObject & ) const
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
