/*************************************************************************
 *
 *  $RCSfile: sdpage.cxx,v $
 *
 *  $Revision: 1.46 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:25:09 $
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

#include <algorithm>

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
#include <svx/xoutx.hxx>
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
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
#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif
#ifndef _SVX_SRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif

#ifndef _SVX_FLDITEM_HXX
#define ITEMID_FIELD    EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#define ITEMID_ADJUST EE_PARA_JUST
#include <svx/adjitem.hxx>


#ifndef SD_DRAW_DOC_SHELL_HXX
#ifdef MAC
#include "::ui:inc:DrawDocShell.hxx"
#else
#ifdef UNX
#include "../ui/inc/DrawDocShell.hxx"
#else
#include "..\ui\inc\DrawDocShell.hxx"
#endif
#endif
#endif

#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
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

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#define MAX_PRESOBJ     5              // Max. Anzahl Praesentationsobjekte

using namespace ::sd;
using namespace ::com::sun::star;

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
    nPaperBin(PAPERBIN_PRINTER_SETTINGS),
    mpItems(NULL)
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

SdPage::~SdPage()
{
    DisconnectLink();

    EndListenOutlineText();

    if( mpItems )
        delete mpItems;
}

/** returns the nIndex'th object from the given PresObjKind, index starts with 1 */
SdrObject* SdPage::GetPresObj(PresObjKind eObjKind, int nIndex)
{
    int nObjFound = 0;          // index of the searched object
    SdrObject* pObj = NULL;
    SdrObject* pObjFound = NULL;

    PresentationObjectList::iterator aIter( maPresObjList.begin() );
    const PresentationObjectList::iterator aEnd( maPresObjList.end() );

    while( (aIter != aEnd) && (nObjFound != nIndex) )
    {
        if( (*aIter).meKind == eObjKind )
        {
            pObj = (*aIter).mpObject;
            nObjFound++;    // found one
        }

        aIter++;
    }

    if (nObjFound == nIndex)
    {
        pObjFound = pObj;
    }
    else if(eObjKind==PRESOBJ_TITLE || eObjKind==PRESOBJ_OUTLINE)
    {
        // for title and outline shapes we will try to find them on the page
        nObjFound = 0;
        int nIdx = 0;
        int nCnt = GetObjCount();

        while((nIdx < nCnt) && (nObjFound != nIndex))
        {
            pObj = GetObj(nIdx);
            SdrObjKind eSdrObjKind = (SdrObjKind) pObj->GetObjIdentifier();

            if (pObj->GetObjInventor() == SdrInventor &&
                (eObjKind==PRESOBJ_TITLE   && eSdrObjKind == OBJ_TITLETEXT ||
                eObjKind==PRESOBJ_OUTLINE && eSdrObjKind == OBJ_OUTLINETEXT))
            {
                nObjFound++;    // found one
            }

            nIdx++;
        }

        if (nObjFound == nIndex)
        {
            // we found the searched object on the page
            pObjFound = pObj;
        }
    }

    return pObjFound;
}

/** creates a presentation object with the given PresObjKind on this page. A user call will be set
*/
SdrObject* SdPage::CreatePresObj(PresObjKind eObjKind, BOOL bVertical, const Rectangle& rRect, BOOL bInsert)
{
    SdrObject* pSdrObj = NULL;

    bool bForceText = false;    // forces the shape text to be set even if its empty
    bool bEmptyPresObj = true;

    switch( eObjKind )
    {
        case PRESOBJ_TITLE:
        {
            pSdrObj = new SdrRectObj(OBJ_TITLETEXT);

            if (bMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(TRUE);
            }
        }
        break;

        case PRESOBJ_OUTLINE:
        {
            pSdrObj = new SdrRectObj(OBJ_OUTLINETEXT);

            if (bMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(TRUE);
            }
        }
        break;

        case PRESOBJ_NOTES:
        {
            pSdrObj = new SdrRectObj(OBJ_TEXT);

            if (bMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(TRUE);
            }
        }
        break;

        case PRESOBJ_TEXT:
        {
            pSdrObj = new SdrRectObj(OBJ_TEXT);
        }
        break;

        case PRESOBJ_GRAPHIC:
        {
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_GRAPHIC ) );
            Graphic  aGraphic( aBmpEx );
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
        break;

        case PRESOBJ_OBJECT:
        {
            pSdrObj = new SdrOle2Obj();
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_OBJECT ) );
            Graphic aGraphic( aBmpEx );
            ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
        }
        break;

        case PRESOBJ_CHART:
        {
            pSdrObj = new SdrOle2Obj();
            ( (SdrOle2Obj*) pSdrObj)->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarChart" )));
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_CHART ) );
            Graphic aGraphic( aBmpEx );
            ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
        }
        break;

        case PRESOBJ_ORGCHART:
        {
            pSdrObj = new SdrOle2Obj();
            ( (SdrOle2Obj*) pSdrObj)->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarOrg" )));
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_ORGCHART ) );
            Graphic aGraphic( aBmpEx );
            ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
        }

        case PRESOBJ_TABLE:
        {
            pSdrObj = new SdrOle2Obj();
            ( (SdrOle2Obj*) pSdrObj)->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarCalc" )));
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_TABLE ) );
            Graphic aGraphic( aBmpEx );
            ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
        }
        break;

#ifdef STARIMAGE_AVAILABLE
        case PRESOBJ_IMAGE:
        {
            pSdrObj = new SdrOle2Obj();
            ( (SdrOle2Obj*) pSdrObj)->SetProgName( String( RTL_CONSTASCII_USTRINGPARAM( "StarImage" )));
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_IMAGE ) );
            Graphic aGraphic( aBmpEx );
            ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
        }
        break;
