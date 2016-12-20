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

#include <config_features.h>

#include <hintids.hxx>
#include <svl/whiter.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/miscopt.hxx>
#include <svl/srchitem.hxx>
#include <svtools/imap.hxx>
#include <sfx2/viewfrm.hxx>
#include <basic/sbstar.hxx>
#include <svl/rectitem.hxx>
#include <svl/ptitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/protitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <vcl/EnumContext.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/svdview.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/commandinfoprovider.hxx>

#include <doc.hxx>
#include <drawdoc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <fmturl.hxx>
#include <fmtclds.hxx>
#include <fmtcnct.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <wview.hxx>
#include <frmatr.hxx>
#include <uitool.hxx>
#include <frmfmt.hxx>
#include <frmsh.hxx>
#include <frmmgr.hxx>
#include <frmdlg.hxx>
#include <swevent.hxx>
#include <usrpref.hxx>
#include <edtwin.hxx>
#include <swdtflvr.hxx>
#include <swwait.hxx>
#include <docstat.hxx>
#include <IDocumentStatistics.hxx>

#include <helpid.h>
#include <cmdid.h>
#include <cfgitems.hxx>
#include <globals.hrc>
#include <popup.hrc>
#include <shells.hrc>
#include "swabstdlg.hxx"
#include "misc.hrc"

#include <svx/dialogs.hrc>

#include <docsh.hxx>
#include <svx/drawitem.hxx>
#include <memory>

#define SwFrameShell
#include <sfx2/msg.hxx>
#include "swslots.hxx"

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

// Prototypes
static void lcl_FrameGetMaxLineWidth(const SvxBorderLine* pBorderLine, SvxBorderLine& rBorderLine);
static const SwFrameFormat* lcl_GetFrameFormatByName(SwWrtShell& rSh, const OUString& rName)
{
    const size_t nCount = rSh.GetFlyCount(FLYCNTTYPE_FRM);
    for( size_t i = 0; i < nCount; ++i )
    {
        const SwFrameFormat* pFormat = rSh.GetFlyNum(i, FLYCNTTYPE_FRM);
        if(pFormat->GetName() == rName)
            return pFormat;
    }
    return nullptr;
}

SFX_IMPL_INTERFACE(SwFrameShell, SwBaseShell)

void SwFrameShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("frame");

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible, RID_FRAME_TOOLBOX);
}

