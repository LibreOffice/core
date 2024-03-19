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

#include <scitems.hxx>
#include <editeng/borderline.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxdlg.hxx>
#include <svl/whiter.hxx>

#include <svl/stritem.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <svl/languageoptions.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/langitem.hxx>
#include <svx/numinf.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <formatsh.hxx>
#include <sc.hrc>
#include <globstr.hrc>
#include <scresid.hxx>
#include <docsh.hxx>
#include <patattr.hxx>
#include <scmod.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <printfun.hxx>
#include <docpool.hxx>
#include <tabvwsh.hxx>
#include <attrib.hxx>

#define ShellClass_ScFormatShell
#define ShellClass_TableFont
#define ShellClass_FormatForSelection
#include <scslots.hxx>

#include <editeng/fontitem.hxx>
#include <sfx2/classificationhelper.hxx>

#include <memory>

using namespace ::com::sun::star;

namespace {

SvxCellHorJustify lclConvertSlotToHAlign( sal_uInt16 nSlot )
{
    SvxCellHorJustify eHJustify = SvxCellHorJustify::Standard;
    switch( nSlot )
    {
        case SID_ALIGN_ANY_HDEFAULT:    eHJustify = SvxCellHorJustify::Standard;   break;
        case SID_ALIGN_ANY_LEFT:        eHJustify = SvxCellHorJustify::Left;       break;
        case SID_ALIGN_ANY_HCENTER:     eHJustify = SvxCellHorJustify::Center;     break;
        case SID_ALIGN_ANY_RIGHT:       eHJustify = SvxCellHorJustify::Right;      break;
        case SID_ALIGN_ANY_JUSTIFIED:   eHJustify = SvxCellHorJustify::Block;      break;
        default:    OSL_FAIL( "lclConvertSlotToHAlign - invalid slot" );
    }
    return eHJustify;
}

SvxCellVerJustify lclConvertSlotToVAlign( sal_uInt16 nSlot )
{
    SvxCellVerJustify eVJustify = SvxCellVerJustify::Standard;
    switch( nSlot )
    {
        case SID_ALIGN_ANY_VDEFAULT:    eVJustify = SvxCellVerJustify::Standard;   break;
        case SID_ALIGN_ANY_TOP:         eVJustify = SvxCellVerJustify::Top;        break;
        case SID_ALIGN_ANY_VCENTER:     eVJustify = SvxCellVerJustify::Center;     break;
        case SID_ALIGN_ANY_BOTTOM:      eVJustify = SvxCellVerJustify::Bottom;     break;
        default:    OSL_FAIL( "lclConvertSlotToVAlign - invalid slot" );
    }
    return eVJustify;
}

} // namespace


SFX_IMPL_INTERFACE(ScFormatShell, SfxShell)

void ScFormatShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT,
                                            SfxVisibilityFlags::Standard | SfxVisibilityFlags::Server,
                                            ToolbarId::Objectbar_Format);
}

ScFormatShell::ScFormatShell(ScViewData& rData) :
    SfxShell(rData.GetViewShell()),
    rViewData(rData)
{
    ScTabViewShell* pTabViewShell = GetViewData().GetViewShell();

    SetPool( &pTabViewShell->GetPool() );
    SfxUndoManager* pMgr = rViewData.GetSfxDocShell()->GetUndoManager();
    SetUndoManager( pMgr );
    if (pMgr && !rViewData.GetDocument().IsUndoEnabled())
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
    SetName("Format");
}

ScFormatShell::~ScFormatShell()
{
}

