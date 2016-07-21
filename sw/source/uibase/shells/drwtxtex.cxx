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

#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svx/svdview.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/svdoutl.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/whiter.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <svtools/langtab.hxx>
#include <svl/languageoptions.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/flditem.hxx>
#include <editeng/editstat.hxx>
#include <svx/hlnkitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svl/slstitm.hxx>
#include <editeng/langitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/writingmodeitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <vcl/window.hxx>
#include <editeng/editview.hxx>
#include <vcl/outdev.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <tools/diagnose_ex.h>

#include <cmdid.h>
#include <doc.hxx>
#include <docstat.hxx>
#include <drwtxtsh.hxx>
#include <edtwin.hxx>
#include <globals.hrc>
#include <hintids.hxx>
#include <initui.hxx>
#include <langhelper.hxx>
#include <chrdlgmodes.hxx>
#include <pardlg.hxx>
#include <shells.hrc>
#include <string.h>
#include <swdtflvr.hxx>
#include <swmodule.hxx>
#include <swwait.hxx>
#include <uitool.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>
#include <wview.hxx>

#include "swabstdlg.hxx"
#include "chrdlg.hrc"
#include "misc.hrc"
#include <memory>

using namespace ::com::sun::star;

void SwDrawTextShell::Execute( SfxRequest &rReq )
{
    SwWrtShell &rSh = GetShell();
    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());
    SfxItemSet aNewAttr(*aEditAttr.GetPool(), aEditAttr.GetRanges());

    const sal_uInt16 nSlot = rReq.GetSlot();

    const sal_uInt16 nWhich = GetPool().GetWhich(nSlot);
    const SfxItemSet *pNewAttrs = rReq.GetArgs();

    bool bRestoreSelection = false;
    ESelection aOldSelection;

    sal_uInt16 nEEWhich = 0;
    switch (nSlot)
    {
        case SID_LANGUAGE_STATUS:
        {
            aOldSelection = pOLV->GetSelection();
            if (!pOLV->GetEditView().HasSelection())
            {
                pOLV->GetEditView().SelectCurrentWord();
            }

            bRestoreSelection = SwLangHelper::SetLanguageStatus(pOLV,rReq,GetView(),rSh);
            break;
        }

        case SID_THES:
        {
            OUString aReplaceText;
            const SfxStringItem* pItem2 = rReq.GetArg<SfxStringItem>(SID_THES);
            if (pItem2)
                aReplaceText = pItem2->GetValue();
            if (!aReplaceText.isEmpty())
                ReplaceTextWithSynonym( pOLV->GetEditView(), aReplaceText );
            break;
        }

        case SID_ATTR_CHAR_FONT:
        case SID_ATTR_CHAR_FONTHEIGHT:
        case SID_ATTR_CHAR_WEIGHT:
        case SID_ATTR_CHAR_POSTURE:
        {
            SfxItemPool* pPool2 = aEditAttr.GetPool()->GetSecondaryPool();
            if( !pPool2 )
                pPool2 = aEditAttr.GetPool();
            SvxScriptSetItem aSetItem( nSlot, *pPool2 );

            // #i78017 establish the same behaviour as in Writer
            SvtScriptType nScriptTypes = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;
            if (nSlot == SID_ATTR_CHAR_FONT)
                nScriptTypes = pOLV->GetSelectedScriptType();

            if (pNewAttrs)
            {
                aSetItem.PutItemForScriptType( nScriptTypes, pNewAttrs->Get( nWhich ) );
                aNewAttr.Put( aSetItem.GetItemSet() );
            }
        }
        break;

        case SID_ATTR_CHAR_COLOR: nEEWhich = EE_CHAR_COLOR; break;
        case SID_ATTR_CHAR_BACK_COLOR: nEEWhich = EE_CHAR_BKGCOLOR; break;

        case SID_ATTR_CHAR_UNDERLINE:
        {
            if ( pNewAttrs )
            {
                const SvxTextLineItem& rTextLineItem = static_cast< const SvxTextLineItem& >( pNewAttrs->Get( pNewAttrs->GetPool()->GetWhich(nSlot) ) );
                aNewAttr.Put( SvxUnderlineItem( rTextLineItem.GetLineStyle(), EE_CHAR_UNDERLINE ) );
            }
            else
            {
                FontLineStyle eFU = static_cast<const SvxUnderlineItem&>(aEditAttr.Get(EE_CHAR_UNDERLINE)).GetLineStyle();
                aNewAttr.Put( SvxUnderlineItem(eFU == LINESTYLE_SINGLE ? LINESTYLE_NONE : LINESTYLE_SINGLE, EE_CHAR_UNDERLINE) );
            }
        }
        break;

        case SID_ATTR_CHAR_OVERLINE:
        {
            FontLineStyle eFO = static_cast<const SvxOverlineItem&>(aEditAttr.Get(EE_CHAR_OVERLINE)).GetLineStyle();
            aNewAttr.Put(SvxOverlineItem(eFO == LINESTYLE_SINGLE ? LINESTYLE_NONE : LINESTYLE_SINGLE, EE_CHAR_OVERLINE));
        }
        break;

        case SID_ATTR_CHAR_CONTOUR:     nEEWhich = EE_CHAR_OUTLINE; break;
        case SID_ATTR_CHAR_SHADOWED:    nEEWhich = EE_CHAR_SHADOW; break;
        case SID_ATTR_CHAR_STRIKEOUT:   nEEWhich = EE_CHAR_STRIKEOUT; break;
        case SID_ATTR_CHAR_WORDLINEMODE: nEEWhich = EE_CHAR_WLM; break;
        case SID_ATTR_CHAR_RELIEF      : nEEWhich = EE_CHAR_RELIEF;  break;
        case SID_ATTR_CHAR_LANGUAGE    : nEEWhich = EE_CHAR_LANGUAGE;break;
        case SID_ATTR_CHAR_KERNING     : nEEWhich = EE_CHAR_KERNING; break;
        case SID_ATTR_CHAR_SCALEWIDTH:   nEEWhich = EE_CHAR_FONTWIDTH; break;
        case SID_ATTR_CHAR_AUTOKERN  :   nEEWhich = EE_CHAR_PAIRKERNING; break;
        case SID_ATTR_CHAR_ESCAPEMENT:   nEEWhich = EE_CHAR_ESCAPEMENT; break;
        case SID_ATTR_PARA_ADJUST_LEFT:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_LEFT, EE_PARA_JUST));
        break;
        case SID_ATTR_PARA_ADJUST_CENTER:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_CENTER, EE_PARA_JUST));
        break;
        case SID_ATTR_PARA_ADJUST_RIGHT:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_RIGHT, EE_PARA_JUST));
        break;
        case SID_ATTR_PARA_ADJUST_BLOCK:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_BLOCK, EE_PARA_JUST));
        break;
        case SID_ATTR_PARA_LRSPACE:
            {
                SvxLRSpaceItem aParaMargin(static_cast<const SvxLRSpaceItem&>(rReq.
                                        GetArgs()->Get(nSlot)));
                aParaMargin.SetWhich( EE_PARA_LRSPACE );
                aNewAttr.Put(aParaMargin);
                rReq.Done();
            }
            break;
        case SID_ATTR_PARA_LINESPACE:
            {
                SvxLineSpacingItem aLineSpace = static_cast<const SvxLineSpacingItem&>(pNewAttrs->Get(
                                                            GetPool().GetWhich(nSlot)));
                aLineSpace.SetWhich( EE_PARA_SBL );
                aNewAttr.Put( aLineSpace );
                rReq.Done();
            }
            break;
        case SID_ATTR_PARA_ULSPACE:
            {
                SvxULSpaceItem aULSpace = static_cast<const SvxULSpaceItem&>(pNewAttrs->Get(
                    GetPool().GetWhich(nSlot)));
                aULSpace.SetWhich( EE_PARA_ULSPACE );
                aNewAttr.Put( aULSpace );
                rReq.Done();
            }
            break;
        case SID_PARASPACE_INCREASE:
        case SID_PARASPACE_DECREASE:
        {
            SvxULSpaceItem aULSpace(
                static_cast< const SvxULSpaceItem& >( aEditAttr.Get( EE_PARA_ULSPACE ) ) );
            sal_uInt16 nUpper = aULSpace.GetUpper();
            sal_uInt16 nLower = aULSpace.GetLower();

            if ( nSlot == SID_PARASPACE_INCREASE )
            {
                nUpper = std::min< sal_uInt16 >( nUpper + 57, 5670 );
                nLower = std::min< sal_uInt16 >( nLower + 57, 5670 );
            }
            else
            {
                nUpper = std::max< sal_Int16 >( nUpper - 57, 0 );
                nLower = std::max< sal_Int16 >( nLower - 57, 0 );
            }

            aULSpace.SetUpper( nUpper );
            aULSpace.SetLower( nLower );
            aNewAttr.Put( aULSpace );
            rReq.Done();
        }
        break;
        case SID_ATTR_PARA_LINESPACE_10:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_ONE_LINE, EE_PARA_SBL);
            aItem.SetPropLineSpace(100);
            aNewAttr.Put(aItem);
        }
        break;
        case SID_ATTR_PARA_LINESPACE_15:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_ONE_POINT_FIVE_LINES, EE_PARA_SBL);
            aItem.SetPropLineSpace(150);
            aNewAttr.Put(aItem);
        }
        break;
        case SID_ATTR_PARA_LINESPACE_20:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_TWO_LINES, EE_PARA_SBL);
            aItem.SetPropLineSpace(200);
            aNewAttr.Put(aItem);
        }
        break;

        case FN_SET_SUPER_SCRIPT:
        {
            SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
            SvxEscapement eEsc = (SvxEscapement ) static_cast<const SvxEscapementItem&>(
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

            if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUPERSCRIPT );
            aNewAttr.Put( aItem );
        }
        break;
        case FN_SET_SUB_SCRIPT:
        {
            SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
            SvxEscapement eEsc = (SvxEscapement ) static_cast<const SvxEscapementItem&>(
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

            if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUBSCRIPT );
            aNewAttr.Put( aItem );
        }
        break;

        case SID_CHAR_DLG_EFFECT:
        case SID_CHAR_DLG:
        case SID_CHAR_DLG_FOR_PARAGRAPH:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxStringItem* pItem = rReq.GetArg<SfxStringItem>(FN_PARAM_1);

            if( !pArgs || pItem )
            {
                aOldSelection = pOLV->GetSelection();
                if (nSlot == SID_CHAR_DLG_FOR_PARAGRAPH)
                {
                    // select current paragraph (and restore selection later on...)
                    EditView & rEditView = pOLV->GetEditView();
                    SwLangHelper::SelectPara( rEditView, rEditView.GetSelection() );
                    bRestoreSelection = true;
                }

                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(dynamic_cast<SwWebView*>( pView) !=  nullptr );
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );
                SfxItemSet aDlgAttr(GetPool(), EE_ITEMS_START, EE_ITEMS_END);

                // util::Language does not exists in the EditEngine! That is why not in set.

                aDlgAttr.Put( aEditAttr );
                aDlgAttr.Put( SvxKerningItem(0, RES_CHRATR_KERNING) );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                assert(pFact && "SwAbstractDialogFactory fail!");

                std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateSwCharDlg(pView->GetWindow(), *pView, aDlgAttr, SwCharDlgMode::Draw));
                assert(pDlg && "Dialog creation failed!");
                if (nSlot == SID_CHAR_DLG_EFFECT)
                {
                    pDlg->SetCurPageId("fonteffects");
                }
                else if (nSlot == SID_CHAR_DLG_FOR_PARAGRAPH)
                {
                    pDlg->SetCurPageId("font");
                }
                else if (pItem)
                {
                    pDlg->SetCurPageId(OUStringToOString(pItem->GetValue(), RTL_TEXTENCODING_UTF8));
                }

                sal_uInt16 nRet = pDlg->Execute();
                if(RET_OK == nRet )
                {
                    rReq.Done( *( pDlg->GetOutputItemSet() ) );
                    aNewAttr.Put(*pDlg->GetOutputItemSet());
                }
                if(RET_OK != nRet)
                    return ;
            }
            else
                aNewAttr.Put(*pArgs);
        }
        break;
        case FN_FORMAT_FOOTNOTE_DLG:
        {
            GetView().ExecFormatFootnote();
            break;
        }
        case FN_NUMBERING_OUTLINE_DLG:
        {
            GetView().ExecNumberingOutline(GetPool());
            rReq.Done();
        }
        break;
        case SID_OPEN_XML_FILTERSETTINGS:
        {
            HandleOpenXmlFilterSettings(rReq);
        }
        break;
        case FN_WORDCOUNT_DIALOG:
        {
            GetView().UpdateWordCount(this, nSlot);
        }
        break;
        case SID_PARA_DLG:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (!pArgs)
            {
                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(dynamic_cast<SwWebView*>( pView) !=  nullptr );
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );
                SfxItemSet aDlgAttr(GetPool(),
                                    EE_ITEMS_START, EE_ITEMS_END,
                                    SID_ATTR_PARA_HYPHENZONE, SID_ATTR_PARA_HYPHENZONE,
                                    SID_ATTR_PARA_SPLIT, SID_ATTR_PARA_SPLIT,
                                    SID_ATTR_PARA_WIDOWS, SID_ATTR_PARA_WIDOWS,
                                    SID_ATTR_PARA_ORPHANS, SID_ATTR_PARA_ORPHANS,
                                    0);

                aDlgAttr.Put(aEditAttr);

                aDlgAttr.Put( SvxHyphenZoneItem( false, RES_PARATR_HYPHENZONE) );
                aDlgAttr.Put( SvxFormatBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
                aDlgAttr.Put( SvxFormatSplitItem( true, RES_PARATR_SPLIT ) );
                aDlgAttr.Put( SvxWidowsItem( 0, RES_PARATR_WIDOWS ) );
                aDlgAttr.Put( SvxOrphansItem( 0, RES_PARATR_ORPHANS ) );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                assert(pFact && "SwAbstractDialogFactory fail!");

                std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateSwParaDlg( GetView().GetWindow(), GetView(), aDlgAttr, nullptr, true ));
                assert(pDlg && "Dialog creation failed!");
                sal_uInt16 nRet = pDlg->Execute();
                if(RET_OK == nRet)
                {
                    rReq.Done( *( pDlg->GetOutputItemSet() ) );
                    aNewAttr.Put(*pDlg->GetOutputItemSet());
                }
                if(RET_OK != nRet)
                    return;
            }
            else
                aNewAttr.Put(*pArgs);
        }
        break;
        case SID_AUTOSPELL_CHECK:
        {
//!! JP 16.03.2001: why??           pSdrView = rSh.GetDrawView();
//!! JP 16.03.2001: why??           pOutliner = pSdrView->GetTextEditOutliner();
            SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
            EEControlBits nCtrl = pOutliner->GetControlWord();

            bool bSet = static_cast<const SfxBoolItem&>(rReq.GetArgs()->Get(
                                                    nSlot)).GetValue();
            if(bSet)
                nCtrl |= EEControlBits::ONLINESPELLING|EEControlBits::ALLOWBIGOBJS;
            else
                nCtrl &= ~EEControlBits::ONLINESPELLING;
            pOutliner->SetControlWord(nCtrl);

            rView.ExecuteSlot(rReq);
        }
        break;
        case SID_HYPERLINK_SETLINK:
        {
            const SfxPoolItem* pItem = nullptr;
            if(pNewAttrs)
                pNewAttrs->GetItemState(nSlot, false, &pItem);

            if(pItem)
            {
                const SvxHyperlinkItem& rHLinkItem = *static_cast<const SvxHyperlinkItem *>(pItem);
                SvxURLField aField(rHLinkItem.GetURL(), rHLinkItem.GetName(), SVXURLFORMAT_APPDEFAULT);
                aField.SetTargetFrame(rHLinkItem.GetTargetFrame());

                const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();

                if (pFieldItem && dynamic_cast< const SvxURLField *>( pFieldItem->GetField() ) != nullptr )
                {
                    // Select field so that it will be deleted during insert
                    ESelection aSel = pOLV->GetSelection();
                    aSel.nEndPos++;
                    pOLV->SetSelection(aSel);
                }
                pOLV->InsertField(SvxFieldItem(aField, EE_FEATURE_FIELD));
            }
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            // Shell switch!
            {
                SdrObject* pTmpObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                SdrPageView* pTmpPV = pSdrView->GetSdrPageView();
                SdrView* pTmpView = pSdrView;

                pSdrView->SdrEndTextEdit(true);

                SfxItemSet aAttr( *aNewAttr.GetPool(),
                            SDRATTR_TEXTDIRECTION,
                            SDRATTR_TEXTDIRECTION );

                aAttr.Put( SvxWritingModeItem(
                    nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ?
                        text::WritingMode_LR_TB
                        : text::WritingMode_TB_RL, SDRATTR_TEXTDIRECTION ) );
                pTmpView->SetAttributes( aAttr );

                rSh.GetView().BeginTextEdit( pTmpObj, pTmpPV, &rSh.GetView().GetEditWin());
                rSh.GetView().AttrChangedNotify( &rSh );
            }
            return;

        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
            SdrObject* pTmpObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            SdrPageView* pTmpPV = pSdrView->GetSdrPageView();
            SdrView* pTmpView = pSdrView;

            pSdrView->SdrEndTextEdit(true);
            bool bLeftToRight = nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT;

            const SfxPoolItem* pPoolItem;
            if( pNewAttrs && SfxItemState::SET == pNewAttrs->GetItemState( nSlot, true, &pPoolItem ) )
            {
                if( !static_cast<const SfxBoolItem*>(pPoolItem)->GetValue() )
                    bLeftToRight = !bLeftToRight;
            }
            SfxItemSet aAttr( *aNewAttr.GetPool(),
                        EE_PARA_JUST, EE_PARA_JUST,
                        EE_PARA_WRITINGDIR, EE_PARA_WRITINGDIR,
                        0 );

            sal_uInt16 nAdjust = SVX_ADJUST_LEFT;
            if( SfxItemState::SET == aEditAttr.GetItemState(EE_PARA_JUST, true, &pPoolItem ) )
                nAdjust = static_cast<const SvxAdjustItem*>(pPoolItem)->GetEnumValue();

            if( bLeftToRight )
            {
                aAttr.Put( SvxFrameDirectionItem( FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR ) );
                if( nAdjust == SVX_ADJUST_RIGHT )
                    aAttr.Put( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );
            }
            else
            {
                aAttr.Put( SvxFrameDirectionItem( FRMDIR_HORI_RIGHT_TOP, EE_PARA_WRITINGDIR ) );
                if( nAdjust == SVX_ADJUST_LEFT )
                    aAttr.Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
            }
            pTmpView->SetAttributes( aAttr );
            rSh.GetView().BeginTextEdit( pTmpObj, pTmpPV, &rSh.GetView().GetEditWin() );
            rSh.GetView().AttrChangedNotify( &rSh );
        }
        return;

        case FN_GROW_FONT_SIZE:
        case FN_SHRINK_FONT_SIZE:
        {
            const SvxFontListItem* pFontListItem = static_cast< const SvxFontListItem* >
                    ( SfxObjectShell::Current()->GetItem( SID_ATTR_CHAR_FONTLIST ) );
            const FontList* pFontList = pFontListItem ? pFontListItem->GetFontList() : nullptr;
            pOLV->GetEditView().ChangeFontSize( nSlot == FN_GROW_FONT_SIZE, pFontList );
        }
        break;

        default:
            assert(false && "wrong dispatcher");
            return;
    }
    if(nEEWhich && pNewAttrs)
    {
        std::unique_ptr<SfxPoolItem> pNewItem(pNewAttrs->Get(nWhich).CloneSetWhich(nEEWhich));
        aNewAttr.Put(*pNewItem);
    }

    SetAttrToMarked(aNewAttr);

    GetView().GetViewFrame()->GetBindings().InvalidateAll(false);

    if (IsTextEdit() && pOLV->GetOutliner()->IsModified())
        rSh.SetModified();

    if (bRestoreSelection)
    {
        // restore selection
        pOLV->GetEditView().SetSelection( aOldSelection );
    }
}