#endif
        case PRESOBJ_BACKGROUND:
        {
            pSdrObj = new SdrRectObj();
            pSdrObj->SetMoveProtect(TRUE);
            pSdrObj->SetResizeProtect(TRUE);
            pSdrObj->SetMarkProtect(TRUE);
        }
        break;

        case PRESOBJ_HANDOUT:
        {
            //Erste Standardseite am SdrPageObj vermerken
            SdrPage* pPage = ( (SdDrawDocument*) pModel )->GetSdPage(0, PK_STANDARD);
            pSdrObj = new SdrPageObj( pPage );
            pSdrObj->SetResizeProtect(TRUE);
        }
        break;

        case PRESOBJ_PAGE:
        {
            //Notizseite am SdrPageObj vermerken
            sal_uInt16 nDestPageNum(GetPageNum());

            if(nDestPageNum)
            {
                // decrement only when != 0, else we get a 0xffff
                nDestPageNum -= 1;
            }

            if(nDestPageNum < pModel->GetPageCount())
            {
                pSdrObj = new SdrPageObj(pModel->GetPage(nDestPageNum));
            }
            else
            {
                pSdrObj = new SdrPageObj();
            }

            pSdrObj->SetResizeProtect(TRUE);
        }
        break;

        case PRESOBJ_HEADER:
        case PRESOBJ_FOOTER:
        case PRESOBJ_DATETIME:
        case PRESOBJ_SLIDENUMBER:
        {
            pSdrObj = new SdrRectObj(OBJ_TEXT);
            bEmptyPresObj = false;
            bForceText = true;
        }
        break;
    }

    if (pSdrObj)
    {
        // #107228# We need to put the shape that will be inserted
        // into the aPresObjList always. This is because the accessibility
        // api creates a XShape wrapper during the call to InsertObject.
        // The creation fails if this is not in the aPresObjList at this point.
        // We will remove it from the aPresObjList after InsertObject since
        // this is needed for the correct working of SetAutoLayout
        const bool bIsAlreadyInserted = IsPresObj(pSdrObj);

        if( !bIsAlreadyInserted )
            InsertPresObj(pSdrObj, eObjKind);

        pSdrObj->SetEmptyPresObj(bEmptyPresObj);
        pSdrObj->SetLogicRect(rRect);

        InsertObject(pSdrObj);

        if( !bIsAlreadyInserted && !bInsert )
            RemovePresObj( pSdrObj );

        if ( pSdrObj->ISA(SdrTextObj) )
        {
            // #96243# Tell the object EARLY that it is vertical to have the
            // defaults for AutoGrowWidth/Height reversed
            if(bVertical)
                ((SdrTextObj*)pSdrObj)->SetVerticalWriting(TRUE);

            SfxItemSet aTempAttr( ((SdDrawDocument*) pModel)->GetPool() );
            if( bVertical )
                aTempAttr.Put( SdrTextMinFrameWidthItem( rRect.GetSize().Width() ) );
            else
                aTempAttr.Put( SdrTextMinFrameHeightItem( rRect.GetSize().Height() ) );

            if (bMaster)
            {
                // Bei Praesentationsobjekten auf der MasterPage soll die
                // Groesse vom Benutzwer frei waehlbar sein

                // #96243# potential problem: This action was still NOT
                // adapted for vertical text. This sure needs to be done.
                if(bVertical)
                    aTempAttr.Put(SdrTextAutoGrowWidthItem(FALSE));
                else
                    aTempAttr.Put(SdrTextAutoGrowHeightItem(FALSE));
            }

            // check if we need another vertical adjustement than the default
            SdrTextVertAdjust eV = SDRTEXTVERTADJUST_TOP;

            if( (eObjKind == PRESOBJ_FOOTER) && (ePageKind != PK_STANDARD) )
            {
                eV = SDRTEXTVERTADJUST_BOTTOM;
            }
            else if( (eObjKind == PRESOBJ_SLIDENUMBER) && (ePageKind != PK_STANDARD) )
            {
                eV = SDRTEXTVERTADJUST_BOTTOM;
            }

            if( eV != SDRTEXTVERTADJUST_TOP )
                aTempAttr.Put(SdrTextVertAdjustItem(eV));

            pSdrObj->SetMergedItemSet(aTempAttr);

            pSdrObj->SetLogicRect(rRect);
        }

        String aString = GetPresObjText(eObjKind);
        if( (aString.Len() || bForceText) && pSdrObj->ISA(SdrTextObj) )
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

        if( (eObjKind == PRESOBJ_HEADER) || (eObjKind == PRESOBJ_FOOTER) || (eObjKind == PRESOBJ_SLIDENUMBER) || (eObjKind == PRESOBJ_DATETIME) )
        {
            SfxItemSet aTempAttr( ((SdDrawDocument*) pModel)->GetPool() );
            aTempAttr.Put( SvxFontHeightItem( 493, 100, EE_CHAR_FONTHEIGHT ) );
            aTempAttr.Put( SvxFontHeightItem( 493, 100, EE_CHAR_FONTHEIGHT_CTL ) );
            aTempAttr.Put( SvxFontHeightItem( 493, 100, EE_CHAR_FONTHEIGHT_CJK ) );

            SvxAdjust eH = SVX_ADJUST_LEFT;

            if( (eObjKind == PRESOBJ_DATETIME) && (ePageKind != PK_STANDARD ) )
            {
                eH = SVX_ADJUST_RIGHT;
            }
            else if( (eObjKind == PRESOBJ_FOOTER) && (ePageKind == PK_STANDARD ) )
            {
                eH = SVX_ADJUST_CENTER;
            }
            else if( eObjKind == PRESOBJ_SLIDENUMBER )
            {
                eH = SVX_ADJUST_RIGHT;
            }

            if( eH != SVX_ADJUST_LEFT )
                aTempAttr.Put(SvxAdjustItem(eH));

            pSdrObj->SetMergedItemSet(aTempAttr);
        }

        pSdrObj->SetUserCall(this);
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
        // #95114# Set style only when one was found (as in 5.2)
        // pSdrObj->NbcSetStyleSheet( GetStyleSheetForPresObj(eObjKind), FALSE );
        if( ePageKind != PK_HANDOUT )
        {
            SfxStyleSheet* pSheetForPresObj = GetStyleSheetForPresObj(eObjKind);
            if(pSheetForPresObj)
                pSdrObj->SetStyleSheet(pSheetForPresObj, FALSE);
        }

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

            pSdrObj->SetMergedItemSet(aSet);
        }
    }

    return(pSdrObj);
}

/*************************************************************************
|*
|* Es werden Praesentationsobjekte auf der Page erzeugt.
|* Alle Praesentationsobjekte erhalten einen UserCall auf die Page.
|*
\************************************************************************/

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

    SfxStyleFamily eFamily = SD_LT_FAMILY;

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
            aName += String(SdResId(STR_LAYOUT_SUBTITLE));
            break;

        case PRESOBJ_HEADER:
        case PRESOBJ_FOOTER:
        case PRESOBJ_DATETIME:
        case PRESOBJ_SLIDENUMBER:
            aName += String(SdResId(STR_LAYOUT_BACKGROUNDOBJECTS));
            break;

        default:
            break;
    }

    SfxStyleSheetBasePool* pStShPool = pModel->GetStyleSheetPool();
    SfxStyleSheetBase*     pResult   = pStShPool->Find(aName, eFamily);
    return (SfxStyleSheet*)pResult;
}

/*************************************************************************
|*
|* Das Praesentationsobjekt rObj hat sich geaendert und wird nicht mehr
|* durch das Praesentationsobjekt der MasterPage referenziert.
|* Der UserCall wird geloescht.
|*
\************************************************************************/