void ScFormatShell::ExecuteStyle( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const sal_uInt16  nSlotId = rReq.GetSlot();

    ScDocShell*         pDocSh      = GetViewData().GetDocShell();
    ScTabViewShell*     pTabViewShell= GetViewData().GetViewShell();
    ScDocument&         rDoc        = pDocSh->GetDocument();
    SfxStyleSheetBasePool*  pStylePool  = rDoc.GetStyleSheetPool();

    if ( (nSlotId == SID_STYLE_PREVIEW)
        || (nSlotId ==  SID_STYLE_END_PREVIEW) )
    {
        if (nSlotId == SID_STYLE_PREVIEW)
        {
            SfxStyleFamily eFamily = SfxStyleFamily::Para;
            const SfxUInt16Item* pFamItem;
            if ( pArgs && (pFamItem = pArgs->GetItemIfSet( SID_STYLE_FAMILY )) )
                eFamily = static_cast<SfxStyleFamily>(pFamItem->GetValue());
            const SfxPoolItem* pNameItem;
            OUString aStyleName;
            if (pArgs && SfxItemState::SET == pArgs->GetItemState( nSlotId, true, &pNameItem ))
                aStyleName = static_cast<const SfxStringItem*>(pNameItem)->GetValue();
            if ( eFamily == SfxStyleFamily::Para ) // CellStyles
            {
                ScMarkData aFuncMark( rViewData.GetMarkData() );
                ScViewUtil::UnmarkFiltered( aFuncMark, rDoc );
                aFuncMark.MarkToMulti();

                if ( !aFuncMark.IsMarked() && !aFuncMark.IsMultiMarked() )
                {
                    SCCOL nCol = rViewData.GetCurX();
                    SCROW nRow = rViewData.GetCurY();
                    SCTAB nTab = rViewData.GetTabNo();
                    ScRange aRange( nCol, nRow, nTab );
                    aFuncMark.SetMarkArea( aRange );
                }
                rDoc.SetPreviewSelection( aFuncMark );
                ScStyleSheet* pPreviewStyle = static_cast<ScStyleSheet*>( pStylePool->Find( aStyleName, eFamily ) );
                rDoc.SetPreviewCellStyle( pPreviewStyle  );
                ScPatternAttr aAttr( *rDoc.GetSelectionPattern( aFuncMark ) );
                aAttr.SetStyleSheet( pPreviewStyle );

                SfxItemSet aItemSet( GetPool() );

                ScPatternAttr aNewAttrs(GetViewData().GetDocument().getCellAttributeHelper());
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

                ScPatternAttr aNewAttrs(GetViewData().GetDocument().getCellAttributeHelper());
                SfxItemSet& rNewSet = aNewAttrs.GetItemSet();
                rNewSet.Put( aItemSet, false );
                rDoc.ApplySelectionPattern( aNewAttrs, aPreviewMark );
                pTabViewShell->UpdateSelectionArea( aPreviewMark, &aAttr );
            }
        }
    }
    else if (nSlotId == SID_CLASSIFICATION_APPLY)
    {
        const SfxPoolItem* pItem = nullptr;
        if (pArgs && pArgs->GetItemState(nSlotId, false, &pItem) == SfxItemState::SET)
        {
            const OUString& rName = static_cast<const SfxStringItem*>(pItem)->GetValue();
            SfxClassificationHelper aHelper(pDocSh->getDocProperties());
            auto eType = SfxClassificationPolicyType::IntellectualProperty;
            if (const SfxStringItem* pNameItem = pArgs->GetItemIfSet(SID_TYPE_NAME, false))
            {
                const OUString& rType = pNameItem->GetValue();
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
    ScTabViewShell*     pTabViewShell   = GetViewData().GetViewShell();
    const SfxItemSet*   pReqArgs        = rReq.GetArgs();
    sal_uInt16          nSlot           = rReq.GetSlot();
    SfxBindings&        rBindings       = pTabViewShell->GetViewFrame().GetBindings();

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox

    // End input
    if ( GetViewData().HasEditView( GetViewData().GetActivePart() ) )
    {
        switch ( nSlot )
        {
            case SID_NUMBER_TYPE_FORMAT:
            case SID_NUMBER_TWODEC:
            case SID_NUMBER_SCIENTIFIC:
            case SID_NUMBER_DATE:
            case SID_NUMBER_CURRENCY:
            case SID_NUMBER_PERCENT:
            case SID_NUMBER_STANDARD:
            case SID_NUMBER_FORMAT:
            case SID_NUMBER_INCDEC:
            case SID_NUMBER_DECDEC:
            case SID_NUMBER_THOUSANDS:
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

    SvNumFormatType nType = GetCurrentNumberFormatType();
    switch ( nSlot )
    {
        case SID_NUMBER_TWODEC:
        {
            const SfxItemSet& rAttrSet = pTabViewShell->GetSelectionPattern()->GetItemSet();
            sal_uInt32 nNumberFormat = rAttrSet.Get(ATTR_VALUE_FORMAT).GetValue();

            if ((nType & SvNumFormatType::NUMBER) && nNumberFormat == 4)
                pTabViewShell->SetNumberFormat( SvNumFormatType::NUMBER );
            else
                pTabViewShell->SetNumberFormat( SvNumFormatType::NUMBER, 4 );
            rBindings.Invalidate( nSlot );
            rReq.Done();
        }
        break;
        case SID_NUMBER_SCIENTIFIC:
            if (nType & SvNumFormatType::SCIENTIFIC)
                pTabViewShell->SetNumberFormat( SvNumFormatType::NUMBER );
            else
                pTabViewShell->SetNumberFormat( SvNumFormatType::SCIENTIFIC );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_DATE:
            if (nType & SvNumFormatType::DATE)
                pTabViewShell->SetNumberFormat( SvNumFormatType::NUMBER );
            else
                pTabViewShell->SetNumberFormat( SvNumFormatType::DATE );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_TIME:
            if (nType & SvNumFormatType::TIME)
                pTabViewShell->SetNumberFormat( SvNumFormatType::NUMBER );
            else
                pTabViewShell->SetNumberFormat( SvNumFormatType::TIME );
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
                    ScDocument& rDoc = rViewData.GetDocument();
                    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
                    const SfxItemSet& rOldSet = pTabViewShell->GetSelectionPattern()->GetItemSet();

                    LanguageType eOldLang = rOldSet.Get( ATTR_LANGUAGE_FORMAT ).GetLanguage();
                    sal_uInt32 nOldFormat = rOldSet.Get( ATTR_VALUE_FORMAT ).GetValue();

                    if ( nOldFormat != nNewFormat )
                    {
                        const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewFormat );
                        ScPatternAttr aNewAttrs(rDoc.getCellAttributeHelper());
                        SfxItemSet& rSet = aNewAttrs.GetItemSet();
                        LanguageType eNewLang = pNewEntry ? pNewEntry->GetLanguage() : LANGUAGE_DONTKNOW;
                        if ( eNewLang != eOldLang && eNewLang != LANGUAGE_DONTKNOW )
                            rSet.Put( SvxLanguageItem( eNewLang, ATTR_LANGUAGE_FORMAT ) );
                        rSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, nNewFormat ) );
                        pTabViewShell->ApplySelectionPattern( aNewAttrs );
                    }
                    else
                        pTabViewShell->SetNumberFormat( SvNumFormatType::NUMBER );
                }
            }
            else
            {
                if ( nType & SvNumFormatType::CURRENCY )
                    pTabViewShell->SetNumberFormat( SvNumFormatType::NUMBER );
                else
                    pTabViewShell->SetNumberFormat( SvNumFormatType::CURRENCY );
            }
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_PERCENT:
            if (nType & SvNumFormatType::PERCENT)
                pTabViewShell->SetNumberFormat( SvNumFormatType::NUMBER );
            else
                pTabViewShell->SetNumberFormat( SvNumFormatType::PERCENT );
            rBindings.Invalidate( nSlot );
            rReq.Done();
            break;
        case SID_NUMBER_STANDARD:
            pTabViewShell->SetNumberFormat( SvNumFormatType::NUMBER );
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
        case SID_NUMBER_THOUSANDS:
        {
            ScDocument& rDoc = rViewData.GetDocument();
            SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
            bool bThousand(false);
            bool bNegRed(false);
            sal_uInt16 nPrecision(0);
            sal_uInt16 nLeadZeroes(0);
            LanguageType eLanguage = ScGlobal::eLnge;

            sal_uInt32 nCurrentNumberFormat = rDoc.GetNumberFormat(
                rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo());
            const SvNumberformat* pEntry = pFormatter->GetEntry(nCurrentNumberFormat);

            if (pEntry)
                eLanguage = pEntry->GetLanguage();

            pFormatter->GetFormatSpecialInfo(nCurrentNumberFormat, bThousand, bNegRed, nPrecision, nLeadZeroes);
            bThousand = !bThousand;
            OUString aCode = pFormatter->GenerateFormat(
                nCurrentNumberFormat,
                eLanguage,
                bThousand,
                bNegRed,
                nPrecision,
                nLeadZeroes);
            pTabViewShell->SetNumFmtByStr(aCode);

            rBindings.Invalidate(nSlot);
            rReq.Done();
        }
        break;
        case SID_NUMBER_FORMAT:
            // symphony version with format interpretation
            if(pReqArgs)
            {
                const SfxPoolItem* pItem;
                ScDocument& rDoc = rViewData.GetDocument();
                SvNumberFormatter* pFormatter = rDoc.GetFormatTable();

                sal_uInt32 nCurrentNumberFormat = rDoc.GetNumberFormat(
                    rViewData.GetCurX(), rViewData.GetCurY(), rViewData.GetTabNo());
                const SvNumberformat* pEntry = pFormatter->GetEntry(nCurrentNumberFormat);

                if(!pEntry)
                    break;

                LanguageType eLanguage = pEntry->GetLanguage();
                SvNumFormatType eType = pEntry->GetMaskedType();

                //Just use eType to judge whether the command is fired for NUMBER/PERCENT/CURRENCY/SCIENTIFIC/FRACTION/TIME
                //In sidebar, users can fire SID_NUMBER_FORMAT command by operating the related UI controls before they are disable
                if(!(eType == SvNumFormatType::ALL
                     || eType == SvNumFormatType::NUMBER
                     || eType == SvNumFormatType::PERCENT
                     || eType == SvNumFormatType::CURRENCY
                     || eType == SvNumFormatType::SCIENTIFIC
                     || eType == SvNumFormatType::TIME
                     || eType == SvNumFormatType::FRACTION))
                    pEntry = nullptr;

                if(SfxItemState::SET == pReqArgs->GetItemState(nSlot, true, &pItem) && pEntry)
                {
                    OUString aCode = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    sal_uInt16 aLen = aCode.getLength();
                    std::unique_ptr<OUString[]> sFormat( new OUString[4] );
                    OUStringBuffer sTmpStr;
                    sal_uInt16 nCount(0);
                    sal_uInt16 nStrCount(0);

                    while(nCount < aLen)
                    {
                        sal_Unicode cChar = aCode[nCount];

                        if(cChar == ',')
                        {
                            sFormat[nStrCount] = sTmpStr.makeStringAndClear();
                            nStrCount++;
                        }
                        else
                        {
                            sTmpStr.append(cChar);
                        }

                        nCount++;

                        if(nStrCount > 3)
                            break;
                    }

                    const bool bThousand = static_cast<bool>(sFormat[0].toInt32());
                    const bool bNegRed = static_cast<bool>(sFormat[1].toInt32());
                    const sal_uInt16 nPrecision = static_cast<sal_uInt16>(sFormat[2].toInt32());
                    const sal_uInt16 nLeadZeroes = static_cast<sal_uInt16>(sFormat[3].toInt32());

                    aCode = pFormatter->GenerateFormat(
                        nCurrentNumberFormat,//modify
                        eLanguage,
                        bThousand,
                        bNegRed,
                        nPrecision,
                        nLeadZeroes);
                    pTabViewShell->SetNumFmtByStr(aCode);
                }
            }
            break;

        case SID_ATTR_NUMBERFORMAT_VALUE:
            if ( pReqArgs )
            {
                if ( const SfxUInt32Item* pItem = pReqArgs->GetItemIfSet( ATTR_VALUE_FORMAT ) )
                {
                    // We have to accomplish this using ApplyAttributes()
                    // because we also need the language information to be
                    // considered.
                    const SfxItemSet& rOldSet =
                        pTabViewShell->GetSelectionPattern()->GetItemSet();
                    SfxItemPool* pDocPool = GetViewData().GetDocument().GetPool();
                    SfxItemSetFixed<ATTR_PATTERN_START, ATTR_PATTERN_END> aNewSet( *pDocPool );
                    aNewSet.Put( *pItem );
                    pTabViewShell->ApplyAttributes( aNewSet, rOldSet );
                }
            }
            break;

        case SID_NUMBER_TYPE_FORMAT:
            if ( pReqArgs )
            {
                const SfxPoolItem* pItem;
                if ( pReqArgs->GetItemState( nSlot, true, &pItem ) == SfxItemState::SET )
                {
                    sal_uInt16 nFormat = static_cast<const SfxUInt16Item *>(pItem)->GetValue();
                    switch(nFormat)
                    {
                    case 0:
                        pTabViewShell->SetNumberFormat( SvNumFormatType::NUMBER); //Modify
                        break;
                    case 1:
                        pTabViewShell->SetNumberFormat( SvNumFormatType::NUMBER, 2 ); //Modify
                        break;
                    case 2:
                        pTabViewShell->SetNumberFormat( SvNumFormatType::PERCENT );
                        break;
                    case 3:
                        pTabViewShell->SetNumberFormat( SvNumFormatType::CURRENCY );
                        break;
                    case 4:
                        pTabViewShell->SetNumberFormat( SvNumFormatType::DATE );
                        break;
                    case 5:
                        pTabViewShell->SetNumberFormat( SvNumFormatType::TIME );
                        break;
                    case 6:
                        pTabViewShell->SetNumberFormat( SvNumFormatType::SCIENTIFIC );
                        break;
                    case 7:
                        pTabViewShell->SetNumberFormat( SvNumFormatType::FRACTION );
                        break;
                    case 8:
                        pTabViewShell->SetNumberFormat( SvNumFormatType::LOGICAL );
                        break;
                    case 9:
                        pTabViewShell->SetNumberFormat( SvNumFormatType::TEXT );
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
    ScTabViewShell* pTabViewShell       = GetViewData().GetViewShell();
    SfxBindings&            rBindings   = rViewData.GetBindings();
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
                if( pSet->GetItemState(GetPool().GetWhichIDFromSlotID(nSlot), true, &pItem  ) == SfxItemState::SET )
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
                            SvxCellHorJustify eJust = static_cast<const SvxHorJustifyItem*>(pItem)->GetValue();
                            // #i78476# update alignment of text in cell edit mode
                            pTabViewShell->UpdateInputHandlerCellAdjust( eJust );
                            pTabViewShell->ApplyAttr( SvxHorJustifyItem( eJust, ATTR_HOR_JUSTIFY ) );
                        }
                        break;
                        case SID_V_ALIGNCELL:
                            pTabViewShell->ApplyAttr( SvxVerJustifyItem( static_cast<const SvxVerJustifyItem*>(pItem)->GetValue(), ATTR_VER_JUSTIFY ) );
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
    ScTabViewShell* pTabViewShell       = GetViewData().GetViewShell();
    SfxBindings&            rBindings   = rViewData.GetBindings();
    const ScPatternAttr*    pAttrs      = pTabViewShell->GetSelectionPattern();
    const SfxItemSet*       pSet        = rReq.GetArgs();
    sal_uInt16                  nSlot       = rReq.GetSlot();
    std::optional<SfxAllItemSet> pNewSet;

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox

    if (  (nSlot == SID_ATTR_CHAR_WEIGHT)
        ||(nSlot == SID_ATTR_CHAR_POSTURE)
        ||(nSlot == SID_ATTR_CHAR_UNDERLINE)
        ||(nSlot == SID_ULINE_VAL_NONE)
        ||(nSlot == SID_ULINE_VAL_SINGLE)
        ||(nSlot == SID_ULINE_VAL_DOUBLE)
        ||(nSlot == SID_ULINE_VAL_DOTTED) )
    {
        pNewSet.emplace( GetPool() );

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
                    if( pSet )
                    {
                        const SfxPoolItem& rUnderline = pSet->Get( ATTR_FONT_UNDERLINE );

                        if( dynamic_cast<const SvxUnderlineItem*>( &rUnderline) !=  nullptr )
                        {
                            pTabViewShell->ApplyAttr( rUnderline );
                            pNewSet->Put( rUnderline );
                        }
                        else if ( auto pTextLineItem = dynamic_cast<const SvxTextLineItem*>( &rUnderline) )
                        {
                            // #i106580# also allow SvxTextLineItem (base class of SvxUnderlineItem)
                            SvxUnderlineItem aNewItem( pTextLineItem->GetLineStyle(), pTextLineItem->Which() );
                            aNewItem.SetColor( pTextLineItem->GetColor() );
                            pTabViewShell->ApplyAttr( aNewItem );
                            pNewSet->Put( aNewItem );
                        }
                    }
                    else
                    {
                        SvxUnderlineItem aUnderline( pAttrs->GetItem( ATTR_FONT_UNDERLINE ) );
                        FontLineStyle eUnderline = (LINESTYLE_NONE != aUnderline.GetLineStyle())
                                    ? LINESTYLE_NONE
                                    : LINESTYLE_SINGLE;
                        aUnderline.SetLineStyle( eUnderline );
                        pTabViewShell->ApplyAttr( aUnderline );
                        pNewSet->Put( aUnderline );
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
                    FontLineStyle eOld = pAttrs->GetItem(ATTR_FONT_UNDERLINE).GetLineStyle();
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
         * "Self-made" functionality of radio buttons
         * At the toggle the default state is used, this means
         * no button was clicked.
         */

        const SfxItemSet&        rAttrSet   = pTabViewShell->GetSelectionPattern()->GetItemSet();
        const SvxHorJustifyItem* pHorJustify = rAttrSet.GetItemIfSet(ATTR_HOR_JUSTIFY);
        const SvxVerJustifyItem* pVerJustify = rAttrSet.GetItemIfSet(ATTR_VER_JUSTIFY );
        SvxCellHorJustify        eHorJustify = SvxCellHorJustify::Standard;
        SvxCellVerJustify        eVerJustify = SvxCellVerJustify::Standard;

        if (pHorJustify)
        {
            eHorJustify = pHorJustify->GetValue();
        }
        if (pVerJustify)
        {
            eVerJustify = pVerJustify->GetValue();
        }

        switch ( nSlot )
        {
            case SID_ALIGNLEFT:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SvxCellHorJustify::Left) ?
                    SvxCellHorJustify::Left : SvxCellHorJustify::Standard, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNRIGHT:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SvxCellHorJustify::Right) ?
                    SvxCellHorJustify::Right : SvxCellHorJustify::Standard, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNCENTERHOR:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SvxCellHorJustify::Center) ?
                    SvxCellHorJustify::Center : SvxCellHorJustify::Standard, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNBLOCK:
                rReq.SetSlot( SID_H_ALIGNCELL );
                rReq.AppendItem( SvxHorJustifyItem(
                    !pHorJustify || (eHorJustify != SvxCellHorJustify::Block) ?
                    SvxCellHorJustify::Block : SvxCellHorJustify::Standard, SID_H_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNTOP:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SvxCellVerJustify::Top) ?
                    SvxCellVerJustify::Top : SvxCellVerJustify::Standard, SID_V_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNBOTTOM:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SvxCellVerJustify::Bottom) ?
                    SvxCellVerJustify::Bottom : SvxCellVerJustify::Standard, SID_V_ALIGNCELL ) );
                ExecuteSlot( rReq, GetInterface() );
                return;

            case SID_ALIGNCENTERVER:
                rReq.SetSlot( SID_V_ALIGNCELL );
                rReq.AppendItem( SvxVerJustifyItem(
                    !pVerJustify || (eVerJustify != SvxCellVerJustify::Center) ?
                    SvxCellVerJustify::Center : SvxCellVerJustify::Standard, SID_V_ALIGNCELL ) );
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
        pNewSet.reset();
    }
    else
    {
        rReq.Done();
    }

}