void SwDrawTextShell::GetState(SfxItemSet& rSet)
{
    if (!IsTextEdit())  // Otherwise sometimes crash!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    SfxItemSet aEditAttr( pOLV->GetAttribs() );
    const SfxPoolItem *pAdjust = nullptr, *pLSpace = nullptr, *pEscItem = nullptr;
    int eAdjust, nLSpace, nEsc;

    while(nWhich)
    {
        sal_uInt16 nSlotId = GetPool().GetSlotId( nWhich );
        bool bFlag = false;
        switch( nSlotId )
        {
            case SID_LANGUAGE_STATUS://20412:
            {
                nSlotId = SwLangHelper::GetLanguageStatus(pOLV,rSet);
                break;
            }

            case SID_THES:
            {
                OUString        aStatusVal;
                LanguageType    nLang = LANGUAGE_NONE;
                bool bIsLookUpWord = GetStatusValueForThesaurusFromContext( aStatusVal, nLang, pOLV->GetEditView() );
                rSet.Put( SfxStringItem( SID_THES, aStatusVal ) );

                // disable "Thesaurus" context menu entry if there is nothing to look up
                uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
                if (!bIsLookUpWord ||
                    !xThes.is() || nLang == LANGUAGE_NONE || !xThes->hasLocale( LanguageTag::convertToLocale( nLang ) ))
                    rSet.DisableItem( SID_THES );

                //! avoid putting the same item as SfxBoolItem at the end of this function
                nSlotId = 0;
                break;
            }

        case SID_ATTR_PARA_ADJUST_LEFT:     eAdjust = SVX_ADJUST_LEFT; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_RIGHT:    eAdjust = SVX_ADJUST_RIGHT; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_CENTER:   eAdjust = SVX_ADJUST_CENTER; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_BLOCK:    eAdjust = SVX_ADJUST_BLOCK; goto ASK_ADJUST;
ASK_ADJUST:
            {
                if( !pAdjust )
                    aEditAttr.GetItemState( EE_PARA_JUST, false, &pAdjust);

                if( !pAdjust || IsInvalidItem( pAdjust ))
                {
                    rSet.InvalidateItem( nSlotId );
                    nSlotId = 0;
                }
                else
                    bFlag = eAdjust == static_cast<const SvxAdjustItem*>(pAdjust)->GetAdjust();
            }
            break;

        case SID_ATTR_PARA_LRSPACE:
            {
                SfxItemState eState = aEditAttr.GetItemState(EE_PARA_LRSPACE);
                if( eState >= SfxItemState::DEFAULT )
                {
                    SvxLRSpaceItem aLR = static_cast<const SvxLRSpaceItem&>( aEditAttr.Get( EE_PARA_LRSPACE ) );
                    aLR.SetWhich(SID_ATTR_PARA_LRSPACE);
                    rSet.Put(aLR);
                }
                else
                    rSet.InvalidateItem(nSlotId);
                nSlotId = 0;
            }
            break;
        case SID_ATTR_PARA_LINESPACE:
            {
                SfxItemState eState = aEditAttr.GetItemState(EE_PARA_SBL);
                if( eState >= SfxItemState::DEFAULT )
                {
                    SvxLineSpacingItem aLR = static_cast<const SvxLineSpacingItem&>( aEditAttr.Get( EE_PARA_SBL ) );
                    rSet.Put(aLR);
                }
                else
                    rSet.InvalidateItem(nSlotId);
                nSlotId = 0;
            }
            break;
        case SID_ATTR_PARA_ULSPACE:
        case SID_PARASPACE_INCREASE:
        case SID_PARASPACE_DECREASE:
            {
                SfxItemState eState = aEditAttr.GetItemState(EE_PARA_ULSPACE);
                if( eState >= SfxItemState::DEFAULT )
                {
                    SvxULSpaceItem aULSpace = static_cast<const SvxULSpaceItem&>( aEditAttr.Get( EE_PARA_ULSPACE ) );
                    if ( !aULSpace.GetUpper() && !aULSpace.GetLower() )
                        rSet.DisableItem( SID_PARASPACE_DECREASE );
                    else if ( aULSpace.GetUpper() >= 5670 && aULSpace.GetLower() >= 5670 )
                        rSet.DisableItem( SID_PARASPACE_INCREASE );
                    if ( nSlotId == SID_ATTR_PARA_ULSPACE )
                    {
                        aULSpace.SetWhich(SID_ATTR_PARA_ULSPACE);
                        rSet.Put(aULSpace);
                    }
                }
                else
                {
                    rSet.DisableItem( SID_PARASPACE_INCREASE );
                    rSet.DisableItem( SID_PARASPACE_DECREASE );
                    rSet.InvalidateItem( SID_ATTR_PARA_ULSPACE );
                }
                nSlotId = 0;
            }
            break;

        case SID_ATTR_PARA_LINESPACE_10:    nLSpace = 100;  goto ASK_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_15:    nLSpace = 150;  goto ASK_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_20:    nLSpace = 200;  goto ASK_LINESPACE;
ASK_LINESPACE:
            {
                if( !pLSpace )
                    aEditAttr.GetItemState( EE_PARA_SBL, false, &pLSpace );

                if( !pLSpace || IsInvalidItem( pLSpace ))
                {
                    rSet.InvalidateItem( nSlotId );
                    nSlotId = 0;
                }
                else if( nLSpace == static_cast<const SvxLineSpacingItem*>(pLSpace)->
                                                GetPropLineSpace() )
                    bFlag = true;
                else
                    nSlotId = 0;
            }
            break;

        case FN_SET_SUPER_SCRIPT:   nEsc = SVX_ESCAPEMENT_SUPERSCRIPT;
                                    goto ASK_ESCAPE;
        case FN_SET_SUB_SCRIPT:     nEsc = SVX_ESCAPEMENT_SUBSCRIPT;
                                    goto ASK_ESCAPE;
ASK_ESCAPE:
            {
                if( !pEscItem )
                    pEscItem = &aEditAttr.Get( EE_CHAR_ESCAPEMENT );

                if( nEsc == static_cast<const SvxEscapementItem*>(
                                                pEscItem)->GetEnumValue() )
                    bFlag = true;
                else
                    nSlotId = 0;
            }
            break;

        case SID_THESAURUS:
        {
            // disable "Thesaurus" if the language is not supported
            const SfxPoolItem &rItem = GetShell().GetDoc()->GetDefault(
                            GetWhichOfScript( RES_CHRATR_LANGUAGE,
                            SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage())) );
            LanguageType nLang = static_cast<const SvxLanguageItem &>(rItem).GetLanguage();

            uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
            if (!xThes.is() || nLang == LANGUAGE_NONE || !xThes->hasLocale( LanguageTag::convertToLocale( nLang ) ))
                rSet.DisableItem( SID_THESAURUS );
            nSlotId = 0;
        }
        break;
        case SID_HANGUL_HANJA_CONVERSION:
        case SID_CHINESE_CONVERSION:
        {
            if (!SvtCJKOptions().IsAnyEnabled())
            {
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, false );
                rSet.DisableItem(nWhich);
            }
            else
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, true );
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            if ( !SvtLanguageOptions().IsVerticalTextEnabled() )
            {
                rSet.DisableItem( nSlotId );
                nSlotId = 0;
            }
            else
            {
                SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
                if( pOutliner )
                    bFlag = pOutliner->IsVertical() ==
                            (SID_TEXTDIRECTION_TOP_TO_BOTTOM == nSlotId);
                else
                {
                    text::WritingMode eMode = (text::WritingMode)
                                    static_cast<const SvxWritingModeItem&>( aEditAttr.Get( SDRATTR_TEXTDIRECTION ) ).GetValue();

                    if( nSlotId == SID_TEXTDIRECTION_LEFT_TO_RIGHT )
                    {
                        bFlag = eMode == text::WritingMode_LR_TB;
                    }
                    else
                    {
                        bFlag = eMode != text::WritingMode_TB_RL;
                    }
                }
            }
            break;
        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
            if ( !SvtLanguageOptions().IsCTLFontEnabled() )
            {
                rSet.DisableItem( nWhich );
                nSlotId = 0;
            }
            else
            {
                SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
                if(pOutliner && pOutliner->IsVertical())
                {
                    rSet.DisableItem( nWhich );
                    nSlotId = 0;
                }
                else
                {
                    switch( static_cast<const SvxFrameDirectionItem&>( aEditAttr.Get( EE_PARA_WRITINGDIR ) ).GetValue() )
                    {
                        case FRMDIR_HORI_LEFT_TOP:
                            bFlag = nWhich == SID_ATTR_PARA_LEFT_TO_RIGHT;
                        break;

                        case FRMDIR_HORI_RIGHT_TOP:
                            bFlag = nWhich != SID_ATTR_PARA_LEFT_TO_RIGHT;
                        break;
                    }
                }
            }
        }
        break;
        case SID_TRANSLITERATE_HALFWIDTH:
        case SID_TRANSLITERATE_FULLWIDTH:
        case SID_TRANSLITERATE_HIRAGANA:
        case SID_TRANSLITERATE_KATAGANA:
        {
            SvtCJKOptions aCJKOptions;
            if(!aCJKOptions.IsChangeCaseMapEnabled())
            {
                rSet.DisableItem(nWhich);
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, false );
            }
            else
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, true );
        }
        break;
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_ZWNBSP :
        case SID_INSERT_ZWSP:
        {
            SvtCTLOptions aCTLOptions;
            bool bEnabled = aCTLOptions.IsCTLFontEnabled();
            GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, bEnabled );
            if(!bEnabled)
                rSet.DisableItem(nWhich);
        }
        break;
        default:
            nSlotId = 0;                // don't know this slot
            break;
        }

        if( nSlotId )
            rSet.Put( SfxBoolItem( nWhich, bFlag ));

        nWhich = aIter.NextWhich();
    }
}

