/*************************************************************************
 *
 *  $RCSfile: sdpage.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include "eetext.hxx"       // definiert ITEMID_... fuer frmitems und textitem
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SVDSURO_HXX //autogen
#include <svx/svdsuro.hxx>
#endif
#ifndef _SVDOUTL_HXX //autogen
#include <svx/svdoutl.hxx>
#endif
#include <svx/editdata.hxx>
#include <svx/pageitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/bulitem.hxx>
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _OUTLOBJ_HXX //autogen
#include <svx/outlobj.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOPAGE_HXX //autogen
#include <svx/svdopage.hxx>
#endif
#ifndef _SVDOPAGE_HXX //autogen
#include <svx/svdopage.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SVX_PBINITEM_HXX //autogen
#include <svx/pbinitem.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif

#ifndef SVX_LIGHT
#ifdef MAC
#include "::ui:inc:docshell.hxx"
#else
#ifdef UNX
#include "../ui/inc/docshell.hxx"
#else
#include "..\ui\inc\docshell.hxx"
#endif
#endif
#include "sdoutl.hxx"
#endif

#include "misc.hxx"
#include "eetext.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "pglink.hxx"
#include "sdresid.hxx"
#include "stlsheet.hxx"
#include "glob.hrc"
#include "glob.hxx"

#define MAX_PRESOBJ     5              // Max. Anzahl Praesentationsobjekte

TYPEINIT2( SdPage, FmFormPage, SdrObjUserCall );

/*************************************************************************
|*
|*      Ctor
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
    pPageLink(NULL),
    bBackgroundFullSize( FALSE ),
    nPaperBin(PAPERBIN_PRINTER_SETTINGS)
{
    // Der Layoutname der Seite wird von SVDRAW benutzt, um die Praesentations-
    // vorlagen der Gliederungsobjekte zu ermitteln. Darum enthaelt er bereits
    // den Bezeichner fuer die Gliederung (STR_LAYOUT_OUTLINE).
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

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

__EXPORT SdPage::~SdPage()
{
#ifndef SVX_LIGHT
    DisconnectLink();
#endif

    EndListenOutlineText();
}


/*************************************************************************
|*
|* Pruefen, ob ein bestimmtes Praesentationobjekt existiert
|* USHORT nIndex: Index des Objekttypes
|*  1.Objekt des Types -> Index = 1
|*  2.Objekt des Types -> Index = 2 usw.
|*
\************************************************************************/