void ScFormatShell::ExecuteAttr( SfxRequest& rReq )
{
    ScTabViewShell*     pTabViewShell = GetViewData().GetViewShell();
    SfxBindings&        rBindings = rViewData.GetBindings();
    const SfxItemSet*   pNewAttrs = rReq.GetArgs();
    sal_uInt16          nSlot = rReq.GetSlot();

    pTabViewShell->HideListBox();                   // Autofilter-DropDown-Listbox
    ScDocument& rDoc = GetViewData().GetDocument();
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
                rDoc.SetPreviewFont(nullptr);
                pTabViewShell->UpdateSelectionArea( rDoc.GetPreviewSelection() );
                break;
            }
            case SID_ATTR_CHAR_COLOR:
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
                pTabViewShell->ExecuteCellFormatDlg(rReq, "font");       // when ToolBar is vertical
                break;

            case SID_BACKGROUND_COLOR:
                {
                    SvxBrushItem aBrushItem(
                                     pTabViewShell->GetSelectionPattern()->GetItem( ATTR_BACKGROUND ) );
                    aBrushItem.SetColor( COL_TRANSPARENT );
                    pTabViewShell->ApplyAttr( aBrushItem, false );
                }
                break;

            case SID_ATTR_ALIGN_LINEBREAK:                  // without parameter as toggle
                {
                    const ScPatternAttr* pAttrs = pTabViewShell->GetSelectionPattern();
                    bool bOld = pAttrs->GetItem(ATTR_LINEBREAK).GetValue();
                    ScLineBreakCell aBreakItem(!bOld);
                    pTabViewShell->ApplyAttr( aBreakItem );

                    SfxAllItemSet aNewSet( GetPool() );
                    aNewSet.Put( aBreakItem );
                    rReq.Done( aNewSet );

                    rBindings.Invalidate( nSlot );
                }
                break;

            case SID_SCATTR_CELLPROTECTION:                  // without parameter as toggle
                {
                    const ScPatternAttr* pAttrs = pTabViewShell->GetSelectionPattern();
                    bool bProtect = pAttrs->GetItem(ATTR_PROTECTION).GetProtection();
                    bool bHideFormula = pAttrs->GetItem(ATTR_PROTECTION).GetHideFormula();
                    bool bHideCell = pAttrs->GetItem(ATTR_PROTECTION).GetHideCell();
                    bool bHidePrint = pAttrs->GetItem(ATTR_PROTECTION).GetHidePrint();

                    ScProtectionAttr aProtectionItem( !bProtect, bHideFormula, bHideCell, bHidePrint );
                    pTabViewShell->ApplyAttr( aProtectionItem );

                    SfxAllItemSet aNewSet( GetPool() );
                    aNewSet.Put( aProtectionItem );
                    aNewSet.Put( SfxBoolItem( SID_SCATTR_CELLPROTECTION, !bProtect ) );
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
                sal_uInt16 nWhich = rPool.GetWhichIDFromSlotID( nSlot );
                const SvxFontItem& rFont = static_cast<const SvxFontItem&>(pNewAttrs->Get( nWhich ));
                SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, rPool );
                SvtScriptType nScript = pTabViewShell->GetSelectionScriptType();
                aSetItem.PutItemForScriptType( nScript, rFont );

                ScMarkData aFuncMark( rViewData.GetMarkData() );
                ScViewUtil::UnmarkFiltered( aFuncMark, rDoc );
                rDoc.SetPreviewFont( aSetItem.GetItemSet().Clone() );
                aFuncMark.MarkToMulti();

                if ( !aFuncMark.IsMarked() && !aFuncMark.IsMultiMarked() )
                {
                    SCCOL nCol = rViewData.GetCurX();
                    SCROW nRow = rViewData.GetCurY();
                    SCTAB nTab = rViewData.GetTabNo();
                    ScRange aRange( nCol, nRow, nTab );
                    aFuncMark.SetMarkArea( aRange );
                }
                rDoc.SetPreviewSelection( aFuncMark );
                pTabViewShell->UpdateSelectionArea( aFuncMark );
                break;
            }
            case SID_ATTR_CHAR_OVERLINE:
            case SID_ATTR_CHAR_STRIKEOUT:
            case SID_ATTR_ALIGN_LINEBREAK:
            case SID_ATTR_CHAR_CONTOUR:
            case SID_ATTR_CHAR_SHADOWED:
            case SID_ATTR_CHAR_RELIEF:
                pTabViewShell->ApplyAttr( pNewAttrs->Get( pNewAttrs->GetPool()->GetWhichIDFromSlotID( nSlot ) ) );
                rBindings.Invalidate( nSlot );
                rBindings.Update( nSlot );
                break;
            case SID_ATTR_CHAR_COLOR:
            case SID_SCATTR_PROTECTION :
            {
                pTabViewShell->ApplyAttr( pNewAttrs->Get( pNewAttrs->GetPool()->GetWhichIDFromSlotID( nSlot) ), false);

                rBindings.Invalidate( nSlot );
                rBindings.Update( nSlot );
            }

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
                    sal_uInt16 nWhich = rPool.GetWhichIDFromSlotID( nSlot );
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
                                pNewAttrs->Get( SID_FRAME_LINESTYLE ).
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
                                SvxBorderLineStyle::SOLID );
                        pTabViewShell->SetDefaultFrameLine( &aDefLine );
                        pTabViewShell->SetSelectionFrameLines( nullptr, false );
                    }
                }
                break;

            case SID_FRAME_LINECOLOR:
                {
                    ::editeng::SvxBorderLine*  pDefLine = pTabViewShell->GetDefaultFrameLine();

                    Color aColor = pNewAttrs->Get( SID_FRAME_LINECOLOR ).GetValue();

                    // Update default line
                    if ( pDefLine )
                    {
                        pDefLine->SetColor( aColor );
                        pTabViewShell->SetSelectionFrameLines( pDefLine, true );
                    }
                    else
                    {
                        ::editeng::SvxBorderLine aDefLine( &aColor, 20, SvxBorderLineStyle::SOLID );
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
                    SfxItemSetFixed<ATTR_PATTERN_START, ATTR_PATTERN_END> aOldSet( *rDoc.GetPool() );
                    SfxItemSetFixed<ATTR_PATTERN_START, ATTR_PATTERN_END> aNewSet( *rDoc.GetPool() );
                    const SfxPoolItem&      rBorderAttr =
                                                pOldAttrs->GetItemSet().
                                                    Get( ATTR_BORDER );

                    // Evaluate border items from controller:

                    if ( const SvxBoxItem* pBoxItem = pNewAttrs->GetItemIfSet( ATTR_BORDER ) )
                    {
                        //  The SvxFrameToolBoxControl toolbox controller uses a default
                        //  SvxBorderLine (all widths 0) to mark the lines that should be set.
                        //  Macro recording uses a SvxBoxItem with the real values (OutWidth > 0)
                        //  or NULL pointers for no lines.
                        //  -> Substitute existing lines with pDefLine only if widths are 0.
                        SvxBoxItem aBoxItem ( *pBoxItem );
                        if ( aBoxItem.GetTop() && aBoxItem.GetTop()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, SvxBoxItemLine::TOP );
                        if ( aBoxItem.GetBottom() && aBoxItem.GetBottom()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, SvxBoxItemLine::BOTTOM );
                        if ( aBoxItem.GetLeft() && aBoxItem.GetLeft()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, SvxBoxItemLine::LEFT );
                        if ( aBoxItem.GetRight() && aBoxItem.GetRight()->GetOutWidth() == 0 )
                            aBoxItem.SetLine( pDefLine, SvxBoxItemLine::RIGHT );
                        aNewSet.Put( aBoxItem );
                        rReq.AppendItem( aBoxItem );
                    }

                    if ( const SvxBoxInfoItem* pBoxInfoItem = pNewAttrs->GetItemIfSet( ATTR_BORDER_INNER ) )
                    {
                        SvxBoxInfoItem aBoxInfoItem( *pBoxInfoItem );
                        if ( aBoxInfoItem.GetHori() && aBoxInfoItem.GetHori()->GetOutWidth() == 0 )
                            aBoxInfoItem.SetLine( pDefLine, SvxBoxInfoItemLine::HORI );
                        if ( aBoxInfoItem.GetVert() && aBoxInfoItem.GetVert()->GetOutWidth() == 0 )
                            aBoxInfoItem.SetLine( pDefLine, SvxBoxInfoItemLine::VERT );
                        aNewSet.Put( aBoxInfoItem );
                        rReq.AppendItem( aBoxInfoItem );
                    }
                    else
                    {
                        SvxBoxInfoItem aBoxInfoItem( ATTR_BORDER_INNER );
                        aBoxInfoItem.SetLine( nullptr, SvxBoxInfoItemLine::HORI );
                        aBoxInfoItem.SetLine( nullptr, SvxBoxInfoItemLine::VERT );
                        aNewSet.Put( aBoxInfoItem );
                    }

                    aOldSet.Put( rBorderAttr );
                    pTabViewShell->ApplyAttributes( aNewSet, aOldSet );
                }
                break;

            case SID_ATTR_BORDER_DIAG_TLBR:
            case SID_ATTR_BORDER_DIAG_BLTR:
                {
                    const ScPatternAttr* pOldAttrs = pTabViewShell->GetSelectionPattern();
                    SfxItemSet aOldSet(pOldAttrs->GetItemSet());
                    SfxItemSet aNewSet(pOldAttrs->GetItemSet());

                    if(SID_ATTR_BORDER_DIAG_TLBR == nSlot)
                    {
                        if(SfxItemState::SET == pNewAttrs->GetItemState(ATTR_BORDER_TLBR))
                        {
                            SvxLineItem aItem(ATTR_BORDER_TLBR);
                            aItem.SetLine(pNewAttrs->Get(ATTR_BORDER_TLBR).GetLine());
                            aNewSet.Put(aItem);
                            rReq.AppendItem(aItem);
                            pTabViewShell->ApplyAttributes(aNewSet, aOldSet);
                        }
                    }
                    else // if( nSlot == SID_ATTR_BORDER_DIAG_BLTR )
                    {
                        if(SfxItemState::SET == pNewAttrs->GetItemState(ATTR_BORDER_BLTR ))
                        {
                            SvxLineItem aItem(ATTR_BORDER_BLTR);
                            aItem.SetLine(pNewAttrs->Get(ATTR_BORDER_BLTR).GetLine());
                            aNewSet.Put(aItem);
                            rReq.AppendItem(aItem);
                            pTabViewShell->ApplyAttributes(aNewSet, aOldSet);
                        }
                    }

                    rBindings.Invalidate(nSlot);
                }
                break;

            // ATTR_BACKGROUND (=SID_ATTR_BRUSH) has to be set to two IDs:
            case SID_BACKGROUND_COLOR:
                {
                    const SvxColorItem&  rNewColorItem = pNewAttrs->Get( SID_BACKGROUND_COLOR );
                    Color aColor = rNewColorItem.GetValue();

                    SvxBrushItem aBrushItem(
                        pTabViewShell->GetSelectionPattern()->GetItem( ATTR_BACKGROUND ) );
                    aBrushItem.SetColor(aColor);
                    aBrushItem.setComplexColor(rNewColorItem.getComplexColor());

                    pTabViewShell->ApplyAttr( aBrushItem, false );
                }
                break;

                case SID_ATTR_BRUSH:
                {
                    SvxBrushItem        aBrushItem( pTabViewShell->GetSelectionPattern()->
                                                GetItem( ATTR_BACKGROUND ) );
                    const SvxBrushItem& rNewBrushItem = static_cast<const SvxBrushItem&>(
                                            pNewAttrs->Get( GetPool().GetWhichIDFromSlotID(nSlot) ) );
                    aBrushItem.SetColor(rNewBrushItem.GetColor());
                    aBrushItem.setComplexColor(rNewBrushItem.getComplexColor());
                    pTabViewShell->ApplyAttr( aBrushItem );
                }
                break;

            case SID_ATTR_BORDER_SHADOW:
                {
                    const SvxShadowItem& rNewShadowItem =
                                            pNewAttrs->Get( ATTR_SHADOW );
                    pTabViewShell->ApplyAttr( rNewShadowItem );
                }
                break;

            default:
            break;
        }

        if( ! rReq.IsAPI() && ! rReq.IsDone() )
            rReq.Done();
    }
}

