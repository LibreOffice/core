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

#include "eetext.hxx"

#include <bf_svx/eeitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/outlobj.hxx>
#include <bf_svx/svdoole2.hxx>
#include <bf_svx/svdograf.hxx>
#include <bf_svx/svdopage.hxx>
#include <bf_svx/pbinitem.hxx>
#include <bf_svx/adjitem.hxx>

#include "sdoutl.hxx"

#include "eetext.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "stlsheet.hxx"
#include "glob.hrc"
#include "glob.hxx"

#include <tools/shl.hxx>
#include "sdmod.hxx"
#include <vcl/svapp.hxx>

namespace binfilter {

#define MAX_PRESOBJ 	5			   // Max. Anzahl Praesentationsobjekte

using namespace ::com::sun::star;

TYPEINIT2( SdPage, FmFormPage, SdrObjUserCall );

/*************************************************************************
|*
|*		Ctor
|*
\************************************************************************/

SdPage::SdPage(SdDrawDocument& rNewDoc, StarBASIC* pBasic, BOOL bMasterPage) :
    FmFormPage(rNewDoc, pBasic, bMasterPage),
    SdrObjUserCall(),
    bSelected(FALSE),
    eFadeSpeed(FADE_SPEED_MEDIUM),
    eFadeEffect(::com::sun::star::presentation::FadeEffect_NONE),
    ePresChange(PRESCHANGE_MANUAL),
    nTime(1),
    bSoundOn(FALSE),
    bExcluded(FALSE),
    eAutoLayout(AUTOLAYOUT_NONE),
    bOwnArrangement(FALSE),
    ePageKind(PK_STANDARD),
    bScaleObjects(TRUE),
    bBackgroundFullSize( FALSE ),
    nPaperBin(PAPERBIN_PRINTER_SETTINGS),
    mpItems(NULL)
{
    aLayoutName  = String(SdResId(STR_LAYOUT_DEFAULT_NAME));
    aLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    aLayoutName += String(SdResId(STR_LAYOUT_OUTLINE));

    eCharSet = gsl_getSystemTextEncoding();

    Size aPageSize(GetSize());

    if (aPageSize.Width() > aPageSize.Height())
    {
        eOrientation = ORIENTATION_LANDSCAPE;
    }
    else
    {
        eOrientation = ORIENTATION_PORTRAIT;
    }
}

SdPage::~SdPage()
{
    EndListenOutlineText();
    if( mpItems )
        delete mpItems;
}

SdrObject* SdPage::GetPresObj(PresObjKind eObjKind, USHORT nIndex)
{
    USHORT nObjFound = 0;          // Index des gewuenschten Objekttypes
    SdrObject* pObj = NULL;
    SdrObject* pObjFound = NULL;
    USHORT nIdx = 0;
    USHORT nCnt = (USHORT) aPresObjList.Count();

    while (nIdx < nCnt && nObjFound != nIndex)
    {
        pObj = (SdrObject*) aPresObjList.GetObject(nIdx);

        if (pObj)
        {
            if (eObjKind == GetPresObjKind(pObj) )
                nObjFound++;	// Uebereinstimmendes Objekt gefunden
        }

        nIdx++;
    }

    if (nObjFound == nIndex)
    {
        pObjFound = pObj;
    }
    else if (eObjKind==PRESOBJ_TITLE || eObjKind==PRESOBJ_OUTLINE)
    {
        nObjFound = 0;
        nIdx = 0;
        nCnt = (USHORT) GetObjCount();

        while (nIdx < nCnt && nObjFound != nIndex)
        {
            pObj = GetObj(nIdx);
            SdrObjKind eSdrObjKind = (SdrObjKind) pObj->GetObjIdentifier();

            if (pObj->GetObjInventor() == SdrInventor &&
                (eObjKind==PRESOBJ_TITLE   && eSdrObjKind == OBJ_TITLETEXT ||
                 eObjKind==PRESOBJ_OUTLINE && eSdrObjKind == OBJ_OUTLINETEXT))
            {
                nObjFound++;	// Uebereinstimmendes Objekt gefunden
            }

            nIdx++;
        }

        if (nObjFound == nIndex)
        {
            // Gewuenschtes Objekt auf der Seite gefunden
            pObjFound = pObj;
        }
    }

    return(pObjFound);
}

SdrObject* SdPage::CreatePresObj(PresObjKind eObjKind, BOOL bVertical, const Rectangle& rRect, BOOL bInsert)
{
    SdrObject* pSdrObj = NULL;

    if (eObjKind == PRESOBJ_TITLE)
     {
         pSdrObj = new SdrRectObj(OBJ_TITLETEXT);
 
         if (bMaster)
         {
             pSdrObj->SetNotVisibleAsMaster(TRUE);
         }
     }
     else if (eObjKind == PRESOBJ_OUTLINE)
     {
         pSdrObj = new SdrRectObj(OBJ_OUTLINETEXT);
 
         if (bMaster)
         {
             pSdrObj->SetNotVisibleAsMaster(TRUE);
         }
     }
     else if (eObjKind == PRESOBJ_NOTES)
     {
         pSdrObj = new SdrRectObj(OBJ_TEXT);
 
         if (bMaster)
         {
             pSdrObj->SetNotVisibleAsMaster(TRUE);
         }
     }
     else if (eObjKind == PRESOBJ_TEXT)
     {
        pSdrObj = new SdrRectObj(OBJ_TEXT);
    }
     else if (eObjKind == PRESOBJ_GRAPHIC)
     {
        pSdrObj = new SdrGrafObj();
    }
     else if (eObjKind == PRESOBJ_OBJECT)
     {
        pSdrObj = new SdrOle2Obj();
    }
     else if (eObjKind == PRESOBJ_CHART)
     {
        pSdrObj = new SdrOle2Obj();
        ( (SdrOle2Obj*) pSdrObj)->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarChart" )));
    }
     else if (eObjKind == PRESOBJ_ORGCHART)
     {
        pSdrObj = new SdrOle2Obj();
        ( (SdrOle2Obj*) pSdrObj)->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarOrg" )));
    }
     else if (eObjKind == PRESOBJ_TABLE)
     {
        pSdrObj = new SdrOle2Obj();
         ( (SdrOle2Obj*) pSdrObj)->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarCalc" )));
    }
    else if (eObjKind == PRESOBJ_BACKGROUND)
    {
        pSdrObj = new SdrRectObj();
        pSdrObj->SetMoveProtect(TRUE);
        pSdrObj->SetResizeProtect(TRUE);
        pSdrObj->SetMarkProtect(TRUE);
    }
     else if (eObjKind == PRESOBJ_HANDOUT)
     {
         SdrPage* pPage = ( (SdDrawDocument*) pModel )->GetSdPage(0, PK_STANDARD);
         pSdrObj = new SdrPageObj( pPage->GetPageNum() );
         pSdrObj->SetResizeProtect(TRUE);
     }
     else if (eObjKind == PRESOBJ_PAGE)
     {
         pSdrObj = new SdrPageObj( GetPageNum() - 1 );
         pSdrObj->SetResizeProtect(TRUE);
     }

    if (pSdrObj)
    {
        const bool bIsAlreadyInserted = aPresObjList.GetPos(pSdrObj) != LIST_ENTRY_NOTFOUND;

        if( !bIsAlreadyInserted )
            aPresObjList.Insert(pSdrObj, LIST_APPEND);

        pSdrObj->SetEmptyPresObj(TRUE);
        pSdrObj->SetLogicRect(rRect);	
        
        InsertObject(pSdrObj);

        if( !bIsAlreadyInserted && !bInsert )
            aPresObjList.Remove( pSdrObj );

        if ( pSdrObj->ISA(SdrTextObj) )
        {
            if(bVertical)
                ((SdrTextObj*)pSdrObj)->SetVerticalWriting(TRUE);

            SfxItemSet aTempAttr( ((SdDrawDocument*) pModel)->GetPool() );
            if( bVertical )
                 aTempAttr.Put( SdrTextMinFrameWidthItem( rRect.GetSize().Width() ) );
            else
                aTempAttr.Put( SdrTextMinFrameHeightItem( rRect.GetSize().Height() ) );

            if (bMaster)
            {
                if(bVertical)
                    aTempAttr.Put(SdrTextAutoGrowWidthItem(FALSE));
                else
                    aTempAttr.Put(SdrTextAutoGrowHeightItem(FALSE));
            }

            pSdrObj->SetItemSet(aTempAttr);
        }

        String aString = GetPresObjText(eObjKind);
        if ( aString.Len() && pSdrObj->ISA(SdrTextObj) )
        {
             SdrOutliner* pOutliner = ( (SdDrawDocument*) GetModel() )->GetInternalOutliner();
 
             USHORT nOutlMode = pOutliner->GetMode();
             pOutliner->Init( OUTLINERMODE_TEXTOBJECT );
             pOutliner->SetMinDepth(0);
             pOutliner->SetStyleSheet( 0, NULL );
             pOutliner->SetVertical( bVertical );
 
             String aEmptyStr;
             SetObjText( (SdrTextObj*) pSdrObj, (SdrOutliner*)pOutliner, eObjKind, aString );
 
             pOutliner->Init( nOutlMode );
             pOutliner->SetStyleSheet( 0, NULL );
        }

        pSdrObj->SetUserCall(this);
        pSdrObj->RecalcBoundRect();

        if (bMaster)
        {
            SdrLayerAdmin& rLayerAdmin = pModel->GetLayerAdmin();

            if (eObjKind == PRESOBJ_BACKGROUND)
            {
                pSdrObj->SetLayer( rLayerAdmin.GetLayerID(String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BCKGRND" )), FALSE) );
            }
            else
            {
                pSdrObj->SetLayer( rLayerAdmin.GetLayerID(String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BACKGRNDOBJ" )), FALSE) );
            }
        }

        SfxStyleSheet* pSheetForPresObj = GetStyleSheetForPresObj(eObjKind);
        if(pSheetForPresObj)
            pSdrObj->SetStyleSheet(pSheetForPresObj, FALSE);

        if (eObjKind == PRESOBJ_OUTLINE)
        {
            for (USHORT nLevel = 1; nLevel < 10; nLevel++)
            {
                String aName(aLayoutName);
                aName += sal_Unicode( ' ' );
                aName += String::CreateFromInt32( nLevel );
                SfxStyleSheet* pSheet = (SfxStyleSheet*)pModel->GetStyleSheetPool()->
                            Find(aName, SD_LT_FAMILY);
                DBG_ASSERT(pSheet, "Vorlage fuer Gliederungsobjekt nicht gefunden");
                if (pSheet)
                    pSdrObj->StartListening(*pSheet);
            }
        }

        if( eObjKind == PRESOBJ_OBJECT || eObjKind == PRESOBJ_CHART || eObjKind == PRESOBJ_ORGCHART || eObjKind == PRESOBJ_TABLE || eObjKind == PRESOBJ_GRAPHIC )
        {
            SfxItemSet aSet( ((SdDrawDocument*) pModel)->GetPool() );
            aSet.Put( SdrTextContourFrameItem( TRUE ) );
            aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER ) );
            pSdrObj->SetItemSet(aSet);
        }
    }

    return(pSdrObj);
}

SfxStyleSheet* SdPage::GetStyleSheetForPresObj(PresObjKind eObjKind)
{
    String aName(GetLayoutName());
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    USHORT nPos = aName.Search(aSep);
    if (nPos != STRING_NOTFOUND)
    {
        nPos += aSep.Len();
        aName.Erase(nPos);
    }

    switch (eObjKind)
    {
        case PRESOBJ_OUTLINE:
        {
            aName = GetLayoutName();
            aName += sal_Unicode( ' ' );
            aName += String::CreateFromInt32( 1 );
        }
        break;

        case PRESOBJ_TITLE:
            aName += String(SdResId(STR_LAYOUT_TITLE));
            break;

        case PRESOBJ_BACKGROUND:
            aName += String(SdResId(STR_LAYOUT_BACKGROUND));
            break;

        case PRESOBJ_NOTES:
            aName += String(SdResId(STR_LAYOUT_NOTES));
            break;

        case PRESOBJ_TEXT:
        {
            aName += String(SdResId(STR_LAYOUT_SUBTITLE));
        }
        break;

        default:
            break;
    }

    SfxStyleSheetBasePool* pStShPool = pModel->GetStyleSheetPool();
    SfxStyleSheetBase*	   pResult	 = pStShPool->Find(aName, SD_LT_FAMILY);
    return (SfxStyleSheet*)pResult;
}

void SdPage::Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect)
{
}

void SdPage::CreateTitleAndLayout(BOOL bInit, BOOL bAPICall )
{
    SdPage* pMasterPage = this;

    if (!bMaster)
    {
        pMasterPage = (SdPage*) GetMasterPage(0);
    }

    if (!pMasterPage)
    {
        return;
    }

    SdrObject* pMasterTitle = NULL;
    SdrObject* pMasterOutline = NULL;
    SdrObject* pMasterBackground = NULL;


    ULONG nMasterIndex = 0;
    ULONG nMasterCount = pMasterPage->GetPresObjList()->Count();
    SdrObject* pMasterObj = NULL;

    for (nMasterIndex = 0; nMasterIndex < nMasterCount; nMasterIndex++)
    {
         pMasterObj = (SdrObject*) pMasterPage->GetPresObjList()->GetObject(nMasterIndex);
 
         if (pMasterObj && pMasterObj->GetObjInventor() == SdrInventor)
         {
             UINT16 nId = pMasterObj->GetObjIdentifier();
 
             if (nId == OBJ_TITLETEXT)
             {
                 pMasterTitle = pMasterObj;
             }
             else if ((ePageKind!=PK_NOTES && nId == OBJ_OUTLINETEXT) ||
                      (ePageKind==PK_NOTES && nId == OBJ_TEXT) )
             {
                 pMasterOutline = pMasterObj;
             }
             else if (nId == OBJ_RECT && pMasterObj->IsEmptyPresObj() )
             {
                 pMasterBackground = pMasterObj;
             }
         }
    }

    if (!pMasterBackground && ePageKind == PK_STANDARD)
    {
        Point aBackgroundPos ( GetLftBorder(), GetUppBorder() );
        Size aBackgroundSize ( GetSize() );
        aBackgroundSize.Width()  -= GetLftBorder() + GetRgtBorder() - 1;
        aBackgroundSize.Height() -= GetUppBorder() + GetLwrBorder() - 1;
        Rectangle aBackgroundRect (aBackgroundPos, aBackgroundSize);
        pMasterPage->CreatePresObj(PRESOBJ_BACKGROUND, FALSE, aBackgroundRect, TRUE);
    }

    BOOL bDeletePresObjOnMaster = FALSE;

    if ((eAutoLayout == AUTOLAYOUT_NONE) && !bAPICall)
    {
        bDeletePresObjOnMaster = TRUE;
        USHORT nPgCount = ((SdDrawDocument*) pModel)->GetSdPageCount(ePageKind);

        for (USHORT i = 0; i < nPgCount && bDeletePresObjOnMaster; i++)
        {
            SdPage* pPage = ((SdDrawDocument*) pModel)->GetSdPage(i, ePageKind);

            if (pPage                                  &&
                pPage->GetMasterPage(0) == pMasterPage &&
                pPage->GetAutoLayout() != AUTOLAYOUT_NONE)
            {
                // Seite referenziert die aktuelle MasterPage
                // und hat ein gueltiges (kein leeres!)  AutoLayout
                bDeletePresObjOnMaster = FALSE;
            }
        }
    }

    if (bDeletePresObjOnMaster && !bMaster)
    {
        // Kein AutoLayout auf der MasterPage erforderlich
        // (Hintergrundobjekt muss jedoch erzeugt werden)

        List* pMasterPresObjList = pMasterPage->GetPresObjList();

        if (pMasterTitle)
        {
            pMasterPresObjList->Remove(pMasterTitle);
            delete pMasterPage->RemoveObject(pMasterTitle->GetOrdNum());
        }

        if (pMasterOutline)
        {
            pMasterPresObjList->Remove(pMasterOutline);
            delete pMasterPage->RemoveObject(pMasterOutline->GetOrdNum());
        }

        return;
    }

     if ( ePageKind == PK_HANDOUT && bInit )
     {
        /******************************************************************
        * Handzettel-Seite
        ******************************************************************/
 
         // alle bisherigen Praesentations-Objekte loeschen
         List* pPresObjList = pMasterPage->GetPresObjList();
 
         for (ULONG i = 0; i < pPresObjList->Count(); i++)
         {
             SdrObject* pObj = (SdrObject*) pPresObjList->GetObject(i);
             delete pMasterPage->RemoveObject(pObj->GetOrdNum());
         }
 
         pMasterPage->GetPresObjList()->Clear();
 
         Size    aArea = GetSize();
         long    nX = GetLftBorder();
         long    nY = GetUppBorder();
         long    nGapW = (nX + GetRgtBorder()) / 2;
         long    nGapH = (nY + GetLwrBorder()) / 2;
         USHORT  nColCnt, nRowCnt;
 
         if ( !nGapW )
        {
            nGapW = aArea.Width() / 10;
            nX = nGapW;
        }
        if ( !nGapH )
        {
            nGapH = aArea.Height() / 10;
            nY = nGapH;
         }
 
         switch ( pMasterPage->GetAutoLayout() )
         {
             case AUTOLAYOUT_HANDOUT1: nColCnt = 1; nRowCnt = 1; break;
             case AUTOLAYOUT_HANDOUT2: nColCnt = 1; nRowCnt = 2; break;
             case AUTOLAYOUT_HANDOUT3: nColCnt = 1; nRowCnt = 3; break;
             case AUTOLAYOUT_HANDOUT4: nColCnt = 2; nRowCnt = 2; break;
             case AUTOLAYOUT_HANDOUT6: nColCnt = 2; nRowCnt = 3; break;
             default: nColCnt = 1; nRowCnt = 1; break;
         }
         aArea.Width() -= nGapW * 2;
         aArea.Height() -= nGapH * 2;
 
         if ( nGapW < aArea.Width() / 10 )
             nGapW = aArea.Width() / 10;
         if ( nGapH < aArea.Height() / 10 )
             nGapH = aArea.Height() / 10;
 
         // bei Querformat Reihen und Spalten vertauschen
         if ( aArea.Width() > aArea.Height() )
         {
             USHORT nTmp = nRowCnt;
             nRowCnt = nColCnt;
             nColCnt = nTmp;
         }
 
         Size aPartArea, aSize;
         aPartArea.Width()  = ((aArea.Width()  + nGapW) / nColCnt) - nGapW;
         aPartArea.Height() = ((aArea.Height() + nGapH) / nRowCnt) - nGapH;
 
         SdrPage* pPage = ((SdDrawDocument*) pModel)->
                             GetSdPage(0, PK_STANDARD);
         if ( pPage )
         {   // tatsaechliche Seitengroesse in das Handout-Rechteck skalieren
             double fH = (double) aPartArea.Width()  / pPage->GetWdt();
             double fV = (double) aPartArea.Height() / pPage->GetHgt();
 
             if ( fH > fV )
                 fH = fV;
             aSize.Width()  = (long) (fH * pPage->GetWdt());
             aSize.Height() = (long) (fH * pPage->GetHgt());
 
             nX += (aPartArea.Width() - aSize.Width()) / 2;
             nY += (aPartArea.Height()- aSize.Height())/ 2;
         }
         else
             aSize = aPartArea;
 
         Point aPos(nX, nY);
         USHORT nPgNum = 0;
 
         for (USHORT nRow = 0; nRow < nRowCnt; nRow++)
         {
             aPos.X() = nX;
 
             for (USHORT nCol = 0; nCol < nColCnt; nCol++)
             {
                 Rectangle aRect(aPos, aSize);
                 SdrPageObj* pPageObj = (SdrPageObj*) pMasterPage->
                 CreatePresObj(PRESOBJ_HANDOUT, FALSE, aRect, TRUE);
 
                 pPageObj->SetPageNum( 2 * nPgNum + 1);
 
                 nPgNum++;
                 aPos.X() += aPartArea.Width() + nGapW;
             }
             aPos.Y() += aPartArea.Height() + nGapH;
         }
     }
 
     if ( !bDeletePresObjOnMaster )
     {
         if (!pMasterTitle && ePageKind != PK_HANDOUT)
         {
             Rectangle aTitleRect = GetTitleRect();
             pMasterPage->CreatePresObj(PRESOBJ_TITLE, FALSE, aTitleRect, TRUE);
         }
 
         if (!pMasterOutline  && ePageKind != PK_HANDOUT)
         {
             Rectangle aLayoutRect = GetLayoutRect();
 
             if (ePageKind == PK_STANDARD)
             {
                 pMasterPage->CreatePresObj(PRESOBJ_OUTLINE, FALSE, aLayoutRect, TRUE);
             }
             else if (ePageKind == PK_NOTES)
             {
                 pMasterPage->CreatePresObj(PRESOBJ_NOTES, FALSE, aLayoutRect, TRUE);
             }
         }
     }
 }

Rectangle SdPage::GetTitleRect() const
{
    Rectangle aTitleRect;

    if (ePageKind != PK_HANDOUT)
    {
        Point aTitlePos ( GetLftBorder(), GetUppBorder() );
        Size aTitleSize ( GetSize() );
        aTitleSize.Width()  -= GetLftBorder() + GetRgtBorder();
        aTitleSize.Height() -= GetUppBorder() + GetLwrBorder();

        if (ePageKind == PK_STANDARD)
        {
            aTitlePos.X() += long( aTitleSize.Width() * 0.0735 );
            aTitlePos.Y() += long( aTitleSize.Height() * 0.083 );
            aTitleSize.Width() = long( aTitleSize.Width() * 0.854 );
            aTitleSize.Height() = long( aTitleSize.Height() * 0.167 );
        }
        else if (ePageKind == PK_NOTES)
        {
            aTitleSize.Height() = (long) (aTitleSize.Height() / 2.5);

            Point aPos = aTitlePos;
            aPos.Y() += long( aTitleSize.Height() * 0.083 );
            Size aPartArea = aTitleSize;
            Size aSize;

            SdrPage* pPage = pModel->GetPage( GetPageNum() - 1 );

            if ( pPage )
            {
                double fH = (double) aPartArea.Width()  / pPage->GetWdt();
                double fV = (double) aPartArea.Height() / pPage->GetHgt();

                if ( fH > fV )
                    fH = fV;
                aSize.Width()  = (long) (fH * pPage->GetWdt());
                aSize.Height() = (long) (fH * pPage->GetHgt());

                aPos.X() += (aPartArea.Width() - aSize.Width()) / 2;
                aPos.Y() += (aPartArea.Height()- aSize.Height())/ 2;
            }

            aTitlePos = aPos;
            aTitleSize = aSize;
        }

        aTitleRect.SetPos(aTitlePos);
        aTitleRect.SetSize(aTitleSize);
    }

    return aTitleRect;
}

Rectangle SdPage::GetLayoutRect() const
{
    Rectangle aLayoutRect;

    if (ePageKind != PK_HANDOUT)
    {
        Point aLayoutPos ( GetLftBorder(), GetUppBorder() );
        Size aLayoutSize ( GetSize() );
        aLayoutSize.Width()  -= GetLftBorder() + GetRgtBorder();
        aLayoutSize.Height() -= GetUppBorder() + GetLwrBorder();

        if (ePageKind == PK_STANDARD)
        {
            aLayoutPos.X() += long( aLayoutSize.Width() * 0.0735 );
            aLayoutPos.Y() += long( aLayoutSize.Height() * 0.278 );
            aLayoutSize.Width() = long( aLayoutSize.Width() * 0.854 );
            aLayoutSize.Height() = long( aLayoutSize.Height() * 0.630 );
            aLayoutRect.SetPos(aLayoutPos);
            aLayoutRect.SetSize(aLayoutSize);
        }
        else if (ePageKind == PK_NOTES)
        {
            aLayoutPos.X() += long( aLayoutSize.Width() * 0.0735 );
            aLayoutPos.Y() += long( aLayoutSize.Height() * 0.472 );
            aLayoutSize.Width() = long( aLayoutSize.Width() * 0.854 );
            aLayoutSize.Height() = long( aLayoutSize.Height() * 0.444 );
            aLayoutRect.SetPos(aLayoutPos);
            aLayoutRect.SetSize(aLayoutSize);
        }
    }

    return aLayoutRect;
}

void SdPage::SetAutoLayout(AutoLayout eLayout, BOOL bInit, BOOL bAPICall )
{
    eAutoLayout = eLayout;
    bOwnArrangement = TRUE;

    CreateTitleAndLayout(bInit, bAPICall);

    if ((eAutoLayout == AUTOLAYOUT_NONE && aPresObjList.Count() == 0) || bMaster)
    {
        bOwnArrangement = FALSE;
        return;
    }

    USHORT nIndex = 0;
    ULONG nCount = aPresObjList.Count();
    SdrObject* pObj = NULL;

    for (nIndex = 0; nIndex < nCount; nIndex++)
    {
        pObj = (SdrObject*) aPresObjList.GetObject(nIndex);

        if ( !pObj || pObj->GetPage() != this )
        {
            aPresObjList.Remove( pObj );
        }
    }

    ULONG nMasterIndex = 0;
    SdPage* pMasterPage = (SdPage*) GetMasterPage(0);
    List* pList = pMasterPage->GetPresObjList();
    ULONG nMasterCount = pList->Count();
    SdrObject* pMasterObj = NULL;

    Rectangle aTitleRect;
    Rectangle aLayoutRect;
    BOOL bFound = FALSE;

    for (nMasterIndex = 0; nMasterIndex < nMasterCount; nMasterIndex++)
    {
        bFound = FALSE;
        pMasterObj = (SdrObject*) pList->GetObject(nMasterIndex);

        if (pMasterObj && pMasterObj->GetObjInventor() == SdrInventor)
        {
            UINT16 nId = pMasterObj->GetObjIdentifier();

            if (nId == OBJ_TITLETEXT )
            {
                aTitleRect = pMasterObj->GetLogicRect();
            }

            if ((ePageKind!=PK_NOTES && nId == OBJ_OUTLINETEXT) ||
                (ePageKind==PK_NOTES && nId == OBJ_TEXT))
            {
                aLayoutRect = pMasterObj->GetLogicRect();
            }
        }
    }

    if (aTitleRect.IsEmpty() && ePageKind != PK_HANDOUT)
    {
        aTitleRect = GetTitleRect();
    }

    if (aLayoutRect.IsEmpty() && ePageKind != PK_HANDOUT)
    {
        aLayoutRect = GetLayoutRect();
    }

    Rectangle   aRect0( aTitleRect );
    Rectangle   aRect1( aLayoutRect );
    Rectangle   aRect2( aLayoutRect );
    Rectangle   aRect3( aLayoutRect );
    Rectangle   aRect4( aLayoutRect );
    Size        aTitleSize( aTitleRect.GetSize() );
    Point       aTitlePos( aTitleRect.TopLeft() );
    Size        aLayoutSize( aLayoutRect.GetSize() );
    Point       aLayoutPos( aLayoutRect.TopLeft() );
    Size        aTempSize;
    Point       aTempPnt;
    PresObjKind nObjKind[ MAX_PRESOBJ ];
    List        aObjList;
    sal_Bool    bRightToLeft = ( GetModel() && static_cast< SdDrawDocument* >( GetModel() )->GetDefaultWritingMode() == ::com::sun::star::text::WritingMode_RL_TB );

    for (nIndex = 0; nIndex < MAX_PRESOBJ; nIndex++)
    {
        nObjKind[nIndex] = PRESOBJ_NONE;
    }


    switch (eAutoLayout)
    {
        case AUTOLAYOUT_NONE:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);

            if ( pObj && pObj->GetUserCall() )
            {
                pObj->SetLogicRect(aRect0);
                pObj->SetUserCall(this);
            }

            if (pObj && !pObj->IsEmptyPresObj() || !bInit)
                aObjList.Insert(pObj, LIST_APPEND);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);

            if ( pObj && pObj->GetUserCall() )
            {
                pObj->SetLogicRect(aRect1);
                pObj->SetUserCall(this);
            }

            if (pObj && !pObj->IsEmptyPresObj() || !bInit)
                aObjList.Insert(pObj, LIST_APPEND);
        }
        break;

