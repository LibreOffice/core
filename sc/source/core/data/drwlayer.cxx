/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drwlayer.cxx,v $
 *
 *  $Revision: 1.53 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:19:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XVISUALOBJECT_HPP_
#include <com/sun/star/embed/XVisualObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XCLASSIFIEDOBJECT_HPP_
#include <com/sun/star/embed/XClassifiedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XCOMPONENTSUPPLIER_HPP_
#include <com/sun/star/embed/XComponentSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>

#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
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
#include <svx/unolingu.hxx>
#include <svx/drawitem.hxx>
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#include <svx/scriptspaceitem.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/docinf.hxx>
#include <sfx2/docfile.hxx>
#include <sot/storage.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/itempool.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <unotools/ucbstreamhelper.hxx>

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

#define DET_ARROW_OFFSET    1000

//  Abstand zur naechsten Zelle beim Loeschen (bShrink), damit der Anker
//  immer an der richtigen Zelle angezeigt wird
//#define SHRINK_DIST       3
//  und noch etwas mehr, damit das Objekt auch sichtbar in der Zelle liegt
#define SHRINK_DIST     25

#define SHRINK_DIST_TWIPS   15

using namespace ::com::sun::star;

// -----------------------------------------------------------------------
//
//  Das Anpassen der Detektiv-UserData muss zusammen mit den Draw-Undo's
//  in der SdrUndoGroup liegen, darum von SdrUndoAction abgeleitet:

class ScUndoObjData : public SdrUndoObj
{
private:
    ScAddress   aOldStt;
    ScAddress   aOldEnd;
    ScAddress   aNewStt;
    ScAddress   aNewEnd;
    BOOL        bHasNew;
public:
                ScUndoObjData( SdrObject* pObj, const ScAddress& rOS, const ScAddress& rOE,
                                                const ScAddress& rNS, const ScAddress& rNE );
                ~ScUndoObjData();

    virtual void     Undo();
    virtual void     Redo();
};

// -----------------------------------------------------------------------

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScTabDeletedHint, SfxHint);
TYPEINIT1(ScTabSizeChangedHint, SfxHint);

static ScDrawObjFactory* pFac = NULL;
static E3dObjFactory* pF3d = NULL;
static USHORT nInst = 0;

SfxObjectShell* ScDrawLayer::pGlobalDrawPersist = NULL;
//REMOVE    SvPersist* ScDrawLayer::pGlobalDrawPersist = NULL;

BOOL bDrawIsInUndo = FALSE;         //! Member

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

void __EXPORT ScUndoObjData::Undo()
{
    ScDrawObjData* pData = ((ScDrawLayer&)rMod).GetObjData( pObj );
    DBG_ASSERT(pData,"ScUndoObjData: Daten nicht da");
    if (pData)
    {
        pData->aStt = aOldStt;
        pData->aEnd = aOldEnd;
    }
}