SdrObject* SdPage::GetPresObj(PresObjKind eObjKind, USHORT nIndex)
{
    USHORT nObjFound = 0;          // Index des gewuenschten Objekttypes
    SdrObject* pObj = NULL;
    SdrObject* pObjFound = NULL;
    USHORT nIdx = 0;
    USHORT nCnt = (USHORT) aPresObjList.Count();

    while (nIdx < nCnt && nObjFound != nIndex)
    {
        /**************************************************************
        * Ist das Objekt in der Praesentationsobjektliste vorhanden?
        **************************************************************/
        pObj = (SdrObject*) aPresObjList.GetObject(nIdx);

        if (pObj)
        {
            if (eObjKind == GetPresObjKind(pObj))
            {
                // Uebereinstimmendes Objekt gefunden
                nObjFound++;
            }
        }

        nIdx++;
    }

    if (nObjFound == nIndex)
    {
        // Gewuenschstes Objekt in PresObjList gefunden
        pObjFound = pObj;
    }
    else if (eObjKind==PRESOBJ_TITLE || eObjKind==PRESOBJ_OUTLINE)
    {
        /**************************************************************
        * Ist das Objekt auf der Seite vorhanden?
        **************************************************************/
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
                // Uebereinstimmendes Objekt gefunden
                nObjFound++;
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



/*************************************************************************
|*
|* Es werden Praesentationsobjekte auf der Page erzeugt.
|* Alle Praesentationsobjekte erhalten einen UserCall auf die Page.
|*
\************************************************************************/

#ifndef SVX_LIGHT
SdrObject* SdPage::CreatePresObj(PresObjKind eObjKind, const Rectangle& rRect,
                                 BOOL bInsert)
{
    /**************************************************************************
    * Praesentationsobjekte werden erzeugt
    **************************************************************************/
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
        Graphic aGraphic ( SdResId(BMP_PRESOBJ_GRAPHIC) );
        OutputDevice &aOutDev = *Application::GetDefaultDevice();
        aOutDev.Push();

        aOutDev.SetMapMode( aGraphic.GetPrefMapMode() );
        Size aSizePix = aOutDev.LogicToPixel( aGraphic.GetPrefSize() );
        aOutDev.SetMapMode(MAP_100TH_MM);

        Size aSize = aOutDev.PixelToLogic(aSizePix);
        Point aPnt (0, 0);
        Rectangle aRect (aPnt, aSize);
        pSdrObj = new SdrGrafObj(aGraphic, aRect);
        aOutDev.Pop();
    }
    else if (eObjKind == PRESOBJ_OBJECT)
    {
        pSdrObj = new SdrOle2Obj();
        Graphic aGraphic( SdResId(BMP_PRESOBJ_OBJECT) );
        ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
    }
    else if (eObjKind == PRESOBJ_CHART)
    {
        pSdrObj = new SdrOle2Obj();
        ( (SdrOle2Obj*) pSdrObj)->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarChart" )));
        Graphic aGraphic( SdResId(BMP_PRESOBJ_CHART) );
        ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
    }
    else if (eObjKind == PRESOBJ_ORGCHART)
    {
        pSdrObj = new SdrOle2Obj();
        ( (SdrOle2Obj*) pSdrObj)->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarOrg" )));
        Graphic aGraphic( SdResId(BMP_PRESOBJ_ORGCHART) );
        ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
    }
    else if (eObjKind == PRESOBJ_TABLE)
    {
        pSdrObj = new SdrOle2Obj();
        ( (SdrOle2Obj*) pSdrObj)->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarCalc" )));
        Graphic aGraphic( SdResId(BMP_PRESOBJ_TABLE) );
        ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
    }
#ifdef STARIMAGE_AVAILABLE
    else if (eObjKind == PRESOBJ_IMAGE)
    {
        pSdrObj = new SdrOle2Obj();
        ( (SdrOle2Obj*) pSdrObj)->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarImage" )));
        Graphic aGraphic( SdResId(BMP_PRESOBJ_IMAGE) );
        ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
    }
#endif
    else if (eObjKind == PRESOBJ_BACKGROUND)
    {
        pSdrObj = new SdrRectObj();
        pSdrObj->SetMoveProtect(TRUE);
        pSdrObj->SetResizeProtect(TRUE);
        pSdrObj->SetMarkProtect(TRUE);
    }
    else if (eObjKind == PRESOBJ_HANDOUT)
    {
        //Erste Standardseite am SdrPageObj vermerken
        SdrPage* pPage = ( (SdDrawDocument*) pModel )->GetSdPage(0, PK_STANDARD);
        pSdrObj = new SdrPageObj( pPage->GetPageNum() );
        pSdrObj->SetResizeProtect(TRUE);
    }
    else if (eObjKind == PRESOBJ_PAGE)
    {
        //Notizseite am SdrPageObj vermerken
        pSdrObj = new SdrPageObj( GetPageNum() - 1 );
        pSdrObj->SetResizeProtect(TRUE);
    }

    if (pSdrObj)
    {
        String aString = GetPresObjText(eObjKind);
        pSdrObj->SetEmptyPresObj(TRUE);
        pSdrObj->SetLogicRect(rRect);

        if ( pSdrObj->ISA(SdrTextObj) )
        {
            SfxItemSet aTempAttr( ((SdDrawDocument*) pModel)->GetPool() );
            SdrTextMinFrameHeightItem aMinHeight( rRect.GetSize().Height() );
            aTempAttr.Put( aMinHeight );

            if (bMaster)
            {
                // Bei Praesentationsobjekten auf der MasterPage soll die
                // Groesse vom Benutzwer frei waehlbar sein
                SdrTextAutoGrowHeightItem aAutoGrowHeight(FALSE);
                aTempAttr.Put(aAutoGrowHeight);
            }

            pSdrObj->NbcSetAttributes(aTempAttr, FALSE);
        }

        if ( aString.Len() && pSdrObj->ISA(SdrTextObj) )
        {
            SdrOutliner* pOutliner = ( (SdDrawDocument*) GetModel() )->GetInternalOutliner();
            USHORT nOutlMode = pOutliner->GetMode();
            pOutliner->Init( OUTLINERMODE_TEXTOBJECT );
            pOutliner->SetMinDepth(0);
            pOutliner->SetStyleSheet( 0, NULL );

            String aEmptyStr;
            SetObjText( (SdrTextObj*) pSdrObj, pOutliner, eObjKind, aString );

            pOutliner->Init( nOutlMode );
            pOutliner->SetStyleSheet( 0, NULL );
        }

        pSdrObj->SetUserCall(this);
        InsertObject(pSdrObj);
        pSdrObj->RecalcBoundRect();

        if (bMaster)
        {
            SdrLayerAdmin& rLayerAdmin = pModel->GetLayerAdmin();

            if (eObjKind == PRESOBJ_BACKGROUND)
            {
                // Hintergrund der MasterPage
                pSdrObj->SetLayer( rLayerAdmin.
                    GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE) );
            }
            else
            {
                // Hintergrundobjekte der MasterPage
                pSdrObj->SetLayer( rLayerAdmin.
                    GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE) );
            }
        }

        // Objekt am StyleSheet anmelden
        pSdrObj->NbcSetStyleSheet( GetStyleSheetForPresObj(eObjKind), FALSE );

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

        if ( eObjKind == PRESOBJ_OBJECT   ||
             eObjKind == PRESOBJ_CHART    ||
             eObjKind == PRESOBJ_ORGCHART ||
             eObjKind == PRESOBJ_TABLE    ||
#ifdef STARIMAGE_AVAILABLE
             eObjKind == PRESOBJ_IMAGE    ||
#endif
             eObjKind == PRESOBJ_GRAPHIC )
        {
            SfxItemSet aSet( ((SdDrawDocument*) pModel)->GetPool() );
            aSet.Put( SdrTextContourFrameItem( TRUE ) );
            aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER ) );
            pSdrObj->NbcSetAttributes( aSet, FALSE );
        }

        if (bInsert)
        {
            aPresObjList.Insert(pSdrObj, LIST_APPEND);
        }
    }

    return(pSdrObj);
}
#endif

/*************************************************************************
|*
|* Es werden Praesentationsobjekte auf der Page erzeugt.
|* Alle Praesentationsobjekte erhalten einen UserCall auf die Page.
|*
\************************************************************************/

#ifndef SVX_LIGHT
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
    SfxStyleSheetBase*     pResult   = pStShPool->Find(aName, SD_LT_FAMILY);
    return (SfxStyleSheet*)pResult;
}
#endif // !SVX_LIGHT

/*************************************************************************
|*
|* Das Praesentationsobjekt rObj hat sich geaendert und wird nicht mehr
|* durch das Praesentationsobjekt der MasterPage referenziert.
|* Der UserCall wird geloescht.
|*
\************************************************************************/