        case AUTOLAYOUT_NOTES:
        {
            nObjKind[0] = PRESOBJ_PAGE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_NOTES;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TITLE:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_TEXT;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_ENUM:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_CHART:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_CHART;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_2TEXT:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            if( bRightToLeft )
                ::std::swap< Rectangle >( aRect1, aRect2 );

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            USHORT nIndex = 1;
            if ( InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList) )
                nIndex++;

            nObjKind[2] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[2], nIndex);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TEXTCHART:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            if( bRightToLeft )
                ::std::swap< Rectangle >( aRect1, aRect2 );

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_CHART;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_ORG:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_ORGCHART;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TEXTCLIP:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            if( bRightToLeft )
                ::std::swap< Rectangle >( aRect1, aRect2 );

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_GRAPHIC;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_CHARTTEXT:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            if( bRightToLeft )
                ::std::swap< Rectangle >( aRect1, aRect2 );

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_CHART;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TAB:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_TABLE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_CLIPTEXT:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            if( bRightToLeft )
                ::std::swap< Rectangle >( aRect1, aRect2 );

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_GRAPHIC;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TEXTOBJ:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            if( bRightToLeft )
                ::std::swap< Rectangle >( aRect1, aRect2 );

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_OBJ:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TEXT2OBJ:
        {
            aTempPnt = aLayoutPos;
            aTempSize = aLayoutSize;
            aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
            aLayoutSize.Width() = long (aLayoutSize.Width() * 0.488);
            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
            aRect3 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos = aTempPnt;
            aLayoutSize = aTempSize;
            aLayoutSize.Width() = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            if( bRightToLeft )
            {
                ::std::swap< long >( aRect1.Left(), aRect2.Left() );
                aRect3.Left() = aRect2.Left();
            }

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2]);
            USHORT nIndex = 1;
            if ( InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList) )
                nIndex++;

            nObjKind[3] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[3], nIndex);
            InsertPresObj(pObj, nObjKind[3], FALSE, aRect3, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_OBJTEXT:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            if( bRightToLeft )
                ::std::swap< Rectangle >( aRect1, aRect2 );

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_OBJOVERTEXT:
        {
            aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_2OBJTEXT:
        {
            aTempPnt = aLayoutPos;
            aTempSize = aLayoutSize;
            aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
            aLayoutSize.Width() = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos = aTempPnt;
            aLayoutSize = aTempSize;
            aLayoutSize.Width() = long (aLayoutSize.Width() * 0.488);
            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect3 = Rectangle (aLayoutPos, aLayoutSize);

            if( bRightToLeft )
            {
                ::std::swap< long >( aRect1.Left(), aRect2.Left() );
                aRect3.Left() = aRect2.Left();
            }

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            USHORT nIndex = 1;
            if ( InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList) )
                nIndex++;

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2], nIndex);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);

            nObjKind[3] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[3]);
            InsertPresObj(pObj, nObjKind[3], FALSE, aRect3, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_2OBJOVERTEXT:
        {
            aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
            aLayoutSize.Width() = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aTempPnt = aLayoutPos;
            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = aTempPnt.X();
            aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
            aLayoutSize.Width() = long (aLayoutSize.Width() / 0.488);
            aRect3 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            USHORT nIndex = 1;
            if ( InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList) )
                nIndex++;

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2], nIndex);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);

            nObjKind[3] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[3]);
            InsertPresObj(pObj, nObjKind[3], FALSE, aRect3, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TEXTOVEROBJ:
        {
            aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_4OBJ:
        {
            ULONG nX = long (aLayoutPos.X());
            ULONG nY = long (aLayoutPos.Y());

            aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (nX + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
            aRect3 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = nX;
            aRect4 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            USHORT nIndex = 1;
            if ( InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList) )
                nIndex++;

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2], nIndex);
            if ( InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList) )
                nIndex++;

            nObjKind[3] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[3], nIndex);
            if ( InsertPresObj(pObj, nObjKind[3], FALSE, aRect3, bInit, aObjList) )
                nIndex++;

            nObjKind[4] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[4], nIndex);
            InsertPresObj(pObj, nObjKind[4], FALSE, aRect4, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_ONLY_TITLE:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART:
        {
            Size aSize( aRect0.GetSize().Height(), aRect1.BottomLeft().Y() - aRect0.TopLeft().Y() );
            aRect0.SetSize( aSize );
            aRect0.SetPos( aTitleRect.TopRight() - Point( aSize.Width(), 0 ) );
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], TRUE, aRect0, bInit, aObjList);
            pObj = GetPresObj(nObjKind[0]);
            if ( pObj )
            {
                pObj->SetItem( SdrTextAutoGrowWidthItem(TRUE) );
                pObj->SetItem( SdrTextAutoGrowHeightItem(FALSE) );
            }
            Size aLayoutSize ( GetSize() );
            aLayoutSize.Height() -= GetUppBorder() + GetLwrBorder();
            aSize.Height() = long ( aRect0.GetSize().Height() * 0.47 );
            aSize.Width() = long( aLayoutSize.Width() * 0.7 );
            aRect1.SetPos( aTitleRect.TopLeft() );
            aRect1.SetSize( aSize );
            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], TRUE, aRect1, bInit, aObjList);
            pObj = GetPresObj(nObjKind[1]);
            if ( pObj )
            {
                pObj->SetItem( SdrTextAutoGrowWidthItem(TRUE) );
                pObj->SetItem( SdrTextAutoGrowHeightItem(FALSE) );
                
                pObj->SetItem( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
                pObj->SetItem( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );
            }
            aSize.Height() = aRect0.GetSize().Height();
            Point aPos( aTitleRect.TopLeft() );
            aPos.Y() += long ( aSize.Height() * 0.53 );
            aRect2.SetPos( aPos );
            aSize.Height() = long ( aRect0.GetSize().Height() * 0.47 );
            aRect2.SetSize( aSize );
            nObjKind[2] = PRESOBJ_CHART;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], FALSE, aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE:
        {
            Size aSize( aRect0.GetSize().Height(), aRect1.BottomLeft().Y() - aRect0.TopLeft().Y() );
            aRect0.SetSize( aSize );
            aRect0.SetPos( aTitleRect.TopRight() - Point( aSize.Width(), 0 ) );
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], TRUE, aRect0, bInit, aObjList);
            pObj = GetPresObj(nObjKind[0]);
            if ( pObj )
            {
                pObj->SetItem( SdrTextAutoGrowWidthItem(TRUE) );
                pObj->SetItem( SdrTextAutoGrowHeightItem(FALSE) );
            }
            Size aLayoutSize ( GetSize() );
            aLayoutSize.Height() -= GetUppBorder() + GetLwrBorder();
            aSize.Height() = aRect0.GetSize().Height();
            aSize.Width() = long( aLayoutSize.Width() * 0.7 );
            aRect1.SetPos( aTitleRect.TopLeft() );
            aRect1.SetSize( aSize );
            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], TRUE, aRect1, bInit, aObjList);
            pObj = GetPresObj(nObjKind[1]);
            if ( pObj )
            {
                pObj->SetItem( SdrTextAutoGrowWidthItem(TRUE) );
                pObj->SetItem( SdrTextAutoGrowHeightItem(FALSE) );
                
                // #90790#
                pObj->SetItem( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
                pObj->SetItem( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );
            }
        }
        break;

        case AUTOLAYOUT_TITLE_VERTICAL_OUTLINE:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], TRUE, aRect1, bInit, aObjList);
            pObj = GetPresObj(nObjKind[1]);
            if ( pObj )
            {
                pObj->SetItem( SdrTextAutoGrowWidthItem(TRUE) );
                pObj->SetItem( SdrTextAutoGrowHeightItem(FALSE) );
                
                // #90790#
                pObj->SetItem( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
                pObj->SetItem( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );
            }
        }
        break;

        case AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], FALSE, aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_GRAPHIC;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], FALSE, aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], TRUE, aRect2, bInit, aObjList);
            pObj = GetPresObj(nObjKind[2]);
            if ( pObj )
            {
                pObj->SetItem( SdrTextAutoGrowWidthItem(TRUE) );
                pObj->SetItem( SdrTextAutoGrowHeightItem(FALSE) );

                // #90790#
                pObj->SetItem( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
                pObj->SetItem( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );
            }
        }
        break;

        default:
        break;
    }

    nCount = aPresObjList.Count();

    for (nIndex=0; nIndex<nCount; nIndex++)
    {
        pObj = (SdrObject*) aPresObjList.GetObject(nIndex);

        if (pObj && aObjList.GetPos(pObj) == LIST_ENTRY_NOTFOUND)
        {
            if ( pObj->IsEmptyPresObj() )
            {
                delete RemoveObject( pObj->GetOrdNum() );
            }
        }
    }

    aPresObjList.Clear();
    aPresObjList = aObjList;

    bOwnArrangement = FALSE;
}