void SwFrameShell::Execute(SfxRequest &rReq)
{
    //First those who do not need FrameMgr.
    SwWrtShell &rSh = GetShell();
    bool bMore = false;
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    sal_uInt16 nSlot = rReq.GetSlot();

    switch ( nSlot )
    {
        case FN_FRAME_TO_ANCHOR:
            if ( rSh.IsFrameSelected() )
            {
                rSh.GotoFlyAnchor();
                rSh.EnterStdMode();
                rSh.CallChgLnk();
            }
            break;
        case SID_FRAME_TO_TOP:
            rSh.SelectionToTop();
            break;

        case SID_FRAME_TO_BOTTOM:
            rSh.SelectionToBottom();
            break;

        case FN_FRAME_UP:
            rSh.SelectionToTop( false );
            break;

        case FN_FRAME_DOWN:
            rSh.SelectionToBottom( false );
            break;
        case FN_INSERT_FRAME:
            if (!pArgs)
            {
                // Frame already exists, open frame dialog for editing.
                SfxStringItem aDefPage(FN_FORMAT_FRAME_DLG, "columns");
                rSh.GetView().GetViewFrame()->GetDispatcher()->ExecuteList(
                        FN_FORMAT_FRAME_DLG,
                        SfxCallMode::SYNCHRON|SfxCallMode::RECORD,
                        { &aDefPage });

            }
            else
            {
                // Frame already exists, only the number of columns will be changed.
                sal_uInt16 nCols = 1;
                if(pArgs->GetItemState(SID_ATTR_COLUMNS, false, &pItem) == SfxItemState::SET)
                    nCols = static_cast<const SfxUInt16Item *>(pItem)->GetValue();

                SfxItemSet aSet(GetPool(),RES_COL,RES_COL);
                rSh.GetFlyFrameAttr( aSet );
                SwFormatCol aCol(static_cast<const SwFormatCol&>(aSet.Get(RES_COL)));
                // GutterWidth will not always passed, hence get firstly
                // (see view2: Execute on this slot)
                sal_uInt16 nGutterWidth = aCol.GetGutterWidth();
                if(!nCols )
                    nCols++;
                aCol.Init(nCols, nGutterWidth, aCol.GetWishWidth());
                aSet.Put(aCol);
                // Template AutoUpdate
                SwFrameFormat* pFormat = rSh.GetSelectedFrameFormat();
                if(pFormat && pFormat->IsAutoUpdateFormat())
                {
                    rSh.AutoUpdateFrame(pFormat, aSet);
                }
                else
                {
                    rSh.StartAllAction();
                    rSh.SetFlyFrameAttr( aSet );
                    rSh.SetModified();
                    rSh.EndAllAction();
                }

            }
            return;

        case SID_HYPERLINK_SETLINK:
        {
            if(pArgs && SfxItemState::SET == pArgs->GetItemState(SID_HYPERLINK_SETLINK, false, &pItem))
            {
                const SvxHyperlinkItem& rHLinkItem = *static_cast<const SvxHyperlinkItem *>(pItem);
                const OUString& rURL = rHLinkItem.GetURL();
                const OUString& rTarget = rHLinkItem.GetTargetFrame();

                SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                rSh.GetFlyFrameAttr( aSet );
                SwFormatURL aURL( static_cast<const SwFormatURL&>(aSet.Get( RES_URL )) );

                OUString sOldName(rHLinkItem.GetName().toAsciiUpperCase());
                OUString sFlyName(rSh.GetFlyName().toAsciiUpperCase());
                if (sOldName != sFlyName)
                {
                    OUString sName(sOldName);
                    sal_uInt16 i = 1;
                    while (rSh.FindFlyByName(sName))
                    {
                        sName = sOldName;
                        sName += "_";
                        sName += OUString::number(i++);
                    }
                    rSh.SetFlyName(sName);
                }
                aURL.SetURL( rURL, false );
                aURL.SetTargetFrameName(rTarget);

                aSet.Put( aURL );
                rSh.SetFlyFrameAttr( aSet );
            }
        }
        break;

        case FN_FRAME_CHAIN:
            rSh.GetView().GetEditWin().SetChainMode( !rSh.GetView().GetEditWin().IsChainMode() );
            break;

        case FN_FRAME_UNCHAIN:
            rSh.Unchain( (SwFrameFormat&)*rSh.GetFlyFrameFormat() );
            GetView().GetViewFrame()->GetBindings().Invalidate(FN_FRAME_CHAIN);
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
            break;
        }
        case SID_OPEN_XML_FILTERSETTINGS:
        {
            HandleOpenXmlFilterSettings(rReq);
        }
        break;
        case FN_WORDCOUNT_DIALOG:
        {
            GetView().UpdateWordCount(this, nSlot);
            break;
        }
        default: bMore = true;
    }

    if ( !bMore )
    {
        return;
    }

    SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE );
    bool bUpdateMgr = true;
    bool bCopyToFormat = false;
    switch ( nSlot )
    {
        case SID_OBJECT_ALIGN_MIDDLE:
        case FN_FRAME_ALIGN_VERT_CENTER:
            aMgr.SetVertOrientation( text::VertOrientation::CENTER );
            break;
        case SID_OBJECT_ALIGN_DOWN :
        case FN_FRAME_ALIGN_VERT_BOTTOM:
            aMgr.SetVertOrientation( text::VertOrientation::BOTTOM );
            break;
        case SID_OBJECT_ALIGN_UP :
        case FN_FRAME_ALIGN_VERT_TOP:
            aMgr.SetVertOrientation( text::VertOrientation::TOP );
            break;

        case FN_FRAME_ALIGN_VERT_CHAR_CENTER:
            aMgr.SetVertOrientation( text::VertOrientation::CHAR_CENTER );
            break;

        case FN_FRAME_ALIGN_VERT_CHAR_BOTTOM:
            aMgr.SetVertOrientation( text::VertOrientation::CHAR_BOTTOM );
            break;

        case FN_FRAME_ALIGN_VERT_CHAR_TOP:
            aMgr.SetVertOrientation( text::VertOrientation::CHAR_TOP );
            break;

        case FN_FRAME_ALIGN_VERT_ROW_CENTER:
            aMgr.SetVertOrientation( text::VertOrientation::LINE_CENTER );
            break;

        case FN_FRAME_ALIGN_VERT_ROW_BOTTOM:
            aMgr.SetVertOrientation( text::VertOrientation::LINE_BOTTOM );
            break;

        case FN_FRAME_ALIGN_VERT_ROW_TOP:
            aMgr.SetVertOrientation( text::VertOrientation::LINE_TOP );
            break;
        case SID_OBJECT_ALIGN_CENTER :
        case FN_FRAME_ALIGN_HORZ_CENTER:
            aMgr.SetHorzOrientation( text::HoriOrientation::CENTER );
            break;
        case SID_OBJECT_ALIGN_RIGHT:
        case FN_FRAME_ALIGN_HORZ_RIGHT:
            aMgr.SetHorzOrientation( text::HoriOrientation::RIGHT );
            break;
        case SID_OBJECT_ALIGN_LEFT:
        case FN_FRAME_ALIGN_HORZ_LEFT:
            aMgr.SetHorzOrientation( text::HoriOrientation::LEFT );
            break;

        case FN_SET_FRM_POSITION:
        {
            aMgr.SetAbsPos(static_cast<const SfxPointItem &>(pArgs->Get
                                (FN_SET_FRM_POSITION)).GetValue());
        }
        break;
        case SID_ATTR_BRUSH:
        {
            if(pArgs)
            {
                aMgr.SetAttrSet( *pArgs );
                bCopyToFormat = true;
            }
        }
        break;
        case SID_ATTR_ULSPACE:
        case SID_ATTR_LRSPACE:
        {
            if(pArgs && SfxItemState::SET == pArgs->GetItemState(GetPool().GetWhich(nSlot), false, &pItem))
            {
                aMgr.SetAttrSet( *pArgs );
                bCopyToFormat = true;
            }
        }
        break;

        case SID_ATTR_TRANSFORM:
        {
            bool bApplyNewSize = false;

            Size aNewSize = aMgr.GetSize();
            if ( SfxItemState::SET == pArgs->GetItemState( SID_ATTR_TRANSFORM_WIDTH, false, &pItem ) )
            {
                aNewSize.setWidth( static_cast< const SfxUInt32Item* >(pItem)->GetValue() );
                bApplyNewSize = true;
            }

            if ( SfxItemState::SET == pArgs->GetItemState( SID_ATTR_TRANSFORM_HEIGHT, false, &pItem ) )
            {
                aNewSize.setHeight( static_cast< const SfxUInt32Item* >(pItem)->GetValue() );
                bApplyNewSize = true;
            }

            if ( bApplyNewSize )
            {
                aMgr.SetSize( aNewSize );
            }
            else
            {
                bUpdateMgr = false;
            }

        }
        break;

        case FN_FORMAT_FRAME_DLG:
        case FN_DRAW_WRAP_DLG:
        {
            const int nSel = rSh.GetSelectionType();
            if (nSel & nsSelectionType::SEL_GRF)
            {
                rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(FN_FORMAT_GRAFIC_DLG);
                bUpdateMgr = false;
            }
            else
            {
                SfxItemSet aSet(GetPool(),  //UUUU sorted by indices
                    RES_FRMATR_BEGIN,       RES_FRMATR_END-1,                       // [82

                    //UUUU FillAttribute support
                    XATTR_FILL_FIRST,       XATTR_FILL_LAST,                        // [1014

                    SID_DOCFRAME,           SID_DOCFRAME,                           // [5598

                    SID_ATTR_BRUSH,         SID_ATTR_BRUSH,                         // [10001
                    SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,                  // [10023
                    SID_ATTR_LRSPACE,       SID_ATTR_ULSPACE,                       // [10048
                    SID_ATTR_PAGE_SIZE,     SID_ATTR_PAGE_SIZE,                     // [10051

                    //UUUU items to hand over XPropertyList things like
                    // XColorList, XHatchList, XGradientList and XBitmapList
                    // to the Area TabPage
                    SID_COLOR_TABLE,        SID_PATTERN_LIST,                        // [10179

                    SID_HTML_MODE,          SID_HTML_MODE,                          // [10414
                    FN_GET_PRINT_AREA,      FN_GET_PRINT_AREA,                      // [21032
                    FN_SURROUND,            FN_HORI_ORIENT,                         // [21303
                    FN_SET_FRM_NAME,        FN_KEEP_ASPECT_RATIO,                   // [21306
                    FN_SET_FRM_ALT_NAME,    FN_SET_FRM_ALT_NAME,                    // [21318
                    FN_OLE_IS_MATH,         FN_MATH_BASELINE_ALIGNMENT,             // [22314
                    FN_PARAM_CHAIN_PREVIOUS, FN_PARAM_CHAIN_NEXT,                   // [22420

                    0);

                //UUUU create needed items for XPropertyList entries from the DrawModel so that
                // the Area TabPage can access them
                const SwDrawModel* pDrawModel = rSh.GetView().GetDocShell()->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();
                pDrawModel->PutAreaListItems(aSet);

                const SwViewOption* pVOpt = rSh.GetViewOptions();
                if(nSel & nsSelectionType::SEL_OLE)
                    aSet.Put( SfxBoolItem(FN_KEEP_ASPECT_RATIO, pVOpt->IsKeepRatio()) );
                aSet.Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(GetView().GetDocShell())));
                aSet.Put(SfxStringItem(FN_SET_FRM_NAME, rSh.GetFlyName()));
                if( nSel & nsSelectionType::SEL_OLE )
                {
                    // #i73249#
                    aSet.Put( SfxStringItem( FN_SET_FRM_ALT_NAME, rSh.GetObjTitle() ) );
                }

                const SwRect &rPg = rSh.GetAnyCurRect(RECT_PAGE);
                SwFormatFrameSize aFrameSize(ATT_VAR_SIZE, rPg.Width(), rPg.Height());
                aFrameSize.SetWhich(GetPool().GetWhich(SID_ATTR_PAGE_SIZE));
                aSet.Put(aFrameSize);

                const SwRect &rPr = rSh.GetAnyCurRect(RECT_PAGE_PRT);
                SwFormatFrameSize aPrtSize(ATT_VAR_SIZE, rPr.Width(), rPr.Height());
                aPrtSize.SetWhich(GetPool().GetWhich(FN_GET_PRINT_AREA));
                aSet.Put(aPrtSize);

                aSet.Put(aMgr.GetAttrSet());
                aSet.SetParent( aMgr.GetAttrSet().GetParent() );

                // On % values initialize size
                SwFormatFrameSize& rSize = const_cast<SwFormatFrameSize&>(static_cast<const SwFormatFrameSize&>(aSet.Get(RES_FRM_SIZE)));
                if (rSize.GetWidthPercent() && rSize.GetWidthPercent() != SwFormatFrameSize::SYNCED)
                    rSize.SetWidth(rSh.GetAnyCurRect(RECT_FLY_EMBEDDED).Width());
                if (rSize.GetHeightPercent() && rSize.GetHeightPercent() != SwFormatFrameSize::SYNCED)
                    rSize.SetHeight(rSh.GetAnyCurRect(RECT_FLY_EMBEDDED).Height());

                // disable vertical positioning for Math Objects anchored 'as char' if baseline alignment is activated
                aSet.Put( SfxBoolItem( FN_MATH_BASELINE_ALIGNMENT,
                        rSh.GetDoc()->getIDocumentSettingAccess().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT ) ) );
                const uno::Reference < embed::XEmbeddedObject > xObj( rSh.GetOleRef() );
                aSet.Put( SfxBoolItem( FN_OLE_IS_MATH, xObj.is() && SotExchange::IsMath( xObj->getClassID() ) ) );

                OString sDefPage;
                if(pArgs && pArgs->GetItemState(FN_FORMAT_FRAME_DLG, false, &pItem) == SfxItemState::SET)
                    sDefPage = OUStringToOString(static_cast<const SfxStringItem *>(pItem)->GetValue(), RTL_TEXTENCODING_UTF8);

                aSet.Put(SfxFrameItem( SID_DOCFRAME, &GetView().GetViewFrame()->GetFrame()));
                FieldUnit eMetric = ::GetDfltMetric(dynamic_cast<SwWebView*>( &GetView()) != nullptr );
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric) ));
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                assert(pFact);
                ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateFrameTabDialog(
                                                        nSel & nsSelectionType::SEL_GRF ? OUString("PictureDialog") :
                                                        nSel & nsSelectionType::SEL_OLE ? OUString("ObjectDialog"):
                                                                                        OUString("FrameDialog"),
                                                        GetView().GetViewFrame(),
                                                        GetView().GetWindow(),
                                                        aSet,
                                                        false,
                                                        sDefPage));
                assert(pDlg);

                if ( nSlot == FN_DRAW_WRAP_DLG )
                {
                    pDlg->SetCurPageId("wrap");
                }

                if ( pDlg->Execute() )
                {
                    const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                    if(pOutSet)
                    {
                        rReq.Done(*pOutSet);
                        if(nSel & nsSelectionType::SEL_OLE &&
                        SfxItemState::SET == pOutSet->GetItemState(FN_KEEP_ASPECT_RATIO, true, &pItem))
                        {
                            SwViewOption aUsrPref( *pVOpt );
                            aUsrPref.SetKeepRatio(static_cast<const SfxBoolItem*>(pItem)->GetValue());
                            SW_MOD()->ApplyUsrPref(aUsrPref, &GetView());
                        }
                        if (SfxItemState::SET == pOutSet->GetItemState(FN_SET_FRM_ALT_NAME, true, &pItem))
                        {
                            // #i73249#
                            rSh.SetObjTitle(static_cast<const SfxStringItem*>(pItem)->GetValue());
                        }
                        // Template AutoUpdate
                        SwFrameFormat* pFormat = rSh.GetSelectedFrameFormat();
                        if(pFormat && pFormat->IsAutoUpdateFormat())
                        {
                            rSh.AutoUpdateFrame(pFormat, *pOutSet);
                            // Anything which is not supported by the format must be set hard.
                            if(SfxItemState::SET == pOutSet->GetItemState(FN_SET_FRM_NAME, false, &pItem))
                                rSh.SetFlyName(static_cast<const SfxStringItem*>(pItem)->GetValue());
                            SfxItemSet aShellSet(GetPool(), RES_FRM_SIZE,   RES_FRM_SIZE,
                                                            RES_SURROUND,   RES_SURROUND,
                                                            RES_ANCHOR,     RES_ANCHOR,
                                                            RES_VERT_ORIENT,RES_HORI_ORIENT,
                                                            0);
                            aShellSet.Put(*pOutSet);
                            aMgr.SetAttrSet(aShellSet);
                            if(SfxItemState::SET == pOutSet->GetItemState(FN_SET_FRM_NAME, false, &pItem))
                                rSh.SetFlyName(static_cast<const SfxStringItem*>(pItem)->GetValue());
                        }
                        else
                            aMgr.SetAttrSet( *pOutSet );

                        const SwFrameFormat* pCurrFlyFormat = rSh.GetFlyFrameFormat();
                        if(SfxItemState::SET ==
                           pOutSet->GetItemState(FN_PARAM_CHAIN_PREVIOUS,
                                                 false, &pItem))
                        {
                            rSh.HideChainMarker();

                            OUString sPrevName =
                                static_cast<const SfxStringItem*>(pItem)->GetValue();
                            const SwFormatChain &rChain = pCurrFlyFormat->GetChain();
                            //needs cast - no non-const method available
                            SwFlyFrameFormat* pFlyFormat =
                                rChain.GetPrev();
                            if(pFlyFormat)
                            {
                                if (pFlyFormat->GetName() != sPrevName)
                                {
                                    rSh.Unchain(*pFlyFormat);
                                }
                                else
                                    sPrevName.clear();
                            }

                            if (!sPrevName.isEmpty())
                            {
                                //needs cast - no non-const method available
                                SwFrameFormat* pPrevFormat = const_cast<SwFrameFormat*>(
                                    lcl_GetFrameFormatByName(rSh, sPrevName));
                                SAL_WARN_IF(!pPrevFormat, "sw.ui", "No frame found!");
                                if(pPrevFormat)
                                {
                                    rSh.Chain(*pPrevFormat, *pCurrFlyFormat);
                                }
                            }
                            rSh.SetChainMarker();
                        }
                        if(SfxItemState::SET ==
                           pOutSet->GetItemState(FN_PARAM_CHAIN_NEXT, false,
                                                 &pItem))
                        {
                            rSh.HideChainMarker();
                            OUString sNextName =
                                static_cast<const SfxStringItem*>(pItem)->GetValue();
                            const SwFormatChain &rChain = pCurrFlyFormat->GetChain();
                            //needs cast - no non-const method available
                            SwFlyFrameFormat* pFlyFormat =
                                rChain.GetNext();
                            if(pFlyFormat)
                            {
                                if (pFlyFormat->GetName() != sNextName)
                                {
                                    rSh.Unchain(*const_cast<SwFlyFrameFormat*>(static_cast<const SwFlyFrameFormat*>( pCurrFlyFormat)));
                                }
                                else
                                    sNextName.clear();
                            }

                            if (!sNextName.isEmpty())
                            {
                                //needs cast - no non-const method available
                                SwFrameFormat* pNextFormat = const_cast<SwFrameFormat*>(
                                    lcl_GetFrameFormatByName(rSh, sNextName));
                                SAL_WARN_IF(!pNextFormat, "sw.ui", "No frame found!");
                                if(pNextFormat)
                                {
                                    rSh.Chain(*const_cast<SwFrameFormat*>(
                                              pCurrFlyFormat), *pNextFormat);
                                }
                            }
                            rSh.SetChainMarker();
                        }
                    }
                }
                else
                    bUpdateMgr = false;
            }
        }
        break;
        case FN_FRAME_MIRROR_ON_EVEN_PAGES:
        {
            SwFormatHoriOrient aHori(aMgr.GetHoriOrient());
            bool bMirror = !aHori.IsPosToggle();
            aHori.SetPosToggle(bMirror);
            SfxItemSet aSet(GetPool(), RES_HORI_ORIENT, RES_HORI_ORIENT);
            aSet.Put(aHori);
            aMgr.SetAttrSet(aSet);
            bCopyToFormat = true;
            rReq.SetReturnValue(SfxBoolItem(nSlot, bMirror));
        }
        break;
        case FN_NAME_SHAPE:
        {
            bUpdateMgr = false;
            SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();
            if ( pSdrView &&
                 pSdrView->GetMarkedObjectCount() == 1 )
            {
                OUString aName(rSh.GetFlyName());
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                assert(pFact);
                ScopedVclPtr<AbstractSvxObjectNameDialog> pDlg(
                    pFact->CreateSvxObjectNameDialog( aName ));

                assert(pDlg);

                if ( pDlg->Execute() == RET_OK )
                {
                    pDlg->GetName(aName);
                    rSh.SetFlyName(aName);
                }
            }
        }
        break;
        // #i73249#
        case FN_TITLE_DESCRIPTION_SHAPE:
        {
            bUpdateMgr = false;
            SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();
            if ( pSdrView &&
                 pSdrView->GetMarkedObjectCount() == 1 )
            {
                OUString aDescription(rSh.GetObjDescription());
                OUString aTitle(rSh.GetObjTitle());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                assert(pFact);
                ScopedVclPtr<AbstractSvxObjectTitleDescDialog> pDlg(
                    pFact->CreateSvxObjectTitleDescDialog( aTitle,
                                                           aDescription ));
                assert(pDlg);

                if ( pDlg->Execute() == RET_OK )
                {
                    pDlg->GetDescription(aDescription);
                    pDlg->GetTitle(aTitle);

                    rSh.SetObjDescription(aDescription);
                    rSh.SetObjTitle(aTitle);
                }
            }
        }
        break;
        default:
            assert(!"wrong dispatcher");
            return;
    }
    if ( bUpdateMgr )
    {
        SwFrameFormat* pFormat = rSh.GetSelectedFrameFormat();
        if ( bCopyToFormat && pFormat && pFormat->IsAutoUpdateFormat() )
        {
            rSh.AutoUpdateFrame(pFormat, aMgr.GetAttrSet());
        }
        else
        {
            aMgr.UpdateFlyFrame();
        }
    }

}