#ifndef SVX_LIGHT
void __EXPORT SdPage::Changed(const SdrObject& rObj, SdrUserCallType eType,
                              const Rectangle& rOldBoundRect)
{
    if (!bOwnArrangement)
    {
        switch (eType)
        {
            case SDRUSERCALL_MOVEONLY:
            case SDRUSERCALL_RESIZE:
            {
                SdrObject* pObj = (SdrObject*) &rObj;

                if (pObj)
                {
                    if (!bMaster)
                    {
                        // Objekt wurde vom Benutzer veraendert und hoert damit nicht
                        // mehr auf die MasterPage-Objekte
                        pObj->SetUserCall(NULL);
                    }
                    else if (pModel)
                    {
                        // MasterPage-Objekt wurde veraendert, daher
                        // Objekte auf allen Seiten anpassen
                        PageKind ePgKind = GetPageKind();
                        USHORT nPageCount = ((SdDrawDocument*) pModel)->GetSdPageCount(ePageKind);

                        for (USHORT i = 0; i < nPageCount; i++)
                        {
                            SdPage* pPage = ((SdDrawDocument*) pModel)->GetSdPage(i, ePageKind);

                            if (pPage && pPage->GetMasterPage(0) == this)
                            {
                                // Seite hoert auf diese MasterPage, daher
                                // AutoLayout anpassen
                                pPage->SetAutoLayout(pPage->GetAutoLayout());
                            }
                        }
                    }
                }
            }
            break;

            case SDRUSERCALL_DELETE:
            case SDRUSERCALL_REMOVED:
            {
                if (!bMaster &&
                    aPresObjList.GetPos((void*) &rObj) != LIST_ENTRY_NOTFOUND)
                {
                    if (!rObj.IsEmptyPresObj())
                    {
                        // In die Liste fuers Undo eintragen, da dieses Objekt
                        // durch das Default-Praesentationsobjekt ersetzt werden
                        // soll.
                        // Im UndoActionHdl des DrawDocs wird der UserCall
                        // auf NULL gesetzt und das Obj aus der Liste ausgetragen
                        ((SdrObject&) rObj).SetUserCall(this);
                        List* pList = ((SdDrawDocument*) pModel)->GetDeletedPresObjList();
                        pList->Insert((void*) &rObj, LIST_APPEND);
                    }
                    else
                    {
                        aPresObjList.Remove((void*) &rObj);
                        ((SdrObject*) &rObj)->SetUserCall(NULL);  // const as const can...
                    }
                }
            }
            break;

            default:
            break;
        }
    }
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Erzeugt auf einer MasterPage Hintergrund, Titel- und Layout-Bereich
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdPage::CreateTitleAndLayout(BOOL bInit)
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

    /**************************************************************************
    * Hintergrund, Titel- und Layout-Bereich werden angelegt
    **************************************************************************/
    SdrObject* pMasterTitle = NULL;
    SdrObject* pMasterOutline = NULL;
    SdrObject* pMasterBackground = NULL;


    ULONG nMasterIndex = 0;
    ULONG nMasterCount = pMasterPage->GetPresObjList()->Count();
    SdrObject* pMasterObj = NULL;

    for (nMasterIndex = 0; nMasterIndex < nMasterCount; nMasterIndex++)
    {
        /******************************************************************
        * Schleife ueber alle Praesentationsobjekte der MasterPage
        ******************************************************************/
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
        /******************************************************************
        * Hintergrundobjekt
        ******************************************************************/
        Point aBackgroundPos ( GetLftBorder(), GetUppBorder() );
        Size aBackgroundSize ( GetSize() );
        aBackgroundSize.Width()  -= GetLftBorder() + GetRgtBorder() - 1;
        aBackgroundSize.Height() -= GetUppBorder() + GetLwrBorder() - 1;
        Rectangle aBackgroundRect (aBackgroundPos, aBackgroundSize);
        pMasterPage->CreatePresObj(PRESOBJ_BACKGROUND, aBackgroundRect, TRUE);
    }

    BOOL bDeletePresObjOnMaster = FALSE;

    if (eAutoLayout == AUTOLAYOUT_NONE)
    {
        // Die aktuelle Seite soll kein AutoLayout haben!
        // Sind die Praesentationsobjekte auf der MasterPage noch notwendig?
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
                CreatePresObj(PRESOBJ_HANDOUT, aRect, TRUE);

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
            /******************************************************************
            * Standard- oder Notiz-Seite: Titelbereich
            ******************************************************************/
            Rectangle aTitleRect = GetTitleRect();
            pMasterPage->CreatePresObj(PRESOBJ_TITLE, aTitleRect, TRUE);
        }

        if (!pMasterOutline  && ePageKind != PK_HANDOUT)
        {
            /******************************************************************
            * Standard- oder Notiz-Seite: Layoutbereich
            ******************************************************************/
            Rectangle aLayoutRect = GetLayoutRect();

            if (ePageKind == PK_STANDARD)
            {
                pMasterPage->CreatePresObj(PRESOBJ_OUTLINE, aLayoutRect, TRUE);
            }
            else if (ePageKind == PK_NOTES)
            {
                pMasterPage->CreatePresObj(PRESOBJ_NOTES, aLayoutRect, TRUE);
            }
        }
    }
}
#endif // !SVX_LIGHT

/*************************************************************************
|*
|* Titelbereich zurueckgeben
|*
\************************************************************************/