void SdPage::NbcInsertObject(SdrObject* pObj, ULONG nPos, const SdrInsertReason* pReason)
{
    FmFormPage::NbcInsertObject(pObj, nPos, pReason);

    SdrLayerID nId = pObj->GetLayer();
    if( bMaster )
    {
        if( nId == 0 )
            pObj->NbcSetLayer( 2 );     // wrong layer. corrected to BackgroundObj layer
    }
    else
    {
        if( nId == 2 )
            pObj->NbcSetLayer( 0 );     // wrong layer. corrected to layout layer
    }
}

SdrObject* SdPage::RemoveObject(ULONG nObjNum)
{
    SdrObject* pObj = FmFormPage::RemoveObject(nObjNum);

    if (pObj && pObj->GetUserCall()!=this &&
        aPresObjList.GetPos(pObj) != LIST_ENTRY_NOTFOUND)
    {
        // Objekt hat keinen UserCall auf diese Seite, es ist jedoch noch in
        // der PresObjList eingetragen -> austragen
        Changed(*pObj, SDRUSERCALL_REMOVED, pObj->GetBoundRect());
    }
    return(pObj);
}

void SdPage::SetSize(const Size& aSize)
{
    Size aOldSize = GetSize();

    if (aSize != aOldSize)
    {
        FmFormPage::SetSize(aSize);
        AdjustBackgroundSize();

        if (aOldSize.Height() == 10 && aOldSize.Width() == 10)
        {
            // Die Seite bekommt erstmalig eine gueltige Groesse gesetzt,
            // daher wird nun die Orientation initialisiert
            if (aSize.Width() > aSize.Height())
            {
                eOrientation = ORIENTATION_LANDSCAPE;
            }
            else
            {
                eOrientation = ORIENTATION_PORTRAIT;
            }
        }
    }
}