void ScFormatShell::GetAttrState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData().GetViewShell();
    const SfxItemSet&    rAttrSet   = pTabViewShell->GetSelectionPattern()->GetItemSet();
    const SvxBrushItem&  rBrushItem = rAttrSet.Get( ATTR_BACKGROUND );
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    rSet.Put( rAttrSet, false );

    //  choose font info according to selection script type
    SvtScriptType nScript = SvtScriptType::NONE;      // GetSelectionScriptType never returns 0
    if ( rSet.GetItemState( ATTR_FONT ) != SfxItemState::UNKNOWN )
    {
        nScript = pTabViewShell->GetSelectionScriptType();
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
                if(SfxItemState::INVALID == rAttrSet.GetItemState(ATTR_BACKGROUND))
                {
                    rSet.InvalidateItem(SID_BACKGROUND_COLOR);
                }
            }
            break;
            case SID_FRAME_LINESTYLE:
            case SID_FRAME_LINECOLOR:
            {
                // handled together because both need the cell border information for decisions
                Color aCol;
                editeng::SvxBorderLine aLine(nullptr,0,SvxBorderLineStyle::SOLID);
                bool bCol = false;
                bool bColDisable = false, bStyleDisable = false;
                std::shared_ptr<SvxBoxItem> aBoxItem(std::make_shared<SvxBoxItem>(ATTR_BORDER));
                std::shared_ptr<SvxBoxInfoItem> aInfoItem(std::make_shared<SvxBoxInfoItem>(ATTR_BORDER_INNER));

                pTabViewShell->GetSelectionFrame(aBoxItem, aInfoItem);

                if( aBoxItem->GetTop() )
                {
                    bCol = true;
                    aCol = aBoxItem->GetTop()->GetColor() ;
                    aLine.SetColor(aCol);
                    aLine.SetWidth( aBoxItem->GetTop()->GetWidth());
                    aLine.SetBorderLineStyle( aBoxItem->GetTop()->GetBorderLineStyle());
                }

                if( aBoxItem->GetBottom() )
                {
                    if(!bCol)
                    {
                        bCol = true;
                        aCol = aBoxItem->GetBottom()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aBoxItem->GetBottom()->GetWidth());
                        aLine.SetBorderLineStyle( aBoxItem->GetBottom()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aBoxItem->GetBottom()->GetColor() )
                            bColDisable = true;
                        if( aLine != *aBoxItem->GetBottom() )
                            bStyleDisable = true;
                    }
                }

                if( aBoxItem->GetLeft() )
                {
                    if(!bCol)
                    {
                        bCol = true;
                        aCol = aBoxItem->GetLeft()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aBoxItem->GetLeft()->GetWidth());
                        aLine.SetBorderLineStyle( aBoxItem->GetLeft()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aBoxItem->GetLeft()->GetColor() )
                            bColDisable = true;
                        if( aLine != *aBoxItem->GetLeft() )
                            bStyleDisable = true;
                    }
                }

                if( aBoxItem->GetRight() )
                {
                    if(!bCol)
                    {
                        bCol = true;
                        aCol = aBoxItem->GetRight()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aBoxItem->GetRight()->GetWidth());
                        aLine.SetBorderLineStyle( aBoxItem->GetRight()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aBoxItem->GetRight()->GetColor() )
                            bColDisable = true;
                        if( aLine != *aBoxItem->GetRight() )
                            bStyleDisable = true;
                    }
                }

                if( aInfoItem->GetVert())
                {
                    if(!bCol)
                    {
                        bCol = true;
                        aCol = aInfoItem->GetVert()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aInfoItem->GetVert()->GetWidth());
                        aLine.SetBorderLineStyle( aInfoItem->GetVert()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aInfoItem->GetVert()->GetColor() )
                            bColDisable = true;
                        if( aLine != *aInfoItem->GetVert() )
                            bStyleDisable = true;
                    }
                }

                if( aInfoItem->GetHori())
                {
                    if(!bCol)
                    {
                        bCol = true;
                        aCol = aInfoItem->GetHori()->GetColor() ;
                        aLine.SetColor(aCol);
                        aLine.SetWidth( aInfoItem->GetHori()->GetWidth());
                        aLine.SetBorderLineStyle( aInfoItem->GetHori()->GetBorderLineStyle());
                    }
                    else
                    {
                        if(aCol != aInfoItem->GetHori()->GetColor() )
                            bColDisable = true;
                        if( aLine != *aInfoItem->GetHori() )
                            bStyleDisable = true;
                    }
                }

                if( !aInfoItem->IsValid( SvxBoxInfoItemValidFlags::VERT )
                    || !aInfoItem->IsValid( SvxBoxInfoItemValidFlags::HORI )
                    || !aInfoItem->IsValid( SvxBoxInfoItemValidFlags::LEFT )
                    || !aInfoItem->IsValid( SvxBoxInfoItemValidFlags::RIGHT )
                    || !aInfoItem->IsValid( SvxBoxInfoItemValidFlags::TOP )
                    || !aInfoItem->IsValid( SvxBoxInfoItemValidFlags::BOTTOM ) )
                {
                    bColDisable = true;
                    bStyleDisable = true;
                }

                if(SID_FRAME_LINECOLOR == nWhich)
                {
                    if(bColDisable) // if different lines have different colors
                    {
                        aCol = COL_TRANSPARENT;
                        rSet.Put( SvxColorItem(aCol, SID_FRAME_LINECOLOR ) );
                        rSet.InvalidateItem(SID_FRAME_LINECOLOR);
                    }
                    else if (!bCol) // if no line available
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
                rSet.Put( rBrushItem.CloneSetWhich(GetPool().GetWhichIDFromSlotID(nWhich)) );
            }
            break;
            case SID_SCATTR_CELLPROTECTION:
            {
                bool bProtect = rAttrSet.Get( ATTR_PROTECTION ).GetProtection();
                rSet.Put( SfxBoolItem(SID_SCATTR_CELLPROTECTION, bProtect) );
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
    ScTabViewShell* pTabViewShell   = GetViewData().GetViewShell();
    const SfxItemSet& rAttrSet  = pTabViewShell->GetSelectionPattern()->GetItemSet();
    rSet.Put( rAttrSet, false ); // Include ItemStates in copy

    //  choose font info according to selection script type
    SvtScriptType nScript = SvtScriptType::NONE;      // GetSelectionScriptType never returns 0
    if ( rSet.GetItemState( ATTR_FONT_WEIGHT ) != SfxItemState::UNKNOWN )
    {
        nScript = pTabViewShell->GetSelectionScriptType();
        ScViewUtil::PutItemScript( rSet, rAttrSet, ATTR_FONT_WEIGHT, nScript );
    }
    if ( rSet.GetItemState( ATTR_FONT_POSTURE ) != SfxItemState::UNKNOWN )
    {
        if (nScript == SvtScriptType::NONE) nScript = pTabViewShell->GetSelectionScriptType();
        ScViewUtil::PutItemScript( rSet, rAttrSet, ATTR_FONT_POSTURE, nScript );
    }

    SfxItemState eState;

    // own control on radio button functionality:

    // underline

    eState = rAttrSet.GetItemState( ATTR_FONT_UNDERLINE );
    if ( eState == SfxItemState::INVALID )
    {
        rSet.InvalidateItem( SID_ULINE_VAL_NONE );
        rSet.InvalidateItem( SID_ULINE_VAL_SINGLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOUBLE );
        rSet.InvalidateItem( SID_ULINE_VAL_DOTTED );
    }
    else
    {
        FontLineStyle eUnderline =
                    rAttrSet.Get(ATTR_FONT_UNDERLINE).GetLineStyle();
        rSet.Put(SfxBoolItem(SID_ULINE_VAL_SINGLE, eUnderline == LINESTYLE_SINGLE));
        rSet.Put(SfxBoolItem(SID_ULINE_VAL_DOUBLE, eUnderline == LINESTYLE_DOUBLE));
        rSet.Put(SfxBoolItem(SID_ULINE_VAL_DOTTED, eUnderline == LINESTYLE_DOTTED));
        rSet.Put(SfxBoolItem(SID_ULINE_VAL_NONE, eUnderline == LINESTYLE_NONE));
    }

    // horizontal alignment

    const SvxHorJustifyItem* pHorJustify = nullptr;
    const SvxVerJustifyItem* pVerJustify = nullptr;
    SvxCellVerJustify        eVerJustify = SvxCellVerJustify::Standard;
    sal_uInt16                   nWhich      = 0;
    bool                     bJustifyStd = false;
    SfxBoolItem              aBoolItem   ( 0, true );

    eState   = rAttrSet.GetItemState( ATTR_HOR_JUSTIFY, true,
                                        reinterpret_cast<const SfxPoolItem**>(&pHorJustify) );
    switch ( eState )
    {
        case SfxItemState::SET:
            {
                switch ( pHorJustify->GetValue() )
                {
                    case SvxCellHorJustify::Standard:
                        break;

                    case SvxCellHorJustify::Left:
                        nWhich = SID_ALIGNLEFT;
                        break;

                    case SvxCellHorJustify::Right:
                        nWhich = SID_ALIGNRIGHT;
                        break;

                    case SvxCellHorJustify::Center:
                        nWhich = SID_ALIGNCENTERHOR;
                        break;

                    case SvxCellHorJustify::Block:
                        nWhich = SID_ALIGNBLOCK;
                        break;

                    case SvxCellHorJustify::Repeat:
                    default:
                        bJustifyStd = true;
                        break;
                }
            }
            break;

        case SfxItemState::INVALID:
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
                eVerJustify = pVerJustify->GetValue();

                switch ( eVerJustify )
                {
                    case SvxCellVerJustify::Top:
                        nWhich = SID_ALIGNTOP;
                        break;

                    case SvxCellVerJustify::Bottom:
                        nWhich = SID_ALIGNBOTTOM;
                        break;

                    case SvxCellVerJustify::Center:
                        nWhich = SID_ALIGNCENTERVER;
                        break;

                    case SvxCellVerJustify::Standard:
                    default:
                        bJustifyStd = true;
                        break;
                }
            }
            break;

        case SfxItemState::INVALID:
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
    ScTabViewShell* pTabViewShell   = GetViewData().GetViewShell();
    std::shared_ptr<SvxBoxItem> aBoxItem(std::make_shared<SvxBoxItem>(ATTR_BORDER));
    std::shared_ptr<SvxBoxInfoItem> aInfoItem(std::make_shared<SvxBoxInfoItem>(ATTR_BORDER_INNER));

    pTabViewShell->GetSelectionFrame( aBoxItem, aInfoItem );

    if ( rSet.GetItemState( ATTR_BORDER ) != SfxItemState::UNKNOWN )
        rSet.Put( *aBoxItem );
    if ( rSet.GetItemState( ATTR_BORDER_INNER ) != SfxItemState::UNKNOWN )
        rSet.Put( *aInfoItem );
}

