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

#include "scitems.hxx"
#include <editeng/borderline.hxx>
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/newstyle.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>

#include <svl/stritem.hxx>
#include <svl/zformat.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/numinf.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/tplpitem.hxx>
#include <editeng/svxenum.hxx>
#include <svx/algitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/fhgtitem.hxx>

#include "formatsh.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "docsh.hxx"
#include "patattr.hxx"
#include "scmod.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "printfun.hxx"
#include "docpool.hxx"
#include "scresid.hxx"
#include "tabvwsh.hxx"
#include "undostyl.hxx"
#include "markdata.hxx"
#include "markarr.hxx"

#define ScFormatShell
#define TableFont
#define FormatForSelection
#include "scslots.hxx"

#include "scabstdlg.hxx"
#include <editeng/fontitem.hxx>
#include <sfx2/classificationhelper.hxx>

#include <memory>

using namespace ::com::sun::star;

namespace {

SvxCellHorJustify lclConvertSlotToHAlign( sal_uInt16 nSlot )
{
    SvxCellHorJustify eHJustify = SVX_HOR_JUSTIFY_STANDARD;
    switch( nSlot )
    {
        case SID_ALIGN_ANY_HDEFAULT:    eHJustify = SVX_HOR_JUSTIFY_STANDARD;   break;
        case SID_ALIGN_ANY_LEFT:        eHJustify = SVX_HOR_JUSTIFY_LEFT;       break;
        case SID_ALIGN_ANY_HCENTER:     eHJustify = SVX_HOR_JUSTIFY_CENTER;     break;
        case SID_ALIGN_ANY_RIGHT:       eHJustify = SVX_HOR_JUSTIFY_RIGHT;      break;
        case SID_ALIGN_ANY_JUSTIFIED:   eHJustify = SVX_HOR_JUSTIFY_BLOCK;      break;
        default:    OSL_FAIL( "lclConvertSlotToHAlign - invalid slot" );
    }
    return eHJustify;
}

SvxCellVerJustify lclConvertSlotToVAlign( sal_uInt16 nSlot )
{
    SvxCellVerJustify eVJustify = SVX_VER_JUSTIFY_STANDARD;
    switch( nSlot )
    {
        case SID_ALIGN_ANY_VDEFAULT:    eVJustify = SVX_VER_JUSTIFY_STANDARD;   break;
        case SID_ALIGN_ANY_TOP:         eVJustify = SVX_VER_JUSTIFY_TOP;        break;
        case SID_ALIGN_ANY_VCENTER:     eVJustify = SVX_VER_JUSTIFY_CENTER;     break;
        case SID_ALIGN_ANY_BOTTOM:      eVJustify = SVX_VER_JUSTIFY_BOTTOM;     break;
        default:    OSL_FAIL( "lclConvertSlotToVAlign - invalid slot" );
    }
    return eVJustify;
}

} // namespace


SFX_IMPL_INTERFACE(ScFormatShell, SfxShell)

void ScFormatShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_SERVER,
                                            RID_OBJECTBAR_FORMAT);
}

ScFormatShell::ScFormatShell(ScViewData* pData) :
    SfxShell(pData->GetViewShell()),
    pViewData(pData)
{
    ScTabViewShell* pTabViewShell = GetViewData()->GetViewShell();

    SetPool( &pTabViewShell->GetPool() );
    ::svl::IUndoManager* pMgr = pViewData->GetSfxDocShell()->GetUndoManager();
    SetUndoManager( pMgr );
    if ( !pViewData->GetDocument()->IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
    SetHelpId(HID_SCSHELL_FORMATSH);
    SetName("Format");
}

ScFormatShell::~ScFormatShell()
{
}

void ScFormatShell::GetStyleState( SfxItemSet& rSet )
{
    ScDocument*             pDoc          = GetViewData()->GetDocument();
    ScTabViewShell*         pTabViewShell = GetViewData()->GetViewShell();
    SfxStyleSheetBasePool*  pStylePool    = pDoc->GetStyleSheetPool();

    bool bProtected = false;
    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB i=0; i<nTabCount; i++)
        if (pDoc->IsTabProtected(i))                // look after protected table
            bProtected = true;

    SfxWhichIter    aIter(rSet);
    sal_uInt16          nWhich = aIter.FirstWhich();
    sal_uInt16          nSlotId = 0;

    while ( nWhich )
    {
        nSlotId = SfxItemPool::IsWhich( nWhich )
                    ? GetPool().GetSlotId( nWhich )
                    : nWhich;

        switch ( nSlotId )
        {
            case SID_STYLE_APPLY:
                if ( !pStylePool )
                    rSet.DisableItem( nSlotId );
                break;

            case SID_STYLE_FAMILY2:     // cell style sheets
            {
                SfxStyleSheet* pStyleSheet = const_cast<SfxStyleSheet*>(
                                             pTabViewShell->GetStyleSheetFromMarked());

                if ( pStyleSheet )
                    rSet.Put( SfxTemplateItem( nSlotId, pStyleSheet->GetName() ) );
                else
                    rSet.Put( SfxTemplateItem( nSlotId, OUString() ) );
            }
            break;

            case SID_STYLE_FAMILY4:     // page style sheets
            {
                SCTAB           nCurTab     = GetViewData()->GetTabNo();
                OUString        aPageStyle  = pDoc->GetPageStyle( nCurTab );
                SfxStyleSheet*  pStyleSheet = pStylePool ? static_cast<SfxStyleSheet*>(pStylePool->
                                    Find( aPageStyle, SfxStyleFamily::Page )) : nullptr;

                if ( pStyleSheet )
                    rSet.Put( SfxTemplateItem( nSlotId, aPageStyle ) );
                else
                    rSet.Put( SfxTemplateItem( nSlotId, OUString() ) );
            }
            break;

            case SID_STYLE_WATERCAN:
            {
                rSet.Put( SfxBoolItem( nSlotId, SC_MOD()->GetIsWaterCan() ) );
            }
            break;

            case SID_STYLE_UPDATE_BY_EXAMPLE:
            {
                std::unique_ptr<SfxPoolItem> pItem;
                pTabViewShell->GetViewFrame()->GetBindings().QueryState(SID_STYLE_FAMILY, pItem);
                SfxUInt16Item* pFamilyItem = dynamic_cast<SfxUInt16Item*>(pItem.get());

                bool bPage = pFamilyItem && SfxStyleFamily::Page == SfxTemplate::NIdToSfxFamilyId(pFamilyItem->GetValue());

                if ( bProtected || bPage )
                    rSet.DisableItem( nSlotId );
            }
            break;

            case SID_STYLE_EDIT:
            case SID_STYLE_DELETE:
            case SID_STYLE_HIDE:
            case SID_STYLE_SHOW:
            {
                std::unique_ptr<SfxPoolItem> pItem;
                pTabViewShell->GetViewFrame()->GetBindings().QueryState(SID_STYLE_FAMILY, pItem);
                SfxUInt16Item* pFamilyItem = dynamic_cast<SfxUInt16Item*>(pItem.get());
                bool bPage = pFamilyItem && SfxStyleFamily::Page == SfxTemplate::NIdToSfxFamilyId(pFamilyItem->GetValue());

                if ( bProtected && !bPage )
                    rSet.DisableItem( nSlotId );
            }
            break;

            default:
                break;
        }

        nWhich = aIter.NextWhich();
    }
}