void SdPage::SetBorder(INT32 nLft, INT32 nUpp, INT32 nRgt, INT32 nLwr)
{
    if (nLft != GetLftBorder() || nUpp != GetUppBorder() ||
        nRgt != GetRgtBorder() || nLwr != GetLwrBorder() )
    {
        FmFormPage::SetBorder(nLft, nUpp, nRgt, nLwr);
        AdjustBackgroundSize();
    }
}

void SdPage::SetLftBorder(INT32 nBorder)
{
    if (nBorder != GetLftBorder() )
    {
        FmFormPage::SetLftBorder(nBorder);
        AdjustBackgroundSize();
    }
}

void SdPage::SetRgtBorder(INT32 nBorder)
{
    if (nBorder != GetRgtBorder() )
    {
        FmFormPage::SetRgtBorder(nBorder);
        AdjustBackgroundSize();
    }
}

void SdPage::SetUppBorder(INT32 nBorder)
{
    if (nBorder != GetUppBorder() )
    {
        FmFormPage::SetUppBorder(nBorder);
        AdjustBackgroundSize();
    }
}

void SdPage::SetLwrBorder(INT32 nBorder)
{
    if (nBorder != GetLwrBorder() )
    {
        FmFormPage::SetLwrBorder(nBorder);
        AdjustBackgroundSize();
    }
}

