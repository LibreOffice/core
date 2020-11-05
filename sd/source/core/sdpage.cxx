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
#include <array>

#include <comphelper/classids.hxx>
#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/lok.hxx>

#include <vcl/svapp.hxx>
#include <editeng/outliner.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editdata.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/bulletitem.hxx>
#include <svx/svdpagv.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdopage.hxx>
#include <sfx2/lokhelper.hxx>
#include <editeng/pbinitem.hxx>
#include <svx/svdundo.hxx>
#include <svl/hint.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/editobj.hxx>
#include <svx/unopage.hxx>
#include <editeng/flditem.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/svditer.hxx>
#include <svx/svdlayer.hxx>
#include <svx/sdtmfitm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtcfitm.hxx>
#include <svx/xfillit0.hxx>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XTimeContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <o3tl/enumarray.hxx>
#include <o3tl/safeint.hxx>
#include <xmloff/autolayout.hxx>

#include <Outliner.hxx>
#include <app.hrc>
#include <createunopageimpl.hxx>
#include <drawdoc.hxx>
#include <sdmod.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <stlsheet.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <bitmaps.hlst>
#include <glob.hxx>
#include <anminfo.hxx>
#include <undo/undomanager.hxx>
#include <undo/undoobjects.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/unoapi.hxx>
#include <unokywds.hxx>

#include <set>

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using ::com::sun::star::uno::Reference;


sal_uInt16 SdPage::mnLastPageId = 1;

/*************************************************************************
|*
|*      Ctor
|*
\************************************************************************/

SdPage::SdPage(SdDrawDocument& rNewDoc, bool bMasterPage)
:   FmFormPage(rNewDoc, bMasterPage)
,   SdrObjUserCall()
,   mePageKind(PageKind::Standard)
,   meAutoLayout(AUTOLAYOUT_NONE)
,   mbSelected(false)
,   mePresChange(PresChange::Manual)
,   mfTime(1.0)
,   mbSoundOn(false)
,   mbExcluded(false)
,   mbLoopSound(false)
,   mbStopSound(false)
,   mbScaleObjects(true)
,   mbBackgroundFullSize( false )
,   meCharSet(osl_getThreadTextEncoding())
,   mnPaperBin(PAPERBIN_PRINTER_SETTINGS)
,   mpPageLink(nullptr)
,   mnTransitionType(0)
,   mnTransitionSubtype(0)
,   mbTransitionDirection(true)
,   mnTransitionFadeColor(0)
,   mfTransitionDuration(2.0)
,   mbIsPrecious(true)
,   mnPageId(mnLastPageId++)
{
    // The name of the layout of the page is used by SVDRAW to determine the
    // presentation template of the outline objects. Therefore, it already
    // contains the designator for the outline (STR_LAYOUT_OUTLINE).
    maLayoutName = SdResId(STR_LAYOUT_DEFAULT_NAME)+ SD_LT_SEPARATOR STR_LAYOUT_OUTLINE;

    // Stuff that former SetModel did also:
    ConnectLink();
}