void ScFormatShell::ExecuteStyle( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const sal_uInt16  nSlotId = rReq.GetSlot();
    if ( !pArgs && nSlotId != SID_STYLE_NEW_BY_EXAMPLE && nSlotId != SID_STYLE_UPDATE_BY_EXAMPLE )
    {
        // in case of vertical toolbar
        pViewData->GetDispatcher().Execute( SID_STYLE_DESIGNER, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
        return;
    }

    SfxBindings&        rBindings   = pViewData->GetBindings();
    const SCTAB         nCurTab     = GetViewData()->GetTabNo();
    ScDocShell*         pDocSh      = GetViewData()->GetDocShell();
    ScTabViewShell*     pTabViewShell= GetViewData()->GetViewShell();
    ScDocument&         rDoc        = pDocSh->GetDocument();
    ScMarkData&         rMark       = GetViewData()->GetMarkData();
    ScModule*           pScMod      = SC_MOD();
    OUString            aRefName;
    bool                bUndo       = rDoc.IsUndoEnabled();
    SfxStyleSheetBasePool*  pStylePool  = rDoc.GetStyleSheetPool();

    if ( (nSlotId == SID_STYLE_PREVIEW)
        || (nSlotId ==  SID_STYLE_END_PREVIEW) )
    {
        if (nSlotId == SID_STYLE_PREVIEW)
        {
            SfxStyleFamily eFamily = SfxStyleFamily::Para;
            const SfxPoolItem* pFamItem;
            if ( pArgs && SfxItemState::SET == pArgs->GetItemState( SID_STYLE_FAMILY, true, &pFamItem ) )
                eFamily = (SfxStyleFamily) static_cast<const SfxUInt16Item*>(pFamItem)->GetValue();
            const SfxPoolItem* pNameItem;
            OUString aStyleName;
            if (pArgs && SfxItemState::SET == pArgs->GetItemState( nSlotId, true, &pNameItem ))
                aStyleName = static_cast<const SfxStringItem*>(pNameItem)->GetValue();
            if ( eFamily == SfxStyleFamily::Para ) // CellStyles
            {
                ScMarkData aFuncMark( pViewData->GetMarkData() );
                ScViewUtil::UnmarkFiltered( aFuncMark, &rDoc );
                aFuncMark.MarkToMulti();

                if ( !aFuncMark.IsMarked() && !aFuncMark.IsMultiMarked() )
                {
                    SCCOL nCol = pViewData->GetCurX();
                    SCROW nRow = pViewData->GetCurY();
                    SCTAB nTab = pViewData->GetTabNo();
                    ScRange aRange( nCol, nRow, nTab );
                    aFuncMark.SetMarkArea( aRange );
                }
                rDoc.SetPreviewSelection( aFuncMark );
                ScStyleSheet* pPreviewStyle = static_cast<ScStyleSheet*>( pStylePool->Find( aStyleName, eFamily ) );
                rDoc.SetPreviewCellStyle( pPreviewStyle  );
                ScPatternAttr aAttr( *rDoc.GetSelectionPattern( aFuncMark ) );
                aAttr.SetStyleSheet( pPreviewStyle );

                SfxItemSet aItemSet( GetPool() );

                ScPatternAttr aNewAttrs( GetViewData()->GetDocument()->GetPool() );
                SfxItemSet& rNewSet = aNewAttrs.GetItemSet();
                rNewSet.Put( aItemSet, false );

                rDoc.ApplySelectionPattern( aNewAttrs, rDoc.GetPreviewSelection() );
                pTabViewShell->UpdateSelectionArea( aFuncMark, &aAttr );
            }
        }
        else
        {
            // No mark at all happens when creating a new document, in which
            // case the selection pattern obtained would be empty (created of
            // GetPool()) anyway and nothing needs to be applied.
            ScMarkData aPreviewMark( rDoc.GetPreviewSelection());
            if (aPreviewMark.IsMarked() || aPreviewMark.IsMultiMarked())
            {
                ScPatternAttr aAttr( *rDoc.GetSelectionPattern( aPreviewMark ) );
                if ( ScStyleSheet* pPreviewStyle = rDoc.GetPreviewCellStyle() )
                    aAttr.SetStyleSheet( pPreviewStyle );
                rDoc.SetPreviewCellStyle(nullptr);

                SfxItemSet aItemSet( GetPool() );

                ScPatternAttr aNewAttrs( GetViewData()->GetDocument()->GetPool() );
                SfxItemSet& rNewSet = aNewAttrs.GetItemSet();
                rNewSet.Put( aItemSet, false );
                rDoc.ApplySelectionPattern( aNewAttrs, aPreviewMark );
                pTabViewShell->UpdateSelectionArea( aPreviewMark, &aAttr );
            }
        }
    }
    else if (   (nSlotId == SID_STYLE_NEW)
        || (nSlotId == SID_STYLE_EDIT)
        || (nSlotId == SID_STYLE_DELETE)
        || (nSlotId == SID_STYLE_HIDE)
        || (nSlotId == SID_STYLE_SHOW)
        || (nSlotId == SID_STYLE_APPLY)
        || (nSlotId == SID_STYLE_WATERCAN)
        || (nSlotId == SID_STYLE_FAMILY)
        || (nSlotId == SID_STYLE_NEW_BY_EXAMPLE)
        || (nSlotId == SID_STYLE_UPDATE_BY_EXAMPLE) )
    {
        SfxStyleSheetBase*      pStyleSheet = nullptr;

        bool bStyleToMarked = false;
        bool bListAction = false;
        bool bAddUndo = false;          // add ScUndoModifyStyle (style modified)
        ScStyleSaveData aOldData;       // for undo/redo
        ScStyleSaveData aNewData;

        SfxStyleFamily eFamily = SfxStyleFamily::Para;
        const SfxPoolItem* pFamItem;
        if ( pArgs && SfxItemState::SET == pArgs->GetItemState( SID_STYLE_FAMILY, true, &pFamItem ) )
            eFamily = (SfxStyleFamily) static_cast<const SfxUInt16Item*>(pFamItem)->GetValue();
        else if ( pArgs && SfxItemState::SET == pArgs->GetItemState( SID_STYLE_FAMILYNAME, true, &pFamItem ) )
        {
            OUString sFamily = static_cast<const SfxStringItem*>(pFamItem)->GetValue();
            if (sFamily == "CellStyles")
                eFamily = SfxStyleFamily::Para;
            else if (sFamily == "PageStyles")
                eFamily = SfxStyleFamily::Page;
        }

        OUString                aStyleName;
        sal_uInt16              nRetMask = 0xffff;

        pStylePool->SetSearchMask( eFamily );

        switch ( nSlotId )
        {
            case SID_STYLE_NEW:
                {
                    const SfxPoolItem* pNameItem;
                    if (pArgs && SfxItemState::SET == pArgs->GetItemState( nSlotId, true, &pNameItem ))
                        aStyleName  = static_cast<const SfxStringItem*>(pNameItem)->GetValue();

                    const SfxPoolItem* pRefItem=nullptr;
                    if (pArgs && SfxItemState::SET == pArgs->GetItemState( SID_STYLE_REFERENCE, true, &pRefItem ))
                    {
                        if(pRefItem!=nullptr)
                            aRefName  = static_cast<const SfxStringItem*>(pRefItem)->GetValue();
                    }

                    pStyleSheet = &(pStylePool->Make( aStyleName, eFamily,
                                                      SFXSTYLEBIT_USERDEF ) );

                    if ( pStyleSheet && pStyleSheet->HasParentSupport() )
                        pStyleSheet->SetParent(aRefName);
                }
                break;

            case SID_STYLE_APPLY:
            {
                const SfxStringItem* pNameItem = rReq.GetArg<SfxStringItem>(SID_APPLY_STYLE);
                const SfxStringItem* pFamilyItem = rReq.GetArg<SfxStringItem>(SID_STYLE_FAMILYNAME);
                if ( pFamilyItem && pNameItem )
                {
                    css::uno::Reference< css::style::XStyleFamiliesSupplier > xModel(pDocSh->GetModel(), css::uno::UNO_QUERY);
                    try
                    {
                        css::uno::Reference< css::container::XNameAccess > xStyles;
                        css::uno::Reference< css::container::XNameAccess > xCont = xModel->getStyleFamilies();
                        xCont->getByName(pFamilyItem->GetValue()) >>= xStyles;
                        css::uno::Reference< css::beans::XPropertySet > xInfo;
                        xStyles->getByName( pNameItem->GetValue() ) >>= xInfo;
                        OUString aUIName;
                        xInfo->getPropertyValue("DisplayName") >>= aUIName;
                        if ( !aUIName.isEmpty() )
                            rReq.AppendItem( SfxStringItem( SID_STYLE_APPLY, aUIName ) );
                    }
                    catch( css::uno::Exception& )
                    {
                    }
                }
                SAL_FALLTHROUGH;
            }
            case SID_STYLE_EDIT:
            case SID_STYLE_DELETE:
            case SID_STYLE_HIDE:
            case SID_STYLE_SHOW:
            case SID_STYLE_NEW_BY_EXAMPLE:
                {
                    const SfxPoolItem* pNameItem;
                    if (pArgs && SfxItemState::SET == pArgs->GetItemState( nSlotId, true, &pNameItem ))
                        aStyleName = static_cast<const SfxStringItem*>(pNameItem)->GetValue();
                    else if ( nSlotId == SID_STYLE_NEW_BY_EXAMPLE )
                    {
                        ScopedVclPtrInstance<SfxNewStyleDlg> pDlg( nullptr, *pStylePool );
                        if ( RET_OK != pDlg->Execute() )
                            return;
                        aStyleName = pDlg->GetName();
                    }

                    pStyleSheet = pStylePool->Find( aStyleName, eFamily );

                    aOldData.InitFromStyle( pStyleSheet );
                }
                break;

            case SID_STYLE_WATERCAN:
            {
                bool bWaterCan = pScMod->GetIsWaterCan();

                if( !bWaterCan )
                {
                    const SfxPoolItem* pItem;

                    if ( SfxItemState::SET ==
                         pArgs->GetItemState( nSlotId, true, &pItem ) )
                    {
                        const SfxStringItem* pStrItem = dynamic_cast< const SfxStringItem *>( pItem );
                        if ( pStrItem )
                        {
                            aStyleName  = pStrItem->GetValue();
                            pStyleSheet = pStylePool->Find( aStyleName, eFamily );

                            if ( pStyleSheet )
                            {
                                static_cast<ScStyleSheetPool*>(pStylePool)->
                                        SetActualStyleSheet( pStyleSheet );
                                rReq.Done();
                            }
                        }
                    }
                }

                if ( !bWaterCan && pStyleSheet )
                {
                    pScMod->SetWaterCan( true );
                    pTabViewShell->SetActivePointer( Pointer(PointerStyle::Fill) );
                    rReq.Done();
                }
                else
                {
                    pScMod->SetWaterCan( false );
                    pTabViewShell->SetActivePointer( Pointer(PointerStyle::Arrow) );
                    rReq.Done();
                }
            }
            break;

            default:
                break;
        }

        // set new style for paintbrush format mode
        if ( nSlotId == SID_STYLE_APPLY && pScMod->GetIsWaterCan() && pStyleSheet )
            static_cast<ScStyleSheetPool*>(pStylePool)->SetActualStyleSheet( pStyleSheet );

        switch ( eFamily )
        {
            case SfxStyleFamily::Para:
            {
                switch ( nSlotId )
                {
                    case SID_STYLE_DELETE:
                    {
                        if ( pStyleSheet )
                        {
                            pTabViewShell->RemoveStyleSheetInUse( pStyleSheet );
                            pStylePool->Remove( pStyleSheet );
                            pTabViewShell->InvalidateAttribs();
                            nRetMask = sal_uInt16(true);
                            bAddUndo = true;
                            rReq.Done();
                        }
                        else
                            nRetMask = sal_uInt16(false);
                    }
                    break;

                    case SID_STYLE_HIDE:
                    case SID_STYLE_SHOW:
                    {
                        if ( pStyleSheet )
                        {
                            pStyleSheet->SetHidden( nSlotId == SID_STYLE_HIDE );
                            pTabViewShell->InvalidateAttribs();
                            rReq.Done();
                        }
                        else
                            nRetMask = sal_uInt16(false);
                    }
                    break;

                    case SID_STYLE_APPLY:
                    {
                        if ( pStyleSheet && !pScMod->GetIsWaterCan() )
                        {
                            // apply style sheet to document
                            pTabViewShell->SetStyleSheetToMarked( static_cast<SfxStyleSheet*>(pStyleSheet) );
                            pTabViewShell->InvalidateAttribs();
                            rReq.Done();
                        }
                    }
                    break;

                    case SID_STYLE_NEW_BY_EXAMPLE:
                    case SID_STYLE_UPDATE_BY_EXAMPLE:
                    {
                        // create/replace style sheet by attributes
                        // at cursor position:

                        const ScPatternAttr* pAttrItem = nullptr;

                        // The query if marked, was always wrong here,
                        // so now no more, and just from the cursor.
                        // If attributes are to be removed from the selection, still need to be
                        // cautious not to adopt items from templates
                        // (GetSelectionPattern also collects items from originals) (# 44748 #)
                        SCCOL       nCol = pViewData->GetCurX();
                        SCROW       nRow = pViewData->GetCurY();
                        pAttrItem = rDoc.GetPattern( nCol, nRow, nCurTab );

                        SfxItemSet aAttrSet = pAttrItem->GetItemSet();
                        aAttrSet.ClearItem( ATTR_MERGE );
                        aAttrSet.ClearItem( ATTR_MERGE_FLAG );

                        // Do not adopt conditional formatting and validity,
                        // because they can not be edited in the template
                        aAttrSet.ClearItem( ATTR_VALIDDATA );
                        aAttrSet.ClearItem( ATTR_CONDITIONAL );

                        if ( SID_STYLE_NEW_BY_EXAMPLE == nSlotId )
                        {
                            if ( bUndo )
                            {
                                OUString aUndo = ScGlobal::GetRscString( STR_UNDO_EDITCELLSTYLE );
                                pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
                                bListAction = true;
                            }

                            bool bConvertBack = false;
                            SfxStyleSheet*  pSheetInUse = const_cast<SfxStyleSheet*>(
                                                          pTabViewShell->GetStyleSheetFromMarked());

                            // when a new style is present and is used in the selection,
                            // then the parent can not be adopted:
                            if ( pStyleSheet && pSheetInUse && pStyleSheet == pSheetInUse )
                                pSheetInUse = nullptr;

                            // if already present, first remove ...
                            if ( pStyleSheet )
                            {
                                // style pointer to names before erase,
                                // otherwise cells will get invalid pointer
                                //!!! As it happens, a method that does it for a particular style
                                rDoc.StylesToNames();
                                bConvertBack = true;
                                pStylePool->Remove(pStyleSheet);
                            }

                            // ...and create new
                            pStyleSheet = &pStylePool->Make( aStyleName, eFamily,
                                                             SFXSTYLEBIT_USERDEF );

                            // when a style is present, then this will become
                            // the parent of the new style:
                            if ( pSheetInUse && pStyleSheet->HasParentSupport() )
                                pStyleSheet->SetParent( pSheetInUse->GetName() );

                            if ( bConvertBack )
                                // Name to style pointer
                                rDoc.UpdStlShtPtrsFrmNms();
                            else
                                rDoc.GetPool()->CellStyleCreated( aStyleName, &rDoc );

                            // Adopt attribute and use style
                            pStyleSheet->GetItemSet().Put( aAttrSet );
                            pTabViewShell->UpdateStyleSheetInUse( pStyleSheet );

                            //  call SetStyleSheetToMarked after adding the ScUndoModifyStyle
                            //  (pStyleSheet pointer is used!)
                            bStyleToMarked = true;
                        }
                        else // ( nSlotId == SID_STYLE_UPDATE_BY_EXAMPLE )
                        {
                            pStyleSheet = const_cast<SfxStyleSheet*>(pTabViewShell->GetStyleSheetFromMarked());

                            if ( pStyleSheet )
                            {
                                aOldData.InitFromStyle( pStyleSheet );

                                if ( bUndo )
                                {
                                    OUString aUndo = ScGlobal::GetRscString( STR_UNDO_EDITCELLSTYLE );
                                    pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
                                    bListAction = true;
                                }

                                pStyleSheet->GetItemSet().Put( aAttrSet );
                                pTabViewShell->UpdateStyleSheetInUse( pStyleSheet );

                                //  call SetStyleSheetToMarked after adding the ScUndoModifyStyle
                                //  (pStyleSheet pointer is used!)
                                bStyleToMarked = true;
                            }
                        }

                        aNewData.InitFromStyle( pStyleSheet );
                        bAddUndo = true;
                        rReq.Done();
                    }
                    break;

                    default:
                        break;
                }
            } // case SfxStyleFamily::Para:
            break;

            case SfxStyleFamily::Page:
            {
                switch ( nSlotId )
                {
                    case SID_STYLE_DELETE:
                    {
                        nRetMask = sal_uInt16( nullptr != pStyleSheet );
                        if ( pStyleSheet )
                        {
                            if ( rDoc.RemovePageStyleInUse( pStyleSheet->GetName() ) )
                            {
                                ScPrintFunc( pDocSh, pTabViewShell->GetPrinter(true), nCurTab ).UpdatePages();
                                rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                                rBindings.Invalidate( FID_RESET_PRINTZOOM );
                            }
                            pStylePool->Remove( pStyleSheet );
                            rBindings.Invalidate( SID_STYLE_FAMILY4 );
                            pDocSh->SetDocumentModified();
                            bAddUndo = true;
                            rReq.Done();
                        }
                    }
                    break;

                    case SID_STYLE_HIDE:
                    case SID_STYLE_SHOW:
                    {
                        nRetMask = sal_uInt16( nullptr != pStyleSheet );
                        if ( pStyleSheet )
                        {
                            pStyleSheet->SetHidden( nSlotId == SID_STYLE_HIDE );
                            rBindings.Invalidate( SID_STYLE_FAMILY4 );
                            pDocSh->SetDocumentModified();
                            rReq.Done();
                        }
                    }
                    break;

                    case SID_STYLE_APPLY:
                    {
                        nRetMask = sal_uInt16( nullptr != pStyleSheet );
                        if ( pStyleSheet && !pScMod->GetIsWaterCan() )
                        {
                            ScUndoApplyPageStyle* pUndoAction = nullptr;
                            SCTAB nTabCount = rDoc.GetTableCount();
                            ScMarkData::iterator itr = rMark.begin(), itrEnd = rMark.end();
                            for (; itr != itrEnd && *itr < nTabCount; ++itr)
                            {
                                OUString aOldName = rDoc.GetPageStyle( *itr );
                                if ( aOldName != aStyleName )
                                {
                                    rDoc.SetPageStyle( *itr, aStyleName );
                                    ScPrintFunc( pDocSh, pTabViewShell->GetPrinter(true), *itr ).UpdatePages();
                                    if( !pUndoAction )
                                        pUndoAction = new ScUndoApplyPageStyle( pDocSh, aStyleName );
                                    pUndoAction->AddSheetAction( *itr, aOldName );
                                }
                            }
                            if( pUndoAction )
                            {
                                pDocSh->GetUndoManager()->AddUndoAction( pUndoAction );
                                pDocSh->SetDocumentModified();
                                rBindings.Invalidate( SID_STYLE_FAMILY4 );
                                rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                                rBindings.Invalidate( FID_RESET_PRINTZOOM );
                            }
                            rReq.Done();
                        }
                    }
                    break;

                    case SID_STYLE_NEW_BY_EXAMPLE:
                    {
                        const OUString& rStrCurStyle = rDoc.GetPageStyle( nCurTab );

                        if ( rStrCurStyle != aStyleName )
                        {
                            SfxStyleSheetBase*  pCurStyle = pStylePool->Find( rStrCurStyle, eFamily );
                            SfxItemSet          aAttrSet  = pCurStyle->GetItemSet();
                            SCTAB               nInTab;
                            bool                bUsed = rDoc.IsPageStyleInUse( aStyleName, &nInTab );

                            // if already present, first remove...
                            if ( pStyleSheet )
                                pStylePool->Remove( pStyleSheet );

                            // ...and create new
                            pStyleSheet = &pStylePool->Make( aStyleName, eFamily,
                                                             SFXSTYLEBIT_USERDEF );

                            // Adopt attribute
                            pStyleSheet->GetItemSet().Put( aAttrSet );
                            pDocSh->SetDocumentModified();

                            // If being used -> Update
                            if ( bUsed )
                                ScPrintFunc( pDocSh, pTabViewShell->GetPrinter(true), nInTab ).UpdatePages();

                            aNewData.InitFromStyle( pStyleSheet );
                            bAddUndo = true;
                            rReq.Done();
                            nRetMask = sal_uInt16(true);
                        }
                    }
                    break;

                    default:
                        break;
                } // switch ( nSlotId )
            } // case SfxStyleFamily::Page:
            break;

            default:
                break;
        } // switch ( eFamily )

        // create new or process through Dialog:
        if ( nSlotId == SID_STYLE_NEW || nSlotId == SID_STYLE_EDIT )
        {
            if ( pStyleSheet )
            {
                SfxStyleFamily  eFam    = pStyleSheet->GetFamily();
                std::unique_ptr<SfxAbstractTabDialog> pDlg;
                sal_uInt16          nRsc    = 0;

                // Store old Items from the style
                SfxItemSet aOldSet = pStyleSheet->GetItemSet();
                OUString aOldName = pStyleSheet->GetName();

                switch ( eFam )
                {
                    case SfxStyleFamily::Page:
                        nRsc = RID_SCDLG_STYLES_PAGE;
                        break;

                    case SfxStyleFamily::Para:
                    default:
                        {
                            SfxItemSet& rSet = pStyleSheet->GetItemSet();

                            const SfxPoolItem* pItem;
                            if ( rSet.GetItemState( ATTR_VALUE_FORMAT,
                                    false, &pItem ) == SfxItemState::SET )
                            {
                                // Produce and format NumberFormat Value from Value and Language
                                sal_uLong nFormat =
                                    static_cast<const SfxUInt32Item*>(pItem)->GetValue();
                                LanguageType eLang =
                                    static_cast<const SvxLanguageItem*>(&rSet.Get(
                                    ATTR_LANGUAGE_FORMAT ))->GetLanguage();
                                sal_uLong nLangFormat = rDoc.GetFormatTable()->
                                    GetFormatForLanguageIfBuiltIn( nFormat, eLang );
                                if ( nLangFormat != nFormat )
                                {
                                    SfxUInt32Item aNewItem( ATTR_VALUE_FORMAT, nLangFormat );
                                    rSet.Put( aNewItem );
                                    aOldSet.Put( aNewItem );
                                    // Also in aOldSet for comparison after the  dialog,
                                    // Otherwise might miss a language change
                                }
                            }

                            std::unique_ptr<SvxNumberInfoItem> pNumberInfoItem(
                                ScTabViewShell::MakeNumberInfoItem(&rDoc, GetViewData()));

                            pDocSh->PutItem( *pNumberInfoItem );
                            nRsc = RID_SCDLG_STYLES_PAR;

                            // Definitely a SvxBoxInfoItem with Table = sal_False in set:
                            // (If there is no item, the dialogue will also delete the
                            // BORDER_OUTER SvxBoxItem from the Template Set)
                            if ( rSet.GetItemState( ATTR_BORDER_INNER, false ) != SfxItemState::SET )
                            {
                                SvxBoxInfoItem aBoxInfoItem( ATTR_BORDER_INNER );
                                aBoxInfoItem.SetTable(false);       // no inner lines
                                aBoxInfoItem.SetDist(true);
                                aBoxInfoItem.SetMinDist(false);
                                rSet.Put( aBoxInfoItem );
                            }
                        }
                        break;
                }

                //  If GetDefDialogParent is a dialog, it must be used
                //  (style catalog)

                vcl::Window* pParent = Application::GetDefDialogParent();
                if ( !pParent || !pParent->IsDialog() )
                {
                    //  GetDefDialogParent dynamically finds the
                    //  topmost parent of the focus window, so IsDialog above is FALSE
                    //  even if called from the style catalog.
                    //  -> Use NULL if a modal dialog is open, to enable the Dialog's
                    //  default parent handling.
                    if ( Application::IsInModalMode() )
                        pParent = nullptr;
                    else
                        pParent = pTabViewShell->GetDialogParent();
                }

                pTabViewShell->SetInFormatDialog(true);

                ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

                pDlg.reset(pFact->CreateScStyleDlg( pParent, *pStyleSheet, nRsc, nRsc ));
                OSL_ENSURE(pDlg, "Dialog create fail!");
                short nResult = pDlg->Execute();
                pTabViewShell->SetInFormatDialog(false);

                if ( nResult == RET_OK )
                {
                    const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

                    if ( pOutSet )
                    {
                        nRetMask = pStyleSheet->GetMask();

                        // Attribute comparisons (earlier in ModifyStyleSheet) now here
                        // with the old values (the style is already changed)
                        if ( SfxStyleFamily::Para == eFam )
                        {
                            SfxItemSet& rNewSet = pStyleSheet->GetItemSet();
                            bool bNumFormatChanged;
                            if ( ScGlobal::CheckWidthInvalidate(
                                                bNumFormatChanged, rNewSet, aOldSet ) )
                                rDoc.InvalidateTextWidth( nullptr, nullptr, bNumFormatChanged );

                            SCTAB nTabCount = rDoc.GetTableCount();
                            for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                                if (rDoc.IsStreamValid(nTab))
                                    rDoc.SetStreamValid(nTab, false);

                            sal_uLong nOldFormat = static_cast<const SfxUInt32Item&>(aOldSet.
                                                    Get( ATTR_VALUE_FORMAT )).GetValue();
                            sal_uLong nNewFormat = static_cast<const SfxUInt32Item&>(rNewSet.
                                                    Get( ATTR_VALUE_FORMAT )).GetValue();
                            if ( nNewFormat != nOldFormat )
                            {
                                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                                const SvNumberformat* pOld = pFormatter->GetEntry( nOldFormat );
                                const SvNumberformat* pNew = pFormatter->GetEntry( nNewFormat );
                                if ( pOld && pNew && pOld->GetLanguage() != pNew->GetLanguage() )
                                    rNewSet.Put( SvxLanguageItem(
                                                    pNew->GetLanguage(), ATTR_LANGUAGE_FORMAT ) );
                            }

                            rDoc.GetPool()->CellStyleCreated( pStyleSheet->GetName(), &rDoc );
                        }
                        else
                        {
                            //! Here also queries for Page Styles

                            OUString aNewName = pStyleSheet->GetName();
                            if ( aNewName != aOldName &&
                                    rDoc.RenamePageStyleInUse( aOldName, aNewName ) )
                            {
                                rBindings.Invalidate( SID_STATUS_PAGESTYLE );
                                rBindings.Invalidate( FID_RESET_PRINTZOOM );
                            }

                            rDoc.ModifyStyleSheet( *pStyleSheet, *pOutSet );
                            rBindings.Invalidate( FID_RESET_PRINTZOOM );
                        }

                        pDocSh->SetDocumentModified();

                        if ( SfxStyleFamily::Para == eFam )
                        {
                            ScTabViewShell::UpdateNumberFormatter(
                                static_cast<const SvxNumberInfoItem&>(
                                    *(pDocSh->GetItem(SID_ATTR_NUMBERFORMAT_INFO)) ));

                            pTabViewShell->UpdateStyleSheetInUse( pStyleSheet );
                            pTabViewShell->InvalidateAttribs();
                        }

                        aNewData.InitFromStyle( pStyleSheet );
                        bAddUndo = true;
                    }
                }
                else
                {
                    if ( nSlotId == SID_STYLE_NEW )
                        pStylePool->Remove( pStyleSheet );
                    else
                    {
                        // If in the mean time something was painted with the
                        // temporary changed item set
                        pDocSh->PostPaintGridAll();
                    }
                }
            }
        }

            rReq.SetReturnValue( SfxUInt16Item( nSlotId, nRetMask ) );

        if ( bAddUndo && bUndo)
            pDocSh->GetUndoManager()->AddUndoAction(
                        new ScUndoModifyStyle( pDocSh, eFamily, aOldData, aNewData ) );

        if ( bStyleToMarked )
        {
            //  call SetStyleSheetToMarked after adding the ScUndoModifyStyle,
            //  so redo will find the modified style
            pTabViewShell->SetStyleSheetToMarked( static_cast<SfxStyleSheet*>(pStyleSheet) );
            pTabViewShell->InvalidateAttribs();
        }

        if ( bListAction )
            pDocSh->GetUndoManager()->LeaveListAction();
    }
    else if (nSlotId == SID_CLASSIFICATION_APPLY)
    {
        const SfxPoolItem* pItem = nullptr;
        if (pArgs && pArgs->GetItemState(nSlotId, false, &pItem) == SfxItemState::SET)
        {
            const OUString& rName = static_cast<const SfxStringItem*>(pItem)->GetValue();
            SfxClassificationHelper aHelper(pDocSh->getDocProperties());
            auto eType = SfxClassificationPolicyType::IntellectualProperty;
            if (pArgs->GetItemState(SID_TYPE_NAME, false, &pItem) == SfxItemState::SET)
            {
                const OUString& rType = static_cast<const SfxStringItem*>(pItem)->GetValue();
                eType = SfxClassificationHelper::stringToPolicyType(rType);
            }
            aHelper.SetBACName(rName, eType);
        }
        else
            SAL_WARN("sc.ui", "missing parameter for SID_CLASSIFICATION_APPLY");
    }
    else
    {
        OSL_FAIL( "Unknown slot (ScViewShell::ExecuteStyle)" );
    }
}

void ScFormatShell::ExecuteNumFormat( SfxRequest& rReq )
{
    ScModule*           pScMod          = SC_MOD();
    ScTabViewShell*     pTabViewShell   = GetViewData()->GetViewShell();
    const SfxItemSet*   pReqArgs        = rReq.GetArgs();
    sal_uInt16          nSlot           = rReq.GetSlot();
    SfxBindings&        rBindings       = pTabViewShell->GetViewFrame()->GetBindings();

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox

    // End input
    if ( GetViewData()->HasEditView( GetViewData()->GetActivePart() ) )
    {
        switch ( nSlot )
        {
            case SID_NUMBER_TWODEC:
            case SID_NUMBER_SCIENTIFIC:
            case SID_NUMBER_DATE:
            case SID_NUMBER_CURRENCY:
            case SID_NUMBER_PERCENT:
            case SID_NUMBER_STANDARD:
            case SID_NUMBER_FORMAT:
            case SID_NUMBER_INCDEC:
            case SID_NUMBER_DECDEC:
            case FID_DEFINE_NAME:
            case FID_ADD_NAME:
            case FID_USE_NAME:
            case FID_INSERT_NAME:
            case SID_SPELL_DIALOG:
            case SID_HANGUL_HANJA_CONVERSION:

            pScMod->InputEnterHandler();
            pTabViewShell->UpdateInputHandler();
            break;

            default:
            break;
        }
    }

    short nType = GetCurrentNumberFormatType();
    SfxItemSet aSet( GetPool(), nSlot, nSlot );
    switch ( nSlot )
    {
        case SID_NUMBER_TWODEC:
            pTabViewShell->SetNumberFormat( css::util::NumberFormat::NUMBER, 4 );       // Standard+4 = #.##0,00
            rReq.Done();
            break;
        case SID_NUMBER_SCIENTIFIC:
            if ((nType & css::util::NumberFormat::SCIENTIFIC))
                pTabViewShell->SetNumberFormat( css::util::NumberFormat::NUMBER );
            else
                pTabViewShell->SetNumberFormat( css::util::NumberFormat::SCIENTIFIC );
            aSet.Put( SfxBoolItem(nSlot, !(nType & css::util::NumberFormat::SCIENTIFIC)) );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_DATE:
            if ((nType & css::util::NumberFormat::DATE))
                pTabViewShell->SetNumberFormat( css::util::NumberFormat::NUMBER );
            else
                pTabViewShell->SetNumberFormat( css::util::NumberFormat::DATE );
            aSet.Put( SfxBoolItem(nSlot, !(nType & css::util::NumberFormat::DATE)) );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_TIME:
            if ((nType & css::util::NumberFormat::TIME))
                pTabViewShell->SetNumberFormat( css::util::NumberFormat::NUMBER );
            else
                pTabViewShell->SetNumberFormat( css::util::NumberFormat::TIME );
            aSet.Put( SfxBoolItem(nSlot, !(nType & css::util::NumberFormat::TIME)) );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_CURRENCY:
            if(pReqArgs)
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->HasItem( SID_NUMBER_CURRENCY, &pItem ) )
                {
                    sal_uInt32 nNewFormat = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
                    ScDocument* pDoc = pViewData->GetDocument();
                    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                    const SfxItemSet& rOldSet = pTabViewShell->GetSelectionPattern()->GetItemSet();

                    LanguageType eOldLang = static_cast<const SvxLanguageItem&>(
                                            rOldSet.Get( ATTR_LANGUAGE_FORMAT ) ).GetLanguage();
                    sal_uInt32 nOldFormat = static_cast<const SfxUInt32Item&>(
                                            rOldSet.Get( ATTR_VALUE_FORMAT ) ).GetValue();

                    if ( nOldFormat != nNewFormat )
                    {
                        const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewFormat );
                        ScPatternAttr aNewAttrs( pDoc->GetPool() );
                        SfxItemSet& rSet = aNewAttrs.GetItemSet();
                        LanguageType eNewLang = pNewEntry ? pNewEntry->GetLanguage() : LANGUAGE_DONTKNOW;
                        if ( eNewLang != eOldLang && eNewLang != LANGUAGE_DONTKNOW )
                            rSet.Put( SvxLanguageItem( eNewLang, ATTR_LANGUAGE_FORMAT ) );
                        rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );
                        pTabViewShell->ApplySelectionPattern( aNewAttrs );
                    }
                    else
                        pTabViewShell->SetNumberFormat( css::util::NumberFormat::NUMBER );
                }
            }
            else
            {
                if ( ( nType & css::util::NumberFormat::CURRENCY ) )
                    pTabViewShell->SetNumberFormat( css::util::NumberFormat::NUMBER );
                else
                    pTabViewShell->SetNumberFormat( css::util::NumberFormat::CURRENCY );
            }
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_PERCENT:
            if ((nType & css::util::NumberFormat::PERCENT))
                pTabViewShell->SetNumberFormat( css::util::NumberFormat::NUMBER );
            else
                pTabViewShell->SetNumberFormat( css::util::NumberFormat::PERCENT );
            aSet.Put( SfxBoolItem(nSlot, !(nType & css::util::NumberFormat::PERCENT)) );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_STANDARD:
            pTabViewShell->SetNumberFormat( css::util::NumberFormat::NUMBER );
            rReq.Done();
            break;
        case SID_NUMBER_INCDEC:
            pTabViewShell->ChangeNumFmtDecimals( true );
            rReq.Done();
            break;
        case SID_NUMBER_DECDEC:
            pTabViewShell->ChangeNumFmtDecimals( false );
            rReq.Done();
            break;

        case SID_NUMBER_FORMAT:
            // symphony version with format interpretation
            if(pReqArgs)
            {
                const SfxPoolItem* pItem;
                ScDocument* pDoc = pViewData->GetDocument();
                SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                LanguageType eLanguage = ScGlobal::eLnge;
                sal_Int16 eType = -1;
                sal_uInt32 nCurrentNumberFormat;

                pDoc->GetNumberFormat(pViewData->GetCurX(), pViewData->GetCurY(), pViewData->GetTabNo(), nCurrentNumberFormat);
                const SvNumberformat* pEntry = pFormatter->GetEntry(nCurrentNumberFormat);

                if(pEntry)
                {
                    eLanguage = pEntry->GetLanguage();
                    eType = pEntry->GetType();
                }

                //Just use eType to judge whether the command is fired for NUMBER/PERCENT/CURRENCY/SCIENTIFIC/FRACTION
                //In sidebar, users can fire SID_NUMBER_FORMAT command by operating the related UI controls before they are disable
                switch(eType)
                {
                case css::util::NumberFormat::ALL:
                case css::util::NumberFormat::NUMBER:
                case css::util::NumberFormat::NUMBER| css::util::NumberFormat::DEFINED:
                case css::util::NumberFormat::PERCENT:
                case css::util::NumberFormat::PERCENT| css::util::NumberFormat::DEFINED:
                case css::util::NumberFormat::CURRENCY:
                case css::util::NumberFormat::CURRENCY|css::util::NumberFormat::DEFINED:
                case css::util::NumberFormat::SCIENTIFIC:
                case css::util::NumberFormat::SCIENTIFIC|css::util::NumberFormat::DEFINED:
                case css::util::NumberFormat::FRACTION:
                case css::util::NumberFormat::FRACTION|css::util::NumberFormat::DEFINED:
                    eType = 0;
                    break;
                default:
                    eType =-1;
                }

                if(SfxItemState::SET == pReqArgs->GetItemState(nSlot, true, &pItem) && eType != -1)
                {
                    OUString aCode = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    sal_uInt16 aLen = aCode.getLength();
                    OUString* sFormat = new OUString[4];
                    OUString sTmpStr = "";
                    sal_uInt16 nCount(0);
                    sal_uInt16 nStrCount(0);

                    while(nCount < aLen)
                    {
                        sal_Unicode cChar = aCode[nCount];

                        if(cChar == ',')
                        {
                            sFormat[nStrCount] = sTmpStr;
                            sTmpStr.clear();
                            nStrCount++;
                        }
                        else
                        {
                            sTmpStr += OUString(cChar);
                        }

                        nCount++;

                        if(nStrCount > 3)
                            break;
                    }

                    const bool bThousand = (bool)sFormat[0].toInt32();
                    const bool bNegRed = (bool)sFormat[1].toInt32();
                    const sal_uInt16 nPrecision = (sal_uInt16)sFormat[2].toInt32();
                    const sal_uInt16 nLeadZeroes = (sal_uInt16)sFormat[3].toInt32();

                    aCode = pFormatter->GenerateFormat(
                        nCurrentNumberFormat,//modify
                        eLanguage,
                        bThousand,
                        bNegRed,
                        nPrecision,
                        nLeadZeroes);
                    pTabViewShell->SetNumFmtByStr(aCode);
                    delete[] sFormat;
                }
            }
            break;

        case SID_ATTR_NUMBERFORMAT_VALUE:
            if ( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( ATTR_VALUE_FORMAT, true, &pItem ) == SfxItemState::SET )
                {
                    // We have to accomplish this using ApplyAttributes()
                    // because we also need the language information to be
                    // considered.
                    const SfxItemSet& rOldSet =
                        pTabViewShell->GetSelectionPattern()->GetItemSet();
                    SfxItemPool* pDocPool = GetViewData()->GetDocument()->GetPool();
                    SfxItemSet aNewSet( *pDocPool, ATTR_PATTERN_START, ATTR_PATTERN_END );
                    aNewSet.Put( *pItem );
                    pTabViewShell->ApplyAttributes( &aNewSet, &rOldSet );
                }
            }
            break;

        case SID_NUMBER_TYPE_FORMAT:
            if ( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
                {
                    sal_uInt16 nFormat = static_cast<const SfxInt16Item *>(pItem)->GetValue();
                    switch(nFormat)
                    {
                    case 0:
                        pTabViewShell->SetNumberFormat( css::util::NumberFormat::NUMBER); //Modify
                        break;
                    case 1:
                        pTabViewShell->SetNumberFormat( css::util::NumberFormat::NUMBER, 2 ); //Modify
                        break;
                    case 2:
                        pTabViewShell->SetNumberFormat( css::util::NumberFormat::PERCENT );
                        break;
                    case 3:
                        pTabViewShell->SetNumberFormat( css::util::NumberFormat::CURRENCY );
                        break;
                    case 4:
                        pTabViewShell->SetNumberFormat( css::util::NumberFormat::DATE );
                        break;
                    case 5:
                        pTabViewShell->SetNumberFormat( css::util::NumberFormat::TIME );
                        break;
                    case 6:
                        pTabViewShell->SetNumberFormat( css::util::NumberFormat::SCIENTIFIC );
                        break;
                    case 7:
                        pTabViewShell->SetNumberFormat( css::util::NumberFormat::FRACTION );
                        break;
                    case 8:
                        pTabViewShell->SetNumberFormat( css::util::NumberFormat::LOGICAL );
                        break;
                    case 9:
                        pTabViewShell->SetNumberFormat( css::util::NumberFormat::TEXT );
                        break;
                    default:
                        ;
                    }
                    rReq.Done();
                }
            }
            break;

        default:
            OSL_FAIL("ExecuteEdit: invalid slot");
            break;
    }
}