void SdPage::SetBackgroundFullSize( BOOL bIn )
{
    if( bIn != bBackgroundFullSize )
    {
        bBackgroundFullSize = bIn;
        AdjustBackgroundSize();
    }
}

BOOL SdPage::InsertPresObj(SdrObject* pObj, PresObjKind eObjKind, BOOL bVertical, Rectangle aRect, BOOL bInit, List& rObjList)
{
    BOOL bIncrement = FALSE;

    if (!pObj && bInit)
    {
        pObj = CreatePresObj(eObjKind, bVertical, aRect);
    }
    else if ( pObj && (pObj->GetUserCall() || bInit) )
    {
        if ( pObj->ISA(SdrGrafObj) && !pObj->IsEmptyPresObj() )
            ( (SdrGrafObj*) pObj)->AdjustToMaxRect( aRect, FALSE );
        else
            pObj->SetLogicRect(aRect);

        pObj->SetUserCall(this);

        if ( pObj->ISA(SdrTextObj) )
        {
            if( ((SdrTextObj*) pObj)->IsVerticalWriting() != bVertical )
            {
                ((SdrTextObj*) pObj)->SetVerticalWriting( bVertical );

                // #94826# here make sure the correct anchoring is used when the object
                // is re-used but orientation is changed
                if(PRESOBJ_OUTLINE == eObjKind)
                {
                    if(bVertical)
                    {
                        // vertical activated on once horizontal outline object
                        ((SdrTextObj*) pObj)->SetItem(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                    }
                    else
                    {
                        // horizontal activated on once vertical outline object
                        ((SdrTextObj*) pObj)->SetItem(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK));
                    }
                }
            }

            if( !bMaster )
            {
                if ( ((SdrTextObj*) pObj)->IsAutoGrowHeight() )
                {
                    // switch off AutoGrowHeight, set new MinHeight
                    SfxItemSet aTempAttr( ((SdDrawDocument*) pModel)->GetPool() );
                    SdrTextMinFrameHeightItem aMinHeight( aRect.GetSize().Height() );
                    aTempAttr.Put( aMinHeight );
                    aTempAttr.Put( SdrTextAutoGrowHeightItem(FALSE) );
                    pObj->SetItemSet(aTempAttr);
                    pObj->SetLogicRect(aRect);

                    // switch on AutoGrowHeight
                    SfxItemSet aAttr( ((SdDrawDocument*) pModel)->GetPool() );
                    aAttr.Put( SdrTextAutoGrowHeightItem(TRUE) );

                    pObj->SetItemSet(aAttr);
                }

                if ( ((SdrTextObj*) pObj)->IsAutoGrowWidth() )
                {
                    // switch off AutoGrowWidth , set new MinWidth
                    SfxItemSet aTempAttr( ((SdDrawDocument*) pModel)->GetPool() );
                    SdrTextMinFrameWidthItem aMinWidth( aRect.GetSize().Width() );
                    aTempAttr.Put( aMinWidth );
                    aTempAttr.Put( SdrTextAutoGrowWidthItem(FALSE) );
                    pObj->SetItemSet(aTempAttr);
                    pObj->SetLogicRect(aRect);

                    // switch on AutoGrowWidth
                    SfxItemSet aAttr( ((SdDrawDocument*) pModel)->GetPool() );
                    aAttr.Put( SdrTextAutoGrowWidthItem(TRUE) );
                    pObj->SetItemSet(aAttr);
                }
            }
        }
    }

    if (pObj)
    {
        bIncrement = TRUE;
        rObjList.Insert(pObj, LIST_APPEND);

        if (eObjKind == PRESOBJ_OUTLINE && pObj->IsEmptyPresObj())
        {
            // Gibt es ev. noch ein Untertitel?
            SdrObject* pSubtitle = GetPresObj(PRESOBJ_TEXT);

            if (pSubtitle)
            {
                // Text des Untertitels in das PRESOBJ_OUTLINE setzen
                OutlinerParaObject* pOutlParaObj = pSubtitle->GetOutlinerParaObject();

                if (pOutlParaObj)
                {
                    if (!pSubtitle->IsEmptyPresObj())
                    {
                        // Text umsetzen
                        SdOutliner* pOutl = ( (SdDrawDocument*) pModel )->GetInternalOutliner( TRUE );
                        pOutl->Clear();
                        pOutl->SetText( *pOutlParaObj );
                        pOutl->SetMinDepth(1, TRUE);
                        pOutlParaObj = pOutl->CreateParaObject();
                        pObj->SetOutlinerParaObject( pOutlParaObj );
                        pOutl->Clear();
                        pObj->SetEmptyPresObj(FALSE);
                    }

                    for (USHORT nLevel = 1; nLevel < 10; nLevel++)
                    {
                        // Neue Vorlage zuweisen
                        String aName(aLayoutName);
                        aName += sal_Unicode( ' ' );
                        aName += String::CreateFromInt32( nLevel );
                        SfxStyleSheet* pSheet = (SfxStyleSheet*) pModel->GetStyleSheetPool()->Find(aName, SD_LT_FAMILY);

                        if (pSheet)
                        {
                            if (nLevel == 1)
                            {
                                SfxStyleSheet* pSubtitleSheet = GetStyleSheetForPresObj(PRESOBJ_TEXT);

                                if (pSubtitleSheet)
                                    pOutlParaObj->ChangeStyleSheetName(SD_LT_FAMILY, pSubtitleSheet->GetName(),
                                                                                     pSheet->GetName());
                            }

                            pObj->StartListening(*pSheet);
                        }
                    }

                    // LRSpace-Item loeschen
                    SfxItemSet aSet(((SdDrawDocument*) pModel)->GetPool(), EE_PARA_LRSPACE, EE_PARA_LRSPACE );

                    aSet.Put(pObj->GetItemSet());

                    aSet.ClearItem(EE_PARA_LRSPACE);

                    pObj->SetItemSet(aSet);

                    // Untertitel loeschen
                    aPresObjList.Remove(pSubtitle);
                    RemoveObject( pObj->GetOrdNum() );
                    ReplaceObject( pObj, pSubtitle->GetOrdNum() );
                    delete pSubtitle;
                }
            }
        }
        else if (eObjKind == PRESOBJ_TEXT && pObj->IsEmptyPresObj())
        {
            // Gibt es ev. noch ein Gliederungsobjekt?
            SdrObject* pOutlineObj = GetPresObj(PRESOBJ_OUTLINE);

            if (pOutlineObj)
            {
                // Text des Gliederungsobjekts in das PRESOBJ_TITLE setzen
                OutlinerParaObject* pOutlParaObj = pOutlineObj->GetOutlinerParaObject();

                if (pOutlParaObj)
                {
                    if (!pOutlineObj->IsEmptyPresObj())
                    {
                        // Text umsetzen
                        SdOutliner* pOutl = ( (SdDrawDocument*) pModel )->GetInternalOutliner();
                        pOutl->Clear();
                        pOutl->SetText( *pOutlParaObj );
                        pOutl->SetMinDepth(0, TRUE);
                        pOutlParaObj = pOutl->CreateParaObject();
                        pObj->SetOutlinerParaObject( pOutlParaObj );
                        pOutl->Clear();
                        pObj->SetEmptyPresObj(FALSE);
                    }

                    // Linken Einzug zuruecksetzen
                    SfxItemSet aSet(((SdDrawDocument*) pModel)->GetPool(), EE_PARA_LRSPACE, EE_PARA_LRSPACE );

                    aSet.Put(pObj->GetItemSet());

                    const SvxLRSpaceItem& rLRItem = (const SvxLRSpaceItem&) aSet.Get(EE_PARA_LRSPACE);
                    SvxLRSpaceItem aNewLRItem(rLRItem);
                    aNewLRItem.SetTxtLeft(0);
                    aSet.Put(aNewLRItem);

                    pObj->SetItemSet(aSet);

                    SfxStyleSheet* pSheet = GetStyleSheetForPresObj(PRESOBJ_TEXT);

                    if (pSheet)
                        pObj->SetStyleSheet(pSheet, TRUE);

                    // Gliederungsobjekt loeschen
                    aPresObjList.Remove(pOutlineObj);
                    RemoveObject( pObj->GetOrdNum() );
                    ReplaceObject( pObj, pOutlineObj->GetOrdNum() );
                    delete pOutlineObj;
                }
            }
        }
    }

    return(bIncrement);
}