Rectangle SdPage::GetTitleRect() const
{
    Rectangle aTitleRect;

    if (ePageKind != PK_HANDOUT)
    {
        /******************************************************************
        * Standard- oder Notiz-Seite: Titelbereich
        ******************************************************************/
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
            // Hoehe beschraenken
            aTitleSize.Height() = (long) (aTitleSize.Height() / 2.5);

            Point aPos = aTitlePos;
            aPos.Y() += long( aTitleSize.Height() * 0.083 );
            Size aPartArea = aTitleSize;
            Size aSize;

            SdrPage* pPage = pModel->GetPage( GetPageNum() - 1 );

            if ( pPage )
            {
                // tatsaechliche Seitengroesse in das Handout-Rechteck skalieren
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


/*************************************************************************
|*
|* Gliederungsbereich zurueckgeben
|*
\************************************************************************/

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


/**************************************************************************
|*
|* Diese Methode weist ein AutoLayout zu
|*
\*************************************************************************/

#ifndef SVX_LIGHT
void SdPage::SetAutoLayout(AutoLayout eLayout, BOOL bInit)
{
    eAutoLayout = eLayout;
    bOwnArrangement = TRUE;

    CreateTitleAndLayout(bInit);

    if ((eAutoLayout == AUTOLAYOUT_NONE && aPresObjList.Count() == 0) ||
        bMaster)
    {
        // MasterPage oder:
        // Kein AutoLayout gewuenscht und keine Praesentationsobjekte
        // vorhanden, also ist nichts zu tun
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
        /**********************************************************************
        * Schleife ueber alle Praesentationsobjekte der MasterPage
        **********************************************************************/
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
        /**********************************************************************
        * Titelobj. auf MasterPage nicht vorhanden -> Defaultgroesse bestimmen
        **********************************************************************/
        aTitleRect = GetTitleRect();
    }

    if (aLayoutRect.IsEmpty() && ePageKind != PK_HANDOUT)
    {
        /**********************************************************************
        * Gliederungsobj. auf MasterPage nicht vorhanden -> Defaultgroesse bestimmen
        **********************************************************************/
        aLayoutRect = GetLayoutRect();
    }

    Rectangle aRect0 = aTitleRect;
    Rectangle aRect1 = aLayoutRect;
    Rectangle aRect2 = aLayoutRect;
    Rectangle aRect3 = aLayoutRect;
    Rectangle aRect4 = aLayoutRect;

    Size aTitleSize = aTitleRect.GetSize();
    Point aTitlePos = aTitleRect.TopLeft();

    Size aLayoutSize = aLayoutRect.GetSize();
    Point aLayoutPos = aLayoutRect.TopLeft();
    Size aTempSize;
    Point aTempPnt;

    PresObjKind nObjKind[MAX_PRESOBJ];

    for (nIndex = 0; nIndex < MAX_PRESOBJ; nIndex++)
    {
        nObjKind[nIndex] = PRESOBJ_NONE;
    }

    List aObjList;

    switch (eAutoLayout)
    {
        case AUTOLAYOUT_NONE:
        {
            /******************************************************************
            * Notwendig fuer Objekte aus dem Gliederungsmodus
            ******************************************************************/
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
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_NOTES;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TITLE:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_TEXT;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_ENUM:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_CHART:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_CHART;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_2TEXT:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            USHORT nIndex = 1;
            if ( InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList) )
                nIndex++;

            nObjKind[2] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[2], nIndex);
            InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TEXTCHART:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_CHART;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_ORG:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_ORGCHART;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TEXTCLIP:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_GRAPHIC;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_CHARTTEXT:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_CHART;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TAB:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_TABLE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_CLIPTEXT:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_GRAPHIC;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_TEXTOBJ:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_OBJ:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);
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

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2]);
            USHORT nIndex = 1;
            if ( InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList) )
                nIndex++;

            nObjKind[3] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[3], nIndex);
            InsertPresObj(pObj, nObjKind[3], aRect3, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_OBJTEXT:
        {
            aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
            aRect1 = Rectangle (aLayoutPos, aLayoutSize);

            aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
            aRect2 = Rectangle (aLayoutPos, aLayoutSize);

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList);
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
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList);
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

            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            USHORT nIndex = 1;
            if ( InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList) )
                nIndex++;

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2], nIndex);
            InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList);

            nObjKind[3] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[3]);
            InsertPresObj(pObj, nObjKind[3], aRect3, bInit, aObjList);
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
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            USHORT nIndex = 1;
            if ( InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList) )
                nIndex++;

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2], nIndex);
            InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList);

            nObjKind[3] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[3]);
            InsertPresObj(pObj, nObjKind[3], aRect3, bInit, aObjList);
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
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);
            InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList);

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2]);
            InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList);
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
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);

            nObjKind[1] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[1]);
            USHORT nIndex = 1;
            if ( InsertPresObj(pObj, nObjKind[1], aRect1, bInit, aObjList) )
                nIndex++;

            nObjKind[2] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[2], nIndex);
            if ( InsertPresObj(pObj, nObjKind[2], aRect2, bInit, aObjList) )
                nIndex++;

            nObjKind[3] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[3], nIndex);
            if ( InsertPresObj(pObj, nObjKind[3], aRect3, bInit, aObjList) )
                nIndex++;

            nObjKind[4] = PRESOBJ_OBJECT;
            pObj = GetPresObj(nObjKind[4], nIndex);
            InsertPresObj(pObj, nObjKind[4], aRect4, bInit, aObjList);
        }
        break;

        case AUTOLAYOUT_ONLY_TITLE:
        {
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);
            InsertPresObj(pObj, nObjKind[0], aRect0, bInit, aObjList);
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
#endif // !SVX_LIGHT

/*************************************************************************
|*
|* Objekt einfuegen
|*
\************************************************************************/

