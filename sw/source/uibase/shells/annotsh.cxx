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

#include <hintids.hxx>

#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <i18nlangtag/mslangid.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/clipfmtitem.hxx>
#include <svl/stritem.hxx>
#include <svl/slstitm.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/svdoutl.hxx>
#include <svl/whiter.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/flditem.hxx>
#include <editeng/editstat.hxx>
#include <svx/hlnkitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <editeng/langitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/writingmodeitem.hxx>
#include <swundo.hxx>
#include <doc.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>
#include <uitool.hxx>
#include <popup.hrc>
#include <chrdlgmodes.hxx>
#include <pardlg.hxx>
#include <swdtflvr.hxx>
#include <drwtxtsh.hxx>
#include <swmodule.hxx>
#include <initui.hxx>
#include <edtwin.hxx>
#include <swwait.hxx>
#include <docstat.hxx>

#include <cmdid.h>
#include <globals.hrc>
#include <shells.hrc>
#include <breakit.hxx>
#include "annotsh.hxx"
#include "view.hxx"
#include <PostItMgr.hxx>
#include <SidebarWin.hxx>

#include "swtypes.hxx"

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include <svx/svxids.hrc>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svl/itempool.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>

#include <svl/languageoptions.hxx>
#include <svtools/langtab.hxx>

#include <docsh.hxx>
#include <svl/undo.hxx>
#include "swabstdlg.hxx"
#include "chrdlg.hrc"
#include "misc.hrc"
#include <app.hrc>

#include <comphelper/string.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <langhelper.hxx>

#include <tools/diagnose_ex.h>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::i18n;

#define SwAnnotationShell

#include <sfx2/msg.hxx>
#include <swslots.hxx>

SFX_IMPL_INTERFACE(SwAnnotationShell, SfxShell)

void SwAnnotationShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, RID_TEXT_TOOLBOX);

    GetStaticInterface()->RegisterPopupMenu(SW_RES(MN_ANNOTATION_POPUPMENU));
}


SfxItemPool* SwAnnotationShell::GetAnnotationPool(SwView& rV)
{
    SwWrtShell &rSh = rV.GetWrtShell();
    return rSh.GetAttrPool().GetSecondaryPool();
}

SwAnnotationShell::SwAnnotationShell( SwView& r )
    : rView(r)
{
    SetPool(SwAnnotationShell::GetAnnotationPool(rView));
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Annotation));
}

SwAnnotationShell::~SwAnnotationShell()
{
}

::svl::IUndoManager* SwAnnotationShell::GetUndoManager()
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr ||
         !pPostItMgr->HasActiveSidebarWin() )
    {
        OSL_ENSURE(pPostItMgr,"PostItMgr::Layout(): We are looping forever");
        return nullptr;
    }
    return &pPostItMgr->GetActiveSidebarWin()->GetOutlinerView()->GetOutliner()->GetUndoManager();
}