void SwFrameShell::GetState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    bool bHtmlMode = 0 != ::GetHtmlMode(rSh.GetView().GetDocShell());
    if (rSh.IsFrameSelected())
    {
        SfxItemSet aSet( rSh.GetAttrPool(),
                            RES_LR_SPACE, RES_UL_SPACE,
                            RES_PROTECT, RES_HORI_ORIENT,
                            RES_OPAQUE, RES_OPAQUE,
                            RES_PRINT, RES_OPAQUE,
                            0 );
        rSh.GetFlyFrameAttr( aSet );

        bool bProtect = rSh.IsSelObjProtected(FlyProtectFlags::Pos) != FlyProtectFlags::NONE;
        bool bParentCntProt = rSh.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent ) != FlyProtectFlags::NONE;

        bProtect |= bParentCntProt;

        const FrameTypeFlags eFrameType = rSh.GetFrameType(nullptr,true);
        SwFlyFrameAttrMgr aMgr( false, &rSh, Frmmgr_Type::NONE );

        SfxWhichIter aIter( rSet );
        sal_uInt16 nWhich = aIter.FirstWhich();
        while ( nWhich )
        {
            switch ( nWhich )
            {
                case RES_FRM_SIZE:
                {
                    SwFormatFrameSize aSz(aMgr.GetFrameSize());
                    rSet.Put(aSz);
                }
                break;
                case RES_VERT_ORIENT:
                case RES_HORI_ORIENT:
                case SID_ATTR_ULSPACE:
                case SID_ATTR_LRSPACE:
                case RES_LR_SPACE:
                case RES_UL_SPACE:
                case RES_PROTECT:
                case RES_OPAQUE:
                case RES_PRINT:
                case RES_SURROUND:
                {
                    rSet.Put(aSet.Get(GetPool().GetWhich(nWhich)));
                }
                break;
                case SID_OBJECT_ALIGN_LEFT   :
                case SID_OBJECT_ALIGN_CENTER :
                case SID_OBJECT_ALIGN_RIGHT  :
                case FN_FRAME_ALIGN_HORZ_CENTER:
                case FN_FRAME_ALIGN_HORZ_RIGHT:
                case FN_FRAME_ALIGN_HORZ_LEFT:
                    if ( (eFrameType & FrameTypeFlags::FLY_INCNT) ||
                         bProtect ||
                         ((nWhich == FN_FRAME_ALIGN_HORZ_CENTER  || nWhich == SID_OBJECT_ALIGN_CENTER) &&
                          bHtmlMode ))
                        rSet.DisableItem( nWhich );
                break;
                case FN_FRAME_ALIGN_VERT_ROW_TOP:
                case FN_FRAME_ALIGN_VERT_ROW_CENTER:
                case FN_FRAME_ALIGN_VERT_ROW_BOTTOM:
                case FN_FRAME_ALIGN_VERT_CHAR_TOP:
                case FN_FRAME_ALIGN_VERT_CHAR_CENTER:
                case FN_FRAME_ALIGN_VERT_CHAR_BOTTOM:
                    if ( !(eFrameType & FrameTypeFlags::FLY_INCNT) || bProtect
                         || (bHtmlMode && FN_FRAME_ALIGN_VERT_CHAR_BOTTOM == nWhich) )
                        rSet.DisableItem( nWhich );
                break;

                case SID_OBJECT_ALIGN_UP     :
                case SID_OBJECT_ALIGN_MIDDLE :
                case SID_OBJECT_ALIGN_DOWN :

                case FN_FRAME_ALIGN_VERT_TOP:
                case FN_FRAME_ALIGN_VERT_CENTER:
                case FN_FRAME_ALIGN_VERT_BOTTOM:
                    if ( bProtect || (bHtmlMode && eFrameType & FrameTypeFlags::FLY_ATCNT))
                        rSet.DisableItem( nWhich );
                    else
                    {
                        // These slots need different labels depending on whether they are anchored in a character
                        // or on a paragraph/page etc.
                        OUString sNewLabel;
                        if (eFrameType & FrameTypeFlags::FLY_INCNT)
                        {
                            switch (nWhich)
                            {
                                case SID_OBJECT_ALIGN_UP     :
                                case FN_FRAME_ALIGN_VERT_TOP:
                                    sNewLabel = SW_RES(STR_TOP_BASE);
                                    break;
                                case SID_OBJECT_ALIGN_MIDDLE :
                                case FN_FRAME_ALIGN_VERT_CENTER:
                                    sNewLabel = SW_RES(STR_CENTER_BASE);
                                    break;
                                case SID_OBJECT_ALIGN_DOWN :
                                case FN_FRAME_ALIGN_VERT_BOTTOM:
                                    if(!bHtmlMode)
                                        sNewLabel = SW_RES(STR_BOTTOM_BASE);
                                    else
                                        rSet.DisableItem( nWhich );
                                break;
                            }
                        }
                        else
                        {
                            if (nWhich != FN_FRAME_ALIGN_VERT_TOP &&
                                    nWhich != SID_OBJECT_ALIGN_UP )
                            {
                                if (aMgr.GetAnchor() == FLY_AT_FLY)
                                {
                                    const SwFrameFormat* pFormat = rSh.IsFlyInFly();
                                    if (pFormat)
                                    {
                                        const SwFormatFrameSize& rFrameSz = pFormat->GetFrameSize();
                                        if (rFrameSz.GetHeightSizeType() != ATT_FIX_SIZE)
                                        {
                                            rSet.DisableItem( nWhich );
                                            break;
                                        }
                                    }
                                }
                            }
                            switch (nWhich)
                            {
                                case SID_OBJECT_ALIGN_UP :
                                case FN_FRAME_ALIGN_VERT_TOP:
                                    sNewLabel = vcl::CommandInfoProvider::GetLabelForCommand(".uno:AlignTop", GetFrame()->GetFrame().GetFrameInterface());
                                    break;
                                case SID_OBJECT_ALIGN_MIDDLE:
                                case FN_FRAME_ALIGN_VERT_CENTER:
                                    sNewLabel = vcl::CommandInfoProvider::GetLabelForCommand(".uno:AlignVerticalCenter", GetFrame()->GetFrame().GetFrameInterface());
                                    break;
                                case SID_OBJECT_ALIGN_DOWN:
                                case FN_FRAME_ALIGN_VERT_BOTTOM:
                                    sNewLabel = vcl::CommandInfoProvider::GetLabelForCommand(".uno:AlignBottom", GetFrame()->GetFrame().GetFrameInterface());
                                    break;
                            }
                        }
                        if ( !sNewLabel.isEmpty() )
                            rSet.Put( SfxStringItem( nWhich, sNewLabel ));
                    }
                break;
                case SID_HYPERLINK_GETLINK:
                {
                    SvxHyperlinkItem aHLinkItem;
                    const SfxPoolItem* pItem;

                    SfxItemSet aURLSet(GetPool(), RES_URL, RES_URL);
                    rSh.GetFlyFrameAttr( aURLSet );

                    if(SfxItemState::SET == aURLSet.GetItemState(RES_URL, true, &pItem))
                    {
                        const SwFormatURL* pFormatURL = static_cast<const SwFormatURL*>(pItem);
                        aHLinkItem.SetURL(pFormatURL->GetURL());
                        aHLinkItem.SetTargetFrame(pFormatURL->GetTargetFrameName());
                        aHLinkItem.SetName(rSh.GetFlyName());
                    }

                    aHLinkItem.SetInsertMode((SvxLinkInsertMode)(aHLinkItem.GetInsertMode() |
                        (bHtmlMode ? HLINK_HTMLMODE : 0)));

                    rSet.Put(aHLinkItem);
                }
                break;

                case FN_FRAME_CHAIN:
                {
                    const int nSel = rSh.GetSelectionType();
                    if (nSel & nsSelectionType::SEL_GRF || nSel & nsSelectionType::SEL_OLE)
                        rSet.DisableItem( FN_FRAME_CHAIN );
                    else
                    {
                        const SwFrameFormat *pFormat = rSh.GetFlyFrameFormat();
                        if ( bParentCntProt || rSh.GetView().GetEditWin().GetApplyTemplate() ||
                             !pFormat || pFormat->GetChain().GetNext() )
                        {
                            rSet.DisableItem( FN_FRAME_CHAIN );
                        }
                        else
                        {
                            bool bChainMode = rSh.GetView().GetEditWin().IsChainMode();
                            rSet.Put( SfxBoolItem( FN_FRAME_CHAIN, bChainMode ) );
                        }
                    }
                }
                break;
                case FN_FRAME_UNCHAIN:
                {
                    const int nSel = rSh.GetSelectionType();
                    if (nSel & nsSelectionType::SEL_GRF || nSel & nsSelectionType::SEL_OLE)
                        rSet.DisableItem( FN_FRAME_UNCHAIN );
                    else
                    {
                        const SwFrameFormat *pFormat = rSh.GetFlyFrameFormat();
                        if ( bParentCntProt || rSh.GetView().GetEditWin().GetApplyTemplate() ||
                             !pFormat || !pFormat->GetChain().GetNext() )
                        {
                            rSet.DisableItem( FN_FRAME_UNCHAIN );
                        }
                    }
                }
                break;
                case SID_FRAME_TO_TOP:
                case SID_FRAME_TO_BOTTOM:
                case FN_FRAME_UP:
                case FN_FRAME_DOWN:
                    if ( bParentCntProt )
                        rSet.DisableItem( nWhich );
                break;

                case SID_ATTR_TRANSFORM:
                {
                    rSet.DisableItem( nWhich );
                }
                break;

                case SID_ATTR_TRANSFORM_PROTECT_SIZE:
                {
                    const FlyProtectFlags eProtection = rSh.IsSelObjProtected( FlyProtectFlags::Size );
                    if ( ( eProtection & FlyProtectFlags::Content ) ||
                         ( eProtection & FlyProtectFlags::Size ) )
                    {
                        rSet.Put( SfxBoolItem( SID_ATTR_TRANSFORM_PROTECT_SIZE, true ) );
                    }
                    else
                    {
                        rSet.Put( SfxBoolItem( SID_ATTR_TRANSFORM_PROTECT_SIZE, false ) );
                    }
                }
                break;

                case SID_ATTR_TRANSFORM_WIDTH:
                {
                    rSet.Put( SfxUInt32Item( SID_ATTR_TRANSFORM_WIDTH, aMgr.GetSize().getWidth() ) );
                }
                break;

                case SID_ATTR_TRANSFORM_HEIGHT:
                {
                    rSet.Put( SfxUInt32Item( SID_ATTR_TRANSFORM_HEIGHT, aMgr.GetSize().getHeight() ) );
                }
                break;

                case FN_FORMAT_FRAME_DLG:
                {
                    const int nSel = rSh.GetSelectionType();
                    if ( bParentCntProt || nSel & nsSelectionType::SEL_GRF)
                        rSet.DisableItem( nWhich );
                }
                break;
                // #i73249#
                case FN_TITLE_DESCRIPTION_SHAPE:
                case FN_NAME_SHAPE:
                {
                    SwWrtShell &rWrtSh = GetShell();
                    SdrView* pSdrView = rWrtSh.GetDrawViewWithValidMarkList();
                    if ( !pSdrView ||
                         pSdrView->GetMarkedObjectCount() != 1 )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

                default:
                    /* do nothing */;
                    break;
            }
            nWhich = aIter.NextWhich();
        }
    }
}