void SdPage::Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect)
{
    if (!bOwnArrangement)
    {
        switch (eType)
        {
            case SDRUSERCALL_MOVEONLY:
            case SDRUSERCALL_RESIZE:
            {
                if( pModel->isLocked() )
                    break;

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

                            if (pPage && this == &(pPage->TRG_GetMasterPage()))
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
            {
                // Ignore the delete call.  This formerly was treated
                // like SDRUSERCALL_REMOVED with the effect that a
                // deleted object was inserted into the
                // DeletedPresObjList which lead to a crash.
                List* pList = ((SdDrawDocument*) pModel)->GetDeletedPresObjList();
                if( pList )
                    pList->Remove((void*) &rObj);

                // this should be redundant, but for safety reasons
                // check if someone deleted a SdrObject without
                // removing it from the presentation object list first
                PresentationObjectList::iterator aIter( FindPresObj( &rObj ) );
                if( aIter != maPresObjList.end() )
                    maPresObjList.erase( aIter );
            }
            break;

            case SDRUSERCALL_REMOVED:
            {
                if( IsPresObj( &rObj ) )
                {
                    // #107844#
                    // Handling of non-empty and empty PresObjs was moved to UndoActionHdl
                    // to allow adding the correct SdrUndoUserCallObj. This may be done here, too,
                    // but it makes more sense to handle all changes to PresObjs in a central
                    // place where the Undo is needed to be fetched anyways.

                    // In die Liste fuers Undo eintragen, da dieses Objekt
                    // durch das Default-Praesentationsobjekt ersetzt werden
                    // soll.
                    // Im UndoActionHdl des DrawDocs wird der UserCall
                    // auf NULL gesetzt und das Obj aus der Liste ausgetragen
                    ((SdrObject&) rObj).SetUserCall(this);
                    List* pList = ((SdDrawDocument*) pModel)->GetDeletedPresObjList();
                    pList->Insert((void*) &rObj, LIST_APPEND);
                }
            }
            break;

            default:
            break;
        }
    }
}

/*************************************************************************
|*
|* Erzeugt auf einer MasterPage Hintergrund, Titel- und Layout-Bereich
|*
\************************************************************************/

void SdPage::CreateTitleAndLayout(BOOL bInit, BOOL bCreate )
{
    SdPage* pMasterPage = this;

    if (!bMaster)
    {
        pMasterPage = (SdPage*)(&(TRG_GetMasterPage()));
    }

    if (!pMasterPage)
    {
        return;
    }

    /**************************************************************************
    * Hintergrund, Titel- und Layout-Bereich werden angelegt
    **************************************************************************/
    if( ePageKind == PK_STANDARD )
    {
        SdrObject* pMasterBackground = pMasterPage->GetPresObj( PRESOBJ_BACKGROUND );
        if( pMasterBackground == NULL )
            pMasterPage->CreateDefaultPresObj(PRESOBJ_BACKGROUND, true);
    }

    if( ( (SdDrawDocument*) GetModel() )->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
    {
        if( ePageKind == PK_HANDOUT && bInit )
        {
            // handout template

            // delete all available handout presentation objects
            PresentationObjectList& rPresObjList = pMasterPage->GetPresObjList();
            PresentationObjectList::iterator aIter( rPresObjList.begin() );
            PresentationObjectList::iterator aEnd( rPresObjList.end() );
            while( aIter != aEnd )
            {
                if( (*aIter).meKind == PRESOBJ_HANDOUT )
                {
                    SdrObject* pObj = (*aIter).mpObject;
                    aIter = rPresObjList.erase( aIter );
                    pMasterPage->RemoveObject(pObj->GetOrdNum());
                }
                else
                {
                    aIter++;
                }
            }

            Size    aArea = GetSize();
            long    nX = GetLftBorder();
            long    nY = GetUppBorder();
            long    nGapW = (nX + GetRgtBorder()) / 2;
            long    nGapH = (nY + GetLwrBorder()) / 2;
            USHORT  nColCnt, nRowCnt;

            const int NOTES_HEADER_FOOTER_HEIGHT = long((aArea.Height() - GetUppBorder() - GetLwrBorder()) * 0.05);

            aArea.Height() -= 2 * NOTES_HEADER_FOOTER_HEIGHT;

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

            Point aPos(nX, nY + NOTES_HEADER_FOOTER_HEIGHT);
            USHORT nPgNum = 0;

            for (USHORT nRow = 0; nRow < nRowCnt; nRow++)
            {
                aPos.X() = nX;

                for (USHORT nCol = 0; nCol < nColCnt; nCol++)
                {
                    Rectangle aRect(aPos, aSize);
                    SdrPageObj* pPageObj = (SdrPageObj*) pMasterPage->CreatePresObj(PRESOBJ_HANDOUT, FALSE, aRect, TRUE);

                    const sal_uInt16 nDestinationPageNum(2 * nPgNum + 1);

                    if(nDestinationPageNum < pModel->GetPageCount())
                    {
                        pPageObj->SetReferencedPage(pModel->GetPage(nDestinationPageNum));
                    }
                    else
                    {
                        pPageObj->SetReferencedPage(0L);
                    }

                    nPgNum++;
                    aPos.X() += aPartArea.Width() + nGapW;
                }
                aPos.Y() += aPartArea.Height() + nGapH;
            }
        }

        if( ePageKind != PK_HANDOUT )
        {
            SdrObject* pMasterTitle = pMasterPage->GetPresObj( PRESOBJ_TITLE );
            if( pMasterTitle == NULL )
                pMasterPage->CreateDefaultPresObj(PRESOBJ_TITLE, true);

            SdrObject* pMasterOutline = pMasterPage->GetPresObj( ePageKind==PK_NOTES ? PRESOBJ_NOTES : PRESOBJ_OUTLINE );
            if( pMasterOutline == NULL )
                pMasterPage->CreateDefaultPresObj( ePageKind == PK_STANDARD ? PRESOBJ_OUTLINE : PRESOBJ_NOTES, true );
        }

        // create header&footer objects

        if( bCreate )
        {
            if( ePageKind != PK_STANDARD )
            {
                SdrObject* pHeader = pMasterPage->GetPresObj( PRESOBJ_HEADER );
                if( pHeader == NULL )
                    pMasterPage->CreateDefaultPresObj( PRESOBJ_HEADER, true );
            }

            SdrObject* pDate   = pMasterPage->GetPresObj( PRESOBJ_DATETIME );
            if( pDate == NULL )
                pMasterPage->CreateDefaultPresObj( PRESOBJ_DATETIME, true );

            SdrObject* pFooter = pMasterPage->GetPresObj( PRESOBJ_FOOTER );
            if( pFooter == NULL )
                pMasterPage->CreateDefaultPresObj( PRESOBJ_FOOTER, true );

            SdrObject* pNumber = pMasterPage->GetPresObj( PRESOBJ_SLIDENUMBER );
            if( pNumber == NULL )
                pMasterPage->CreateDefaultPresObj( PRESOBJ_SLIDENUMBER, true );
        }
    }
}

SdrObject* SdPage::CreateDefaultPresObj(PresObjKind eObjKind, bool bInsert)
{
    if( eObjKind == PRESOBJ_TITLE )
    {
        Rectangle aTitleRect( GetTitleRect() );
        return CreatePresObj(PRESOBJ_TITLE, FALSE, aTitleRect, bInsert);
    }
    else if( eObjKind == PRESOBJ_OUTLINE )
    {
        Rectangle aLayoutRect( GetLayoutRect() );
        return CreatePresObj( PRESOBJ_OUTLINE, FALSE, aLayoutRect, bInsert);
    }
    else if( eObjKind == PRESOBJ_NOTES )
    {
        Rectangle aLayoutRect( GetLayoutRect() );
        return CreatePresObj( PRESOBJ_NOTES, FALSE, aLayoutRect, bInsert);
    }
    else if( (eObjKind == PRESOBJ_FOOTER) || (eObjKind == PRESOBJ_DATETIME) || (eObjKind == PRESOBJ_SLIDENUMBER) || (eObjKind == PRESOBJ_HEADER ) )
    {
        // create footer objects for standard master page
        if( ePageKind == PK_STANDARD )
        {
            const long nLftBorder = GetLftBorder();
            const long nUppBorder = GetUppBorder();

            Point aTitlePos ( nLftBorder, nUppBorder );
            Size aPageSize ( GetSize() );
            aPageSize.Width()  -= nLftBorder + GetRgtBorder();
            aPageSize.Height() -= nUppBorder + GetLwrBorder();

            const int Y = long(nUppBorder + aPageSize.Height() * 0.911);
            const int W1 = long(aPageSize.Width() * 0.233);
            const int W2 = long(aPageSize.Width() * 0.317);
            const int H = long(aPageSize.Height() * 0.069);

            if( eObjKind == PRESOBJ_DATETIME )
            {
                Point aPos( long(nLftBorder+(aPageSize.Width()*0.05)), Y );
                Size aSize( W1, H );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_DATETIME, FALSE, aRect, bInsert );
            }
            else if( eObjKind == PRESOBJ_FOOTER )
            {
                Point aPos( long(nLftBorder+ aPageSize.Width() * 0.342), Y );
                Size aSize( W2, H );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_FOOTER, FALSE, aRect, bInsert );
            }
            else if( eObjKind == PRESOBJ_SLIDENUMBER )
            {
                Point aPos( long(nLftBorder+(aPageSize.Width()*0.717)), Y );
                Size aSize( W1, H );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_SLIDENUMBER, FALSE, aRect, bInsert );
            }
            else
            {
                DBG_ERROR( "SdPage::CreateDefaultPresObj() - can't create a header placeholder for a slide master" );
                return NULL;
            }
        }
        else
        {
            // create header&footer objects for handout and notes master
            Point aTitlePos ( GetLftBorder(), GetUppBorder() );
            Size aPageSize ( GetSize() );
            aPageSize.Width()  -= GetLftBorder() + GetRgtBorder();
            aPageSize.Height() -= GetUppBorder() + GetLwrBorder();


            const int NOTES_HEADER_FOOTER_WIDTH = long(aPageSize.Width() * 0.434);
            const int NOTES_HEADER_FOOTER_HEIGHT = long(aPageSize.Height() * 0.05);

            Size aSize( NOTES_HEADER_FOOTER_WIDTH, NOTES_HEADER_FOOTER_HEIGHT );

            const int X1 = GetLftBorder();
            const int X2 = GetLftBorder() + long(aPageSize.Width() - NOTES_HEADER_FOOTER_WIDTH);
            const int Y1 = GetUppBorder();
            const int Y2 = GetUppBorder() + long(aPageSize.Height() - NOTES_HEADER_FOOTER_HEIGHT );

            if( eObjKind == PRESOBJ_HEADER )
            {
                Point aPos( X1, Y1 );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_HEADER, FALSE, aRect, bInsert );
            }
            else if( eObjKind == PRESOBJ_DATETIME )
            {
                Point aPos( X2, Y1 );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_DATETIME, FALSE, aRect, bInsert );
            }
            else if( eObjKind == PRESOBJ_FOOTER )
            {
                Point aPos( X1, Y2 );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_FOOTER, FALSE, aRect, bInsert );
            }
            else if( eObjKind == PRESOBJ_SLIDENUMBER )
            {
                Point aPos( X2, Y2 );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_SLIDENUMBER, FALSE, aRect, bInsert );
            }

            DBG_ERROR("SdPage::CreateDefaultPresObj() - this should not happen!");
            return NULL;
        }
    }
    else if( eObjKind == PRESOBJ_BACKGROUND )
    {
        Point aBackgroundPos ( GetLftBorder(), GetUppBorder() );
        Size aBackgroundSize ( GetSize() );
        aBackgroundSize.Width()  -= GetLftBorder() + GetRgtBorder() - 1;
        aBackgroundSize.Height() -= GetUppBorder() + GetLwrBorder() - 1;
        Rectangle aBackgroundRect(aBackgroundPos, aBackgroundSize);
        return CreatePresObj( PRESOBJ_BACKGROUND, FALSE, aBackgroundRect, bInsert );
    }
    else
    {
        DBG_ERROR("SdPage::CreateDefaultPresObj() - unknown PRESOBJ kind" );
        return NULL;
    }
}

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
            aTitlePos.X() += long( aTitleSize.Width() * 0.05 );
            aTitlePos.Y() += long( aTitleSize.Height() * 0.0399 );
            aTitleSize.Width() = long( aTitleSize.Width() * 0.9 );
            aTitleSize.Height() = long( aTitleSize.Height() * 0.167 );
        }
        else if (ePageKind == PK_NOTES)
        {
            Point aPos = aTitlePos;
            aPos.Y() += long( aTitleSize.Height() * 0.076 );

            // Hoehe beschraenken
            aTitleSize.Height() = (long) (aTitleSize.Height() * 0.375);

            Size aPartArea = aTitleSize;
            Size aSize;
            sal_uInt16 nDestPageNum(GetPageNum());
            SdrPage* pPage = 0L;

            if(nDestPageNum)
            {
                // only decrement if != 0, else we get 0xffff
                nDestPageNum -= 1;
            }

            if(nDestPageNum < pModel->GetPageCount())
            {
                pPage = pModel->GetPage(nDestPageNum);
            }

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
            aLayoutPos.X() += long( aLayoutSize.Width() * 0.05 );
            aLayoutPos.Y() += long( aLayoutSize.Height() * 0.234 );
            aLayoutSize.Width() = long( aLayoutSize.Width() * 0.9 );
            aLayoutSize.Height() = long( aLayoutSize.Height() * 0.66 );
            aLayoutRect.SetPos(aLayoutPos);
            aLayoutRect.SetSize(aLayoutSize);
        }
        else if (ePageKind == PK_NOTES)
        {
            aLayoutPos.X() += long( aLayoutSize.Width() * 0.1 );
            aLayoutPos.Y() += long( aLayoutSize.Height() * 0.475 );
            aLayoutSize.Width() = long( aLayoutSize.Width() * 0.8 );
            aLayoutSize.Height() = long( aLayoutSize.Height() * 0.45 );
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

void SdPage::SetAutoLayout(AutoLayout eLayout, BOOL bInit, BOOL bCreate )
{
    eAutoLayout = eLayout;
    bOwnArrangement = TRUE;

    CreateTitleAndLayout(bInit, bCreate);

    if ((eAutoLayout == AUTOLAYOUT_NONE && maPresObjList.empty()) || bMaster)
    {
        // MasterPage oder:
        // Kein AutoLayout gewuenscht und keine Praesentationsobjekte
        // vorhanden, also ist nichts zu tun
        bOwnArrangement = FALSE;
        return;
    }

    SdPage& rMasterPage = (SdPage&)TRG_GetMasterPage();

    Rectangle aTitleRect;
    Rectangle aLayoutRect;
    BOOL bFound = FALSE;

    SdrObject* pMasterTitle = rMasterPage.GetPresObj( PRESOBJ_TITLE );
    SdrObject* pMasterOutline = rMasterPage.GetPresObj( ePageKind==PK_NOTES ? PRESOBJ_NOTES : PRESOBJ_OUTLINE );

    if( pMasterTitle )
        aTitleRect = pMasterTitle->GetLogicRect();

    if( pMasterOutline )
        aLayoutRect = pMasterOutline->GetLogicRect();

    if( ePageKind != PK_HANDOUT )
    {
        if (aTitleRect.IsEmpty() )
        {
            /**********************************************************************
            * Titelobj. auf MasterPage nicht vorhanden -> Defaultgroesse bestimmen
            **********************************************************************/
            aTitleRect = GetTitleRect();
        }

        if (aLayoutRect.IsEmpty() )
        {
            /**********************************************************************
            * Gliederungsobj. auf MasterPage nicht vorhanden -> Defaultgroesse bestimmen
            **********************************************************************/
            aLayoutRect = GetLayoutRect();
        }
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
    PresentationObjectList aObjList;
    sal_Bool    bRightToLeft = ( GetModel() && static_cast< SdDrawDocument* >( GetModel() )->GetDefaultWritingMode() == ::com::sun::star::text::WritingMode_RL_TB );
    SdrObject*  pObj;

    switch (eAutoLayout)
    {
        case AUTOLAYOUT_NONE:
        {
            /******************************************************************
            * Notwendig fuer Objekte aus dem Gliederungsmodus
            ******************************************************************/
            nObjKind[0] = PRESOBJ_TITLE;
            pObj = GetPresObj(nObjKind[0]);

            if( pObj && pObj->GetUserCall() )
            {
                pObj->SetLogicRect(aRect0);
                pObj->SetUserCall(this);
            }

            if(pObj && (!pObj->IsEmptyPresObj() || !bInit))
                aObjList.push_back(PresentationObjectDescriptor(pObj, nObjKind[0]));

            nObjKind[1] = PRESOBJ_OUTLINE;
            pObj = GetPresObj(nObjKind[1]);

            if( pObj && pObj->GetUserCall() )
            {
                pObj->SetLogicRect(aRect1);
                pObj->SetUserCall(this);
            }

            if(pObj && (!pObj->IsEmptyPresObj() || !bInit))
                aObjList.push_back(PresentationObjectDescriptor(pObj, nObjKind[1]));
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
                SfxItemSet aNewSet(pObj->GetMergedItemSet());
                aNewSet.Put( SdrTextAutoGrowWidthItem(TRUE) );
                aNewSet.Put( SdrTextAutoGrowHeightItem(FALSE) );
                pObj->SetMergedItemSet(aNewSet);
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
                SfxItemSet aNewSet(pObj->GetMergedItemSet());

                aNewSet.Put( SdrTextAutoGrowWidthItem(TRUE) );
                aNewSet.Put( SdrTextAutoGrowHeightItem(FALSE) );

                // #90790#
                aNewSet.Put( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
                aNewSet.Put( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );

                pObj->SetMergedItemSet(aNewSet);
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
                SfxItemSet aNewSet(pObj->GetMergedItemSet());
                aNewSet.Put( SdrTextAutoGrowWidthItem(TRUE) );
                aNewSet.Put( SdrTextAutoGrowHeightItem(FALSE) );
                pObj->SetMergedItemSet(aNewSet);
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
                SfxItemSet aNewSet(pObj->GetMergedItemSet());

                aNewSet.Put( SdrTextAutoGrowWidthItem(TRUE) );
                aNewSet.Put( SdrTextAutoGrowHeightItem(FALSE) );

                // #90790#
                aNewSet.Put( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
                aNewSet.Put( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );

                pObj->SetMergedItemSet(aNewSet);
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
                SfxItemSet aNewSet(pObj->GetMergedItemSet());

                aNewSet.Put( SdrTextAutoGrowWidthItem(TRUE) );
                aNewSet.Put( SdrTextAutoGrowHeightItem(FALSE) );

                // #90790#
                aNewSet.Put( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
                aNewSet.Put( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );

                pObj->SetMergedItemSet(aNewSet);
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
                SfxItemSet aNewSet(pObj->GetMergedItemSet());

                aNewSet.Put( SdrTextAutoGrowWidthItem(TRUE) );
                aNewSet.Put( SdrTextAutoGrowHeightItem(FALSE) );

                // #90790#
                aNewSet.Put( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
                aNewSet.Put( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );

                pObj->SetMergedItemSet(aNewSet);
            }
        }
        break;

        default:
        break;
    }

    // now delete all empty presentation objects that are no longer
    // used by the new layout and then make the generated presentation
    // object list the one that is used by the page
    {
        PresentationObjectList::iterator aIter( maPresObjList.begin() );
        const PresentationObjectList::iterator aEnd( maPresObjList.end() );

        const PresentationObjectList::iterator aObjListBegin( aObjList.begin() );
        const PresentationObjectList::iterator aObjListEnd( aObjList.end() );

        for(; aIter != aEnd; aIter++)
        {
            if( ::std::find( aObjListBegin, aObjListEnd, (*aIter) ) == aObjListEnd )
            {
                if ( (*aIter).mpObject->IsEmptyPresObj() )
                {
                    delete RemoveObject( (*aIter).mpObject->GetOrdNum() );
                }
            }
        }

        maPresObjList.swap( aObjList );
    }

    bOwnArrangement = FALSE;
}

/*************************************************************************
|*
|* Objekt einfuegen
|*
\************************************************************************/

void SdPage::NbcInsertObject(SdrObject* pObj, ULONG nPos, const SdrInsertReason* pReason)
{
    FmFormPage::NbcInsertObject(pObj, nPos, pReason);

    ((SdDrawDocument*) pModel)->InsertObject(pObj, this);

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

SdrObject* SdPage::RemoveObject(ULONG nObjNum)
{
    SdrObject* pObj = FmFormPage::RemoveObject(nObjNum);

    if(pObj && (pObj->GetUserCall()!=this) && IsPresObj(pObj))
    {
        // Objekt hat keinen UserCall auf diese Seite, es ist jedoch noch in
        // der PresObjList eingetragen -> austragen
        Changed(*pObj, SDRUSERCALL_REMOVED, pObj->GetLastBoundRect());
    }

    ((SdDrawDocument*) pModel)->RemoveObject(pObj, this);

    return(pObj);
}

/*************************************************************************
|*
|* Objekt loeschen, ohne Broadcast
|*
\************************************************************************/

SdrObject* SdPage::NbcRemoveObject(ULONG nObjNum)
{
    SdrObject* pObj = FmFormPage::NbcRemoveObject(nObjNum);

    if (pObj && (pObj->GetUserCall()!=this) && IsPresObj(pObj) )
    {
        // Objekt hat keinen UserCall auf diese Seite, es ist jedoch noch in
        // der PresObjList eingetragen -> austragen
        Changed(*pObj, SDRUSERCALL_REMOVED, pObj->GetLastBoundRect());
    }

    ((SdDrawDocument*) pModel)->RemoveObject(pObj, this);

    return(pObj);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

// #95876# Also overload ReplaceObject methods to realize when
// objects are removed with this mechanism instead of RemoveObject
SdrObject* SdPage::NbcReplaceObject(SdrObject* pNewObj, ULONG nObjNum)
{
    SdrObject* pOldObj = FmFormPage::NbcReplaceObject(pNewObj, nObjNum);
    if(pOldObj && (pOldObj->GetUserCall()!=this) && IsPresObj(pOldObj))
        Changed(*pOldObj, SDRUSERCALL_REMOVED, pOldObj->GetLastBoundRect());
    return pOldObj;
}

// #95876# Also overload ReplaceObject methods to realize when
// objects are removed with this mechanism instead of RemoveObject
SdrObject* SdPage::ReplaceObject(SdrObject* pNewObj, ULONG nObjNum)
{
    SdrObject* pOldObj = FmFormPage::ReplaceObject(pNewObj, nObjNum);
    if(pOldObj && (pOldObj->GetUserCall()!=this) && IsPresObj(pOldObj))
        Changed(*pOldObj, SDRUSERCALL_REMOVED, pOldObj->GetLastBoundRect());
    return pOldObj;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

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


/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdPage::SetBorder(INT32 nLft, INT32 nUpp, INT32 nRgt, INT32 nLwr)
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

void SdPage::SetLftBorder(INT32 nBorder)
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

void SdPage::SetRgtBorder(INT32 nBorder)
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

void SdPage::SetUppBorder(INT32 nBorder)
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

void SdPage::SetLwrBorder(INT32 nBorder)
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

void SdPage::ScaleObjects(const Size& rNewPageSize, const Rectangle& rNewBorderRect, BOOL bScaleAllObj)
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

    ULONG nObjCnt = (bScaleObjects ? GetObjCount() : 0);

    for (ULONG nObj = 0; nObj < nObjCnt; nObj++)
    {
        BOOL bIsPresObjOnMaster = FALSE;
        SfxStyleSheet* pSheet = NULL;

        // Alle Objekte
        pObj = GetObj(nObj);

        if (bMaster && IsPresObj(pObj))
        {
            // Es ist ein Praesentationsobjekt auf der MasterPage
            bIsPresObjOnMaster = TRUE;
        }

        if (pObj)
        {
            USHORT nIndexBackground = 0;
            // #88084# remember aTopLeft as original TopLeft
            Point aTopLeft(pObj->GetCurrentBoundRect().TopLeft());

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
                    // #88084# use aTopLeft as original TopLeft
                    aRefPnt = aTopLeft;
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

                                SvxFontHeightItem& rOldHgt = (SvxFontHeightItem&) rSet.Get(EE_CHAR_FONTHEIGHT);
                                ULONG nFontHeight = rOldHgt.GetHeight();
                                nFontHeight = long(nFontHeight * (double) aFractY);
                                rSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT));

                                if( SFX_ITEM_AVAILABLE == rSet.GetItemState( EE_CHAR_FONTHEIGHT_CJK ) )
                                {
                                    rOldHgt = (SvxFontHeightItem&) rSet.Get(EE_CHAR_FONTHEIGHT_CJK);
                                    nFontHeight = rOldHgt.GetHeight();
                                    nFontHeight = long(nFontHeight * (double) aFractY);
                                    rSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CJK));
                                }

                                if( SFX_ITEM_AVAILABLE == rSet.GetItemState( EE_CHAR_FONTHEIGHT_CTL ) )
                                {
                                    rOldHgt = (SvxFontHeightItem&) rSet.Get(EE_CHAR_FONTHEIGHT_CTL);
                                    nFontHeight = rOldHgt.GetHeight();
                                    nFontHeight = long(nFontHeight * (double) aFractY);
                                    rSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CTL));
                                }

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

                                    SvxFontHeightItem& rOldHgt = (SvxFontHeightItem&) aTempSet.Get(EE_CHAR_FONTHEIGHT);
                                    ULONG nFontHeight = rOldHgt.GetHeight();
                                    nFontHeight = long(nFontHeight * (double) aFractY);
                                    aTempSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT));

                                    if( SFX_ITEM_AVAILABLE == aTempSet.GetItemState( EE_CHAR_FONTHEIGHT_CJK ) )
                                    {
                                        rOldHgt = (SvxFontHeightItem&) aTempSet.Get(EE_CHAR_FONTHEIGHT_CJK);
                                        nFontHeight = rOldHgt.GetHeight();
                                        nFontHeight = long(nFontHeight * (double) aFractY);
                                        aTempSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CJK));
                                    }

                                    if( SFX_ITEM_AVAILABLE == aTempSet.GetItemState( EE_CHAR_FONTHEIGHT_CTL ) )
                                    {
                                        rOldHgt = (SvxFontHeightItem&) aTempSet.Get(EE_CHAR_FONTHEIGHT_CTL);
                                        nFontHeight = rOldHgt.GetHeight();
                                        nFontHeight = long(nFontHeight * (double) aFractY);
                                        aTempSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CTL));
                                    }

                                    // Bullet anpassen
                                    ((SdStyleSheet*) pSheet)->AdjustToFontHeight(aTempSet, FALSE);

                                    // Sonderbehandlung: die INVALIDS auf NULL-Pointer
                                    // zurueckgesetzen (sonst landen INVALIDs oder
                                    // Pointer auf die DefaultItems in der Vorlage;
                                    // beides wuerde die Attribut-Vererbung unterbinden)
                                    aTempSet.ClearInvalidItems();

                                    // Sonderbehandlung: nur die gueltigen Anteile des
                                    // BulletItems
                                    if (aTempSet.GetItemState(EE_PARA_BULLET) == SFX_ITEM_AVAILABLE)
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
                                SfxItemSet& rSet = pSheet->GetItemSet();
                                rSet.Put( SvxFontHeightItem(nFontHeight, EE_CHAR_FONTHEIGHT ));
                                rSet.Put( SvxFontHeightItem(nFontHeight, EE_CHAR_FONTHEIGHT_CJK ));
                                rSet.Put( SvxFontHeightItem(nFontHeight, EE_CHAR_FONTHEIGHT_CTL ));
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
                        ULONG nScriptType = pObj->GetOutlinerParaObject()->GetTextObject().GetScriptType();
                        USHORT nWhich = EE_CHAR_FONTHEIGHT;
                        if ( nScriptType == SCRIPTTYPE_ASIAN )
                            nWhich = EE_CHAR_FONTHEIGHT_CJK;
                        else if ( nScriptType == SCRIPTTYPE_COMPLEX )
                            nWhich = EE_CHAR_FONTHEIGHT_CTL;

                        // #88084# use more modern method to scale the text height
                        sal_uInt32 nFontHeight = ((SvxFontHeightItem&)pObj->GetMergedItem(nWhich)).GetHeight();
                        sal_uInt32 nNewFontHeight = sal_uInt32((double)nFontHeight * (double)aFractY);

                        pObj->SetMergedItem(SvxFontHeightItem(nNewFontHeight, 100, nWhich));
                    }
                }
            }

            if (bScaleObjects && !pObj->IsEdgeObj())
            {
                /**************************************************************
                * Objektposition skalieren
                **************************************************************/
                Point aNewPos;

                // #76447# corrected scaling; only distances may be scaled
                // #88084# use aTopLeft as original TopLeft
                aNewPos.X() = long((aTopLeft.X() - GetLftBorder()) * (double)aFractX) + nLeft;
                aNewPos.Y() = long((aTopLeft.Y() - GetUppBorder()) * (double)aFractY) + nUpper;

                Size aVec(aNewPos.X() - aTopLeft.X(), aNewPos.Y() - aTopLeft.Y());

                if (aVec.Height() != 0 || aVec.Width() != 0)
                {
                    pObj->NbcMove(aVec);
                }

                Rectangle aBoundRect = pObj->GetCurrentBoundRect();

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

                pObj->SetChanged();
                pObj->BroadcastObjectChange();
            }
        }
    }

    bOwnArrangement = FALSE;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