void SwAnnotationShell::Exec( SfxRequest &rReq )
{
    //TODO: clean this up!!!!
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());
    SfxItemSet aNewAttr(*aEditAttr.GetPool(), aEditAttr.GetRanges());

    sal_uInt16 nSlot = rReq.GetSlot();
    sal_uInt16 nWhich = GetPool().GetWhich(nSlot);
    const SfxItemSet *pNewAttrs = rReq.GetArgs();
    sal_uInt16 nEEWhich = 0;
    switch (nSlot)
    {
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
        case SID_ATTR_PARA_LRSPACE:
            {
                SvxLRSpaceItem aParaMargin(static_cast<const SvxLRSpaceItem&>(rReq.
                                        GetArgs()->Get(nSlot)));
                aParaMargin.SetWhich( EE_PARA_LRSPACE );

                aNewAttr.Put(aParaMargin);
                rReq.Done();
                break;
            }
        case SID_ATTR_PARA_LINESPACE:
            {
                SvxLineSpacingItem aParaMargin = static_cast<const SvxLineSpacingItem&>(pNewAttrs->Get(
                                                            GetPool().GetWhich(nSlot)));
                aParaMargin.SetWhich( EE_PARA_SBL );

                aNewAttr.Put(aParaMargin);
                rReq.Done();
                break;
            }
        case SID_ATTR_PARA_ULSPACE:
            {
                SvxULSpaceItem aULSpace = static_cast<const SvxULSpaceItem&>(pNewAttrs->Get(
                    GetPool().GetWhich(nSlot)));
                aULSpace.SetWhich( EE_PARA_ULSPACE );
                aNewAttr.Put( aULSpace );
                rReq.Done();
            }
            break;
        case FN_GROW_FONT_SIZE:
        case FN_SHRINK_FONT_SIZE:
        {
            const SvxFontListItem* pFontListItem = static_cast< const SvxFontListItem* >
                    ( SfxObjectShell::Current()->GetItem( SID_ATTR_CHAR_FONTLIST ) );
            const FontList* pFontList = pFontListItem ? pFontListItem->GetFontList() : nullptr;
            pOLV->GetEditView().ChangeFontSize( nSlot == FN_GROW_FONT_SIZE, pFontList );
        }
        break;

        case SID_ATTR_CHAR_FONT:
        case SID_ATTR_CHAR_FONTHEIGHT:
        case SID_ATTR_CHAR_WEIGHT:
        case SID_ATTR_CHAR_POSTURE:
            {
                SfxItemPool* pSecondPool = aEditAttr.GetPool()->GetSecondaryPool();
                if( !pSecondPool )
                    pSecondPool = aEditAttr.GetPool();
                SvxScriptSetItem aSetItem( nSlot, *pSecondPool );
                aSetItem.PutItemForScriptType( pOLV->GetSelectedScriptType(), pNewAttrs->Get( nWhich ));
                aNewAttr.Put( aSetItem.GetItemSet() );
                rReq.Done();
                break;
            }
        case SID_ATTR_CHAR_COLOR: nEEWhich = EE_CHAR_COLOR; break;
        case SID_ATTR_CHAR_BACK_COLOR: nEEWhich = EE_CHAR_BKGCOLOR; break;
        case SID_ATTR_CHAR_UNDERLINE:
        {
            if( rReq.GetArgs() )
            {
                const SvxUnderlineItem* pItem = rReq.GetArg<SvxUnderlineItem>(SID_ATTR_CHAR_UNDERLINE);
                if (pItem)
                {
                    aNewAttr.Put(*pItem);
                }
                else
                {
                    FontUnderline eFU = static_cast<const SvxUnderlineItem&>( aEditAttr.Get( EE_CHAR_UNDERLINE ) ).GetLineStyle();
                    aNewAttr.Put( SvxUnderlineItem( eFU != UNDERLINE_NONE ?UNDERLINE_NONE : UNDERLINE_SINGLE,  EE_CHAR_UNDERLINE ) );
                }
            }
            break;
        }
        case SID_ATTR_CHAR_OVERLINE:
        {
             FontUnderline eFO = static_cast<const SvxOverlineItem&>(aEditAttr.Get(EE_CHAR_OVERLINE)).GetLineStyle();
            aNewAttr.Put(SvxOverlineItem(eFO == UNDERLINE_SINGLE ? UNDERLINE_NONE : UNDERLINE_SINGLE, EE_CHAR_OVERLINE));
            break;
        }
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
        case SID_SELECTALL:
        {
            Outliner * pOutliner = pOLV->GetOutliner();
            if(pOutliner)
            {
                sal_Int32 nParaCount = pOutliner->GetParagraphCount();
                if (nParaCount > 0)
                    pOLV->SelectRange(0L, nParaCount );
            }
            break;
        }
        case FN_FORMAT_RESET:
        {
            pPostItMgr->GetActiveSidebarWin()->ResetAttributes();
            rReq.Done();
            break;
        }
        case FN_SET_SUPER_SCRIPT:
        {
            SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
            SvxEscapement eEsc = (SvxEscapement ) static_cast<const SvxEscapementItem&>(
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

            if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUPERSCRIPT );
            aNewAttr.Put( aItem, EE_CHAR_ESCAPEMENT );
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
            aNewAttr.Put( aItem, EE_CHAR_ESCAPEMENT );
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
                    // Select the field so that it will be deleted during insert
                    ESelection aSel = pOLV->GetSelection();
                    aSel.nEndPos++;
                    pOLV->SetSelection(aSel);
                }
                if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED)
                    pOLV->InsertField(SvxFieldItem(aField, EE_FEATURE_FIELD));
            }
            break;
        }
        case FN_INSERT_SOFT_HYPHEN:
        case FN_INSERT_HARDHYPHEN:
        case FN_INSERT_HARD_SPACE:
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_ZWNBSP :
        case SID_INSERT_ZWSP:
        {
            sal_Unicode cIns = 0;
            switch(rReq.GetSlot())
            {
                case FN_INSERT_SOFT_HYPHEN: cIns = CHAR_SOFTHYPHEN; break;
                case FN_INSERT_HARDHYPHEN: cIns = CHAR_HARDHYPHEN; break;
                case FN_INSERT_HARD_SPACE: cIns = CHAR_HARDBLANK; break;
                case SID_INSERT_RLM : cIns = CHAR_RLM ; break;
                case SID_INSERT_LRM : cIns = CHAR_LRM ; break;
                case SID_INSERT_ZWSP : cIns = CHAR_ZWSP ; break;
                case SID_INSERT_ZWNBSP: cIns = CHAR_ZWNBSP; break;
            }
            pOLV->InsertText( OUString(cIns));
            rReq.Done();
            break;
        }
        case SID_CHARMAP:
        {
            if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED)
                InsertSymbol(rReq);
            break;
        }
        case FN_INSERT_STRING:
        {
            const SfxPoolItem* pItem = nullptr;
            if (pNewAttrs)
                pNewAttrs->GetItemState(nSlot, false, &pItem );
            if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED)
                pOLV->InsertText(static_cast<const SfxStringItem *>(pItem)->GetValue());
            break;
        }
        case FN_FORMAT_FOOTNOTE_DLG:
        {
            rView.ExecFormatFootnote();
            break;
        }
        case FN_NUMBERING_OUTLINE_DLG:
        {
            rView.ExecNumberingOutline(GetPool());
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
            rView.UpdateWordCount(this, nSlot);
            break;
        }
        case SID_CHAR_DLG_EFFECT:
        case SID_CHAR_DLG:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxStringItem* pItem = rReq.GetArg<SfxStringItem>(FN_PARAM_1);

            if( !pArgs || pItem )
            {
                /* mod
                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(dynamic_cast<SwWebView*>( pView) !=  nullptr );
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
                */
                SfxItemSet aDlgAttr(GetPool(), EE_ITEMS_START, EE_ITEMS_END);

                // util::Language does not exist in the EditEngine! Therefore not included in the set.

                aDlgAttr.Put( aEditAttr );
                aDlgAttr.Put( SvxKerningItem(0, RES_CHRATR_KERNING) );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateSwCharDlg( rView.GetWindow(), rView, aDlgAttr, DLG_CHAR_ANN));
                OSL_ENSURE(pDlg, "Dialog creation failed!");
                if (nSlot == SID_CHAR_DLG_EFFECT)
                {
                    pDlg->SetCurPageId("fonteffects");
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
        break;
        }
        case SID_PARA_DLG:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (!pArgs)
            {
                /* mod todo ???
                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(dynamic_cast<SwWebView*>( pView) !=  nullptr );
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
                */
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
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateSwParaDlg( rView.GetWindow(), rView, aDlgAttr,DLG_STD, nullptr, true ));
                OSL_ENSURE(pDlg, "Dialog creation failed!");
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
            break;
        }

        case SID_AUTOSPELL_CHECK:
        {
            rView.ExecuteSlot(rReq);
            break;
        }
        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
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
            pOLV->SetAttribs(aAttr);
            break;
        }
    }

    if(nEEWhich && pNewAttrs)
        aNewAttr.Put(pNewAttrs->Get(nWhich), nEEWhich);

    Rectangle aNullRect;
    Rectangle aOutRect = pOLV->GetOutputArea();
    if (aNullRect != aOutRect)
        pOLV->SetAttribs(aNewAttr);

    rView.GetViewFrame()->GetBindings().InvalidateAll(false);
    if ( pOLV->GetOutliner()->IsModified() )
        rView.GetWrtShell().SetModified();

}