namespace
{
    void clearChildNodes(css::uno::Reference<css::animations::XAnimationNode> const & rAnimationNode)
    {
        css::uno::Reference<css::container::XEnumerationAccess > xEnumerationAccess(rAnimationNode, UNO_QUERY);
        if (!xEnumerationAccess.is())
            return;
        css::uno::Reference<css::container::XEnumeration> xEnumeration = xEnumerationAccess->createEnumeration();
        if (!xEnumeration.is())
            return;
        while (xEnumeration->hasMoreElements())
        {
            css::uno::Reference<css::animations::XAnimationNode> xChildNode(xEnumeration->nextElement(), UNO_QUERY);
            if (!xChildNode.is())
                continue;
            clearChildNodes(xChildNode);
            css::uno::Reference<css::animations::XTimeContainer> xAnimationNode(rAnimationNode, UNO_QUERY);
            if (!xAnimationNode.is())
            {
                SAL_WARN("sd.core", "can't remove node child, possible leak");
                continue;
            }
            xAnimationNode->removeChild(xChildNode);
        }
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

    clearChildNodes(mxAnimationNode);

    // clear SdrObjects with broadcasting
    ClearSdrObjList();
}

namespace {

struct OrdNumSorter
{
    bool operator()( SdrObject const * p1, SdrObject const * p2 )
    {
        return p1->GetOrdNum() < p2->GetOrdNum();
    }
};

}

/** returns the nIndex'th object from the given PresObjKind, index starts with 1 */
SdrObject* SdPage::GetPresObj(PresObjKind eObjKind, int nIndex, bool bFuzzySearch /* = false */ )
{
    // first sort all matching shapes with z-order
    std::vector< SdrObject* > aMatches;

    SdrObject* pObj = nullptr;
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
            else if( bFuzzySearch && (eObjKind == PresObjKind::Outline) )
            {
                switch( pInfo->mePresObjKind )
                {
                case PresObjKind::Graphic:
                case PresObjKind::Object:
                case PresObjKind::Chart:
                case PresObjKind::OrgChart:
                case PresObjKind::Table:
                case PresObjKind::Calc:
                case PresObjKind::Media:
                    bFound = true;
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

    if( nIndex > 0 )
        nIndex--;

    if( (nIndex >= 0) && ( aMatches.size() > o3tl::make_unsigned(nIndex)) )
    {
        if( aMatches.size() > 1 )
            std::nth_element( aMatches.begin(), aMatches.begin() + nIndex, aMatches.end(),
                              OrdNumSorter() );
        return aMatches[nIndex];
    }

    return nullptr;
}

/** create background properties */
void SdPage::EnsureMasterPageDefaultBackground()
{
    if(!mbMaster)
        return;

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
        // no style found, assert and set at least drawing::FillStyle_NONE
        OSL_FAIL("No Style for MasterPageBackground fill found (!)");
        getSdrPageProperties().PutItem(XFillStyleItem(drawing::FillStyle_NONE));
    }
}

/** creates a presentation object with the given PresObjKind on this page. A user call will be set
*/
SdrObject* SdPage::CreatePresObj(PresObjKind eObjKind, bool bVertical, const ::tools::Rectangle& rRect )
{
    SfxUndoManager* pUndoManager(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetUndoManager());
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    SdrObject* pSdrObj = nullptr;

    bool bForceText = false;    // forces the shape text to be set even if it's empty
    bool bEmptyPresObj = true;

    switch( eObjKind )
    {
        case PresObjKind::Title:
        {
            pSdrObj = new SdrRectObj(getSdrModelFromSdrPage(), OBJ_TITLETEXT);

            if (mbMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(true);
            }
        }
        break;

        case PresObjKind::Outline:
        {
            pSdrObj = new SdrRectObj(getSdrModelFromSdrPage(), OBJ_OUTLINETEXT);

            if (mbMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(true);
            }
        }
        break;

        case PresObjKind::Notes:
        {
            pSdrObj = new SdrRectObj(getSdrModelFromSdrPage(), OBJ_TEXT);

            if (mbMaster)
            {
                pSdrObj->SetNotVisibleAsMaster(true);
            }
        }
        break;

        case PresObjKind::Text:
        {
            pSdrObj = new SdrRectObj(getSdrModelFromSdrPage(), OBJ_TEXT);
        }
        break;

        case PresObjKind::Graphic:
        {
            BitmapEx aBmpEx(BMP_PRESOBJ_GRAPHIC);
            Graphic  aGraphic( aBmpEx );
            OutputDevice &aOutDev = *Application::GetDefaultDevice();
            aOutDev.Push();

            aOutDev.SetMapMode( aGraphic.GetPrefMapMode() );
            Size aSizePix = aOutDev.LogicToPixel( aGraphic.GetPrefSize() );
            aOutDev.SetMapMode(MapMode(MapUnit::Map100thMM));

            Size aSize = aOutDev.PixelToLogic(aSizePix);
            Point aPnt (0, 0);
            ::tools::Rectangle aRect (aPnt, aSize);
            pSdrObj = new SdrGrafObj(getSdrModelFromSdrPage(), aGraphic, aRect);
            aOutDev.Pop();
        }
        break;

        case PresObjKind::Media:
        case PresObjKind::Object:
        {
            pSdrObj = new SdrOle2Obj(getSdrModelFromSdrPage());
            BitmapEx aBmpEx(BMP_PRESOBJ_OBJECT);
            Graphic aGraphic( aBmpEx );
            static_cast<SdrOle2Obj*>(pSdrObj)->SetGraphic(aGraphic);
        }
        break;

        case PresObjKind::Chart:
        {
            pSdrObj = new SdrOle2Obj(getSdrModelFromSdrPage());
            static_cast<SdrOle2Obj*>(pSdrObj)->SetProgName( "StarChart" );
            BitmapEx aBmpEx(BMP_PRESOBJ_CHART);
            Graphic aGraphic( aBmpEx );
            static_cast<SdrOle2Obj*>(pSdrObj)->SetGraphic(aGraphic);
        }
        break;

        case PresObjKind::OrgChart:
        {
            pSdrObj = new SdrOle2Obj(getSdrModelFromSdrPage());
            static_cast<SdrOle2Obj*>(pSdrObj)->SetProgName( "StarOrg" );
            BitmapEx aBmpEx(BMP_PRESOBJ_ORGCHART);
            Graphic aGraphic( aBmpEx );
            static_cast<SdrOle2Obj*>(pSdrObj)->SetGraphic(aGraphic);
        }
        break;

        case PresObjKind::Table:
        case PresObjKind::Calc:
        {
            pSdrObj = new SdrOle2Obj(getSdrModelFromSdrPage());
            static_cast<SdrOle2Obj*>(pSdrObj)->SetProgName( "StarCalc" );
            BitmapEx aBmpEx(BMP_PRESOBJ_TABLE);
            Graphic aGraphic( aBmpEx );
            static_cast<SdrOle2Obj*>(pSdrObj)->SetGraphic(aGraphic);
        }
        break;

        case PresObjKind::Handout:
        {
            // Save the first standard page at SdrPageObj
            // #i105146# We want no content to be displayed for PageKind::Handout,
            // so just never set a page as content
            pSdrObj = new SdrPageObj(getSdrModelFromSdrPage(), nullptr);
        }
        break;

        case PresObjKind::Page:
        {
            // Save note pages at SdrPageObj
            sal_uInt16 nDestPageNum(GetPageNum());

            if(nDestPageNum)
            {
                // decrement only when != 0, else we get a 0xffff
                nDestPageNum -= 1;
            }

            if (nDestPageNum < getSdrModelFromSdrPage().GetPageCount())
            {
                pSdrObj = new SdrPageObj(getSdrModelFromSdrPage(), getSdrModelFromSdrPage().GetPage(nDestPageNum));
            }
            else
            {
                pSdrObj = new SdrPageObj(getSdrModelFromSdrPage());
            }

            pSdrObj->SetResizeProtect(true);
        }
        break;

        case PresObjKind::Header:
        case PresObjKind::Footer:
        case PresObjKind::DateTime:
        case PresObjKind::SlideNumber:
        {
            pSdrObj = new SdrRectObj(getSdrModelFromSdrPage(), OBJ_TEXT);
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

        if ( dynamic_cast< const SdrTextObj *>( pSdrObj ) !=  nullptr )
        {
            // Tell the object EARLY that it is vertical to have the
            // defaults for AutoGrowWidth/Height reversed
            if(bVertical)
                static_cast<SdrTextObj*>(pSdrObj)->SetVerticalWriting(true);

            SfxItemSet aTempAttr(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetPool());
            if( bVertical )
                aTempAttr.Put( makeSdrTextMinFrameWidthItem( rRect.GetSize().Width() ) );
            else
                aTempAttr.Put( makeSdrTextMinFrameHeightItem( rRect.GetSize().Height() ) );

            if (mbMaster)
            {
                // The size of presentation objects on the master page have to
                // be freely selectable by the user.

                // potential problem: This action was still NOT
                // adapted for vertical text. This sure needs to be done.
                if(bVertical)
                    aTempAttr.Put(makeSdrTextAutoGrowWidthItem(false));
                else
                    aTempAttr.Put(makeSdrTextAutoGrowHeightItem(false));
            }

            // check if we need another vertical adjustment than the default
            SdrTextVertAdjust eV = SDRTEXTVERTADJUST_TOP;

            if( (eObjKind == PresObjKind::Footer) && (mePageKind != PageKind::Standard) )
            {
                eV = SDRTEXTVERTADJUST_BOTTOM;
            }
            else if( (eObjKind == PresObjKind::SlideNumber) && (mePageKind != PageKind::Standard) )
            {
                eV = SDRTEXTVERTADJUST_BOTTOM;
            }

            if( eV != SDRTEXTVERTADJUST_TOP )
                aTempAttr.Put(SdrTextVertAdjustItem(eV));

            pSdrObj->SetMergedItemSet(aTempAttr);

            pSdrObj->SetLogicRect(rRect);
        }

        OUString aString = GetPresObjText(eObjKind);
        if( (!aString.isEmpty() || bForceText) && dynamic_cast< const SdrTextObj *>( pSdrObj ) !=  nullptr )
        {
            SdrOutliner* pOutliner = static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetInternalOutliner();

            OutlinerMode nOutlMode = pOutliner->GetMode();
            pOutliner->Init( OutlinerMode::TextObject );
            pOutliner->SetStyleSheet( 0, nullptr );
            pOutliner->SetVertical( bVertical );

            SetObjText( static_cast<SdrTextObj*>(pSdrObj), pOutliner, eObjKind, aString );

            pOutliner->Init( nOutlMode );
            pOutliner->SetStyleSheet( 0, nullptr );
        }

        if( (eObjKind == PresObjKind::Header) || (eObjKind == PresObjKind::Footer) || (eObjKind == PresObjKind::SlideNumber) || (eObjKind == PresObjKind::DateTime) )
        {
            SfxItemSet aTempAttr(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetPool());
            aTempAttr.Put( SvxFontHeightItem( 493, 100, EE_CHAR_FONTHEIGHT ) );
            aTempAttr.Put( SvxFontHeightItem( 493, 100, EE_CHAR_FONTHEIGHT_CTL ) );
            aTempAttr.Put( SvxFontHeightItem( 493, 100, EE_CHAR_FONTHEIGHT_CJK ) );

            SvxAdjust eH = SvxAdjust::Left;

            if( (eObjKind == PresObjKind::DateTime) && (mePageKind != PageKind::Standard ) )
            {
                eH = SvxAdjust::Right;
            }
            else if( (eObjKind == PresObjKind::Footer) && (mePageKind == PageKind::Standard ) )
            {
                eH = SvxAdjust::Center;
            }
            else if( eObjKind == PresObjKind::SlideNumber )
            {
                eH = SvxAdjust::Right;
            }

            if( eH != SvxAdjust::Left )
                aTempAttr.Put(SvxAdjustItem(eH, EE_PARA_JUST ));

            pSdrObj->SetMergedItemSet(aTempAttr);
        }

        if (mbMaster)
        {
            SdrLayerAdmin& rLayerAdmin(getSdrModelFromSdrPage().GetLayerAdmin());

            // background objects of the master page
            pSdrObj->SetLayer( rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects) );
        }

        // Subscribe object at the style sheet
        // Set style only when one was found (as in 5.2)
        if( mePageKind != PageKind::Handout )
        {
            SfxStyleSheet* pSheetForPresObj = GetStyleSheetForPresObj(eObjKind);
            if(pSheetForPresObj)
                pSdrObj->SetStyleSheet(pSheetForPresObj, false);
        }

        if (eObjKind == PresObjKind::Outline)
        {
            for (sal_uInt16 nLevel = 1; nLevel < 10; nLevel++)
            {
                OUString aName( maLayoutName + " " + OUString::number( nLevel ) );
                SfxStyleSheet* pSheet = static_cast<SfxStyleSheet*>(getSdrModelFromSdrPage().GetStyleSheetPool()->Find(aName, SfxStyleFamily::Page));
                DBG_ASSERT(pSheet, "StyleSheet for outline object not found");
                if (pSheet)
                    pSdrObj->StartListening(*pSheet, DuplicateHandling::Allow);
            }
        }

        if ( eObjKind == PresObjKind::Object   ||
             eObjKind == PresObjKind::Chart    ||
             eObjKind == PresObjKind::OrgChart ||
             eObjKind == PresObjKind::Calc    ||
             eObjKind == PresObjKind::Graphic )
        {
            SfxItemSet aSet( static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetPool() );
            aSet.Put( makeSdrTextContourFrameItem( true ) );
            aSet.Put( SvxAdjustItem( SvxAdjust::Center, EE_PARA_JUST ) );

            pSdrObj->SetMergedItemSet(aSet);
        }

        if( bUndo )
        {
            pUndoManager->AddUndoAction(getSdrModelFromSdrPage().GetSdrUndoFactory().CreateUndoNewObject(*pSdrObj));

            pUndoManager->AddUndoAction( std::make_unique<UndoObjectPresentationKind>( *pSdrObj ) );
            pUndoManager->AddUndoAction( std::make_unique<UndoObjectUserCall>(*pSdrObj) );
        }

        InsertPresObj(pSdrObj, eObjKind);
        pSdrObj->SetUserCall(this);

        pSdrObj->RecalcBoundRect();
    }

    return pSdrObj;
}

/*************************************************************************
|*
|* Creates presentation objects on the master page.
|* All presentation objects get a UserCall to the page.
|*
\************************************************************************/

SfxStyleSheet* SdPage::GetStyleSheetForMasterPageBackground() const
{
    OUString aName(GetLayoutName());
    OUString aSep( SD_LT_SEPARATOR );
    sal_Int32 nPos = aName.indexOf(aSep);

    if (nPos != -1)
    {
        nPos = nPos + aSep.getLength();
        aName = aName.copy(0, nPos);
    }

    aName += STR_LAYOUT_BACKGROUND;

    SfxStyleSheetBasePool* pStShPool = getSdrModelFromSdrPage().GetStyleSheetPool();
    SfxStyleSheetBase*     pResult   = pStShPool->Find(aName, SfxStyleFamily::Page);
    return static_cast<SfxStyleSheet*>(pResult);
}

SfxStyleSheet* SdPage::GetStyleSheetForPresObj(PresObjKind eObjKind) const
{
    OUString aName(GetLayoutName());
    OUString aSep( SD_LT_SEPARATOR );
    sal_Int32 nPos = aName.indexOf(aSep);
    if (nPos != -1)
    {
        nPos = nPos + aSep.getLength();
        aName = aName.copy(0, nPos);
    }

    switch (eObjKind)
    {
        case PresObjKind::Outline:
        {
            aName = GetLayoutName() + " " + OUString::number( 1 );
        }
        break;

        case PresObjKind::Title:
            aName += STR_LAYOUT_TITLE;
            break;

        case PresObjKind::Notes:
            aName += STR_LAYOUT_NOTES;
            break;

        case PresObjKind::Text:
            aName += STR_LAYOUT_SUBTITLE;
            break;

        case PresObjKind::Header:
        case PresObjKind::Footer:
        case PresObjKind::DateTime:
        case PresObjKind::SlideNumber:
            aName += STR_LAYOUT_BACKGROUNDOBJECTS;
            break;

        default:
            break;
    }

    SfxStyleSheetBasePool* pStShPool = getSdrModelFromSdrPage().GetStyleSheetPool();
    SfxStyleSheetBase*     pResult   = pStShPool->Find(aName, SfxStyleFamily::Page);
    return static_cast<SfxStyleSheet*>(pResult);
}

/** returns the presentation style with the given helpid from this masterpage or this
    slides masterpage */
SdStyleSheet* SdPage::getPresentationStyle( sal_uInt32 nHelpId ) const
{
    OUString aStyleName( GetLayoutName() );
    const OUString aSep( SD_LT_SEPARATOR );
    sal_Int32 nIndex = aStyleName.indexOf(aSep);
    if( nIndex != -1 )
        aStyleName = aStyleName.copy(0, nIndex + aSep.getLength());

    const char *pNameId;
    bool bOutline = false;
    switch( nHelpId )
    {
    case HID_PSEUDOSHEET_TITLE:             pNameId = STR_LAYOUT_TITLE;             break;
    case HID_PSEUDOSHEET_SUBTITLE:          pNameId = STR_LAYOUT_SUBTITLE;          break;
    case HID_PSEUDOSHEET_OUTLINE1:
    case HID_PSEUDOSHEET_OUTLINE2:
    case HID_PSEUDOSHEET_OUTLINE3:
    case HID_PSEUDOSHEET_OUTLINE4:
    case HID_PSEUDOSHEET_OUTLINE5:
    case HID_PSEUDOSHEET_OUTLINE6:
    case HID_PSEUDOSHEET_OUTLINE7:
    case HID_PSEUDOSHEET_OUTLINE8:
    case HID_PSEUDOSHEET_OUTLINE9:          pNameId = STR_LAYOUT_OUTLINE; bOutline = true; break;
    case HID_PSEUDOSHEET_BACKGROUNDOBJECTS: pNameId = STR_LAYOUT_BACKGROUNDOBJECTS; break;
    case HID_PSEUDOSHEET_BACKGROUND:        pNameId = STR_LAYOUT_BACKGROUND;        break;
    case HID_PSEUDOSHEET_NOTES:             pNameId = STR_LAYOUT_NOTES;             break;

    default:
        OSL_FAIL( "SdPage::getPresentationStyle(), illegal argument!" );
        return nullptr;
    }
    aStyleName += OUString::createFromAscii(pNameId);
    if (bOutline)
    {
        aStyleName += " " +
            OUString::number( sal_Int32( nHelpId - HID_PSEUDOSHEET_OUTLINE ));
    }

    SfxStyleSheetBasePool* pStShPool = getSdrModelFromSdrPage().GetStyleSheetPool();
    SfxStyleSheetBase*     pResult   = pStShPool->Find(aStyleName, SfxStyleFamily::Page);
    return dynamic_cast<SdStyleSheet*>(pResult);
}

/*************************************************************************
|*
|* The presentation object rObj has changed and is no longer referenced by the
|* presentation object of the master page.
|* The UserCall is deleted.
|*
\************************************************************************/

void SdPage::Changed(const SdrObject& rObj, SdrUserCallType eType, const ::tools::Rectangle& )
{
    if (maLockAutoLayoutArrangement.isLocked())
        return;

    switch (eType)
    {
        case SdrUserCallType::MoveOnly:
        case SdrUserCallType::Resize:
        {
            if ( getSdrModelFromSdrPage().isLocked())
                break;

            if (!mbMaster)
            {
                if (rObj.GetUserCall())
                {
                    SdrObject& _rObj = const_cast<SdrObject&>(rObj);
                    SfxUndoManager* pUndoManager
                        = static_cast<SdDrawDocument&>(getSdrModelFromSdrPage())
                              .GetUndoManager();
                    const bool bUndo
                        = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

                    if (bUndo)
                        pUndoManager->AddUndoAction(
                            std::make_unique<UndoObjectUserCall>(_rObj));

                    // Object was resized by user and does not listen to its slide anymore
                    _rObj.SetUserCall(nullptr);
                }
            }
            else
            {
                // Object of the master page changed, therefore adjust
                // object on all pages
                sal_uInt16 nPageCount = static_cast<SdDrawDocument&>(getSdrModelFromSdrPage())
                                            .GetSdPageCount(mePageKind);

                for (sal_uInt16 i = 0; i < nPageCount; i++)
                {
                    SdPage* pLoopPage = static_cast<SdDrawDocument&>(getSdrModelFromSdrPage())
                                            .GetSdPage(i, mePageKind);

                    if (pLoopPage && this == &(pLoopPage->TRG_GetMasterPage()))
                    {
                        // Page listens to this master page, therefore
                        // adjust AutoLayout
                        pLoopPage->SetAutoLayout(pLoopPage->GetAutoLayout());
                    }
                }
            }
        }
        break;

        case SdrUserCallType::Delete:
        case SdrUserCallType::Removed:
        default:
            break;
    }
}

/*************************************************************************
|*
|* Creates on a master page: background, title- and layout area
|*
\************************************************************************/

void SdPage::CreateTitleAndLayout(bool bInit, bool bCreate )
{
    SfxUndoManager* pUndoManager(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetUndoManager());
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    SdPage* pMasterPage = this;

    if (!mbMaster)
    {
        pMasterPage = static_cast<SdPage*>(&(TRG_GetMasterPage()));
    }

    if (!pMasterPage)
    {
        return;
    }

    /**************************************************************************
    * create background, title- and layout area
    **************************************************************************/
    if( mePageKind == PageKind::Standard )
    {
        pMasterPage->EnsureMasterPageDefaultBackground();
    }

    if (static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetDocumentType() != DocumentType::Impress)
        return;

    if( mePageKind == PageKind::Handout && bInit )
    {
        // handout template

        // delete all available handout presentation objects
        SdrObject *pObj=nullptr;
        while( (pObj = pMasterPage->GetPresObj(PresObjKind::Handout)) != nullptr )
        {
            pMasterPage->RemoveObject(pObj->GetOrdNum());

            if( bUndo )
            {
                pUndoManager->AddUndoAction(getSdrModelFromSdrPage().GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));
            }
            else
            {
                SdrObject::Free( pObj );
            }
        }

        std::vector< ::tools::Rectangle > aAreas;
        CalculateHandoutAreas( static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()), pMasterPage->GetAutoLayout(), false, aAreas );

        const bool bSkip = pMasterPage->GetAutoLayout() == AUTOLAYOUT_HANDOUT3;
        std::vector< ::tools::Rectangle >::iterator iter( aAreas.begin() );

        while( iter != aAreas.end() )
        {
            SdrPageObj* pPageObj = static_cast<SdrPageObj*>(pMasterPage->CreatePresObj(PresObjKind::Handout, false, (*iter++)) );
            // #i105146# We want no content to be displayed for PageKind::Handout,
            // so just never set a page as content
            pPageObj->SetReferencedPage(nullptr);

            if( bSkip && iter != aAreas.end() )
                ++iter;
        }
    }

    if( mePageKind != PageKind::Handout )
    {
        SdrObject* pMasterTitle = pMasterPage->GetPresObj( PresObjKind::Title );
        if( pMasterTitle == nullptr )
            pMasterPage->CreateDefaultPresObj(PresObjKind::Title);

        SdrObject* pMasterOutline = pMasterPage->GetPresObj( mePageKind==PageKind::Notes ? PresObjKind::Notes : PresObjKind::Outline );
        if( pMasterOutline == nullptr )
            pMasterPage->CreateDefaultPresObj( mePageKind == PageKind::Standard ? PresObjKind::Outline : PresObjKind::Notes );
    }

    // create header&footer objects

    if( !bCreate )
        return;

    if( mePageKind != PageKind::Standard )
    {
        SdrObject* pHeader = pMasterPage->GetPresObj( PresObjKind::Header );
        if( pHeader == nullptr )
            pMasterPage->CreateDefaultPresObj( PresObjKind::Header );
    }