void SwDrawTextShell::GetDrawTextCtrlState(SfxItemSet& rSet)
{
    if (!IsTextEdit())  // Otherwise crash!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    SvtScriptType nScriptType = pOLV->GetSelectedScriptType();
    while(nWhich)
    {
        sal_uInt16 nEEWhich = 0;
        sal_uInt16 nSlotId = GetPool().GetSlotId( nWhich );
        switch( nSlotId )
        {
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
            case SID_ATTR_CHAR_WEIGHT:
            case SID_ATTR_CHAR_POSTURE:
            {
                SfxItemPool* pEditPool = aEditAttr.GetPool()->GetSecondaryPool();
                if( !pEditPool )
                    pEditPool = aEditAttr.GetPool();
                SvxScriptSetItem aSetItem( nSlotId, *pEditPool );
                aSetItem.GetItemSet().Put( aEditAttr, false );
                const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScriptType );
                if( pI )
                {
                    std::unique_ptr<SfxPoolItem> pNewItem(pI->CloneSetWhich(nWhich));
                    rSet.Put( *pNewItem );
                }
                else
                    rSet.InvalidateItem( nWhich );
            }
            break;
            case SID_ATTR_CHAR_COLOR: nEEWhich = EE_CHAR_COLOR; break;
            case SID_ATTR_CHAR_BACK_COLOR: nEEWhich = EE_CHAR_BKGCOLOR; break;
            case SID_ATTR_CHAR_UNDERLINE: nEEWhich = EE_CHAR_UNDERLINE;break;
            case SID_ATTR_CHAR_OVERLINE: nEEWhich = EE_CHAR_OVERLINE;break;
            case SID_ATTR_CHAR_CONTOUR: nEEWhich = EE_CHAR_OUTLINE; break;
            case SID_ATTR_CHAR_SHADOWED:  nEEWhich = EE_CHAR_SHADOW;break;
            case SID_ATTR_CHAR_STRIKEOUT: nEEWhich = EE_CHAR_STRIKEOUT;break;
            case SID_AUTOSPELL_CHECK:
            {
                const SfxPoolItem* pState = rView.GetSlotState(nWhich);
                if (pState)
                    rSet.Put(SfxBoolItem(nWhich, static_cast<const SfxBoolItem*>(pState)->GetValue()));
                else
                    rSet.DisableItem( nWhich );
                break;
            }
            case SID_ATTR_CHAR_WORDLINEMODE: nEEWhich = EE_CHAR_WLM; break;
            case SID_ATTR_CHAR_RELIEF      : nEEWhich = EE_CHAR_RELIEF;  break;
            case SID_ATTR_CHAR_LANGUAGE    : nEEWhich = EE_CHAR_LANGUAGE;break;
            case SID_ATTR_CHAR_KERNING     : nEEWhich = EE_CHAR_KERNING; break;
            case SID_ATTR_CHAR_SCALEWIDTH:   nEEWhich = EE_CHAR_FONTWIDTH;break;
            case SID_ATTR_CHAR_AUTOKERN  :   nEEWhich = EE_CHAR_PAIRKERNING; break;
            case SID_ATTR_CHAR_ESCAPEMENT:   nEEWhich = EE_CHAR_ESCAPEMENT; break;
            case FN_GROW_FONT_SIZE:
            case FN_SHRINK_FONT_SIZE:
            {
                SfxItemPool* pEditPool = aEditAttr.GetPool()->GetSecondaryPool();
                if( !pEditPool )
                    pEditPool = aEditAttr.GetPool();

                SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONTHEIGHT, *pEditPool );
                aSetItem.GetItemSet().Put( aEditAttr, false );
                const SvxFontHeightItem* pSize( static_cast<const SvxFontHeightItem*>( aSetItem.GetItemOfScript( nScriptType ) ) );

                if( pSize )
                {
                    sal_uInt32 nSize = pSize->GetHeight();
                    if( nSize >= 19998 )
                        rSet.DisableItem( FN_GROW_FONT_SIZE );
                    else if( nSize <= 40 )
                        rSet.DisableItem( FN_SHRINK_FONT_SIZE );
                }
            }
        }
        if(nEEWhich)
        {
            std::unique_ptr<SfxPoolItem> pNewItem(aEditAttr.Get(nEEWhich).CloneSetWhich(nWhich));
            rSet.Put(*pNewItem);
        }

        nWhich = aIter.NextWhich();
    }
}

