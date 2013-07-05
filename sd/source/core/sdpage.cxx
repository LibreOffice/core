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


#include <algorithm>

#include <comphelper/classids.hxx>

#include <vcl/svapp.hxx>
#include <editeng/outliner.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editdata.hxx>
#include <svx/pageitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/bulletitem.hxx>
#include <svx/svdpagv.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdopage.hxx>
#include <sfx2/printer.hxx>
#include <basic/basmgr.hxx>
#include <editeng/pbinitem.hxx>
#include <svx/svdundo.hxx>
#include <svl/smplhint.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svx/unopage.hxx>
#include <editeng/flditem.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svditer.hxx>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <rtl/ustring.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <unotools/streamwrap.hxx>
#include <rtl/uri.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <osl/file.h>

#include "../ui/inc/DrawDocShell.hxx"
#include "Outliner.hxx"
#include "app.hrc"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "pglink.hxx"
#include "sdresid.hxx"
#include "stlsheet.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "helpids.h"
#include "anminfo.hxx"
#include "undo/undomanager.hxx"
#include "undo/undoobjects.hxx"
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/unoapi.hxx>

#include <set>

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using ::com::sun::star::uno::Reference;

using ::com::sun::star::io::XInputStream;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::beans::PropertyValue;


TYPEINIT2( SdPage, FmFormPage, SdrObjUserCall );

/*************************************************************************
|*
|*      Ctor
|*
\************************************************************************/

SdPage::SdPage(SdDrawDocument& rNewDoc, StarBASIC* pBasic, sal_Bool bMasterPage)
:   FmFormPage(rNewDoc, pBasic, bMasterPage)
,   SdrObjUserCall()
,   mePageKind(PK_STANDARD)
,   meAutoLayout(AUTOLAYOUT_NONE)
,   mbSelected(sal_False)
,   mePresChange(PRESCHANGE_MANUAL)
,   mfTime(1.0)
,   mbSoundOn(sal_False)
,   mbExcluded(sal_False)
,   mbLoopSound(sal_False)
,   mbStopSound(sal_False)
,   mbScaleObjects(sal_True)
,   mbBackgroundFullSize( sal_False )
,   meCharSet(osl_getThreadTextEncoding())
,   mnPaperBin(PAPERBIN_PRINTER_SETTINGS)
,   mpPageLink(NULL)
,   mpItems(NULL)
,   mnTransitionType(0)
,   mnTransitionSubtype(0)
,   mbTransitionDirection(sal_True)
,   mnTransitionFadeColor(0)
,   mfTransitionDuration(2.0)
,   mbIsPrecious(true)
{
    // The name of the layout of the page is used by SVDRAW to determine the
    // presentation template of the outline objects. Therefore, it already
    // contains the designator for the outline (STR_LAYOUT_OUTLINE).
    OUStringBuffer aBuf(SdResId(STR_LAYOUT_DEFAULT_NAME).toString());
    aBuf.append(SD_LT_SEPARATOR).append(SdResId(STR_LAYOUT_OUTLINE).toString());
    maLayoutName = aBuf.makeStringAndClear();

    Size aPageSize(GetSize());

    if (aPageSize.Width() > aPageSize.Height())
    {
        meOrientation = ORIENTATION_LANDSCAPE;
    }
    else
    {
        meOrientation = ORIENTATION_PORTRAIT;
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

struct OrdNumSorter
{
    bool operator()( SdrObject* p1, SdrObject* p2 )
    {
        return p1->GetOrdNum() < p2->GetOrdNum();
    }
};

/** returns the nIndex'th object from the given PresObjKind, index starts with 1 */
SdrObject* SdPage::GetPresObj(PresObjKind eObjKind, int nIndex, bool bFuzzySearch /* = false */ )
{
    // first sort all matching shapes with z-order
    std::vector< SdrObject* > aMatches;

    SdrObject* pObj = 0;
    maPresentationShapeList.seekShape(0);

    while( (pObj = maPresentationShapeList.getNextShape()) )
    {
        SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pObj);
        if( pInfo )
        {
            bool bFound = false;
            if( pInfo->mePresObjKind == eObjKind )
            {
                bFound = true;
            }
            else if( bFuzzySearch && (eObjKind == PRESOBJ_OUTLINE) )
            {
                switch( pInfo->mePresObjKind )
                {
                case PRESOBJ_GRAPHIC:
                case PRESOBJ_OBJECT:
                case PRESOBJ_CHART:
                case PRESOBJ_ORGCHART:
                case PRESOBJ_TABLE:
                case PRESOBJ_CALC:
                case PRESOBJ_IMAGE:
                case PRESOBJ_MEDIA:
                    bFound = sal_True;
                    break;
                default:
                    break;
                }
            }
            if( bFound )
            {
                aMatches.push_back( pObj );
            }
        }
    }

    if( aMatches.size() > 1 )
    {
        OrdNumSorter aSortHelper;
        std::sort( aMatches.begin(), aMatches.end(), aSortHelper );
    }

    if( nIndex > 0 )
        nIndex--;

    if( (nIndex >= 0) && ( aMatches.size() > static_cast<unsigned int>(nIndex)) )
        return aMatches[nIndex];

    return 0;
}

/** create background properties */
void SdPage::EnsureMasterPageDefaultBackground()
{
    if(mbMaster)
    {
        // no hard attributes on MasterPage attributes
        getSdrPageProperties().ClearItem();
        SfxStyleSheet* pSheetForPresObj = GetStyleSheetForMasterPageBackground();

        if(pSheetForPresObj)
        {
            // set StyleSheet for background fill attributes
            getSdrPageProperties().SetStyleSheet(pSheetForPresObj);
        }
        else
        {
            // no style found, assert and set at least XFILL_NONE
            OSL_FAIL("No Style for MasterPageBackground fill found (!)");
            getSdrPageProperties().PutItem(XFillStyleItem(XFILL_NONE));
        }
    }
}

/** creates a presentation object with the given PresObjKind on this page. A user call will be set
*/
SdrObject* SdPage::CreatePresObj(PresObjKind eObjKind, sal_Bool bVertical, const Rectangle& rRect, sal_Bool /* bInsert */ )
{
    ::svl::IUndoManager* pUndoManager = pModel ? static_cast<SdDrawDocument*>(pModel)->GetUndoManager() : 0;
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    SdrObject* pSdrObj = NULL;

    bool bForceText = false;    // forces the shape text to be set even if its empty
    bool bEmptyPresObj = true;

    switch( eObjKind )
    {
        case PRESOBJ_TITLE:
        {
            pSdrObj = new SdrRectObj(OBJ_TITLETEXT);

            if (mbMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(sal_True);
            }
        }
        break;

        case PRESOBJ_OUTLINE:
        {
            pSdrObj = new SdrRectObj(OBJ_OUTLINETEXT);

            if (mbMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(sal_True);
            }
        }
        break;

        case PRESOBJ_NOTES:
        {
            pSdrObj = new SdrRectObj(OBJ_TEXT);

            if (mbMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(sal_True);
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

        case PRESOBJ_MEDIA:
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
            ( (SdrOle2Obj*) pSdrObj)->SetProgName( OUString( "StarChart" ) );
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_CHART ) );
            Graphic aGraphic( aBmpEx );
            ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
        }
        break;

        case PRESOBJ_ORGCHART:
        {
            pSdrObj = new SdrOle2Obj();
            ( (SdrOle2Obj*) pSdrObj)->SetProgName( OUString( "StarOrg" ) );
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_ORGCHART ) );
            Graphic aGraphic( aBmpEx );
            ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
        }
        break;

        case PRESOBJ_TABLE:
        case PRESOBJ_CALC:
        {
            pSdrObj = new SdrOle2Obj();
            ( (SdrOle2Obj*) pSdrObj)->SetProgName( OUString( "StarCalc" ) );
            BitmapEx aBmpEx( SdResId( BMP_PRESOBJ_TABLE ) );
            Graphic aGraphic( aBmpEx );
            ( (SdrOle2Obj*) pSdrObj)->SetGraphic(&aGraphic);
        }
        break;

        case PRESOBJ_HANDOUT:
        {
            // Save the first standard page at SdrPageObj
            // #i105146# We want no content to be displayed for PK_HANDOUT,
            // so just never set a page as content
            pSdrObj = new SdrPageObj(0);
        }
        break;

        case PRESOBJ_PAGE:
        {
            // Save note pages at SdrPageObj
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

            pSdrObj->SetResizeProtect(sal_True);
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
        default:
            break;
    }

    if (pSdrObj)
    {
        pSdrObj->SetEmptyPresObj(bEmptyPresObj);
        pSdrObj->SetLogicRect(rRect);

        InsertObject(pSdrObj);

        if ( pSdrObj->ISA(SdrTextObj) )
        {
            // Tell the object EARLY that it is vertical to have the
            // defaults for AutoGrowWidth/Height reversed
            if(bVertical)
                ((SdrTextObj*)pSdrObj)->SetVerticalWriting(sal_True);

            SfxItemSet aTempAttr( ((SdDrawDocument*) pModel)->GetPool() );
            if( bVertical )
                aTempAttr.Put( SdrTextMinFrameWidthItem( rRect.GetSize().Width() ) );
            else
                aTempAttr.Put( SdrTextMinFrameHeightItem( rRect.GetSize().Height() ) );

            if (mbMaster)
            {
                // The size of presentation objects on the master page have to
                // be freely selectable by the user.

                // potential problem: This action was still NOT
                // adapted for vertical text. This sure needs to be done.
                if(bVertical)
                    aTempAttr.Put(SdrTextAutoGrowWidthItem(sal_False));
                else
                    aTempAttr.Put(SdrTextAutoGrowHeightItem(sal_False));
            }

            // check if we need another vertical adjustement than the default
            SdrTextVertAdjust eV = SDRTEXTVERTADJUST_TOP;

            if( (eObjKind == PRESOBJ_FOOTER) && (mePageKind != PK_STANDARD) )
            {
                eV = SDRTEXTVERTADJUST_BOTTOM;
            }
            else if( (eObjKind == PRESOBJ_SLIDENUMBER) && (mePageKind != PK_STANDARD) )
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

            sal_uInt16 nOutlMode = pOutliner->GetMode();
            pOutliner->Init( OUTLINERMODE_TEXTOBJECT );
            pOutliner->SetStyleSheet( 0, NULL );
            pOutliner->SetVertical( bVertical );

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

            if( (eObjKind == PRESOBJ_DATETIME) && (mePageKind != PK_STANDARD ) )
            {
                eH = SVX_ADJUST_RIGHT;
            }
            else if( (eObjKind == PRESOBJ_FOOTER) && (mePageKind == PK_STANDARD ) )
            {
                eH = SVX_ADJUST_CENTER;
            }
            else if( eObjKind == PRESOBJ_SLIDENUMBER )
            {
                eH = SVX_ADJUST_RIGHT;
            }

            if( eH != SVX_ADJUST_LEFT )
                aTempAttr.Put(SvxAdjustItem(eH, EE_PARA_JUST ));

            pSdrObj->SetMergedItemSet(aTempAttr);
        }

        if (mbMaster)
        {
            SdrLayerAdmin& rLayerAdmin = pModel->GetLayerAdmin();

            // background objects of the master page
            pSdrObj->SetLayer( rLayerAdmin.
                GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), sal_False) );
        }

        // Subscribe object at the style sheet
        // Set style only when one was found (as in 5.2)
        if( mePageKind != PK_HANDOUT )
        {
            SfxStyleSheet* pSheetForPresObj = GetStyleSheetForPresObj(eObjKind);
            if(pSheetForPresObj)
                pSdrObj->SetStyleSheet(pSheetForPresObj, sal_False);
        }

        if (eObjKind == PRESOBJ_OUTLINE)
        {
            for (sal_uInt16 nLevel = 1; nLevel < 10; nLevel++)
            {
                String aName(maLayoutName);
                aName += sal_Unicode( ' ' );
                aName += OUString::number( nLevel );
                SfxStyleSheet* pSheet = (SfxStyleSheet*)pModel->GetStyleSheetPool()->Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
                DBG_ASSERT(pSheet, "StyleSheet for outline object not found");
                if (pSheet)
                    pSdrObj->StartListening(*pSheet);
            }
        }

        if ( eObjKind == PRESOBJ_OBJECT   ||
             eObjKind == PRESOBJ_CHART    ||
             eObjKind == PRESOBJ_ORGCHART ||
             eObjKind == PRESOBJ_CALC    ||
             eObjKind == PRESOBJ_GRAPHIC )
        {
            SfxItemSet aSet( ((SdDrawDocument*) pModel)->GetPool() );
            aSet.Put( SdrTextContourFrameItem( sal_True ) );
            aSet.Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );

            pSdrObj->SetMergedItemSet(aSet);
        }

        if( bUndo )
        {
            pUndoManager->AddUndoAction(pModel->GetSdrUndoFactory().CreateUndoNewObject(*pSdrObj));
        }

        if( bUndo )
        {
            pUndoManager->AddUndoAction( new UndoObjectPresentationKind( *pSdrObj ) );
            pUndoManager->AddUndoAction( new UndoObjectUserCall(*pSdrObj) );
        }

        InsertPresObj(pSdrObj, eObjKind);
        pSdrObj->SetUserCall(this);

        pSdrObj->RecalcBoundRect();
    }

    return(pSdrObj);
}