    SdrObject* pDate   = pMasterPage->GetPresObj( PresObjKind::DateTime );
    if( pDate == nullptr )
        pMasterPage->CreateDefaultPresObj( PresObjKind::DateTime );

    SdrObject* pFooter = pMasterPage->GetPresObj( PresObjKind::Footer );
    if( pFooter == nullptr )
        pMasterPage->CreateDefaultPresObj( PresObjKind::Footer );

    SdrObject* pNumber = pMasterPage->GetPresObj( PresObjKind::SlideNumber );
    if( pNumber == nullptr )
        pMasterPage->CreateDefaultPresObj( PresObjKind::SlideNumber );
}

namespace {

const o3tl::enumarray<PageKind, char const *> PageKindVector = {
    "PageKind::Standard", "PageKind::Notes", "PageKind::Handout"
};

const o3tl::enumarray<PresObjKind, const char*> PresObjKindVector = {
    "PRESOBJ_NONE", "PRESOBJ_TITLE", "PRESOBJ_OUTLINE",
    "PRESOBJ_TEXT" ,"PRESOBJ_GRAPHIC" , "PRESOBJ_OBJECT",
    "PRESOBJ_CHART", "PRESOBJ_ORGCHART", "PRESOBJ_TABLE",
    "PRESOBJ_PAGE", "PRESOBJ_HANDOUT",
    "PRESOBJ_NOTES","PRESOBJ_HEADER", "PRESOBJ_FOOTER",
    "PRESOBJ_DATETIME", "PRESOBJ_SLIDENUMBER", "PRESOBJ_CALC",
    "PRESOBJ_MEDIA"
};

void getPresObjProp( const SdPage& rPage, const char* sObjKind, const char* sPageKind, double presObjPropValue[] )
{
    bool bNoObjectFound = true;  //used to break from outer loop

    const std::vector< Reference<XNode> >& objectInfo = static_cast< const SdDrawDocument& >(rPage.getSdrModelFromSdrPage()).GetObjectVector();
    for( const Reference<XNode>& objectNode : objectInfo )
    {
        if(bNoObjectFound)
        {
            Reference<XNamedNodeMap> objectattrlist = objectNode->getAttributes();
            Reference<XNode> objectattr = objectattrlist->getNamedItem("type");
            OUString sObjType = objectattr->getNodeValue();

            if (sObjType.equalsAscii(sObjKind))
            {
                Reference<XNodeList> objectChildren = objectNode->getChildNodes();
                const int objSize = objectChildren->getLength();

                for( int j=0; j< objSize; j++)
                {
                    Reference<XNode> obj = objectChildren->item(j);
                    OUString nodename = obj->getNodeName();

                    //check whether child is blank 'text-node' or 'object-prop' node
                    if(nodename == "object-prop")
                    {
                        Reference<XNamedNodeMap> ObjAttributes = obj->getAttributes();
                        Reference<XNode> ObjPageKind = ObjAttributes->getNamedItem("pagekind");
                        OUString sObjPageKind = ObjPageKind->getNodeValue();

                        if (sObjPageKind.equalsAscii(sPageKind))
                        {
                            Reference<XNode> ObjSizeHeight = ObjAttributes->getNamedItem("relative-height");
                            OUString sValue = ObjSizeHeight->getNodeValue();
                            presObjPropValue[0] = sValue.toDouble();

                            Reference<XNode> ObjSizeWidth = ObjAttributes->getNamedItem("relative-width");
                            sValue = ObjSizeWidth->getNodeValue();
                            presObjPropValue[1] = sValue.toDouble();

                            Reference<XNode> ObjPosX = ObjAttributes->getNamedItem("relative-posX");
                            sValue = ObjPosX->getNodeValue();
                            presObjPropValue[2] = sValue.toDouble();

                            Reference<XNode> ObjPosY = ObjAttributes->getNamedItem("relative-posY");
                            sValue = ObjPosY->getNodeValue();
                            presObjPropValue[3] = sValue.toDouble();

                            bNoObjectFound = false;
                            break;
                        }
                    }
                }
            }
        }
        else
            break;
    }
}

}

SdrObject* SdPage::CreateDefaultPresObj(PresObjKind eObjKind)
{
    if( eObjKind == PresObjKind::Title )
    {
        ::tools::Rectangle aTitleRect( GetTitleRect() );
        return CreatePresObj(PresObjKind::Title, false, aTitleRect);
    }
    else if( eObjKind == PresObjKind::Outline )
    {
        ::tools::Rectangle aLayoutRect( GetLayoutRect() );
        return CreatePresObj( PresObjKind::Outline, false, aLayoutRect);
    }
    else if( eObjKind == PresObjKind::Notes )
    {
        ::tools::Rectangle aLayoutRect( GetLayoutRect() );
        return CreatePresObj( PresObjKind::Notes, false, aLayoutRect);
    }
    else if( (eObjKind == PresObjKind::Footer) || (eObjKind == PresObjKind::DateTime) || (eObjKind == PresObjKind::SlideNumber) || (eObjKind == PresObjKind::Header ) )
    {
        double propvalue[] = {0,0,0,0};
        const char* sObjKind = PresObjKindVector[eObjKind];
        const char* sPageKind = PageKindVector[mePageKind];
        // create footer objects for standard master page
        if( mePageKind == PageKind::Standard )
        {
            const ::tools::Long nLftBorder = GetLeftBorder();
            const ::tools::Long nUppBorder = GetUpperBorder();

            Point aPos ( nLftBorder, nUppBorder );
            Size aSize ( GetSize() );

            aSize.AdjustWidth( -(nLftBorder + GetRightBorder()) );
            aSize.AdjustHeight( -(nUppBorder + GetLowerBorder()) );

            getPresObjProp( *this, sObjKind, sPageKind, propvalue);
            aPos.AdjustX(::tools::Long( aSize.Width() * propvalue[2] ) );
            aPos.AdjustY(::tools::Long( aSize.Height() * propvalue[3] ) );
            aSize.setWidth( ::tools::Long( aSize.Width() * propvalue[1] ) );
            aSize.setHeight( ::tools::Long( aSize.Height() * propvalue[0] ) );

            if(eObjKind == PresObjKind::Header )
            {
                OSL_FAIL( "SdPage::CreateDefaultPresObj() - can't create a header placeholder for a master slide" );
                return nullptr;
            }
            else
            {
                ::tools::Rectangle aRect( aPos, aSize );
                return CreatePresObj( eObjKind, false, aRect );
            }
        }
        else
        {
            // create header&footer objects for handout and notes master
            Size aPageSize ( GetSize() );
            aPageSize.AdjustWidth( -(GetLeftBorder() + GetRightBorder()) );
            aPageSize.AdjustHeight( -(GetUpperBorder() + GetLowerBorder()) );

            Point aPosition ( GetLeftBorder(), GetUpperBorder() );

            getPresObjProp( *this, sObjKind, sPageKind, propvalue);
            int NOTES_HEADER_FOOTER_WIDTH = ::tools::Long(aPageSize.Width() * propvalue[1]);
            int NOTES_HEADER_FOOTER_HEIGHT = ::tools::Long(aPageSize.Height() * propvalue[0]);
            Size aSize( NOTES_HEADER_FOOTER_WIDTH, NOTES_HEADER_FOOTER_HEIGHT );
            Point aPos ( 0 ,0 );
            if( propvalue[2] == 0 )
                aPos.setX( aPosition.X() );
            else
                aPos.setX( aPosition.X() + ::tools::Long( aPageSize.Width() - NOTES_HEADER_FOOTER_WIDTH ) );
            if( propvalue[3] == 0 )
                aPos.setY( aPosition.Y() );
            else
                aPos.setY( aPosition.Y() + ::tools::Long( aPageSize.Height() - NOTES_HEADER_FOOTER_HEIGHT ) );

            ::tools::Rectangle aRect( aPos, aSize );
            return CreatePresObj( eObjKind, false, aRect );
        }
    }
    else
    {
        OSL_FAIL("SdPage::CreateDefaultPresObj() - unknown PRESOBJ kind" );
        return nullptr;
    }
}

void SdPage::DestroyDefaultPresObj(PresObjKind eObjKind)
{
    SdrObject* pObject = GetPresObj( eObjKind );

    if( pObject )
    {
        SdDrawDocument* pDoc(static_cast< SdDrawDocument* >(&getSdrModelFromSdrPage()));
        const bool bUndo = pDoc->IsUndoEnabled();
        if( bUndo )
            pDoc->AddUndo(pDoc->GetSdrUndoFactory().CreateUndoDeleteObject(*pObject));
        SdrObjList* pOL = pObject->getParentSdrObjListFromSdrObject();
        pOL->RemoveObject(pObject->GetOrdNumDirect());

        if( !bUndo )
            SdrObject::Free(pObject);
    }
}

/*************************************************************************
|*
|* return title area
|*
\************************************************************************/