void SwAnnotationShell::GetState(SfxItemSet& rSet)
{
    //TODO: clean this up!!!
    // FN_SET_SUPER_SCRIPT
    //SID_ATTR_PARA_ADJUST
    //SID_ATTR_PARA_ADJUST_BLOCK

    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while(nWhich)
    {
        sal_uInt16 nEEWhich = 0;
        sal_uInt16 nSlotId = GetPool().GetSlotId( nWhich );
        switch( nSlotId )
        {
            case SID_ATTR_PARA_LRSPACE:
            {
                SfxItemState eState = aEditAttr.GetItemState( EE_PARA_LRSPACE );
                if( eState >= SfxItemState::DEFAULT )
                {
                    SvxLRSpaceItem aLR = static_cast<const SvxLRSpaceItem&>( aEditAttr.Get( EE_PARA_LRSPACE ) );
                    aLR.SetWhich(SID_ATTR_PARA_LRSPACE);
                    rSet.Put(aLR);
                }
                else
                    rSet.InvalidateItem(nSlotId);
            }
            break;
            case SID_ATTR_PARA_LINESPACE:
            {
                SfxItemState eState = aEditAttr.GetItemState( EE_PARA_SBL );
                if( eState >= SfxItemState::DEFAULT )
                {
                    SvxLineSpacingItem aLR = static_cast<const SvxLineSpacingItem&>( aEditAttr.Get( EE_PARA_SBL ) );
                    rSet.Put(aLR);
                }
                else
                    rSet.InvalidateItem(nSlotId);
            }
            break;
            case SID_ATTR_PARA_ULSPACE:
            case SID_PARASPACE_INCREASE:
            case SID_PARASPACE_DECREASE:
                {
                    SfxItemState eState = aEditAttr.GetItemState( EE_PARA_ULSPACE );
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
                }
                break;
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
            case SID_ATTR_CHAR_WEIGHT:
            case SID_ATTR_CHAR_POSTURE:
                {
                    SvtScriptType nScriptType = pOLV->GetSelectedScriptType();
                    SfxItemPool* pSecondPool = aEditAttr.GetPool()->GetSecondaryPool();
                    if( !pSecondPool )
                        pSecondPool = aEditAttr.GetPool();
                    SvxScriptSetItem aSetItem( nSlotId, *pSecondPool );
                    aSetItem.GetItemSet().Put( aEditAttr, false );
                    const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScriptType );
                    if( pI )
                        rSet.Put( *pI, nWhich );
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
            case SID_ATTR_CHAR_LANGUAGE    : nEEWhich = EE_CHAR_LANGUAGE;break;
            case SID_ATTR_CHAR_ESCAPEMENT: nEEWhich = EE_CHAR_ESCAPEMENT;break;
            case SID_ATTR_CHAR_KERNING:  nEEWhich = EE_CHAR_KERNING;break;
            case FN_SET_SUPER_SCRIPT:
            case FN_SET_SUB_SCRIPT:
            {
                sal_uInt16 nEsc = 0;
                const SfxPoolItem *pEscItem = nullptr;
                if (nWhich==FN_SET_SUPER_SCRIPT)
                    nEsc = SVX_ESCAPEMENT_SUPERSCRIPT;
                else
                    nEsc = SVX_ESCAPEMENT_SUBSCRIPT;

                if( !pEscItem )
                    pEscItem = &aEditAttr.Get( EE_CHAR_ESCAPEMENT );

                if( nEsc == static_cast<const SvxEscapementItem*>(pEscItem)->GetEnumValue() )
                    rSet.Put( SfxBoolItem( nWhich, true ));
                else
                    rSet.InvalidateItem( nWhich );
                break;
            }
            case SID_ATTR_PARA_ADJUST_LEFT:
            case SID_ATTR_PARA_ADJUST_RIGHT:
            case SID_ATTR_PARA_ADJUST_CENTER:
            case SID_ATTR_PARA_ADJUST_BLOCK:
                {
                    const SfxPoolItem *pAdjust = nullptr;
                    int eAdjust = 0;

                    if (nWhich==SID_ATTR_PARA_ADJUST_LEFT)
                        eAdjust = SVX_ADJUST_LEFT;
                    else if (nWhich==SID_ATTR_PARA_ADJUST_RIGHT)
                        eAdjust = SVX_ADJUST_RIGHT;
                    else if (nWhich==SID_ATTR_PARA_ADJUST_CENTER)
                        eAdjust = SVX_ADJUST_CENTER;
                    else if (nWhich==SID_ATTR_PARA_ADJUST_BLOCK)
                        eAdjust = SVX_ADJUST_BLOCK;

                    if( !pAdjust )
                        aEditAttr.GetItemState( EE_PARA_JUST, false, &pAdjust);

                    if( !pAdjust || IsInvalidItem( pAdjust ))
                        rSet.InvalidateItem( nSlotId ), nSlotId = 0;
                    else
                    {
                        if ( eAdjust == static_cast<const SvxAdjustItem*>(pAdjust)->GetAdjust())
                            rSet.Put( SfxBoolItem( nWhich, true ));
                        else
                            rSet.InvalidateItem( nWhich );
                    }
                    break;
                }
            case SID_ATTR_PARA_LINESPACE_10:
            case SID_ATTR_PARA_LINESPACE_15:
            case SID_ATTR_PARA_LINESPACE_20:
                {
                    const SfxPoolItem *pLSpace = nullptr;
                    int nLSpace = 0;

                    if (nWhich==SID_ATTR_PARA_LINESPACE_10)
                        nLSpace = 100;
                    else if (nWhich==SID_ATTR_PARA_LINESPACE_15)
                        nLSpace = 150;
                    else if (nWhich==SID_ATTR_PARA_LINESPACE_20)
                        nLSpace = 200;

                    if( !pLSpace )
                        aEditAttr.GetItemState( EE_PARA_SBL, false, &pLSpace );

                    if( !pLSpace || IsInvalidItem( pLSpace ))
                        rSet.InvalidateItem( nSlotId ), nSlotId = 0;
                    else
                    {
                        if( nLSpace == static_cast<const SvxLineSpacingItem*>(pLSpace)->GetPropLineSpace() )
                            rSet.Put( SfxBoolItem( nWhich, true ));
                        else
                            rSet.InvalidateItem( nWhich );
                    }
                    break;
                }
            case SID_AUTOSPELL_CHECK:
            {
                const SfxPoolItem* pState = rView.GetSlotState(nWhich);
                if (pState)
                    rSet.Put(SfxBoolItem(nWhich, static_cast<const SfxBoolItem*>(pState)->GetValue()));
                else
                    rSet.DisableItem( nWhich );
                break;
            }
            case SID_ATTR_PARA_LEFT_TO_RIGHT:
            case SID_ATTR_PARA_RIGHT_TO_LEFT:
            {
                if ( !SvtLanguageOptions().IsCTLFontEnabled() )
                    rSet.DisableItem( nWhich );
                else
                {
                    if(pOLV->GetOutliner() && pOLV->GetOutliner()->IsVertical())
                        rSet.DisableItem( nWhich );
                    else
                    {
                        bool bFlag = false;
                        switch( static_cast<const SvxFrameDirectionItem&>( aEditAttr.Get( EE_PARA_WRITINGDIR ) ).GetValue() )
                        {
                            case FRMDIR_HORI_LEFT_TOP:
                            {
                                bFlag = nWhich == SID_ATTR_PARA_LEFT_TO_RIGHT;
                                rSet.Put( SfxBoolItem( nWhich, bFlag ));
                                break;
                            }
                            case FRMDIR_HORI_RIGHT_TOP:
                            {
                                bFlag = nWhich != SID_ATTR_PARA_LEFT_TO_RIGHT;
                                rSet.Put( SfxBoolItem( nWhich, bFlag ));
                                break;
                            }
                        }
                    }
                }
            }
            break;
            case SID_INSERT_RLM :
            case SID_INSERT_LRM :
            case SID_INSERT_ZWNBSP :
            case SID_INSERT_ZWSP:
            {
                SvtCTLOptions aCTLOptions;
                bool bEnabled = aCTLOptions.IsCTLFontEnabled();
                rView.GetViewFrame()->GetBindings().SetVisibleState( nWhich, bEnabled );
                if(!bEnabled)
                    rSet.DisableItem(nWhich);
            }
            break;
            default:
                rSet.InvalidateItem( nWhich );
                break;
        }

        if(nEEWhich)
        {
            rSet.Put(aEditAttr.Get(nEEWhich), nWhich);
        if(nEEWhich == EE_CHAR_KERNING)
        {
            SfxItemState eState = aEditAttr.GetItemState( EE_CHAR_KERNING );
            if ( eState == SfxItemState::DONTCARE )
            {
                rSet.InvalidateItem(EE_CHAR_KERNING);
            }
        }
        }

        if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()==SwPostItHelper::DELETED)
            rSet.DisableItem( nWhich );

        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::ExecSearch(SfxRequest& rReq, bool bNoMessage)
{
    rView.ExecSearch(rReq,bNoMessage);
}

void SwAnnotationShell::StateSearch(SfxItemSet &rSet)
{
    rView.StateSearch(rSet);
}

void SwAnnotationShell::ExecClpbrd(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());
    SfxItemSet aNewAttr(*aEditAttr.GetPool(), aEditAttr.GetRanges());

    long aOldHeight = pPostItMgr->GetActiveSidebarWin()->GetPostItTextHeight();
    sal_uInt16 nSlot = rReq.GetSlot();
    switch (nSlot)
    {
        case SID_CUT:
            if ( (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED) && pOLV->HasSelection() )
                pOLV->Cut();
            break;
        case SID_COPY:
            if( pOLV->HasSelection() )
                pOLV->Copy();
            break;
        case SID_PASTE:
            if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED)
                pOLV->Paste();
            break;
        case SID_PASTE_SPECIAL:
        {
            if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED)
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                std::unique_ptr<SfxAbstractPasteDialog> pDlg(pFact->CreatePasteDialog( &rView.GetEditWin() ));

                pDlg->Insert( SotClipboardFormatId::STRING, OUString() );
                pDlg->Insert( SotClipboardFormatId::RTF,    OUString() );

                TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( &rView.GetEditWin() ) );

                SotClipboardFormatId nFormat = pDlg->GetFormat( aDataHelper.GetTransferable() );

                if (nFormat != SotClipboardFormatId::NONE)
                {
                    if (nFormat == SotClipboardFormatId::STRING)
                        pOLV->Paste();
                    else
                        pOLV->PasteSpecial();
                }
            }
            break;
        }
        case SID_CLIPBOARD_FORMAT_ITEMS:
        {
            SotClipboardFormatId nFormat = SotClipboardFormatId::NONE;
            const SfxPoolItem* pItem;
            if ( rReq.GetArgs() && rReq.GetArgs()->GetItemState(nSlot, true, &pItem) == SfxItemState::SET &&
                                    dynamic_cast< const SfxUInt32Item *>( pItem ) !=  nullptr )
            {
                nFormat = static_cast<SotClipboardFormatId>(static_cast<const SfxUInt32Item*>(pItem)->GetValue());
            }

            if ( nFormat != SotClipboardFormatId::NONE )
            {
                if (SotClipboardFormatId::STRING == nFormat)
                    pOLV->Paste();
                else
                    pOLV->PasteSpecial();
            }
            break;
        }
    }
    pPostItMgr->GetActiveSidebarWin()->ResizeIfNecessary(aOldHeight,pPostItMgr->GetActiveSidebarWin()->GetPostItTextHeight());
}