void __EXPORT ScUndoObjData::Redo()
{
    ScDrawObjData* pData = ((ScDrawLayer&)rMod).GetObjData( pObj );
    DBG_ASSERT(pData,"ScUndoObjData: Daten nicht da");
    if (pData)
    {
        pData->aStt = aNewStt;
        pData->aEnd = aNewEnd;
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

BOOL lcl_MirrorCheckRect(const Rectangle& rRect, BOOL bNegativePage)
{
    BOOL bMirrorChange = false;

    if ( bNegativePage )
    {
        if(rRect.Left() >= 0 && rRect.Right() > 0)
            bMirrorChange = true;
    }
    else
    {
        if(rRect.Left() < 0 && rRect.Right() <= 0)
            bMirrorChange = true;
    }

    return bMirrorChange;
}

// -----------------------------------------------------------------------


ScDrawLayer::ScDrawLayer( ScDocument* pDocument, const String& rName ) :
    FmFormModel( SvtPathOptions().GetPalettePath(),
                 NULL,                          // SfxItemPool* Pool
                 pGlobalDrawPersist ?
                     pGlobalDrawPersist :
                     ( pDocument ? pDocument->GetDocumentShell() : NULL ),
                 TRUE ),        // bUseExtColorTable (is set below)
    aName( rName ),
    pDoc( pDocument ),
    pUndoGroup( NULL ),
    bRecording( FALSE ),
    bAdjustEnabled( TRUE ),
    bHyphenatorSet( FALSE )
{
    pGlobalDrawPersist = NULL;          // nur einmal benutzen

    SfxObjectShell* pObjSh = pDocument ? pDocument->GetDocumentShell() : NULL;
    if ( pObjSh )
    {
        SetObjectShell( pObjSh );

        // set color table
        SvxColorTableItem* pColItem = (SvxColorTableItem*) pObjSh->GetItem( SID_COLOR_TABLE );
        XColorTable* pXCol = pColItem ? pColItem->GetColorTable() : XColorTable::GetStdColorTable();
        SetColorTable( pXCol );
    }
    else
        SetColorTable( XColorTable::GetStdColorTable() );

    SetSwapGraphics(TRUE);
//  SetSwapAsynchron(TRUE);     // an der View

    SetScaleUnit(MAP_100TH_MM);
    SfxItemPool& rPool = GetItemPool();
    rPool.SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
    SvxFrameDirectionItem aModeItem( FRMDIR_ENVIRONMENT, EE_PARA_WRITINGDIR );
    rPool.SetPoolDefaultItem( aModeItem );

    // #i33700#
    // Set shadow distance defaults as PoolDefaultItems. Details see bug.
    rPool.SetPoolDefaultItem(SdrShadowXDistItem(300));
    rPool.SetPoolDefaultItem(SdrShadowYDistItem(300));

    // #111216# default for script spacing depends on locale, see SdDrawDocument ctor in sd
    LanguageType eOfficeLanguage = Application::GetSettings().GetLanguage();
    if ( eOfficeLanguage == LANGUAGE_KOREAN || eOfficeLanguage == LANGUAGE_KOREAN_JOHAB ||
         eOfficeLanguage == LANGUAGE_JAPANESE )
    {
        // secondary is edit engine pool
        rPool.GetSecondaryPool()->SetPoolDefaultItem( SvxScriptSpaceItem( FALSE, EE_PARA_ASIANCJKSPACING ) );
    }

    rPool.FreezeIdRanges();                         // the pool is also used directly

    SdrLayerAdmin& rAdmin = GetLayerAdmin();
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

    Outliner& rHitOutliner = GetHitTestOutliner();
    rHitOutliner.SetCalcFieldValueHdl( LINK( pScMod, ScModule, CalcFieldValueHdl ) );

    // #95129# SJ: set FontHeight pool defaults without changing static SdrEngineDefaults
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

__EXPORT ScDrawLayer::~ScDrawLayer()
{
    Broadcast(SdrHint(HINT_MODELCLEARED));

    // #116168#
    //Clear();
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

        bHyphenatorSet = TRUE;
    }
}

SdrPage* __EXPORT ScDrawLayer::AllocPage(FASTBOOL bMasterPage)
{
    //  don't create basic until it is needed
    StarBASIC* pBasic = NULL;
    ScDrawPage* pPage = new ScDrawPage( *this, pBasic, sal::static_int_cast<BOOL>(bMasterPage) );
    return pPage;
}

BOOL ScDrawLayer::HasObjects() const
{
    BOOL bFound = FALSE;

    USHORT nCount = GetPageCount();
    for (USHORT i=0; i<nCount && !bFound; i++)
        if (GetPage(i)->GetObjCount())
            bFound = TRUE;

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

BOOL ScDrawLayer::ScAddPage( SCTAB nTab )
{
    if (bDrawIsInUndo)
        return FALSE;   // not inserted

    ScDrawPage* pPage = (ScDrawPage*)AllocPage( FALSE );
    InsertPage(pPage, static_cast<sal_uInt16>(nTab));
    if (bRecording)
        AddCalcUndo(new SdrUndoNewPage(*pPage));

    return TRUE;        // inserted
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
}

void ScDrawLayer::ScRenamePage( SCTAB nTab, const String& rNewName )
{
    ScDrawPage* pPage = (ScDrawPage*) GetPage(static_cast<sal_uInt16>(nTab));
    if (pPage)
        pPage->SetName(rNewName);
}

void ScDrawLayer::ScMovePage( USHORT nOldPos, USHORT nNewPos )
{
    MovePage( nOldPos, nNewPos );
}

void ScDrawLayer::ScCopyPage( USHORT nOldPos, USHORT nNewPos, BOOL bAlloc )
{
    //! remove argument bAlloc (always FALSE)

    if (bDrawIsInUndo)
        return;

    SdrPage* pOldPage = GetPage(nOldPos);
    SdrPage* pNewPage = bAlloc ? AllocPage(FALSE) : GetPage(nNewPos);

    // kopieren

    if (pOldPage && pNewPage)
    {
        SdrObjListIter aIter( *pOldPage, IM_FLAT );
        SdrObject* pOldObject = aIter.Next();
        while (pOldObject)
        {
            // #116235#
            SdrObject* pNewObject = pOldObject->Clone();
            //SdrObject* pNewObject = pOldObject->Clone( pNewPage, this );
            pNewObject->SetModel(this);
            pNewObject->SetPage(pNewPage);

            pNewObject->NbcMove(Size(0,0));
            pNewPage->InsertObject( pNewObject );
            if (bRecording)
                AddCalcUndo( new SdrUndoInsertObj( *pNewObject ) );

            pOldObject = aIter.Next();
        }
    }

    if (bAlloc)
        InsertPage(pNewPage, nNewPos);
}

inline BOOL IsInBlock( const ScAddress& rPos, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2 )
{
    return rPos.Col() >= nCol1 && rPos.Col() <= nCol2 &&
           rPos.Row() >= nRow1 && rPos.Row() <= nRow2;
}

void ScDrawLayer::MoveCells( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                                SCsCOL nDx,SCsROW nDy )
{
    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page nicht gefunden");
    if (!pPage)
        return;

    BOOL bNegativePage = pDoc && pDoc->IsNegativePage( nTab );

    ULONG nCount = pPage->GetObjCount();
    for ( ULONG i = 0; i < nCount; i++ )
    {
        SdrObject* pObj = pPage->GetObj( i );
        ScDrawObjData* pData = GetObjDataTab( pObj, nTab );
        if( pData )
        {
            ScAddress aOldStt = pData->aStt;
            ScAddress aOldEnd = pData->aEnd;
            BOOL bChange = FALSE;
            if ( pData->bValidStart && IsInBlock( pData->aStt, nCol1,nRow1, nCol2,nRow2 ) )
            {
                pData->aStt.IncCol( nDx);
                pData->aStt.IncRow( nDy);
                bChange = TRUE;
            }
            if ( pData->bValidEnd && IsInBlock( pData->aEnd, nCol1,nRow1, nCol2,nRow2 ) )
            {
                pData->aEnd.IncCol( nDx);
                pData->aEnd.IncRow( nDy);
                bChange = TRUE;
            }
            if (bChange)
            {
                if ( pObj->ISA(SdrRectObj) && pData->bValidStart && pData->bValidEnd )
                {
                    pData->aStt.PutInOrder( pData->aEnd);
                }
                AddCalcUndo( new ScUndoObjData( pObj, aOldStt, aOldEnd, pData->aStt, pData->aEnd ) );
                RecalcPos( pObj, pData, bNegativePage );
            }
        }
    }
}

void ScDrawLayer::SetPageSize( USHORT nPageNo, const Size& rSize )
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

        BOOL bNegativePage = pDoc && pDoc->IsNegativePage( static_cast<SCTAB>(nPageNo) );

        ULONG nCount = pPage->GetObjCount();
        for ( ULONG i = 0; i < nCount; i++ )
        {
            SdrObject* pObj = pPage->GetObj( i );
            ScDrawObjData* pData = GetObjDataTab( pObj, static_cast<SCTAB>(nPageNo) );
            if( pData )
                RecalcPos( pObj, pData, bNegativePage );
        }
    }
}

void ScDrawLayer::RecalcPos( SdrObject* pObj, ScDrawObjData* pData, BOOL bNegativePage )
{
    DBG_ASSERT( pDoc, "ScDrawLayer::RecalcPos without document" );
    if ( !pDoc )
        return;

    BOOL bArrow = ( pObj->IsPolyObj() && pObj->GetPointCount()==2 );    // Pfeil ?
    BOOL bCircle = ( pObj->ISA(SdrCircObj) );                           // Kreis (Gueltigkeit)
    BOOL bCaption = ( pObj->ISA(SdrCaptionObj) && pObj->GetLayer() == SC_LAYER_INTERN );        // Notiz

    if (bCaption)
    {
        SdrCaptionObj* pCaptObj = (SdrCaptionObj*) pObj;

        SCCOL nCol = pData->aStt.Col();
        SCROW nRow = pData->aStt.Row();
        SCTAB nTab = pData->aStt.Tab();
        Point aPos( pDoc->GetColOffset( nCol+1, nTab ),
                    pDoc->GetRowOffset( nRow, nTab ) );
        TwipsToMM( aPos.X() );
        TwipsToMM( aPos.Y() );
        aPos.X() -= 10;
        if ( bNegativePage )
            aPos.X() = -aPos.X();

        Point aOldTail = pCaptObj->GetTailPos();
        if ( aOldTail != aPos )
        {
            pCaptObj->SetTailPos(aPos);

                    ScPostIt aNote(pDoc);
                    if (pDoc->GetNote( pData->aStt.Col(), pData->aStt.Row(), nTab, aNote))
                    {
                        Rectangle aRect = pCaptObj->GetLogicRect();
                        if(lcl_MirrorCheckRect( aRect, bNegativePage ))
                        {
                            MirrorRectRTL( aRect );
                            pCaptObj->SetLogicRect( aRect );
                            aNote.SetRectangle(aRect);
                            pDoc->SetNote( pData->aStt.Col(), pData->aStt.Row(), nTab, aNote);
                        }
                    }
                    else // new note in creation
                    {
                        Rectangle aRect = aNote.DefaultRectangle(ScAddress(pData->aStt.Col(), pData->aStt.Row(), nTab));
                        if(lcl_MirrorCheckRect( aRect, bNegativePage ))
                            MirrorRectRTL( aRect );
                        pCaptObj->SetLogicRect( aRect );
                    }
                    if (bRecording)
                        AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
        }
    }
    else if (bCircle)                   // Kreis (Gueltigkeit)
    {
        SCCOL nCol = pData->aStt.Col();
        SCROW nRow = pData->aStt.Row();
        SCTAB nTab = pData->aStt.Tab();
        Point aPos( pDoc->GetColOffset( nCol, nTab ), pDoc->GetRowOffset( nRow, nTab ) );
        TwipsToMM( aPos.X() );
        TwipsToMM( aPos.Y() );

        //  Berechnung und Werte wie in detfunc.cxx

        Size aSize( (long) ( pDoc->GetColWidth(nCol, nTab) * HMM_PER_TWIPS ),
                    (long) ( pDoc->GetRowHeight(nRow, nTab) * HMM_PER_TWIPS ) );
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
            pObj->SetLogicRect(aRect);
        }
    }
    else if (bArrow)                    // Pfeil
    {
        //! nicht mehrere Undos fuer ein Objekt erzeugen (hinteres kann dann weggelassen werden)

        if( pData->bValidStart )
        {
            Point aPos(
                pDoc->GetColOffset( pData->aStt.Col(), pData->aStt.Tab() ),
                pDoc->GetRowOffset( pData->aStt.Row(), pData->aStt.Tab() ) );
            if( !( pDoc->GetColFlags( pData->aStt.Col(), pData->aStt.Tab() )
                 & CR_HIDDEN ) )
                aPos.X() += pDoc->GetColWidth( pData->aStt.Col(), pData->aStt.Tab() ) / 4;
            if( !( pDoc->GetRowFlags( pData->aStt.Row(), pData->aStt.Tab() )
                 & CR_HIDDEN ) )
                aPos.Y() += pDoc->GetRowHeight( pData->aStt.Row(), pData->aStt.Tab() ) / 2;
            TwipsToMM( aPos.X() );
            TwipsToMM( aPos.Y() );
            Point aStartPos = aPos;
            if ( bNegativePage )
                aStartPos.X() = -aStartPos.X();     // don't modify aPos - used below
            if ( pObj->GetPoint(0) != aStartPos )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                pObj->SetPoint( aStartPos, 0 );
            }

            if( !pData->bValidEnd )
            {
                Point aEndPos( aPos.X() + DET_ARROW_OFFSET, aPos.Y() - DET_ARROW_OFFSET );
                if (aEndPos.Y() < 0)
                    aEndPos.Y() += 2*DET_ARROW_OFFSET;
                if ( bNegativePage )
                    aEndPos.X() = -aEndPos.X();
                if ( pObj->GetPoint(1) != aEndPos )
                {
                    if (bRecording)
                        AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                    pObj->SetPoint( aEndPos, 1 );
                }
            }
        }
        if( pData->bValidEnd )
        {
            Point aPos(
                pDoc->GetColOffset( pData->aEnd.Col(), pData->aEnd.Tab() ),
                pDoc->GetRowOffset( pData->aEnd.Row(), pData->aEnd.Tab() ) );
            if( !( pDoc->GetColFlags( pData->aEnd.Col(), pData->aEnd.Tab() )
                 & CR_HIDDEN ) )
                aPos.X() += pDoc->GetColWidth( pData->aEnd.Col(), pData->aEnd.Tab() ) / 4;
            if( !( pDoc->GetRowFlags( pData->aEnd.Row(), pData->aEnd.Tab() )
                 & CR_HIDDEN ) )
                aPos.Y() += pDoc->GetRowHeight( pData->aEnd.Row(), pData->aEnd.Tab() ) / 2;
            TwipsToMM( aPos.X() );
            TwipsToMM( aPos.Y() );
            Point aEndPos = aPos;
            if ( bNegativePage )
                aEndPos.X() = -aEndPos.X();         // don't modify aPos - used below
            if ( pObj->GetPoint(1) != aEndPos )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                pObj->SetPoint( aEndPos, 1 );
            }

            if( !pData->bValidStart )
            {
                Point aStartPos( aPos.X() - DET_ARROW_OFFSET, aPos.Y() - DET_ARROW_OFFSET );
                if (aStartPos.X() < 0)
                    aStartPos.X() += 2*DET_ARROW_OFFSET;
                if (aStartPos.Y() < 0)
                    aStartPos.Y() += 2*DET_ARROW_OFFSET;
                if ( bNegativePage )
                    aStartPos.X() = -aStartPos.X();
                if ( pObj->GetPoint(0) != aStartPos )
                {
                    if (bRecording)
                        AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                    pObj->SetPoint( aStartPos, 0 );
                }
            }
        }
    }
    else                                // Referenz-Rahmen
    {
        DBG_ASSERT( pData->bValidStart, "RecalcPos: kein Start" );
        Point aPos(
            pDoc->GetColOffset( pData->aStt.Col(), pData->aStt.Tab() ),
            pDoc->GetRowOffset( pData->aStt.Row(), pData->aStt.Tab() ) );
        TwipsToMM( aPos.X() );
        TwipsToMM( aPos.Y() );

        if( pData->bValidEnd )
        {
            Point aEnd(
                pDoc->GetColOffset( pData->aEnd.Col()+1, pData->aEnd.Tab() ),
                pDoc->GetRowOffset( pData->aEnd.Row()+1, pData->aEnd.Tab() ) );
            TwipsToMM( aEnd.X() );
            TwipsToMM( aEnd.Y() );

            Rectangle aNew( aPos, aEnd );
            if ( bNegativePage )
                MirrorRectRTL( aNew );
            if ( pObj->GetLogicRect() != aNew )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                pObj->SetLogicRect(aNew);
            }
        }
        else
        {
            if ( bNegativePage )
                aPos.X() = -aPos.X();
            if ( pObj->GetRelativePos() != aPos )
            {
                if (bRecording)
                    AddCalcUndo( new SdrUndoGeoObj( *pObj ) );
                pObj->SetRelativePos( aPos );
            }
        }
    }

}