void ScFormatShell::ExecuteAlignment( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell       = GetViewData()->GetViewShell();
    SfxBindings&            rBindings   = pViewData->GetBindings();
    const SfxItemSet*       pSet        = rReq.GetArgs();
    sal_uInt16                  nSlot       = rReq.GetSlot();

    pTabViewShell->HideListBox();   // Autofilter-DropDown-Listbox

    switch( nSlot )
    {
        // pseudo slots for Format menu
        case SID_ALIGN_ANY_HDEFAULT:
        case SID_ALIGN_ANY_LEFT:
        case SID_ALIGN_ANY_HCENTER:
        case SID_ALIGN_ANY_RIGHT:
        case SID_ALIGN_ANY_JUSTIFIED:
            pTabViewShell->ApplyAttr( SvxHorJustifyItem( lclConvertSlotToHAlign( nSlot ), ATTR_HOR_JUSTIFY ) );
        break;
        case SID_ALIGN_ANY_VDEFAULT:
        case SID_ALIGN_ANY_TOP:
        case SID_ALIGN_ANY_VCENTER:
        case SID_ALIGN_ANY_BOTTOM:
            pTabViewShell->ApplyAttr( SvxVerJustifyItem( lclConvertSlotToVAlign( nSlot ), ATTR_VER_JUSTIFY ) );
        break;

        default:
            if( pSet )
            {
                const SfxPoolItem* pItem = nullptr;
                if( pSet->GetItemState(GetPool().GetWhich(nSlot), true, &pItem  ) == SfxItemState::SET )
                {

                    switch ( nSlot )
                    {
                        case SID_ATTR_ALIGN_HOR_JUSTIFY:
                        case SID_ATTR_ALIGN_VER_JUSTIFY:
                        case SID_ATTR_ALIGN_INDENT:
                        case SID_ATTR_ALIGN_HYPHENATION:
                        case SID_ATTR_ALIGN_DEGREES:
                        case SID_ATTR_ALIGN_LOCKPOS:
                        case SID_ATTR_ALIGN_MARGIN:
                        case SID_ATTR_ALIGN_STACKED:
                            pTabViewShell->ApplyAttr( *pItem );
                        break;

                        case SID_H_ALIGNCELL:
                        {
                            SvxCellHorJustify eJust = (SvxCellHorJustify)static_cast<const SvxHorJustifyItem*>(pItem)->GetValue();
                            // #i78476# update alignment of text in cell edit mode
                            pTabViewShell->UpdateInputHandlerCellAdjust( eJust );
                            pTabViewShell->ApplyAttr( SvxHorJustifyItem( eJust, ATTR_HOR_JUSTIFY ) );
                        }
                        break;
                        case SID_V_ALIGNCELL:
                            pTabViewShell->ApplyAttr( SvxVerJustifyItem( (SvxCellVerJustify)static_cast<const SvxVerJustifyItem*>(pItem)->GetValue(), ATTR_VER_JUSTIFY ) );
                        break;
                        default:
                            OSL_FAIL( "ExecuteAlignment: invalid slot" );
                            return;
                    }
                }
            }
    }
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_LEFT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_RIGHT );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_BLOCK );
    rBindings.Invalidate( SID_ATTR_PARA_ADJUST_CENTER);
    rBindings.Invalidate( SID_ALIGNLEFT );
    rBindings.Invalidate( SID_ALIGNRIGHT );
    rBindings.Invalidate( SID_ALIGNCENTERHOR );
    rBindings.Invalidate( SID_ALIGNBLOCK );
    rBindings.Invalidate( SID_ALIGNTOP );
    rBindings.Invalidate( SID_ALIGNBOTTOM );
    rBindings.Invalidate( SID_ALIGNCENTERVER );
    rBindings.Invalidate( SID_V_ALIGNCELL );
    rBindings.Invalidate( SID_H_ALIGNCELL );
    // pseudo slots for Format menu
    rBindings.Invalidate( SID_ALIGN_ANY_HDEFAULT );
    rBindings.Invalidate( SID_ALIGN_ANY_LEFT );
    rBindings.Invalidate( SID_ALIGN_ANY_HCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_RIGHT );
    rBindings.Invalidate( SID_ALIGN_ANY_JUSTIFIED );
    rBindings.Invalidate( SID_ALIGN_ANY_VDEFAULT );
    rBindings.Invalidate( SID_ALIGN_ANY_TOP );
    rBindings.Invalidate( SID_ALIGN_ANY_VCENTER );
    rBindings.Invalidate( SID_ALIGN_ANY_BOTTOM );
    rBindings.Update();

    if( ! rReq.IsAPI() )
        rReq.Done();
}