/*************************************************************************
|*
|* Creates presentation objects on the master page.
|* All presentation objects get a UserCall to the page.
|*
\************************************************************************/

SfxStyleSheet* SdPage::GetStyleSheetForMasterPageBackground() const
{
    String aName(GetLayoutName());
    String aSep( SD_LT_SEPARATOR );
    sal_uInt16 nPos = aName.Search(aSep);

    if (nPos != STRING_NOTFOUND)
    {
        nPos = nPos + aSep.Len();
        aName.Erase(nPos);
    }

    aName += SD_RESSTR(STR_LAYOUT_BACKGROUND);

    SfxStyleSheetBasePool* pStShPool = pModel->GetStyleSheetPool();
    SfxStyleSheetBase*     pResult   = pStShPool->Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
    return (SfxStyleSheet*)pResult;
}

SfxStyleSheet* SdPage::GetStyleSheetForPresObj(PresObjKind eObjKind) const
{
    String aName(GetLayoutName());
    String aSep( SD_LT_SEPARATOR );
    sal_uInt16 nPos = aName.Search(aSep);
    if (nPos != STRING_NOTFOUND)
    {
        nPos = nPos + aSep.Len();
        aName.Erase(nPos);
    }

    switch (eObjKind)
    {
        case PRESOBJ_OUTLINE:
        {
            aName = GetLayoutName();
            aName += sal_Unicode( ' ' );
            aName += OUString::number( 1 );
        }
        break;

        case PRESOBJ_TITLE:
            aName += SD_RESSTR(STR_LAYOUT_TITLE);
            break;

        case PRESOBJ_NOTES:
            aName += SD_RESSTR(STR_LAYOUT_NOTES);
            break;

        case PRESOBJ_TEXT:
            aName += SD_RESSTR(STR_LAYOUT_SUBTITLE);
            break;

        case PRESOBJ_HEADER:
        case PRESOBJ_FOOTER:
        case PRESOBJ_DATETIME:
        case PRESOBJ_SLIDENUMBER:
            aName += SD_RESSTR(STR_LAYOUT_BACKGROUNDOBJECTS);
            break;

        default:
            break;
    }

    SfxStyleSheetBasePool* pStShPool = pModel->GetStyleSheetPool();
    SfxStyleSheetBase*     pResult   = pStShPool->Find(aName, SD_STYLE_FAMILY_MASTERPAGE);
    return (SfxStyleSheet*)pResult;
}

/** returns the presentation style with the given helpid from this masterpage or this
    slides masterpage */
SdStyleSheet* SdPage::getPresentationStyle( sal_uInt32 nHelpId ) const
{
    String aStyleName( pPage->GetLayoutName() );
    const String aSep( SD_LT_SEPARATOR );
    aStyleName.Erase(aStyleName.Search(aSep) + aSep.Len());

    sal_uInt16 nNameId;
    switch( nHelpId )
    {
    case HID_PSEUDOSHEET_TITLE:             nNameId = STR_LAYOUT_TITLE;             break;
    case HID_PSEUDOSHEET_SUBTITLE:          nNameId = STR_LAYOUT_SUBTITLE;          break;
    case HID_PSEUDOSHEET_OUTLINE1:
    case HID_PSEUDOSHEET_OUTLINE2:
    case HID_PSEUDOSHEET_OUTLINE3:
    case HID_PSEUDOSHEET_OUTLINE4:
    case HID_PSEUDOSHEET_OUTLINE5:
    case HID_PSEUDOSHEET_OUTLINE6:
    case HID_PSEUDOSHEET_OUTLINE7:
    case HID_PSEUDOSHEET_OUTLINE8:
    case HID_PSEUDOSHEET_OUTLINE9:          nNameId = STR_LAYOUT_OUTLINE;           break;
    case HID_PSEUDOSHEET_BACKGROUNDOBJECTS: nNameId = STR_LAYOUT_BACKGROUNDOBJECTS; break;
    case HID_PSEUDOSHEET_BACKGROUND:        nNameId = STR_LAYOUT_BACKGROUND;        break;
    case HID_PSEUDOSHEET_NOTES:             nNameId = STR_LAYOUT_NOTES;             break;

    default:
        OSL_FAIL( "SdPage::getPresentationStyle(), illegal argument!" );
        return 0;
    }
    aStyleName.Append( SD_RESSTR( nNameId ) );
    if( nNameId == STR_LAYOUT_OUTLINE )
    {
        aStyleName.Append( sal_Unicode( ' ' ));
        aStyleName.Append( OUString::number( sal_Int32( nHelpId - HID_PSEUDOSHEET_OUTLINE )));
    }

    SfxStyleSheetBasePool* pStShPool = pModel->GetStyleSheetPool();
    SfxStyleSheetBase*     pResult   = pStShPool->Find(aStyleName, SD_STYLE_FAMILY_MASTERPAGE);
    return dynamic_cast<SdStyleSheet*>(pResult);
}

/*************************************************************************
|*
|* The presentation object rObj has changed and is no longer referenzed by the
|* presentation object of the master page.
|* The UserCall is deleted.
|*
\************************************************************************/

void SdPage::Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& )
{
    if (!maLockAutoLayoutArrangement.isLocked())
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
                    if (!mbMaster)
                    {
                        if( pObj->GetUserCall() )
                        {
                            ::svl::IUndoManager* pUndoManager = pModel ? static_cast<SdDrawDocument*>(pModel)->GetUndoManager() : 0;
                            const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

                            if( bUndo )
                                pUndoManager->AddUndoAction( new UndoObjectUserCall(*pObj) );

                            // Objekt was resized by user and does not listen to its slide anymore
                            pObj->SetUserCall(0);
                        }
                    }
                    else if (pModel)
                    {
                        // Object of the master page changed, therefore adjust
                        // object on all pages
                        sal_uInt16 nPageCount = ((SdDrawDocument*) pModel)->GetSdPageCount(mePageKind);

                        for (sal_uInt16 i = 0; i < nPageCount; i++)
                        {
                            SdPage* pLoopPage = ((SdDrawDocument*) pModel)->GetSdPage(i, mePageKind);

                            if (pLoopPage && this == &(pLoopPage->TRG_GetMasterPage()))
                            {
                                // Page listens to this master page, therefore
                                // adjust AutoLayout
                                pLoopPage->SetAutoLayout(pLoopPage->GetAutoLayout());
                            }
                        }
                    }
                }
            }
            break;

            case SDRUSERCALL_DELETE:
            case SDRUSERCALL_REMOVED:
            default:
                break;
        }
    }
}

/*************************************************************************
|*
|* Creates on a master page: background, title- and layout area
|*
\************************************************************************/

void SdPage::CreateTitleAndLayout(sal_Bool bInit, sal_Bool bCreate )
{
    ::svl::IUndoManager* pUndoManager = pModel ? static_cast<SdDrawDocument*>(pModel)->GetUndoManager() : 0;
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    SdPage* pMasterPage = this;

    if (!mbMaster)
    {
        pMasterPage = (SdPage*)(&(TRG_GetMasterPage()));
    }

    if (!pMasterPage)
    {
        return;
    }

    /**************************************************************************
    * create background, title- and layout area
    **************************************************************************/
    if( mePageKind == PK_STANDARD )
    {
        pMasterPage->EnsureMasterPageDefaultBackground();
    }

    if( ( (SdDrawDocument*) GetModel() )->GetDocumentType() == DOCUMENT_TYPE_IMPRESS )
    {
        if( mePageKind == PK_HANDOUT && bInit )
        {
            // handout template

            // delete all available handout presentation objects
            SdrObject *pObj=NULL;
            while( (pObj = pMasterPage->GetPresObj(PRESOBJ_HANDOUT)) != 0 )
            {
                pMasterPage->RemoveObject(pObj->GetOrdNum());

                if( bUndo )
                {
                    pUndoManager->AddUndoAction(pModel->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
                }
                else
                {
                    SdrObject::Free( pObj );
                }
            }

            std::vector< Rectangle > aAreas;
            CalculateHandoutAreas( *static_cast< SdDrawDocument* >(GetModel() ), pMasterPage->GetAutoLayout(), false, aAreas );

            const bool bSkip = pMasterPage->GetAutoLayout() == AUTOLAYOUT_HANDOUT3;
            std::vector< Rectangle >::iterator iter( aAreas.begin() );

            while( iter != aAreas.end() )
            {
                SdrPageObj* pPageObj = static_cast<SdrPageObj*>(pMasterPage->CreatePresObj(PRESOBJ_HANDOUT, sal_False, (*iter++), sal_True) );
                // #i105146# We want no content to be displayed for PK_HANDOUT,
                // so just never set a page as content
                pPageObj->SetReferencedPage(0L);

                if( bSkip && iter != aAreas.end() )
                    ++iter;
            }
        }

        if( mePageKind != PK_HANDOUT )
        {
            SdrObject* pMasterTitle = pMasterPage->GetPresObj( PRESOBJ_TITLE );
            if( pMasterTitle == NULL )
                pMasterPage->CreateDefaultPresObj(PRESOBJ_TITLE, true);

            SdrObject* pMasterOutline = pMasterPage->GetPresObj( mePageKind==PK_NOTES ? PRESOBJ_NOTES : PRESOBJ_OUTLINE );
            if( pMasterOutline == NULL )
                pMasterPage->CreateDefaultPresObj( mePageKind == PK_STANDARD ? PRESOBJ_OUTLINE : PRESOBJ_NOTES, true );
        }

        // create header&footer objects

        if( bCreate )
        {
            if( mePageKind != PK_STANDARD )
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
        return CreatePresObj(PRESOBJ_TITLE, sal_False, aTitleRect, bInsert);
    }
    else if( eObjKind == PRESOBJ_OUTLINE )
    {
        Rectangle aLayoutRect( GetLayoutRect() );
        return CreatePresObj( PRESOBJ_OUTLINE, sal_False, aLayoutRect, bInsert);
    }
    else if( eObjKind == PRESOBJ_NOTES )
    {
        Rectangle aLayoutRect( GetLayoutRect() );
        return CreatePresObj( PRESOBJ_NOTES, sal_False, aLayoutRect, bInsert);
    }
    else if( (eObjKind == PRESOBJ_FOOTER) || (eObjKind == PRESOBJ_DATETIME) || (eObjKind == PRESOBJ_SLIDENUMBER) || (eObjKind == PRESOBJ_HEADER ) )
    {
        // create footer objects for standard master page
        if( mePageKind == PK_STANDARD )
        {
            const long nLftBorder = GetLftBorder();
            const long nUppBorder = GetUppBorder();

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
                return CreatePresObj( PRESOBJ_DATETIME, sal_False, aRect, bInsert );
            }
            else if( eObjKind == PRESOBJ_FOOTER )
            {
                Point aPos( long(nLftBorder+ aPageSize.Width() * 0.342), Y );
                Size aSize( W2, H );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_FOOTER, sal_False, aRect, bInsert );
            }
            else if( eObjKind == PRESOBJ_SLIDENUMBER )
            {
                Point aPos( long(nLftBorder+(aPageSize.Width()*0.717)), Y );
                Size aSize( W1, H );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_SLIDENUMBER, sal_False, aRect, bInsert );
            }
            else
            {
                OSL_FAIL( "SdPage::CreateDefaultPresObj() - can't create a header placeholder for a slide master" );
                return NULL;
            }
        }
        else
        {
            // create header&footer objects for handout and notes master
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
                return CreatePresObj( PRESOBJ_HEADER, sal_False, aRect, bInsert );
            }
            else if( eObjKind == PRESOBJ_DATETIME )
            {
                Point aPos( X2, Y1 );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_DATETIME, sal_False, aRect, bInsert );
            }
            else if( eObjKind == PRESOBJ_FOOTER )
            {
                Point aPos( X1, Y2 );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_FOOTER, sal_False, aRect, bInsert );
            }
            else if( eObjKind == PRESOBJ_SLIDENUMBER )
            {
                Point aPos( X2, Y2 );
                Rectangle aRect( aPos, aSize );
                return CreatePresObj( PRESOBJ_SLIDENUMBER, sal_False, aRect, bInsert );
            }

            OSL_FAIL("SdPage::CreateDefaultPresObj() - this should not happen!");
            return NULL;
        }
    }
    else
    {
        OSL_FAIL("SdPage::CreateDefaultPresObj() - unknown PRESOBJ kind" );
        return NULL;
    }
}

/*************************************************************************
|*
|* return title area
|*
\************************************************************************/