PresObjKind SdPage::GetPresObjKind(SdrObject* pObj)
{
    PresObjKind eObjKind = PRESOBJ_NONE;

    if (pObj && aPresObjList.GetPos(pObj) != LIST_ENTRY_NOTFOUND &&
        pObj->GetObjInventor() == SdrInventor)
    {

        SdrObjKind eSdrObjKind = (SdrObjKind) pObj->GetObjIdentifier();

        if (eSdrObjKind==OBJ_TITLETEXT)
        {
            eObjKind = PRESOBJ_TITLE;
        }
        else if (eSdrObjKind==OBJ_OUTLINETEXT)
        {
            eObjKind = PRESOBJ_OUTLINE;
        }
        else if (eSdrObjKind==OBJ_TEXT && ePageKind==PK_NOTES)
        {
            eObjKind = PRESOBJ_NOTES;
        }
        else if (eSdrObjKind==OBJ_TEXT && ePageKind!=PK_NOTES)
        {
            eObjKind = PRESOBJ_TEXT;
        }
        else if (eSdrObjKind==OBJ_GRAF)
        {
            eObjKind = PRESOBJ_GRAPHIC;
        }
        else if (eSdrObjKind==OBJ_RECT)
        {
            eObjKind = PRESOBJ_BACKGROUND;
        }
        else if (eSdrObjKind==OBJ_PAGE && ePageKind==PK_HANDOUT)
        {
            eObjKind = PRESOBJ_HANDOUT;
        }
        else if (eSdrObjKind==OBJ_PAGE && ePageKind!=PK_HANDOUT)
        {
            eObjKind = PRESOBJ_PAGE;
        }
        else if (eSdrObjKind==OBJ_OLE2)
        {
            String aName = ( (SdrOle2Obj*) pObj)->GetProgName();

            if (aName.EqualsAscii( "StarChart" ))
            {
                eObjKind = PRESOBJ_CHART;
            }
            else if (aName.EqualsAscii( "StarOrg" ))
            {
                eObjKind = PRESOBJ_ORGCHART;
            }
            else if (aName.EqualsAscii( "StarCalc" ))
            {
                eObjKind = PRESOBJ_TABLE;
            }
            else
            {
                eObjKind = PRESOBJ_OBJECT;
            }
        }
    }

    return(eObjKind);
}