void __EXPORT SdPage::NbcInsertObject(SdrObject* pObj, ULONG nPos,
                                      const SdrInsertReason* pReason)
{
    FmFormPage::NbcInsertObject(pObj, nPos, pReason);

#ifndef SVX_LIGHT
    ((SdDrawDocument*) pModel)->InsertObject(pObj, this);
#endif

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


/*************************************************************************
|*
|* Objekt loeschen
|*
\************************************************************************/

#ifndef SVX_LIGHT
SdrObject* __EXPORT SdPage::RemoveObject(ULONG nObjNum)
{
    SdrObject* pObj = FmFormPage::RemoveObject(nObjNum);

    if (pObj && pObj->GetUserCall()!=this &&
        aPresObjList.GetPos(pObj) != LIST_ENTRY_NOTFOUND)
    {
        // Objekt hat keinen UserCall auf diese Seite, es ist jedoch noch in
        // der PresObjList eingetragen -> austragen
        Changed(*pObj, SDRUSERCALL_REMOVED, pObj->GetBoundRect());
    }

    ((SdDrawDocument*) pModel)->RemoveObject(pObj, this);

    return(pObj);
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Objekt loeschen, ohne Broadcast
|*
\************************************************************************/

#ifndef SVX_LIGHT
SdrObject* __EXPORT SdPage::NbcRemoveObject(ULONG nObjNum)
{
    SdrObject* pObj = FmFormPage::NbcRemoveObject(nObjNum);

    if (pObj && pObj->GetUserCall()!=this &&
        aPresObjList.GetPos(pObj) != LIST_ENTRY_NOTFOUND)
    {
        // Objekt hat keinen UserCall auf diese Seite, es ist jedoch noch in
        // der PresObjList eingetragen -> austragen
        Changed(*pObj, SDRUSERCALL_REMOVED, pObj->GetBoundRect());
    }

    ((SdDrawDocument*) pModel)->RemoveObject(pObj, this);

    return(pObj);
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|*
|*
\************************************************************************/

void __EXPORT SdPage::SetSize(const Size& aSize)
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


/*************************************************************************
|*
|*
|*
\************************************************************************/

void __EXPORT SdPage::SetBorder(INT32 nLft, INT32 nUpp, INT32 nRgt, INT32 nLwr)
{
    if (nLft != GetLftBorder() || nUpp != GetUppBorder() ||
        nRgt != GetRgtBorder() || nLwr != GetLwrBorder() )
    {
        FmFormPage::SetBorder(nLft, nUpp, nRgt, nLwr);
        AdjustBackgroundSize();
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void __EXPORT SdPage::SetLftBorder(INT32 nBorder)
{
    if (nBorder != GetLftBorder() )
    {
        FmFormPage::SetLftBorder(nBorder);
        AdjustBackgroundSize();
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void __EXPORT SdPage::SetRgtBorder(INT32 nBorder)
{
    if (nBorder != GetRgtBorder() )
    {
        FmFormPage::SetRgtBorder(nBorder);
        AdjustBackgroundSize();
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void __EXPORT SdPage::SetUppBorder(INT32 nBorder)
{
    if (nBorder != GetUppBorder() )
    {
        FmFormPage::SetUppBorder(nBorder);
        AdjustBackgroundSize();
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void __EXPORT SdPage::SetLwrBorder(INT32 nBorder)
{
    if (nBorder != GetLwrBorder() )
    {
        FmFormPage::SetLwrBorder(nBorder);
        AdjustBackgroundSize();
    }
}

/*************************************************************************
|*
|* Setzt BackgroundFullSize und ruft dann AdjustBackground auf
|*
\************************************************************************/

void SdPage::SetBackgroundFullSize( BOOL bIn )
{
    if( bIn != bBackgroundFullSize )
    {
        bBackgroundFullSize = bIn;
        AdjustBackgroundSize();
    }
}

/*************************************************************************
|*
|* Alle Objekte an neue Seitengroesse anpassen
|*
|* bScaleAllObj: Alle Objekte werden in die neue Flaeche innerhalb der
|* Seitenraender skaliert. Dabei werden die Position, Groesse und bei
|* Praesentationsobjekten auf der MasterPage auch die Schrifthoehe der
|* Praesentationsvorlagen skaliert.
|*
\************************************************************************/

#ifndef SVX_LIGHT
void __EXPORT SdPage::ScaleObjects(const Size& rNewPageSize,
                                   const Rectangle& rNewBorderRect,
                                   BOOL bScaleAllObj)
{
    bOwnArrangement = TRUE;
    bScaleObjects = bScaleAllObj;
    SdrObject* pObj = NULL;
    Point aRefPnt(0, 0);
    Size aNewPageSize(rNewPageSize);
    INT32 nLeft  = rNewBorderRect.Left();
    INT32 nRight = rNewBorderRect.Right();
    INT32 nUpper = rNewBorderRect.Top();
    INT32 nLower = rNewBorderRect.Bottom();

    // Negative Werte stehen fuer nicht zu aendernde Werte
    // -> aktuelle Werte verwenden
    if (aNewPageSize.Width() < 0)
    {
        aNewPageSize.Width() = GetWdt();
    }
    if (aNewPageSize.Height() < 0)
    {
        aNewPageSize.Height() = GetHgt();
    }
    if (nLeft < 0)
    {
        nLeft = GetLftBorder();
    }
    if (nRight < 0)
    {
        nRight = GetRgtBorder();
    }
    if (nUpper < 0)
    {
        nUpper = GetUppBorder();
    }
    if (nLower < 0)
    {
        nLower = GetLwrBorder();
    }

    Point aBackgroundPos(nLeft, nUpper);
    Size aBackgroundSize(aNewPageSize);
    Rectangle aBorderRect (aBackgroundPos, aBackgroundSize);

    if (bScaleObjects)
    {
        aBackgroundSize.Width()  -= nLeft  + nRight;
        aBackgroundSize.Height() -= nUpper + nLower;
        aBorderRect.SetSize(aBackgroundSize);
        aNewPageSize = aBackgroundSize;
    }

    long nOldWidth  = GetWdt() - GetLftBorder() - GetRgtBorder();
    long nOldHeight = GetHgt() - GetUppBorder() - GetLwrBorder();

    Fraction aFractX = Fraction(aNewPageSize.Width(), nOldWidth);
    Fraction aFractY = Fraction(aNewPageSize.Height(), nOldHeight);

//    ULONG nObjCnt = (bScaleObjects ? GetObjCount() : aPresObjList.Count());
    ULONG nObjCnt = (bScaleObjects ? GetObjCount() : 0);

    for (ULONG nObj = 0; nObj < nObjCnt; nObj++)
    {
        BOOL bIsPresObjOnMaster = FALSE;
        SfxStyleSheet* pSheet = NULL;

        if (bScaleObjects)
        {
            // Alle Objekte
            pObj = GetObj(nObj);

            if (bMaster && aPresObjList.GetPos(pObj) != LIST_ENTRY_NOTFOUND)
            {
                // Es ist ein Praesentationsobjekt auf der MasterPage
                bIsPresObjOnMaster = TRUE;
            }
        }
        else
        {
            // Nur Praesentationsobjekte
            pObj = (SdrObject*) aPresObjList.GetObject(nObj);

            if (bMaster)
            {
                bIsPresObjOnMaster = TRUE;
            }
        }

        if (pObj)
        {
            USHORT nIndexBackground = 0;

            if (bIsPresObjOnMaster &&
                (ePageKind == PK_HANDOUT ||
                 pObj == GetPresObj(PRESOBJ_BACKGROUND, nIndexBackground)))
            {
                /**************************************************************
                * 1. Praesentationsobjekte auf Handzettelseite sollen nur positioniert werden
                * 2. Hintergrundobjekt wird nicht skaliert
                **************************************************************/
            }
            else if (!pObj->IsEdgeObj())
            {
                /**************************************************************
                * Objekt skalieren
                **************************************************************/
                if (bScaleObjects)
                {
                    aRefPnt = pObj->GetBoundRect().TopLeft();
                }

                pObj->Resize(aRefPnt, aFractX, aFractY);

                if (bScaleObjects)
                {
                    SdrObjKind eObjKind = (SdrObjKind) pObj->GetObjIdentifier();

                    if (bIsPresObjOnMaster)
                    {
                        /**********************************************************
                        * Praesentationsvorlage: Texthoehe anpassen
                        **********************************************************/
                        USHORT nIndexTitle = 0;
                        USHORT nIndexOutline = 0;
                        USHORT nIndexNotes = 0;

                        if (pObj == GetPresObj(PRESOBJ_TITLE, nIndexTitle))
                        {
                            SfxStyleSheet* pSheet = GetStyleSheetForPresObj(PRESOBJ_TITLE);

                            if (pSheet)
                            {
                                SfxItemSet& rSet = pSheet->GetItemSet();
                                const SvxFontHeightItem& rOldHgt = (SvxFontHeightItem&)
                                rSet.Get(EE_CHAR_FONTHEIGHT);
                                ULONG nFontHeight = rOldHgt.GetHeight();
                                nFontHeight = long(nFontHeight * (double) aFractY);
                                rSet.Put(SvxFontHeightItem(nFontHeight));
                                pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                            }
                        }
                        else if (pObj == GetPresObj(PRESOBJ_OUTLINE, nIndexOutline))
                        {
                            ULONG nHeight = pObj->GetLogicRect().GetSize().Height() / 9;
                            String aName(GetLayoutName());
                            aName += sal_Unicode( ' ' );

                            for (USHORT i=1; i<=9; i++)
                            {
                                String aLayoutName(aName);
                                aLayoutName += String::CreateFromInt32( (sal_Int32)i );
                                SfxStyleSheet* pSheet = (SfxStyleSheet*)
                                ((SdDrawDocument*) pModel)->GetStyleSheetPool()->
                                Find(aLayoutName, SD_LT_FAMILY);

                                if (pSheet)
                                {
                                    // Neue Fonthoehe berechnen
                                    SfxItemSet aTempSet(pSheet->GetItemSet());
                                    const SvxFontHeightItem& rOldHgt = (SvxFontHeightItem&)
                                    aTempSet.Get(EE_CHAR_FONTHEIGHT);
                                    ULONG nFontHeight = rOldHgt.GetHeight();
                                    nFontHeight = long(nFontHeight * (double) aFractY);
                                    aTempSet.Put(SvxFontHeightItem(nFontHeight));

                                    // Bullet anpassen
                                    ((SdStyleSheet*) pSheet)->AdjustToFontHeight(aTempSet, FALSE);

                                    // Sonderbehandlung: die INVALIDS auf NULL-Pointer
                                    // zurueckgesetzen (sonst landen INVALIDs oder
                                    // Pointer auf die DefaultItems in der Vorlage;
                                    // beides wuerde die Attribut-Vererbung unterbinden)
                                    aTempSet.ClearInvalidItems();

                                    // Sonderbehandlung: nur die gueltigen Anteile des
                                    // BulletItems
                                    if (aTempSet.GetItemState(EE_PARA_BULLET) == SFX_ITEM_SET)
                                    {
                                        SvxBulletItem aOldBulItem((SvxBulletItem&) pSheet->GetItemSet().Get(EE_PARA_BULLET));
                                        SvxBulletItem& rNewBulItem = (SvxBulletItem&) aTempSet.Get(EE_PARA_BULLET);
                                        aOldBulItem.CopyValidProperties(rNewBulItem);
                                        aTempSet.Put(aOldBulItem);
                                    }

                                    pSheet->GetItemSet().Put(aTempSet);

                                    pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                                }
                            }
                        }
                        else if (pObj == GetPresObj(PRESOBJ_NOTES, nIndexNotes))
                        {
                            SfxStyleSheet* pSheet = GetStyleSheetForPresObj(PRESOBJ_NOTES);

                            if (pSheet)
                            {
                                ULONG nHeight = pObj->GetLogicRect().GetSize().Height();
                                ULONG nFontHeight = (ULONG) (nHeight * 0.0741);
                                pSheet->GetItemSet().Put(SvxFontHeightItem(nFontHeight));
                                pSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                            }
                        }
                    }
                    else if ( eObjKind != OBJ_TITLETEXT   &&
                              eObjKind != OBJ_OUTLINETEXT &&
                              pObj->ISA(SdrTextObj)       &&
                              pObj->GetOutlinerParaObject() )
                    {
                        /******************************************************
                        * Normales Textobjekt: Texthoehe anpassen
                        ******************************************************/
                        pObj->GetOutlinerParaObject()->RemoveCharAttribs( EE_CHAR_FONTHEIGHT );
                        SfxItemSet aSet( ((SdDrawDocument*) pModel)->GetPool(), EE_CHAR_FONTHEIGHT, EE_CHAR_FONTHEIGHT );
                        pObj->TakeAttributes(aSet, TRUE, FALSE);
                        const SvxFontHeightItem& rOldHgt = (SvxFontHeightItem&) aSet.Get(EE_CHAR_FONTHEIGHT);
                        ULONG nFontHeight = rOldHgt.GetHeight();
                        nFontHeight = long(nFontHeight * (double) aFractY);
                        aSet.Put(SvxFontHeightItem(nFontHeight));
                        pObj->NbcSetAttributes(aSet, FALSE);
                    }
                }
            }

            if (bScaleObjects && !pObj->IsEdgeObj())
            {
                /**************************************************************
                * Objektposition skalieren
                **************************************************************/
                Rectangle aBoundRect(pObj->GetBoundRect());
                Point aOldPos(aBoundRect.TopLeft());
                Point aNewPos(aOldPos);

                // Position skalieren
                aNewPos.X() = long(aNewPos.X() * (double) aFractX);
                aNewPos.Y() = long(aNewPos.Y() * (double) aFractY);

                // Offset beruecksichtigen
                aNewPos.X() += nLeft  - GetLftBorder();
                aNewPos.Y() += nUpper - GetUppBorder();

                Size aVec(aNewPos.X() - aOldPos.X(), aNewPos.Y() - aOldPos.Y());

                if (aVec.Height() != 0 || aVec.Width() != 0)
                {
                    pObj->NbcMove(aVec);
                }

                aBoundRect = pObj->GetBoundRect();

                if (!aBorderRect.IsInside(aBoundRect))
                {
                    /**********************************************************
                    * Objekt liegt nicht vollstaendig innerhalb der Raender
                    * -> Position korrigieren
                    **********************************************************/
                    Point aOldPos(aBoundRect.TopLeft());
                    Point aNewPos(aOldPos);

                    // Position links oben ggf. korrigieren
                    aNewPos.X() = Max(aNewPos.X(), aBorderRect.Left());
                    aNewPos.Y() = Max(aNewPos.Y(), aBorderRect.Top());
                    Size aVec(aNewPos.X() - aOldPos.X(), aNewPos.Y() - aOldPos.Y());

                    if (aVec.Height() != 0 || aVec.Width() != 0)
                    {
                        pObj->NbcMove(aVec);
                    }

                    // Position rechts unten ggf. korrigieren
                    aOldPos = aBoundRect.BottomRight();
                    aNewPos = aOldPos;
                    aNewPos.X() = Min(aNewPos.X(), aBorderRect.Right());
                    aNewPos.Y() = Min(aNewPos.Y(), aBorderRect.Bottom());
                    aVec = Size(aNewPos.X() - aOldPos.X(), aNewPos.Y() - aOldPos.Y());

                    if (aVec.Height() != 0 || aVec.Width() != 0)
                    {
                        pObj->NbcMove(aVec);
                    }
                }

                pObj->SendRepaintBroadcast();
            }
        }
    }

    bOwnArrangement = FALSE;
}
#endif // !SVX_LIGHT

/*************************************************************************
|*
|*
|*
\************************************************************************/

#ifndef SVX_LIGHT
BOOL __EXPORT SdPage::InsertPresObj(SdrObject* pObj, PresObjKind eObjKind,
                                    Rectangle aRect, BOOL bInit, List& rObjList)
{
    BOOL bIncrement = FALSE;

    if (!pObj && bInit)
    {
        pObj = CreatePresObj(eObjKind, aRect);
    }
    else if ( pObj && (pObj->GetUserCall() || bInit) )
    {
        if ( pObj->ISA(SdrGrafObj) && !pObj->IsEmptyPresObj() )
            ( (SdrGrafObj*) pObj)->AdjustToMaxRect( aRect, FALSE );
        else
            pObj->SetLogicRect(aRect);

        pObj->SetUserCall(this);

        if ( pObj->ISA(SdrTextObj) && ( (SdrTextObj*) pObj)->IsAutoGrowHeight()
             && !bMaster )
        {
            // AutoGrowHeight ausschalten, MinHeight neu setzen
            SfxItemSet aTempAttr( ((SdDrawDocument*) pModel)->GetPool() );
            SdrTextMinFrameHeightItem aMinHeight( aRect.GetSize().Height() );
            aTempAttr.Put( aMinHeight );
            SdrTextAutoGrowHeightItem aAutoGrowHeight(FALSE);
            aTempAttr.Put(aAutoGrowHeight);
            pObj->NbcSetAttributes(aTempAttr, FALSE);
            pObj->SetLogicRect(aRect);

            // AutoGrowHeight einschalten
            SfxItemSet aAttr( ((SdDrawDocument*) pModel)->GetPool() );
            SdrTextAutoGrowHeightItem aAutoGrowHeightOn(TRUE);
            aAttr.Put(aAutoGrowHeightOn);
            pObj->NbcSetAttributes(aAttr, FALSE);
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
                    pObj->TakeAttributes(aSet, TRUE, FALSE);
                    aSet.ClearItem(EE_PARA_LRSPACE);
                    pObj->NbcSetAttributes(aSet, FALSE);

                    // Untertitel loeschen
                    aPresObjList.Remove(pSubtitle);
                    RemoveObject( pObj->GetOrdNum() );
                    ReplaceObject( pObj, pSubtitle->GetOrdNum() );
                    ( (SdDrawDocument*) pModel)->RemoveObject( pSubtitle, this );
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
                    pObj->TakeAttributes(aSet, TRUE, FALSE);
                    const SvxLRSpaceItem& rLRItem = (const SvxLRSpaceItem&) aSet.Get(EE_PARA_LRSPACE);
                    SvxLRSpaceItem aNewLRItem(rLRItem);
                    aNewLRItem.SetTxtLeft(0);
                    aSet.Put(aNewLRItem);
                    pObj->NbcSetAttributes(aSet, FALSE);

                    SfxStyleSheet* pSheet = GetStyleSheetForPresObj(PRESOBJ_TEXT);

                    if (pSheet)
                        pObj->SetStyleSheet(pSheet, TRUE);

                    // Gliederungsobjekt loeschen
                    aPresObjList.Remove(pOutlineObj);
                    RemoveObject( pObj->GetOrdNum() );
                    ReplaceObject( pObj, pOutlineObj->GetOrdNum() );
                    ( (SdDrawDocument*) pModel)->RemoveObject( pOutlineObj, this );
                    delete pOutlineObj;
                }
            }
        }
    }

    return(bIncrement);
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Liefert den PresObjKind eines Objektes zurueck
|*
\************************************************************************/

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
#ifdef STARIMAGE_AVAILABLE
            else if (aName.EqualsAscii( "StarImage" ))
            {
                eObjKind = PRESOBJ_IMAGE;
            }
#endif
            else
            {
                eObjKind = PRESOBJ_OBJECT;
            }
        }
    }

    return(eObjKind);
}

/*************************************************************************
|*
|* BASIC anfordern
|*
\************************************************************************/

#ifndef SVX_LIGHT
void __EXPORT SdPage::RequestBasic()
{
    SdDrawDocShell* pDocShell = ( (SdDrawDocument*) GetModel() )->GetDocSh();

    if (pDocShell)
    {
        SetBasic( pDocShell->GetBasicManager()->GetLib(0) );
    }
}
#endif // !SVX_LIGHT

/*************************************************************************
|*
|* Text des Objektes setzen
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdPage::SetObjText(SdrTextObj* pObj, SdrOutliner* pOutliner,
                        PresObjKind eObjKind,
                        const String& rString )
{
    if ( pObj )
    {
        DBG_ASSERT( pObj->ISA(SdrTextObj), "SetObjText: Kein SdrTextObj!" );
        Outliner* pOutl = pOutliner;

        if (!pOutliner)
        {
            SfxItemPool* pPool = ((SdDrawDocument*) GetModel())->GetDrawOutliner().GetEmptyItemSet().GetPool();
            pOutl = new Outliner( pPool, OUTLINERMODE_OUTLINEOBJECT );
            pOutl->SetRefDevice( ((SdDrawDocument*) GetModel())->GetDocSh()
                                 ->GetPrinter(TRUE) );
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

        // MT: Wird nicht gebraucht...
        // pOutl->SetUpdateMode(TRUE);

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
#endif // !SVX_LIGHT

/*************************************************************************
|*
|* Link & Daten von einem VControl empfangen
|*
\************************************************************************/

void SdPage::SetLinkData(const String& rLinkName, const String& rLinkData)
{
}

/*************************************************************************
|*
|* Layoutname setzen
|*
\************************************************************************/
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


/*************************************************************************
|*
|* Seitenname zurueckgeben und ggf. generieren
|*
\************************************************************************/

const String& SdPage::GetName()
{
    if (aPageName.Len() == 0)
    {
        if ((ePageKind == PK_STANDARD || ePageKind == PK_NOTES) &&
            !bMaster)
        {
            /**********************************************************************
            * Defaultname fuer Handzettelseiten
            **********************************************************************/
            // Seitennummer per Methode holen, denn die Drawing Engine laesst
            // die Nummern bis zur naechsten Anforderung unsortiert
            USHORT  nNum = (GetPageNum() + 1) / 2;
            BOOL    bUpper = FALSE;

            aCreatedPageName = String(SdResId(STR_PAGE));
            aCreatedPageName += sal_Unicode( ' ' );
            aCreatedPageName += ((SdDrawDocument*) GetModel())->CreatePageNumValue(nNum);
        }
        else
        {
            /******************************************************************
            * Defaultname fuer Handzettelseiten
            ******************************************************************/
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


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdPage::AdjustBackgroundSize()
{
    SdrObject* pObj = GetPresObj(PRESOBJ_BACKGROUND);

    if (pObj)
    {
        // Hintergrund-Objekt verschieben
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



/*************************************************************************
|*
|* Liefert den Default-Text eines PresObjektes zurueck
|*
\************************************************************************/

#ifndef SVX_LIGHT
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
#ifdef STARIMAGE_AVAILABLE
    else if (eObjKind == PRESOBJ_IMAGE)
    {
        aString = String ( SdResId( STR_PRESOBJ_IMAGE ) );
    }
#endif

    return(aString);
}
#endif // !SVX_LIGHT