Rectangle SdPage::GetTitleRect() const
{
    Rectangle aTitleRect;

    if (mePageKind != PK_HANDOUT)
    {
        /******************************************************************
        * standard- or note page: title area
        ******************************************************************/
        Point aTitlePos ( GetLftBorder(), GetUppBorder() );
        Size aTitleSize ( GetSize() );
        aTitleSize.Width()  -= GetLftBorder() + GetRgtBorder();
        aTitleSize.Height() -= GetUppBorder() + GetLwrBorder();

        if (mePageKind == PK_STANDARD)
        {
            aTitlePos.X() += long( aTitleSize.Width() * 0.05 );
            aTitlePos.Y() += long( aTitleSize.Height() * 0.0399 );
            aTitleSize.Width() = long( aTitleSize.Width() * 0.9 );
            aTitleSize.Height() = long( aTitleSize.Height() * 0.167 );
        }
        else if (mePageKind == PK_NOTES)
        {
            Point aPos = aTitlePos;
            aPos.Y() += long( aTitleSize.Height() * 0.076 );

            // limit height
            aTitleSize.Height() = (long) (aTitleSize.Height() * 0.375);

            Size aPartArea = aTitleSize;
            Size aSize;
            sal_uInt16 nDestPageNum(GetPageNum());
            SdrPage* pRefPage = 0L;

            if(nDestPageNum)
            {
                // only decrement if != 0, else we get 0xffff
                nDestPageNum -= 1;
            }

            if(nDestPageNum < pModel->GetPageCount())
            {
                pRefPage = pModel->GetPage(nDestPageNum);
            }

            if ( pRefPage )
            {
                // scale actually page size into handout rectangle
                double fH = (double) aPartArea.Width()  / pRefPage->GetWdt();
                double fV = (double) aPartArea.Height() / pRefPage->GetHgt();

                if ( fH > fV )
                    fH = fV;
                aSize.Width()  = (long) (fH * pRefPage->GetWdt());
                aSize.Height() = (long) (fH * pRefPage->GetHgt());

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
|* return outline area
|*
\************************************************************************/

Rectangle SdPage::GetLayoutRect() const
{
    Rectangle aLayoutRect;

    if (mePageKind != PK_HANDOUT)
    {
        Point aLayoutPos ( GetLftBorder(), GetUppBorder() );
        Size aLayoutSize ( GetSize() );
        aLayoutSize.Width()  -= GetLftBorder() + GetRgtBorder();
        aLayoutSize.Height() -= GetUppBorder() + GetLwrBorder();

        if (mePageKind == PK_STANDARD)
        {
            aLayoutPos.X() += long( aLayoutSize.Width() * 0.05 );
            aLayoutPos.Y() += long( aLayoutSize.Height() * 0.234 );
            aLayoutSize.Width() = long( aLayoutSize.Width() * 0.9 );
            aLayoutSize.Height() = long( aLayoutSize.Height() * 0.58 );
            aLayoutRect.SetPos(aLayoutPos);
            aLayoutRect.SetSize(aLayoutSize);
        }
        else if (mePageKind == PK_NOTES)
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
|* assign a AutoLayout
|*
\*************************************************************************/

const int MAX_PRESOBJS = 7; // maximum number of presentation objects per layout
const int VERTICAL = 0x8000;
const int PRESOBJPROP = 4;

struct LayoutDescriptor
{
    int mnLayout;
    PresObjKind meKind[MAX_PRESOBJS];
    bool mbVertical[MAX_PRESOBJS];

    LayoutDescriptor( int nLayout, int k0 = 0, int k1 = 0, int k2 = 0, int k3 = 0, int k4 = 0, int k5 = 0, int k6 = 0 );
};

LayoutDescriptor::LayoutDescriptor( int nLayout, int k0, int k1, int k2, int k3, int k4, int k5, int k6 )
: mnLayout( nLayout )
{
    meKind[0] = static_cast<PresObjKind>(k0 & (~VERTICAL)); mbVertical[0] = (k0 & VERTICAL) == VERTICAL;
    meKind[1] = static_cast<PresObjKind>(k1 & (~VERTICAL)); mbVertical[1] = (k1 & VERTICAL) == VERTICAL;
    meKind[2] = static_cast<PresObjKind>(k2 & (~VERTICAL)); mbVertical[2] = (k2 & VERTICAL) == VERTICAL;
    meKind[3] = static_cast<PresObjKind>(k3 & (~VERTICAL)); mbVertical[3] = (k3 & VERTICAL) == VERTICAL;
    meKind[4] = static_cast<PresObjKind>(k4 & (~VERTICAL)); mbVertical[4] = (k4 & VERTICAL) == VERTICAL;
    meKind[5] = static_cast<PresObjKind>(k5 & (~VERTICAL)); mbVertical[5] = (k5 & VERTICAL) == VERTICAL;
    meKind[6] = static_cast<PresObjKind>(k6 & (~VERTICAL)); mbVertical[6] = (k6 & VERTICAL) == VERTICAL;
}

static const LayoutDescriptor& GetLayoutDescriptor( AutoLayout eLayout )
{
    static const LayoutDescriptor aLayouts[AUTOLAYOUT__END-AUTOLAYOUT__START] =
    {
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_TEXT ),                                 // AUTOLAYOUT_TITLE
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OUTLINE ),                              // AUTOLAYOUT_ENUM
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OUTLINE ),                              // AUTOLAYOUT_CHART
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_2TEXT
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_TEXTCHART
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OUTLINE ),                              // AUTOLAYOUT_ORG
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_TEXTCLbIP
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_CHARTTEXT
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OUTLINE ),                              // AUTOLAYOUT_TAB
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_CLIPTEXT
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_TEXTOBJ
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OBJECT ),                               // AUTOLAYOUT_OBJ
        LayoutDescriptor( 2, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),    // AUTOLAYOUT_TEXT2OBJ
        LayoutDescriptor( 1, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_TEXTOBJ
        LayoutDescriptor( 4, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_OBJOVERTEXT
        LayoutDescriptor( 3, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),    // AUTOLAYOUT_2OBJTEXT
        LayoutDescriptor( 5, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),    // AUTOLAYOUT_2OBJOVERTEXT
        LayoutDescriptor( 4, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),             // AUTOLAYOUT_TEXTOVEROBJ
        LayoutDescriptor( 6, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE,                   // AUTOLAYOUT_4OBJ
            PRESOBJ_OUTLINE, PRESOBJ_OUTLINE ),
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_NONE ),                                 // AUTOLAYOUT_ONLY_TITLE
        LayoutDescriptor( 0, PRESOBJ_NONE ),                                                // AUTOLAYOUT_NONE
        LayoutDescriptor( 0, PRESOBJ_PAGE, PRESOBJ_NOTES ),                                 // AUTOLAYOUT_NOTES
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT1
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT2
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT3
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT4
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT6
        LayoutDescriptor( 7, PRESOBJ_TITLE|VERTICAL, PRESOBJ_OUTLINE|VERTICAL, PRESOBJ_OUTLINE ),// AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART
        LayoutDescriptor( 8, PRESOBJ_TITLE|VERTICAL, PRESOBJ_OUTLINE|VERTICAL ),            // AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE
        LayoutDescriptor( 0, PRESOBJ_TITLE, PRESOBJ_OUTLINE|VERTICAL ),                     // AUTOLAYOUT_TITLE_VERTICAL_OUTLINE
        LayoutDescriptor( 9, PRESOBJ_TITLE, PRESOBJ_OUTLINE|VERTICAL, PRESOBJ_OUTLINE|VERTICAL ),   // AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART
        LayoutDescriptor( 0 ),                                                              // AUTOLAYOUT_HANDOUT9
        LayoutDescriptor( 10, PRESOBJ_TEXT, PRESOBJ_NONE ),                                 // AUTOLAYOUT_ONLY_TEXT
        LayoutDescriptor( 6, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE,               // AUTOLAYOUT_4CLIPART
            PRESOBJ_GRAPHIC, PRESOBJ_GRAPHIC ),
        LayoutDescriptor( 11, PRESOBJ_TITLE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE,              // AUTOLAYOUT_6CLIPART
            PRESOBJ_OUTLINE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE, PRESOBJ_OUTLINE )
    };

    if( (eLayout < AUTOLAYOUT__START) || (eLayout >= AUTOLAYOUT__END) )
        eLayout = AUTOLAYOUT_NONE;

    return aLayouts[ eLayout - AUTOLAYOUT__START ];
}

#define EXPAND_PROTOCOL "vnd.sun.star.expand:"
//to get the root element of the xml file
Reference<XElement> getRootElement()
{
    const Reference<css::uno::XComponentContext> xContext(comphelper_getProcessComponentContext());
    Reference< XMultiServiceFactory > xServiceFactory(xContext->getServiceManager(), UNO_QUERY_THROW );
    Reference< util::XMacroExpander > xMacroExpander =util::theMacroExpander::get(xContext);
    Reference< XMultiServiceFactory > xConfigProvider =configuration::theDefaultProvider::get( xContext );

    Any propValue = uno::makeAny(
        beans::PropertyValue(
            "nodepath", -1,
            uno::makeAny( OUString( "/org.openoffice.Office.Impress/Misc" )),
            beans::PropertyState_DIRECT_VALUE ) );

    Reference<container::XNameAccess> xNameAccess(
        xConfigProvider->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationAccess",
            Sequence<Any>( &propValue, 1 ) ), UNO_QUERY_THROW );
    Sequence< rtl::OUString > aFiles;
    xNameAccess->getByName( "LayoutListFiles" ) >>= aFiles;
    rtl::OUString aURL;
    for( sal_Int32 i=0; i<aFiles.getLength(); ++i )
    {
        aURL = aFiles[i];
        if( aURL.startsWith( EXPAND_PROTOCOL ) )
        {
            // cut protocol
            rtl::OUString aMacro( aURL.copy( sizeof ( EXPAND_PROTOCOL ) -1 ) );
            // decode uric class chars
            aMacro = rtl::Uri::decode( aMacro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
            // expand macro string
            aURL = xMacroExpander->expandMacros( aMacro );
        }
    }
    if( aURL.startsWith( "file://" ) )
    {
        rtl::OUString aSysPath;
        if( osl_getSystemPathFromFileURL( aURL.pData, &aSysPath.pData ) == osl_File_E_None )
            aURL = aSysPath;
    }
    const Reference<XDocumentBuilder> xDocBuilder(css::xml::dom::DocumentBuilder::create(comphelper::getComponentContext(xServiceFactory)));
    const Reference<XDocument> xDoc = xDocBuilder->parseURI(aURL);
    const Reference<XElement> xRoot = xDoc->getDocumentElement();
    return xRoot;//this loops seems to work only once,so temporary returning the root element
}

//read the information from XML file(traversing from layout node)
void readLayoutPropFromFile(const Reference<XElement>& root, const rtl::OUString& sLayoutType, const rtl::OUString& sPresObjKind, double propvalue[])
{
    long presobjsize;
    long layoutlistsize;
    rtl::OUString sLayoutAttName;
    rtl::OUString sPresObjKindAttName;
    bool bnoprop=true;                   //use it to skip the remaining loop ,once propvalue is obtained
    const Reference<XNodeList> layoutlist = root->getElementsByTagName("layout");
    layoutlistsize=layoutlist->getLength();

    for( long i=0; i<layoutlistsize ;i++)
    {
        if(bnoprop)
        {
            Reference<XNode> layoutnode = layoutlist->item(i);      //get i'th layout element
            Reference<XNamedNodeMap> layoutattrlist =layoutnode->getAttributes();
            Reference<XNode> layoutattr = layoutattrlist->getNamedItem("type");
            sLayoutAttName=layoutattr->getNodeValue();              //get the attribute value of layout(i.e it's type)

            if(sLayoutAttName==sLayoutType)//check string comparision func                      //compare it with the given parameter of the function
            {
                Reference<XNodeList> layoutchildrens = layoutnode->getChildNodes();
                presobjsize = layoutchildrens->getLength();         //get the length of that of the layout(number of pres objects)
                for( long j=0; j< presobjsize ; j++)
                {
                    Reference<XNode> presobj = layoutchildrens->item(j);    //get the j'th presobj for that layout
                    Reference<XNamedNodeMap> presObjAttributes = presobj->getAttributes();
                    Reference<XNode> presObjKindAttr = presObjAttributes->getNamedItem("kind");
                    sPresObjKindAttName = presObjKindAttr->getNodeValue();  //get the value of it's presobj kind
                    if(sPresObjKindAttName==sPresObjKind)
                    {
                        Reference<XNode> presObjPosX = presObjAttributes->getNamedItem("layout-pos-x");
                        rtl::OUString sValue = presObjPosX->getNodeValue();
                        propvalue[0] = sValue.toDouble();
                        Reference<XNode> presObjPosY = presObjAttributes->getNamedItem("layout-pos-y");
                        sValue = presObjPosY->getNodeValue();
                        propvalue[1] = sValue.toDouble();
                        Reference<XNode> presObjSizeHeight = presObjAttributes->getNamedItem("layout-size-height");
                        sValue = presObjSizeHeight->getNodeValue();
                        propvalue[2] = sValue.toDouble();
                        Reference<XNode> presObjSizeWidth = presObjAttributes->getNamedItem("layout-size-width");
                        sValue = presObjSizeWidth->getNodeValue();
                        propvalue[3] = sValue.toDouble();
                        bnoprop=false;
                        break;
                    }
                    else
                        continue;
                }
            }
            else
                continue;
        }
        else
            break;
    }
}

static void CalcAutoLayoutRectangles( SdPage& rPage, int nLayout, Rectangle* rRectangle )
{
    Rectangle aTitleRect;
    Rectangle aLayoutRect;
    double propvalue[4];

    if( rPage.GetPageKind() != PK_HANDOUT )
    {
        SdPage& rMasterPage = static_cast<SdPage&>(rPage.TRG_GetMasterPage());
        SdrObject* pMasterTitle = rMasterPage.GetPresObj( PRESOBJ_TITLE );
        SdrObject* pMasterSubTitle = rMasterPage.GetPresObj( PRESOBJ_TEXT );
        SdrObject* pMasterOutline = rMasterPage.GetPresObj( rPage.GetPageKind()==PK_NOTES ? PRESOBJ_NOTES : PRESOBJ_OUTLINE );

        if( pMasterTitle )
            aTitleRect = pMasterTitle->GetLogicRect();

        if (aTitleRect.IsEmpty() )
            aTitleRect = rPage.GetTitleRect();
        if( pMasterSubTitle )
            aLayoutRect = pMasterSubTitle->GetLogicRect();
        else if( pMasterOutline )
            aLayoutRect = pMasterOutline->GetLogicRect();

        if (aLayoutRect.IsEmpty() )
            aLayoutRect = rPage.GetLayoutRect();
    }

    rRectangle[0] = aTitleRect;

    int i;
    for( i = 1; i < MAX_PRESOBJS; i++ )
        rRectangle[i] = aLayoutRect;
    i=0;
    for(i=0; i< PRESOBJPROP; i++)
        propvalue[i]=0;

    const Reference<XElement> root= getRootElement();//get the root element of my xml file

    Point       aTitlePos( aTitleRect.TopLeft() );
    Size        aLayoutSize( aLayoutRect.GetSize() );
    Point       aLayoutPos( aLayoutRect.TopLeft() );
    Size        aTempSize;
    Point       aTempPnt;

    sal_Bool    bRightToLeft = ( rPage.GetModel() && static_cast< SdDrawDocument* >( rPage.GetModel() )->GetDefaultWritingMode() == ::com::sun::star::text::WritingMode_RL_TB );

    switch( nLayout )
    {
    case 0: // default layout using only the title and layout area
        break; // do nothing
    case 1: // title, 2 shapes
    case 9: // title, 2 vertical shapes
        readLayoutPropFromFile(root, "AUTOLAYOUT_TITLE_2VTEXT" ,"PRESOBJ_OUTLINE1" ,propvalue);
        aLayoutPos.X() = propvalue[0];
        aLayoutPos.Y() = propvalue[1];
        aLayoutSize.Height() = propvalue[2];
        aLayoutSize.Width() = propvalue[3];
        rRectangle[1] = Rectangle (aLayoutPos, aLayoutSize);

        readLayoutPropFromFile(root, "AUTOLAYOUT_TITLE_2VTEXT" ,"PRESOBJ_OUTLINE2" ,propvalue);
        aLayoutPos.X() = propvalue[0];
        aLayoutPos.Y() = propvalue[1];
        aLayoutSize.Height() = propvalue[2];
        aLayoutSize.Width() = propvalue[3];
        rRectangle[2] = Rectangle (aLayoutPos, aLayoutSize);

        if( bRightToLeft && (nLayout != 9) )
            ::std::swap( rRectangle[1], rRectangle[2] );
        break;
    case 2: // title, shape, 2 shapes
        aTempPnt = aLayoutPos;
        aTempSize = aLayoutSize;
        aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
        aLayoutSize.Width() = long (aLayoutSize.Width() * 0.488);
        aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
        rRectangle[2] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
        rRectangle[3] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos = aTempPnt;
        aLayoutSize = aTempSize;
        aLayoutSize.Width() = long (aLayoutSize.Width() * 0.488);
        rRectangle[1] = Rectangle (aLayoutPos, aLayoutSize);

        if( bRightToLeft )
        {
            ::std::swap( rRectangle[1].Left(), rRectangle[2].Left() );
            rRectangle[3].Left() = rRectangle[2].Left();
        }
        break;
    case 3: // title, 2 shapes, shape
        aTempPnt = aLayoutPos;
        aTempSize = aLayoutSize;
        aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
        aLayoutSize.Width() = long (aLayoutSize.Width() * 0.488);
        rRectangle[1] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
        rRectangle[2] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos = aTempPnt;
        aLayoutSize = aTempSize;
        aLayoutSize.Width() = long (aLayoutSize.Width() * 0.488);
        aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
        rRectangle[3] = Rectangle (aLayoutPos, aLayoutSize);

        if( bRightToLeft )
        {
            ::std::swap( rRectangle[1].Left(), rRectangle[2].Left() );
            rRectangle[3].Left() = rRectangle[2].Left();
        }
        break;
    case 4: // title, shape above shape
        aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
        rRectangle[1] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
        rRectangle[2] = Rectangle (aLayoutPos, aLayoutSize);
        break;

    case 5: // title, 2 shapes above shape
        aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
        aLayoutSize.Width() = long (aLayoutSize.Width() * 0.488);
        rRectangle[1] = Rectangle (aLayoutPos, aLayoutSize);

        aTempPnt = aLayoutPos;
        aLayoutPos.X() = long (aLayoutPos.X() + aLayoutSize.Width() * 1.05);
        rRectangle[2] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.X() = aTempPnt.X();
        aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
        aLayoutSize.Width() = long (aLayoutSize.Width() / 0.488);
        rRectangle[3] = Rectangle (aLayoutPos, aLayoutSize);
        break;
    case 6: // title, 4 shapes
    {
        sal_uLong nX = long (aLayoutPos.X());

        aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
        aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.488);
        rRectangle[1] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.X() = long (nX + aLayoutSize.Width() * 1.05);
        rRectangle[2] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
        rRectangle[3] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.X() = nX;
        rRectangle[4] = Rectangle (aLayoutPos, aLayoutSize);
        break;
    }
    case 7: // vertical title, shape above shape
    {
        Size aSize( rRectangle[0].GetSize().Height(), rRectangle[1].BottomLeft().Y() - rRectangle[0].TopLeft().Y() );
        rRectangle[0].SetSize( aSize );
        rRectangle[0].SetPos( aTitleRect.TopRight() - Point( aSize.Width(), 0 ) );

        Size aPageSize ( rPage.GetSize() );
        aPageSize.Height() -= rPage.GetUppBorder() + rPage.GetLwrBorder();
        aSize.Height() = long ( rRectangle[0].GetSize().Height() * 0.47 );
        aSize.Width() = long( aPageSize.Width() * 0.7 );
        rRectangle[1].SetPos( aTitleRect.TopLeft() );
        rRectangle[1].SetSize( aSize );

        aSize.Height() = rRectangle[0].GetSize().Height();
        Point aPos( aTitleRect.TopLeft() );
        aPos.Y() += long ( aSize.Height() * 0.53 );
        rRectangle[2].SetPos( aPos );
        aSize.Height() = long ( rRectangle[0].GetSize().Height() * 0.47 );
        rRectangle[2].SetSize( aSize );
        break;
    }
    case 8: // vertical title, shape
    {
        Size aSize( rRectangle[0].GetSize().Height(), rRectangle[1].BottomLeft().Y() - rRectangle[0].TopLeft().Y() );
        rRectangle[0].SetSize( aSize );
        rRectangle[0].SetPos( aTitleRect.TopRight() - Point( aSize.Width(), 0 ) );

        Size aPageSize ( rPage.GetSize() );
        aPageSize.Height() -= rPage.GetUppBorder() + rPage.GetLwrBorder();
        aSize.Height() = rRectangle[0].GetSize().Height();
        aSize.Width() = long( aPageSize.Width() * 0.7 );
        rRectangle[1].SetPos( aTitleRect.TopLeft() );
        rRectangle[1].SetSize( aSize );
        break;
    }
    case 10: // onlytext
    {
        Size aSize( rRectangle[0].GetSize().Width(), rRectangle[1].BottomLeft().Y() - rRectangle[0].TopLeft().Y() );
        rRectangle[0].SetSize( aSize );
        rRectangle[0].SetPos( aTitlePos);
        break;
    }
    case 11: // title, 6 shapes
    {
        sal_uLong nX = long (aLayoutPos.X());

        aLayoutSize.Height() = long (aLayoutSize.Height() * 0.477);
        aLayoutSize.Width()  = long (aLayoutSize.Width() * 0.322);
        rRectangle[1] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.X() = long (nX + aLayoutSize.Width() * 1.05);
        rRectangle[2] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.X() = long (nX + aLayoutSize.Width() * 2 * 1.05);
        rRectangle[3] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.Y() = long (aLayoutPos.Y() + aLayoutSize.Height() * 1.095);
        rRectangle[4] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.X() = long (nX + aLayoutSize.Width() * 1.05);
        rRectangle[5] = Rectangle (aLayoutPos, aLayoutSize);

        aLayoutPos.X() = nX;
        rRectangle[6] = Rectangle (aLayoutPos, aLayoutSize);

        break;
    }

    }
}