void ScFormatShell::GetAlignState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData().GetViewShell();
    const SfxItemSet& rAttrSet    = pTabViewShell->GetSelectionPattern()->GetItemSet();
    SfxWhichIter    aIter(rSet);
    sal_uInt16          nWhich = aIter.FirstWhich();

    SvxCellHorJustify eHAlign = SvxCellHorJustify::Standard;
    bool bHasHAlign = rAttrSet.GetItemState( ATTR_HOR_JUSTIFY ) != SfxItemState::INVALID;
    if( bHasHAlign )
        eHAlign = rAttrSet.Get( ATTR_HOR_JUSTIFY ).GetValue();

    SvxCellVerJustify eVAlign = SvxCellVerJustify::Standard;
    bool bHasVAlign = rAttrSet.GetItemState( ATTR_VER_JUSTIFY ) != SfxItemState::INVALID;
    if( bHasVAlign )
        eVAlign = rAttrSet.Get( ATTR_VER_JUSTIFY ).GetValue();

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
    ScTabViewShell* pTabViewShell   = GetViewData().GetViewShell();
    ScDocument& rDoc                = rViewData.GetDocument();
    const SfxItemSet& rAttrSet      = pTabViewShell->GetSelectionPattern()->GetItemSet();
    const SfxItemState eItemState   = rAttrSet.GetItemState( ATTR_VALUE_FORMAT );
    sal_uInt32 nNumberFormat        = rAttrSet.Get(ATTR_VALUE_FORMAT).GetValue();
    SvNumberFormatter* pFormatter   = rDoc.GetFormatTable();
                                      // If item state is default or set it
                                      // indicates one number format so we
                                      // don't have to iterate over all
                                      // selected cells' attribute ranges to
                                      // determine selected types.
                                      // Does *NOT* include the
                                      // SvNumFormatType::DEFINED bit.
    const SvNumFormatType nType     = (eItemState >= SfxItemState::DEFAULT ? pFormatter->GetType( nNumberFormat) :
                                       GetCurrentNumberFormatType());
    NfIndexTableOffset nOffset      = SvNumberFormatter::GetIndexTableOffset(nNumberFormat);

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_NUMBER_THOUSANDS:
                {
                    bool bEnable = (SfxItemState::INVALID != eItemState);
                    if (bEnable)
                    {
                        bEnable = ((nType != SvNumFormatType::ALL) && (nType &
                                (SvNumFormatType::NUMBER |
                                 SvNumFormatType::PERCENT |
                                 SvNumFormatType::CURRENCY |
                                 SvNumFormatType::FRACTION)));
                        if (bEnable)
                        {
                            bool bThousand( false );
                            bool bNegRed( false );
                            sal_uInt16 nPrecision( 0 );
                            sal_uInt16 nLeadZeroes( 0 );
                            pFormatter->GetFormatSpecialInfo( nNumberFormat, bThousand, bNegRed, nPrecision, nLeadZeroes);
                            rSet.Put( SfxBoolItem( nWhich, bThousand));
                        }
                    }
                    if (!bEnable)
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;
            case SID_NUMBER_FORMAT:
                // symphony version with format interpretation
                {
                    if(SfxItemState::INVALID != eItemState)
                    {
                        bool bThousand(false);
                        bool bNegRed(false);
                        sal_uInt16 nPrecision(0);
                        sal_uInt16 nLeadZeroes(0);

                        pFormatter->GetFormatSpecialInfo(nNumberFormat,bThousand, bNegRed, nPrecision, nLeadZeroes);

                        const SvNumberformat* pFormatEntry = pFormatter->GetEntry( nNumberFormat );
                        if (pFormatEntry && (pFormatEntry->GetType() & SvNumFormatType::SCIENTIFIC))
                        {
                            // if scientific, bThousand is used for engineering notation
                            const sal_uInt16 nIntegerDigits = pFormatEntry->GetFormatIntegerDigits();
                            bThousand = nIntegerDigits > 0 && ((nIntegerDigits % 3) == 0);
                        }
                        OUString aFormat;
                        static constexpr OUString sBreak = u","_ustr;
                        const OUString sThousand = OUString::number(static_cast<sal_Int32>(bThousand));
                        const OUString sNegRed = OUString::number(static_cast<sal_Int32>(bNegRed));
                        const OUString sPrecision = OUString::number(nPrecision);
                        const OUString sLeadZeroes = OUString::number(nLeadZeroes);
                        const OUString sNatNum12 = OUString::number( static_cast< sal_Int32 >( pFormatter->IsNatNum12( nNumberFormat ) ) );

                        aFormat += sThousand +
                            sBreak +
                            sNegRed +
                            sBreak +
                            sPrecision +
                            sBreak +
                            sLeadZeroes +
                            sBreak +
                            sNatNum12 +
                            sBreak;

                        rSet.Put(SfxStringItem(nWhich, aFormat));

                        if (comphelper::LibreOfficeKit::isActive())
                        {
                            OUString sPayload = ".uno:NumberFormat=" + aFormat;
                            GetViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED,
                                OUStringToOString(sPayload, RTL_TEXTENCODING_ASCII_US));
                        }
                    }
                    else
                    {
                        rSet.InvalidateItem( nWhich );
                    }
                }
                break;

            case SID_NUMBER_TYPE_FORMAT:
                {
                    sal_Int16 nFormatCategory = -1;
                    if ( eItemState >= SfxItemState::DEFAULT ) //Modify for more robust
                    {
                        switch(nType)
                        {
                        case SvNumFormatType::NUMBER:
                            // Determine if General format.
                            if ((nNumberFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
                                nFormatCategory = 0;
                            else
                                nFormatCategory = 1;
                            break;
                        case SvNumFormatType::PERCENT:
                            nFormatCategory = 2;
                            break;
                        case SvNumFormatType::CURRENCY:
                            nFormatCategory = 3;
                            break;
                        case SvNumFormatType::DATE:
                            //Add
                        case SvNumFormatType::DATETIME:
                            nFormatCategory = 4;
                            break;
                        case SvNumFormatType::TIME:
                            nFormatCategory = 5;
                            break;
                        case SvNumFormatType::SCIENTIFIC:
                            nFormatCategory = 6;
                            break;
                        case SvNumFormatType::FRACTION:
                            nFormatCategory = 7;
                            break;
                        case SvNumFormatType::LOGICAL:
                            nFormatCategory = 8;
                            break;
                        case SvNumFormatType::TEXT:
                            nFormatCategory = 9;
                            break;
                        default:
                            nFormatCategory = -1;   //for more robust
                        }
                        if( nFormatCategory == -1 )
                            rSet.InvalidateItem( nWhich );
                        else
                            rSet.Put( SfxUInt16Item( nWhich, nFormatCategory ) );
                    }
                    else
                    {
                        rSet.InvalidateItem( nWhich );
                    }

                }
                break;
            case SID_NUMBER_CURRENCY:
                rSet.Put( SfxBoolItem(nWhich, bool(nType & SvNumFormatType::CURRENCY)) );
                break;
            case SID_NUMBER_SCIENTIFIC:
                rSet.Put( SfxBoolItem(nWhich, bool(nType & SvNumFormatType::SCIENTIFIC)) );
                break;
            case SID_NUMBER_DATE:
                rSet.Put( SfxBoolItem(nWhich, bool(nType & SvNumFormatType::DATE)) );
                break;
            case SID_NUMBER_PERCENT:
                rSet.Put( SfxBoolItem(nWhich, bool(nType & SvNumFormatType::PERCENT)) );
                break;
            case SID_NUMBER_TIME:
                rSet.Put( SfxBoolItem(nWhich, bool(nType & SvNumFormatType::TIME)) );
                break;
            case SID_NUMBER_TWODEC:
                    rSet.Put( SfxBoolItem(nWhich, (nType & SvNumFormatType::NUMBER) && nOffset == NF_NUMBER_1000DEC2 ) );
                break;
            case SID_NUMBER_STANDARD:
                    rSet.Put( SfxBoolItem(nWhich, (nType & SvNumFormatType::NUMBER) && (nNumberFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0) );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

void ScFormatShell::ExecuteTextDirection( const SfxRequest& rReq )
{
    ScTabViewShell* pTabViewShell = GetViewData().GetViewShell();
    pTabViewShell->HideListBox();               // Autofilter-DropDown-Listbox
    bool bEditMode = false;
    if ( GetViewData().HasEditView( GetViewData().GetActivePart() ) )
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
            ScPatternAttr aAttr(GetViewData().GetDocument().getCellAttributeHelper());
            SfxItemSet& rItemSet = aAttr.GetItemSet();
            rItemSet.Put( ScVerticalStackCell( bVert ) );
            rItemSet.Put( SfxBoolItem( ATTR_VERTICAL_ASIAN, bVert ) );
            pTabViewShell->ApplySelectionPattern( aAttr );
            pTabViewShell->AdjustBlockHeight();
        }
        break;

        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
            SvxFrameDirection eDirection = ( nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT ) ?
                                                SvxFrameDirection::Horizontal_LR_TB : SvxFrameDirection::Horizontal_RL_TB;
            pTabViewShell->ApplyAttr( SvxFrameDirectionItem( eDirection, ATTR_WRITINGDIR ) );
        }
        break;
    }
    if (bEditMode)
        SC_MOD()->SetInputMode( SC_INPUT_TABLE );
}