void ScFormatShell::ExecuteTextAttr( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell       = GetViewData()->GetViewShell();
    SfxBindings&            rBindings   = pViewData->GetBindings();
    const ScPatternAttr*    pAttrs      = pTabViewShell->GetSelectionPattern();
    const SfxItemSet*       pSet        = rReq.GetArgs();
    sal_uInt16                  nSlot       = rReq.GetSlot();
    SfxAllItemSet*          pNewSet = nullptr;

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox

    if (  (nSlot == SID_ATTR_CHAR_WEIGHT)
        ||(nSlot == SID_ATTR_CHAR_POSTURE)
        ||(nSlot == SID_ATTR_CHAR_UNDERLINE)
        ||(nSlot == SID_ULINE_VAL_NONE)
        ||(nSlot == SID_ULINE_VAL_SINGLE)
        ||(nSlot == SID_ULINE_VAL_DOUBLE)
        ||(nSlot == SID_ULINE_VAL_DOTTED) )
    {
        pNewSet = new SfxAllItemSet( GetPool() );

        switch ( nSlot )
        {
            case SID_ATTR_CHAR_WEIGHT:
            {
                // #i78017 establish the same behaviour as in Writer
                SvtScriptType nScript = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;

                SfxItemPool& rPool = GetPool();
                SvxScriptSetItem aSetItem( nSlot, rPool );
                if ( pSet )
                    aSetItem.PutItemForScriptType( nScript, pSet->Get( ATTR_FONT_WEIGHT ) );
                else
                {
                    //  toggle manually

                    FontWeight eWeight = WEIGHT_BOLD;
                    SvxScriptSetItem aOldSetItem( nSlot, rPool );
                    aOldSetItem.GetItemSet().Put( pAttrs->GetItemSet(), false );
                    const SfxPoolItem* pCore = aOldSetItem.GetItemOfScript( nScript );
                    if ( pCore && static_cast<const SvxWeightItem*>(pCore)->GetWeight() == WEIGHT_BOLD )
                        eWeight = WEIGHT_NORMAL;

                    aSetItem.PutItemForScriptType( nScript, SvxWeightItem( eWeight, ATTR_FONT_WEIGHT ) );
                }
                pTabViewShell->ApplyUserItemSet( aSetItem.GetItemSet() );
                pNewSet->Put( aSetItem.GetItemSet(), false );
            }
            break;

            case SID_ATTR_CHAR_POSTURE:
            {
                // #i78017 establish the same behaviour as in Writer
                SvtScriptType nScript = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;

                SfxItemPool& rPool = GetPool();
                SvxScriptSetItem aSetItem( nSlot, rPool );
                if ( pSet )
                    aSetItem.PutItemForScriptType( nScript, pSet->Get( ATTR_FONT_POSTURE ) );
                else
                {
                    //  toggle manually

                    FontItalic eItalic = ITALIC_NORMAL;
                    SvxScriptSetItem aOldSetItem( nSlot, rPool );
                    aOldSetItem.GetItemSet().Put( pAttrs->GetItemSet(), false );
                    const SfxPoolItem* pCore = aOldSetItem.GetItemOfScript( nScript );
                    if ( pCore && static_cast<const SvxPostureItem*>(pCore)->GetPosture() == ITALIC_NORMAL )
                        eItalic = ITALIC_NONE;

                    aSetItem.PutItemForScriptType( nScript, SvxPostureItem( eItalic, ATTR_FONT_POSTURE ) );
                }
                pTabViewShell->ApplyUserItemSet( aSetItem.GetItemSet() );
                pNewSet->Put( aSetItem.GetItemSet(), false );
            }
            break;

            case SID_ATTR_CHAR_UNDERLINE:
                {
                    FontLineStyle       eUnderline;

                    if( pSet )
                    {
                        const SfxPoolItem& rUnderline = pSet->Get( ATTR_FONT_UNDERLINE );

                        if( dynamic_cast<const SvxUnderlineItem*>( &rUnderline) !=  nullptr )
                        {
                            pTabViewShell->ApplyAttr( rUnderline );
                            pNewSet->Put( rUnderline,rUnderline.Which() );
                        }
                        else if ( dynamic_cast<const SvxTextLineItem*>( &rUnderline) !=  nullptr )
                        {
                            // #i106580# also allow SvxTextLineItem (base class of SvxUnderlineItem)
                            const SvxTextLineItem& rTextLineItem = static_cast<const SvxTextLineItem&>(rUnderline);
                            SvxUnderlineItem aNewItem( rTextLineItem.GetLineStyle(), rTextLineItem.Which() );
                            aNewItem.SetColor( rTextLineItem.GetColor() );
                            pTabViewShell->ApplyAttr( aNewItem );
                            pNewSet->Put( aNewItem, aNewItem.Which() );
                        }
                    }
                    else
                    {
                        SvxUnderlineItem aUnderline( static_cast<const SvxUnderlineItem&>(
                                                        pAttrs->GetItem(
                                                            ATTR_FONT_UNDERLINE ) ) );
                        eUnderline = (LINESTYLE_NONE != aUnderline.GetLineStyle())
                                    ? LINESTYLE_NONE
                                    : LINESTYLE_SINGLE;
                        aUnderline.SetLineStyle( eUnderline );
                        pTabViewShell->ApplyAttr( aUnderline );
                        pNewSet->Put( aUnderline,aUnderline.Which() );
                    }
                }
                break;

            case SID_ULINE_VAL_NONE:
                pTabViewShell->ApplyAttr( SvxUnderlineItem( LINESTYLE_NONE, ATTR_FONT_UNDERLINE ) );
                break;
            case SID_ULINE_VAL_SINGLE:      // Toggles
            case SID_ULINE_VAL_DOUBLE:
            case SID_ULINE_VAL_DOTTED:
                {
                    FontLineStyle eOld = static_cast<const SvxUnderlineItem&>(
                                            pAttrs->GetItem(ATTR_FONT_UNDERLINE)).GetLineStyle();
                    FontLineStyle eNew = eOld;
                    switch (nSlot)
                    {
                        case SID_ULINE_VAL_SINGLE:
                            eNew = ( eOld == LINESTYLE_SINGLE ) ? LINESTYLE_NONE : LINESTYLE_SINGLE;
                            break;
                        case SID_ULINE_VAL_DOUBLE:
                            eNew = ( eOld == LINESTYLE_DOUBLE ) ? LINESTYLE_NONE : LINESTYLE_DOUBLE;
                            break;
                        case SID_ULINE_VAL_DOTTED:
                            eNew = ( eOld == LINESTYLE_DOTTED ) ? LINESTYLE_NONE : LINESTYLE_DOTTED;
                            break;
                    }
                    pTabViewShell->ApplyAttr( SvxUnderlineItem( eNew, ATTR_FONT_UNDERLINE ) );
                }
                break;

            default:
                break;
        }
        rBindings.Invalidate( nSlot );
    }
    else
    {
        /*
         * "Selbstgemachte" RadioButton-Funktionalitaet
         * Beim Toggle gibt es den Standard-State, d.h. kein
         * Button ist gedrueckt
         */

        const SfxItemSet&        rAttrSet   = pTabViewShell->GetSelectionPattern()->GetItemSet();
        const SfxPoolItem*       pItem       = nullptr;
        const SvxHorJustifyItem* pHorJustify = nullptr;
        const SvxVerJustifyItem* pVerJustify = nullptr;
        SvxCellHorJustify        eHorJustify = SVX_HOR_JUSTIFY_STANDARD;
        SvxCellVerJustify        eVerJustify = SVX_VER_JUSTIFY_STANDARD;

        if (rAttrSet.GetItemState(ATTR_HOR_JUSTIFY, true,&pItem ) == SfxItemState::SET)
        {
            pHorJustify = static_cast<const SvxHorJustifyItem*>(pItem);
            eHorJustify = SvxCellHorJustify( pHorJustify->GetValue() );
        }
        if (rAttrSet.GetItemState(ATTR_VER_JUSTIFY, true,&pItem ) == SfxItemState::SET)
        {
            pVerJustify = static_cast<const SvxVerJustifyItem*>(pItem);
            eVerJustify = SvxCellVerJustify( pVerJustify->GetValue() );
        }

        switch ( nSlot )
        {
            case SID_ALIGNLEFT:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SVX_HOR_JUSTIFY_LEFT) ?
                    SVX_HOR_JUSTIFY_LEFT : SVX_HOR_JUSTIFY_STANDARD, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNRIGHT:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SVX_HOR_JUSTIFY_RIGHT) ?
                    SVX_HOR_JUSTIFY_RIGHT : SVX_HOR_JUSTIFY_STANDARD, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNCENTERHOR:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SVX_HOR_JUSTIFY_CENTER) ?
                    SVX_HOR_JUSTIFY_CENTER : SVX_HOR_JUSTIFY_STANDARD, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNBLOCK:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SVX_HOR_JUSTIFY_BLOCK) ?
                    SVX_HOR_JUSTIFY_BLOCK : SVX_HOR_JUSTIFY_STANDARD, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNTOP:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SVX_VER_JUSTIFY_TOP) ?
                    SVX_VER_JUSTIFY_TOP : SVX_VER_JUSTIFY_STANDARD, SID_V_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNBOTTOM:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SVX_VER_JUSTIFY_BOTTOM) ?
                    SVX_VER_JUSTIFY_BOTTOM : SVX_VER_JUSTIFY_STANDARD, SID_V_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNCENTERVER:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SVX_VER_JUSTIFY_CENTER) ?
                    SVX_VER_JUSTIFY_CENTER : SVX_VER_JUSTIFY_STANDARD, SID_V_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            default:
            break;
        }

    }

    rBindings.Update();

    if( pNewSet )
    {
        rReq.Done( *pNewSet );
        delete pNewSet;
    }
    else
    {
        rReq.Done();
    }

}