void SdPage::SetObjText(SdrTextObj* pObj, SdrOutliner* pOutliner, PresObjKind eObjKind, const String& rString )
{
    if ( pObj )
    {
        DBG_ASSERT( pObj->ISA(SdrTextObj), "SetObjText: Kein SdrTextObj!" );
        Outliner* pOutl = pOutliner;

        if (!pOutliner)
        {
            SfxItemPool* pPool = ((SdDrawDocument*) GetModel())->GetDrawOutliner().GetEmptyItemSet().GetPool();
            pOutl = new Outliner( pPool, OUTLINERMODE_OUTLINEOBJECT );
            pOutl->SetRefDevice( SD_MOD()->GetRefDevice( *( (SdDrawDocument*) GetModel() )->GetDocSh() ) );
            pOutl->SetEditTextObjectPool(pPool);
            pOutl->SetStyleSheetPool((SfxStyleSheetPool*)GetModel()->GetStyleSheetPool());
            pOutl->SetMinDepth(0);
            pOutl->EnableUndo(FALSE);
            pOutl->SetUpdateMode( FALSE );
        }

        USHORT nOutlMode = pOutl->GetMode();
        USHORT nMinDepth = pOutl->GetMinDepth();
        Size aPaperSize = pOutl->GetPaperSize();
        BOOL bUpdateMode = pOutl->GetUpdateMode();
        pOutl->SetUpdateMode(FALSE);
        pOutl->SetParaAttribs( 0, pOutl->GetEmptyItemSet() );

        pOutl->SetStyleSheet(0, pObj->GetStyleSheet());

        String aString;

        if (eObjKind == PRESOBJ_OUTLINE)
        {
             pOutl->Init( OUTLINERMODE_OUTLINEOBJECT );
             pOutl->SetMinDepth(1);
 
             aString += sal_Unicode( '\t' );
             aString += rString;
 
             if (bMaster)
             {
                 pOutl->SetStyleSheet( 0, GetStyleSheetForPresObj(eObjKind) );
                 aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t" ));
                 aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER2 ) );
 
                 aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t" ));
                 aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER3 ) );
 
                 aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t" ));
                 aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER4 ) );
 
                 aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t\t" ));
                 aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER5 ) );
 
                 aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t\t\t" ));
                 aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER6 ) );
 
                 aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t\t\t\t" ));
                 aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER7 ) );
 
                 aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t\t\t\t\t" ));
                 aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER8 ) );
 
                 aString += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "\n\t\t\t\t\t\t\t\t\t" ));
                 aString += String ( SdResId( STR_PRESOBJ_MPOUTLLAYER9 ) );
             }
        }
        else if (eObjKind == PRESOBJ_TITLE)
        {
            pOutl->Init( OUTLINERMODE_TITLEOBJECT );
            pOutl->SetMinDepth(0);
            aString += rString;
        }
        else
        {
             pOutl->Init( OUTLINERMODE_TEXTOBJECT );
             pOutl->SetMinDepth(0);
             aString += rString;
         }

        pOutl->SetPaperSize( pObj->GetLogicRect().GetSize() );

        pOutl->SetText( aString, pOutl->GetParagraph( 0 ) );

        ( (SdrTextObj*) pObj)->SetOutlinerParaObject( pOutl->CreateParaObject() );

        if (!pOutliner)
        {
            delete pOutl;
            pOutl = NULL;
        }
        else
        {
            // Outliner restaurieren
            pOutl->Init( nOutlMode );
            pOutl->SetParaAttribs( 0, pOutl->GetEmptyItemSet() );
            pOutl->SetUpdateMode( bUpdateMode );
            pOutl->SetMinDepth( nMinDepth );
            pOutl->SetPaperSize( aPaperSize );
        }
    }
}