void findAutoLayoutShapesImpl( SdPage& rPage, const LayoutDescriptor& rDescriptor, std::vector< SdrObject* >& rShapes, bool bInit, bool bSwitchLayout )
{
    int i;

    // init list of indexes for each presentation shape kind
    // this is used to find subsequent shapes with the same presentation shape kind
    int PresObjIndex[PRESOBJ_MAX];
    for( i = 0; i < PRESOBJ_MAX; i++ ) PresObjIndex[i] = 1;

    bool bMissing = false;

    // for each entry in the layoutdescriptor, arrange a presentation shape
    for (i = 0; (i < MAX_PRESOBJS) && (rDescriptor.meKind[i] != PRESOBJ_NONE); i++)
    {
        PresObjKind eKind = rDescriptor.meKind[i];
        SdrObject* pObj = 0;
        while( (pObj = rPage.GetPresObj( eKind, PresObjIndex[eKind], true )) != 0 )
        {
            PresObjIndex[eKind]++; // on next search for eKind, find next shape with same eKind

            if( !bSwitchLayout || !pObj->IsEmptyPresObj() )
            {
                rShapes[i] = pObj;
                break;
            }
        }

        if( !pObj )
            bMissing = true;
    }

    if( bMissing && bInit )
    {
        // for each entry in the layoutdescriptor, look for an alternative shape
        for (i = 0; (i < MAX_PRESOBJS) && (rDescriptor.meKind[i] != PRESOBJ_NONE); i++)
        {
            if( rShapes[i] )
                continue;

            PresObjKind eKind = rDescriptor.meKind[i];

            SdrObject* pObj = 0;
            bool bFound = false;

            const int nShapeCount = rPage.GetObjCount();
            int nShapeIndex = 0;
            while((nShapeIndex < nShapeCount) && !bFound )
            {
                pObj = rPage.GetObj(nShapeIndex++);

                if( pObj->IsEmptyPresObj() )
                    continue;

                if( pObj->GetObjInventor() != SdrInventor )
                    continue;

                // do not reuse shapes that are already part of the layout
                if( std::find( rShapes.begin(), rShapes.end(), pObj ) != rShapes.end() )
                    continue;

                bool bPresStyle = pObj->GetStyleSheet() && (pObj->GetStyleSheet()->GetFamily() == SD_STYLE_FAMILY_MASTERPAGE);
                SdrObjKind eSdrObjKind = static_cast< SdrObjKind >( pObj->GetObjIdentifier() );

                switch( eKind )
                {
                case PRESOBJ_TITLE:
                    bFound = eSdrObjKind == OBJ_TITLETEXT;
                    break;
                case PRESOBJ_TABLE:
                    bFound = eSdrObjKind == OBJ_TABLE;
                    break;
                case PRESOBJ_MEDIA:
                    bFound = eSdrObjKind == OBJ_MEDIA;
                    break;
                case PRESOBJ_OUTLINE:
                    bFound = (eSdrObjKind == OBJ_OUTLINETEXT) ||
                             ((eSdrObjKind == OBJ_TEXT) && bPresStyle) ||
                             (eSdrObjKind == OBJ_TABLE) || (eSdrObjKind == OBJ_MEDIA) || (eSdrObjKind == OBJ_GRAF) || (eSdrObjKind == OBJ_OLE2);
                    break;
                case PRESOBJ_GRAPHIC:
                    bFound = eSdrObjKind == OBJ_GRAF;
                    break;
                case PRESOBJ_OBJECT:
                    if( eSdrObjKind == OBJ_OLE2 )
                    {
                        SdrOle2Obj* pOle2 = dynamic_cast< SdrOle2Obj* >( pObj );
                        if( pOle2 )
                        {
                            if( pOle2->IsEmpty() )
                                bFound = true;
                            else if( rPage.GetModel() )
                            {
                                SdrModel* pSdrModel = rPage.GetModel();
                                ::comphelper::IEmbeddedHelper *pPersist = pSdrModel->GetPersist();
                                if( pPersist )
                                {
                                    uno::Reference < embed::XEmbeddedObject > xObject = pPersist->getEmbeddedObjectContainer().
                                            GetEmbeddedObject( static_cast< SdrOle2Obj* >( pObj )->GetPersistName() );

                                    // TODO CL->KA: Why is this not working anymore?
                                    if( xObject.is() )
                                    {
                                        SvGlobalName aClassId( xObject->getClassID() );

                                        const SvGlobalName aAppletClassId( SO3_APPLET_CLASSID );
                                        const SvGlobalName aPluginClassId( SO3_PLUGIN_CLASSID );
                                        const SvGlobalName aIFrameClassId( SO3_IFRAME_CLASSID );

                                        if( aPluginClassId != aClassId && aAppletClassId != aClassId && aIFrameClassId != aClassId )
                                        {
                                            bFound = true;
                                        }
                                    }
                                }
                             }
                         }
                    }
                    break;
                case PRESOBJ_CHART:
                case PRESOBJ_CALC:
                    if( eSdrObjKind == OBJ_OLE2 )
                    {
                        SdrOle2Obj* pOle2 = dynamic_cast< SdrOle2Obj* >( pObj );
                        if( pOle2 )
                        {
                            if(
                                ((eKind == PRESOBJ_CHART) &&
                                    ( pOle2->GetProgName().EqualsAscii( "StarChart" ) || pOle2->IsChart() ) )
                                ||
                                ((eKind == PRESOBJ_CALC) &&
                                    ( pOle2->GetProgName().EqualsAscii( "StarCalc" ) || pOle2->IsCalc() ) ) )
                            {
                                bFound = true;
                            }
                        }
                        break;
                    }
                    else if( eSdrObjKind == OBJ_TABLE )
                    {
                        bFound = true;
                    }
                    break;
                case PRESOBJ_PAGE:
                case PRESOBJ_HANDOUT:
                    bFound = eSdrObjKind == OBJ_PAGE;
                    break;
                case PRESOBJ_NOTES:
                case PRESOBJ_TEXT:
                    bFound = (bPresStyle && (eSdrObjKind == OBJ_TEXT)) || (eSdrObjKind == OBJ_OUTLINETEXT);
                    break;
                default:
                    break;
                }
            }

            if( bFound )
                rShapes[i] = pObj;
        }
    }
}

