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

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <futempl.hxx>

#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/eeitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxdlg.hxx>
#include <editeng/numitem.hxx>
#include <svx/svdopage.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <svx/svditer.hxx>
#include <svx/sdr/properties/properties.hxx>
#include <svl/intitem.hxx>

#include <sfx2/viewfrm.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflhtit.hxx>
#include <o3tl/string_view.hxx>
#include <app.hrc>
#include <stlsheet.hxx>
#include <sdpage.hxx>
#include <stlpool.hxx>
#include <sdmod.hxx>
#include <View.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <ViewShell.hxx>

#include <strings.hrc>
#include <prlayout.hxx>
#include <sdresid.hxx>
#include <OutlineView.hxx>
#include <sdabstdlg.hxx>
#include <memory>

using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::style;

namespace sd
{


FuTemplate::FuTemplate (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq )
    : FuPoor( pViewSh, pWin, pView, pDoc, rReq )
{
}

rtl::Reference<FuPoor> FuTemplate::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuTemplate( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuTemplate::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt16 nSId = rReq.GetSlot();

    // get StyleSheet parameter
    SfxStyleSheetBasePool* pSSPool = mpDoc->GetDocSh()->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = nullptr;

    const SfxPoolItem* pItem;
    SfxStyleFamily nFamily = SfxStyleFamily(USHRT_MAX);
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( SID_STYLE_FAMILY,
        false, &pItem ))
    {
        nFamily = static_cast<SfxStyleFamily>( pArgs->Get( SID_STYLE_FAMILY ).GetValue());
    }
    else if( pArgs && SfxItemState::SET == pArgs->GetItemState( SID_STYLE_FAMILYNAME,
        false, &pItem ))
    {
        OUString sFamily = pArgs->Get( SID_STYLE_FAMILYNAME ).GetValue();
        if (sFamily == "graphics")
            nFamily = SfxStyleFamily::Para;
        else
            nFamily = SfxStyleFamily::Pseudo;
    }

    OUString aStyleName;
    sal_uInt16 nRetMask = static_cast<sal_uInt16>(SfxStyleSearchBits::All);