void SdPage::SetLayoutName(String aName)
{
    aLayoutName = aName;

    if( bMaster )
    {
        String aSep( RTL_CONSTASCII_USTRINGPARAM(SD_LT_SEPARATOR) );
        USHORT nPos = aLayoutName.Search( aSep );
        if ( nPos != STRING_NOTFOUND )
        {
            aPageName = aLayoutName;
            aPageName.Erase( nPos );
        }
    }
}

const String& SdPage::GetName()
{
    if (aPageName.Len() == 0)
    {
        if ((ePageKind == PK_STANDARD || ePageKind == PK_NOTES) &&
            !bMaster)
        {
            USHORT	nNum = (GetPageNum() + 1) / 2;
            BOOL	bUpper = FALSE;

            aCreatedPageName = String(SdResId(STR_PAGE));
            aCreatedPageName += sal_Unicode( ' ' );
            aCreatedPageName += ((SdDrawDocument*) GetModel())->CreatePageNumValue(nNum);
        }
         else
         {
             aCreatedPageName = String(SdResId(STR_LAYOUT_DEFAULT_NAME));
         }
     }
     else
     {
         aCreatedPageName = aPageName;
     }
 
     if (ePageKind == PK_NOTES)
     {
         aCreatedPageName += sal_Unicode( ' ' );
         aCreatedPageName += String(SdResId(STR_NOTES));
     }
    else if (ePageKind == PK_HANDOUT && bMaster)
    {
         aCreatedPageName += String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( " (" ));
         aCreatedPageName += String(SdResId(STR_HANDOUT));
         aCreatedPageName += sal_Unicode( ')' );
    }

    return aCreatedPageName;
}

void SdPage::AdjustBackgroundSize()
{
    SdrObject* pObj = GetPresObj(PRESOBJ_BACKGROUND);

    if (pObj)
    {
        pObj->SetMoveProtect(FALSE);
        pObj->SetResizeProtect(FALSE);
        bOwnArrangement = TRUE;

        Point aBackgroundPos;
        Size aBackgroundSize( GetSize() );

        if( !bBackgroundFullSize )
        {
            aBackgroundPos = Point( GetLftBorder(), GetUppBorder() );
            aBackgroundSize.Width()  -= GetLftBorder() + GetRgtBorder() - 1;
            aBackgroundSize.Height() -= GetUppBorder() + GetLwrBorder() - 1;
        }
        Rectangle aBackgroundRect (aBackgroundPos, aBackgroundSize);
        pObj->SetLogicRect(aBackgroundRect);
        bOwnArrangement = FALSE;
        pObj->SetMoveProtect(TRUE);
        pObj->SetResizeProtect(TRUE);
    }
}

void SdPage::SetOrientation( Orientation eOrient)
{
    eOrientation = eOrient;
}

Orientation SdPage::GetOrientation() const
{
    return eOrientation;
}

String SdPage::GetPresObjText(PresObjKind eObjKind)
{
    String aString;

    if (eObjKind == PRESOBJ_TITLE)
    {
        if (bMaster)
        {
            if (ePageKind != PK_NOTES)
            {
                aString = String ( SdResId( STR_PRESOBJ_MPTITLE ) );
            }
            else
            {
                aString = String ( SdResId( STR_PRESOBJ_MPNOTESTITLE ) );
            }
        }
        else
        {
            aString = String ( SdResId( STR_PRESOBJ_TITLE ) );
        }
    }
    else if (eObjKind == PRESOBJ_OUTLINE)
    {
        if (bMaster)
        {
            aString = String ( SdResId( STR_PRESOBJ_MPOUTLINE ) );
        }
        else
        {
            aString = String ( SdResId( STR_PRESOBJ_OUTLINE ) );
        }
    }
    else if (eObjKind == PRESOBJ_NOTES)
    {
        if (bMaster)
        {
            aString = String ( SdResId( STR_PRESOBJ_MPNOTESTEXT ) );
        }
        else
        {
            aString = String ( SdResId( STR_PRESOBJ_NOTESTEXT ) );
        }
    }
    else if (eObjKind == PRESOBJ_TEXT)
    {
        aString = String ( SdResId( STR_PRESOBJ_TEXT ) );
    }
    else if (eObjKind == PRESOBJ_GRAPHIC)
    {
        aString = String ( SdResId( STR_PRESOBJ_GRAPHIC ) );
    }
    else if (eObjKind == PRESOBJ_OBJECT)
    {
        aString = String ( SdResId( STR_PRESOBJ_OBJECT ) );
    }
    else if (eObjKind == PRESOBJ_CHART)
    {
        aString = String ( SdResId( STR_PRESOBJ_CHART ) );
    }
    else if (eObjKind == PRESOBJ_ORGCHART)
    {
        aString = String ( SdResId( STR_PRESOBJ_ORGCHART ) );
    }
    else if (eObjKind == PRESOBJ_TABLE)
    {
        aString = String ( SdResId( STR_PRESOBJ_TABLE ) );
    }

    return(aString);
}

extern uno::Reference< uno::XInterface > createUnoPageImpl( SdPage* pPage );

uno::Reference< uno::XInterface > SdPage::createUnoPage()
{
    return createUnoPageImpl( this );
}
}