void SdPage::SetAutoLayout(AutoLayout eLayout, sal_Bool bInit, sal_Bool bCreate )
{
    sd::ScopeLockGuard aGuard( maLockAutoLayoutArrangement );

    const bool bSwitchLayout = eLayout != GetAutoLayout();

    ::svl::IUndoManager* pUndoManager = pModel ? static_cast<SdDrawDocument*>(pModel)->GetUndoManager() : 0;
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    meAutoLayout = eLayout;

    // if needed, creates and initialises the presentation shapes on this slides master page
    CreateTitleAndLayout(bInit, bCreate);

    if((meAutoLayout == AUTOLAYOUT_NONE && maPresentationShapeList.isEmpty()) || mbMaster)
    {
        // MasterPage or no layout and no presentation shapes available, nothing to do
        return;
    }

    Rectangle aRectangle[MAX_PRESOBJS];
    const LayoutDescriptor& aDescriptor = GetLayoutDescriptor( meAutoLayout );
    CalcAutoLayoutRectangles( *this, aDescriptor.mnLayout, aRectangle );

    std::set< SdrObject* > aUsedPresentationObjects;


    std::vector< SdrObject* > aLayoutShapes(PRESOBJ_MAX, 0);
    findAutoLayoutShapesImpl( *this, aDescriptor, aLayoutShapes, bInit, bSwitchLayout );

    int i;

    // for each entry in the layoutdescriptor, arrange a presentation shape
    for (i = 0; (i < MAX_PRESOBJS) && (aDescriptor.meKind[i] != PRESOBJ_NONE); i++)
    {
        PresObjKind eKind = aDescriptor.meKind[i];
        SdrObject* pObj = InsertAutoLayoutShape( aLayoutShapes[i], eKind, aDescriptor.mbVertical[i], aRectangle[i], bInit );
        if( pObj )
            aUsedPresentationObjects.insert(pObj); // remember that we used this empty shape
    }

    // now delete all empty presentation objects that are no longer used by the new layout
    if( bInit )
    {
        SdrObject* pObj = 0;
        maPresentationShapeList.seekShape(0);

        while( (pObj = maPresentationShapeList.getNextShape()) )
        {
            if( aUsedPresentationObjects.count(pObj) == 0 )
            {

                if( pObj->IsEmptyPresObj() )
                {
                    if( bUndo )
                        pUndoManager->AddUndoAction(pModel->GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));

                    RemoveObject( pObj->GetOrdNum() );

                    if( !bUndo )
                        SdrObject::Free( pObj );
                }
/* #i108541# keep non empty pres obj as pres obj even if they are not part of the current layout */
            }
        }
    }
}

/*************************************************************************
|*
|* insert object
|*
\************************************************************************/

void SdPage::NbcInsertObject(SdrObject* pObj, sal_uLong nPos, const SdrInsertReason* pReason)
{
    FmFormPage::NbcInsertObject(pObj, nPos, pReason);

    ((SdDrawDocument*) pModel)->InsertObject(pObj, this);

    SdrLayerID nId = pObj->GetLayer();
    if( mbMaster )
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
|* remove object
|*
\************************************************************************/

SdrObject* SdPage::RemoveObject(sal_uLong nObjNum)
{
    onRemoveObject(GetObj( nObjNum ));
    return FmFormPage::RemoveObject(nObjNum);
}

/*************************************************************************
|*
|* remove object without broadcast
|*
\************************************************************************/

SdrObject* SdPage::NbcRemoveObject(sal_uLong nObjNum)
{
    onRemoveObject(GetObj( nObjNum ));
    return FmFormPage::NbcRemoveObject(nObjNum);
}

// Also overload ReplaceObject methods to realize when
// objects are removed with this mechanism instead of RemoveObject
SdrObject* SdPage::NbcReplaceObject(SdrObject* pNewObj, sal_uLong nObjNum)
{
    onRemoveObject(GetObj( nObjNum ));
    return FmFormPage::NbcReplaceObject(pNewObj, nObjNum);
}

// Also overload ReplaceObject methods to realize when
// objects are removed with this mechanism instead of RemoveObject
SdrObject* SdPage::ReplaceObject(SdrObject* pNewObj, sal_uLong nObjNum)
{
    onRemoveObject(GetObj( nObjNum ));
    return FmFormPage::ReplaceObject(pNewObj, nObjNum);
}

// -------------------------------------------------------------------------

// called after a shape is removed or replaced from this slide

void SdPage::onRemoveObject( SdrObject* pObject )
{
    if( pObject )
    {
        RemovePresObj(pObject);

        if( pModel )
            static_cast<SdDrawDocument*>(pModel)->RemoveObject(pObject, this);

        removeAnimations( pObject );
    }
}

void SdPage::SetSize(const Size& aSize)
{
    Size aOldSize = GetSize();

    if (aSize != aOldSize)
    {
        FmFormPage::SetSize(aSize);

        if (aOldSize.Height() == 10 && aOldSize.Width() == 10)
        {
            // this page gets a valid size for the first time. Therefore
            // we initialize the orientation.
            if (aSize.Width() > aSize.Height())
            {
                meOrientation = ORIENTATION_LANDSCAPE;
            }
            else
            {
                meOrientation = ORIENTATION_PORTRAIT;
            }
        }
    }
}

void SdPage::SetBorder(sal_Int32 nLft, sal_Int32 nUpp, sal_Int32 nRgt, sal_Int32 nLwr)
{
    if (nLft != GetLftBorder() || nUpp != GetUppBorder() ||
        nRgt != GetRgtBorder() || nLwr != GetLwrBorder() )
    {
        FmFormPage::SetBorder(nLft, nUpp, nRgt, nLwr);
    }
}

void SdPage::SetLftBorder(sal_Int32 nBorder)
{
    if (nBorder != GetLftBorder() )
    {
        FmFormPage::SetLftBorder(nBorder);
    }
}

void SdPage::SetRgtBorder(sal_Int32 nBorder)
{
    if (nBorder != GetRgtBorder() )
    {
        FmFormPage::SetRgtBorder(nBorder);
    }
}

void SdPage::SetUppBorder(sal_Int32 nBorder)
{
    if (nBorder != GetUppBorder() )
    {
        FmFormPage::SetUppBorder(nBorder);
    }
}

void SdPage::SetLwrBorder(sal_Int32 nBorder)
{
    if (nBorder != GetLwrBorder() )
    {
        FmFormPage::SetLwrBorder(nBorder);
    }
}

/*************************************************************************
|*
|* Sets BackgroundFullSize and then calls AdjustBackground
|*
\************************************************************************/

void SdPage::SetBackgroundFullSize( sal_Bool bIn )
{
    if( bIn != mbBackgroundFullSize )
    {
        mbBackgroundFullSize = bIn;
    }
}

/*************************************************************************
|*
|* Adjust all objects to new page size.
|*
|* bScaleAllObj: all objects are scaled into the new area within the page
|* margins. We scale the position and size. For presentation objects on the
|* master page, we also scale the font height of the presentation template.
|*
\************************************************************************/

void SdPage::ScaleObjects(const Size& rNewPageSize, const Rectangle& rNewBorderRect, sal_Bool bScaleAllObj)
{
    sd::ScopeLockGuard aGuard( maLockAutoLayoutArrangement );

    mbScaleObjects = bScaleAllObj;
    SdrObject* pObj = NULL;
    Point aRefPnt(0, 0);
    Size aNewPageSize(rNewPageSize);
    sal_Int32 nLeft  = rNewBorderRect.Left();
    sal_Int32 nRight = rNewBorderRect.Right();
    sal_Int32 nUpper = rNewBorderRect.Top();
    sal_Int32 nLower = rNewBorderRect.Bottom();

    // negative values are fixed values
    // -> use up to date values
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

    if (mbScaleObjects)
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

    sal_uLong nObjCnt = (mbScaleObjects ? GetObjCount() : 0);

    for (sal_uLong nObj = 0; nObj < nObjCnt; nObj++)
    {
        sal_Bool bIsPresObjOnMaster = sal_False;

        // all Objects
        pObj = GetObj(nObj);

        if (mbMaster && IsPresObj(pObj))
        {
            // There is a presentation object on the master page
            bIsPresObjOnMaster = sal_True;
        }

        if (pObj)
        {
            // remember aTopLeft as original TopLeft
            Point aTopLeft(pObj->GetCurrentBoundRect().TopLeft());

            if (!pObj->IsEdgeObj())
            {
                /**************************************************************
                * Scale objects
                **************************************************************/
                if (mbScaleObjects)
                {
                    // use aTopLeft as original TopLeft
                    aRefPnt = aTopLeft;
                }

                pObj->Resize(aRefPnt, aFractX, aFractY);

                if (mbScaleObjects)
                {
                    SdrObjKind eObjKind = (SdrObjKind) pObj->GetObjIdentifier();

                    if (bIsPresObjOnMaster)
                    {
                        /**********************************************************
                        * presentation template: adjust test height
                        **********************************************************/
                        sal_uInt16 nIndexTitle = 0;
                        sal_uInt16 nIndexOutline = 0;
                        sal_uInt16 nIndexNotes = 0;

                        if (pObj == GetPresObj(PRESOBJ_TITLE, nIndexTitle))
                        {
                            SfxStyleSheet* pTitleSheet = GetStyleSheetForPresObj(PRESOBJ_TITLE);

                            if (pTitleSheet)
                            {
                                SfxItemSet& rSet = pTitleSheet->GetItemSet();

                                SvxFontHeightItem& rOldHgt = (SvxFontHeightItem&) rSet.Get(EE_CHAR_FONTHEIGHT);
                                sal_uLong nFontHeight = rOldHgt.GetHeight();
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

                                pTitleSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                            }
                        }
                        else if (pObj == GetPresObj(PRESOBJ_OUTLINE, nIndexOutline))
                        {
                            String aName(GetLayoutName());
                            aName += sal_Unicode( ' ' );

                            for (sal_uInt16 i=1; i<=9; i++)
                            {
                                String sLayoutName(aName);
                                sLayoutName += OUString::number( (sal_Int32)i );
                                SfxStyleSheet* pOutlineSheet = (SfxStyleSheet*)((SdDrawDocument*) pModel)->GetStyleSheetPool()->Find(sLayoutName, SD_STYLE_FAMILY_MASTERPAGE);

                                if (pOutlineSheet)
                                {
                                    // Calculate new font height
                                    SfxItemSet aTempSet(pOutlineSheet->GetItemSet());

                                    SvxFontHeightItem& rOldHgt = (SvxFontHeightItem&) aTempSet.Get(EE_CHAR_FONTHEIGHT);
                                    sal_uLong nFontHeight = rOldHgt.GetHeight();
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

                                    // adjust bullet
                                    ((SdStyleSheet*) pOutlineSheet)->AdjustToFontHeight(aTempSet, sal_False);

                                    // Special treatment: reset the INVALIDS to
                                    // NULL pointer (otherwise we have INVALID's
                                    // or pointer to the DefaultItems in the
                                    // template; both would suppress the
                                    // attribute inheritance)
                                    aTempSet.ClearInvalidItems();

                                    // Special treatment: only the valid parts
                                    // of the BulletItems
                                    if (aTempSet.GetItemState(EE_PARA_BULLET) == SFX_ITEM_AVAILABLE)
                                    {
                                        SvxBulletItem aOldBulItem((SvxBulletItem&) pOutlineSheet->GetItemSet().Get(EE_PARA_BULLET));
                                        SvxBulletItem& rNewBulItem = (SvxBulletItem&) aTempSet.Get(EE_PARA_BULLET);
                                        aOldBulItem.CopyValidProperties(rNewBulItem);
                                        aTempSet.Put(aOldBulItem);
                                    }

                                    pOutlineSheet->GetItemSet().Put(aTempSet);
                                    pOutlineSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                                }
                            }
                        }
                        else if (pObj == GetPresObj(PRESOBJ_NOTES, nIndexNotes))
                        {
                            SfxStyleSheet* pNotesSheet = GetStyleSheetForPresObj(PRESOBJ_NOTES);

                            if (pNotesSheet)
                            {
                                sal_uLong nHeight = pObj->GetLogicRect().GetSize().Height();
                                sal_uLong nFontHeight = (sal_uLong) (nHeight * 0.0741);
                                SfxItemSet& rSet = pNotesSheet->GetItemSet();
                                rSet.Put( SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT ));
                                rSet.Put( SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CJK ));
                                rSet.Put( SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CTL ));
                                pNotesSheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                            }
                        }
                    }
                    else if ( eObjKind != OBJ_TITLETEXT   &&
                              eObjKind != OBJ_OUTLINETEXT &&
                              pObj->ISA(SdrTextObj)       &&
                              pObj->GetOutlinerParaObject() )
                    {
                        /******************************************************
                        * normal text object: adjust text height
                        ******************************************************/
                        sal_uLong nScriptType = pObj->GetOutlinerParaObject()->GetTextObject().GetScriptType();
                        sal_uInt16 nWhich = EE_CHAR_FONTHEIGHT;
                        if ( nScriptType == SCRIPTTYPE_ASIAN )
                            nWhich = EE_CHAR_FONTHEIGHT_CJK;
                        else if ( nScriptType == SCRIPTTYPE_COMPLEX )
                            nWhich = EE_CHAR_FONTHEIGHT_CTL;

                        // use more modern method to scale the text height
                        sal_uInt32 nFontHeight = ((SvxFontHeightItem&)pObj->GetMergedItem(nWhich)).GetHeight();
                        sal_uInt32 nNewFontHeight = sal_uInt32((double)nFontHeight * (double)aFractY);

                        pObj->SetMergedItem(SvxFontHeightItem(nNewFontHeight, 100, nWhich));
                    }
                }
            }

            if (mbScaleObjects && !pObj->IsEdgeObj())
            {
                /**************************************************************
                * scale object position
                **************************************************************/
                Point aNewPos;

                // corrected scaling; only distances may be scaled
                // use aTopLeft as original TopLeft
                aNewPos.X() = long((aTopLeft.X() - GetLftBorder()) * (double)aFractX) + nLeft;
                aNewPos.Y() = long((aTopLeft.Y() - GetUppBorder()) * (double)aFractY) + nUpper;

                Size aVec(aNewPos.X() - aTopLeft.X(), aNewPos.Y() - aTopLeft.Y());

                if (aVec.Height() != 0 || aVec.Width() != 0)
                {
                    pObj->NbcMove(aVec);
                }

                pObj->SetChanged();
                pObj->BroadcastObjectChange();
            }
        }
    }
}