BOOL SdPage::InsertPresObj(SdrObject* pObj, PresObjKind eObjKind, BOOL bVertical,
                            Rectangle aRect, BOOL bInit, PresentationObjectList& rObjList)
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
                        ((SdrTextObj*) pObj)->SetMergedItem(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT));
                    }
                    else
                    {
                        // horizontal activated on once vertical outline object
                        ((SdrTextObj*) pObj)->SetMergedItem(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK));
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
                    pObj->SetMergedItemSet(aTempAttr);
                    pObj->SetLogicRect(aRect);

                    // switch on AutoGrowHeight
                    SfxItemSet aAttr( ((SdDrawDocument*) pModel)->GetPool() );
                    aAttr.Put( SdrTextAutoGrowHeightItem(TRUE) );

                    pObj->SetMergedItemSet(aAttr);
                }

                if ( ((SdrTextObj*) pObj)->IsAutoGrowWidth() )
                {
                    // switch off AutoGrowWidth , set new MinWidth
                    SfxItemSet aTempAttr( ((SdDrawDocument*) pModel)->GetPool() );
                    SdrTextMinFrameWidthItem aMinWidth( aRect.GetSize().Width() );
                    aTempAttr.Put( aMinWidth );
                    aTempAttr.Put( SdrTextAutoGrowWidthItem(FALSE) );
                    pObj->SetMergedItemSet(aTempAttr);
                    pObj->SetLogicRect(aRect);

                    // switch on AutoGrowWidth
                    SfxItemSet aAttr( ((SdDrawDocument*) pModel)->GetPool() );
                    aAttr.Put( SdrTextAutoGrowWidthItem(TRUE) );
                    pObj->SetMergedItemSet(aAttr);
                }
            }
        }
    }

    if (pObj)
    {
        bIncrement = TRUE;
        rObjList.push_back(PresentationObjectDescriptor(pObj, eObjKind));

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
                        ::sd::Outliner* pOutl = ( (SdDrawDocument*) pModel )->GetInternalOutliner( TRUE );
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

                    aSet.Put(pObj->GetMergedItemSet());

                    aSet.ClearItem(EE_PARA_LRSPACE);

                    pObj->SetMergedItemSet(aSet);

                    // Untertitel loeschen
                    RemovePresObj(pSubtitle);
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
                        ::sd::Outliner* pOutl = ( (SdDrawDocument*) pModel )->GetInternalOutliner();
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

                    aSet.Put(pObj->GetMergedItemSet());

                    const SvxLRSpaceItem& rLRItem = (const SvxLRSpaceItem&) aSet.Get(EE_PARA_LRSPACE);
                    SvxLRSpaceItem aNewLRItem(rLRItem);
                    aNewLRItem.SetTxtLeft(0);
                    aSet.Put(aNewLRItem);

                    pObj->SetMergedItemSet(aSet);

                    SfxStyleSheet* pSheet = GetStyleSheetForPresObj(PRESOBJ_TEXT);

                    if (pSheet)
                        pObj->SetStyleSheet(pSheet, TRUE);

                    // Gliederungsobjekt loeschen
                    RemovePresObj(pOutlineObj);
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


/*************************************************************************
|*
|* Liefert den PresObjKind eines Objektes zurueck
|*
\************************************************************************/

PresObjKind SdPage::GetPresObjKind(SdrObject* pObj)
{
    PresentationObjectList::iterator aIter( FindPresObj( pObj ) );
    const PresentationObjectList::iterator aEnd( maPresObjList.end() );

    if( aIter != aEnd )
    {
        return (*aIter).meKind;
    }
    else
    {
        return PRESOBJ_NONE;
    }

/*
    PresObjKind eObjKind = PRESOBJ_NONE;

    if (pObj && IsPresObj(pObj) && pObj->GetObjInventor() == SdrInventor)
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
*/
}

bool SdPage::IsPresObj(const SdrObject* pObj)
{
    return FindPresObj( pObj ) != maPresObjList.end();
}

void SdPage::RemovePresObj(const SdrObject* pObj)
{
    PresentationObjectList::iterator aIter( FindPresObj( pObj ) );
    if( aIter != maPresObjList.end() )
    {
        maPresObjList.erase( aIter );
    }
    else
    {
        DBG_ERROR("presentation object not in list before removal");
    }
}

void SdPage::InsertPresObj(SdrObject* pObj, PresObjKind eKind )
{
    DBG_ASSERT( pObj, "invalid presentation object inserted!" );
    DBG_ASSERT( !IsPresObj(pObj), "presentation object inserted twice!" );
    if( pObj )
        maPresObjList.push_back( PresentationObjectDescriptor( pObj, eKind ) );
}

PresentationObjectList::iterator SdPage::FindPresObj(const SdrObject* pObj)
{
    PresentationObjectList::iterator aIter( maPresObjList.begin() );
    const PresentationObjectList::iterator aEnd( maPresObjList.end() );

    while( aIter != aEnd )
    {
        if( (*aIter).mpObject == pObj )
            break;
        aIter++;
    }

    return aIter;
}

/*************************************************************************
|*
|* BASIC anfordern
|*
\************************************************************************/

void SdPage::RequestBasic()
{
    ::sd::DrawDocShell* pDocShell =
          static_cast<SdDrawDocument*>(GetModel())->GetDocSh();

    if (pDocShell)
    {
        SetBasic( pDocShell->GetBasicManager()->GetLib(0) );
    }
}

/*************************************************************************
|*
|* Text des Objektes setzen
|*
\************************************************************************/

void SdPage::SetObjText(SdrTextObj* pObj, SdrOutliner* pOutliner, PresObjKind eObjKind, const String& rString )
{
    if ( pObj )
    {
        DBG_ASSERT( pObj->ISA(SdrTextObj), "SetObjText: Kein SdrTextObj!" );
        ::Outliner* pOutl = pOutliner;

        if (!pOutliner)
        {
            SfxItemPool* pPool = ((SdDrawDocument*) GetModel())->GetDrawOutliner().GetEmptyItemSet().GetPool();
            pOutl = new ::Outliner( pPool, OUTLINERMODE_OUTLINEOBJECT );
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

        // #95114# Always set the object's StyleSheet at the Outliner to
        // use the current objects StyleSheet. Thus it's the same as in
        // SetText(...).
        // #95114# Moved this implementation from where SetObjText(...) was called
        // to inside this method to work even when outliner is fetched here.
        pOutl->SetStyleSheet(0, pObj->GetStyleSheet());

        String aString;

        switch( eObjKind )
        {
            case PRESOBJ_OUTLINE:
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
            break;

            case PRESOBJ_TITLE:
            {
                pOutl->Init( OUTLINERMODE_TITLEOBJECT );
                pOutl->SetMinDepth(0);
                aString += rString;
            }
            break;

            default:
            {
                pOutl->Init( OUTLINERMODE_TEXTOBJECT );
                pOutl->SetMinDepth(0);
                aString += rString;

                // check if we need to add a text field
                SvxFieldData* pData = NULL;

                switch( eObjKind )
                {
                case PRESOBJ_HEADER:
                    pData = new SvxHeaderField();
                    break;
                case PRESOBJ_FOOTER:
                    pData = new SvxFooterField();
                    break;
                case PRESOBJ_SLIDENUMBER:
                    pData = new SvxPageField();
                    break;
                case PRESOBJ_DATETIME:
                    pData = new SvxDateTimeField();
                    break;
                }

                if( pData )
                {
                    ESelection e;
                    SvxFieldItem aField( *pData );
                    pOutl->QuickInsertField(aField,e);
                    delete pData;
                }
            }
            break;
        }

        pOutl->SetPaperSize( pObj->GetLogicRect().GetSize() );

        if( aString.Len() )
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
|*
|*
\************************************************************************/

void SdPage::SetOrientation( Orientation eOrient)
{
    eOrientation = eOrient;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

Orientation SdPage::GetOrientation() const
{
    return eOrientation;
}

/*************************************************************************
|*
|* Liefert den Default-Text eines PresObjektes zurueck
|*
\************************************************************************/

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

extern uno::Reference< uno::XInterface > createUnoPageImpl( SdPage* pPage );

uno::Reference< uno::XInterface > SdPage::createUnoPage()
{
    return createUnoPageImpl( this );
}




void SdPage::SetName (const String& rName)
{
    FmFormPage::SetName (rName);
    ActionChanged();
}




HeaderFooterSettings& SdPage::getHeaderFooterSettings()
{
    if( ePageKind == PK_HANDOUT && !bMaster )
    {
        return (((SdPage&)TRG_GetMasterPage()).maHeaderFooterSettings);
    }
    else
    {
        return maHeaderFooterSettings;
    }
}

bool SdPage::checkVisibility(
    ::sdr::contact::ViewObjectContact& rOriginal,
    ::sdr::contact::DisplayInfo& rDisplayInfo,
    bool bEdit )
{
    if( !FmFormPage::checkVisibility( rOriginal, rDisplayInfo, bEdit ) )
        return false;

    SdrObject* pObj = rOriginal.GetViewContact().TryToGetSdrObject();
    if( pObj == NULL )
        return false;

    const bool bIsPrinting(rDisplayInfo.OutputToPrinter());
    const SdrPageView* pPageView = rDisplayInfo.GetPageView();
    const bool bIsInsidePageObj(pPageView && pPageView->GetPage() != rDisplayInfo.GetProcessedPage());

    // empty presentation objects only visible during edit mode
    if( (bIsPrinting || !bEdit || bIsInsidePageObj ) && pObj->IsEmptyPresObj() )
    {
        if( (pObj->GetObjInventor() != SdrInventor) || ( (pObj->GetObjIdentifier() != OBJ_RECT) && (pObj->GetObjIdentifier() != OBJ_PAGE) ) )
            return false;
    }

    if( ( pObj->GetObjInventor() == SdrInventor ) && ( pObj->GetObjIdentifier() == OBJ_TEXT ) )
    {
        SdPage* pPage = (SdPage*)pObj->GetPage();
        if( pPage )
        {
            PresObjKind eKind = pPage->GetPresObjKind(pObj);

            if((eKind == PRESOBJ_FOOTER) || (eKind == PRESOBJ_HEADER) || (eKind == PRESOBJ_DATETIME) || (eKind == PRESOBJ_SLIDENUMBER) )
            {
                const bool bMasterObj(rDisplayInfo.GetMasterPagePainting());
                if( bMasterObj || ( pPage->GetPageKind() == PK_HANDOUT && bIsPrinting ) )
                {
                    if( pPageView )
                    {
                        // get the page that is currently painted
                        SdPage* pPage = (SdPage*)rDisplayInfo.GetProcessedPage();

                        if( pPage )
                        {
                            // if we are not on a masterpage, see if we have to draw this header&footer object at all
                            const sd::HeaderFooterSettings& rSettings = pPage->getHeaderFooterSettings();

                            switch( eKind )
                            {
                            case PRESOBJ_FOOTER:
                                return rSettings.mbFooterVisible;
                            case PRESOBJ_HEADER:
                                return rSettings.mbHeaderVisible;
                            case PRESOBJ_DATETIME:
                                return rSettings.mbDateTimeVisible;
                            case PRESOBJ_SLIDENUMBER:
                                return rSettings.mbSlideNumberVisible;
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

HeaderFooterSettings::HeaderFooterSettings()
{
    mbHeaderVisible = true;
    mbFooterVisible = true;
    mbSlideNumberVisible = false;
    mbDateTimeVisible = true;
    mbDateTimeIsFixed = true;
    meDateTimeFormat = SVXDATEFORMAT_A;
}

bool HeaderFooterSettings::operator==( const HeaderFooterSettings& rSettings )
{
    return (mbHeaderVisible == rSettings.mbHeaderVisible) &&
           (maHeaderText == rSettings.maHeaderText) &&
           (mbFooterVisible == rSettings.mbFooterVisible) &&
           (maFooterText == rSettings.maFooterText) &&
           (mbSlideNumberVisible == rSettings.mbSlideNumberVisible) &&
           (mbDateTimeVisible == rSettings.mbDateTimeVisible) &&
           (mbDateTimeIsFixed == rSettings.mbDateTimeIsFixed) &&
           (meDateTimeFormat == rSettings.meDateTimeFormat) &&
           (maDateTimeText == rSettings.maDateTimeText);
}

// stl functions to use with the presentation object list

void sd::removePresObj_func::operator() (PresentationObjectDescriptor x)
{
    delete mpPage->RemoveObject(x.mpObject->GetOrdNum());
}