void ScFormatShell::GetTextDirectionState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell = GetViewData().GetViewShell();
    const SfxItemSet& rAttrSet = pTabViewShell->GetSelectionPattern()->GetItemSet();

    bool bVertDontCare =
        (rAttrSet.GetItemState( ATTR_VERTICAL_ASIAN ) == SfxItemState::INVALID) ||
        (rAttrSet.GetItemState( ATTR_STACKED ) == SfxItemState::INVALID);
    bool bLeftRight = !bVertDontCare &&
        !rAttrSet.Get( ATTR_STACKED ).GetValue();
    bool bTopBottom = !bVertDontCare && !bLeftRight &&
        rAttrSet.Get( ATTR_VERTICAL_ASIAN ).GetValue();

    bool bBidiDontCare = (rAttrSet.GetItemState( ATTR_WRITINGDIR ) == SfxItemState::INVALID);
    EEHorizontalTextDirection eBidiDir = EEHorizontalTextDirection::Default;
    if ( !bBidiDontCare )
    {
        SvxFrameDirection eCellDir = rAttrSet.Get( ATTR_WRITINGDIR ).GetValue();
        if ( eCellDir == SvxFrameDirection::Environment )
            eBidiDir = GetViewData().GetDocument().
                                GetEditTextDirection( GetViewData().GetTabNo() );
        else if ( eCellDir == SvxFrameDirection::Horizontal_RL_TB )
            eBidiDir = EEHorizontalTextDirection::R2L;
        else
            eBidiDir = EEHorizontalTextDirection::L2R;
    }

    bool bDisableCTLFont = !SvtCTLOptions::IsCTLFontEnabled();
    bool bDisableVerticalText = !SvtCJKOptions::IsVerticalTextEnabled();

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
                        rSet.Put( SfxBoolItem( nWhich, eBidiDir == EEHorizontalTextDirection::L2R ) );
                    else
                        rSet.Put( SfxBoolItem( nWhich, eBidiDir == EEHorizontalTextDirection::R2L ) );
                }
        }
        nWhich = aIter.NextWhich();
    }
}