::tools::Rectangle SdPage::GetTitleRect() const
{
    ::tools::Rectangle aTitleRect;

    if (mePageKind != PageKind::Handout)
    {
        double propvalue[] = {0,0,0,0};

        /******************************************************************
        * standard- or note page: title area
        ******************************************************************/
        Point aTitlePos ( GetLeftBorder(), GetUpperBorder() );
        Size aTitleSize ( GetSize() );
        aTitleSize.AdjustWidth( -(GetLeftBorder() + GetRightBorder()) );
        aTitleSize.AdjustHeight( -(GetUpperBorder() + GetLowerBorder()) );
        const char* sPageKind = PageKindVector[mePageKind];

        if (mePageKind == PageKind::Standard)
         {
            getPresObjProp( *this , "PRESOBJ_TITLE" ,sPageKind, propvalue);
            aTitlePos.AdjustX(::tools::Long( aTitleSize.Width() * propvalue[2] ) );
            aTitlePos.AdjustY(::tools::Long( aTitleSize.Height() * propvalue[3] ) );
            aTitleSize.setWidth( ::tools::Long( aTitleSize.Width() * propvalue[1] ) );
            aTitleSize.setHeight( ::tools::Long( aTitleSize.Height() * propvalue[0] ) );
        }
        else if (mePageKind == PageKind::Notes)
        {
            Point aPos = aTitlePos;
            getPresObjProp( *this, "PRESOBJ_TITLE" ,sPageKind, propvalue);
            aPos.AdjustX(::tools::Long( aTitleSize.Width() * propvalue[2] ) );
            aPos.AdjustY(::tools::Long( aTitleSize.Height() * propvalue[3] ) );

            // limit height
            aTitleSize.setHeight( ::tools::Long( aTitleSize.Height() * propvalue[0] ) );
            aTitleSize.setWidth( ::tools::Long( aTitleSize.Width() * propvalue[1] ) );

            Size aPartArea = aTitleSize;
            Size aSize;
            sal_uInt16 nDestPageNum(GetPageNum());
            SdrPage* pRefPage = nullptr;

            if(nDestPageNum)
            {
                // only decrement if != 0, else we get 0xffff
                nDestPageNum -= 1;
            }

            if(nDestPageNum < getSdrModelFromSdrPage().GetPageCount())
            {
                pRefPage = getSdrModelFromSdrPage().GetPage(nDestPageNum);
            }

            if ( pRefPage )
            {
                // scale actually page size into handout rectangle
                double fH = pRefPage->GetWidth() == 0
                    ? 0 : static_cast<double>(aPartArea.Width())  / pRefPage->GetWidth();
                double fV = pRefPage->GetHeight() == 0
                    ? 0 : static_cast<double>(aPartArea.Height()) / pRefPage->GetHeight();

                if ( fH > fV )
                    fH = fV;
                aSize.setWidth( static_cast<::tools::Long>(fH * pRefPage->GetWidth()) );
                aSize.setHeight( static_cast<::tools::Long>(fH * pRefPage->GetHeight()) );

                aPos.AdjustX((aPartArea.Width() - aSize.Width()) / 2 );
                aPos.AdjustY((aPartArea.Height()- aSize.Height())/ 2 );
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

::tools::Rectangle SdPage::GetLayoutRect() const
{
    ::tools::Rectangle aLayoutRect;

    if (mePageKind != PageKind::Handout)
    {
        double propvalue[] = {0,0,0,0};

        Point aLayoutPos ( GetLeftBorder(), GetUpperBorder() );
        Size aLayoutSize ( GetSize() );
        aLayoutSize.AdjustWidth( -(GetLeftBorder() + GetRightBorder()) );
        aLayoutSize.AdjustHeight( -(GetUpperBorder() + GetLowerBorder()) );
        const char* sPageKind = PageKindVector[mePageKind];

        if (mePageKind == PageKind::Standard)
        {
            getPresObjProp( *this ,"PRESOBJ_OUTLINE", sPageKind, propvalue);
            aLayoutPos.AdjustX(::tools::Long( aLayoutSize.Width() * propvalue[2] ) );
            aLayoutPos.AdjustY(::tools::Long( aLayoutSize.Height() * propvalue[3] ) );
            aLayoutSize.setWidth( ::tools::Long( aLayoutSize.Width() * propvalue[1] ) );
            aLayoutSize.setHeight( ::tools::Long( aLayoutSize.Height() * propvalue[0] ) );
            aLayoutRect.SetPos(aLayoutPos);
            aLayoutRect.SetSize(aLayoutSize);
        }
        else if (mePageKind == PageKind::Notes)
        {
            getPresObjProp( *this, "PRESOBJ_NOTES", sPageKind, propvalue);
            aLayoutPos.AdjustX(::tools::Long( aLayoutSize.Width() * propvalue[2] ) );
            aLayoutPos.AdjustY(::tools::Long( aLayoutSize.Height() * propvalue[3] ) );
            aLayoutSize.setWidth( ::tools::Long( aLayoutSize.Width() * propvalue[1] ) );
            aLayoutSize.setHeight( ::tools::Long( aLayoutSize.Height() * propvalue[0] ) );
            aLayoutRect.SetPos(aLayoutPos);
            aLayoutRect.SetSize(aLayoutSize);
        }
    }

    return aLayoutRect;
}

/**************************************************************************
|*
|* assign an AutoLayout
|*
\*************************************************************************/

const int MAX_PRESOBJS = 7; // maximum number of presentation objects per layout
const int VERTICAL = 0x8000;

static constexpr PresObjKind operator|(PresObjKind e, int x)
{
    return static_cast<PresObjKind>(static_cast<int>(e) | x);
}

namespace {

struct LayoutDescriptor
{
    PresObjKind meKind[MAX_PRESOBJS];
    bool mbVertical[MAX_PRESOBJS];

    LayoutDescriptor( PresObjKind k0 = PresObjKind::NONE, PresObjKind k1 = PresObjKind::NONE, PresObjKind k2 = PresObjKind::NONE, PresObjKind k3 = PresObjKind::NONE, PresObjKind k4 = PresObjKind::NONE, PresObjKind k5 = PresObjKind::NONE, PresObjKind k6 = PresObjKind::NONE );
};

}

LayoutDescriptor::LayoutDescriptor( PresObjKind k0, PresObjKind k1, PresObjKind k2, PresObjKind k3, PresObjKind k4, PresObjKind k5, PresObjKind k6 )
{
    auto removeVertical = [&] (PresObjKind k) { return static_cast<PresObjKind>(static_cast<int>(k) & ~VERTICAL); };
    auto isVertical = [&] (PresObjKind k) { return bool(static_cast<int>(k) & VERTICAL); };
    meKind[0] = removeVertical(k0); mbVertical[0] = isVertical(k0);
    meKind[1] = removeVertical(k1); mbVertical[1] = isVertical(k1);
    meKind[2] = removeVertical(k2); mbVertical[2] = isVertical(k2);
    meKind[3] = removeVertical(k3); mbVertical[3] = isVertical(k3);
    meKind[4] = removeVertical(k4); mbVertical[4] = isVertical(k4);
    meKind[5] = removeVertical(k5); mbVertical[5] = isVertical(k5);
    meKind[6] = removeVertical(k6); mbVertical[6] = isVertical(k6);
}

static const LayoutDescriptor& GetLayoutDescriptor( AutoLayout eLayout )
{
    static const LayoutDescriptor aLayouts[AUTOLAYOUT_END-AUTOLAYOUT_START] =
    {
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Text ),                                 // AUTOLAYOUT_TITLE
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline ),                              // AUTOLAYOUT_TITLE_CONTENT
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline ),                              // AUTOLAYOUT_CHART
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline ),             // AUTOLAYOUT_TITLE_2CONTENT
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline ),             // AUTOLAYOUT_TEXTCHART
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline ),                              // AUTOLAYOUT_ORG
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline ),             // AUTOLAYOUT_TEXTCLbIP
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline ),             // AUTOLAYOUT_CHARTTEXT
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline ),                              // AUTOLAYOUT_TAB
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline ),             // AUTOLAYOUT_CLIPTEXT
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline ),             // AUTOLAYOUT_TEXTOBJ
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Object ),                               // AUTOLAYOUT_OBJ
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline, PresObjKind::Outline ),    // AUTOLAYOUT_TITLE_CONTENT_2CONTENT
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline ),             // AUTOLAYOUT_TEXTOBJ
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline ),             // AUTOLAYOUT_TITLE_CONTENT_OVER_CONTENT
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline, PresObjKind::Outline ),    // AUTOLAYOUT_TITLE_2CONTENT_CONTENT
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline, PresObjKind::Outline ),    // AUTOLAYOUT_TITLE_2CONTENT_OVER_CONTENT
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline ),             // AUTOLAYOUT_TEXTOVEROBJ
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline,                   // AUTOLAYOUT_TITLE_4CONTENT
            PresObjKind::Outline, PresObjKind::Outline ),
        LayoutDescriptor( PresObjKind::Title, PresObjKind::NONE ),                                 // AUTOLAYOUT_TITLE_ONLY
        LayoutDescriptor( PresObjKind::NONE ),                                                // AUTOLAYOUT_NONE
        LayoutDescriptor( PresObjKind::Page, PresObjKind::Notes ),                                 // AUTOLAYOUT_NOTES
        LayoutDescriptor( ),                                                              // AUTOLAYOUT_HANDOUT1
        LayoutDescriptor( ),                                                              // AUTOLAYOUT_HANDOUT2
        LayoutDescriptor( ),                                                              // AUTOLAYOUT_HANDOUT3
        LayoutDescriptor( ),                                                              // AUTOLAYOUT_HANDOUT4
        LayoutDescriptor( ),                                                              // AUTOLAYOUT_HANDOUT6
        LayoutDescriptor( PresObjKind::Title|VERTICAL, PresObjKind::Outline|VERTICAL, PresObjKind::Outline ),// AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT
        LayoutDescriptor( PresObjKind::Title|VERTICAL, PresObjKind::Outline|VERTICAL ),            // AUTOLAYOUT_VTITLE_VCONTENT
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline|VERTICAL ),                     // AUTOLAYOUT_TITLE_VCONTENT
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline|VERTICAL, PresObjKind::Outline|VERTICAL ),   // AUTOLAYOUT_TITLE_2VTEXT
        LayoutDescriptor( ),                                                              // AUTOLAYOUT_HANDOUT9
        LayoutDescriptor( PresObjKind::Text, PresObjKind::NONE ),                                 // AUTOLAYOUT_ONLY_TEXT
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline,               // AUTOLAYOUT_4CLIPART
            PresObjKind::Graphic, PresObjKind::Graphic ),
        LayoutDescriptor( PresObjKind::Title, PresObjKind::Outline, PresObjKind::Outline,              // AUTOLAYOUT_TITLE_6CONTENT
            PresObjKind::Outline, PresObjKind::Outline, PresObjKind::Outline, PresObjKind::Outline )
    };

    if( (eLayout < AUTOLAYOUT_START) || (eLayout >= AUTOLAYOUT_END) )
        eLayout = AUTOLAYOUT_NONE;

    return aLayouts[ eLayout - AUTOLAYOUT_START ];
}

static OUString enumtoString(AutoLayout aut)
{
    OUString retstr;
    switch (aut)
    {
        case AUTOLAYOUT_TITLE_CONTENT:
            retstr="AUTOLAYOUT_TITLE_CONTENT";
            break;
        case AUTOLAYOUT_TITLE_CONTENT_OVER_CONTENT:
            retstr="AUTOLAYOUT_TITLE_CONTENT_OVER_CONTENT";
            break;
        case AUTOLAYOUT_TITLE_CONTENT_2CONTENT:
            retstr="AUTOLAYOUT_TITLE_CONTENT_2CONTENT";
            break;
        case AUTOLAYOUT_TITLE_4CONTENT:
            retstr="AUTOLAYOUT_TITLE_4CONTENT";
            break;
        case AUTOLAYOUT_ONLY_TEXT:
            retstr="AUTOLAYOUT_ONLY_TEXT";
            break;
        case AUTOLAYOUT_TITLE_ONLY:
            retstr="AUTOLAYOUT_TITLE_ONLY";
            break;
        case AUTOLAYOUT_TITLE_6CONTENT:
            retstr="AUTOLAYOUT_TITLE_6CONTENT";
            break;
        case AUTOLAYOUT_START:
            retstr="AUTOLAYOUT_START";
            break;
        case AUTOLAYOUT_TITLE_2CONTENT_CONTENT:
            retstr="AUTOLAYOUT_TITLE_2CONTENT_CONTENT";
            break;
        case AUTOLAYOUT_TITLE_2CONTENT_OVER_CONTENT:
            retstr="AUTOLAYOUT_TITLE_2CONTENT_OVER_CONTENT";
            break;
        case AUTOLAYOUT_TITLE_2CONTENT:
            retstr="AUTOLAYOUT_TITLE_2CONTENT";
            break;
        case AUTOLAYOUT_VTITLE_VCONTENT:
            retstr="AUTOLAYOUT_VTITLE_VCONTENT";
            break;
        case AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT:
            retstr="AUTOLAYOUT_VTITLE_VCONTENT_OVER_VCONTENT";
            break;
        case AUTOLAYOUT_TITLE_VCONTENT:
            retstr="AUTOLAYOUT_TITLE_VCONTENT";
            break;
        case AUTOLAYOUT_TITLE_2VTEXT:
            retstr="AUTOLAYOUT_TITLE_2VTEXT";
            break;
        default:
            retstr="unknown";
            break;
        // case AUTOLAYOUT_TITLE_4SCONTENT:            return "AUTOLAYOUT_TITLE_4SCONTENT";
    }
    return retstr;
}