BOOL ScDrawLayer::GetPrintArea( ScRange& rRange, BOOL bSetHor, BOOL bSetVer ) const
{
    DBG_ASSERT( pDoc, "ScDrawLayer::GetPrintArea without document" );
    if ( !pDoc )
        return FALSE;

    SCTAB nTab = rRange.aStart.Tab();
    DBG_ASSERT( rRange.aEnd.Tab() == nTab, "GetPrintArea: Tab unterschiedlich" );

    BOOL bNegativePage = pDoc->IsNegativePage( nTab );

    BOOL bAny = FALSE;
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
        nStartY = pDoc->FastGetRowHeight( 0, rRange.aStart.Row()-1, nTab);
        nEndY = nStartY + pDoc->FastGetRowHeight( rRange.aStart.Row(),
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

            Rectangle aObjRect = pObject->GetCurrentBoundRect();
            BOOL bFit = TRUE;
            if ( !bSetHor && ( aObjRect.Right() < nStartX || aObjRect.Left() > nEndX ) )
                bFit = FALSE;
            if ( !bSetVer && ( aObjRect.Bottom() < nStartY || aObjRect.Top() > nEndY ) )
                bFit = FALSE;
            if ( bFit )
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
                bAny = TRUE;
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
            SCROW nRow = pDoc->FastGetRowForHeight( nTab, nStartY);
            rRange.aStart.SetRow( nRow>0 ? (nRow-1) : 0);
            nRow = pDoc->FastGetRowForHeight( nTab, nEndY);
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
//! DBG_ASSERT( !bRecording, "BeginCalcUndo ohne GetCalcUndo" );

    DELETEZ(pUndoGroup);
    bRecording = TRUE;
}

SdrUndoGroup* ScDrawLayer::GetCalcUndo()
{
//! DBG_ASSERT( bRecording, "GetCalcUndo ohne BeginCalcUndo" );

    SdrUndoGroup* pRet = pUndoGroup;
    pUndoGroup = NULL;
    bRecording = FALSE;
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

    BOOL bNegativePage = pDoc && pDoc->IsNegativePage( nTab );

    // fuer Shrinking!
    Rectangle aNew( rArea );
    BOOL bShrink = FALSE;
    if ( rMove.X() < 0 || rMove.Y() < 0 )       // verkleinern
    {
        if ( rTopLeft != rArea.TopLeft() )      // sind gleich beim Verschieben von Zellen
        {
            bShrink = TRUE;
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
            if ( GetObjData( pObject ) )                    // Detektiv-Pfeil ?
            {
                // hier nichts
            }
            else if ( pObject->ISA( SdrEdgeObj ) )          // Verbinder?
            {
                //  hier auch nichts
                //! nicht verbundene Enden wie bei Linien (s.u.) behandeln?
            }
            else if ( pObject->IsPolyObj() && pObject->GetPointCount()==2 )
            {
                for (USHORT i=0; i<2; i++)
                {
                    BOOL bMoved = FALSE;
                    Point aPoint = pObject->GetPoint(i);
                    lcl_ReverseTwipsToMM( aPoint );
                    if (rArea.IsInside(aPoint))
                    {
                        aPoint += rMove; bMoved = TRUE;
                    }
                    else if (bShrink && aNew.IsInside(aPoint))
                    {
                        //  Punkt ist in betroffener Zelle - Test auf geloeschten Bereich
                        if ( rMove.X() && aPoint.X() >= rArea.Left() + rMove.X() )
                        {
                            aPoint.X() = rArea.Left() + rMove.X() - SHRINK_DIST_TWIPS;
                            if ( aPoint.X() < 0 ) aPoint.X() = 0;
                            bMoved = TRUE;
                        }
                        if ( rMove.Y() && aPoint.Y() >= rArea.Top() + rMove.Y() )
                        {
                            aPoint.Y() = rArea.Top() + rMove.Y() - SHRINK_DIST_TWIPS;
                            if ( aPoint.Y() < 0 ) aPoint.Y() = 0;
                            bMoved = TRUE;
                        }
                    }
                    if( bMoved )
                    {
                        AddCalcUndo( new SdrUndoGeoObj( *pObject ) );
                        lcl_TwipsToMM( aPoint );
                        pObject->SetPoint( aPoint, i );
                    }
                }
            }
            else
            {
                Rectangle aObjRect = pObject->GetLogicRect();
                // aOldMMPos: not converted, millimeters
                Point aOldMMPos = bNegativePage ? aObjRect.TopRight() : aObjRect.TopLeft();
                lcl_ReverseTwipsToMM( aObjRect );
                Point aTopLeft = bNegativePage ? aObjRect.TopRight() : aObjRect.TopLeft();  // logical left
                Size aMoveSize;
                BOOL bDoMove = FALSE;
                if (rArea.IsInside(aTopLeft))
                {
                    aMoveSize = Size(rMove.X(),rMove.Y());
                    bDoMove = TRUE;
                }
                else if (bShrink && aNew.IsInside(aTopLeft))
                {
                    //  Position ist in betroffener Zelle - Test auf geloeschten Bereich
                    if ( rMove.X() && aTopLeft.X() >= rArea.Left() + rMove.X() )
                    {
                        aMoveSize.Width() = rArea.Left() + rMove.X() - SHRINK_DIST - aTopLeft.X();
                        bDoMove = TRUE;
                    }
                    if ( rMove.Y() && aTopLeft.Y() >= rArea.Top() + rMove.Y() )
                    {
                        aMoveSize.Height() = rArea.Top() + rMove.Y() - SHRINK_DIST - aTopLeft.Y();
                        bDoMove = TRUE;
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

                    AddCalcUndo( new SdrUndoMoveObj( *pObject, aMoveSize ) );
                    pObject->Move( aMoveSize );
                }
                else if ( rArea.IsInside( bNegativePage ? aObjRect.BottomLeft() : aObjRect.BottomRight() ) &&
                            !pObject->IsResizeProtect() )
                {
                    //  geschuetzte Groessen werden nicht veraendert
                    //  (Positionen schon, weil sie ja an der Zelle "verankert" sind)
                    AddCalcUndo( new SdrUndoGeoObj( *pObject ) );
                    long nOldSizeX = aObjRect.Right() - aObjRect.Left() + 1;
                    long nOldSizeY = aObjRect.Bottom() - aObjRect.Top() + 1;
                    long nLogMoveX = rMove.X() * ( bNegativePage ? -1 : 1 );    // logical direction
                    pObject->Resize( aOldMMPos, Fraction( nOldSizeX+nLogMoveX, nOldSizeX ),
                                                Fraction( nOldSizeY+rMove.Y(), nOldSizeY ) );
                }
            }
        }
        pObject = aIter.Next();
    }
}

void ScDrawLayer::MoveArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                            SCsCOL nDx,SCsROW nDy, BOOL bInsDel )
{
    DBG_ASSERT( pDoc, "ScDrawLayer::MoveArea without document" );
    if ( !pDoc )
        return;

    if (!bAdjustEnabled)
        return;

    BOOL bNegativePage = pDoc->IsNegativePage( nTab );

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
        aMove.Y() += pDoc->FastGetRowHeight( nRow1, nRow1+nDy-1, nTab);
    else
        aMove.Y() -= pDoc->FastGetRowHeight( nRow1+nDy, nRow1-1, nTab);

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

    MoveCells( nTab, nCol1,nRow1, nCol2,nRow2, nDx,nDy );
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

    BOOL bNegativePage = pDoc->IsNegativePage( nTab );
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

    Rectangle aRect;
    Point aTopLeft;

    aRect.Top() += pDoc->FastGetRowHeight( 0, nRow-1, nTab);
    aTopLeft.Y() = aRect.Top();
    aRect.Top() += pDoc->FastGetRowHeight(nRow,nTab);

    aRect.Bottom() = MAXMM;
    aRect.Left() = 0;
    aRect.Right() = MAXMM;

    //! aTopLeft ist falsch, wenn mehrere Zeilen auf einmal ausgeblendet werden

    BOOL bNegativePage = pDoc->IsNegativePage( nTab );
    if ( bNegativePage )
    {
        MirrorRectRTL( aRect );
        aTopLeft.X() = -aTopLeft.X();
    }

    MoveAreaTwips( nTab, aRect, Point( 0,nDifTwips ), aTopLeft );
}

BOOL ScDrawLayer::HasObjectsInRows( SCTAB nTab, SCROW nStartRow, SCROW nEndRow )
{
    DBG_ASSERT( pDoc, "ScDrawLayer::HasObjectsInRows without document" );
    if ( !pDoc )
        return FALSE;

    Rectangle aTestRect;

    aTestRect.Top() += pDoc->FastGetRowHeight( 0, nStartRow-1, nTab);

    if (nEndRow==MAXROW)
        aTestRect.Bottom() = MAXMM;
    else
    {
        aTestRect.Bottom() = aTestRect.Top();
        aTestRect.Bottom() += pDoc->FastGetRowHeight( nStartRow, nEndRow, nTab);
        TwipsToMM( aTestRect.Bottom() );
    }

    TwipsToMM( aTestRect.Top() );

    aTestRect.Left()  = 0;
    aTestRect.Right() = MAXMM;

    BOOL bNegativePage = pDoc->IsNegativePage( nTab );
    if ( bNegativePage )
        MirrorRectRTL( aTestRect );

    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page nicht gefunden");
    if (!pPage)
        return FALSE;

    BOOL bFound = FALSE;

    Rectangle aObjRect;
    SdrObjListIter aIter( *pPage );
    SdrObject* pObject = aIter.Next();
    while ( pObject && !bFound )
    {
        aObjRect = pObject->GetSnapRect();  //! GetLogicRect ?
        if (aTestRect.IsInside(aObjRect.TopLeft()) || aTestRect.IsInside(aObjRect.BottomLeft()))
            bFound = TRUE;

        pObject = aIter.Next();
    }

    return bFound;
}

void ScDrawLayer::DeleteObjects( SCTAB nTab )
{
    SdrPage* pPage = GetPage(static_cast<sal_uInt16>(nTab));
    DBG_ASSERT(pPage,"Page ?");
    if (!pPage)
        return;

    pPage->RecalcObjOrdNums();

    long    nDelCount = 0;
    ULONG   nObjCount = pPage->GetObjCount();
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
                AddCalcUndo( new SdrUndoRemoveObj( *ppObj[nDelCount-i] ) );

        for (i=1; i<=nDelCount; i++)
            pPage->RemoveObject( ppObj[nDelCount-i]->GetOrdNum() );

        delete[] ppObj;
    }
}

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

    pPage->RecalcObjOrdNums();

    long    nDelCount = 0;
    ULONG   nObjCount = pPage->GetObjCount();
    if (nObjCount)
    {
        Rectangle aDelRect = pDoc->GetMMRect( nCol1, nRow1, nCol2, nRow2, nTab );

        SdrObject** ppObj = new SdrObject*[nObjCount];

        SdrObjListIter aIter( *pPage, IM_FLAT );
        SdrObject* pObject = aIter.Next();
        while (pObject)
        {
            Rectangle aObjRect = pObject->GetCurrentBoundRect();
            if ( aDelRect.IsInside( aObjRect ) )
                ppObj[nDelCount++] = pObject;

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
                pPage->RecalcObjOrdNums();
                long    nDelCount = 0;
                ULONG   nObjCount = pPage->GetObjCount();
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
                        Rectangle aObjRect = pObject->GetCurrentBoundRect();
                        if ( aMarkBound.IsInside( aObjRect ) )
                        {
                            ScRange aRange = pDoc->GetRange( nTab, aObjRect );
                            if (rMark.IsAllMarked(aRange))
                                ppObj[nDelCount++] = pObject;
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
            Rectangle aObjRect = pOldObject->GetCurrentBoundRect();
            if ( rRange.IsInside( aObjRect ) && pOldObject->GetLayer() != SC_LAYER_INTERN )
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
                    // #116235#
                    SdrObject* pNewObject = pOldObject->Clone();
                    //SdrObject* pNewObject = pOldObject->Clone( pDestPage, pDestModel );
                    pNewObject->SetModel(pDestModel);
                    pNewObject->SetPage(pDestPage);

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

BOOL lcl_IsAllInRange( const ScRangeList& rRanges, const ScRange& rClipRange )
{
    //  check if every range of rRanges is completely in rClipRange

    ULONG nCount = rRanges.Count();
    for (ULONG i=0; i<nCount; i++)
    {
        ScRange aRange = *rRanges.GetObject(i);
        if ( !rClipRange.In( aRange ) )
        {
            return FALSE;   // at least one range is not valid
        }
    }

    return TRUE;            // everything is fine
}

BOOL lcl_MoveRanges( ScRangeList& rRanges, const ScRange& rSourceRange, const ScAddress& rDestPos )
{
    BOOL bChanged = FALSE;

    ULONG nCount = rRanges.Count();
    for (ULONG i=0; i<nCount; i++)
    {
        ScRange* pRange = rRanges.GetObject(i);
        if ( rSourceRange.In( *pRange ) )
        {
            SCsCOL nDiffX = rDestPos.Col() - (SCsCOL)rSourceRange.aStart.Col();
            SCsROW nDiffY = rDestPos.Row() - (SCsROW)rSourceRange.aStart.Row();
            SCsTAB nDiffZ = rDestPos.Tab() - (SCsTAB)rSourceRange.aStart.Tab();
            pRange->Move( nDiffX, nDiffY, nDiffZ );
            bChanged = TRUE;
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

    BOOL bMirrorObj = ( rSourceRange.Left() < 0 && rSourceRange.Right() < 0 &&
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
    BOOL bResize = FALSE;
    // sizes can differ by 1 from twips->1/100mm conversion for equal cell sizes,
    // don't resize to empty size when pasting into hidden columns or rows
    if ( Abs(nWidthDiff) > 1 && nDestWidth > 1 && nSourceWidth > 1 )
    {
        aHorFract = Fraction( nDestWidth, nSourceWidth );
        bResize = TRUE;
    }
    if ( Abs(nHeightDiff) > 1 && nDestHeight > 1 && nSourceHeight > 1 )
    {
        aVerFract = Fraction( nDestHeight, nSourceHeight );
        bResize = TRUE;
    }
    Point aRefPos = rDestRange.TopLeft();       // for resizing (after moving)

    SdrObjListIter aIter( *pSrcPage, IM_FLAT );
    SdrObject* pOldObject = aIter.Next();
    while (pOldObject)
    {
        Rectangle aObjRect = pOldObject->GetCurrentBoundRect();
        if ( rSourceRange.IsInside( aObjRect ) )
        {
            // #116235#
            SdrObject* pNewObject = pOldObject->Clone();
            //SdrObject* pNewObject = pOldObject->Clone( pDestPage, this );
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

            //  handle chart data references (after InsertObject)

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
                    String aNewName = ((SdrOle2Obj*)pNewObject)->GetPersistName();

                    //! need to set new DataProvider, or does Chart handle this itself?

                    ScRangeListRef xRanges( new ScRangeList );
                    BOOL bColHeaders = FALSE;
                    BOOL bRowHeaders = FALSE;
                    pDoc->GetOldChartParameters( aNewName, *xRanges, bColHeaders, bRowHeaders );

                    if ( xRanges->Count() > 0 )
                    {
                        ScDocument* pClipDoc = pClipModel->GetDocument();

                        //  a clipboard document and its source share the same document item pool,
                        //  so the pointers can be compared to see if this is copy&paste within
                        //  the same document
                        BOOL bSameDoc = pDoc && pClipDoc && pDoc->GetPool() == pClipDoc->GetPool();

                        BOOL bDestClip = pDoc && pDoc->IsClipboard();

                        BOOL bInSourceRange = FALSE;
                        ScRange aClipRange;
                        if ( pClipDoc )
                        {
                            SCCOL nClipStartX;
                            SCROW nClipStartY;
                            SCCOL nClipEndX;
                            SCROW nClipEndY;
                            pClipDoc->GetClipStart( nClipStartX, nClipStartY );
                            pClipDoc->GetClipArea( nClipEndX, nClipEndY, TRUE );
                            nClipEndX = nClipEndX + nClipStartX;
                            nClipEndY += nClipStartY;   // GetClipArea returns the difference

                            aClipRange = ScRange( nClipStartX, nClipStartY, nSourceTab,
                                                    nClipEndX, nClipEndY, nSourceTab );

                            bInSourceRange = lcl_IsAllInRange( *xRanges, aClipRange );
                        }

                        // always lose references when pasting into a clipboard document (transpose)
                        if ( ( bInSourceRange || bSameDoc ) && !bDestClip )
                        {
                            if ( bInSourceRange )
                            {
                                if ( rDestPos != aClipRange.aStart )
                                {
                                    //  update the data ranges to the new (copied) position
                                    ScRangeListRef xNewRanges = new ScRangeList( *xRanges );
                                    if ( lcl_MoveRanges( *xNewRanges, aClipRange, rDestPos ) )
                                    {
                                        pDoc->UpdateChartArea( aNewName, xNewRanges, bColHeaders, bRowHeaders, FALSE );
                                    }
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
                            //  -> break connection to source data

                            //  (see ScDocument::UpdateChartListenerCollection, PastingDrawFromOtherDoc)

                            //! need chart interface to switch to own data
                        }
                    }
                }
            }
        }

        pOldObject = aIter.Next();
    }
}

void ScDrawLayer::MirrorRTL( SdrObject* pObj )
{
    UINT16 nIdent = pObj->GetObjIdentifier();

    //  don't mirror OLE or graphics, otherwise ask the object
    //  if it can be mirrored
    BOOL bCanMirror = ( nIdent != OBJ_GRAF && nIdent != OBJ_OLE2 );
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

// static
void ScDrawLayer::MirrorRectRTL( Rectangle& rRect )
{
    //  mirror and swap left/right
    long nTemp = rRect.Left();
    rRect.Left() = -rRect.Right();
    rRect.Right() = -nTemp;
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
    //  TRUE if rName is the object's Name or PersistName
    //  (used to find a named object)

    return ( pObj->GetName() == rName ||
            ( pObj->GetObjIdentifier() == OBJ_OLE2 &&
              static_cast<SdrOle2Obj*>(pObj)->GetPersistName() == rName ) );
}

SdrObject* ScDrawLayer::GetNamedObject( const String& rName, USHORT nId, SCTAB& rFoundTab ) const
{
    sal_uInt16 nTabCount = GetPageCount();
    for (sal_uInt16 nTab=0; nTab<nTabCount; nTab++)
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

    BOOL bThere = TRUE;
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

ULONG ScDrawLayer::GetDefTextHeight() const
{
    return nDefTextHgt;     // protected in SdrModel
}

void ScDrawLayer::SetAnchor( SdrObject* pObj, ScAnchorType eType )
{
    // Ein an der Seite verankertes Objekt zeichnet sich durch eine Anker-Pos
    // von (0,1) aus. Das ist ein shabby Trick, der aber funktioniert!
    Point aAnchor( 0, eType == SCA_PAGE ? 1 : 0 );
    pObj->SetAnchorPos( aAnchor );
}

ScAnchorType ScDrawLayer::GetAnchor( const SdrObject* pObj )
{
    Point aAnchor( pObj->GetAnchorPos() );
    return ( aAnchor.Y() != 0 ) ? SCA_PAGE : SCA_CELL;
}

ScDrawObjData* ScDrawLayer::GetObjData( SdrObject* pObj, BOOL bCreate )     // static
{
    USHORT nCount = pObj->GetUserDataCount();
    for( USHORT i = 0; i < nCount; i++ )
    {
        SdrObjUserData* pData = pObj->GetUserData( i );
        if( pData && pData->GetInventor() == SC_DRAWLAYER
                    && pData->GetId() == SC_UD_OBJDATA )
            return (ScDrawObjData*) pData;
    }
    if( bCreate )
    {
        ScDrawObjData* pData = new ScDrawObjData;
        pObj->InsertUserData( pData, 0 );
        return pData;
    }
    return NULL;
}

ScDrawObjData* ScDrawLayer::GetObjDataTab( SdrObject* pObj, SCTAB nTab )    // static
{
    ScDrawObjData* pData = GetObjData( pObj );
    if ( pData )
    {
        if ( pData->bValidStart )
            pData->aStt.SetTab( nTab );
        if ( pData->bValidEnd )
            pData->aEnd.SetTab( nTab );
    }
    return pData;
}

ScIMapInfo* ScDrawLayer::GetIMapInfo( SdrObject* pObj )             // static
{
    USHORT nCount = pObj->GetUserDataCount();
    for( USHORT i = 0; i < nCount; i++ )
    {
        SdrObjUserData* pData = pObj->GetUserData( i );
        if( pData && pData->GetInventor() == SC_DRAWLAYER
                    && pData->GetId() == SC_UD_IMAPDATA )
            return (ScIMapInfo*) pData;
    }
    return NULL;
}

// static:
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
        BOOL        bObjSupported = FALSE;

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

            bObjSupported = TRUE;
        }
        else if ( pObj->ISA( SdrOle2Obj ) ) // OLE-Objekt
        {
            // TODO/LEAN: working with visual area needs running state
            aGraphSize = ((SdrOle2Obj*)pObj)->GetOrigObjSize();
            bObjSupported = TRUE;
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

ScMacroInfo* ScDrawLayer::GetMacroInfo( SdrObject* pObj, BOOL bCreate )             // static
{
    USHORT nCount = pObj->GetUserDataCount();
    for( USHORT i = 0; i < nCount; i++ )
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

void ScDrawLayer::SetGlobalDrawPersist(SfxObjectShell* pPersist)            // static
{
    DBG_ASSERT(!pGlobalDrawPersist,"SetGlobalDrawPersist mehrfach");
    pGlobalDrawPersist = pPersist;
}

void __EXPORT ScDrawLayer::SetChanged( sal_Bool bFlg /* = sal_True */ )
{
    if ( bFlg && pDoc )
        pDoc->SetChartListenerCollectionNeedsUpdate( TRUE );
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