    switch( nSId )
    {
        case SID_STYLE_APPLY:
        case SID_STYLE_EDIT:
        case SID_STYLE_DELETE:
        case SID_STYLE_HIDE:
        case SID_STYLE_SHOW:
        case SID_STYLE_FAMILY:
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            const SfxStringItem* pNameItem = rReq.GetArg<SfxStringItem>(SID_APPLY_STYLE);
            const SfxStringItem* pFamilyItem = rReq.GetArg<SfxStringItem>(SID_STYLE_FAMILYNAME);
            if ( pFamilyItem && pNameItem )
            {
                try
                {
                    Reference< XStyleFamiliesSupplier > xModel(mpDoc->GetDocSh()->GetModel(), UNO_QUERY_THROW );
                    Reference< XNameAccess > xCont( xModel->getStyleFamilies() );
                    Reference< XNameAccess > xStyles( xCont->getByName(pFamilyItem->GetValue()), UNO_QUERY_THROW );
                    Reference< XPropertySet > xInfo( xStyles->getByName( pNameItem->GetValue() ), UNO_QUERY_THROW );

                    OUString aUIName;
                    xInfo->getPropertyValue( u"DisplayName"_ustr ) >>= aUIName;
                    if ( !aUIName.isEmpty() )
                        rReq.AppendItem( SfxStringItem( nSId, aUIName ) );
                }
                catch( Exception& )
                {
                }
            }

            if (pArgs && pArgs->GetItemState(nSId) == SfxItemState::SET)
                aStyleName = static_cast<const SfxStringItem &>( pArgs->Get( nSId ) ).GetValue();
        }
    }

    switch( nSId )
    {
        case SID_STYLE_NEW:
        {
            SfxStyleSheetBase *p = pSSPool->Find(aStyleName, nFamily );
            if(p)
            {
                pSSPool->Remove(p);
                p = nullptr;
            }
            pStyleSheet = &pSSPool->Make( aStyleName, nFamily, SfxStyleSearchBits::UserDefined );

            if (pArgs && pArgs->GetItemState(SID_STYLE_REFERENCE) == SfxItemState::SET)
            {
                OUString aParentName( pArgs->Get(SID_STYLE_REFERENCE).GetValue());
                pStyleSheet->SetParent(aParentName);
            }
            else
            {
                pStyleSheet->SetParent(SdResId(STR_STANDARD_STYLESHEET_NAME));
            }
        }
        break;

        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            // at the moment, the dialog to enter the name of the template is still opened
            SfxStyleSheetBase *p = pSSPool->Find(aStyleName, nFamily );
            if(p)
            {
                pSSPool->Remove(p);
                p = nullptr;
            }
            pStyleSheet = &pSSPool->Make( aStyleName, nFamily, SfxStyleSearchBits::UserDefined );
            pStyleSheet->SetParent(SdResId(STR_STANDARD_STYLESHEET_NAME));
        }
        break;

        case SID_STYLE_EDIT:
            pStyleSheet = pSSPool->Find( aStyleName, nFamily);
        break;

        case SID_STYLE_DELETE:
            pStyleSheet = pSSPool->Find( aStyleName, nFamily);
            if( pStyleSheet )
            {
                pSSPool->Remove( pStyleSheet );
                nRetMask = sal_uInt16(true);
                mpDoc->SetChanged();
            }
            else
            {
                nRetMask = sal_uInt16(false);
            }
        break;

        case SID_STYLE_HIDE:
        case SID_STYLE_SHOW:
            pStyleSheet = pSSPool->Find( aStyleName, nFamily);
            pStyleSheet->SetHidden( nSId == SID_STYLE_HIDE );
            nRetMask = sal_uInt16(true);
        break;

        case SID_STYLE_APPLY:
            // apply the template to the document
            pStyleSheet = pSSPool->Find( aStyleName, nFamily);

            // do not set presentation styles, they will be set implicit
            if ( pStyleSheet && pStyleSheet->GetFamily() != SfxStyleFamily::Pseudo )
            {
                SfxStyleSheet* pOldStyleSheet = mpView->GetStyleSheet();
                OUString aStr;

                if( // if the object had no style sheet, allow all
                    !pOldStyleSheet ||

                    // allow if old and new style sheet has same family
                    pStyleSheet->GetFamily() == pOldStyleSheet->GetFamily() ||

                    // allow if old was background objects and new is graphics
                    (pStyleSheet->GetFamily() == SfxStyleFamily::Para && pOldStyleSheet->GetHelpId( aStr ) == HID_PSEUDOSHEET_BACKGROUNDOBJECTS) ||

                    // allow if old was presentation and we are a drawing document
                    (pOldStyleSheet->GetFamily() == SfxStyleFamily::Page && mpDoc->GetDocumentType() == DocumentType::Draw) )
                {
                    mpView->SetStyleSheet( static_cast<SfxStyleSheet*>(pStyleSheet));
                    mpDoc->SetChanged();
                    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_FAMILY2 );
                }
            }
        break;

        case SID_STYLE_WATERCAN:
        {
            if( !SD_MOD()->GetWaterCan() )
            {
                if (pArgs && pArgs->GetItemState( nSId ) == SfxItemState::SET)
                {
                    aStyleName = static_cast<const SfxStringItem &>( pArgs->Get( nSId ) ).GetValue();
                    SD_MOD()->SetWaterCan( true );
                    pStyleSheet = pSSPool->Find( aStyleName, nFamily);
                }
                // no presentation object templates, they are only allowed implicitly
                if( pStyleSheet && pStyleSheet->GetFamily() != SfxStyleFamily::Pseudo )
                {
                    static_cast<SdStyleSheetPool*>( pSSPool )->SetActualStyleSheet( pStyleSheet );

                    // we switch explicitly into selection mode
                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_OBJECT_SELECT,
                                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );

                }
                else
                    SD_MOD()->SetWaterCan( false );
            }
            else
            {
                SD_MOD()->SetWaterCan( false );
                // we have to re-enable to tools-bar
                mpViewShell->Invalidate();
            }
        }
        break;

        default:
        break;
    }

    switch( nSId )
    {
        case SID_STYLE_NEW:
        case SID_STYLE_EDIT:
        {
            PresentationObjects ePO = PresentationObjects::Outline_1;

            if( pStyleSheet )
            {
                ScopedVclPtr<SfxAbstractTabDialog> pStdDlg;
                ScopedVclPtr<SfxAbstractTabDialog> pPresDlg;
                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();

                SfxStyleFamily eFamily = pStyleSheet->GetFamily();

                if (eFamily == SfxStyleFamily::Para)
                {
                    pStdDlg.disposeAndReset(pFact ? pFact->CreateSdTabTemplateDlg(mpViewShell->GetFrameWeld(), mpDoc->GetDocSh(), *pStyleSheet, mpDoc, mpView) : nullptr);
                }
                else if (eFamily == SfxStyleFamily::Pseudo)
                {
                    OUString aName(pStyleSheet->GetName());
                    bool bBackground = false;
                    bool bOldDocInOtherLanguage = false;

                    if (aName == SdResId(STR_PSEUDOSHEET_TITLE))
                    {
                        ePO    = PresentationObjects::Title;
                    }
                    else if (aName == SdResId(STR_PSEUDOSHEET_SUBTITLE))
                    {
                        ePO    = PresentationObjects::Subtitle;
                    }
                    else if (aName ==
                             SdResId(STR_PSEUDOSHEET_BACKGROUND))
                    {
                        bBackground = true;
                        ePO    = PresentationObjects::Background;
                    }
                    else if (aName ==
                             SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS))
                    {
                        ePO    = PresentationObjects::BackgroundObjects;
                    }
                    else if (aName ==
                             SdResId(STR_PSEUDOSHEET_NOTES))
                    {
                        ePO    = PresentationObjects::Notes;
                    }
                    else if(aName.indexOf(SdResId(STR_PSEUDOSHEET_OUTLINE)) != -1)
                    {
                        OUString aOutlineStr(SdResId(STR_PSEUDOSHEET_OUTLINE));
                        // determine number, mind the blank between name and number
                        std::u16string_view aNumStr(aName.subView(aOutlineStr.getLength() + 1));
                        sal_uInt16 nLevel = static_cast<sal_uInt16>(o3tl::toInt32(aNumStr));
                        switch (nLevel)
                        {
                            case 1: ePO = PresentationObjects::Outline_1; break;
                            case 2: ePO = PresentationObjects::Outline_2; break;
                            case 3: ePO = PresentationObjects::Outline_3; break;
                            case 4: ePO = PresentationObjects::Outline_4; break;
                            case 5: ePO = PresentationObjects::Outline_5; break;
                            case 6: ePO = PresentationObjects::Outline_6; break;
                            case 7: ePO = PresentationObjects::Outline_7; break;
                            case 8: ePO = PresentationObjects::Outline_8; break;
                            case 9: ePO = PresentationObjects::Outline_9; break;
                        }
                    }
                    else
                    {
                        OSL_FAIL("StyleSheet from older version with different language");
                        bOldDocInOtherLanguage = true;
                    }

                    if( !bOldDocInOtherLanguage )
                    {
                        pPresDlg.disposeAndReset(pFact ? pFact->CreateSdPresLayoutTemplateDlg(mpDocSh,  mpViewShell->GetFrameWeld(), bBackground, *pStyleSheet, ePO, pSSPool ) : nullptr);
                    }
                }

                sal_uInt16 nResult = RET_CANCEL;
                const SfxItemSet* pOutSet = nullptr;
                if (pStdDlg)
                {
                    nResult = pStdDlg->Execute();
                    pOutSet = pStdDlg->GetOutputItemSet();
                }
                else if( pPresDlg )
                {
                    nResult = pPresDlg->Execute();
                    pOutSet = pPresDlg->GetOutputItemSet();
                }

                switch( nResult )
                {
                    case RET_OK:
                    {
                        nRetMask = static_cast<sal_uInt16>(pStyleSheet->GetMask());

                        if (eFamily == SfxStyleFamily::Pseudo)
                        {
                            SfxItemSet aTempSet(*pOutSet);
                            /* Extract SvxBrushItem out of set and insert SvxColorItem */
                            const SvxBrushItem* pBrushItem = aTempSet.GetItem<SvxBrushItem>( SID_ATTR_BRUSH_CHAR );

                            if ( pBrushItem )
                            {
                                SvxColorItem aBackColorItem(pBrushItem->GetColor(), EE_CHAR_BKGCOLOR);
                                aTempSet.ClearItem( EE_CHAR_BKGCOLOR );
                                aTempSet.Put( aBackColorItem );
                            }
                            static_cast<SdStyleSheet*>(pStyleSheet)->AdjustToFontHeight(aTempSet);

                            /* Special treatment: reset the INVALIDS to
                               NULL-Pointer (otherwise INVALIDs or pointer point
                               to DefaultItems in the template; both would
                               prevent the attribute inheritance) */
                            aTempSet.ClearInvalidItems();

                            // EE_PARA_NUMBULLET item is only valid in first outline template
                            if( (ePO >= PresentationObjects::Outline_2) && (ePO <= PresentationObjects::Outline_9) )
                            {
                                if (aTempSet.GetItemState(EE_PARA_NUMBULLET) == SfxItemState::SET)
                                {
                                    SvxNumRule aRule(aTempSet.GetItem<SvxNumBulletItem>(EE_PARA_NUMBULLET)->GetNumRule());

                                    OUString sStyleName(SdResId(STR_PSEUDOSHEET_OUTLINE) + " 1");
                                    SfxStyleSheetBase* pFirstStyleSheet = pSSPool->Find( sStyleName, SfxStyleFamily::Pseudo);

                                    if(pFirstStyleSheet)
                                    {
                                        pFirstStyleSheet->GetItemSet().Put( SvxNumBulletItem( aRule, EE_PARA_NUMBULLET ));
                                        SdStyleSheet* pRealSheet = static_cast<SdStyleSheet*>(pFirstStyleSheet)->GetRealStyleSheet();
                                        pRealSheet->Broadcast(SfxHint(SfxHintId::DataChanged));
                                    }

                                    aTempSet.ClearItem( EE_PARA_NUMBULLET );
                                }
                            }

                            pStyleSheet->GetItemSet().Put(aTempSet);
                            SdStyleSheet::BroadcastSdStyleSheetChange(pStyleSheet, ePO, pSSPool);
                        }

                        SfxItemSet& rAttr = pStyleSheet->GetItemSet();

                        sdr::properties::CleanupFillProperties( rAttr );

                        // check for unique names of named items for xml
                        if( rAttr.GetItemState( XATTR_FILLBITMAP ) == SfxItemState::SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_FILLBITMAP );
                            std::unique_ptr<SfxPoolItem> pNewItem = static_cast<const XFillBitmapItem*>(pOldItem)->checkForUniqueItem( mpDoc );
                            if( pNewItem )
                            {
                                rAttr.Put( std::move(pNewItem) );
                            }
                        }
                        if( rAttr.GetItemState( XATTR_LINEDASH ) == SfxItemState::SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_LINEDASH );
                            std::unique_ptr<SfxPoolItem> pNewItem = static_cast<const XLineDashItem*>(pOldItem)->checkForUniqueItem( mpDoc );
                            if( pNewItem )
                            {
                                rAttr.Put( std::move(pNewItem) );
                            }
                        }
                        if( rAttr.GetItemState( XATTR_LINESTART ) == SfxItemState::SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_LINESTART );
                            std::unique_ptr<SfxPoolItem> pNewItem = static_cast<const XLineStartItem*>(pOldItem)->checkForUniqueItem( mpDoc );
                            if( pNewItem )
                            {
                                rAttr.Put( std::move(pNewItem) );
                            }
                        }
                        if( rAttr.GetItemState( XATTR_LINEEND ) == SfxItemState::SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_LINEEND );
                            std::unique_ptr<SfxPoolItem> pNewItem = static_cast<const XLineEndItem*>(pOldItem)->checkForUniqueItem( mpDoc );
                            if( pNewItem )
                            {
                                rAttr.Put( std::move(pNewItem) );
                            }
                        }
                        if( rAttr.GetItemState( XATTR_FILLGRADIENT ) == SfxItemState::SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_FILLGRADIENT );
                            std::unique_ptr<SfxPoolItem> pNewItem = static_cast<const XFillGradientItem*>(pOldItem)->checkForUniqueItem( mpDoc );
                            if( pNewItem )
                            {
                                rAttr.Put( std::move(pNewItem) );
                            }
                        }
                        if( rAttr.GetItemState( XATTR_FILLFLOATTRANSPARENCE ) == SfxItemState::SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_FILLFLOATTRANSPARENCE );
                            std::unique_ptr<SfxPoolItem> pNewItem = static_cast<const XFillFloatTransparenceItem*>(pOldItem)->checkForUniqueItem( mpDoc );
                            if( pNewItem )
                            {
                                rAttr.Put( std::move(pNewItem) );
                            }
                        }
                        if( rAttr.GetItemState( XATTR_FILLHATCH ) == SfxItemState::SET )
                        {
                            const SfxPoolItem* pOldItem = rAttr.GetItem( XATTR_FILLHATCH );
                            std::unique_ptr<SfxPoolItem> pNewItem = static_cast<const XFillHatchItem*>(pOldItem)->checkForUniqueItem( mpDoc );
                            if( pNewItem )
                            {
                                rAttr.Put( std::move(pNewItem) );
                            }
                        }

                        static_cast<SfxStyleSheet*>( pStyleSheet )->Broadcast( SfxHint( SfxHintId::DataChanged ) );

                        DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >( mpViewShell );
                        if( pDrawViewShell )
                        {
                            PageKind ePageKind = pDrawViewShell->GetPageKind();
                            if( ePageKind == PageKind::Notes || ePageKind == PageKind::Handout )
                            {
                                SdPage* pPage = mpViewShell->GetActualPage();

                                if(pDrawViewShell->GetEditMode() == EditMode::MasterPage)
                                {
                                    pPage = static_cast<SdPage*>((&(pPage->TRG_GetMasterPage())));
                                }

                                if( pPage )
                                {
                                    SdrObjListIter aIter( pPage );
                                    while( aIter.IsMore() )
                                    {
                                        SdrObject* pObj = aIter.Next();
                                        if( dynamic_cast< const SdrPageObj *>( pObj ) !=  nullptr )
                                        {
                                            // repaint only
                                            pObj->ActionChanged();
                                            // pObj->SendRepaintBroadcast();
                                        }
                                    }
                                }
                            }
                        }

                        if( mpDoc->GetOnlineSpell() )
                        {
                            if( SfxItemState::SET == rAttr.GetItemState(EE_CHAR_LANGUAGE, false ) ||
                                SfxItemState::SET == rAttr.GetItemState(EE_CHAR_LANGUAGE_CJK, false ) ||
                                SfxItemState::SET == rAttr.GetItemState(EE_CHAR_LANGUAGE_CTL, false ) )
                            {
                                mpDoc->StopOnlineSpelling();
                                mpDoc->StartOnlineSpelling();
                            }
                        }

                        mpDoc->SetChanged();
                    }
                    break;

                    default:
                    {
                        if( nSId == SID_STYLE_NEW )
                            pSSPool->Remove( pStyleSheet );
                    }
                    return; // Cancel
                }
            }
        }
        break;

        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            if( pStyleSheet )
            {
                nRetMask = static_cast<sal_uInt16>(pStyleSheet->GetMask());
                SfxItemSet aCoreSet( mpDoc->GetPool() );
                mpView->GetAttributes( aCoreSet, true );

                // if the object had a template, this becomes parent of the new template
                SfxStyleSheet* pOldStyle = mpView->GetStyleSheet();

                // if pOldStyle == pStyleSheet -> recursion
                if( pOldStyle != pStyleSheet )
                {
                    if (pOldStyle)
                    {
                        pStyleSheet->SetParent(pOldStyle->GetName());
                    }

                    SfxItemSet* pStyleSet = &pStyleSheet->GetItemSet();
                    pStyleSet->Put(aCoreSet);

                    /* apply template (but not when somebody is editing a text.
                       To do this, the edit engine had to be capable to use
                       templates on a character level. */
                    if (!mpView->GetTextEditObject())
                    {
                        mpView->SetStyleSheet( static_cast<SfxStyleSheet*>(pStyleSheet));
                    }

                    static_cast<SfxStyleSheet*>( pStyleSheet )->Broadcast( SfxHint( SfxHintId::DataChanged ) );
                    mpDoc->SetChanged();

                    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_FAMILY2 );
                }
            }
        }
        break;

        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            if ((mpView->AreObjectsMarked() && mpView->GetMarkedObjectList().GetMarkCount() == 1) ||
                 dynamic_cast< const OutlineView *>( mpView ) !=  nullptr)
            {
                pStyleSheet = mpView->GetStyleSheet();

                if( pStyleSheet )
                {
                    nRetMask = static_cast<sal_uInt16>(pStyleSheet->GetMask());
                    SfxItemSet aCoreSet( mpDoc->GetPool() );
                    mpView->GetAttributes( aCoreSet );

                    SfxItemSet* pStyleSet = &pStyleSheet->GetItemSet();
                    pStyleSet->Put( aCoreSet );

                    mpView->SetStyleSheet( static_cast<SfxStyleSheet*>(pStyleSheet));

                    static_cast<SfxStyleSheet*>( pStyleSheet )->Broadcast( SfxHint( SfxHintId::DataChanged ) );
                    mpDoc->SetChanged();
                    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SID_STYLE_FAMILY2 );
                }
            }
        }
        break;

    }
    if( nRetMask != static_cast<sal_uInt16>(SfxStyleSearchBits::All) )
        rReq.SetReturnValue( SfxUInt16Item( nSId, nRetMask ) );
}

void FuTemplate::Activate()
{
}

void FuTemplate::Deactivate()
{
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