static void CalcAutoLayoutRectangles( SdPage const & rPage,::tools::Rectangle* rRectangle ,const OUString& sLayoutType )
{
    ::tools::Rectangle aTitleRect;
    ::tools::Rectangle aLayoutRect;

    if( rPage.GetPageKind() != PageKind::Handout )
    {
        SdPage& rMasterPage = static_cast<SdPage&>(rPage.TRG_GetMasterPage());
        SdrObject* pMasterTitle = rMasterPage.GetPresObj( PresObjKind::Title );
        SdrObject* pMasterSubTitle = rMasterPage.GetPresObj( PresObjKind::Text );
        SdrObject* pMasterOutline = rMasterPage.GetPresObj( rPage.GetPageKind()==PageKind::Notes ? PresObjKind::Notes : PresObjKind::Outline );

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
    for( int i = 1; i < MAX_PRESOBJS; i++ )
        rRectangle[i] = aLayoutRect;

    const Point aTitlePos( aTitleRect.TopLeft() );
    const Size  aLayoutSize( aLayoutRect.GetSize() );
    const Point aLayoutPos( aLayoutRect.TopLeft() );
    double propvalue[] = {0,0,0,0};

    const std::vector< Reference<XNode> >& layoutInfo = static_cast< const SdDrawDocument& >(rPage.getSdrModelFromSdrPage()).GetLayoutVector();
    auto aIter = std::find_if(layoutInfo.begin(), layoutInfo.end(),
        [&sLayoutType](const Reference<XNode>& layoutNode) {
            Reference<XNamedNodeMap> layoutAttrList = layoutNode->getAttributes();

            // get the attribute value of layout (i.e it's type)
            OUString sLayoutAttName = layoutAttrList->getNamedItem("type")->getNodeValue();
            return sLayoutAttName == sLayoutType;
        });
    if (aIter == layoutInfo.end())
        return;

    int count=0;
    Reference<XNode> layoutNode = *aIter;
    Reference<XNodeList> layoutChildren = layoutNode->getChildNodes();
    const int presobjsize = layoutChildren->getLength();
    for( int j=0; j< presobjsize ; j++)
    {
        OUString nodename;
        Reference<XNode> presobj = layoutChildren->item(j);
        nodename=presobj->getNodeName();

        //check whether child is blank 'text-node' or 'presobj' node
        if(nodename == "presobj")
        {
            // TODO: rework sd to permit arbitrary number of presentation objects
            assert(count < MAX_PRESOBJS);

            Reference<XNamedNodeMap> presObjAttributes = presobj->getAttributes();

            Reference<XNode> presObjSizeHeight = presObjAttributes->getNamedItem("relative-height");
            OUString sValue = presObjSizeHeight->getNodeValue();
            propvalue[0] = sValue.toDouble();

            Reference<XNode> presObjSizeWidth = presObjAttributes->getNamedItem("relative-width");
            sValue = presObjSizeWidth->getNodeValue();
            propvalue[1] = sValue.toDouble();

            Reference<XNode> presObjPosX = presObjAttributes->getNamedItem("relative-posX");
            sValue = presObjPosX->getNodeValue();
            propvalue[2] = sValue.toDouble();

            Reference<XNode> presObjPosY = presObjAttributes->getNamedItem("relative-posY");
            sValue = presObjPosY->getNodeValue();
            propvalue[3] = sValue.toDouble();

            if(count == 0)
            {
                Size aSize ( aTitleRect.GetSize() );
                aSize.setHeight( basegfx::fround(aSize.Height() * propvalue[0]) );
                aSize.setWidth( basegfx::fround(aSize.Width() * propvalue[1]) );
                Point aPos( basegfx::fround(aTitlePos.X() +(aSize.Width() * propvalue[2])),
                            basegfx::fround(aTitlePos.Y() + (aSize.Height() * propvalue[3])) );
                rRectangle[count] = ::tools::Rectangle(aPos, aSize);
                count = count+1;
            }
            else
            {
                Size aSize( basegfx::fround(aLayoutSize.Width() * propvalue[1]),
                            basegfx::fround(aLayoutSize.Height() * propvalue[0]) );
                Point aPos( basegfx::fround(aLayoutPos.X() +(aSize.Width() * propvalue[2])),
                            basegfx::fround(aLayoutPos.Y() + (aSize.Height() * propvalue[3])) );
                rRectangle[count] = ::tools::Rectangle (aPos, aSize);
                count = count+1;
            }
        }
    }
}

static void findAutoLayoutShapesImpl( SdPage& rPage, const LayoutDescriptor& rDescriptor, std::array<SdrObject*, MAX_PRESOBJS>& rShapes, bool bInit, bool bSwitchLayout )
{
    // init list of indexes for each presentation shape kind
    // this is used to find subsequent shapes with the same presentation shape kind
    o3tl::enumarray<PresObjKind,int> PresObjIndex;
    PresObjIndex.fill(1);

    bool bMissing = false;

    // for each entry in the layoutdescriptor, arrange a presentation shape
    for (int i = 0; (i < MAX_PRESOBJS) && (rDescriptor.meKind[i] != PresObjKind::NONE); i++)
    {
        PresObjKind eKind = rDescriptor.meKind[i];
        SdrObject* pObj = nullptr;
        while( (pObj = rPage.GetPresObj( eKind, PresObjIndex[eKind], true )) != nullptr )
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

    if( !(bMissing && bInit) )
        return;

    // for each entry in the layoutdescriptor, look for an alternative shape
    for (int i = 0; (i < MAX_PRESOBJS) && (rDescriptor.meKind[i] != PresObjKind::NONE); i++)
    {
        if( rShapes[i] )
            continue;

        PresObjKind eKind = rDescriptor.meKind[i];

        SdrObject* pObj = nullptr;
        bool bFound = false;

        const size_t nShapeCount = rPage.GetObjCount();
        for(size_t nShapeIndex = 0; nShapeIndex < nShapeCount && !bFound; ++nShapeIndex )
        {
            pObj = rPage.GetObj(nShapeIndex);

            if( pObj->IsEmptyPresObj() )
                continue;

            if( pObj->GetObjInventor() != SdrInventor::Default )
                continue;

            // do not reuse shapes that are already part of the layout
            if( std::find( rShapes.begin(), rShapes.end(), pObj ) != rShapes.end() )
                continue;

            bool bPresStyle = pObj->GetStyleSheet() && (pObj->GetStyleSheet()->GetFamily() == SfxStyleFamily::Page);
            SdrObjKind eSdrObjKind = pObj->GetObjIdentifier();

            switch( eKind )
            {
            case PresObjKind::Title:
                bFound = eSdrObjKind == OBJ_TITLETEXT;
                break;
            case PresObjKind::Table:
                bFound = eSdrObjKind == OBJ_TABLE;
                break;
            case PresObjKind::Media:
                bFound = eSdrObjKind == OBJ_MEDIA;
                break;
            case PresObjKind::Outline:
                bFound = (eSdrObjKind == OBJ_OUTLINETEXT) ||
                         ((eSdrObjKind == OBJ_TEXT) && bPresStyle) ||
                         (eSdrObjKind == OBJ_TABLE) || (eSdrObjKind == OBJ_MEDIA) || (eSdrObjKind == OBJ_GRAF) || (eSdrObjKind == OBJ_OLE2);
                break;
            case PresObjKind::Graphic:
                bFound = eSdrObjKind == OBJ_GRAF;
                break;
            case PresObjKind::Object:
                if( eSdrObjKind == OBJ_OLE2 )
                {
                    SdrOle2Obj* pOle2 = dynamic_cast< SdrOle2Obj* >( pObj );
                    if( pOle2 )
                    {
                        if( pOle2->IsEmpty() )
                            bFound = true;
                        else
                        {
                            ::comphelper::IEmbeddedHelper* pPersist(rPage.getSdrModelFromSdrPage().GetPersist());

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
            case PresObjKind::Chart:
            case PresObjKind::Calc:
                if( eSdrObjKind == OBJ_OLE2 )
                {
                    SdrOle2Obj* pOle2 = dynamic_cast< SdrOle2Obj* >( pObj );
                    if( pOle2 )
                    {
                        if(
                            ((eKind == PresObjKind::Chart) &&
                                ( pOle2->GetProgName() == "StarChart" || pOle2->IsChart() ) )
                            ||
                            ((eKind == PresObjKind::Calc) &&
                                ( pOle2->GetProgName() == "StarCalc" || pOle2->IsCalc() ) ) )
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
            case PresObjKind::Page:
            case PresObjKind::Handout:
                bFound = eSdrObjKind == OBJ_PAGE;
                break;
            case PresObjKind::Notes:
            case PresObjKind::Text:
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

void SdPage::SetAutoLayout(AutoLayout eLayout, bool bInit, bool bCreate )
{
    sd::ScopeLockGuard aGuard( maLockAutoLayoutArrangement );

    const bool bSwitchLayout = eLayout != GetAutoLayout();

    SfxUndoManager* pUndoManager(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetUndoManager());
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    meAutoLayout = eLayout;

    // if needed, creates and initialises the presentation shapes on this slides master page
    CreateTitleAndLayout(bInit, bCreate);

    if((meAutoLayout == AUTOLAYOUT_NONE && maPresentationShapeList.isEmpty()) || mbMaster)
    {
        // MasterPage or no layout and no presentation shapes available, nothing to do
        return;
    }

    ::tools::Rectangle aRectangle[MAX_PRESOBJS];
    const LayoutDescriptor& aDescriptor = GetLayoutDescriptor( meAutoLayout );
    OUString sLayoutName( enumtoString(meAutoLayout) );
    CalcAutoLayoutRectangles( *this, aRectangle, sLayoutName);

    o3tl::sorted_vector< SdrObject* > aUsedPresentationObjects;

    std::array<SdrObject*, MAX_PRESOBJS > aLayoutShapes;
    aLayoutShapes.fill(nullptr);
    findAutoLayoutShapesImpl( *this, aDescriptor, aLayoutShapes, bInit, bSwitchLayout );

    // for each entry in the layoutdescriptor, arrange a presentation shape
    for (int i = 0; (i < MAX_PRESOBJS) && (aDescriptor.meKind[i] != PresObjKind::NONE); i++)
    {
        PresObjKind eKind = aDescriptor.meKind[i];
        SdrObject* pObj = InsertAutoLayoutShape( aLayoutShapes[i], eKind, aDescriptor.mbVertical[i], aRectangle[i], bInit );
        if( pObj )
            aUsedPresentationObjects.insert(pObj); // remember that we used this empty shape
    }

    // now delete all empty presentation objects that are no longer used by the new layout
    if( !bInit )
        return;

    SdrObject* pObj = nullptr;
    maPresentationShapeList.seekShape(0);

    while( (pObj = maPresentationShapeList.getNextShape()) )
    {
        if( aUsedPresentationObjects.count(pObj) == 0 )
        {

            if( pObj->IsEmptyPresObj() )
            {
                if( bUndo )
                    pUndoManager->AddUndoAction(getSdrModelFromSdrPage().GetSdrUndoFactory().CreateUndoDeleteObject(*pObj));

                RemoveObject( pObj->GetOrdNum() );

                if( !bUndo )
                    SdrObject::Free( pObj );
            }
/* #i108541# keep non empty pres obj as pres obj even if they are not part of the current layout */
        }
    }
}

/*************************************************************************
|*
|* insert object
|*
\************************************************************************/

void SdPage::NbcInsertObject(SdrObject* pObj, size_t nPos)
{
    FmFormPage::NbcInsertObject(pObj, nPos);

    static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).InsertObject(pObj);

    SdrLayerID nId = pObj->GetLayer();
    if( mbMaster )
    {
        if( nId == SdrLayerID(0) )
            pObj->NbcSetLayer( SdrLayerID(2) );     // wrong layer. corrected to BackgroundObj layer
    }
    else
    {
        if( nId == SdrLayerID(2) )
            pObj->NbcSetLayer( SdrLayerID(0) );     // wrong layer. corrected to layout layer
    }
}

/*************************************************************************
|*
|* remove object
|*
\************************************************************************/

SdrObject* SdPage::RemoveObject(size_t nObjNum)
{
    onRemoveObject(GetObj( nObjNum ));
    return FmFormPage::RemoveObject(nObjNum);
}

/*************************************************************************
|*
|* remove object without broadcast
|*
\************************************************************************/

SdrObject* SdPage::NbcRemoveObject(size_t nObjNum)
{
    onRemoveObject(GetObj( nObjNum ));
    return FmFormPage::NbcRemoveObject(nObjNum);
}

// Also override ReplaceObject methods to realize when
// objects are removed with this mechanism instead of RemoveObject
SdrObject* SdPage::ReplaceObject(SdrObject* pNewObj, size_t nObjNum)
{
    onRemoveObject(GetObj( nObjNum ));
    return FmFormPage::ReplaceObject(pNewObj, nObjNum);
}

// called after a shape is removed or replaced from this slide

void SdPage::onRemoveObject( SdrObject* pObject )
{
    if( pObject )
    {
        RemovePresObj(pObject);

        static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).RemoveObject(pObject);

        removeAnimations( pObject );
    }
}

void SdPage::SetSize(const Size& aSize)
{
    Size aOldSize = GetSize();

    if (aSize != aOldSize)
    {
        FmFormPage::SetSize(aSize);
    }
}

void SdPage::SetBorder(sal_Int32 nLft, sal_Int32 nUpp, sal_Int32 nRgt, sal_Int32 nLwr)
{
    if (nLft != GetLeftBorder() || nUpp != GetUpperBorder() ||
        nRgt != GetRightBorder() || nLwr != GetLowerBorder() )
    {
        FmFormPage::SetBorder(nLft, nUpp, nRgt, nLwr);
    }
}

void SdPage::SetLeftBorder(sal_Int32 nBorder)
{
    if (nBorder != GetLeftBorder() )
    {
        FmFormPage::SetLeftBorder(nBorder);
    }
}

void SdPage::SetRightBorder(sal_Int32 nBorder)
{
    if (nBorder != GetRightBorder() )
    {
        FmFormPage::SetRightBorder(nBorder);
    }
}

void SdPage::SetUpperBorder(sal_Int32 nBorder)
{
    if (nBorder != GetUpperBorder() )
    {
        FmFormPage::SetUpperBorder(nBorder);
    }
}

void SdPage::SetLowerBorder(sal_Int32 nBorder)
{
    if (nBorder != GetLowerBorder() )
    {
        FmFormPage::SetLowerBorder(nBorder);
    }
}

/*************************************************************************
|*
|* Sets BackgroundFullSize and then calls AdjustBackground
|*
\************************************************************************/

void SdPage::SetBackgroundFullSize( bool bIn )
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

void SdPage::ScaleObjects(const Size& rNewPageSize, const ::tools::Rectangle& rNewBorderRect, bool bScaleAllObj)
{
    sd::ScopeLockGuard aGuard( maLockAutoLayoutArrangement );

    mbScaleObjects = bScaleAllObj;
    SdrObject* pObj = nullptr;
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
        aNewPageSize.setWidth( GetWidth() );
    }
    if (aNewPageSize.Height() < 0)
    {
        aNewPageSize.setHeight( GetHeight() );
    }
    if (nLeft < 0)
    {
        nLeft = GetLeftBorder();
    }
    if (nRight < 0)
    {
        nRight = GetRightBorder();
    }
    if (nUpper < 0)
    {
        nUpper = GetUpperBorder();
    }
    if (nLower < 0)
    {
        nLower = GetLowerBorder();
    }

    Size aBackgroundSize(aNewPageSize);

    if (mbScaleObjects)
    {
        aBackgroundSize.AdjustWidth( -(nLeft  + nRight) );
        aBackgroundSize.AdjustHeight( -(nUpper + nLower) );
        aNewPageSize = aBackgroundSize;
    }

    ::tools::Long nOldWidth  = GetWidth() - GetLeftBorder() - GetRightBorder();
    ::tools::Long nOldHeight = GetHeight() - GetUpperBorder() - GetLowerBorder();

    Fraction aFractX(aNewPageSize.Width(), nOldWidth);
    Fraction aFractY(aNewPageSize.Height(), nOldHeight);

    const size_t nObjCnt = (mbScaleObjects ? GetObjCount() : 0);

    for (size_t nObj = 0; nObj < nObjCnt; ++nObj)
    {
        bool bIsPresObjOnMaster = false;

        // all Objects
        pObj = GetObj(nObj);

        if (mbMaster && IsPresObj(pObj))
        {
            // There is a presentation object on the master page
            bIsPresObjOnMaster = true;
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
                    SdrObjKind eObjKind = pObj->GetObjIdentifier();

                    if (bIsPresObjOnMaster)
                    {
                        /**********************************************************
                        * presentation template: adjust test height
                        **********************************************************/

                        if (pObj == GetPresObj(PresObjKind::Title, 0))
                        {
                            SfxStyleSheet* pTitleSheet = GetStyleSheetForPresObj(PresObjKind::Title);

                            if (pTitleSheet)
                            {
                                SfxItemSet& rSet = pTitleSheet->GetItemSet();

                                const SvxFontHeightItem& rOldHgt = rSet.Get(EE_CHAR_FONTHEIGHT);
                                sal_uLong nFontHeight = rOldHgt.GetHeight();
                                nFontHeight = ::tools::Long(nFontHeight * static_cast<double>(aFractY));
                                rSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT));

                                if( SfxItemState::DEFAULT == rSet.GetItemState( EE_CHAR_FONTHEIGHT_CJK ) )
                                {
                                    const SvxFontHeightItem& rOldHgt2 = rSet.Get(EE_CHAR_FONTHEIGHT_CJK);
                                    nFontHeight = rOldHgt2.GetHeight();
                                    nFontHeight = ::tools::Long(nFontHeight * static_cast<double>(aFractY));
                                    rSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CJK));
                                }

                                if( SfxItemState::DEFAULT == rSet.GetItemState( EE_CHAR_FONTHEIGHT_CTL ) )
                                {
                                    const SvxFontHeightItem& rOldHgt2 = rSet.Get(EE_CHAR_FONTHEIGHT_CTL);
                                    nFontHeight = rOldHgt2.GetHeight();
                                    nFontHeight = ::tools::Long(nFontHeight * static_cast<double>(aFractY));
                                    rSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CTL));
                                }

                                pTitleSheet->Broadcast(SfxHint(SfxHintId::DataChanged));
                            }
                        }
                        else if (pObj == GetPresObj(PresObjKind::Outline, 0))
                        {
                            OUString aName(GetLayoutName() + " ");

                            for (sal_Int32 i=1; i<=9; i++)
                            {
                                OUString sLayoutName( aName + OUString::number( i ) );
                                SfxStyleSheet* pOutlineSheet = static_cast<SfxStyleSheet*>(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetStyleSheetPool()->Find(sLayoutName, SfxStyleFamily::Page));

                                if (pOutlineSheet)
                                {
                                    // Calculate new font height
                                    SfxItemSet aTempSet(pOutlineSheet->GetItemSet());

                                    const SvxFontHeightItem& rOldHgt = aTempSet.Get(EE_CHAR_FONTHEIGHT);
                                    sal_uLong nFontHeight = rOldHgt.GetHeight();
                                    nFontHeight = ::tools::Long(nFontHeight * static_cast<double>(aFractY));
                                    aTempSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT));

                                    if( SfxItemState::DEFAULT == aTempSet.GetItemState( EE_CHAR_FONTHEIGHT_CJK ) )
                                    {
                                        const SvxFontHeightItem& rOldHgt2 = aTempSet.Get(EE_CHAR_FONTHEIGHT_CJK);
                                        nFontHeight = rOldHgt2.GetHeight();
                                        nFontHeight = ::tools::Long(nFontHeight * static_cast<double>(aFractY));
                                        aTempSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CJK));
                                    }

                                    if( SfxItemState::DEFAULT == aTempSet.GetItemState( EE_CHAR_FONTHEIGHT_CTL ) )
                                    {
                                        const SvxFontHeightItem& rOldHgt2 = aTempSet.Get(EE_CHAR_FONTHEIGHT_CTL);
                                        nFontHeight = rOldHgt2.GetHeight();
                                        nFontHeight = ::tools::Long(nFontHeight * static_cast<double>(aFractY));
                                        aTempSet.Put(SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CTL));
                                    }

                                    // adjust bullet
                                    static_cast<SdStyleSheet*>(pOutlineSheet)->AdjustToFontHeight(aTempSet, false);

                                    // Special treatment: reset the INVALIDS to
                                    // NULL pointer (otherwise we have INVALID's
                                    // or pointer to the DefaultItems in the
                                    // template; both would suppress the
                                    // attribute inheritance)
                                    aTempSet.ClearInvalidItems();

                                    // Special treatment: only the valid parts
                                    // of the BulletItems
                                    if (aTempSet.GetItemState(EE_PARA_BULLET) == SfxItemState::DEFAULT)
                                    {
                                        SvxBulletItem aOldBulItem( pOutlineSheet->GetItemSet().Get(EE_PARA_BULLET) );
                                        const SvxBulletItem& rNewBulItem = aTempSet.Get(EE_PARA_BULLET);
                                        aOldBulItem.CopyValidProperties(rNewBulItem);
                                        aTempSet.Put(aOldBulItem);
                                    }

                                    pOutlineSheet->GetItemSet().Put(aTempSet);
                                    pOutlineSheet->Broadcast(SfxHint(SfxHintId::DataChanged));
                                }
                            }
                        }
                        else if (pObj == GetPresObj(PresObjKind::Notes, 0))
                        {
                            SfxStyleSheet* pNotesSheet = GetStyleSheetForPresObj(PresObjKind::Notes);

                            if (pNotesSheet)
                            {
                                sal_uLong nHeight = pObj->GetLogicRect().GetSize().Height();
                                sal_uLong nFontHeight = static_cast<sal_uLong>(nHeight * 0.0741);
                                SfxItemSet& rSet = pNotesSheet->GetItemSet();
                                rSet.Put( SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT ));
                                rSet.Put( SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CJK ));
                                rSet.Put( SvxFontHeightItem(nFontHeight, 100, EE_CHAR_FONTHEIGHT_CTL ));
                                pNotesSheet->Broadcast(SfxHint(SfxHintId::DataChanged));
                            }
                        }
                    }
                    else if ( eObjKind != OBJ_TITLETEXT   &&
                              eObjKind != OBJ_OUTLINETEXT &&
                              dynamic_cast< const SdrTextObj *>( pObj ) !=  nullptr       &&
                              pObj->GetOutlinerParaObject() )
                    {
                        /******************************************************
                        * normal text object: adjust text height
                        ******************************************************/
                        SvtScriptType nScriptType = pObj->GetOutlinerParaObject()->GetTextObject().GetScriptType();
                        sal_uInt16 nWhich = EE_CHAR_FONTHEIGHT;
                        if ( nScriptType == SvtScriptType::ASIAN )
                            nWhich = EE_CHAR_FONTHEIGHT_CJK;
                        else if ( nScriptType == SvtScriptType::COMPLEX )
                            nWhich = EE_CHAR_FONTHEIGHT_CTL;

                        // use more modern method to scale the text height
                        sal_uInt32 nFontHeight = static_cast<const SvxFontHeightItem&>(pObj->GetMergedItem(nWhich)).GetHeight();
                        sal_uInt32 nNewFontHeight = sal_uInt32(static_cast<double>(nFontHeight) * static_cast<double>(aFractY));

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
                aNewPos.setX( ::tools::Long((aTopLeft.X() - GetLeftBorder()) * static_cast<double>(aFractX)) + nLeft );
                aNewPos.setY( ::tools::Long((aTopLeft.Y() - GetUpperBorder()) * static_cast<double>(aFractY)) + nUpper );

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

static SdrObject* convertPresentationObjectImpl(SdPage& rPage, SdrObject* pSourceObj, PresObjKind& eObjKind, bool bVertical, const ::tools::Rectangle& rRect)
{
    SdDrawDocument& rModel(static_cast< SdDrawDocument& >(rPage.getSdrModelFromSdrPage()));
    if( !pSourceObj )
        return pSourceObj;

    SfxUndoManager* pUndoManager = rModel.GetUndoManager();
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && rPage.IsInserted();

    SdrObject* pNewObj = pSourceObj;
    if((eObjKind == PresObjKind::Outline) && (pSourceObj->GetObjIdentifier() == OBJ_TEXT) )
    {
        pNewObj = rPage.CreatePresObj(PresObjKind::Outline, bVertical, rRect);

        // Set text of the subtitle into PRESOBJ_OUTLINE
        OutlinerParaObject* pOutlParaObj = pSourceObj->GetOutlinerParaObject();

        if(pOutlParaObj)
        {
            // assign text
            SdOutliner* pOutl = rModel.GetInternalOutliner();
            pOutl->Clear();
            pOutl->SetText( *pOutlParaObj );
            std::unique_ptr<OutlinerParaObject> pNew = pOutl->CreateParaObject();
            pOutlParaObj = pNew.get();
            pNewObj->SetOutlinerParaObject( std::move(pNew) );
            pOutl->Clear();
            pNewObj->SetEmptyPresObj(false);

            for (sal_uInt16 nLevel = 1; nLevel < 10; nLevel++)
            {
                // assign new template
                OUString aName( rPage.GetLayoutName() + " " + OUString::number( nLevel ) );
                SfxStyleSheet* pSheet = static_cast<SfxStyleSheet*>( rModel.GetStyleSheetPool()->Find(aName, SfxStyleFamily::Page) );

                if (pSheet && nLevel == 1)
                {
                    SfxStyleSheet* pSubtitleSheet = rPage.GetStyleSheetForPresObj(PresObjKind::Text);

                    if (pSubtitleSheet)
                        pOutlParaObj->ChangeStyleSheetName(SfxStyleFamily::Page, pSubtitleSheet->GetName(), pSheet->GetName());
                }
            }

            // Remove LRSpace item
            SfxItemSet aSet(rModel.GetPool(), svl::Items<EE_PARA_LRSPACE, EE_PARA_LRSPACE>{} );

            aSet.Put(pNewObj->GetMergedItemSet());

            aSet.ClearItem(EE_PARA_LRSPACE);

            pNewObj->SetMergedItemSet(aSet);

            if( bUndo )
                pUndoManager->AddUndoAction( rModel.GetSdrUndoFactory().CreateUndoDeleteObject(*pSourceObj) );

            // Remove outline shape from page
            rPage.RemoveObject( pSourceObj->GetOrdNum() );

            if( !bUndo )
                SdrObject::Free( pSourceObj );
        }
    }
    else if((eObjKind == PresObjKind::Text) && (pSourceObj->GetObjIdentifier() == OBJ_OUTLINETEXT) )
    {
        // is there an outline shape we can use to replace empty subtitle shape?
        pNewObj = rPage.CreatePresObj(PresObjKind::Text, bVertical, rRect);

        // Set text of the outline object into PRESOBJ_TITLE
        OutlinerParaObject* pOutlParaObj = pSourceObj->GetOutlinerParaObject();

        if(pOutlParaObj)
        {
            // assign text
            SdOutliner* pOutl = rModel.GetInternalOutliner();
            pOutl->Clear();
            pOutl->SetText( *pOutlParaObj );
            pNewObj->SetOutlinerParaObject( pOutl->CreateParaObject() );
            pOutl->Clear();
            pNewObj->SetEmptyPresObj(false);

            // reset left indent
            SfxItemSet aSet(rModel.GetPool(), svl::Items<EE_PARA_LRSPACE, EE_PARA_LRSPACE>{} );

            aSet.Put(pNewObj->GetMergedItemSet());

            const SvxLRSpaceItem& rLRItem = aSet.Get(EE_PARA_LRSPACE);
            SvxLRSpaceItem aNewLRItem(rLRItem);
            aNewLRItem.SetTextLeft(0);
            aSet.Put(aNewLRItem);

            pNewObj->SetMergedItemSet(aSet);

            SfxStyleSheet* pSheet = rPage.GetStyleSheetForPresObj(PresObjKind::Text);
            if (pSheet)
                pNewObj->SetStyleSheet(pSheet, true);

            // Remove subtitle shape from page
            if( bUndo )
                pUndoManager->AddUndoAction(rModel.GetSdrUndoFactory().CreateUndoDeleteObject(*pSourceObj));

            rPage.RemoveObject( pSourceObj->GetOrdNum() );

            if( !bUndo )
                SdrObject::Free( pSourceObj );
        }
    }
    else if((eObjKind == PresObjKind::Outline) && (pSourceObj->GetObjIdentifier() != OBJ_OUTLINETEXT) )
    {
        switch( pSourceObj->GetObjIdentifier() )
        {
        case OBJ_TABLE: eObjKind = PresObjKind::Table; break;
        case OBJ_MEDIA: eObjKind = PresObjKind::Media; break;
        case OBJ_GRAF: eObjKind = PresObjKind::Graphic; break;
        case OBJ_OLE2: eObjKind = PresObjKind::Object; break;
        default: break;
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
    @param  rRect
        The rectangle that should be used to transform the shape
    @param  bInit
        If true the shape is created if not found
    @returns
        A presentation shape that was either found or created with the given parameters
*/
SdrObject* SdPage::InsertAutoLayoutShape(SdrObject* pObj, PresObjKind eObjKind, bool bVertical, const ::tools::Rectangle& rRect, bool bInit)
{
    SfxUndoManager* pUndoManager(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetUndoManager());
    const bool bUndo = pUndoManager && pUndoManager->IsInListAction() && IsInserted();

    if (!pObj && bInit)
    {
        pObj = CreatePresObj(eObjKind, bVertical, rRect);
    }
    else if ( pObj && (pObj->GetUserCall() || bInit) )
    {
        // convert object if shape type does not match kind (f.e. converting outline text to subtitle text)
        if( bInit )
            pObj = convertPresentationObjectImpl(*this, pObj, eObjKind, bVertical, rRect);

        if( bUndo )
        {
            pUndoManager->AddUndoAction( getSdrModelFromSdrPage().GetSdrUndoFactory().CreateUndoGeoObject( *pObj ) );
            pUndoManager->AddUndoAction( getSdrModelFromSdrPage().GetSdrUndoFactory().CreateUndoAttrObject( *pObj, true, true ) );
            pUndoManager->AddUndoAction( std::make_unique<UndoObjectUserCall>( *pObj ) );
        }

        pObj->AdjustToMaxRect(rRect);

        pObj->SetUserCall(this);

        SdrTextObj* pTextObject = dynamic_cast< SdrTextObj* >(pObj);
        if( pTextObject )
        {
            if( pTextObject->IsVerticalWriting() != bVertical )
            {
                pTextObject->SetVerticalWriting( bVertical );

                // here make sure the correct anchoring is used when the object
                // is re-used but orientation is changed
                if(PresObjKind::Outline == eObjKind)
                    pTextObject->SetMergedItem(SdrTextHorzAdjustItem( bVertical ? SDRTEXTHORZADJUST_RIGHT : SDRTEXTHORZADJUST_BLOCK ));
            }

            if( !mbMaster && (pTextObject->GetObjIdentifier() != OBJ_TABLE) )
            {
                if ( pTextObject->IsAutoGrowHeight() )
                {
                    // switch off AutoGrowHeight, set new MinHeight
                    SfxItemSet aTempAttr( static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetPool() );
                    SdrMetricItem aMinHeight( makeSdrTextMinFrameHeightItem(rRect.GetSize().Height()) );
                    aTempAttr.Put( aMinHeight );
                    aTempAttr.Put( makeSdrTextAutoGrowHeightItem(false) );
                    pTextObject->SetMergedItemSet(aTempAttr);
                    pTextObject->SetLogicRect(rRect);

                    // switch on AutoGrowHeight
                    SfxItemSet aAttr( static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetPool() );
                    aAttr.Put( makeSdrTextAutoGrowHeightItem(true) );

                    pTextObject->SetMergedItemSet(aAttr);
                }

                if ( pTextObject->IsAutoGrowWidth() )
                {
                    // switch off AutoGrowWidth , set new MinWidth
                    SfxItemSet aTempAttr( static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetPool() );
                    SdrMetricItem aMinWidth( makeSdrTextMinFrameWidthItem(rRect.GetSize().Width()) );
                    aTempAttr.Put( aMinWidth );
                    aTempAttr.Put( makeSdrTextAutoGrowWidthItem(false) );
                    pTextObject->SetMergedItemSet(aTempAttr);
                    pTextObject->SetLogicRect(rRect);

                    // switch on AutoGrowWidth
                    SfxItemSet aAttr( static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetPool() );
                    aAttr.Put( makeSdrTextAutoGrowWidthItem(true) );
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
                pUndoManager->AddUndoAction( std::make_unique<UndoObjectPresentationKind>( *pObj ) );

            InsertPresObj( pObj, eObjKind );
        }

        // make adjustments for vertical title and outline shapes
        if( bVertical && (( eObjKind == PresObjKind::Title) || (eObjKind == PresObjKind::Outline)))
        {
            SfxItemSet aNewSet(pObj->GetMergedItemSet());
            aNewSet.Put( makeSdrTextAutoGrowWidthItem(true) );
            aNewSet.Put( makeSdrTextAutoGrowHeightItem(false) );
            if( eObjKind == PresObjKind::Outline )
            {
                aNewSet.Put( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
                aNewSet.Put( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );
            }
            pObj->SetMergedItemSet(aNewSet);
        }
    }

    if ( pObj && (pObj->GetUserCall() || bInit) && ( pObj->IsEmptyPresObj() || dynamic_cast< const SdrGrafObj *>( pObj ) ==  nullptr ) )
        pObj->AdjustToMaxRect(rRect);

    return pObj;
}

/*************************************************************************
|*
|* Returns the PresObjKind of an object
|*
\************************************************************************/

PresObjKind SdPage::GetPresObjKind(SdrObject* pObj) const
{
    PresObjKind eKind = PresObjKind::NONE;
    if( (pObj != nullptr) && (maPresentationShapeList.hasShape(*pObj)) )
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
            pInfo->mePresObjKind = PresObjKind::NONE;
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
|* Set the text of an object
|*
\************************************************************************/

void SdPage::SetObjText(SdrTextObj* pObj, SdrOutliner* pOutliner, PresObjKind eObjKind, const OUString& rString )
{
    if ( !pObj )
        return;

    DBG_ASSERT( dynamic_cast< const SdrTextObj *>( pObj ) !=  nullptr, "SetObjText: No SdrTextObj!" );
    ::Outliner* pOutl = pOutliner;

    if (!pOutliner)
    {
        SfxItemPool* pPool(static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetDrawOutliner().GetEmptyItemSet().GetPool());
        pOutl = new ::Outliner( pPool, OutlinerMode::OutlineObject );
        pOutl->SetRefDevice( SD_MOD()->GetVirtualRefDevice() );
        pOutl->SetEditTextObjectPool(pPool);
        pOutl->SetStyleSheetPool(static_cast<SfxStyleSheetPool*>(getSdrModelFromSdrPage().GetStyleSheetPool()));
        pOutl->EnableUndo(false);
        pOutl->SetUpdateMode( false );
    }

    OutlinerMode nOutlMode = pOutl->GetMode();
    Size aPaperSize = pOutl->GetPaperSize();
    bool bUpdateMode = pOutl->GetUpdateMode();
    pOutl->SetUpdateMode(false);
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
        case PresObjKind::Outline:
        {
            pOutl->Init( OutlinerMode::OutlineObject );

            aString += "\t" + rString;

            if (mbMaster)
            {
                pOutl->SetStyleSheet( 0, GetStyleSheetForPresObj(eObjKind) );
                aString += "\n\t\t" +
                    SdResId(STR_PRESOBJ_MPOUTLLAYER2) +
                    "\n\t\t\t" +
                    SdResId(STR_PRESOBJ_MPOUTLLAYER3) +
                    "\n\t\t\t\t" +
                    SdResId(STR_PRESOBJ_MPOUTLLAYER4) +
                    "\n\t\t\t\t\t" +
                    SdResId(STR_PRESOBJ_MPOUTLLAYER5) +
                    "\n\t\t\t\t\t\t" +
                    SdResId(STR_PRESOBJ_MPOUTLLAYER6) +
                    "\n\t\t\t\t\t\t\t" +
                    SdResId(STR_PRESOBJ_MPOUTLLAYER7);

            }
        }
        break;

        case PresObjKind::Title:
        {
            pOutl->Init( OutlinerMode::TitleObject );
            aString += rString;
        }
        break;

        default:
        {
            pOutl->Init( OutlinerMode::TextObject );
            aString += rString;

            // check if we need to add a text field
            std::unique_ptr<SvxFieldData> pData;

            switch( eObjKind )
            {
            case PresObjKind::Header:
                pData.reset(new SvxHeaderField());
                break;
            case PresObjKind::Footer:
                pData .reset(new SvxFooterField());
                break;
            case PresObjKind::SlideNumber:
                pData.reset(new SvxPageField());
                break;
            case PresObjKind::DateTime:
                pData.reset(new SvxDateTimeField());
                break;
            default:
                break;
            }

            if( pData )
            {
                ESelection e;
                SvxFieldItem aField( *pData, EE_FEATURE_FIELD );
                pOutl->QuickInsertField(aField,e);
            }
        }
        break;
    }

    pOutl->SetPaperSize( pObj->GetLogicRect().GetSize() );

    if( !aString.isEmpty() )
        pOutl->SetText( aString, pOutl->GetParagraph( 0 ) );

    pObj->SetOutlinerParaObject( pOutl->CreateParaObject() );

    if (!pOutliner)
    {
        delete pOutl;
        pOutl = nullptr;
    }
    else
    {
        // restore the outliner
        pOutl->Init( nOutlMode );
        pOutl->SetParaAttribs( 0, pOutl->GetEmptyItemSet() );
        pOutl->SetUpdateMode( bUpdateMode );
        pOutl->SetPaperSize( aPaperSize );
    }
}

/*************************************************************************
|*
|* Set the name of the layout
|*
\************************************************************************/
void SdPage::SetLayoutName(const OUString& aName)
{
    maLayoutName = aName;

    if( mbMaster )
    {
        sal_Int32 nPos = maLayoutName.indexOf(SD_LT_SEPARATOR);
        if (nPos != -1)
            FmFormPage::SetName(maLayoutName.copy(0, nPos));
    }
}

/*************************************************************************
|*
|* Return the page name and generates it if necessary
|*
\************************************************************************/

const OUString& SdPage::GetName() const
{
    OUString aCreatedPageName( maCreatedPageName );
    if (GetRealName().isEmpty())
    {
        if ((mePageKind == PageKind::Standard || mePageKind == PageKind::Notes) && !mbMaster)
        {
            // default name for handout pages
            sal_uInt16  nNum = (GetPageNum() + 1) / 2;

            aCreatedPageName = SdResId(STR_PAGE) + " ";
            if (static_cast<SdDrawDocument&>(getSdrModelFromSdrPage()).GetDocumentType() == DocumentType::Draw )
                 aCreatedPageName = SdResId(STR_PAGE_NAME) + " ";

            if( getSdrModelFromSdrPage().GetPageNumType() == css::style::NumberingType::NUMBER_NONE )
            {
                // if the document has number none as a formatting
                // for page numbers we still default to arabic numbering
                // to keep the default page names unique
                aCreatedPageName += OUString::number( static_cast<sal_Int32>(nNum) );
            }
            else
            {
                aCreatedPageName += static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).CreatePageNumValue(nNum);
            }
        }
        else
        {
            /******************************************************************
            * default name for note pages
            ******************************************************************/
            aCreatedPageName = SdResId(STR_LAYOUT_DEFAULT_NAME);
        }
    }
    else
    {
        aCreatedPageName = GetRealName();
    }

    if (mePageKind == PageKind::Notes)
    {
        aCreatedPageName += " " + SdResId(STR_NOTES);
    }
    else if (mePageKind == PageKind::Handout && mbMaster)
    {
        aCreatedPageName += " (" + SdResId(STR_HANDOUT) + ")";
    }

    const_cast< SdPage* >(this)->maCreatedPageName = aCreatedPageName;
    return maCreatedPageName;
}

void SdPage::SetOrientation( Orientation /*eOrient*/)
{
    // Do nothing
}

Orientation SdPage::GetOrientation() const
{
    Size aSize = GetSize();
    if ( aSize.getWidth() > aSize.getHeight() )
    {
        return Orientation::Landscape;
    }
    else
    {
        return Orientation::Portrait;
    }
}

/*************************************************************************
|*
|* returns the default text of a PresObjektes
|*
\************************************************************************/

OUString SdPage::GetPresObjText(PresObjKind eObjKind) const
{
    OUString aString;

#if defined(IOS) || defined(ANDROID)
    bool isMobileDevice = true;
#else
    bool isMobileDevice = false;
    if (const SfxViewShell* pCurrentViewShell = SfxViewShell::Current())
        isMobileDevice = pCurrentViewShell->isLOKMobilePhone() || pCurrentViewShell->isLOKTablet();
#endif

    if (eObjKind == PresObjKind::Title)
    {
        if (mbMaster)
        {
            if (mePageKind != PageKind::Notes)
            {
                if (isMobileDevice)
                    aString = SdResId(STR_PRESOBJ_MPTITLE_MOBILE);
                else
                    aString = SdResId(STR_PRESOBJ_MPTITLE);
            }
            else
            {
                if (isMobileDevice)
                    aString = SdResId(STR_PRESOBJ_MPNOTESTITLE_MOBILE);
                else
                    aString = SdResId(STR_PRESOBJ_MPNOTESTITLE);
            }
        }
        else if (isMobileDevice)
            aString = SdResId(STR_PRESOBJ_TITLE_MOBILE);
        else
            aString = SdResId(STR_PRESOBJ_TITLE);
    }
    else if (eObjKind == PresObjKind::Outline)
    {
        if (mbMaster)
        {
            if (isMobileDevice)
                aString = SdResId(STR_PRESOBJ_MPOUTLINE_MOBILE);
            else
                aString = SdResId(STR_PRESOBJ_MPOUTLINE);
        }
        else if (isMobileDevice)
            aString = SdResId(STR_PRESOBJ_OUTLINE_MOBILE);
        else
            aString = SdResId(STR_PRESOBJ_OUTLINE);
    }
    else if (eObjKind == PresObjKind::Notes)
    {
        if (mbMaster)
        {
            if (isMobileDevice)
                aString = SdResId(STR_PRESOBJ_MPNOTESTEXT_MOBILE);
            else
                aString = SdResId(STR_PRESOBJ_MPNOTESTEXT);
        }
        else if (isMobileDevice)
            aString = SdResId(STR_PRESOBJ_NOTESTEXT_MOBILE);
        else
            aString = SdResId(STR_PRESOBJ_NOTESTEXT);
    }
    else if (eObjKind == PresObjKind::Text)
    {
        if (isMobileDevice)
            aString = SdResId(STR_PRESOBJ_TEXT_MOBILE);
        else
            aString = SdResId(STR_PRESOBJ_TEXT);
    }
    else if (eObjKind == PresObjKind::Graphic)
    {
        aString = SdResId( STR_PRESOBJ_GRAPHIC );
    }
    else if (eObjKind == PresObjKind::Object)
    {
        aString = SdResId( STR_PRESOBJ_OBJECT );
    }
    else if (eObjKind == PresObjKind::Chart)
    {
        aString = SdResId( STR_PRESOBJ_CHART );
    }
    else if (eObjKind == PresObjKind::OrgChart)
    {
        aString = SdResId( STR_PRESOBJ_ORGCHART );
    }
    else if (eObjKind == PresObjKind::Calc)
    {
        aString = SdResId( STR_PRESOBJ_TABLE );
    }

    return aString;
}

uno::Reference< uno::XInterface > SdPage::createUnoPage()
{
    return createUnoPageImpl( this );
}

/** returns the SdPage implementation for the given XDrawPage or 0 if not available */
SdPage* SdPage::getImplementation( const css::uno::Reference< css::drawing::XDrawPage >& xPage )
{
    try
    {
        auto pUnoPage = comphelper::getUnoTunnelImplementation<SvxDrawPage>(xPage);
        if( pUnoPage )
            return static_cast< SdPage* >( pUnoPage->GetSdrPage() );
    }
    catch( css::uno::Exception& )
    {
        OSL_FAIL("sd::SdPage::getImplementation(), exception caught!" );
    }

    return nullptr;
}

sal_Int64 SdPage::GetHashCode() const
{
    return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
}

void SdPage::SetName (const OUString& rName)
{
    OUString aOldName( GetName() );
    FmFormPage::SetName (rName);
    static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).UpdatePageRelativeURLs(aOldName, rName);
    ActionChanged();
}

const HeaderFooterSettings& SdPage::getHeaderFooterSettings() const
{
    if( mePageKind == PageKind::Handout && !mbMaster )
    {
        return static_cast<SdPage&>(TRG_GetMasterPage()).maHeaderFooterSettings;
    }
    else
    {
        return maHeaderFooterSettings;
    }
}

void SdPage::setHeaderFooterSettings( const sd::HeaderFooterSettings& rNewSettings )
{
    if( mePageKind == PageKind::Handout && !mbMaster )
    {
        static_cast<SdPage&>(TRG_GetMasterPage()).maHeaderFooterSettings = rNewSettings;
    }
    else
    {
        maHeaderFooterSettings = rNewSettings;
    }

    SetChanged();

    if(!TRG_HasMasterPage())
        return;

    TRG_GetMasterPageDescriptorViewContact().ActionChanged();

    // #i119056# For HeaderFooterSettings SdrObjects are used, but the properties
    // used are not part of their model data, but kept in SD. This data is applied
    // using a 'backdoor' on primitive creation. Thus, the normal mechanism to detect
    // object changes does not work here. It is necessary to trigger updates here
    // directly. BroadcastObjectChange used for PagePreview invalidations,
    // flushViewObjectContacts used to invalidate and flush all visualizations in
    // edit views.
    SdPage* pMasterPage = dynamic_cast< SdPage* >(&TRG_GetMasterPage());

    if(!pMasterPage)
        return;

    SdrObject* pCandidate = pMasterPage->GetPresObj( PresObjKind::Header );

    if(pCandidate)
    {
        pCandidate->BroadcastObjectChange();
        pCandidate->GetViewContact().flushViewObjectContacts();
    }

    pCandidate = pMasterPage->GetPresObj( PresObjKind::DateTime );

    if(pCandidate)
    {
        pCandidate->BroadcastObjectChange();
        pCandidate->GetViewContact().flushViewObjectContacts();
    }

    pCandidate = pMasterPage->GetPresObj( PresObjKind::Footer );

    if(pCandidate)
    {
        pCandidate->BroadcastObjectChange();
        pCandidate->GetViewContact().flushViewObjectContacts();
    }

    pCandidate = pMasterPage->GetPresObj( PresObjKind::SlideNumber );

    if(pCandidate)
    {
        pCandidate->BroadcastObjectChange();
        pCandidate->GetViewContact().flushViewObjectContacts();
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
    if( pObj == nullptr )
        return false;

    const SdrPage* pVisualizedPage = GetSdrPageFromXDrawPage(rOriginal.GetObjectContact().getViewInformation2D().getVisualizedPage());
    const bool bIsPrinting(rOriginal.GetObjectContact().isOutputToPrinter() || rOriginal.GetObjectContact().isOutputToPDFFile());
    const SdrPageView* pPageView = rOriginal.GetObjectContact().TryToGetSdrPageView();
    const bool bIsInsidePageObj(pPageView && pPageView->GetPage() != pVisualizedPage);

    // empty presentation objects only visible during edit mode
    if( (bIsPrinting || !bEdit || bIsInsidePageObj ) && pObj->IsEmptyPresObj() )
    {
        if( (pObj->GetObjInventor() != SdrInventor::Default) || ( (pObj->GetObjIdentifier() != OBJ_RECT) && (pObj->GetObjIdentifier() != OBJ_PAGE) ) )
            return false;
    }

    if( ( pObj->GetObjInventor() == SdrInventor::Default ) && ( pObj->GetObjIdentifier() == OBJ_TEXT ) )
    {
        const SdPage* pCheckPage = dynamic_cast< const SdPage* >(pObj->getSdrPageFromSdrObject());

        if( pCheckPage )
        {
            PresObjKind eKind = pCheckPage->GetPresObjKind(pObj);

            if((eKind == PresObjKind::Footer) || (eKind == PresObjKind::Header) || (eKind == PresObjKind::DateTime) || (eKind == PresObjKind::SlideNumber) )
            {
                const bool bSubContentProcessing(rDisplayInfo.GetSubContentActive());

                if( bSubContentProcessing || ( pCheckPage->GetPageKind() == PageKind::Handout && bIsPrinting ) )
                {
                    // use the page that is currently processed
                    const SdPage* pVisualizedSdPage = dynamic_cast< const SdPage* >(pVisualizedPage);

                    if( pVisualizedSdPage )
                    {
                        // if we are not on a masterpage, see if we have to draw this header&footer object at all
                        const sd::HeaderFooterSettings& rSettings = pVisualizedSdPage->getHeaderFooterSettings();

                        switch( eKind )
                        {
                        case PresObjKind::Footer:
                            return rSettings.mbFooterVisible;
                        case PresObjKind::Header:
                            return rSettings.mbHeaderVisible;
                        case PresObjKind::DateTime:
                            return rSettings.mbDateTimeVisible;
                        case PresObjKind::SlideNumber:
                            return rSettings.mbSlideNumberVisible;
                        default:
                            break;
                        }
                    }
                }
            } // check for placeholders on master
            else if( (eKind != PresObjKind::NONE) && pCheckPage->IsMasterPage() && ( pVisualizedPage != pCheckPage ) )
            {
                // presentation objects on master slide are always invisible if slide is shown.
                return false;
            }
        }
    }

    // i63977, do not print SdrpageObjs from master pages
    if( ( pObj->GetObjInventor() == SdrInventor::Default ) && ( pObj->GetObjIdentifier() == OBJ_PAGE ) )
    {
        if( pObj->getSdrPageFromSdrObject() && pObj->getSdrPageFromSdrObject()->IsMasterPage() )
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

        if (ePresObjKind == PresObjKind::Title   ||
            ePresObjKind == PresObjKind::Outline ||
            ePresObjKind == PresObjKind::Notes   ||
            ePresObjKind == PresObjKind::Text)
        {
            OUString aString( GetPresObjText(ePresObjKind) );

            if (!aString.isEmpty())
            {
                bool bVertical = false;
                OutlinerParaObject* pOldPara = pTextObj->GetOutlinerParaObject();
                if( pOldPara )
                    bVertical = pOldPara->IsVertical();  // is old para object vertical?

                SetObjText( pTextObj, nullptr, ePresObjKind, aString );

                if( pOldPara )
                {
                    // Here, only the vertical flag for the
                    // OutlinerParaObjects needs to be changed. The
                    // AutoGrowWidth/Height items still exist in the
                    // not changed object.
                    if(pTextObj->GetOutlinerParaObject()
                        && pTextObj->GetOutlinerParaObject()->IsVertical() != bVertical)
                    {
                        ::tools::Rectangle aObjectRect = pTextObj->GetSnapRect();
                        pTextObj->GetOutlinerParaObject()->SetVertical(bVertical);
                        pTextObj->SetSnapRect(aObjectRect);
                    }
                }

                pTextObj->SetTextEditOutliner( nullptr );  // to make stylesheet settings work
                pTextObj->NbcSetStyleSheet( GetStyleSheetForPresObj(ePresObjKind), true );
                pTextObj->SetEmptyPresObj(true);
                bRet = true;
            }
        }
    }
    return bRet;
}

void SdPage::CalculateHandoutAreas( SdDrawDocument& rModel, AutoLayout eLayout, bool bHorizontal, std::vector< ::tools::Rectangle >& rAreas )
{
    SdPage& rHandoutMaster = *rModel.GetMasterSdPage( 0, PageKind::Handout );

    static const sal_uInt16 aOffsets[5][9] =
    {
        { 0, 1, 2, 3, 4, 5, 6, 7, 8 }, // AUTOLAYOUT_HANDOUT9, Portrait, Horizontal order
        { 0, 2, 4, 1, 3, 5, 0, 0, 0 }, // AUTOLAYOUT_HANDOUT3, Landscape, Vertical
        { 0, 2, 1, 3, 0, 0, 0, 0, 0 }, // AUTOLAYOUT_HANDOUT4, Landscape, Vertical
        { 0, 3, 1, 4, 2, 5, 0, 0, 0 }, // AUTOLAYOUT_HANDOUT4, Portrait, Vertical
        { 0, 3, 6, 1, 4, 7, 2, 5, 8 }, // AUTOLAYOUT_HANDOUT9, Landscape, Vertical
    };

    const sal_uInt16* pOffsets = aOffsets[0];

    Size aArea = rHandoutMaster.GetSize();
    const bool bLandscape = aArea.Width() > aArea.Height();

    if( eLayout == AUTOLAYOUT_NONE )
    {
        // use layout from handout master
        SdrObjListIter aShapeIter(&rHandoutMaster);

        std::vector< ::tools::Rectangle > vSlidesAreas;
        while ( aShapeIter.IsMore() )
        {
            SdrPageObj* pPageObj = dynamic_cast<SdrPageObj*>( aShapeIter.Next() );
            // get slide rectangles
            if (pPageObj)
                vSlidesAreas.push_back( pPageObj->GetCurrentBoundRect() );
        }

        if ( !bHorizontal || vSlidesAreas.size() < 4 )
        { // top to bottom, then right
            rAreas.swap( vSlidesAreas );
        }
        else
        { // left to right, then down
            switch ( vSlidesAreas.size() )
            {
                case 4:
                    pOffsets = aOffsets[2];
                    break;

                default:
                    [[fallthrough]];
                case 6:
                    pOffsets = aOffsets[ bLandscape ? 3 : 1 ];
                    break;

                case 9:
                    pOffsets = aOffsets[4];
                    break;
            }

            rAreas.resize( static_cast<size_t>(vSlidesAreas.size()) );

            for( const ::tools::Rectangle& rRect : vSlidesAreas )
            {
                rAreas[*pOffsets++] = rRect;
            }
        }
    }
    else
    {
        const ::tools::Long nGapW = 1000; // gap is 1cm
        const ::tools::Long nGapH = 1000;

        ::tools::Long nLeftBorder = rHandoutMaster.GetLeftBorder();
        ::tools::Long nRightBorder = rHandoutMaster.GetRightBorder();
        ::tools::Long nTopBorder = rHandoutMaster.GetUpperBorder();
        ::tools::Long nBottomBorder = rHandoutMaster.GetLowerBorder();

        const ::tools::Long nHeaderFooterHeight = static_cast< ::tools::Long >( (aArea.Height() - nTopBorder - nLeftBorder) * 0.05  );

        nTopBorder += nHeaderFooterHeight;
        nBottomBorder += nHeaderFooterHeight;

        ::tools::Long nX = nGapW + nLeftBorder;
        ::tools::Long nY = nGapH + nTopBorder;

        aArea.AdjustWidth( -(nGapW * 2 + nLeftBorder + nRightBorder) );
        aArea.AdjustHeight( -(nGapH * 2 + nTopBorder + nBottomBorder) );

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

        rAreas.resize(static_cast<size_t>(nColCnt) * nRowCnt);

        Size aPartArea, aSize;
        aPartArea.setWidth( (aArea.Width()  - ((nColCnt-1) * nGapW) ) / nColCnt );
        aPartArea.setHeight( (aArea.Height() - ((nRowCnt-1) * nGapH) ) / nRowCnt );

        SdrPage* pFirstPage = rModel.GetMasterSdPage(0, PageKind::Standard);
        if (pFirstPage && pFirstPage->GetWidth() && pFirstPage->GetHeight())
        {
            // scale actual size into handout rect
            double fScale = static_cast<double>(aPartArea.Width()) / static_cast<double>(pFirstPage->GetWidth());

            aSize.setHeight( static_cast<::tools::Long>(fScale * pFirstPage->GetHeight() ) );
            if( aSize.Height() > aPartArea.Height() )
            {
                fScale = static_cast<double>(aPartArea.Height()) / static_cast<double>(pFirstPage->GetHeight());
                aSize.setHeight( aPartArea.Height() );
                aSize.setWidth( static_cast<::tools::Long>(fScale * pFirstPage->GetWidth()) );
            }
            else
            {
                aSize.setWidth( aPartArea.Width() );
            }

            nX += (aPartArea.Width() - aSize.Width()) / 2;
            nY += (aPartArea.Height()- aSize.Height())/ 2;
        }
        else
        {
            aSize = aPartArea;
        }

        Point aPos( nX, nY );

        const bool bRTL = rModel.GetDefaultWritingMode() == css::text::WritingMode_RL_TB;

        const ::tools::Long nOffsetX = (aPartArea.Width() + nGapW) * (bRTL ? -1 : 1);
        const ::tools::Long nOffsetY = aPartArea.Height() + nGapH;
        const ::tools::Long nStartX = bRTL ? nOffsetX*(1 - nColCnt) + nX : nX;

        for(sal_uInt16 nRow = 0; nRow < nRowCnt; nRow++)
        {
            aPos.setX( nStartX );
            for(sal_uInt16 nCol = 0; nCol < nColCnt; nCol++)
            {
                rAreas[*pOffsets++] = ::tools::Rectangle(aPos, aSize);
                aPos.AdjustX(nOffsetX );
            }

            aPos.AdjustY(nOffsetY );
        }
    }
}

void SdPage::SetPrecious (const bool bIsPrecious)
{
    mbIsPrecious = bIsPrecious;
}

HeaderFooterSettings::HeaderFooterSettings()
{
    mbHeaderVisible = true;
    mbFooterVisible = true;
    mbSlideNumberVisible = false;
    mbDateTimeVisible = true;
    mbDateTimeIsFixed = true;
    meDateFormat = SvxDateFormat::A;
    meTimeFormat = SvxTimeFormat::AppDefault;
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
           (meDateFormat == rSettings.meDateFormat) &&
           (meTimeFormat == rSettings.meTimeFormat) &&
           (maDateTimeText == rSettings.maDateTimeText);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