void ScFormatShell::ExecuteAttr( SfxRequest& rReq )
{
    ScTabViewShell*     pTabViewShell = GetViewData()->GetViewShell();
    SfxBindings&        rBindings = pViewData->GetBindings();
    const SfxItemSet*   pNewAttrs = rReq.GetArgs();
    sal_uInt16          nSlot = rReq.GetSlot();

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox
    ScDocument* pDoc = GetViewData()->GetDocument();
    if ( !pNewAttrs )
    {
        switch ( nSlot )
        {
            case SID_GROW_FONT_SIZE:
            case SID_SHRINK_FONT_SIZE:
            {
                SfxItemPool& rPool = GetPool();
                SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONTHEIGHT, rPool );
                aSetItem.GetItemSet().Put( pTabViewShell->GetSelectionPattern()->GetItemSet(), false );

                SvtScriptType nScriptTypes = pTabViewShell->GetSelectionScriptType();
                const SvxFontHeightItem* pSize( static_cast<const SvxFontHeightItem*>( aSetItem.GetItemOfScript( nScriptTypes ) ) );

                if ( pSize )
                {
                    SvxFontHeightItem aSize( *pSize );
                    sal_uInt32 nSize = aSize.GetHeight();

                    const sal_uInt32 nFontInc = 40;      // 2pt
                    const sal_uInt32 nFontMaxSz = 19998; // 999.9pt
                    if ( nSlot == SID_GROW_FONT_SIZE )
                        nSize = std::min< sal_uInt32 >( nSize + nFontInc, nFontMaxSz );
                    else
                        nSize = std::max< sal_Int32 >( nSize - nFontInc, nFontInc );

                    aSize.SetHeight( nSize );
                    aSetItem.PutItemForScriptType( nScriptTypes, aSize );
                    pTabViewShell->ApplyUserItemSet( aSetItem.GetItemSet() );
                }
                rBindings.Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
            }
            break;

            case SID_ATTR_CHAR_ENDPREVIEW_FONT:
            {
                pDoc->SetPreviewFont(nullptr);
                pTabViewShell->UpdateSelectionArea( pDoc->GetPreviewSelection() );
                break;
            }
            case SID_ATTR_CHAR_COLOR:
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
                pTabViewShell->ExecuteCellFormatDlg(rReq, "font");       // when ToolBar is vertical
                break;

            case SID_BACKGROUND_COLOR:
                {
                    SvxBrushItem aBrushItem( static_cast<const SvxBrushItem&>(
                                     pTabViewShell->GetSelectionPattern()->GetItem( ATTR_BACKGROUND ) ) );
                    aBrushItem.SetColor( COL_TRANSPARENT );
                    pTabViewShell->ApplyAttr( aBrushItem );
                }
                break;

            case SID_ATTR_ALIGN_LINEBREAK:                  // without parameter as toggle
                {
                    const ScPatternAttr* pAttrs = pTabViewShell->GetSelectionPattern();
                    bool bOld = static_cast<const SfxBoolItem&>(pAttrs->GetItem(ATTR_LINEBREAK)).GetValue();
                    SfxBoolItem aBreakItem( ATTR_LINEBREAK, !bOld );
                    pTabViewShell->ApplyAttr( aBreakItem );

                    SfxAllItemSet aNewSet( GetPool() );
                    aNewSet.Put( aBreakItem,aBreakItem.Which() );
                    rReq.Done( aNewSet );

                    rBindings.Invalidate( nSlot );
                }
                break;
        }
    }
    else
    {
        switch ( nSlot )
        {
            case SID_ATTR_CHAR_PREVIEW_FONT:
            {
                SfxItemPool& rPool = GetPool();
                sal_uInt16 nWhich = rPool.GetWhich( nSlot );
                const SvxFontItem& rFont = static_cast<const SvxFontItem&>(pNewAttrs->Get( nWhich ));
                SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, rPool );
                SvtScriptType nScript = pTabViewShell->GetSelectionScriptType();
                aSetItem.PutItemForScriptType( nScript, rFont );

                ScMarkData aFuncMark( pViewData->GetMarkData() );
                ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );
                pDoc->SetPreviewFont( aSetItem.GetItemSet().Clone() );
                aFuncMark.MarkToMulti();

                if ( !aFuncMark.IsMarked() && !aFuncMark.IsMultiMarked() )
                {
                    SCCOL nCol = pViewData->GetCurX();
                    SCROW nRow = pViewData->GetCurY();
                    SCTAB nTab = pViewData->GetTabNo();
                    ScRange aRange( nCol, nRow, nTab );
                    aFuncMark.SetMarkArea( aRange );
                }
                pDoc->SetPreviewSelection( aFuncMark );
                pTabViewShell->UpdateSelectionArea( aFuncMark );
                break;
            }
            case SID_ATTR_CHAR_OVERLINE:
            case SID_ATTR_CHAR_STRIKEOUT:
            case SID_ATTR_ALIGN_LINEBREAK:
            case SID_ATTR_CHAR_COLOR:
            case SID_ATTR_CHAR_CONTOUR:
            case SID_ATTR_CHAR_SHADOWED:
            case SID_ATTR_CHAR_RELIEF:
            case SID_SCATTR_PROTECTION :
                pTabViewShell->ApplyAttr( pNewAttrs->Get( pNewAttrs->GetPool()->GetWhich( nSlot ) ) );
                rBindings.Invalidate( nSlot );
                rBindings.Update( nSlot );
                break;

            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
                {
                    // #i78017 establish the same behaviour as in Writer
                    SvtScriptType nScript = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;
                    if (nSlot == SID_ATTR_CHAR_FONT)
                        nScript = pTabViewShell->GetSelectionScriptType();

                    SfxItemPool& rPool = GetPool();
                    SvxScriptSetItem aSetItem( nSlot, rPool );
                    sal_uInt16 nWhich = rPool.GetWhich( nSlot );
                    aSetItem.PutItemForScriptType( nScript, pNewAttrs->Get( nWhich ) );

                    pTabViewShell->ApplyUserItemSet( aSetItem.GetItemSet() );

                    rBindings.Invalidate( nSlot );
                    rBindings.Update( nSlot );
                }
                break;

            case SID_FRAME_LINESTYLE:
                {
                    // Update default line
                    const ::editeng::SvxBorderLine* pLine =
                            static_cast<const SvxLineItem&>(
                                pNewAttrs->Get( SID_FRAME_LINESTYLE )).
                                GetLine();

                    if ( pLine )
                    {
                        ::editeng::SvxBorderLine* pDefLine = pTabViewShell->GetDefaultFrameLine();

                        if ( pDefLine )
                        {
                            pDefLine->SetBorderLineStyle(
                                    pLine->GetBorderLineStyle());
                            pDefLine->SetWidth( pLine->GetWidth( ) );
                            pTabViewShell->SetSelectionFrameLines( pDefLine, false );
                        }
                        else
                        {
                            pTabViewShell->SetDefaultFrameLine( pLine );
                            pTabViewShell->GetDefaultFrameLine()->SetColor( COL_BLACK );
                            pTabViewShell->SetSelectionFrameLines( pLine, false );
                        }
                    }
                    else
                    {
                        Color           aColorBlack( COL_BLACK );
                        ::editeng::SvxBorderLine aDefLine( &aColorBlack, 20,
                                table::BorderLineStyle::SOLID );
                        pTabViewShell->SetDefaultFrameLine( &aDefLine );
                        pTabViewShell->SetSelectionFrameLines( nullptr, false );
                    }
                }
                break;

            case SID_FRAME_LINECOLOR:
                {
                    ::editeng::SvxBorderLine*  pDefLine = pTabViewShell->GetDefaultFrameLine();
                    const Color&    rColor = static_cast<const SvxColorItem&>(
                                        pNewAttrs->Get( SID_FRAME_LINECOLOR )).
                                            GetValue();

                    // Update default line
                    if ( pDefLine )
                    {
                        pDefLine->SetColor( rColor );
                        pTabViewShell->SetSelectionFrameLines( pDefLine, true );
                    }
                    else
                    {
                        ::editeng::SvxBorderLine aDefLine( &rColor, 20,
                                table::BorderLineStyle::SOLID );
                        pTabViewShell->SetDefaultFrameLine( &aDefLine );
                        pTabViewShell->SetSelectionFrameLines( &aDefLine, false );
                    }
                }
                break;

            case SID_ATTR_BORDER_OUTER:
            case SID_ATTR_BORDER:
                {
                    ::editeng::SvxBorderLine*          pDefLine = pTabViewShell->GetDefaultFrameLine();
                    const ScPatternAttr*    pOldAttrs = pTabViewShell->GetSelectionPattern();
                    std::unique_ptr<SfxItemSet> pOldSet(
                                                new SfxItemSet(
                                                        *(pDoc->GetPool()),
                                                        ATTR_PATTERN_START,
                                                        ATTR_PATTERN_END ));
                    std::unique_ptr<SfxItemSet> pNewSet(
                                                new SfxItemSet(
                                                        *(pDoc->GetPool()),
                                                        ATTR_PATTERN_START,
                                                        ATTR_PATTERN_END ));
                    const SfxPoolItem&      rBorderAttr =
                                                pOldAttrs->GetItemSet().
                                                    Get( ATTR_BORDER );

                    // Evaluate border items from controller:
                    const SfxPoolItem* pItem = nullptr;

                    if ( pNewAttrs->GetItemState( ATTR_BORDER, true, &pItem )
                         == SfxItemState::SET )
                    {
                        //  The SvxFrameToolBoxControl toolbox controller uses a default
                        //  SvxBorderLine (all widths 0) to mark the lines that should be set.
                        //  Macro recording uses a SvxBoxItem with the real values (OutWidth > 0)
                        //  or NULL pointers for no lines.
                        //  -> Substitute existing lines with pDefLine only if widths are 0.
                        SvxBoxItem aBoxItem ( *static_cast<const SvxBoxItem*>(pItem ));
                        if ( aBoxItem.GetTop() && aBoxItem.GetTop()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, SvxBoxItemLine::TOP );
                        if ( aBoxItem.GetBottom() && aBoxItem.GetBottom()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, SvxBoxItemLine::BOTTOM );
                        if ( aBoxItem.GetLeft() && aBoxItem.GetLeft()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, SvxBoxItemLine::LEFT );
                        if ( aBoxItem.GetRight() && aBoxItem.GetRight()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, SvxBoxItemLine::RIGHT );
                        pNewSet->Put( aBoxItem );
                        rReq.AppendItem( aBoxItem );
                    }

                    if ( pNewAttrs->GetItemState( ATTR_BORDER_INNER, true, &pItem )
                         == SfxItemState::SET )
                    {
                        SvxBoxInfoItem aBoxInfoItem( *static_cast<const SvxBoxInfoItem*>(pItem) );
                        if ( aBoxInfoItem.GetHori() && aBoxInfoItem.GetHori()->GetOutWidth() == 0 )
                            aBoxInfoItem.SetLine( pDefLine, SvxBoxInfoItemLine::HORI );
                        if ( aBoxInfoItem.GetVert() && aBoxInfoItem.GetVert()->GetOutWidth() == 0 )
                            aBoxInfoItem.SetLine( pDefLine, SvxBoxInfoItemLine::VERT );
                        pNewSet->Put( aBoxInfoItem );
                        rReq.AppendItem( aBoxInfoItem );
                    }
                    else
                    {
                        SvxBoxInfoItem aBoxInfoItem( ATTR_BORDER_INNER );
                        aBoxInfoItem.SetLine( nullptr, SvxBoxInfoItemLine::HORI );
                        aBoxInfoItem.SetLine( nullptr, SvxBoxInfoItemLine::VERT );
                        pNewSet->Put( aBoxInfoItem );
                    }

                    pOldSet->Put( rBorderAttr );
                    pTabViewShell->ApplyAttributes( pNewSet.get(), pOldSet.get() );
                }
                break;

            case SID_ATTR_BORDER_DIAG_TLBR:
            case SID_ATTR_BORDER_DIAG_BLTR:
                {
                    const ScPatternAttr* pOldAttrs = pTabViewShell->GetSelectionPattern();
                    std::unique_ptr<SfxItemSet> pOldSet(new SfxItemSet(pOldAttrs->GetItemSet()));
                    std::unique_ptr<SfxItemSet> pNewSet(new SfxItemSet(pOldAttrs->GetItemSet()));
                    const SfxPoolItem* pItem = nullptr;

                    if(SID_ATTR_BORDER_DIAG_TLBR == nSlot)
                    {
                        if(SfxItemState::SET == pNewAttrs->GetItemState(ATTR_BORDER_TLBR, true, &pItem))
                        {
                            SvxLineItem aItem(ATTR_BORDER_TLBR);
                            aItem.SetLine(static_cast<const SvxLineItem&>(pNewAttrs->Get(ATTR_BORDER_TLBR)).GetLine());
                            pNewSet->Put(aItem);
                            rReq.AppendItem(aItem);
                            pTabViewShell->ApplyAttributes(pNewSet.get(), pOldSet.get());
                        }
                    }
                    else // if( nSlot == SID_ATTR_BORDER_DIAG_BLTR )
                    {
                        if(SfxItemState::SET == pNewAttrs->GetItemState(ATTR_BORDER_BLTR, true, &pItem ))
                        {
                            SvxLineItem aItem(ATTR_BORDER_BLTR);
                            aItem.SetLine(static_cast<const SvxLineItem&>(pNewAttrs->Get(ATTR_BORDER_BLTR)).GetLine());
                            pNewSet->Put(aItem);
                            rReq.AppendItem(aItem);
                            pTabViewShell->ApplyAttributes(pNewSet.get(), pOldSet.get());
                        }
                    }

                    rBindings.Invalidate(nSlot);
                }
                break;

            // ATTR_BACKGROUND (=SID_ATTR_BRUSH) has to be set to two IDs:
            case SID_BACKGROUND_COLOR:
                {
                    const SvxColorItem  rNewColorItem = static_cast<const SvxColorItem&>(
                                            pNewAttrs->Get( SID_BACKGROUND_COLOR ) );

                    SvxBrushItem        aBrushItem( static_cast<const SvxBrushItem&>(
                                            pTabViewShell->GetSelectionPattern()->
                                                GetItem( ATTR_BACKGROUND ) ) );

                    aBrushItem.SetColor( rNewColorItem.GetValue() );

                    pTabViewShell->ApplyAttr( aBrushItem );
                }
                break;

                case SID_ATTR_BRUSH:
                {
                    SvxBrushItem        aBrushItem( static_cast<const SvxBrushItem&>(
                                            pTabViewShell->GetSelectionPattern()->
                                                GetItem( ATTR_BACKGROUND ) ) );
                    const SvxBrushItem& rNewBrushItem = static_cast<const SvxBrushItem&>(
                                            pNewAttrs->Get( GetPool().GetWhich(nSlot) ) );
                    aBrushItem.SetColor(rNewBrushItem.GetColor());
                    pTabViewShell->ApplyAttr( aBrushItem );
                }
                break;

            case SID_ATTR_BORDER_SHADOW:
                {
                    const SvxShadowItem& rNewShadowItem = static_cast<const SvxShadowItem&>(
                                            pNewAttrs->Get( ATTR_SHADOW ) );
                    pTabViewShell->ApplyAttr( rNewShadowItem );
                }
                break;

            default:
            break;
        }

        if( ! rReq.IsAPI() )
            if( ! rReq.IsDone() )
                rReq.Done();
    }
}