SdrObject* convertPresentationObjectImpl( SdPage& rPage, SdrObject* pSourceObj, PresObjKind& eObjKind, bool bVertical, Rectangle aRect )
{
    SdDrawDocument* pModel = static_cast< SdDrawDocument* >( rPage.GetModel() );
    DBG_ASSERT( pModel, "sd::convertPresentationObjectImpl(), no model on page!" );
    if( !pModel || !pSourceObj )
        return pSourceObj;

    ::svl::IUndoManager* pUndoManager = static_cast<SdDrawDocument*>(pModel)->GetUndoManager();
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && rPage.IsInserted();

    SdrObject* pNewObj = pSourceObj;
    if((eObjKind == PRESOBJ_OUTLINE) && (pSourceObj->GetObjIdentifier() == OBJ_TEXT) )
    {
        pNewObj = rPage.CreatePresObj(PRESOBJ_OUTLINE, bVertical, aRect);

        // Set text of the subtitle into PRESOBJ_OUTLINE
        OutlinerParaObject* pOutlParaObj = pSourceObj->GetOutlinerParaObject();

        if(pOutlParaObj)
        {
            // assign text
            ::sd::Outliner* pOutl = pModel->GetInternalOutliner( sal_True );
            pOutl->Clear();
            pOutl->SetText( *pOutlParaObj );
            pOutlParaObj = pOutl->CreateParaObject();
            pNewObj->SetOutlinerParaObject( pOutlParaObj );
            pOutl->Clear();
            pNewObj->SetEmptyPresObj(sal_False);

            for (sal_uInt16 nLevel = 1; nLevel < 10; nLevel++)
            {
                // assign new template
                String aName(rPage.GetLayoutName());
                aName += sal_Unicode( ' ' );
                aName += OUString::number( nLevel );
                SfxStyleSheet* pSheet = static_cast<SfxStyleSheet*>( pModel->GetStyleSheetPool()->Find(aName, SD_STYLE_FAMILY_MASTERPAGE) );

                if (pSheet)
                {
                    if (nLevel == 1)
                    {
                        SfxStyleSheet* pSubtitleSheet = rPage.GetStyleSheetForPresObj(PRESOBJ_TEXT);

                        if (pSubtitleSheet)
                            pOutlParaObj->ChangeStyleSheetName(SD_STYLE_FAMILY_MASTERPAGE, pSubtitleSheet->GetName(), pSheet->GetName());
                    }

                    pNewObj->StartListening(*pSheet);
                }
            }

            // Remove LRSpace item
            SfxItemSet aSet(pModel->GetPool(), EE_PARA_LRSPACE, EE_PARA_LRSPACE );

            aSet.Put(pNewObj->GetMergedItemSet());

            aSet.ClearItem(EE_PARA_LRSPACE);

            pNewObj->SetMergedItemSet(aSet);

            if( bUndo )
                pUndoManager->AddUndoAction( pModel->GetSdrUndoFactory().CreateUndoDeleteObject(*pSourceObj) );

            // Remove outline shape from page
            rPage.RemoveObject( pSourceObj->GetOrdNum() );

            if( !bUndo )
                SdrObject::Free( pSourceObj );
        }
    }
    else if((eObjKind == PRESOBJ_TEXT) && (pSourceObj->GetObjIdentifier() == OBJ_OUTLINETEXT) )
    {
        // is there an outline shape we can use to replace empty subtitle shape?
        pNewObj = rPage.CreatePresObj(PRESOBJ_TEXT, bVertical, aRect);

        // Set text of the outline object into PRESOBJ_TITLE
        OutlinerParaObject* pOutlParaObj = pSourceObj->GetOutlinerParaObject();

        if(pOutlParaObj)
        {
            // assign text
            ::sd::Outliner* pOutl = pModel->GetInternalOutliner();
            pOutl->Clear();
            pOutl->SetText( *pOutlParaObj );
            pOutlParaObj = pOutl->CreateParaObject();
            pNewObj->SetOutlinerParaObject( pOutlParaObj );
            pOutl->Clear();
            pNewObj->SetEmptyPresObj(sal_False);

            // reset left indent
            SfxItemSet aSet(pModel->GetPool(), EE_PARA_LRSPACE, EE_PARA_LRSPACE );

            aSet.Put(pNewObj->GetMergedItemSet());

            const SvxLRSpaceItem& rLRItem = (const SvxLRSpaceItem&) aSet.Get(EE_PARA_LRSPACE);
            SvxLRSpaceItem aNewLRItem(rLRItem);
            aNewLRItem.SetTxtLeft(0);
            aSet.Put(aNewLRItem);

            pNewObj->SetMergedItemSet(aSet);

            SfxStyleSheet* pSheet = rPage.GetStyleSheetForPresObj(PRESOBJ_TEXT);
            if (pSheet)
                pNewObj->SetStyleSheet(pSheet, sal_True);

            // Remove subtitle shape from page
            if( bUndo )
                pUndoManager->AddUndoAction(pModel->GetSdrUndoFactory().CreateUndoDeleteObject(*pSourceObj));

            rPage.RemoveObject( pSourceObj->GetOrdNum() );

            if( !bUndo )
                SdrObject::Free( pSourceObj );
        }
    }
    else if((eObjKind == PRESOBJ_OUTLINE) && (pSourceObj->GetObjIdentifier() != OBJ_OUTLINETEXT) )
    {
        switch( pSourceObj->GetObjIdentifier() )
        {
        case OBJ_TABLE: eObjKind = PRESOBJ_TABLE; break;
        case OBJ_MEDIA: eObjKind = PRESOBJ_MEDIA; break;
        case OBJ_GRAF: eObjKind = PRESOBJ_GRAPHIC; break;
        case OBJ_OLE2: eObjKind = PRESOBJ_OBJECT; break;
        }
    }

    return pNewObj;
}