SwFrameShell::SwFrameShell(SwView &_rView) :
    SwBaseShell( _rView )
{
    SetName("Frame");
    SetHelpId(SW_FRAMESHELL);

    // #96392# Use this to announce it is the frame shell who creates the selection.
    SwTransferable::CreateSelection( _rView.GetWrtShell(), this );

    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Frame));
}

SwFrameShell::~SwFrameShell()
{
    // #96392# Only clear the selection if it was this frame shell who created it.
    SwTransferable::ClearSelection( GetShell(), this );
}

void SwFrameShell::ExecFrameStyle(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    bool bDefault = false;
    if (!rSh.IsFrameSelected())
        return;
    // At first pick the default BoxItem out of the pool.
    // If unequal to regular box item, then it has already
    // been changed (New one is no default).
    const SvxBoxItem* pPoolBoxItem = static_cast<const SvxBoxItem*>(::GetDfltAttr(RES_BOX));

    const SfxItemSet *pArgs = rReq.GetArgs();
    SfxItemSet aFrameSet(rSh.GetAttrPool(), RES_BOX, RES_BOX);

    rSh.GetFlyFrameAttr( aFrameSet );
    const SvxBoxItem& rBoxItem = static_cast<const SvxBoxItem&>(aFrameSet.Get(RES_BOX));

    if (pPoolBoxItem == &rBoxItem)
        bDefault = true;

    SvxBoxItem aBoxItem(rBoxItem);

    SvxBorderLine aBorderLine;
    const SfxPoolItem *pItem = nullptr;

    if(pArgs)    // Any controller can sometimes deliver nothing #48169#
    {
        switch (rReq.GetSlot())
        {
            case SID_ATTR_BORDER:
            {
                if (pArgs->GetItemState(RES_BOX, true, &pItem) == SfxItemState::SET)
                {
                    SvxBoxItem aNewBox(*static_cast<const SvxBoxItem *>(pItem));
                    const SvxBorderLine* pBorderLine;

                    if ((pBorderLine = aBoxItem.GetTop()) != nullptr)
                        lcl_FrameGetMaxLineWidth(pBorderLine, aBorderLine);
                    if ((pBorderLine = aBoxItem.GetBottom()) != nullptr)
                        lcl_FrameGetMaxLineWidth(pBorderLine, aBorderLine);
                    if ((pBorderLine = aBoxItem.GetLeft()) != nullptr)
                        lcl_FrameGetMaxLineWidth(pBorderLine, aBorderLine);
                    if ((pBorderLine = aBoxItem.GetRight()) != nullptr)
                        lcl_FrameGetMaxLineWidth(pBorderLine, aBorderLine);

                    if(aBorderLine.GetOutWidth() == 0)
                    {
                        aBorderLine.SetBorderLineStyle(
                                table::BorderLineStyle::SOLID);
                        aBorderLine.SetWidth( DEF_LINE_WIDTH_0 );
                    }
                    //Set distance only if the request is received from the controller.

#if HAVE_FEATURE_SCRIPTING
                    if(!StarBASIC::IsRunning())
#endif
                    {
                        // TODO: should this copy 4 individual Dist instead?
                        aNewBox.SetAllDistances(rBoxItem.GetSmallestDistance());
                    }

                    aBoxItem = aNewBox;
                    SvxBorderLine aDestBorderLine;

                    if( aBoxItem.GetTop() != nullptr )
                        aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::TOP);
                    if( aBoxItem.GetBottom() != nullptr )
                        aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::BOTTOM);
                    if( aBoxItem.GetLeft() != nullptr )
                        aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::LEFT);
                    if( aBoxItem.GetRight() != nullptr )
                        aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::RIGHT);
                }
            }
            break;

            case SID_FRAME_LINESTYLE:
            {
                if (pArgs->GetItemState(SID_FRAME_LINESTYLE, false, &pItem) == SfxItemState::SET)
                {
                    const SvxLineItem* pLineItem =
                            static_cast<const SvxLineItem*>(pItem);

                    if ( pLineItem->GetLine() )
                    {
                        aBorderLine = *(pLineItem->GetLine());

                        if (!aBoxItem.GetTop() && !aBoxItem.GetBottom() &&
                            !aBoxItem.GetLeft() && !aBoxItem.GetRight())
                        {
                            aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::TOP);
                            aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::BOTTOM);
                            aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::LEFT);
                            aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::RIGHT);
                        }
                        else
                        {
                            if( aBoxItem.GetTop() )
                            {
                                aBorderLine.SetColor( aBoxItem.GetTop()->GetColor() );
                                aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::TOP);
                            }
                            if( aBoxItem.GetBottom() )
                            {
                                aBorderLine.SetColor( aBoxItem.GetBottom()->GetColor());
                                aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::BOTTOM);
                            }
                            if( aBoxItem.GetLeft() )
                            {
                                aBorderLine.SetColor( aBoxItem.GetLeft()->GetColor());
                                aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::LEFT);
                            }
                            if( aBoxItem.GetRight() )
                            {
                                aBorderLine.SetColor(aBoxItem.GetRight()->GetColor());
                                aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::RIGHT);
                            }
                        }
                    }
                    else
                    {
                        aBoxItem.SetLine(nullptr, SvxBoxItemLine::TOP);
                        aBoxItem.SetLine(nullptr, SvxBoxItemLine::BOTTOM);
                        aBoxItem.SetLine(nullptr, SvxBoxItemLine::LEFT);
                        aBoxItem.SetLine(nullptr, SvxBoxItemLine::RIGHT);
                    }
                }
            }
            break;

            case SID_FRAME_LINECOLOR:
            {
                if (pArgs->GetItemState(SID_FRAME_LINECOLOR, false, &pItem) == SfxItemState::SET)
                {
                    const Color& rNewColor = static_cast<const SvxColorItem*>(pItem)->GetValue();

                    if (!aBoxItem.GetTop() && !aBoxItem.GetBottom() &&
                        !aBoxItem.GetLeft() && !aBoxItem.GetRight())
                    {
                        aBorderLine.SetColor( rNewColor );
                        aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::TOP);
                        aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::BOTTOM);
                        aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::LEFT);
                        aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::RIGHT);
                    }
                    else
                    {
                        if ( aBoxItem.GetTop() )
                            const_cast<SvxBorderLine*>(aBoxItem.GetTop())->SetColor( rNewColor );
                        if ( aBoxItem.GetBottom() )
                            const_cast<SvxBorderLine*>(aBoxItem.GetBottom())->SetColor( rNewColor );
                        if ( aBoxItem.GetLeft() )
                            const_cast<SvxBorderLine*>(aBoxItem.GetLeft())->SetColor( rNewColor );
                        if ( aBoxItem.GetRight() )
                            const_cast<SvxBorderLine*>(aBoxItem.GetRight())->SetColor( rNewColor );
                    }
                }
            }
            break;
        }
    }
    if (bDefault && (aBoxItem.GetTop() || aBoxItem.GetBottom() ||
        aBoxItem.GetLeft() || aBoxItem.GetRight()))
    {
        aBoxItem.SetAllDistances(MIN_BORDER_DIST);
    }
    aFrameSet.Put( aBoxItem );
    // Template AutoUpdate
    SwFrameFormat* pFormat = rSh.GetSelectedFrameFormat();
    if(pFormat && pFormat->IsAutoUpdateFormat())
    {
        rSh.AutoUpdateFrame(pFormat, aFrameSet);
    }
    else
        rSh.SetFlyFrameAttr( aFrameSet );

}