void SwDrawTextShell::ExecClpbrd(SfxRequest &rReq)
{
    if (!IsTextEdit())  // Otherwise crash!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();

    ESelection aSel(pOLV->GetSelection());
    const bool bCopy = (aSel.nStartPara != aSel.nEndPara) || (aSel.nStartPos != aSel.nEndPos);
    sal_uInt16 nId = rReq.GetSlot();
    switch( nId )
    {
        case SID_CUT:
            if (bCopy)
                pOLV->Cut();
            return;

        case SID_COPY:
            if (bCopy)
                pOLV->Copy();
            return;

        case SID_PASTE:
            pOLV->PasteSpecial();
            break;

        default:
            OSL_FAIL("wrong dispatcher");
            return;
    }
}

void SwDrawTextShell::StateClpbrd(SfxItemSet &rSet)
{
    if (!IsTextEdit())  // Otherwise crash!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    ESelection aSel(pOLV->GetSelection());
    const bool bCopy = (aSel.nStartPara != aSel.nEndPara) ||
        (aSel.nStartPos != aSel.nEndPos);

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
        case SID_CUT:
        case SID_COPY:
            if( !bCopy )
                rSet.DisableItem( nWhich );
            break;

        case SID_PASTE:
            {
                TransferableDataHelper aDataHelper(
                    TransferableDataHelper::CreateFromSystemClipboard( &GetView().GetEditWin() ) );

                if( !aDataHelper.GetXTransferable().is()
                    || !SwTransferable::IsPaste( GetShell(), aDataHelper ) )
                {
                    rSet.DisableItem( nWhich );
                }
            }
            break;

        case SID_PASTE_SPECIAL:
        case SID_CLIPBOARD_FORMAT_ITEMS:
            rSet.DisableItem( nWhich );
            break;
        }

        nWhich = aIter.NextWhich();
    }
}