/** reuses or creates a presentation shape for an auto layout that fits the given parameter

    @param  eObjKind
        The kind of presentation shape we like to have
    @param  nIndex
        If > 1 we skip the first nIndex-1 shapes with the presentation shape kind eObjKind while
        looking for an existing presentation shape
    @param  bVertical
        If true, the shape is created vertical if bInit is true
    @param  aRect
        The rectangle that should be used to transform the shape
    @param  bInit
        If true the shape is created if not found
    @returns
        A presentation shape that was either found or created with the given parameters
*/
SdrObject* SdPage::InsertAutoLayoutShape( SdrObject* pObj, PresObjKind eObjKind, bool bVertical, Rectangle aRect, bool bInit )
{
    ::svl::IUndoManager* pUndoManager = pModel ? static_cast<SdDrawDocument*>(pModel)->GetUndoManager() : 0;
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    if (!pObj && bInit)
    {
        pObj = CreatePresObj(eObjKind, bVertical, aRect);
    }
    else if ( pObj && (pObj->GetUserCall() || bInit) )
    {
        // convert object if shape type does not match kind (f.e. converting outline text to subtitle text)
        if( bInit )
            pObj = convertPresentationObjectImpl( *this, pObj, eObjKind, bVertical, aRect );

        if( bUndo )
        {
            pUndoManager->AddUndoAction( pModel->GetSdrUndoFactory().CreateUndoGeoObject( *pObj ) );
            pUndoManager->AddUndoAction( pModel->GetSdrUndoFactory().CreateUndoAttrObject( *pObj, sal_True, sal_True ) );
            pUndoManager->AddUndoAction( new UndoObjectUserCall( *pObj ) );
        }

            ( /*(SdrGrafObj*)*/ pObj)->AdjustToMaxRect( aRect );

        pObj->SetUserCall(this);

        SdrTextObj* pTextObject = dynamic_cast< SdrTextObj* >(pObj);
        if( pTextObject )
        {
            if( pTextObject->IsVerticalWriting() != (bVertical ? sal_True : sal_False) )
            {
                pTextObject->SetVerticalWriting( bVertical );

                // here make sure the correct anchoring is used when the object
                // is re-used but orientation is changed
                if(PRESOBJ_OUTLINE == eObjKind)
                    pTextObject->SetMergedItem(SdrTextHorzAdjustItem( bVertical ? SDRTEXTHORZADJUST_RIGHT : SDRTEXTHORZADJUST_BLOCK ));
            }

            if( !mbMaster && (pTextObject->GetObjIdentifier() != OBJ_TABLE) )
            {
                if ( pTextObject->IsAutoGrowHeight() )
                {
                    // switch off AutoGrowHeight, set new MinHeight
                    SfxItemSet aTempAttr( ((SdDrawDocument*) pModel)->GetPool() );
                    SdrTextMinFrameHeightItem aMinHeight( aRect.GetSize().Height() );
                    aTempAttr.Put( aMinHeight );
                    aTempAttr.Put( SdrTextAutoGrowHeightItem(sal_False) );
                    pTextObject->SetMergedItemSet(aTempAttr);
                    pTextObject->SetLogicRect(aRect);

                    // switch on AutoGrowHeight
                    SfxItemSet aAttr( ((SdDrawDocument*) pModel)->GetPool() );
                    aAttr.Put( SdrTextAutoGrowHeightItem(sal_True) );

                    pTextObject->SetMergedItemSet(aAttr);
                }

                if ( pTextObject->IsAutoGrowWidth() )
                {
                    // switch off AutoGrowWidth , set new MinWidth
                    SfxItemSet aTempAttr( ((SdDrawDocument*) pModel)->GetPool() );
                    SdrTextMinFrameWidthItem aMinWidth( aRect.GetSize().Width() );
                    aTempAttr.Put( aMinWidth );
                    aTempAttr.Put( SdrTextAutoGrowWidthItem(sal_False) );
                    pTextObject->SetMergedItemSet(aTempAttr);
                    pTextObject->SetLogicRect(aRect);

                    // switch on AutoGrowWidth
                    SfxItemSet aAttr( ((SdDrawDocument*) pModel)->GetPool() );
                    aAttr.Put( SdrTextAutoGrowWidthItem(sal_True) );
                    pTextObject->SetMergedItemSet(aAttr);
                }
            }
        }
    }

    if(pObj && bInit )
    {
        if( !IsPresObj( pObj ) )
        {
            if( bUndo )
                pUndoManager->AddUndoAction( new UndoObjectPresentationKind( *pObj ) );

            InsertPresObj( pObj, eObjKind );
        }

        // make adjustments for vertical title and outline shapes
        if( bVertical && (( eObjKind == PRESOBJ_TITLE) || (eObjKind == PRESOBJ_OUTLINE)))
        {
            SfxItemSet aNewSet(pObj->GetMergedItemSet());
            aNewSet.Put( SdrTextAutoGrowWidthItem(sal_True) );
            aNewSet.Put( SdrTextAutoGrowHeightItem(sal_False) );
            if( eObjKind == PRESOBJ_OUTLINE )
            {
                aNewSet.Put( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
                aNewSet.Put( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );
            }
            pObj->SetMergedItemSet(aNewSet);
        }
    }

    if ( pObj && (pObj->GetUserCall() || bInit) && ( pObj->IsEmptyPresObj() || !pObj->ISA(SdrGrafObj) ) )
        pObj->AdjustToMaxRect( aRect );

    return pObj;
}


/*************************************************************************
|*
|* Returns the PresObjKind of a object
|*
\************************************************************************/

PresObjKind SdPage::GetPresObjKind(SdrObject* pObj) const
{
    PresObjKind eKind = PRESOBJ_NONE;
    if( (pObj != 0) && (maPresentationShapeList.hasShape(*pObj)) )
    {
        SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pObj);
        if( pInfo )
            eKind = pInfo->mePresObjKind;
    }

    return eKind;
}

bool SdPage::IsPresObj(const SdrObject* pObj)
{
    return pObj && maPresentationShapeList.hasShape( const_cast<SdrObject&>(*pObj) );
}

void SdPage::RemovePresObj(const SdrObject* pObj)
{
    if( pObj && maPresentationShapeList.hasShape(const_cast<SdrObject&>(*pObj)) )
    {
        SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(const_cast<SdrObject&>(*pObj));
        if( pInfo )
            pInfo->mePresObjKind = PRESOBJ_NONE;
        maPresentationShapeList.removeShape(const_cast<SdrObject&>(*pObj));
    }
}

void SdPage::InsertPresObj(SdrObject* pObj, PresObjKind eKind )
{
    DBG_ASSERT( pObj, "sd::SdPage::InsertPresObj(), invalid presentation object inserted!" );
    DBG_ASSERT( !IsPresObj(pObj), "sd::SdPage::InsertPresObj(), presentation object inserted twice!" );
    if( pObj )
    {
        SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pObj, true);
        if( pInfo )
            pInfo->mePresObjKind = eKind;
        maPresentationShapeList.addShape(*pObj);
    }
}

/*************************************************************************
|*
|* Set the text of a object
|*
\************************************************************************/

void SdPage::SetObjText(SdrTextObj* pObj, SdrOutliner* pOutliner, PresObjKind eObjKind, const String& rString )
{
    if ( pObj )
    {
        DBG_ASSERT( pObj->ISA(SdrTextObj), "SetObjText: No SdrTextObj!" );
        ::Outliner* pOutl = pOutliner;

        if (!pOutliner)
        {
            SfxItemPool* pPool = ((SdDrawDocument*) GetModel())->GetDrawOutliner().GetEmptyItemSet().GetPool();
            pOutl = new ::Outliner( pPool, OUTLINERMODE_OUTLINEOBJECT );
            pOutl->SetRefDevice( SD_MOD()->GetRefDevice( *( (SdDrawDocument*) GetModel() )->GetDocSh() ) );
            pOutl->SetEditTextObjectPool(pPool);
            pOutl->SetStyleSheetPool((SfxStyleSheetPool*)GetModel()->GetStyleSheetPool());
            pOutl->EnableUndo(sal_False);
            pOutl->SetUpdateMode( sal_False );
        }

        sal_uInt16 nOutlMode = pOutl->GetMode();
        Size aPaperSize = pOutl->GetPaperSize();
        sal_Bool bUpdateMode = pOutl->GetUpdateMode();
        pOutl->SetUpdateMode(sal_False);
        pOutl->SetParaAttribs( 0, pOutl->GetEmptyItemSet() );

        // Always set the object's StyleSheet at the Outliner to
        // use the current objects StyleSheet. Thus it's the same as in
        // SetText(...).
        // Moved this implementation from where SetObjText(...) was called
        // to inside this method to work even when outliner is fetched here.
        pOutl->SetStyleSheet(0, pObj->GetStyleSheet());

        OUString aString;

        switch( eObjKind )
        {
            case PRESOBJ_OUTLINE:
            {
                pOutl->Init( OUTLINERMODE_OUTLINEOBJECT );

                aString += "\t";
                aString += rString;

                if (mbMaster)
                {
                    pOutl->SetStyleSheet( 0, GetStyleSheetForPresObj(eObjKind) );
                    aString += "\n\t\t";
                    aString += SD_RESSTR(STR_PRESOBJ_MPOUTLLAYER2);

                    aString += "\n\t\t\t";
                    aString += SD_RESSTR(STR_PRESOBJ_MPOUTLLAYER3);

                    aString += "\n\t\t\t\t";
                    aString += SD_RESSTR(STR_PRESOBJ_MPOUTLLAYER4);

                    aString += "\n\t\t\t\t\t";
                    aString += SD_RESSTR(STR_PRESOBJ_MPOUTLLAYER5);

                    aString += "\n\t\t\t\t\t\t";
                    aString += SD_RESSTR(STR_PRESOBJ_MPOUTLLAYER6);

                    aString += "\n\t\t\t\t\t\t\t";
                    aString += SD_RESSTR(STR_PRESOBJ_MPOUTLLAYER7);

                }
            }
            break;

            case PRESOBJ_TITLE:
            {
                pOutl->Init( OUTLINERMODE_TITLEOBJECT );
                aString += rString;
            }
            break;

            default:
            {
                pOutl->Init( OUTLINERMODE_TEXTOBJECT );
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
                default:
                    break;
                }

                if( pData )
                {
                    ESelection e;
                    SvxFieldItem aField( *pData, EE_FEATURE_FIELD );
                    pOutl->QuickInsertField(aField,e);
                    delete pData;
                }
            }
            break;
        }

        pOutl->SetPaperSize( pObj->GetLogicRect().GetSize() );

        if( !aString.isEmpty() )
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
            pOutl->SetPaperSize( aPaperSize );
        }
    }
}

/*************************************************************************
|*
|* Set the name of the layout
|*
\************************************************************************/
void SdPage::SetLayoutName(OUString aName)
{
    maLayoutName = aName;

    if( mbMaster )
    {
        OUString aSep(SD_LT_SEPARATOR);
        sal_Int32 nPos = maLayoutName.indexOf(aSep);
        if (nPos != -1)
            FmFormPage::SetName(maLayoutName.copy(0, nPos));
    }
}


/*************************************************************************
|*
|* Return the page name and generates it if necessary
|*
\************************************************************************/

const String& SdPage::GetName() const
{
    String aCreatedPageName( maCreatedPageName );
    if (GetRealName().Len() == 0)
    {
        if ((mePageKind == PK_STANDARD || mePageKind == PK_NOTES) && !mbMaster)
        {
            // default name for handout pages
            sal_uInt16  nNum = (GetPageNum() + 1) / 2;

            aCreatedPageName = SD_RESSTR(STR_PAGE);
            aCreatedPageName += sal_Unicode( ' ' );
            if( GetModel()->GetPageNumType() == SVX_NUMBER_NONE )
            {
                // if the document has number none as a formating
                // for page numbers we still default to arabic numbering
                // to keep the default page names unique
                aCreatedPageName += OUString::number( (sal_Int32)nNum );
            }
            else
            {
                aCreatedPageName += ((SdDrawDocument*) GetModel())->CreatePageNumValue(nNum);
            }
        }
        else
        {
            /******************************************************************
            * default name for note pages
            ******************************************************************/
            aCreatedPageName = SD_RESSTR(STR_LAYOUT_DEFAULT_NAME);
        }
    }
    else
    {
        aCreatedPageName = GetRealName();
    }

    if (mePageKind == PK_NOTES)
    {
        aCreatedPageName += sal_Unicode( ' ' );
        aCreatedPageName += SD_RESSTR(STR_NOTES);
    }
    else if (mePageKind == PK_HANDOUT && mbMaster)
    {
        aCreatedPageName += OUString(" (");
        aCreatedPageName += SdResId(STR_HANDOUT).toString();
        aCreatedPageName += sal_Unicode( ')' );
    }

    const_cast< SdPage* >(this)->maCreatedPageName = aCreatedPageName;
    return maCreatedPageName;
}

void SdPage::SetOrientation( Orientation eOrient)
{
    meOrientation = eOrient;
}

Orientation SdPage::GetOrientation() const
{
    return meOrientation;
}

/*************************************************************************
|*
|* returns the default text of a PresObjektes
|*
\************************************************************************/

String SdPage::GetPresObjText(PresObjKind eObjKind) const
{
    String aString;

    if (eObjKind == PRESOBJ_TITLE)
    {
        if (mbMaster)
        {
            if (mePageKind != PK_NOTES)
            {
                aString = SD_RESSTR( STR_PRESOBJ_MPTITLE );
            }
            else
            {
                aString = SD_RESSTR( STR_PRESOBJ_MPNOTESTITLE );
            }
        }
        else
        {
            aString = SD_RESSTR( STR_PRESOBJ_TITLE );
        }
    }
    else if (eObjKind == PRESOBJ_OUTLINE)
    {
        if (mbMaster)
        {
            aString = SD_RESSTR( STR_PRESOBJ_MPOUTLINE );
        }
        else
        {
            aString = SD_RESSTR( STR_PRESOBJ_OUTLINE );
        }
    }
    else if (eObjKind == PRESOBJ_NOTES)
    {
        if (mbMaster)
        {
            aString = SD_RESSTR( STR_PRESOBJ_MPNOTESTEXT );
        }
        else
        {
            aString = SD_RESSTR( STR_PRESOBJ_NOTESTEXT );
        }
    }
    else if (eObjKind == PRESOBJ_TEXT)
    {
        aString = SD_RESSTR( STR_PRESOBJ_TEXT );
    }
    else if (eObjKind == PRESOBJ_GRAPHIC)
    {
        aString = SD_RESSTR( STR_PRESOBJ_GRAPHIC );
    }
    else if (eObjKind == PRESOBJ_OBJECT)
    {
        aString = SD_RESSTR( STR_PRESOBJ_OBJECT );
    }
    else if (eObjKind == PRESOBJ_CHART)
    {
        aString = SD_RESSTR( STR_PRESOBJ_CHART );
    }
    else if (eObjKind == PRESOBJ_ORGCHART)
    {
        aString = SD_RESSTR( STR_PRESOBJ_ORGCHART );
    }
    else if (eObjKind == PRESOBJ_CALC)
    {
        aString = SD_RESSTR( STR_PRESOBJ_TABLE );
    }

    return(aString);
}

extern uno::Reference< uno::XInterface > createUnoPageImpl( SdPage* pPage );

uno::Reference< uno::XInterface > SdPage::createUnoPage()
{
    return createUnoPageImpl( this );
}