void ScFormatShell::GetAttrState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    const SfxItemSet&    rAttrSet   = pTabViewShell->GetSelectionPattern()->GetItemSet();
    const SvxBrushItem&  rBrushItem = static_cast<const SvxBrushItem&>(rAttrSet.Get( ATTR_BACKGROUND ));
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    rSet.Put( rAttrSet, false );

    //  choose font info according to selection script type
    SvtScriptType nScript = SvtScriptType::NONE;      // GetSelectionScriptType never returns 0
    if ( rSet.GetItemState( ATTR_FONT ) != SfxItemState::UNKNOWN )
    {
        if (nScript == SvtScriptType::NONE) nScript = pTabViewShell->GetSelectionScriptType();
        ScViewUtil::PutItemScript( rSet, rAttrSet, ATTR_FONT, nScript );
    }
    if ( rSet.GetItemState( ATTR_FONT_HEIGHT ) != SfxItemState::UNKNOWN )
    {
        if (nScript == SvtScriptType::NONE) nScript = pTabViewShell->GetSelectionScriptType();
        ScViewUtil::PutItemScript( rSet, rAttrSet, ATTR_FONT_HEIGHT, nScript );
    }

    while ( nWhich )
    {
        switch(nWhich)
        {
            case SID_BACKGROUND_COLOR:
            {
                rSet.Put( SvxColorItem( rBrushItem.GetColor(), SID_BACKGROUND_COLOR ) );
                if(SfxItemState::DONTCARE == rAttrSet.GetItemState(ATTR_BACKGROUND))
                {
                    rSet.InvalidateItem(SID_BACKGROUND_COLOR);
                }
            }
            break;
            case SID_FRAME_LINESTYLE:
            case SID_FRAME_LINECOLOR:
            {
                // handled together because both need the cell border information for decisions
                Color aCol = 0;
                editeng::SvxBorderLine aLine(nullptr,0,0);
                bool bCol = false;
                bool bColDisable = false, bStyleDisable = false;
                SvxBoxItem aBoxItem(ATTR_BORDER);
                SvxBoxInfoItem aInfoItem(ATTR_BORDER_INNER);

                pTabViewShell->GetSelectionFrame(aBoxItem, aInfoItem);

                if( aBoxItem.GetTop() )
                {
                    bCol = true;
                    aCol = aBoxItem.GetTop()->GetColor() ;
                    aLine.SetColor(aCol);
                    aLine.SetWidth( aBoxItem.GetTop()->GetWidth());
                    aLine.SetBorderLineStyle( aBoxItem.GetTop()->GetBorderLineStyle());
                }

                if( aBoxItem.GetBottom() )
                {
                    if(!bCol)
                    {
                        bCol = true;
                        aCol = aBoxItem.GetBottom()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aBoxItem.GetBottom()->GetWidth());
                        aLine.SetBorderLineStyle( aBoxItem.GetBottom()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aBoxItem.GetBottom()->GetColor() )
                            bColDisable = true;
                        if(!( aLine == *(aBoxItem.GetBottom())) )
                            bStyleDisable = true;
                    }
                }

                if( aBoxItem.GetLeft() )
                {
                    if(!bCol)
                    {
                        bCol = true;
                        aCol = aBoxItem.GetLeft()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aBoxItem.GetLeft()->GetWidth());
                        aLine.SetBorderLineStyle( aBoxItem.GetLeft()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aBoxItem.GetLeft()->GetColor() )
                            bColDisable = true;
                        if(!( aLine == *(aBoxItem.GetLeft())) )
                            bStyleDisable = true;
                    }
                }

                if( aBoxItem.GetRight() )
                {
                    if(!bCol)
                    {
                        bCol = true;
                        aCol = aBoxItem.GetRight()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aBoxItem.GetRight()->GetWidth());
                        aLine.SetBorderLineStyle( aBoxItem.GetRight()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aBoxItem.GetRight()->GetColor() )
                            bColDisable = true;
                        if(!( aLine == *(aBoxItem.GetRight())) )
                            bStyleDisable = true;
                    }
                }

                if( aInfoItem.GetVert())
                {
                    if(!bCol)
                    {
                        bCol = true;
                        aCol = aInfoItem.GetVert()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aInfoItem.GetVert()->GetWidth());
                        aLine.SetBorderLineStyle( aInfoItem.GetVert()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aInfoItem.GetVert()->GetColor() )
                            bColDisable = true;
                        if(!( aLine == *(aInfoItem.GetVert())) )
                            bStyleDisable = true;
                    }
                }

                if( aInfoItem.GetHori())
                {
                    if(!bCol)
                    {
                        bCol = true;
                        aCol = aInfoItem.GetHori()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aInfoItem.GetHori()->GetWidth());
                        aLine.SetBorderLineStyle( aInfoItem.GetHori()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aInfoItem.GetHori()->GetColor() )
                            bColDisable = true;
                        if(!( aLine == *(aInfoItem.GetHori())) )
                            bStyleDisable = true;
                    }
                }

                if( !aInfoItem.IsValid( SvxBoxInfoItemValidFlags::VERT )
                    || !aInfoItem.IsValid( SvxBoxInfoItemValidFlags::HORI )
                    || !aInfoItem.IsValid( SvxBoxInfoItemValidFlags::LEFT )
                    || !aInfoItem.IsValid( SvxBoxInfoItemValidFlags::RIGHT )
                    || !aInfoItem.IsValid( SvxBoxInfoItemValidFlags::TOP )
                    || !aInfoItem.IsValid( SvxBoxInfoItemValidFlags::BOTTOM ) )
                {
                    bColDisable = true;
                    bStyleDisable = true;
                }

                if(SID_FRAME_LINECOLOR == nWhich)
                {
                    if(bColDisable) // if different lines have differernt colors
                    {
                        aCol = COL_TRANSPARENT;
                        rSet.Put( SvxColorItem(aCol, SID_FRAME_LINECOLOR ) );
                        rSet.InvalidateItem(SID_FRAME_LINECOLOR);
                    }
                    else if( !bCol && !bColDisable) // if no line available
                    {
                        aCol = COL_AUTO;
                        rSet.Put( SvxColorItem(aCol, SID_FRAME_LINECOLOR ) );
                    }
                    else
                        rSet.Put( SvxColorItem(aCol, SID_FRAME_LINECOLOR ) );
                }
                else // if( nWhich == SID_FRAME_LINESTYLE)
                {
                    if(bStyleDisable) // if have several lines but don't have same style
                    {
                        aLine.SetWidth( 1 );
                        SvxLineItem aItem(SID_FRAME_LINESTYLE);
                        aItem.SetLine(&aLine);
                        rSet.Put( aItem );
                        rSet.InvalidateItem(SID_FRAME_LINESTYLE);
                    }
                    else // all the lines have same style or no line available, use initial value (0,0,0,0)
                    {
                        SvxLineItem aItem(SID_FRAME_LINESTYLE);
                        aItem.SetLine(&aLine);
                        rSet.Put( aItem );
                    }
                }
            }
            break;
            case SID_ATTR_BRUSH:
            {
                std::unique_ptr<SfxPoolItem> pNewItem(rBrushItem.CloneSetWhich(GetPool().GetWhich(nWhich)));
                rSet.Put( *pNewItem );
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }

    // stuff for sidebar panels
    Invalidate(SID_ATTR_ALIGN_DEGREES);
    Invalidate(SID_ATTR_ALIGN_LOCKPOS);
    Invalidate(SID_ATTR_ALIGN_STACKED);
}