// Hyperlink status

void SwDrawTextShell::StateInsert(SfxItemSet &rSet)
{
    if (!IsTextEdit())  // Otherwise crash!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_HYPERLINK_GETLINK:
                {
                    SvxHyperlinkItem aHLinkItem;
                    aHLinkItem.SetInsertMode(HLINK_FIELD);

                    const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();

                    if (pFieldItem)
                    {
                        const SvxURLField* pURLField = dynamic_cast<const SvxURLField*>(pFieldItem->GetField());

                        if (pURLField)
                        {
                            aHLinkItem.SetName(pURLField->GetRepresentation());
                            aHLinkItem.SetURL(pURLField->GetURL());
                            aHLinkItem.SetTargetFrame(pURLField->GetTargetFrame());
                        }
                    }
                    else
                    {
                        OUString sSel(pOLV->GetSelected());
                        sSel = sSel.copy(0, std::min<sal_Int32>(255, sSel.getLength()));
                        aHLinkItem.SetName(comphelper::string::stripEnd(sSel, ' '));
                    }

                    sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
                    aHLinkItem.SetInsertMode((SvxLinkInsertMode)(aHLinkItem.GetInsertMode() |
                        ((nHtmlMode & HTMLMODE_ON) != 0 ? HLINK_HTMLMODE : 0)));

                    rSet.Put(aHLinkItem);
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