/** returns the SdPage implementation for the given XDrawPage or 0 if not available */
SdPage* SdPage::getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage )
{
    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUnoTunnel( xPage, ::com::sun::star::uno::UNO_QUERY );
        if( xUnoTunnel.is() )
        {
            SvxDrawPage* pUnoPage = reinterpret_cast<SvxDrawPage*>(sal::static_int_cast<sal_uIntPtr>(xUnoTunnel->getSomething( SvxDrawPage::getUnoTunnelId()) ) );
            if( pUnoPage )
                return static_cast< SdPage* >( pUnoPage->GetSdrPage() );
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        OSL_FAIL("sd::SdPage::getImplementation(), exception caught!" );
    }

    return 0;
}

void SdPage::SetName (const String& rName)
{
    String aOldName = GetName();
    FmFormPage::SetName (rName);
    static_cast<SdDrawDocument*>(pModel)->UpdatePageRelativeURLs(aOldName, rName);
    ActionChanged();
}

const HeaderFooterSettings& SdPage::getHeaderFooterSettings() const
{
    if( mePageKind == PK_HANDOUT && !mbMaster )
    {
        return (((SdPage&)TRG_GetMasterPage()).maHeaderFooterSettings);
    }
    else
    {
        return maHeaderFooterSettings;
    }
}

void SdPage::setHeaderFooterSettings( const sd::HeaderFooterSettings& rNewSettings )
{
    if( mePageKind == PK_HANDOUT && !mbMaster )
    {
        (((SdPage&)TRG_GetMasterPage()).maHeaderFooterSettings) = rNewSettings;
    }
    else
    {
        maHeaderFooterSettings = rNewSettings;
    }

    SetChanged();
    if(TRG_HasMasterPage())
    {
        TRG_GetMasterPageDescriptorViewContact().ActionChanged();
    }
}

bool SdPage::checkVisibility(
    const sdr::contact::ViewObjectContact& rOriginal,
    const sdr::contact::DisplayInfo& rDisplayInfo,
    bool bEdit )
{
    if( !FmFormPage::checkVisibility( rOriginal, rDisplayInfo, bEdit ) )
        return false;

    SdrObject* pObj = rOriginal.GetViewContact().TryToGetSdrObject();
    if( pObj == NULL )
        return false;

    const SdrPage* pVisualizedPage = GetSdrPageFromXDrawPage(rOriginal.GetObjectContact().getViewInformation2D().getVisualizedPage());
    const bool bIsPrinting(rOriginal.GetObjectContact().isOutputToPrinter() || rOriginal.GetObjectContact().isOutputToPDFFile());
    const SdrPageView* pPageView = rOriginal.GetObjectContact().TryToGetSdrPageView();
    const bool bIsInsidePageObj(pPageView && pPageView->GetPage() != pVisualizedPage);

    // empty presentation objects only visible during edit mode
    if( (bIsPrinting || !bEdit || bIsInsidePageObj ) && pObj->IsEmptyPresObj() )
    {
        if( (pObj->GetObjInventor() != SdrInventor) || ( (pObj->GetObjIdentifier() != OBJ_RECT) && (pObj->GetObjIdentifier() != OBJ_PAGE) ) )
            return false;
    }

    if( ( pObj->GetObjInventor() == SdrInventor ) && ( pObj->GetObjIdentifier() == OBJ_TEXT ) )
    {
           const SdPage* pCheckPage = dynamic_cast< const SdPage* >(pObj->GetPage());

        if( pCheckPage )
        {
            PresObjKind eKind = pCheckPage->GetPresObjKind(pObj);

            if((eKind == PRESOBJ_FOOTER) || (eKind == PRESOBJ_HEADER) || (eKind == PRESOBJ_DATETIME) || (eKind == PRESOBJ_SLIDENUMBER) )
            {
                const bool bSubContentProcessing(rDisplayInfo.GetSubContentActive());

                if( bSubContentProcessing || ( pCheckPage->GetPageKind() == PK_HANDOUT && bIsPrinting ) )
                {
                    // use the page that is currently processed
                    const SdPage* pVisualizedSdPage = dynamic_cast< const SdPage* >(pVisualizedPage);

                    if( pVisualizedSdPage )
                    {
                        // if we are not on a masterpage, see if we have to draw this header&footer object at all
                        const sd::HeaderFooterSettings& rSettings = pVisualizedSdPage->getHeaderFooterSettings();

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
                        default:
                            break;
                        }
                    }
                }
            } // check for placeholders on master
            else if( (eKind != PRESOBJ_NONE) && pCheckPage->IsMasterPage() && ( pVisualizedPage != pCheckPage ) )
            {
                // presentation objects on master slide are always invisible if slide is shown.
                return false;
            }
        }
    }

    // i63977, do not print SdrpageObjs from master pages
    if( ( pObj->GetObjInventor() == SdrInventor ) && ( pObj->GetObjIdentifier() == OBJ_PAGE ) )
    {
        if( pObj->GetPage() && pObj->GetPage()->IsMasterPage() )
            return false;
    }

    return true;
}

bool SdPage::RestoreDefaultText( SdrObject* pObj )
{
    bool bRet = false;

    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( pObj );

    if( pTextObj )
    {
        PresObjKind ePresObjKind = GetPresObjKind(pTextObj);

        if (ePresObjKind == PRESOBJ_TITLE   ||
            ePresObjKind == PRESOBJ_OUTLINE ||
            ePresObjKind == PRESOBJ_NOTES   ||
            ePresObjKind == PRESOBJ_TEXT)
        {
            String aString( GetPresObjText(ePresObjKind) );

            if (aString.Len())
            {
                sal_Bool bVertical = sal_False;
                OutlinerParaObject* pOldPara = pTextObj->GetOutlinerParaObject();
                if( pOldPara )
                    bVertical = pOldPara->IsVertical();  // is old para object vertical?

                SetObjText( pTextObj, 0, ePresObjKind, aString );

                if( pOldPara )
                {
                    // Here, only the vertical flag for the
                    // OutlinerParaObjects needs to be changed. The
                    // AutoGrowWidth/Height items still exist in the
                    // not changed object.
                    if(pTextObj
                        && pTextObj->GetOutlinerParaObject()
                        && pTextObj->GetOutlinerParaObject()->IsVertical() != (bool)bVertical)
                    {
                        Rectangle aObjectRect = pTextObj->GetSnapRect();
                        pTextObj->GetOutlinerParaObject()->SetVertical(bVertical);
                        pTextObj->SetSnapRect(aObjectRect);
                    }
                }

                pTextObj->SetTextEditOutliner( NULL );  // to make stylesheet settings work
                pTextObj->NbcSetStyleSheet( GetStyleSheetForPresObj(ePresObjKind), sal_True );
                pTextObj->SetEmptyPresObj(sal_True);
                bRet = true;
            }
        }
    }
    return bRet;
}

void SdPage::CalculateHandoutAreas( SdDrawDocument& rModel, AutoLayout eLayout, bool bHorizontal, std::vector< Rectangle >& rAreas )
{
    SdPage& rHandoutMaster = *rModel.GetMasterSdPage( 0, PK_HANDOUT );

    if( eLayout == AUTOLAYOUT_NONE )
    {
        // use layout from handout master
        SdrObjListIter aShapeIter (rHandoutMaster);
        while (aShapeIter.IsMore())
        {
            SdrPageObj* pPageObj = dynamic_cast<SdrPageObj*>(aShapeIter.Next());
            if (pPageObj)
                rAreas.push_back( pPageObj->GetCurrentBoundRect() );
        }
    }
    else
    {
        Size    aArea = rHandoutMaster.GetSize();

        const long nGapW = 1000; // gap is 1cm
        const long nGapH = 1000;

        long nLeftBorder = rHandoutMaster.GetLftBorder();
        long nRightBorder = rHandoutMaster.GetRgtBorder();
        long nTopBorder = rHandoutMaster.GetUppBorder();
        long nBottomBorder = rHandoutMaster.GetLwrBorder();

        const long nHeaderFooterHeight = static_cast< long >( (aArea.Height() - nTopBorder - nLeftBorder) * 0.05  );

        nTopBorder += nHeaderFooterHeight;
        nBottomBorder += nHeaderFooterHeight;

        long nX = nGapW + nLeftBorder;
        long nY = nGapH + nTopBorder;

        aArea.Width() -= nGapW * 2 + nLeftBorder + nRightBorder;
        aArea.Height() -= nGapH * 2 + nTopBorder + nBottomBorder;

        const bool bLandscape = aArea.Width() > aArea.Height();

        static const sal_uInt16 aOffsets[5][9] =
        {
            { 0, 1, 2, 3, 4, 5, 6, 7, 8 }, // AUTOLAYOUT_HANDOUT9, Portrait, Horizontal order
            { 0, 2, 4, 1, 3, 5, 0, 0, 0 }, // AUTOLAYOUT_HANDOUT3, Landscape, Vertical
            { 0, 2, 1, 3, 0, 0, 0, 0, 0 }, // AUTOLAYOUT_HANDOUT4, Landscape, Vertical
            { 0, 3, 1, 4, 2, 5, 0, 0, 0 }, // AUTOLAYOUT_HANDOUT4, Portrait, Vertical
            { 0, 3, 6, 1, 4, 7, 2, 5, 8 }, // AUTOLAYOUT_HANDOUT9, Landscape, Vertical
        };

        const sal_uInt16* pOffsets = aOffsets[0];
        sal_uInt16  nColCnt = 0, nRowCnt = 0;
        switch ( eLayout )
        {
            case AUTOLAYOUT_HANDOUT1:
                nColCnt = 1; nRowCnt = 1;
                break;

            case AUTOLAYOUT_HANDOUT2:
                if( bLandscape )
                {
                    nColCnt = 2; nRowCnt = 1;
                }
                else
                {
                    nColCnt = 1; nRowCnt = 2;
                }
                break;

            case AUTOLAYOUT_HANDOUT3:
                if( bLandscape )
                {
                    nColCnt = 3; nRowCnt = 2;
                }
                else
                {
                    nColCnt = 2; nRowCnt = 3;
                }
                pOffsets = aOffsets[ bLandscape ? 1 : 0 ];
                break;

            case AUTOLAYOUT_HANDOUT4:
                nColCnt = 2; nRowCnt = 2;
                pOffsets = aOffsets[ bHorizontal ? 0 : 2 ];
                break;

            case AUTOLAYOUT_HANDOUT6:
                if( bLandscape )
                {
                    nColCnt = 3; nRowCnt = 2;
                }
                else
                {
                    nColCnt = 2; nRowCnt = 3;
                }
                if( !bHorizontal )
                    pOffsets = aOffsets[ bLandscape ? 1 : 3 ];
                break;

            default:
            case AUTOLAYOUT_HANDOUT9:
                nColCnt = 3; nRowCnt = 3;

                if( !bHorizontal )
                    pOffsets = aOffsets[4];
                break;
        }

        rAreas.resize( nColCnt * nRowCnt );

        Size aPartArea, aSize;
        aPartArea.Width()  = ((aArea.Width()  - ((nColCnt-1) * nGapW) ) / nColCnt);
        aPartArea.Height() = ((aArea.Height() - ((nRowCnt-1) * nGapH) ) / nRowCnt);

        SdrPage* pFirstPage = rModel.GetMasterSdPage(0, PK_STANDARD);
        if ( pFirstPage )
        {
            // scale actual size into handout rect
            double fScale = (double)aPartArea.Width() / (double)pFirstPage->GetWdt();

            aSize.Height() = (long)(fScale * pFirstPage->GetHgt() );
            if( aSize.Height() > aPartArea.Height() )
            {
                fScale = (double)aPartArea.Height() / (double)pFirstPage->GetHgt();
                aSize.Height() = aPartArea.Height();
                aSize.Width() = (long)(fScale * pFirstPage->GetWdt());
            }
            else
            {
                aSize.Width() = aPartArea.Width();
            }

            nX += (aPartArea.Width() - aSize.Width()) / 2;
            nY += (aPartArea.Height()- aSize.Height())/ 2;
        }
        else
        {
            aSize = aPartArea;
        }

        Point aPos( nX, nY );

        const bool bRTL = rModel.GetDefaultWritingMode() == ::com::sun::star::text::WritingMode_RL_TB;

        const long nOffsetX = (aPartArea.Width() + nGapW) * (bRTL ? -1 : 1);
        const long nOffsetY = aPartArea.Height() + nGapH;
        const long nStartX = bRTL ? nOffsetX*(1 - nColCnt) + nX : nX;

        for(sal_uInt16 nRow = 0; nRow < nRowCnt; nRow++)
        {
            aPos.X() = nStartX;
            for(sal_uInt16 nCol = 0; nCol < nColCnt; nCol++)
            {
                rAreas[*pOffsets++] = Rectangle(aPos, aSize);
                aPos.X() += nOffsetX;
            }

            aPos.Y() += nOffsetY;
        }
    }
}




void SdPage::SetPrecious (const bool bIsPrecious)
{
    mbIsPrecious = bIsPrecious;
}




bool SdPage::IsPrecious (void) const
{
    return mbIsPrecious;
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

bool HeaderFooterSettings::operator==( const HeaderFooterSettings& rSettings ) const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