void SwAnnotationShell::StateClpbrd(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;
    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();

    TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( &rView.GetEditWin() ) );
    bool bPastePossible = ( aDataHelper.HasFormat( SotClipboardFormatId::STRING ) || aDataHelper.HasFormat( SotClipboardFormatId::RTF ) );
    bPastePossible = bPastePossible &&  (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()!=SwPostItHelper::DELETED);

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_CUT:
            {
                if ( (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()==SwPostItHelper::DELETED) || !pOLV->HasSelection() )
                    rSet.DisableItem( nWhich );
                break;
            }
            case SID_COPY:
            {
                if( !pOLV->HasSelection() )
                    rSet.DisableItem( nWhich );
                break;
            }
            case SID_PASTE:
            case SID_PASTE_SPECIAL:
                {
                    if( !bPastePossible )
                        rSet.DisableItem( nWhich );
                    break;
                }
            case SID_CLIPBOARD_FORMAT_ITEMS:
                {
                    if ( bPastePossible )
                    {
                        SvxClipboardFormatItem aFormats( SID_CLIPBOARD_FORMAT_ITEMS );
                        if ( aDataHelper.HasFormat( SotClipboardFormatId::RTF ) )
                            aFormats.AddClipbrdFormat( SotClipboardFormatId::RTF );
                        aFormats.AddClipbrdFormat( SotClipboardFormatId::STRING );
                        rSet.Put( aFormats );
                    }
                    else
                        rSet.DisableItem( nWhich );
                    break;
                }
        }
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::StateStatusLine(SfxItemSet &rSet)
{
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    while( nWhich )
    {
        switch( nWhich )
        {
            case FN_STAT_SELMODE:
            {
                rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 0));
                rSet.DisableItem( nWhich );
                break;
            }
            case FN_STAT_TEMPLATE:
            {
                rSet.DisableItem( nWhich );
                break;
            }
        }
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::StateInsert(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
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
                        const SvxFieldData* pField = pFieldItem->GetField();

                        if (dynamic_cast< const SvxURLField *>( pField ) !=  nullptr)
                        {
                            aHLinkItem.SetName(static_cast<const SvxURLField*>( pField)->GetRepresentation());
                            aHLinkItem.SetURL(static_cast<const SvxURLField*>( pField)->GetURL());
                            aHLinkItem.SetTargetFrame(static_cast<const SvxURLField*>( pField)->GetTargetFrame());
                        }
                    }
                    else
                    {
                        OUString sSel(pOLV->GetSelected());
                        sSel = sSel.copy(0, std::min<sal_Int32>(255, sSel.getLength()));
                        aHLinkItem.SetName(comphelper::string::stripEnd(sSel, ' '));
                    }

                    sal_uInt16 nHtmlMode = ::GetHtmlMode(rView.GetDocShell());
                    aHLinkItem.SetInsertMode((SvxLinkInsertMode)(aHLinkItem.GetInsertMode() |
                        ((nHtmlMode & HTMLMODE_ON) != 0 ? HLINK_HTMLMODE : 0)));

                    rSet.Put(aHLinkItem);
                }
                break;
        }

        if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()==SwPostItHelper::DELETED)
            rSet.DisableItem( nWhich );

        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::NoteExec(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr )
        return;

    sal_uInt16 nSlot = rReq.GetSlot();
    switch (nSlot)
    {
    case FN_REPLY:
    case FN_POSTIT:
    case FN_DELETE_COMMENT:
        if ( pPostItMgr->HasActiveSidebarWin() )
            pPostItMgr->GetActiveSidebarWin()->ExecuteCommand(nSlot);
        break;

        case FN_DELETE_ALL_NOTES:
            pPostItMgr->Delete();
            break;
        case FN_FORMAT_ALL_NOTES:
            pPostItMgr->ExecuteFormatAllDialog(rView);
            break;
        case FN_DELETE_NOTE_AUTHOR:
        {
            const SfxStringItem* pItem = rReq.GetArg<SfxStringItem>(nSlot);
            if ( pItem )
                pPostItMgr->Delete( pItem->GetValue() );
            break;
        }
        case FN_HIDE_NOTE:
            break;
        case FN_HIDE_ALL_NOTES:
            pPostItMgr->Hide();
            break;
        case FN_HIDE_NOTE_AUTHOR:
        {
            const SfxStringItem* pItem = rReq.GetArg<SfxStringItem>(nSlot);
            if ( pItem )
                pPostItMgr->Hide( pItem->GetValue() );
        }
    }
}