void ScFormatShell::GetTextAttrState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    const SfxItemSet& rAttrSet  = pTabViewShell->GetSelectionPattern()->GetItemSet();
    rSet.Put( rAttrSet, false ); // Include ItemStates in copy

    //  choose font info according to selection script type
    SvtScriptType nScript = SvtScriptType::NONE;      // GetSelectionScriptType never returns 0
    if ( rSet.GetItemState( ATTR_FONT_WEIGHT ) != SfxItemState::UNKNOWN )
    {
        if (nScript == SvtScriptType::NONE) nScript = pTabViewShell->GetSelectionScriptType();
        ScViewUtil::PutItemScript( rSet, rAttrSet, ATTR_FONT_WEIGHT, nScript );
    }
    if ( rSet.GetItemState( ATTR_FONT_POSTURE ) != SfxItemState::UNKNOWN )
    {
        if (nScript == SvtScriptType::NONE) nScript = pTabViewShell->GetSelectionScriptType();
        ScViewUtil::PutItemScript( rSet, rAttrSet, ATTR_FONT_POSTURE, nScript );
    }

    SfxItemState eState;

    // own control on radio button functionallity:

    // underline

    eState = rAttrSet.GetItemState( ATTR_FONT_UNDERLINE );
    if ( eState == SfxItemState::DONTCARE )
    {
        rSet.InvalidateItem( SID_ULINE_VAL_NONE );
        rSet.InvalidateItem( SID_ULINE_VAL_SINGLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOUBLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOTTED );
    }
    else
    {
        FontLineStyle eUnderline = static_cast<const SvxUnderlineItem&>(
                    rAttrSet.Get(ATTR_FONT_UNDERLINE)).GetLineStyle();
        sal_uInt16 nId = SID_ULINE_VAL_NONE;
        switch (eUnderline)
        {
            case LINESTYLE_SINGLE:  nId = SID_ULINE_VAL_SINGLE; break;
            case LINESTYLE_DOUBLE:  nId = SID_ULINE_VAL_DOUBLE; break;
            case LINESTYLE_DOTTED:  nId = SID_ULINE_VAL_DOTTED; break;
            default:
                break;
        }
        rSet.Put( SfxBoolItem( nId, true ) );
    }

    // horizontal alignment

    const SvxHorJustifyItem* pHorJustify = nullptr;
    const SvxVerJustifyItem* pVerJustify = nullptr;
    SvxCellVerJustify        eVerJustify = SVX_VER_JUSTIFY_STANDARD;
    sal_uInt16                   nWhich      = 0;
    bool                     bJustifyStd = false;
    SfxBoolItem              aBoolItem   ( 0, true );

    eState   = rAttrSet.GetItemState( ATTR_HOR_JUSTIFY, true,
                                        reinterpret_cast<const SfxPoolItem**>(&pHorJustify) );
    switch ( eState )
    {
        case SfxItemState::SET:
            {
                switch ( SvxCellHorJustify( pHorJustify->GetValue() ) )
                {
                    case SVX_HOR_JUSTIFY_STANDARD:
                        break;

                    case SVX_HOR_JUSTIFY_LEFT:
                        nWhich = SID_ALIGNLEFT;
                        break;

                    case SVX_HOR_JUSTIFY_RIGHT:
                        nWhich = SID_ALIGNRIGHT;
                        break;

                    case SVX_HOR_JUSTIFY_CENTER:
                        nWhich = SID_ALIGNCENTERHOR;
                        break;

                    case SVX_HOR_JUSTIFY_BLOCK:
                        nWhich = SID_ALIGNBLOCK;
                        break;

                    case SVX_HOR_JUSTIFY_REPEAT:
                    default:
                        bJustifyStd = true;
                        break;
                }
            }
            break;

        case SfxItemState::DONTCARE:
            rSet.InvalidateItem( SID_ALIGNLEFT );
            rSet.InvalidateItem( SID_ALIGNRIGHT );
            rSet.InvalidateItem( SID_ALIGNCENTERHOR );
            rSet.InvalidateItem( SID_ALIGNBLOCK );
            break;

        default:
            bJustifyStd = true;
            break;
    }

    if ( nWhich )
    {
        aBoolItem.SetWhich( nWhich );
        rSet.Put( aBoolItem );
    }
    else if ( bJustifyStd )
    {
        aBoolItem.SetValue( false );
        aBoolItem.SetWhich( SID_ALIGNLEFT );      rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNRIGHT );     rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNCENTERHOR ); rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNBLOCK );     rSet.Put( aBoolItem );
        bJustifyStd = false;
    }

    // vertical alignment

    nWhich = 0;
    aBoolItem.SetValue( true );

    eState = rAttrSet.GetItemState( ATTR_VER_JUSTIFY, true,
                                    reinterpret_cast<const SfxPoolItem**>(&pVerJustify) );

    switch ( eState )
    {
        case SfxItemState::SET:
            {
                eVerJustify = SvxCellVerJustify( pVerJustify->GetValue() );

                switch ( eVerJustify )
                {
                    case SVX_VER_JUSTIFY_TOP:
                        nWhich = SID_ALIGNTOP;
                        break;

                    case SVX_VER_JUSTIFY_BOTTOM:
                        nWhich = SID_ALIGNBOTTOM;
                        break;

                    case SVX_VER_JUSTIFY_CENTER:
                        nWhich = SID_ALIGNCENTERVER;
                        break;

                    case SVX_VER_JUSTIFY_STANDARD:
                    default:
                        bJustifyStd = true;
                        break;
                }
            }
            break;

        case SfxItemState::DONTCARE:
            rSet.InvalidateItem( SID_ALIGNTOP );
            rSet.InvalidateItem( SID_ALIGNBOTTOM );
            rSet.InvalidateItem( SID_ALIGNCENTERVER );
            break;

        default:
            bJustifyStd = true;
            break;
    }

    if ( nWhich )
    {
        aBoolItem.SetWhich( nWhich );
        rSet.Put( aBoolItem );
    }
    else if ( bJustifyStd )
    {
        aBoolItem.SetValue( false );
        aBoolItem.SetWhich( SID_ALIGNTOP );       rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNBOTTOM );    rSet.Put( aBoolItem );
        aBoolItem.SetWhich( SID_ALIGNCENTERVER ); rSet.Put( aBoolItem );
    }
}

void ScFormatShell::GetBorderState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    SvxBoxItem      aBoxItem( ATTR_BORDER );
    SvxBoxInfoItem  aInfoItem( ATTR_BORDER_INNER );

    pTabViewShell->GetSelectionFrame( aBoxItem, aInfoItem );

    if ( rSet.GetItemState( ATTR_BORDER ) != SfxItemState::UNKNOWN )
        rSet.Put( aBoxItem );
    if ( rSet.GetItemState( ATTR_BORDER_INNER ) != SfxItemState::UNKNOWN )
        rSet.Put( aInfoItem );
}

void ScFormatShell::GetAlignState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    const SfxItemSet& rAttrSet    = pTabViewShell->GetSelectionPattern()->GetItemSet();
    SfxWhichIter    aIter(rSet);
    sal_uInt16          nWhich = aIter.FirstWhich();

    SvxCellHorJustify eHAlign = SVX_HOR_JUSTIFY_STANDARD;
    bool bHasHAlign = rAttrSet.GetItemState( ATTR_HOR_JUSTIFY ) != SfxItemState::DONTCARE;
    if( bHasHAlign )
        eHAlign = (SvxCellHorJustify)static_cast<const SvxHorJustifyItem&>(rAttrSet.Get( ATTR_HOR_JUSTIFY )).GetValue();

    SvxCellVerJustify eVAlign = SVX_VER_JUSTIFY_STANDARD;
    bool bHasVAlign = rAttrSet.GetItemState( ATTR_VER_JUSTIFY ) != SfxItemState::DONTCARE;
    if( bHasVAlign )
        eVAlign = (SvxCellVerJustify)static_cast<const SvxVerJustifyItem&>(rAttrSet.Get( ATTR_VER_JUSTIFY )).GetValue();

    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_H_ALIGNCELL:
                if ( bHasHAlign )
                    rSet.Put( SvxHorJustifyItem( eHAlign, nWhich ));
            break;
            case SID_V_ALIGNCELL:
                if ( bHasVAlign )
                    rSet.Put( SvxVerJustifyItem( eVAlign, nWhich ));
            break;

            // pseudo slots for Format menu
            case SID_ALIGN_ANY_HDEFAULT:
            case SID_ALIGN_ANY_LEFT:
            case SID_ALIGN_ANY_HCENTER:
            case SID_ALIGN_ANY_RIGHT:
            case SID_ALIGN_ANY_JUSTIFIED:
                rSet.Put( SfxBoolItem( nWhich, bHasHAlign && (eHAlign == lclConvertSlotToHAlign( nWhich )) ) );
            break;
            case SID_ALIGN_ANY_VDEFAULT:
            case SID_ALIGN_ANY_TOP:
            case SID_ALIGN_ANY_VCENTER:
            case SID_ALIGN_ANY_BOTTOM:
                rSet.Put( SfxBoolItem( nWhich, bHasVAlign && (eVAlign == lclConvertSlotToVAlign( nWhich )) ) );
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