static void lcl_FrameGetMaxLineWidth(const SvxBorderLine* pBorderLine, SvxBorderLine& rBorderLine)
{
    if(pBorderLine->GetWidth() > rBorderLine.GetWidth())
        rBorderLine.SetWidth(pBorderLine->GetWidth());

    rBorderLine.SetBorderLineStyle(pBorderLine->GetBorderLineStyle());
    rBorderLine.SetColor(pBorderLine->GetColor());
}

void SwFrameShell::GetLineStyleState(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    bool bParentCntProt = rSh.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent ) != FlyProtectFlags::NONE;

    if (bParentCntProt)
    {
        if (rSh.IsFrameSelected())
            rSet.DisableItem( SID_FRAME_LINECOLOR );

        rSet.DisableItem( SID_ATTR_BORDER );
        rSet.DisableItem( SID_FRAME_LINESTYLE );
    }
    else
    {
        if (rSh.IsFrameSelected())
        {
            SfxItemSet aFrameSet( rSh.GetAttrPool(), RES_BOX, RES_BOX );

            rSh.GetFlyFrameAttr(aFrameSet);

            const SvxBorderLine* pLine = static_cast<const SvxBoxItem&>(aFrameSet.Get(RES_BOX)).GetTop();
            rSet.Put(SvxColorItem(pLine ? pLine->GetColor() : Color(), SID_FRAME_LINECOLOR));
        }
    }
}