void SwAnnotationShell::GetNoteState(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while(nWhich)
    {
        sal_uInt16 nSlotId = GetPool().GetSlotId( nWhich );
        switch( nSlotId )
        {
        case FN_POSTIT:
        case FN_DELETE_NOTE_AUTHOR:
        case FN_DELETE_ALL_NOTES:
        case FN_FORMAT_ALL_NOTES:
        case FN_HIDE_NOTE:
        case FN_HIDE_NOTE_AUTHOR:
        case FN_HIDE_ALL_NOTES:
        case FN_DELETE_COMMENT:
            {
                if( !pPostItMgr
                    || !pPostItMgr->HasActiveAnnotationWin() )
                {
                    rSet.DisableItem(nWhich);
                }
                break;
            }

        case FN_REPLY:
            {
                if ( !pPostItMgr ||
                     !pPostItMgr->HasActiveAnnotationWin() )
                {
                    rSet.DisableItem(nWhich);
                }
                else
                {
                    SvtUserOptions aUserOpt;
                    OUString sAuthor;
                    if( (sAuthor = aUserOpt.GetFullName()).isEmpty() &&
                        (sAuthor = aUserOpt.GetID()).isEmpty() )
                        sAuthor = SW_RES( STR_REDLINE_UNKNOWN_AUTHOR );
                    if (sAuthor == pPostItMgr->GetActiveSidebarWin()->GetAuthor())
                        rSet.DisableItem(nWhich);
                }
                break;
            }
            default:
                rSet.InvalidateItem( nWhich );
                break;
        }

        if (pPostItMgr && pPostItMgr->HasActiveSidebarWin())
        {
            if ( (pPostItMgr->GetActiveSidebarWin()->IsProtected()) &&
                    ( (nSlotId==FN_DELETE_COMMENT) || (nSlotId==FN_REPLY) ) )
                rSet.DisableItem( nWhich );
        }
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::ExecLingu(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());
    sal_uInt16 nSlot = rReq.GetSlot();
    SwWrtShell &rSh = rView.GetWrtShell();
    bool bRestoreSelection = false;
    ESelection aOldSelection;

    switch (nSlot)
    {
        case SID_LANGUAGE_STATUS:
        {
            aOldSelection = pOLV->GetSelection();
            if (!pOLV->GetEditView().HasSelection())
            {
                pOLV->GetEditView().SelectCurrentWord();
            }

            bRestoreSelection = SwLangHelper::SetLanguageStatus(pOLV,rReq,rView,rSh);
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
        case SID_THESAURUS:
        {
            pOLV->StartThesaurus();
            break;
        }
        case SID_HANGUL_HANJA_CONVERSION:
            pOLV->StartTextConversion( LANGUAGE_KOREAN, LANGUAGE_KOREAN, nullptr,
                    i18n::TextConversionOption::CHARACTER_BY_CHARACTER, true, false );
            break;

        case SID_CHINESE_CONVERSION:
        {
                //open ChineseTranslationDialog
                Reference< XComponentContext > xContext(
                    ::cppu::defaultBootstrap_InitialComponentContext() ); //@todo get context from calc if that has one
                if(xContext.is())
                {
                    Reference< lang::XMultiComponentFactory > xMCF( xContext->getServiceManager() );
                    if(xMCF.is())
                    {
                        Reference< ui::dialogs::XExecutableDialog > xDialog(
                                xMCF->createInstanceWithContext(
                                    "com.sun.star.linguistic2.ChineseTranslationDialog", xContext),
                                UNO_QUERY);
                        Reference< lang::XInitialization > xInit( xDialog, UNO_QUERY );
                        if( xInit.is() )
                        {
                            //  initialize dialog
                            Reference< awt::XWindow > xDialogParentWindow(nullptr);
                            Sequence<Any> aSeq(1);
                            Any* pArray = aSeq.getArray();
                            PropertyValue aParam;
                            aParam.Name = "ParentWindow";
                            aParam.Value <<= makeAny(xDialogParentWindow);
                            pArray[0] <<= makeAny(aParam);
                            xInit->initialize( aSeq );

                            //execute dialog
                            sal_Int16 nDialogRet = xDialog->execute();
                            if( RET_OK == nDialogRet )
                            {
                                //get some parameters from the dialog
                                bool bToSimplified = true;
                                bool bUseVariants = true;
                                bool bCommonTerms = true;
                                Reference< beans::XPropertySet >  xProp( xDialog, UNO_QUERY );
                                if( xProp.is() )
                                {
                                    try
                                    {
                                        xProp->getPropertyValue( "IsDirectionToSimplified" ) >>= bToSimplified;
                                        xProp->getPropertyValue( "IsUseCharacterVariants" ) >>= bUseVariants;
                                        xProp->getPropertyValue( "IsTranslateCommonTerms" ) >>= bCommonTerms;
                                    }
                                    catch (const Exception&)
                                    {
                                    }
                                }

                                //execute translation
                                sal_Int16 nSourceLang = bToSimplified ? LANGUAGE_CHINESE_TRADITIONAL : LANGUAGE_CHINESE_SIMPLIFIED;
                                sal_Int16 nTargetLang = bToSimplified ? LANGUAGE_CHINESE_SIMPLIFIED : LANGUAGE_CHINESE_TRADITIONAL;
                                sal_Int32 nOptions    = bUseVariants ? i18n::TextConversionOption::USE_CHARACTER_VARIANTS : 0;
                                if( !bCommonTerms )
                                    nOptions = nOptions | i18n::TextConversionOption::CHARACTER_BY_CHARACTER;

                                vcl::Font aTargetFont = OutputDevice::GetDefaultFont( DefaultFontType::CJK_TEXT,
                                            nTargetLang, GetDefaultFontFlags::OnlyOne );

                                pOLV->StartTextConversion( nSourceLang, nTargetLang, &aTargetFont, nOptions, false, false );
                            }
                        }
                        Reference< lang::XComponent > xComponent( xDialog, UNO_QUERY );
                        if( xComponent.is() )
                            xComponent->dispose();
                    }
                }
            }
            break;
    }

    if (bRestoreSelection)
    {
        // restore selection
        pOLV->GetEditView().SetSelection( aOldSelection );
    }
}

void SwAnnotationShell::GetLinguState(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while(nWhich)
    {
        switch (nWhich)
        {
            case SID_LANGUAGE_STATUS:
            {
                SwLangHelper::GetLanguageStatus(pOLV,rSet);
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
                break;
            }

            // disable "Thesaurus" if the language is not supported
            case SID_THESAURUS:
            {
                const SfxPoolItem &rItem = rView.GetWrtShell().GetDoc()->GetDefault(
                            GetWhichOfScript( RES_CHRATR_LANGUAGE,
                            SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage())) );
                LanguageType nLang = static_cast<const SvxLanguageItem &>(
                                                        rItem).GetLanguage();
                uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
                if (!xThes.is() || nLang == LANGUAGE_NONE ||
                    !xThes->hasLocale( LanguageTag::convertToLocale( nLang ) ))
                    rSet.DisableItem( SID_THESAURUS );
            }
            break;
            case SID_HANGUL_HANJA_CONVERSION:
            case SID_CHINESE_CONVERSION:
            {
                if (!SvtCJKOptions().IsAnyEnabled())
                {
                    rView.GetViewFrame()->GetBindings().SetVisibleState( nWhich, false );
                    rSet.DisableItem(nWhich);
                }
                else
                    rView.GetViewFrame()->GetBindings().SetVisibleState( nWhich, true );
            }
            break;
        }

        if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()==SwPostItHelper::DELETED)
            rSet.DisableItem( nWhich );

        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::ExecTransliteration(SfxRequest &rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if (!pPostItMgr || !pPostItMgr->HasActiveSidebarWin())
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();

    if (!pOLV)
        return;

    using namespace ::com::sun::star::i18n;

    sal_uInt32 nMode = 0;

    switch( rReq.GetSlot() )
    {
        case SID_TRANSLITERATE_SENTENCE_CASE:
            nMode = TransliterationModulesExtra::SENTENCE_CASE;
            break;
        case SID_TRANSLITERATE_TITLE_CASE:
            nMode = TransliterationModulesExtra::TITLE_CASE;
            break;
        case SID_TRANSLITERATE_TOGGLE_CASE:
            nMode = TransliterationModulesExtra::TOGGLE_CASE;
            break;
        case SID_TRANSLITERATE_UPPER:
            nMode = TransliterationModules_LOWERCASE_UPPERCASE;
            break;
        case SID_TRANSLITERATE_LOWER:
            nMode = TransliterationModules_UPPERCASE_LOWERCASE;
            break;
        case SID_TRANSLITERATE_HALFWIDTH:
            nMode = TransliterationModules_FULLWIDTH_HALFWIDTH;
            break;
        case SID_TRANSLITERATE_FULLWIDTH:
            nMode = TransliterationModules_HALFWIDTH_FULLWIDTH;
            break;
        case SID_TRANSLITERATE_HIRAGANA:
            nMode = TransliterationModules_KATAKANA_HIRAGANA;
            break;
        case SID_TRANSLITERATE_KATAGANA:
            nMode = TransliterationModules_HIRAGANA_KATAKANA;
            break;

        default:
            OSL_ENSURE(false, "wrong dispatcher");
    }

    if( nMode )
        pOLV->TransliterateText( nMode );
}

void SwAnnotationShell::ExecRotateTransliteration( SfxRequest & rReq )
{
    if( rReq.GetSlot() == SID_TRANSLITERATE_ROTATE_CASE )
    {
        SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
        if (!pPostItMgr || !pPostItMgr->HasActiveSidebarWin())
            return;

        OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();

        if (!pOLV)
            return;

        pOLV->TransliterateText(m_aRotateCase.getNextMode());
    }
}

void SwAnnotationShell::ExecUndo(SfxRequest &rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    ::svl::IUndoManager* pUndoManager = GetUndoManager();
    SwWrtShell &rSh = rView.GetWrtShell();

    long aOldHeight = rView.GetPostItMgr()->HasActiveSidebarWin()
                      ? rView.GetPostItMgr()->GetActiveSidebarWin()->GetPostItTextHeight()
                      : 0;

    sal_uInt16 nId = rReq.GetSlot();
    sal_uInt16 nCnt = 1;
    const SfxPoolItem* pItem=nullptr;
    if( pArgs && SfxItemState::SET == pArgs->GetItemState( nId, false, &pItem ) )
        nCnt = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
    switch( nId )
    {
        case SID_UNDO:
        {
            if ( pUndoManager )
            {
                sal_uInt16 nCount = pUndoManager->GetUndoActionCount();
                sal_uInt16 nSteps = nCnt;
                if ( nCount < nCnt )
                {
                    nCnt = nCnt - nCount;
                    nSteps = nCount;
                }
                else
                    nCnt = 0;

                while( nSteps-- )
                    pUndoManager->Undo();
            }

            if ( nCnt )
                rSh.Do( SwWrtShell::UNDO, nCnt );

            break;
        }

        case SID_REDO:
        {
            if ( pUndoManager )
            {
                sal_uInt16 nCount = pUndoManager->GetRedoActionCount();
                sal_uInt16 nSteps = nCnt;
                if ( nCount < nCnt )
                {
                    nCnt = nCnt - nCount;
                    nSteps = nCount;
                }
                else
                    nCnt = 0;

                while( nSteps-- )
                    pUndoManager->Redo();
            }

            if ( nCnt )
                rSh.Do( SwWrtShell::REDO, nCnt );

            break;
        }
    }

    rView.GetViewFrame()->GetBindings().InvalidateAll(false);

    if (rView.GetPostItMgr()->HasActiveSidebarWin())
        rView.GetPostItMgr()->GetActiveSidebarWin()->ResizeIfNecessary(aOldHeight,rView.GetPostItMgr()->GetActiveSidebarWin()->GetPostItTextHeight());
}

void SwAnnotationShell::StateUndo(SfxItemSet &rSet)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    ::svl::IUndoManager* pUndoManager = GetUndoManager();
    SfxViewFrame *pSfxViewFrame = rView.GetViewFrame();
    SwWrtShell &rSh = rView.GetWrtShell();

    while( nWhich )
    {
        switch ( nWhich )
        {
        case SID_UNDO:
            {
                sal_uInt16 nCount = pUndoManager ? pUndoManager->GetUndoActionCount() : 0;
                if ( nCount )
                    pSfxViewFrame->GetSlotState( nWhich, pSfxViewFrame->GetInterface(), &rSet );
                else if (rSh.GetLastUndoInfo(nullptr, nullptr))
                {
                    rSet.Put( SfxStringItem( nWhich, rSh.GetDoString(SwWrtShell::UNDO)) );
                }
                else
                    rSet.DisableItem(nWhich);
                break;
            }
        case SID_REDO:
            {
                sal_uInt16 nCount = pUndoManager ? pUndoManager->GetRedoActionCount() : 0;
                if ( nCount )
                    pSfxViewFrame->GetSlotState( nWhich, pSfxViewFrame->GetInterface(), &rSet );
                else if (rSh.GetFirstRedoInfo(nullptr))
                {
                    rSet.Put(SfxStringItem( nWhich, rSh.GetDoString(SwWrtShell::REDO)) );
                }
                else
                    rSet.DisableItem(nWhich);
                break;
            }
        case SID_GETUNDOSTRINGS:
        case SID_GETREDOSTRINGS:
            {
                if( pUndoManager )
                {
                    OUString (::svl::IUndoManager:: *fnGetComment)( size_t, bool const ) const;

                    sal_uInt16 nCount;
                    if( SID_GETUNDOSTRINGS == nWhich )
                    {
                        nCount = pUndoManager->GetUndoActionCount();
                        fnGetComment = &::svl::IUndoManager::GetUndoActionComment;
                    }
                    else
                    {
                        nCount = pUndoManager->GetRedoActionCount();
                        fnGetComment = &::svl::IUndoManager::GetRedoActionComment;
                    }

                    OUString sList;
                    if( nCount )
                    {
                        for( sal_uInt16 n = 0; n < nCount; ++n )
                            sList += (pUndoManager->*fnGetComment)( n, ::svl::IUndoManager::TopLevel ) + "\n";
                    }

                    SfxStringListItem aItem( nWhich );
                    if ((nWhich == SID_GETUNDOSTRINGS) &&
                        rSh.GetLastUndoInfo(nullptr, nullptr))
                    {
                        rSh.GetDoStrings( SwWrtShell::UNDO, aItem );
                    }
                    else if ((nWhich == SID_GETREDOSTRINGS) &&
                             (rSh.GetFirstRedoInfo(nullptr)))
                    {
                        rSh.GetDoStrings( SwWrtShell::UNDO, aItem );
                    }

                    sList += aItem.GetString();
                    aItem.SetString( sList );
                    rSet.Put( aItem );
                }
                else
                    rSet.DisableItem( nWhich );
            }
            break;

        default:
            {
                pSfxViewFrame->GetSlotState( nWhich, pSfxViewFrame->GetInterface(), &rSet );
                break;
            }

        }

        if (pPostItMgr->GetActiveSidebarWin()->GetLayoutStatus()==SwPostItHelper::DELETED)
            rSet.DisableItem( nWhich );

        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::StateDisableItems( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

void SwAnnotationShell::InsertSymbol(SfxRequest& rReq)
{
    SwPostItMgr* pPostItMgr = rView.GetPostItMgr();
    if ( !pPostItMgr || !pPostItMgr->HasActiveSidebarWin() )
        return;

    OutlinerView* pOLV = pPostItMgr->GetActiveSidebarWin()->GetOutlinerView();

    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem = nullptr;
    if( pArgs )
        pArgs->GetItemState(GetPool().GetWhich(SID_CHARMAP), false, &pItem);

    OUString sSym;
    OUString sFontName;
    if ( pItem )
    {
        sSym = static_cast<const SfxStringItem*>(pItem)->GetValue();
        const SfxPoolItem* pFtItem = nullptr;
        pArgs->GetItemState( GetPool().GetWhich(SID_ATTR_SPECIALCHAR), false, &pFtItem);
        const SfxStringItem* pFontItem = dynamic_cast<const SfxStringItem*>( pFtItem  );
        if ( pFontItem )
            sFontName = pFontItem->GetValue();
    }

    SfxItemSet aSet(pOLV->GetAttribs());
    SvtScriptType nScript = pOLV->GetSelectedScriptType();
    SvxFontItem aSetDlgFont( RES_CHRATR_FONT );
    {
        SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, *aSet.GetPool() );
        aSetItem.GetItemSet().Put( aSet, false );
        const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScript );
        if( pI )
            aSetDlgFont = *static_cast<const SvxFontItem*>(pI);
        else
            aSetDlgFont = static_cast<const SvxFontItem&>(aSet.Get( GetWhichOfScript(
                        SID_ATTR_CHAR_FONT,
                        SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() ) )));
        if (sFontName.isEmpty())
            sFontName = aSetDlgFont.GetFamilyName();
    }

    vcl::Font aFont(sFontName, Size(1,1));
    if( sSym.isEmpty() )
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();

        SfxAllItemSet aAllSet( GetPool() );
        aAllSet.Put( SfxBoolItem( FN_PARAM_1, false ) );

        SwViewOption aOpt(*rView.GetWrtShell().GetViewOptions());
        OUString sSymbolFont = aOpt.GetSymbolFont();
        if( !sSymbolFont.isEmpty() )
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, sSymbolFont ) );
        else
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, aSetDlgFont.GetFamilyName() ) );

        // If character is selected then it can be shown.
        std::unique_ptr<SfxAbstractDialog> pDlg(pFact->CreateSfxDialog( rView.GetWindow(), aAllSet,
            rView.GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP ));

        sal_uInt16 nResult = pDlg->Execute();
        if( nResult == RET_OK )
        {
            const SfxStringItem* pCItem = SfxItemSet::GetItem<SfxStringItem>(pDlg->GetOutputItemSet(), SID_CHARMAP, false);
            const SvxFontItem* pFontItem = SfxItemSet::GetItem<SvxFontItem>(pDlg->GetOutputItemSet(), SID_ATTR_CHAR_FONT, false);
            if ( pFontItem )
            {
                aFont.SetName( pFontItem->GetFamilyName() );
                aFont.SetStyleName( pFontItem->GetStyleName() );
                aFont.SetCharSet( pFontItem->GetCharSet() );
                aFont.SetPitch( pFontItem->GetPitch() );
            }

            if ( pCItem )
            {
                sSym  = pCItem->GetValue();
                aOpt.SetSymbolFont(aFont.GetName());
                SW_MOD()->ApplyUsrPref(aOpt, &rView);
            }
        }
    }

    if( !sSym.isEmpty() )
    {
        // do not flicker
        pOLV->HideCursor();
        Outliner * pOutliner = pOLV->GetOutliner();
        pOutliner->SetUpdateMode(false);

        SfxItemSet aOldSet( pOLV->GetAttribs() );
        SfxItemSet aFontSet( *aOldSet.GetPool(),
                            EE_CHAR_FONTINFO, EE_CHAR_FONTINFO,
                            EE_CHAR_FONTINFO_CJK, EE_CHAR_FONTINFO_CJK,
                            EE_CHAR_FONTINFO_CTL, EE_CHAR_FONTINFO_CTL,
                            0 );
        aFontSet.Set( aOldSet );

        // Insert string
        pOLV->InsertText( sSym);

        // Attributing (set font)
        SfxItemSet aSetFont( *aFontSet.GetPool(), aFontSet.GetRanges() );
        SvxFontItem aFontItem (aFont.GetFamily(),    aFont.GetName(),
                                aFont.GetStyleName(), aFont.GetPitch(),
                                aFont.GetCharSet(),
                                EE_CHAR_FONTINFO );
        SvtScriptType nScriptBreak = g_pBreakIt->GetAllScriptsOfText( sSym );
        if( SvtScriptType::LATIN & nScriptBreak )
            aSetFont.Put( aFontItem, EE_CHAR_FONTINFO );
        if( SvtScriptType::ASIAN & nScriptBreak )
            aSetFont.Put( aFontItem, EE_CHAR_FONTINFO_CJK );
        if( SvtScriptType::COMPLEX & nScriptBreak )
            aSetFont.Put( aFontItem, EE_CHAR_FONTINFO_CTL );
        pOLV->SetAttribs(aSetFont);

        // Erase selection
        ESelection aSel(pOLV->GetSelection());
        aSel.nStartPara = aSel.nEndPara;
        aSel.nStartPos = aSel.nEndPos;
        pOLV->SetSelection(aSel);

        // Restore old font
        pOLV->SetAttribs( aFontSet );

        // From now on show it again
        pOutliner->SetUpdateMode(true);
        pOLV->ShowCursor();

        rReq.AppendItem( SfxStringItem( GetPool().GetWhich(SID_CHARMAP), sSym ) );
        if(!aFont.GetName().isEmpty())
            rReq.AppendItem( SfxStringItem( SID_ATTR_SPECIALCHAR, aFont.GetName() ) );
        rReq.Done();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