void ScFormatShell::GetNumFormatState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    ScDocument* pDoc                = pViewData->GetDocument();
    short nType                     = GetCurrentNumberFormatType();

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_NUMBER_FORMAT:
                // symphony version with format interpretation
                {
                    const SfxItemSet& rAttrSet = pTabViewShell->GetSelectionPattern()->GetItemSet();

                    if(SfxItemState::DONTCARE != rAttrSet.GetItemState(ATTR_VALUE_FORMAT))
                    {
                        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                        sal_uInt32 nNumberFormat = static_cast<const SfxUInt32Item&>(rAttrSet.Get(ATTR_VALUE_FORMAT)).GetValue();
                        bool bThousand(false);
                        bool bNegRed(false);
                        sal_uInt16 nPrecision(0);
                        sal_uInt16 nLeadZeroes(0);

                        pFormatter->GetFormatSpecialInfo(nNumberFormat,bThousand, bNegRed, nPrecision, nLeadZeroes);

                        const SvNumberformat* pFormatEntry = pFormatter->GetEntry( nNumberFormat );
                        if (pFormatEntry && (pFormatEntry->GetType() & css::util::NumberFormat::SCIENTIFIC))
                        {
                            // if scientific, bThousand is used for engineering notation
                            const sal_uInt16 nIntegerDigits = pFormatEntry->GetFormatIntegerDigits();
                            if (nIntegerDigits > 0 && ((nIntegerDigits % 3) == 0))
                                bThousand = true;
                            else
                                bThousand = false;
                        }
                        OUString aFormat;
                        static OUString sBreak = ",";
                        const OUString sThousand = OUString::number(static_cast<sal_Int32>(bThousand));
                        const OUString sNegRed = OUString::number(static_cast<sal_Int32>(bNegRed));
                        const OUString sPrecision = OUString::number(nPrecision);
                        const OUString sLeadZeroes = OUString::number(nLeadZeroes);

                        aFormat += sThousand;
                        aFormat += sBreak;
                        aFormat += sNegRed;
                        aFormat += sBreak;
                        aFormat += sPrecision;
                        aFormat += sBreak;
                        aFormat += sLeadZeroes;
                        aFormat += sBreak;

                        rSet.Put(SfxStringItem(nWhich, aFormat));
                    }
                    else
                    {
                        rSet.InvalidateItem( nWhich );
                    }
                }
                break;

            case SID_NUMBER_TYPE_FORMAT:
                {
                    sal_Int16 aFormatCode = -1;
                    const SfxItemSet& rAttrSet  = pTabViewShell->GetSelectionPattern()->GetItemSet();
                    if ( rAttrSet.GetItemState( ATTR_VALUE_FORMAT ) >= SfxItemState::DEFAULT ) //Modify for more robust
                    {
                        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                        sal_uInt32 nNumberFormat = pTabViewShell->GetSelectionPattern()->GetNumberFormat( pFormatter );
                        const SvNumberformat* pFormatEntry = pFormatter->GetEntry( nNumberFormat );
                        bool bStandard = false;

                        if ( pFormatEntry )
                        {
                            aFormatCode = pFormatEntry->GetType();
                            bStandard = pFormatEntry->IsStandard();
                        }

                        switch(aFormatCode)
                        {
                        case css::util::NumberFormat::NUMBER:
                        case css::util::NumberFormat::NUMBER| css::util::NumberFormat::DEFINED:
                            //use format code and standard format code to judge whether it is General,
                            if (bStandard)
                                aFormatCode = 0;
                            else
                                aFormatCode = 1;
                            break;
                        case css::util::NumberFormat::PERCENT:
                        case css::util::NumberFormat::PERCENT| css::util::NumberFormat::DEFINED:
                            aFormatCode = 2;
                            break;
                        case css::util::NumberFormat::CURRENCY:
                        case css::util::NumberFormat::CURRENCY| css::util::NumberFormat::DEFINED:
                            aFormatCode = 3;
                            break;
                        case css::util::NumberFormat::DATE:
                        case css::util::NumberFormat::DATE| css::util::NumberFormat::DEFINED:
                            //Add
                        case css::util::NumberFormat::DATETIME:
                        case css::util::NumberFormat::DATETIME | css::util::NumberFormat::DEFINED:
                            aFormatCode = 4;
                            break;
                        case css::util::NumberFormat::TIME:
                        case css::util::NumberFormat::TIME| css::util::NumberFormat::DEFINED:
                            aFormatCode = 5;
                            break;
                        case css::util::NumberFormat::SCIENTIFIC:
                        case css::util::NumberFormat::SCIENTIFIC| css::util::NumberFormat::DEFINED:
                            aFormatCode = 6;
                            break;
                        case css::util::NumberFormat::FRACTION:
                        case css::util::NumberFormat::FRACTION| css::util::NumberFormat::DEFINED:
                            aFormatCode = 7;
                            break;
                        case css::util::NumberFormat::LOGICAL:
                        case css::util::NumberFormat::LOGICAL| css::util::NumberFormat::DEFINED:
                            aFormatCode = 8;
                            break;
                        case css::util::NumberFormat::TEXT:
                        case css::util::NumberFormat::TEXT| css::util::NumberFormat::DEFINED:
                            aFormatCode = 9;
                            break;
                        default:
                            aFormatCode = -1;   //for more robust
                        }
                        if( aFormatCode == -1 )
                            rSet.InvalidateItem( nWhich );
                        else
                            rSet.Put( SfxInt16Item( nWhich, aFormatCode ) );
                    }
                    else
                    {
                        rSet.InvalidateItem( nWhich );
                    }

                }
                break;
            case SID_NUMBER_CURRENCY:
                {
                    const SfxItemSet& rAttrSet = pTabViewShell->GetSelectionPattern()->GetItemSet();
                    if( SfxItemState::DONTCARE != rAttrSet.GetItemState( ATTR_VALUE_FORMAT ) )
                    {
                        sal_uInt32 nNumberFormat = static_cast<const SfxUInt32Item&>(
                                                   rAttrSet.Get( ATTR_VALUE_FORMAT ) ).GetValue();
                        rSet.Put( SfxUInt32Item( nWhich, nNumberFormat ) );
                    }
                    else
                        rSet.InvalidateItem( nWhich );
                }
                break;
            case SID_NUMBER_SCIENTIFIC:
                rSet.Put( SfxBoolItem(nWhich, (nType & css::util::NumberFormat::SCIENTIFIC)) );
                break;
            case SID_NUMBER_DATE:
                rSet.Put( SfxBoolItem(nWhich, (nType & css::util::NumberFormat::DATE)) );
                break;
            case SID_NUMBER_PERCENT:
                rSet.Put( SfxBoolItem(nWhich, (nType & css::util::NumberFormat::PERCENT)) );
                break;
            case SID_NUMBER_TIME:
                rSet.Put( SfxBoolItem(nWhich, (nType & css::util::NumberFormat::TIME)) );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

void ScFormatShell::ExecuteTextDirection( SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell = GetViewData()->GetViewShell();
    pTabViewShell->HideListBox();               // Autofilter-DropDown-Listbox
    bool bEditMode = false;
    if ( GetViewData()->HasEditView( GetViewData()->GetActivePart() ) )
    {
        bEditMode=true;
        SC_MOD()->InputEnterHandler();
        pTabViewShell->UpdateInputHandler();
    }
    sal_uInt16 nSlot = rReq.GetSlot();
    switch( nSlot )
    {
        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
        {
            bool bVert = (nSlot == SID_TEXTDIRECTION_TOP_TO_BOTTOM);
            ScPatternAttr aAttr( GetViewData()->GetDocument()->GetPool() );
            SfxItemSet& rItemSet = aAttr.GetItemSet();
            rItemSet.Put( SfxBoolItem( ATTR_STACKED, bVert ) );
            rItemSet.Put( SfxBoolItem( ATTR_VERTICAL_ASIAN, bVert ) );
            pTabViewShell->ApplySelectionPattern( aAttr );
            pTabViewShell->AdjustBlockHeight();
        }
        break;

        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
            SvxFrameDirection eDirection = ( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT ) ?
                                                FRMDIR_HORI_LEFT_TOP : FRMDIR_HORI_RIGHT_TOP;
            pTabViewShell->ApplyAttr( SvxFrameDirectionItem( eDirection, ATTR_WRITINGDIR ) );
        }
        break;
    }
    if (bEditMode)
        SC_MOD()->SetInputMode( SC_INPUT_TABLE );
}

void ScFormatShell::GetTextDirectionState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell = GetViewData()->GetViewShell();
    const SfxItemSet& rAttrSet = pTabViewShell->GetSelectionPattern()->GetItemSet();

    bool bVertDontCare =
        (rAttrSet.GetItemState( ATTR_VERTICAL_ASIAN ) == SfxItemState::DONTCARE) ||
        (rAttrSet.GetItemState( ATTR_STACKED ) == SfxItemState::DONTCARE);
    bool bLeftRight = !bVertDontCare &&
        !static_cast<const SfxBoolItem&>(rAttrSet.Get( ATTR_STACKED )).GetValue();
    bool bTopBottom = !bVertDontCare && !bLeftRight &&
        static_cast<const SfxBoolItem&>(rAttrSet.Get( ATTR_VERTICAL_ASIAN )).GetValue();

    bool bBidiDontCare = (rAttrSet.GetItemState( ATTR_WRITINGDIR ) == SfxItemState::DONTCARE);
    EEHorizontalTextDirection eBidiDir = EE_HTEXTDIR_DEFAULT;
    if ( !bBidiDontCare )
    {
        SvxFrameDirection eCellDir = (SvxFrameDirection)static_cast<const SvxFrameDirectionItem&>(
                                        rAttrSet.Get( ATTR_WRITINGDIR )).GetValue();
        if ( eCellDir == FRMDIR_ENVIRONMENT )
            eBidiDir = (EEHorizontalTextDirection)GetViewData()->GetDocument()->
                                GetEditTextDirection( GetViewData()->GetTabNo() );
        else if ( eCellDir == FRMDIR_HORI_RIGHT_TOP )
            eBidiDir = EE_HTEXTDIR_R2L;
        else
            eBidiDir = EE_HTEXTDIR_L2R;
    }

    SvtLanguageOptions  aLangOpt;
    bool bDisableCTLFont = !aLangOpt.IsCTLFontEnabled();
    bool bDisableVerticalText = !aLangOpt.IsVerticalTextEnabled();

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        switch( nWhich )
        {
            case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
            case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
                if ( bDisableVerticalText )
                    rSet.DisableItem( nWhich );
                else
                {
                    if( bVertDontCare )
                        rSet.InvalidateItem( nWhich );
                    else if ( nWhich == SID_TEXTDIRECTION_LEFT_TO_RIGHT )
                        rSet.Put( SfxBoolItem( nWhich, bLeftRight ) );
                    else
                        rSet.Put( SfxBoolItem( nWhich, bTopBottom ) );
                }
            break;

            case SID_ATTR_PARA_LEFT_TO_RIGHT:
            case SID_ATTR_PARA_RIGHT_TO_LEFT:
                if ( bDisableCTLFont )
                    rSet.DisableItem( nWhich );
                else
                {
                    if ( bTopBottom )
                        rSet.DisableItem( nWhich );
                    else if ( bBidiDontCare )
                        rSet.InvalidateItem( nWhich );
                    else if ( nWhich == SID_ATTR_PARA_LEFT_TO_RIGHT )
                        rSet.Put( SfxBoolItem( nWhich, eBidiDir == EE_HTEXTDIR_L2R ) );
                    else
                        rSet.Put( SfxBoolItem( nWhich, eBidiDir == EE_HTEXTDIR_R2L ) );
                }
        }
        nWhich = aIter.NextWhich();
    }
}

void ScFormatShell::ExecFormatPaintbrush( SfxRequest& rReq )
{
    ScViewFunc* pView = pViewData->GetView();
    if ( pView->HasPaintBrush() )
    {
        // cancel paintbrush mode
        pView->ResetBrushDocument();
    }
    else
    {
        bool bLock = false;
        const SfxItemSet *pArgs = rReq.GetArgs();
        if( pArgs && pArgs->Count() >= 1 )
            bLock = static_cast<const SfxBoolItem&>(pArgs->Get(SID_FORMATPAINTBRUSH)).GetValue();

        // in case of multi selection, deselect all and use the cursor position
        ScRange aDummy;
        if ( pViewData->GetSimpleArea(aDummy) != SC_MARK_SIMPLE )
            pView->Unmark();

        ScDocument* pBrushDoc = new ScDocument( SCDOCMODE_CLIP );
        pView->CopyToClip( pBrushDoc, false, true );
        pView->SetBrushDocument( pBrushDoc, bLock );
    }
}

void ScFormatShell::StateFormatPaintbrush( SfxItemSet& rSet )
{
    if ( pViewData->HasEditView( pViewData->GetActivePart() ) )
        rSet.DisableItem( SID_FORMATPAINTBRUSH );
    else
        rSet.Put( SfxBoolItem( SID_FORMATPAINTBRUSH, pViewData->GetView()->HasPaintBrush() ) );
}

short ScFormatShell::GetCurrentNumberFormatType()
{
    short nType = css::util::NumberFormat::ALL;
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData aMark(GetViewData()->GetMarkData());
    const SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
    if (!pFormatter)
        return nType;

    // TODO: Find out how to get a selected table range in case multiple tables
    // are selected.  Currently we only check for the current active table.

    if ( aMark.IsMarked() || aMark.IsMultiMarked() )
    {
        aMark.MarkToMulti();
        ScRange aRange;
        aMark.GetMultiMarkArea(aRange);

        const ScMultiSel& rMultiSel = aMark.GetMultiSelData();

        short nComboType = css::util::NumberFormat::ALL;
        bool bFirstItem = true;
        for (SCCOL nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
        {
            if (!rMultiSel.HasMarks(nCol))
                continue;

            SCROW nRow1, nRow2;
            ScMultiSelIter aMultiIter(rMultiSel, nCol);
            while (aMultiIter.Next(nRow1, nRow2))
            {
                ScRange aColRange(nCol, nRow1, aRange.aStart.Tab());
                aColRange.aEnd.SetRow(nRow2);
                sal_uInt32 nNumFmt = pDoc->GetNumberFormat(aColRange);
                const SvNumberformat* pEntry = pFormatter->GetEntry(nNumFmt);
                if (!pEntry)
                    return 0;

                short nThisType = pEntry->GetType();
                if (bFirstItem)
                {
                    bFirstItem = false;
                    nComboType = nThisType;
                }
                else if (nComboType != nThisType)
                    // mixed number format type.
                    return css::util::NumberFormat::ALL;
            }
        }
        nType = nComboType;
    }
    else
    {
        sal_uInt32 nNumFmt;
        pDoc->GetNumberFormat( pViewData->GetCurX(), pViewData->GetCurY(),
                               pViewData->GetTabNo(), nNumFmt );
        const SvNumberformat* pEntry = pFormatter->GetEntry( nNumFmt );
        nType = pEntry ? pEntry->GetType() : 0;
    }
    return nType;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