void  SwFrameShell::StateInsert(SfxItemSet &rSet)
{
    const int nSel = GetShell().GetSelectionType();
    if ( (nSel & nsSelectionType::SEL_GRF)
        || (nSel & nsSelectionType::SEL_OLE) )
    {
        rSet.DisableItem(FN_INSERT_FRAME);
    }
    else if ( GetShell().CursorInsideInputField() )
    {
        rSet.DisableItem(FN_INSERT_FRAME);
    }
}

//UUUU
void SwFrameShell::GetDrawAttrStateTextFrame(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();

    if(rSh.IsFrameSelected())
    {
        rSh.GetFlyFrameAttr(rSet);
    }
    else
    {
        SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();

        if(pSdrView)
        {
            rSet.Put(pSdrView->GetDefaultAttr());
        }
    }
}

//UUUU
void SwFrameShell::ExecDrawAttrArgsTextFrame(SfxRequest& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    SwWrtShell& rSh = GetShell();

    if(pArgs)
    {
        if(rSh.IsFrameSelected())
        {
            rSh.SetFlyFrameAttr(const_cast< SfxItemSet& >(*pArgs));
        }
        else
        {
            SdrView* pSdrView = rSh.GetDrawViewWithValidMarkList();

            if(pSdrView)
            {
                pSdrView->SetDefaultAttr(*pArgs, false);
            }
        }
    }
    else
    {
        SfxDispatcher* pDis = rSh.GetView().GetViewFrame()->GetDispatcher();

        switch(rReq.GetSlot())
        {
            case SID_ATTR_FILL_STYLE:
            case SID_ATTR_FILL_COLOR:
            case SID_ATTR_FILL_GRADIENT:
            case SID_ATTR_FILL_HATCH:
            case SID_ATTR_FILL_BITMAP:
            case SID_ATTR_FILL_TRANSPARENCE:
            case SID_ATTR_FILL_FLOATTRANSPARENCE:
            {
                pDis->Execute(SID_ATTRIBUTES_AREA);
                break;
            }
        }
    }
}