void ScFormatShell::ExecFormatPaintbrush( const SfxRequest& rReq )
{
    ScViewFunc* pView = rViewData.GetView();
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
            bLock = pArgs->Get(SID_FORMATPAINTBRUSH).GetValue();

        // in case of multi selection, deselect all and use the cursor position
        ScRange aDummy;
        if ( rViewData.GetSimpleArea(aDummy) != SC_MARK_SIMPLE )
            pView->Unmark();

        ScDocumentUniquePtr pBrushDoc(new ScDocument( SCDOCMODE_CLIP ));
        pView->CopyToClip( pBrushDoc.get(), false, true );
        pView->SetBrushDocument( std::move(pBrushDoc), bLock );
    }
}

void ScFormatShell::StateFormatPaintbrush( SfxItemSet& rSet )
{
    if ( rViewData.HasEditView( rViewData.GetActivePart() ) )
        rSet.DisableItem( SID_FORMATPAINTBRUSH );
    else
        rSet.Put( SfxBoolItem( SID_FORMATPAINTBRUSH, rViewData.GetView()->HasPaintBrush() ) );
}

SvNumFormatType ScFormatShell::GetCurrentNumberFormatType()
{
    SvNumFormatType nType = SvNumFormatType::ALL;
    ScDocument& rDoc = GetViewData().GetDocument();
    ScMarkData aMark(GetViewData().GetMarkData());
    const SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
    if (!pFormatter)
        return nType;

    // TODO: Find out how to get a selected table range in case multiple tables
    // are selected.  Currently we only check for the current active table.

    if ( aMark.IsMarked() || aMark.IsMultiMarked() )
    {
        aMark.MarkToMulti();
        const ScRange& aRange = aMark.GetMultiMarkArea();
        const ScMultiSel& rMultiSel = aMark.GetMultiSelData();

        SvNumFormatType nComboType = SvNumFormatType::ALL;
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
                sal_uInt32 nNumFmt = rDoc.GetNumberFormat(aColRange);
                SvNumFormatType nThisType = pFormatter->GetType(nNumFmt);
                if (bFirstItem)
                {
                    bFirstItem = false;
                    nComboType = nThisType;
                }
                else if (nComboType != nThisType)
                    // mixed number format type.
                    return SvNumFormatType::ALL;
            }
        }
        nType = nComboType;
    }
    else
    {
        sal_uInt32 nNumFmt = rDoc.GetNumberFormat( rViewData.GetCurX(), rViewData.GetCurY(),
                               rViewData.GetTabNo());
        nType = pFormatter->GetType( nNumFmt );
    }
    return nType;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