//UUUU
void SwFrameShell::ExecDrawDlgTextFrame(SfxRequest& rReq)
{
    switch(rReq.GetSlot())
    {
        case SID_ATTRIBUTES_AREA:
        {
            SwWrtShell& rSh = GetShell();

            if(rSh.IsFrameSelected())
            {
                SdrView* pView = rSh.GetDrawView();
                SdrModel* pDoc = pView->GetModel();
                SfxItemSet aNewAttr(pDoc->GetItemPool());

                // get attributes from FlyFrame
                rSh.GetFlyFrameAttr(aNewAttr);

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                assert(pFact);
                ScopedVclPtr<AbstractSvxAreaTabDialog> pDlg(pFact->CreateSvxAreaTabDialog(
                    nullptr,
                    &aNewAttr,
                    pDoc,
                    false));
                assert(pDlg);

                if(RET_OK == pDlg->Execute())
                {
                    // set attributes at FlyFrame
                    rSh.SetFlyFrameAttr(const_cast< SfxItemSet& >(*pDlg->GetOutputItemSet()));

                    static sal_uInt16 aInval[] =
                    {
                        SID_ATTR_FILL_STYLE,
                        SID_ATTR_FILL_COLOR,
                        SID_ATTR_FILL_TRANSPARENCE,
                        SID_ATTR_FILL_FLOATTRANSPARENCE,
                        0
                    };

                    SfxBindings &rBnd = GetView().GetViewFrame()->GetBindings();

                    rBnd.Invalidate(aInval);
                    rBnd.Update(SID_ATTR_FILL_STYLE);
                    rBnd.Update(SID_ATTR_FILL_COLOR);
                    rBnd.Update(SID_ATTR_FILL_TRANSPARENCE);
                    rBnd.Update(SID_ATTR_FILL_FLOATTRANSPARENCE);
                }
            }

            break;
        }
    }
}

//UUUU
void SwFrameShell::DisableStateTextFrame(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich(aIter.FirstWhich());

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_ATTRIBUTES_AREA:
            {
                SwWrtShell& rSh = GetShell();

                if(!rSh.IsFrameSelected())
                {
                    rSet.DisableItem(nWhich);
                }

                break;
            }
            default:
            {
                rSet.DisableItem(nWhich);
                break;
            }
        }

        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
