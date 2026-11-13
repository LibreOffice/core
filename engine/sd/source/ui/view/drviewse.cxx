/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/presentation/XPresentation.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <i18nutil/unicode.hxx>
#include <i18nutil/transliteration.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cpo/uno/Any.hxx>

#include <COKit/COKit.hxx>
#include <comphelper/hash.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <editeng/editstat.hxx>
#include <editeng/outlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svl/urlbmk.hxx>
#include <svx/clipfmtitem.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdoedge.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdotext.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtaitm.hxx>
#include <svl/itemset.hxx>
#include <svx/svdoashp.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/poolitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/eeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>
#include <editeng/flditem.hxx>
#include <svx/obj3d.hxx>
#include <svx/svdobjkind.hxx>
#include <svx/svdouno.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/ipclient.hxx>
#include <avmedia/mediawindow.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/docfile.hxx>
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <svl/cryptosign.hxx>
#include <tools/debug.hxx>
#include <tools/UniqueID.hxx>
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>
#include <svx/svdoashp.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/unit_conversion.hxx>

#include <DrawViewShell.hxx>
#include <slideshow.hxx>
#include <ViewShellHint.hxx>
#include <framework/FrameworkHelper.hxx>
#include <app.hrc>
#include <strings.hrc>

#include <drawdoc.hxx>
#include <fusel.hxx>
#include <futext.hxx>
#include <fuconrec.hxx>
#include <fuconcs.hxx>
#include <fuconuno.hxx>
#include <fuconbez.hxx>
#include <fuediglu.hxx>
#include <fuconarc.hxx>
#include <fucon3d.hxx>
#include <sdresid.hxx>
#include <unokywds.hxx>
#include <Outliner.hxx>
#include <sdpage.hxx>
#include <FrameView.hxx>
#include <zoomlist.hxx>
#include <drawview.hxx>
#include <DrawDocShell.hxx>
#include <ViewShellBase.hxx>
#include <ToolBarManager.hxx>
#include <anminfo.hxx>
#include <optsitem.hxx>
#include <Window.hxx>
#include <fuformatpaintbrush.hxx>
#include <fuzoom.hxx>
#include <sdmod.hxx>
#include <basegfx/utils/zoomtools.hxx>
#include <officecfg/Office/Draw.hxx>
#include <officecfg/Office/Impress.hxx>
#include <sfx2/kit/helper.hxx>
#include <SlideSorter.hxx>
#include <SlideSorterViewShell.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsClipboard.hxx>
#include <svl/srchitem.hxx>
#include <svx/srchdlg.hxx>

#include <algorithm>
#include <array>
#include <functional>
#include <libxml/xmlwriter.h>
#include <rtl/strbuf.hxx>
#include <optional>

using namespace ::com::sun::star;
using namespace ::cpo;
using namespace ::cpo::uno;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::beans;

namespace sd {

// Permanent Functions

// the rectangle a shape is to be created in, given with the request in twips on the slide; the
// slide's own area does not limit it, a shape may lie beside the slide
static std::optional<::tools::Rectangle> lcl_getRequestedRectangle(SfxRequest const& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    if (!pArgs)
        return std::nullopt;

    const SfxInt32Item* pXItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_2, false);
    const SfxInt32Item* pYItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_3, false);
    const SfxInt32Item* pWidthItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_4, false);
    const SfxInt32Item* pHeightItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_5, false);
    if (!pXItem || !pYItem || !pWidthItem || !pHeightItem)
        return std::nullopt;

    constexpr sal_Int32 nMostTwips = 10000000;
    const sal_Int32 nX(pXItem->GetValue());
    const sal_Int32 nY(pYItem->GetValue());
    const sal_Int32 nWidth(pWidthItem->GetValue());
    const sal_Int32 nHeight(pHeightItem->GetValue());
    if (nWidth <= 0 || nHeight <= 0 || std::abs(nX) > nMostTwips || std::abs(nY) > nMostTwips
        || nWidth > nMostTwips || nHeight > nMostTwips)
        return std::nullopt;

    auto toMm100 = [](sal_Int32 nTwips) {
        return static_cast<::tools::Long>(
            o3tl::convert(nTwips, o3tl::Length::twip, o3tl::Length::mm100));
    };
    return ::tools::Rectangle(Point(toMm100(nX), toMm100(nY)),
                              Size(toMm100(nWidth), toMm100(nHeight)));
}

// the two points a line or a connector is to be created between, given with the request in twips
// on the slide; FN_PARAM_4 is left out, as the rectangle function reads it as IsSticky for every
// slot
static std::optional<std::pair<Point, Point>> lcl_getRequestedLine(SfxRequest const& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    if (!pArgs)
        return std::nullopt;

    const SfxInt32Item* pStartXItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_2, false);
    const SfxInt32Item* pStartYItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_3, false);
    const SfxInt32Item* pEndXItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_5, false);
    const SfxInt32Item* pEndYItem = pArgs->GetItem<SfxInt32Item>(FN_PARAM_6, false);
    if (!pStartXItem || !pStartYItem || !pEndXItem || !pEndYItem)
        return std::nullopt;

    constexpr sal_Int32 nMostTwips = 10000000;
    const sal_Int32 aCoordinates[] = { pStartXItem->GetValue(), pStartYItem->GetValue(),
                                       pEndXItem->GetValue(), pEndYItem->GetValue() };
    for (sal_Int32 nCoordinate : aCoordinates)
        if (std::abs(nCoordinate) > nMostTwips)
            return std::nullopt;

    auto toMm100 = [](sal_Int32 nTwips) {
        return static_cast<::tools::Long>(
            o3tl::convert(nTwips, o3tl::Length::twip, o3tl::Length::mm100));
    };
    const Point aStart(toMm100(aCoordinates[0]), toMm100(aCoordinates[1]));
    const Point aEnd(toMm100(aCoordinates[2]), toMm100(aCoordinates[3]));
    if (aStart == aEnd)
        return std::nullopt;
    return std::make_pair(aStart, aEnd);
}

// what the ends of a connector are to be tied to, given with the request as four numbers: the id of
// the object at the start and the index of its glue point, then the same for the end; an object id
// of 0 leaves that end free, and the index counts as the glue point properties of a connector do
static std::optional<std::array<sal_Int64, 4>> lcl_getRequestedTies(SfxRequest const& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    const auto* pTies
        = pArgs ? dynamic_cast<const SfxStringItem*>(pArgs->GetItem(FN_PARAM_1, false)) : nullptr;
    if (!pTies)
        return std::nullopt;

    std::array<sal_Int64, 4> aTies;
    sal_Int32 nAt = 0;
    for (sal_Int64& rTie : aTies)
    {
        const std::u16string_view aNumber(o3tl::getToken(pTies->GetValue(), 0, ' ', nAt));
        if (aNumber.empty())
            return std::nullopt;
        rTie = o3tl::toInt64(aNumber);
    }
    return aTies;
}

static void ImpAddPrintableCharactersToTextEdit(SfxRequest const & rReq, ::sd::View* pView)
{
    // evtl. feed characters to activated textedit
    const SfxItemSet* pSet = rReq.GetArgs();

    if(!pSet)
        return;

    OUString aInputString;

    if(SfxItemState::SET == pSet->GetItemState(SID_ATTR_CHAR))
        aInputString = pSet->Get(SID_ATTR_CHAR).GetValue();

    if(aInputString.isEmpty())
        return;

    OutlinerView* pOLV = pView->GetTextEditOutlinerView();

    if(pOLV)
    {
        for(sal_Int32 a(0); a < aInputString.getLength(); a++)
        {
            vcl::KeyCode aKeyCode;
            // tdf#38669 - create the key event using a Unicode character
            KeyEvent aKeyEvent(aInputString[a], aKeyCode);

            // add actual character
            pOLV->PostKeyEvent(aKeyEvent);
        }
    }
}

void DrawViewShell::FuPermanent(SfxRequest& rReq)
{
    sal_uInt16 nSId = rReq.GetSlot();
    // a text the client dragged out or clicked is a new one, never one for the text tool on
    const bool bTextInRectangle = (nSId == SID_ATTR_CHAR || nSId == SID_ATTR_CHAR_VERTICAL)
                                  && lcl_getRequestedRectangle(rReq).has_value();

    if( HasCurrentFunction() && !bTextInRectangle &&
        ( nSId == SID_TEXTEDIT || nSId == SID_ATTR_CHAR || nSId == SID_TEXT_FITTOSIZE ||
          nSId == SID_ATTR_CHAR_VERTICAL || nSId == SID_TEXT_FITTOSIZE_VERTICAL ) )
    {
        rtl::Reference<FuPoor> xFunc( GetCurrentFunction() );

        FuText* pFuText = dynamic_cast< FuText* >( xFunc.get() );

        if( pFuText )
        {
            pFuText->SetPermanent(true);
            xFunc->ReceiveRequest( rReq );

            Invalidate();

            // evtl. feed characters to activated textedit
            if(SID_ATTR_CHAR == nSId && GetView() && GetView()->IsTextEdit())
                ImpAddPrintableCharactersToTextEdit(rReq, GetView());

            rReq.Done();
            return;
        }
    }

    CheckLineTo (rReq);
    sal_uInt16 nOldSId = 0;
    bool bPermanent = false;

    if( !mpDrawView )
        return;

    if(HasCurrentFunction())
    {
        if( (nSId == SID_FORMATPAINTBRUSH) && (GetCurrentFunction()->GetSlotID() == SID_TEXTEDIT) )
        {
            // save text edit mode for format paintbrush!
            SetOldFunction( GetCurrentFunction() );
        }
        else
        {
            if(GetOldFunction() == GetCurrentFunction())
            {
                SetOldFunction(nullptr);
            }
        }

        if ( (bTextInRectangle ||
              (nSId != SID_TEXTEDIT && nSId != SID_ATTR_CHAR && nSId != SID_TEXT_FITTOSIZE &&
               nSId != SID_ATTR_CHAR_VERTICAL && nSId != SID_TEXT_FITTOSIZE_VERTICAL &&
               nSId != SID_FORMATPAINTBRUSH)) &&
             mpDrawView->IsTextEdit() )
        {
            mpDrawView->SdrEndTextEdit();
        }

        if( HasCurrentFunction() )
        {
            nOldSId = GetCurrentFunction()->GetSlotID();

            if (!bTextInRectangle && (nOldSId == nSId ||
                ((nOldSId == SID_TEXTEDIT || nOldSId == SID_ATTR_CHAR || nOldSId == SID_TEXT_FITTOSIZE ||
                nOldSId == SID_ATTR_CHAR_VERTICAL || nOldSId == SID_TEXT_FITTOSIZE_VERTICAL) &&
                (nSId == SID_TEXTEDIT || nSId == SID_ATTR_CHAR || nSId == SID_TEXT_FITTOSIZE ||
                nSId == SID_ATTR_CHAR_VERTICAL || nSId == SID_TEXT_FITTOSIZE_VERTICAL ))))
            {
                bPermanent = true;
            }

            GetCurrentFunction()->Deactivate();
        }

        SetCurrentFunction(nullptr);

        SfxBindings& rBind = GetViewFrame()->GetBindings();
        rBind.Invalidate(nOldSId);
        rBind.Update(nOldSId);
    }

    // for COKit - choosing a shape should construct it directly
    bool bCreateDirectly = false;
    bool bRectangle = false;
    // where the shape goes when the request says, instead of the middle of the slide
    std::optional<::tools::Rectangle> oRequestedRectangle;
    // where a line goes when the request says, from its start to its end
    std::optional<std::pair<Point, Point>> oRequestedLine;
    // the id the client gave the object it creates, one this view reserved for it; nought for
    // none, where the object takes the next id as any other
    sal_uInt64 nGivenId = 0;
    // all slots come here, and some have another kind of item at FN_PARAM_7
    if (const SfxItemSet* pIdArgs = rReq.GetArgs())
        if (const auto* pId
            = dynamic_cast<const SfxUInt32Item*>(pIdArgs->GetItem(FN_PARAM_7, false));
            pId && TakeReservedObjectId(pId->GetValue()))
            nGivenId = pId->GetValue();
    // whether a text made at once grows in width too
    bool bTextGrows = false;

    switch ( nSId )
    {
        case SID_TEXTEDIT:  // BASIC ???
        case SID_ATTR_CHAR:
        case SID_ATTR_CHAR_VERTICAL:
        case SID_TEXT_FITTOSIZE:
        case SID_TEXT_FITTOSIZE_VERTICAL:
        {
            SetCurrentFunction( FuText::Create(*this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq) );
            GetCurrentFunction()->DoExecute(rReq);

            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( SID_ATTR_CHAR );
            rBindings.Invalidate( SID_ATTR_CHAR_VERTICAL );
            rBindings.Invalidate( SID_TEXT_FITTOSIZE );
            rBindings.Invalidate( SID_TEXT_FITTOSIZE_VERTICAL );

            // evtl. feed characters to activated textedit
            if(SID_ATTR_CHAR == nSId && GetView() && GetView()->IsTextEdit())
                ImpAddPrintableCharactersToTextEdit(rReq, GetView());

            rReq.Done();

            const SfxItemSet* pArgs = rReq.GetArgs();
            if (pArgs && pArgs->HasItem(FN_PARAM_1))
                bCreateDirectly = static_cast<const SfxBoolItem&>(pArgs->Get(FN_PARAM_1)).GetValue();

            // a text dragged out in the client comes with its rectangle and is made at once; a
            // click gives a text that grows in width as well, as the office's text tool makes it
            oRequestedRectangle = lcl_getRequestedRectangle(rReq);
            if (oRequestedRectangle)
            {
                bCreateDirectly = true;
                if (const SfxBoolItem* pGrow = pArgs->GetItem<SfxBoolItem>(FN_PARAM_6, false))
                    bTextGrows = pGrow->GetValue();
            }
        }
        break;

        case SID_FM_CREATE_CONTROL:
        {
            SetCurrentFunction( FuConstructUnoControl::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq, bPermanent ) );
            rReq.Done();
        }
        break;

        case SID_FM_CREATE_FIELDCONTROL:
        {
            const SfxUnoAnyItem* pDescriptorItem = rReq.GetArg<SfxUnoAnyItem>(SID_FM_DATACCESS_DESCRIPTOR);
            DBG_ASSERT( pDescriptorItem, "DrawViewShell::FuPermanent(SID_FM_CREATE_FIELDCONTROL): invalid request args!" );

            if(pDescriptorItem)
            {
                // get the form view
                FmFormView* pFormView = mpDrawView.get();
                SdrPageView* pPageView = pFormView ? pFormView->GetSdrPageView() : nullptr;

                if(pPageView)
                {
                    svx::ODataAccessDescriptor aDescriptor(pDescriptorItem->GetValue());
                    rtl::Reference<SdrObject> pNewDBField = pFormView->CreateFieldControl(aDescriptor);

                    if(pNewDBField)
                    {
                        ::tools::Rectangle aVisArea = GetActiveWindow()->PixelToLogic(::tools::Rectangle(Point(0,0), GetActiveWindow()->GetOutputSizePixel()));
                        Point aObjPos(aVisArea.Center());
                        Size aObjSize(pNewDBField->GetLogicRect().GetSize());
                        aObjPos.AdjustX( -(aObjSize.Width() / 2) );
                        aObjPos.AdjustY( -(aObjSize.Height() / 2) );
                        ::tools::Rectangle aNewObjectRectangle(aObjPos, aObjSize);

                        pNewDBField->SetLogicRect(aNewObjectRectangle);

                        GetView()->InsertObjectAtView(pNewDBField.get(), *pPageView);
                    }
                }
            }
            rReq.Done();
        }
        break;

        case SID_OBJECT_SELECT:
        case SID_OBJECT_ROTATE:
        case SID_OBJECT_MIRROR:
        case SID_OBJECT_CROP:
        case SID_OBJECT_TRANSPARENCE:
        case SID_OBJECT_GRADIENT:
        case SID_OBJECT_SHEAR:
        case SID_OBJECT_CROOK_ROTATE:
        case SID_OBJECT_CROOK_SLANT:
        case SID_OBJECT_CROOK_STRETCH:
        case SID_CONVERT_TO_3D_LATHE:
        {
            sal_uInt16 nSlotId = rReq.GetSlot();

            // toggle function
            if( nOldSId == nSlotId )
            {
                nSlotId = SID_OBJECT_SELECT;
                rReq.SetSlot( nSlotId );
            }

            const SdrMarkList&  rMarkList = mpDrawView->GetMarkedObjectList();
            if (nSlotId == SID_OBJECT_CROOK_ROTATE ||
                nSlotId == SID_OBJECT_CROOK_SLANT ||
                nSlotId == SID_OBJECT_CROOK_STRETCH)
            {
                if ( rMarkList.GetMarkCount() > 0 &&
                    !mpDrawView->IsCrookAllowed( mpDrawView->IsCrookNoContortion() ) )
                {
                    if ( mpDrawView->IsPresObjSelected() )
                    {
                        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                                      SdResId(STR_ACTION_NOTPOSSIBLE)));
                        xInfoBox->run();
                    }
                    else
                    {
                        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                                       SdResId(STR_ASK_FOR_CONVERT_TO_BEZIER)));
                        if (xQueryBox->run() == RET_YES )
                        {
                            // implicit transformation into bezier
                            weld::WaitObject aWait(GetFrameWeld());
                            mpDrawView->ConvertMarkedToPathObj(false);
                        }
                    }
                }
            }
            else if (nSlotId == SID_OBJECT_SHEAR)
            {
                size_t i = 0;
                const size_t nMarkCnt = rMarkList.GetMarkCount();
                bool b3DObjMarked = false;

                while (i < nMarkCnt && !b3DObjMarked)
                {
                    if (DynCastE3dObject( rMarkList.GetMark(i)->GetMarkedSdrObj() ))
                    {
                        b3DObjMarked = true;
                    }
                    else
                    {
                        i++;
                    }
                }

                if ( nMarkCnt > 0 && !b3DObjMarked &&
                     (!mpDrawView->IsShearAllowed() || !mpDrawView->IsDistortAllowed()) )
                {
                    if ( mpDrawView->IsPresObjSelected() )
                    {
                        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                                      SdResId(STR_ACTION_NOTPOSSIBLE)));
                        xInfoBox->run();
                    }
                    else
                    {
                        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                                       SdResId(STR_ASK_FOR_CONVERT_TO_BEZIER)));
                        if (xQueryBox->run() == RET_YES)
                        {
                            // implicit transformation into bezier
                            weld::WaitObject aWait(GetFrameWeld());
                            mpDrawView->ConvertMarkedToPathObj(false);
                        }
                    }
                }
            }

            SetCurrentFunction( FuSelection::Create(*this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq) );
            rReq.Done();
            Invalidate( SID_OBJECT_SELECT );
        }
        break;

        case SID_DRAW_LINE:
        case SID_DRAW_XLINE:
        case SID_DRAW_MEASURELINE:
        case SID_LINE_ARROW_START:
        case SID_LINE_ARROW_END:
        case SID_LINE_ARROWS:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_ARROW_SQUARE:
        case SID_LINE_SQUARE_ARROW:

        case SID_DRAW_RECT:
        case SID_DRAW_RECT_NOFILL:
        case SID_DRAW_RECT_ROUND:
        case SID_DRAW_RECT_ROUND_NOFILL:
        case SID_DRAW_SQUARE:
        case SID_DRAW_SQUARE_NOFILL:
        case SID_DRAW_SQUARE_ROUND:
        case SID_DRAW_SQUARE_ROUND_NOFILL:
        case SID_DRAW_ELLIPSE:
        case SID_DRAW_ELLIPSE_NOFILL:
        case SID_DRAW_CIRCLE:
        case SID_DRAW_CIRCLE_NOFILL:
        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
        case SID_TOOL_CONNECTOR:
        case SID_CONNECTOR_ARROW_START:
        case SID_CONNECTOR_ARROW_END:
        case SID_CONNECTOR_ARROWS:
        case SID_CONNECTOR_CIRCLE_START:
        case SID_CONNECTOR_CIRCLE_END:
        case SID_CONNECTOR_CIRCLES:
        case SID_CONNECTOR_LINE:
        case SID_CONNECTOR_LINE_ARROW_START:
        case SID_CONNECTOR_LINE_ARROW_END:
        case SID_CONNECTOR_LINE_ARROWS:
        case SID_CONNECTOR_LINE_CIRCLE_START:
        case SID_CONNECTOR_LINE_CIRCLE_END:
        case SID_CONNECTOR_LINE_CIRCLES:
        case SID_CONNECTOR_CURVE:
        case SID_CONNECTOR_CURVE_ARROW_START:
        case SID_CONNECTOR_CURVE_ARROW_END:
        case SID_CONNECTOR_CURVE_ARROWS:
        case SID_CONNECTOR_CURVE_CIRCLE_START:
        case SID_CONNECTOR_CURVE_CIRCLE_END:
        case SID_CONNECTOR_CURVE_CIRCLES:
        case SID_CONNECTOR_LINES:
        case SID_CONNECTOR_LINES_ARROW_START:
        case SID_CONNECTOR_LINES_ARROW_END:
        case SID_CONNECTOR_LINES_ARROWS:
        case SID_CONNECTOR_LINES_CIRCLE_START:
        case SID_CONNECTOR_LINES_CIRCLE_END:
        case SID_CONNECTOR_LINES_CIRCLES:
        case SID_INSERT_SIGNATURELINE:
        {
            if (nSId == SID_INSERT_SIGNATURELINE)
            {
                // See if a signing cert is passed as a parameter: if so, parse that.
                OString aSignatureCert;
                OString aSignatureKey;
                const SfxStringItem* pSignatureCert = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
                if (pSignatureCert)
                {
                    aSignatureCert = pSignatureCert->GetValue().toUtf8();
                }
                const SfxStringItem* pSignatureKey = rReq.GetArg<SfxStringItem>(FN_PARAM_2);
                if (pSignatureKey)
                {
                    aSignatureKey = pSignatureKey->GetValue().toUtf8();
                }
                bool bExternal = false;
                const SfxBoolItem* pExternal = rReq.GetArg<SfxBoolItem>(FN_PARAM_3);
                if (pExternal)
                {
                    bExternal = pExternal->GetValue();
                }

                SfxViewFrame* pFrame = GetFrame();
                SfxViewShell* pViewShell = pFrame ? pFrame->GetViewShell() : nullptr;
                if (pViewShell)
                {
                    svl::crypto::CertificateOrName aCertificateOrName;
                    if (!aSignatureCert.isEmpty() && !aSignatureKey.isEmpty())
                    {
                        aCertificateOrName.m_xCertificate = KitHelper::getSigningCertificate(aSignatureCert, aSignatureKey);
                    }
                    else if (bExternal)
                    {
                        aCertificateOrName.m_aName = pViewShell->GetKitAuthor();
                    }
                    // Always set the signing certificate, to clear data from a previous dispatch.
                    pViewShell->SetSigningCertificate(aCertificateOrName);
                }
            }

            bCreateDirectly = comphelper::COKit::isActive();
            bRectangle = true;

            // a line dragged out in the client comes with its two points and is made at once
            switch (nSId)
            {
                case SID_DRAW_LINE:
                case SID_DRAW_XLINE:
                case SID_DRAW_MEASURELINE:
                case SID_LINE_ARROW_START:
                case SID_LINE_ARROW_END:
                case SID_LINE_ARROWS:
                case SID_LINE_ARROW_CIRCLE:
                case SID_LINE_CIRCLE_ARROW:
                case SID_LINE_ARROW_SQUARE:
                case SID_LINE_SQUARE_ARROW:
                case SID_TOOL_CONNECTOR:
                case SID_CONNECTOR_ARROW_START:
                case SID_CONNECTOR_ARROW_END:
                case SID_CONNECTOR_ARROWS:
                case SID_CONNECTOR_CIRCLE_START:
                case SID_CONNECTOR_CIRCLE_END:
                case SID_CONNECTOR_CIRCLES:
                case SID_CONNECTOR_LINE:
                case SID_CONNECTOR_LINE_ARROW_START:
                case SID_CONNECTOR_LINE_ARROW_END:
                case SID_CONNECTOR_LINE_ARROWS:
                case SID_CONNECTOR_LINE_CIRCLE_START:
                case SID_CONNECTOR_LINE_CIRCLE_END:
                case SID_CONNECTOR_LINE_CIRCLES:
                case SID_CONNECTOR_CURVE:
                case SID_CONNECTOR_CURVE_ARROW_START:
                case SID_CONNECTOR_CURVE_ARROW_END:
                case SID_CONNECTOR_CURVE_ARROWS:
                case SID_CONNECTOR_CURVE_CIRCLE_START:
                case SID_CONNECTOR_CURVE_CIRCLE_END:
                case SID_CONNECTOR_CURVE_CIRCLES:
                case SID_CONNECTOR_LINES:
                case SID_CONNECTOR_LINES_ARROW_START:
                case SID_CONNECTOR_LINES_ARROW_END:
                case SID_CONNECTOR_LINES_ARROWS:
                case SID_CONNECTOR_LINES_CIRCLE_START:
                case SID_CONNECTOR_LINES_CIRCLE_END:
                case SID_CONNECTOR_LINES_CIRCLES:
                    oRequestedLine = lcl_getRequestedLine(rReq);
                    break;
            }
            if (oRequestedLine)
            {
                bCreateDirectly = true;
                oRequestedRectangle
                    = ::tools::Rectangle::Normalize(oRequestedLine->first, oRequestedLine->second);
            }

            SetCurrentFunction( FuConstructRectangle::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq, bPermanent ) );
            rReq.Done();
        }
        break;
        case SID_DRAW_POLYGON:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_XPOLYGON:
        case SID_DRAW_XPOLYGON_NOFILL:
        case SID_DRAW_FREELINE:
        case SID_DRAW_FREELINE_NOFILL:
        case SID_DRAW_BEZIER_FILL:          // BASIC
        case SID_DRAW_BEZIER_NOFILL:        // BASIC
        {
            // Direct mode means no interactive drawing, just insert the shape with reasonable
            // defaults -- to be consistent with the line insert case above.
            bCreateDirectly = comphelper::COKit::isActive();

            SetCurrentFunction( FuConstructBezierPolygon::Create(*this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq, bPermanent) );
            rReq.Done();
        }
        break;

        case SID_GLUE_EDITMODE:
        {
            if (nOldSId != SID_GLUE_EDITMODE)
            {
                SetCurrentFunction( FuEditGluePoints::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq, bPermanent ) );
            }
            else
            {
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }

            rReq.Done();
        }
        break;

        case SID_DRAW_ARC:
        case SID_DRAW_CIRCLEARC:
        case SID_DRAW_PIE:
        case SID_DRAW_PIE_NOFILL:
        case SID_DRAW_CIRCLEPIE:
        case SID_DRAW_CIRCLEPIE_NOFILL:
        case SID_DRAW_ELLIPSECUT:
        case SID_DRAW_ELLIPSECUT_NOFILL:
        case SID_DRAW_CIRCLECUT:
        case SID_DRAW_CIRCLECUT_NOFILL:
        {
            SetCurrentFunction( FuConstructArc::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq, bPermanent) );
            rReq.Done();
        }
        break;

        case SID_3D_CUBE:
        case SID_3D_SHELL:
        case SID_3D_SPHERE:
        case SID_3D_TORUS:
        case SID_3D_HALF_SPHERE:
        case SID_3D_CYLINDER:
        case SID_3D_CONE:
        case SID_3D_PYRAMID:
        {
            SetCurrentFunction( FuConstruct3dObject::Create(*this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq, bPermanent ) );
            rReq.Done();
        }
        break;

        case SID_DRAWTBX_CS_BASIC :
        case SID_DRAWTBX_CS_SYMBOL :
        case SID_DRAWTBX_CS_ARROW :
        case SID_DRAWTBX_CS_FLOWCHART :
        case SID_DRAWTBX_CS_CALLOUT :
        case SID_DRAWTBX_CS_STAR :
        case SID_DRAW_CS_ID :
        {
            SetCurrentFunction( FuConstructCustomShape::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq, bPermanent ) );
            rReq.Done();

            bCreateDirectly = comphelper::COKit::isActive();
            const SfxItemSet* pArgs = rReq.GetArgs();
            if (pArgs && pArgs->HasItem(FN_PARAM_1))
            {
                bCreateDirectly = static_cast<const SfxBoolItem&>(pArgs->Get(FN_PARAM_1)).GetValue();
            }

            // a shape dragged out in the client comes with its rectangle and is made at once
            oRequestedRectangle = lcl_getRequestedRectangle(rReq);
            if (oRequestedRectangle)
                bCreateDirectly = true;

            if ( nSId != SID_DRAW_CS_ID )
            {
                SfxBindings& rBind = GetViewFrame()->GetBindings();
                rBind.Invalidate( nSId );
                rBind.Update( nSId );
            }
        }
        break;

        case SID_FORMATPAINTBRUSH:
        {
            SetCurrentFunction( FuFormatPaintBrush::Create( *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            rReq.Done();
            SfxBindings& rBind = GetViewFrame()->GetBindings();
            rBind.Invalidate( nSId );
            rBind.Update( nSId );
            break;
        }

        case SID_ZOOM_MODE:
        case SID_ZOOM_PANNING:
        {
            if (nOldSId != nSId)
            {
                mbZoomOnPage = false;
                SetCurrentFunction( FuZoom::Create(*this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), rReq ) );
            }
            else
            {
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }
            rReq.Done();
        }
        break;

        default:
           break;
    }

    if(HasOldFunction())
    {
        sal_uInt16 nSlotId = GetOldFunction()->GetSlotID();

        GetOldFunction()->Deactivate();
        SetOldFunction(nullptr);

        SfxBindings& rBind = GetViewFrame()->GetBindings();
        rBind.Invalidate( nSlotId );
        rBind.Update( nSlotId );
    }

    if(HasCurrentFunction())
    {
        GetCurrentFunction()->Activate();
        SetOldFunction( GetCurrentFunction() );
    }

    // invalidate shell, is faster than every individually (says MI)
    // now explicit the last slot incl. Update()
    Invalidate();

    // CTRL-SID_OBJECT_SELECT -> select first draw object if none is selected yet
    if(SID_OBJECT_SELECT == nSId && HasCurrentFunction() && (rReq.GetModifier() & KEY_MOD1))
    {
        const SdrMarkList&  rMarkList = GetView()->GetMarkedObjectList();
        if(rMarkList.GetMarkCount() == 0)
        {
            // select first object
            GetView()->UnmarkAllObj();
            GetView()->MarkNextObj(true);

            // ...and make it visible
            if(rMarkList.GetMarkCount() != 0)
                GetView()->MakeVisible(GetView()->GetAllMarkedRect(), *GetActiveWindow());
        }
    }

    // with qualifier construct directly
    if(!(HasCurrentFunction() && ((rReq.GetModifier() & KEY_MOD1) || bCreateDirectly)))
        return;

    const Size aDefaultObjectSize(GetDefaultObjectSize());
    sal_uInt32 nDefaultObjectSizeWidth(aDefaultObjectSize.Width());
    sal_uInt32 nDefaultObjectSizeHeight(aDefaultObjectSize.Height());
    if (nSId == SID_INSERT_SIGNATURELINE)
    {
        // Half of the default to better match the space available for signatures in many real-world
        // documents.
        nDefaultObjectSizeWidth *= 0.5;
        nDefaultObjectSizeHeight *= 0.5;
    }

    // calc position and size
    ::tools::Rectangle aVisArea = GetActiveWindow()->PixelToLogic(::tools::Rectangle(Point(0,0), GetActiveWindow()->GetOutputSizePixel()));
    if (comphelper::COKit::isActive())
    {
        // aVisArea is nonsensical in the COKit case, use the slide size
        aVisArea = ::tools::Rectangle(Point(), getCurrentPage()->GetSize());
    }

    Point aPagePos = aVisArea.Center();
    aPagePos.AdjustX( -sal_Int32(nDefaultObjectSizeWidth / 2) );
    aPagePos.AdjustY( -sal_Int32(nDefaultObjectSizeHeight / 2) );
    ::tools::Rectangle aNewObjectRectangle(aPagePos, Size(nDefaultObjectSizeWidth, nDefaultObjectSizeHeight));
    if (oRequestedRectangle)
        aNewObjectRectangle = *oRequestedRectangle;
    SdrPageView* pPageView = mpDrawView->GetSdrPageView();

    if(!pPageView)
        return;

    // the object made first is the new one, so it takes the id the client gave it
    UniqueID::giveNext(nGivenId);
    rtl::Reference<SdrObject> pObj
        = GetCurrentFunction()->CreateDefaultObject(nSId, aNewObjectRectangle);
    UniqueID::giveNext(0);

    if(!pObj)
        return;

    // the default line lies across the middle of its rectangle; a requested one goes from its
    // start to its end, so the arrow of the start stays at the start
    if (oRequestedLine)
    {
        const auto& [aStart, aEnd] = *oRequestedLine;
        if (auto pPath = dynamic_cast<SdrPathObj*>(pObj.get()))
        {
            basegfx::B2DPolygon aLine;
            aLine.append(basegfx::B2DPoint(aStart.X(), aStart.Y()));
            aLine.append(basegfx::B2DPoint(aEnd.X(), aEnd.Y()));
            pPath->SetPathPoly(basegfx::B2DPolyPolygon(aLine));
        }
        else if (auto pMeasure = dynamic_cast<SdrMeasureObj*>(pObj.get()))
        {
            pMeasure->SetPoint(aStart, 0);
            pMeasure->SetPoint(aEnd, 1);
        }
        else if (auto pConnector = dynamic_cast<SdrEdgeObj*>(pObj.get()))
        {
            pConnector->SetTailPoint(true, aStart);
            pConnector->SetTailPoint(false, aEnd);
        }
    }

    if (bTextGrows)
    {
        // the text starts where it was clicked and grows to the right, as the office's text tool
        // makes it
        if (SdrTextObj* pText = DynCastSdrTextObj(pObj.get()))
        {
            pText->SetMergedItem(makeSdrTextAutoGrowWidthItem(true));
            pText->SetMergedItem(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT));
            pText->AdjustTextFrameWidthAndHeight();
        }
    }

    auto pObjTmp = pObj.get();
    // insert into page
    GetView()->InsertObjectAtView(pObj.get(), *pPageView);

    // a connector's run is laid out between the objects it ties only while all are on one page,
    // so its ends are tied as the client found them once it is in; the ties are part of the
    // object, which the undo of the insertion takes out whole
    if (auto pConnector = oRequestedLine ? dynamic_cast<SdrEdgeObj*>(pObjTmp) : nullptr)
    {
        const auto oTies = lcl_getRequestedTies(rReq);
        for (const bool bStart : { true, false })
        {
            const sal_Int64 nObjectId(oTies ? (*oTies)[bStart ? 0 : 2] : 0);
            SdrObject* pNode
                = nObjectId > 0 ? pPageView->GetPage()->FindObjectByUniqueID(nObjectId) : nullptr;
            pConnector->MoveEndTo(bStart, bStart ? oRequestedLine->first : oRequestedLine->second,
                                  pNode,
                                  oTies ? static_cast<sal_Int32>((*oTies)[bStart ? 1 : 3]) : -1);
        }
        pConnector->SetChanged();
        pConnector->BroadcastObjectChange();
    }

    // Now that pFuActual has done what it was created for we
    // can switch on the edit mode for callout objects.
    switch (nSId)
    {
        case SID_DRAW_CAPTION:
        case SID_DRAW_CAPTION_VERTICAL:
        case SID_ATTR_CHAR:
        case SID_ATTR_CHAR_VERTICAL:
        case SID_TEXT_FITTOSIZE:
        case SID_TEXT_FITTOSIZE_VERTICAL:
        {
            // a text the client dragged out or clicked is edited by the text function that made
            // it, which goes back to the selection when the edit ends and removes a text left
            // empty, as the office's text tool does
            if (bTextInRectangle)
            {
                FuText* pFuText = dynamic_cast<FuText*>(GetCurrentFunction().get());
                SdrTextObj* pText = DynCastSdrTextObj(pObjTmp);
                if (pFuText && pText)
                {
                    mpDrawView->MarkObj(pText, pPageView);
                    pFuText->EditCreatedText(*pText);
                    return;
                }
            }

            // Make FuText the current function.
            SfxUInt16Item aItem (SID_TEXTEDIT, 1);
            GetViewFrame()->GetDispatcher()->
                ExecuteList(SID_TEXTEDIT, SfxCallMode::SYNCHRON, { &aItem });
            // Put text object into edit mode.
            GetView()->SdrBeginTextEdit(static_cast<SdrTextObj*>(pObjTmp), pPageView);
            break;
        }
    }

    if ((bCreateDirectly || bRectangle) && !bPermanent)
    {
        GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
    }
}

Size DrawViewShell::GetDefaultObjectSize() const
{
    SdOptions* pOptions = SdModule::get()->GetSdOptions(GetDoc()->GetDocumentType());
    return Size(pOptions->GetDefaultObjectSizeWidth(), pOptions->GetDefaultObjectSizeHeight());
}

sal_uInt64 DrawViewShell::ReserveObjectIds(sal_uInt32 nCount)
{
    const sal_uInt64 nFirst(UniqueID::reserve(nCount));
    for (sal_uInt32 nAt = 0; nAt < nCount; ++nAt)
        maReservedObjectIds.insert(nFirst + nAt);
    return nFirst;
}

bool DrawViewShell::TakeReservedObjectId(sal_uInt64 nId)
{
    return maReservedObjectIds.erase(nId) > 0;
}

rtl::Reference<SdrObject> DrawViewShell::MakeDefaultShape(sal_uInt16 nSlotId,
                                                          const OUString& rShapeType)
{
    // the custom shape tool makes it as it makes a shape created directly, from the kind of
    // object the view is set to create, which is put back after
    const SdrObjKind eKind(mpDrawView->GetCurrentObjIdentifier());
    const SdrInventor eInventor(mpDrawView->GetCurrentObjInventor());
    mpDrawView->SetCurrentObj(SdrObjKind::CustomShape);

    SfxAllItemSet aArguments(GetDoc()->GetPool());
    aArguments.Put(SfxStringItem(nSlotId, rShapeType));
    SfxRequest aRequest(nSlotId, SfxCallMode::SYNCHRON, aArguments);
    rtl::Reference<FuPoor> xFunction = FuConstructCustomShape::Create(
        *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), aRequest, false);
    rtl::Reference<SdrObject> xShape = xFunction->CreateDefaultObject(
        nSlotId, ::tools::Rectangle(Point(), GetDefaultObjectSize()));

    mpDrawView->SetCurrentObj(eKind, eInventor);
    return xShape;
}

// when the libraries that make a shape and write how it looks were built, as the times they were
// last changed; a new build changes it, so looks kept from another build are not taken
static const OString& lcl_getShapeBuild()
{
    static const OString aBuild = []() {
        const oslGenericFunction aInLibraries[] = {
            reinterpret_cast<oslGenericFunction>(&lcl_getRequestedRectangle),
            reinterpret_cast<oslGenericFunction>(&SdrObjCustomShape::doConstructOrthogonal),
            reinterpret_cast<oslGenericFunction>(
                &drawinglayer::Primitive2dJsonProcessor::writeGraphicBase64),
        };
        std::set<OUString> aLibraries;
        for (oslGenericFunction pFunction : aInLibraries)
        {
            OUString aUrl;
            if (osl::Module::getUrlFromAddress(pFunction, aUrl))
                aLibraries.insert(aUrl);
        }
        OStringBuffer aTimes;
        for (const OUString& rUrl : aLibraries)
        {
            osl::DirectoryItem aItem;
            osl::FileStatus aStatus(osl_FileStatus_Mask_ModifyTime);
            if (osl::DirectoryItem::get(rUrl, aItem) != osl::FileBase::E_None
                || aItem.getFileStatus(aStatus) != osl::FileBase::E_None)
                continue;
            const TimeValue aTime(aStatus.getModifyTime());
            aTimes.append(OString::number(aTime.Seconds) + "." + OString::number(aTime.Nanosec)
                          + " ");
        }
        return aTimes.makeStringAndClear();
    }();
    return aBuild;
}

OString DrawViewShell::GetShapeStyleKey()
{
    // every new shape takes the same attributes but its geometry, so we look at a rectangle
    rtl::Reference<SdrObject> xShape(MakeDefaultShape(SID_DRAWTBX_CS_BASIC, u"rectangle"_ustr));
    if (!xShape)
        return OString();

    xmlBufferPtr pBuffer = xmlBufferCreate();
    xmlTextWriterPtr pWriter = xmlNewTextWriterMemory(pBuffer, 0);
    (void)xmlTextWriterStartDocument(pWriter, nullptr, nullptr, nullptr);
    // the attributes the shape inherits from its style count as well
    for (const SfxItemSet* pSet = &xShape->GetMergedItemSet(); pSet; pSet = pSet->GetParent())
        pSet->dumpAsXml(pWriter);
    (void)xmlTextWriterEndDocument(pWriter);
    xmlFreeTextWriter(pWriter);
    OString aDump(reinterpret_cast<const char*>(xmlBufferContent(pBuffer)));
    xmlBufferFree(pBuffer);

    // the items name their place in memory, which differs from shape to shape
    OStringBuffer aKeyed(aDump.getLength());
    for (sal_Int32 nAt = 0; nAt < aDump.getLength();)
    {
        const sal_Int32 nPointer = aDump.indexOf(" ptr=\"", nAt);
        const sal_Int32 nEnd = nPointer < 0 ? -1 : aDump.indexOf('"', nPointer + 6);
        if (nEnd < 0)
        {
            aKeyed.append(aDump.subView(nAt));
            break;
        }
        aKeyed.append(aDump.subView(nAt, nPointer - nAt));
        nAt = nEnd + 1;
    }

    aKeyed.append(lcl_getShapeBuild());
    const std::vector<unsigned char> aHash(comphelper::Hash::calculateHash(
        aKeyed.getStr(), aKeyed.getLength(), comphelper::HashType::SHA1));
    return OString(comphelper::hashToString(aHash));
}

rtl::Reference<SdrObject> DrawViewShell::MakeDefaultLineObject(sal_uInt16 nSlotId)
{
    SdrObjKind eMade;
    switch (nSlotId)
    {
        case SID_DRAW_MEASURELINE:
            eMade = SdrObjKind::Measure;
            break;
        case SID_TOOL_CONNECTOR:
        case SID_CONNECTOR_ARROW_START:
        case SID_CONNECTOR_ARROW_END:
        case SID_CONNECTOR_ARROWS:
        case SID_CONNECTOR_CIRCLE_START:
        case SID_CONNECTOR_CIRCLE_END:
        case SID_CONNECTOR_CIRCLES:
        case SID_CONNECTOR_LINE:
        case SID_CONNECTOR_LINE_ARROW_START:
        case SID_CONNECTOR_LINE_ARROW_END:
        case SID_CONNECTOR_LINE_ARROWS:
        case SID_CONNECTOR_LINE_CIRCLE_START:
        case SID_CONNECTOR_LINE_CIRCLE_END:
        case SID_CONNECTOR_LINE_CIRCLES:
        case SID_CONNECTOR_CURVE:
        case SID_CONNECTOR_CURVE_ARROW_START:
        case SID_CONNECTOR_CURVE_ARROW_END:
        case SID_CONNECTOR_CURVE_ARROWS:
        case SID_CONNECTOR_CURVE_CIRCLE_START:
        case SID_CONNECTOR_CURVE_CIRCLE_END:
        case SID_CONNECTOR_CURVE_CIRCLES:
        case SID_CONNECTOR_LINES:
        case SID_CONNECTOR_LINES_ARROW_START:
        case SID_CONNECTOR_LINES_ARROW_END:
        case SID_CONNECTOR_LINES_ARROWS:
        case SID_CONNECTOR_LINES_CIRCLE_START:
        case SID_CONNECTOR_LINES_CIRCLE_END:
        case SID_CONNECTOR_LINES_CIRCLES:
            eMade = SdrObjKind::Edge;
            break;
        default:
            return nullptr;
    }

    // the view is set to create that kind of object while the tool makes it, and put back after
    const SdrObjKind eKind(mpDrawView->GetCurrentObjIdentifier());
    const SdrInventor eInventor(mpDrawView->GetCurrentObjInventor());
    mpDrawView->SetCurrentObj(eMade);

    SfxRequest aRequest(nSlotId, SfxCallMode::SYNCHRON, GetDoc()->GetPool());
    rtl::Reference<FuPoor> xFunction = FuConstructRectangle::Create(
        *this, GetActiveWindow(), mpDrawView.get(), *GetDoc(), aRequest, false);
    rtl::Reference<SdrObject> xLine = xFunction->CreateDefaultObject(
        nSlotId, ::tools::Rectangle(Point(), GetDefaultObjectSize()));

    mpDrawView->SetCurrentObj(eKind, eInventor);
    return xLine;
}

void DrawViewShell::FuDeleteSelectedObjects()
{
    if( !mpDrawView )
        return;

    bool bConsumed = false;

    //if any placeholders are selected
    if (mpDrawView->IsPresObjSelected(false))
    {
        //If there are placeholders in the list which can be toggled
        //off in edit->master->master elements then do that here,
        std::vector<SdrObject*> aPresMarksToRemove;
        const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
        for (size_t i=0; i < rMarkList.GetMarkCount(); ++i)
        {
            SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
            SdPage* pPage = static_cast<SdPage*>(pObj->getSdrPageFromSdrObject());
            PresObjKind eKind = pPage->GetPresObjKind(pObj);
            if (eKind == PresObjKind::Footer || eKind == PresObjKind::Header ||
                eKind == PresObjKind::DateTime || eKind == PresObjKind::SlideNumber)
            {
                aPresMarksToRemove.push_back(pObj);
            }
        }

        for (SdrObject* pObj : aPresMarksToRemove)
        {
            //Unmark object
            mpDrawView->MarkObj(pObj, mpDrawView->GetSdrPageView(), true);
            SdPage* pPage = static_cast<SdPage*>(pObj->getSdrPageFromSdrObject());
            //remove placeholder from master page
            pPage->DestroyDefaultPresObj(pPage->GetPresObjKind(pObj));
        }

        bConsumed = true;
    }

    // placeholders which cannot be deleted selected
    if (mpDrawView->IsPresObjSelected(false, true, false, true))
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      SdResId(STR_ACTION_NOTPOSSIBLE)));
        xInfoBox->run();
        bConsumed = true;
    }

    if (bConsumed)
        return;

    vcl::KeyCode aKCode(KEY_DELETE);
    KeyEvent aKEvt( 0, aKCode);

    bConsumed = mpDrawView->getSmartTags().KeyInput( aKEvt );

    if (!bConsumed && HasCurrentFunction())
        bConsumed = GetCurrentFunction()->KeyInput(aKEvt);

    if (!bConsumed)
        mpDrawView->DeleteMarked();
}

IMPL_LINK(DrawViewShell, SearchDialogHdl, SfxRequest&, rReq, void)
{
    // Get the parameter from the request
    const SfxItemSet* pArgs = rReq.GetArgs();
    bool bInitialFocusOnReplace = false;

    if (pArgs)
    {
        const SfxBoolItem* pBoolItem = pArgs->GetItemIfSet(SID_SEARCH_DLG, false);
        if (pBoolItem)
            bInitialFocusOnReplace = pBoolItem->GetValue();
    }

    // Get existing search item to preserve settings
    std::unique_ptr<SvxSearchItem> pSearchItem;
    std::unique_ptr<SvxSearchItem> pExistingItem;
    if (GetViewFrame()->GetBindings().QueryState(SID_SEARCH_ITEM, pExistingItem) >= SfxItemState::DEFAULT && pExistingItem)
    {
        pSearchItem.reset(pExistingItem->Clone());
    }
    else
    {
        pSearchItem = std::make_unique<SvxSearchItem>(SID_SEARCH_ITEM);
    }

    // Only set the focus parameter, preserving all other settings
    pSearchItem->SetInitialFocusOnReplace(bInitialFocusOnReplace);

    // Execute the search dialog with the configured item
    const SfxPoolItem* ppArgs[] = { pSearchItem.get(), nullptr };
    GetViewFrame()->GetBindings().GetDispatcher()->Execute(SID_SEARCH_ITEM, SfxCallMode::SYNCHRON, ppArgs);
    GetViewFrame()->ToggleChildWindow(SvxSearchDialogWrapper::GetChildWindowId());
}

// The view with the id that pViewIdItem gives, when it is another view of the same document.
static SfxViewShell* GetOtherViewShell(const SfxViewShell& rThisShell,
                                       const SfxInt32Item* pViewIdItem)
{
    SfxViewShell* pShell = pViewIdItem ? KitHelper::getViewOfId(pViewIdItem->GetValue()) : nullptr;
    if (!pShell || pShell == &rThisShell || pShell->GetDocId() != rThisShell.GetDocId())
        return nullptr;
    return pShell;
}

// The Impress or Draw view shell of pShell, when pShell shows a page of slides or drawings.
static DrawViewShell* GetDrawViewShellOf(SfxViewShell* pShell)
{
    auto pBase = dynamic_cast<ViewShellBase*>(pShell);
    return pBase ? dynamic_cast<DrawViewShell*>(pBase->GetMainViewShell().get()) : nullptr;
}

// pObjectIdItem gives the unique id of rObj, as a decimal number.
static bool IsObjectOf(const SdrObject& rObj, const SfxStringItem* pObjectIdItem)
{
    return pObjectIdItem && !pObjectIdItem->GetValue().isEmpty()
           && pObjectIdItem->GetValue().toUInt64() == rObj.GetUniqueID();
}

// The view of rShell shows the page that holds rObj.
static bool ShowsPageOf(DrawViewShell& rShell, const SdrObject& rObj)
{
    SdrPageView* pPageView = rShell.GetView()->GetSdrPageView();
    return pPageView && pPageView->GetPage() == rObj.getSdrPageFromSdrObject();
}

// The view of rShell edits the text of rTextObject, as typing into a selected shape starts an edit.
// It returns false when the edit does not start, or when rTextObject is no longer on its page. A
// new text box without text leaves its page when its edit ends.
static bool BeginTextEditOf(DrawViewShell& rShell, SdrTextObj& rTextObject)
{
    if (!rTextObject.IsInserted())
        return false;

    ::sd::View* pView = rShell.GetView();
    if (pView->IsTextEdit())
        pView->SdrEndTextEdit();
    pView->UnmarkAll();
    pView->MarkObj(&rTextObject, pView->GetSdrPageView());
    rShell.GetViewFrame()->GetDispatcher()->Execute(SID_ATTR_CHAR, SfxCallMode::SYNCHRON);
    return pView->GetTextEditObject() == &rTextObject;
}

// Run rFunction with rShell as the current view, as when the user of rShell acts in it. The view
// that was current before is current again afterwards.
static void RunAsView(const SfxViewShell& rShell, const std::function<void()>& rFunction)
{
    const int nCurrentViewId = KitHelper::getCurrentView();
    KitHelper::setView(KitHelper::getView(rShell));
    comphelper::ScopeGuard aRestoreView([nCurrentViewId] { KitHelper::setView(nCurrentViewId); });
    rFunction();
}

void DrawViewShell::FuSupport(SfxRequest& rReq)
{
    if( rReq.GetSlot() == SID_STYLE_FAMILY && rReq.GetArgs())
        GetDocSh()->SetStyleFamily(static_cast<SfxStyleFamily>(rReq.GetArgs()->Get( SID_STYLE_FAMILY ).GetValue()));

    CheckLineTo (rReq);

    if( !mpDrawView )
        return;

    sal_uInt16 nSId = rReq.GetSlot();

    switch ( nSId )
    {
        case SID_CLEAR_UNDO_STACK:
        {
            GetDocSh()->ClearUndoBuffer();
        }
        break;

        case SID_PRESENTATION_END:
            break;

        case SID_TAKE_OVER_TEXT_EDIT:
        {
            // The view that edits the text of the given object writes its text into the object and
            // leaves it. Then this view edits the object.
            SfxViewShell* pEditingShell
                = GetOtherViewShell(GetViewShellBase(), rReq.GetArg<SfxInt32Item>(FN_PARAM_1));
            DrawViewShell* pEditingDrawShell = GetDrawViewShellOf(pEditingShell);
            rtl::Reference<SdrTextObj> xTextObject
                = pEditingDrawShell ? pEditingDrawShell->GetView()->GetTextEditObject() : nullptr;
            if (!xTextObject || !IsObjectOf(*xTextObject, rReq.GetArg<SfxStringItem>(FN_PARAM_2))
                || !ShowsPageOf(*this, *xTextObject))
                break;

            // The other view leaves the text as when its own user leaves it, so the undo step of
            // that text belongs to that view.
            RunAsView(*pEditingShell,
                      [pEditingDrawShell] { pEditingDrawShell->GetView()->SdrEndTextEdit(); });
            if (!BeginTextEditOf(*this, *xTextObject))
            {
                // The other view edits the object again, when the object is still on its page.
                RunAsView(*pEditingShell, [pEditingDrawShell, &xTextObject] {
                    BeginTextEditOf(*pEditingDrawShell, *xTextObject);
                });
                break;
            }

            // The requests for the text went to the view that edited the object before.
            pEditingDrawShell->maTextEditRequests.clear();
            mnTextEditTakenFromViewId = KitHelper::getView(*pEditingShell);
            mnTextEditTakenObjectId = xTextObject->GetUniqueID();

            // The view that edited the object learns which view edits it now.
            KitHelper::notifyTextEditLocked(GetViewShellBase(), *pEditingShell, "takenover");
            rReq.Done();
        }
        break;

        case SID_REQUEST_TEXT_EDIT:
        {
            // This view asks the view that edits the text of the given object on this page to
            // leave it.
            SfxViewShell* pEditingShell
                = GetOtherViewShell(GetViewShellBase(), rReq.GetArg<SfxInt32Item>(FN_PARAM_1));
            DrawViewShell* pEditingDrawShell = GetDrawViewShellOf(pEditingShell);
            SdrTextObj* pTextObject
                = pEditingDrawShell ? pEditingDrawShell->GetView()->GetTextEditObject() : nullptr;
            if (!pTextObject || !IsObjectOf(*pTextObject, rReq.GetArg<SfxStringItem>(FN_PARAM_2))
                || !ShowsPageOf(*this, *pTextObject))
                break;

            pEditingDrawShell->maTextEditRequests[KitHelper::getView(GetViewShellBase())]
                = pTextObject->GetUniqueID();
            KitHelper::notifyTextEditLocked(GetViewShellBase(), *pEditingShell, "requested");
            rReq.Done();
        }
        break;

        case SID_REJECT_TEXT_EDIT_REQUEST:
        {
            // This view goes on with the text it edits, and the view that asked for that text
            // learns so. Only a view that asked for the text that this view edits now gets the
            // answer.
            SfxViewShell* pAskingShell
                = GetOtherViewShell(GetViewShellBase(), rReq.GetArg<SfxInt32Item>(FN_PARAM_1));
            SdrTextObj* pTextObject = mpDrawView->GetTextEditObject();
            if (!pAskingShell || !pTextObject)
                break;
            auto itRequest = maTextEditRequests.find(KitHelper::getView(*pAskingShell));
            if (itRequest == maTextEditRequests.end()
                || itRequest->second != pTextObject->GetUniqueID())
                break;

            maTextEditRequests.erase(itRequest);
            KitHelper::notifyTextEditLocked(GetViewShellBase(), *pAskingShell, "rejected");
            rReq.Done();
        }
        break;

        case SID_RETURN_TEXT_EDIT:
        {
            // This view writes its text into the given object and leaves it. Then the given view
            // edits the object again. That is the view this view took the object over from, and
            // its user can still edit the document. A given view that edits some text by then
            // goes on with that text, and this view keeps the object.
            SfxViewShell* pOwnerShell
                = GetOtherViewShell(GetViewShellBase(), rReq.GetArg<SfxInt32Item>(FN_PARAM_1));
            DrawViewShell* pOwnerDrawShell = GetDrawViewShellOf(pOwnerShell);
            rtl::Reference<SdrTextObj> xTextObject = mpDrawView->GetTextEditObject();
            if (!pOwnerDrawShell || !xTextObject
                || !IsObjectOf(*xTextObject, rReq.GetArg<SfxStringItem>(FN_PARAM_2))
                || KitHelper::getView(*pOwnerShell) != mnTextEditTakenFromViewId
                || xTextObject->GetUniqueID() != mnTextEditTakenObjectId
                || pOwnerShell->IsKitReadOnlyView() || !ShowsPageOf(*pOwnerDrawShell, *xTextObject)
                || pOwnerDrawShell->GetView()->IsTextEdit())
                break;

            mpDrawView->SdrEndTextEdit();
            // The other view starts its edit as when its own user starts it.
            bool bOwnerEdits = false;
            RunAsView(*pOwnerShell, [&bOwnerEdits, pOwnerDrawShell, &xTextObject] {
                bOwnerEdits = BeginTextEditOf(*pOwnerDrawShell, *xTextObject);
            });
            if (!bOwnerEdits)
            {
                // This view edits the object again, when the object is still on its page.
                BeginTextEditOf(*this, *xTextObject);
                break;
            }

            mnTextEditTakenFromViewId = -1;
            mnTextEditTakenObjectId = 0;
            KitHelper::notifyTextEditLocked(GetViewShellBase(), *pOwnerShell, "returned");
            rReq.Done();
        }
        break;

        case SID_BEZIER_EDIT:
        {
            mpDrawView->SetFrameDragSingles(!mpDrawView->IsFrameDragSingles());

            /******************************************************************
            * turn ObjectBar on
            ******************************************************************/
            if( dynamic_cast< FuSelection* >( GetCurrentFunction().get() ) || dynamic_cast< FuConstructBezierPolygon* >( GetCurrentFunction().get() ) )
            {
                // Tell the tool bar manager about the context change.
                GetViewShellBase().GetToolBarManager()->SelectionHasChanged(*this,*mpDrawView);
            }

            Invalidate(SID_BEZIER_EDIT);
        }
        break;

        case SID_OBJECT_CLOSE:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if ( rMarkList.GetMark(0) && !mpDrawView->IsAction() )
            {
                SdrPathObj* pPathObj = static_cast<SdrPathObj*>( rMarkList.GetMark(0)->GetMarkedSdrObj());
                const bool bUndo = mpDrawView->IsUndoEnabled();
                if( bUndo )
                    mpDrawView->BegUndo(SdResId(STR_UNDO_BEZCLOSE));

                mpDrawView->UnmarkAllPoints();

                if( bUndo )
                    mpDrawView->AddUndo(std::make_unique<SdrUndoGeoObj>(*pPathObj));

                pPathObj->ToggleClosed();

                if( bUndo )
                    mpDrawView->EndUndo();
            }
            rReq.Done();
        }
        break;

        case SID_CUT:
        {
            if ( mpDrawView->IsPresObjSelected(false, true, false, true) )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                //tdf#126197: EndTextEdit in all views if current one is not in TextEdit
                if ( !mpDrawView->IsTextEdit() )
                    mpDrawView->EndTextEditAllViews();

                if(HasCurrentFunction())
                {
                    GetCurrentFunction()->DoCut();
                }
                else if(mpDrawView)
                {
                    mpDrawView->DoCut();
                }
            }
        }
        break;

        case SID_COPY:
        {
            if ( mpDrawView->IsPresObjSelected(false, true, false, true) )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                if(HasCurrentFunction())
                {
                    GetCurrentFunction()->DoCopy();
                }
                else if( mpDrawView )
                {
                    mpDrawView->DoCopy();
                }
            }
        }
        break;

        case SID_PASTE:
        {
            weld::WaitObject aWait(GetFrameWeld());

            if(HasCurrentFunction())
            {
                GetCurrentFunction()->DoPaste();
            }
            else if(mpDrawView)
            {
                mpDrawView->DoPaste();
            }

        }
        break;

        case SID_PASTE_SLIDE:
        case SID_COPY_SLIDE:
        {
            if (::sd::slidesorter::SlideSorterViewShell* pSlideSorterViewShell =
                sd::slidesorter::SlideSorterViewShell::GetSlideSorter(GetViewShellBase()))
                    pSlideSorterViewShell->GetSlideSorter().GetController().FuSupport(rReq);
            Cancel();
            rReq.Done();
        }
        break;

        case SID_UNICODE_NOTATION_TOGGLE:
        {
            if( mpDrawView->IsTextEdit() )
            {
                OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();
                if( pOLV )
                {
                    OUString sInput = pOLV->GetSurroundingText();
                    ESelection aSel( pOLV->GetSelection() );
                    if (aSel.start.nIndex > aSel.end.nIndex)
                        aSel.end.nIndex = aSel.start.nIndex;

                    //calculate a valid end-position by reading logical characters
                    sal_Int32 nUtf16Pos=0;
                    while ((nUtf16Pos < sInput.getLength()) && (nUtf16Pos < aSel.end.nIndex))
                    {
                        sInput.iterateCodePoints(&nUtf16Pos);
                        //The mouse can set the cursor in the middle of a multi-unit character,
                        //    so reset to the proper end of the logical characters
                        if (nUtf16Pos > aSel.end.nIndex)
                            aSel.end.nIndex = nUtf16Pos;
                    }

                    ToggleUnicodeCodepoint aToggle;
                    while( nUtf16Pos && aToggle.AllowMoreInput( sInput[nUtf16Pos-1]) )
                        --nUtf16Pos;
                    OUString sReplacement = aToggle.ReplacementString();
                    if( !sReplacement.isEmpty() )
                    {
                        OUString sStringToReplace = aToggle.StringToReplace();
                        mpDrawView->BegUndo(sStringToReplace +"->"+ sReplacement);
                        aSel.start.nIndex = aSel.end.nIndex - sStringToReplace.getLength();
                        pOLV->SetSelection( aSel );
                        pOLV->InsertText(sReplacement, true);
                        mpDrawView->EndUndo();
                    }
                }
            }
        }
        break;

        case SID_PASTE_UNFORMATTED:
        {
            weld::WaitObject aWait(GetFrameWeld());

            if(HasCurrentFunction())
            {
                GetCurrentFunction()->DoPasteUnformatted();
            }
            else if(mpDrawView)
            {
                TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
                if (aDataHelper.GetTransferable().is())
                {
                    sal_Int8 nAction = DND_ACTION_COPY;
                    mpDrawView->InsertData( aDataHelper,
                                            GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(), GetActiveWindow()->GetOutputSizePixel() ).Center() ),
                                            nAction, false, SotClipboardFormatId::STRING);
                }
            }

        }
        break;

        case SID_CLIPBOARD_FORMAT_ITEMS:
        {
            weld::WaitObject aWait(GetFrameWeld());
            TransferableDataHelper  aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( GetActiveWindow() ) );
            const SfxItemSet*       pReqArgs = rReq.GetArgs();
            SotClipboardFormatId    nFormat = SotClipboardFormatId::NONE;

            if( pReqArgs )
            {
                const SfxUInt32Item* pIsActive = rReq.GetArg<SfxUInt32Item>(SID_CLIPBOARD_FORMAT_ITEMS);
                nFormat = static_cast<SotClipboardFormatId>(pIsActive->GetValue());
            }

            if( nFormat != SotClipboardFormatId::NONE && aDataHelper.GetTransferable().is() )
            {
                sal_Int8 nAction = DND_ACTION_COPY;

                if( !mpDrawView->InsertData( aDataHelper,
                                          GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(), GetActiveWindow()->GetOutputSizePixel() ).Center() ),
                                          nAction, false, nFormat ) )
                {
                    INetBookmark    aINetBookmark( u""_ustr, u""_ustr );

                    if( ( aDataHelper.HasFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK ) &&
                          aDataHelper.GetINetBookmark( SotClipboardFormatId::NETSCAPE_BOOKMARK, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SotClipboardFormatId::FILEGRPDESCRIPTOR ) &&
                          aDataHelper.GetINetBookmark( SotClipboardFormatId::FILEGRPDESCRIPTOR, aINetBookmark ) ) ||
                        ( aDataHelper.HasFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR ) &&
                          aDataHelper.GetINetBookmark( SotClipboardFormatId::UNIFORMRESOURCELOCATOR, aINetBookmark ) ) )
                    {
                        InsertURLField(aINetBookmark.GetURL(), aINetBookmark.GetDescription(), u""_ustr, u""_ustr);
                    }
                }
            }
        }
        break;

        case SID_DELETE:
        {
            if ( mpDrawView->IsTextEdit() )
            {
                OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

                if (pOLV)
                {
                    vcl::KeyCode aKCode(KEY_DELETE);
                    KeyEvent aKEvt( 0, aKCode);
                    // We use SdrObjEditView to handle DEL for underflow handling
                    (void)mpDrawView->KeyInput(aKEvt, nullptr);
                }
            }
            else
            {
                mpDrawView->EndTextEditAllViews();

                // the request is done where every object that was selected is out of the
                // document after it, which a client that takes them away at once relies on; a
                // placeholder that is only emptied, or one that cannot go, leaves it undone
                std::vector<rtl::Reference<SdrObject>> aSelected;
                const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                for (size_t nMark = 0; nMark < rMarkList.GetMarkCount(); ++nMark)
                    aSelected.emplace_back(rMarkList.GetMark(nMark)->GetMarkedSdrObj());

                FuDeleteSelectedObjects();

                if (!aSelected.empty()
                    && std::none_of(aSelected.begin(), aSelected.end(),
                                    [](const rtl::Reference<SdrObject>& rObject)
                                    { return rObject->IsInserted(); }))
                    rReq.Done();
            }
        }
        break;

        case SID_NOTES_MODE:
        case SID_SLIDE_MASTER_MODE:
        case SID_NOTES_MASTER_MODE:
        case SID_HANDOUT_MASTER_MODE:

            // AutoLayouts have to be ready.
            GetDoc()->StopWorkStartupDelay();
            [[fallthrough]];

        case SID_DRAWINGMODE:
        case SID_SLIDE_SORTER_MODE:
        case SID_OUTLINE_MODE:
            // Let the sub-shell manager handle the slot handling.
            framework::FrameworkHelper::Instance(GetViewShellBase())->HandleModeChangeSlot(
                nSId,
                rReq);
            break;

        case SID_MASTERPAGE:          // BASIC
        {
            // AutoLayouts needs to be finished
            GetDoc()->StopWorkStartupDelay();

            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if ( pReqArgs )
            {
                const SfxBoolItem* pIsActive = rReq.GetArg<SfxBoolItem>(SID_MASTERPAGE);
                mbIsLayerModeActive = pIsActive->GetValue ();
            }

            Broadcast (
                ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_START));

            // turn on default layer of MasterPage
            mpDrawView->SetActiveLayer(sUNO_LayerName_background_objects);

            ChangeEditMode(EditMode::MasterPage, mbIsLayerModeActive);

            if(HasCurrentFunction(SID_BEZIER_EDIT))
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);

            Broadcast (
                ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_END));

            InvalidateWindows();
            Invalidate();

            rReq.Done();
        }
        break;

        case SID_CLOSE_MASTER_VIEW:
        {
            Broadcast (
                ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_START));

            // Switch page back to the first one.  Not doing so leads to a
            // crash.  This seems to be some bug in the edit mode switching
            // and page switching methods.
            SwitchPage (0);
            ChangeEditMode(EditMode::Page, IsLayerModeActive());
            Broadcast (
                ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_END));

            if(HasCurrentFunction(SID_BEZIER_EDIT))
            {
                GetViewFrame()->GetDispatcher()->Execute(
                    SID_OBJECT_SELECT,
                    SfxCallMode::ASYNCHRON);
            }

            rReq.Done();
        }
        break;

        case SID_RULER:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            // Remember old ruler state
            bool bOldHasRuler(HasRuler());

            if ( pReqArgs )
            {
                const SfxBoolItem* pIsActive = rReq.GetArg<SfxBoolItem>(SID_RULER);
                SetRuler (pIsActive->GetValue ());
            }
            else SetRuler (!HasRuler());

            // Did ruler state change? Tell that to SdOptions, too.
            bool bHasRuler(HasRuler());

            if(bOldHasRuler != bHasRuler)
            {
                std::shared_ptr<comphelper::ConfigurationChanges> batch(
                    comphelper::ConfigurationChanges::create());

                if (GetDoc()->GetDocumentType() == DocumentType::Impress)
                {
                    officecfg::Office::Impress::Layout::Display::Ruler::set(bHasRuler, batch);
                }
                else
                {
                    officecfg::Office::Draw::Layout::Display::Ruler::set(bHasRuler, batch);
                }

                batch->commit();
            }

            Invalidate (SID_RULER);
            Resize();
            rReq.Done ();
        }
        break;

        case SID_SIZE_PAGE:
        case SID_SIZE_PAGE_WIDTH:  // BASIC
        {
            mbZoomOnPage = ( rReq.GetSlot() == SID_SIZE_PAGE );

            SdrPageView* pPageView = mpDrawView->GetSdrPageView();

            if ( pPageView )
            {
                Point aPagePos(0, 0); // = pPageView->GetOffset();
                Size aPageSize = pPageView->GetPage()->GetSize();

                aPagePos.AdjustX(aPageSize.Width()  / 2 );
                aPageSize.setWidth( static_cast<::tools::Long>(aPageSize.Width() * 1.03) );

                if( rReq.GetSlot() == SID_SIZE_PAGE )
                {
                    aPagePos.AdjustY(aPageSize.Height() / 2 );
                    aPageSize.setHeight( static_cast<::tools::Long>(aPageSize.Height() * 1.03) );
                    aPagePos.AdjustY( -(aPageSize.Height() / 2) );
                }
                else
                {
                    Point aPt = GetActiveWindow()->PixelToLogic( Point( 0, GetActiveWindow()->GetSizePixel().Height() / 2 ) );
                    aPagePos.AdjustY(aPt.Y() );
                    aPageSize.setHeight( 2 );
                }

                aPagePos.AdjustX( -(aPageSize.Width()  / 2) );

                SetZoomRect( ::tools::Rectangle( aPagePos, aPageSize ) );

                ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
                mpZoomList->InsertZoomRect(aVisAreaWin);
            }
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            rReq.Done ();
        }
        break;

        case SID_SIZE_REAL:  // BASIC
        {
            mbZoomOnPage = false;
            SetZoom( 100 );
            ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            rReq.Done ();
        }
        break;

        case SID_ZOOM_OUT:  // BASIC
        {
            const sal_uInt16 nOldZoom = GetActiveWindow()->GetZoom();
            const sal_uInt16 nNewZoom = basegfx::zoomtools::zoomOut(nOldZoom);
            SetZoom(nNewZoom);

            mbZoomOnPage = false;
            ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            rReq.Done ();
        }
        break;

        case SID_ZOOM_IN:
        {
            const sal_uInt16 nOldZoom = GetActiveWindow()->GetZoom();
            const sal_uInt16 nNewZoom = basegfx::zoomtools::zoomIn(nOldZoom);
            SetZoom(nNewZoom);

            mbZoomOnPage = false;
            ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
            mpZoomList->InsertZoomRect(aVisAreaWin);
            Invalidate( SID_ZOOM_IN );
            Invalidate(SID_ZOOM_OUT);
            Invalidate( SID_ZOOM_PANNING );
            rReq.Done ();
        }
        break;

        case SID_SIZE_VISAREA:
        {
            ::tools::Rectangle aVisArea = mpFrameView->GetVisArea();
            Size aVisAreaSize = aVisArea.GetSize();

            if (!aVisAreaSize.IsEmpty())
            {
                mbZoomOnPage = false;
                SetZoomRect(aVisArea);
                Invalidate( SID_ZOOM_IN );
                Invalidate( SID_ZOOM_OUT );
                Invalidate( SID_ZOOM_PANNING );
            }
            rReq.Done ();
        }
        break;

        // name confusion: SID_SIZE_OPTIMAL -> Zoom onto selected objects
        // --> Is offered as object zoom in program
        case SID_SIZE_OPTIMAL:  // BASIC
        {
            mbZoomOnPage = false;
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if ( rMarkList.GetMarkCount() != 0 )
            {
                maMarkRect = mpDrawView->GetAllMarkedRect();
                ::tools::Long nW = static_cast<::tools::Long>(maMarkRect.GetWidth()  * 1.03);
                ::tools::Long nH = static_cast<::tools::Long>(maMarkRect.GetHeight() * 1.03);
                Point aPos = maMarkRect.Center();
                aPos.AdjustX( -(nW / 2) );
                aPos.AdjustY( -(nH / 2) );
                if ( nW && nH )
                {
                    SetZoomRect(::tools::Rectangle(aPos, Size(nW, nH)));

                    ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
                    mpZoomList->InsertZoomRect(aVisAreaWin);
                }
            }
            Invalidate( SID_ZOOM_IN );
            Invalidate( SID_ZOOM_OUT );
            Invalidate( SID_ZOOM_PANNING );
            rReq.Done ();
        }
        break;

        // name confusion: SID_SIZE_ALL -> Zoom onto all objects
        // --> Is offered as optimal in program
        case SID_SIZE_ALL:  // BASIC
        {
            mbZoomOnPage = false;
            SdrPageView* pPageView = mpDrawView->GetSdrPageView();

            if( pPageView )
            {
                ::tools::Rectangle aBoundRect( pPageView->GetObjList()->GetAllObjBoundRect() );

                ::tools::Long nW = static_cast<::tools::Long>(aBoundRect.GetWidth() * 1.03);
                ::tools::Long nH = static_cast<::tools::Long>(aBoundRect.GetHeight() * 1.03);
                Point aPos = aBoundRect.Center();
                aPos.AdjustX( -(nW / 2) );
                aPos.AdjustY( -(nH / 2) );
                if ( nW && nH )
                {
                    SetZoomRect( ::tools::Rectangle( aPos, Size( nW, nH ) ) );

                    ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(0,0),
                                              GetActiveWindow()->GetOutputSizePixel()) );
                    mpZoomList->InsertZoomRect(aVisAreaWin);
                }

                Invalidate( SID_ZOOM_IN );
                Invalidate( SID_ZOOM_OUT );
                Invalidate( SID_ZOOM_PANNING );
            }
            rReq.Done ();
        }
        break;

        case SID_ZOOM_PREV:
        {
            if (mpDrawView->IsTextEdit())
            {
                mpDrawView->SdrEndTextEdit();
            }

            if (mpZoomList->IsPreviousPossible())
            {
                // set previous ZoomRect
                SetZoomRect(mpZoomList->GetPreviousZoomRect());
            }
            rReq.Done ();
        }
        break;

        case SID_ZOOM_NEXT:
        {
            if (mpDrawView->IsTextEdit())
            {
                mpDrawView->SdrEndTextEdit();
            }

            if (mpZoomList->IsNextPossible())
            {
                // set next ZoomRect
                SetZoomRect(mpZoomList->GetNextZoomRect());
            }
            rReq.Done ();
        }
        break;

        case SID_GLUE_INSERT_POINT:
        case SID_GLUE_PERCENT:
        case SID_GLUE_ESCDIR:
        case SID_GLUE_ESCDIR_LEFT:
        case SID_GLUE_ESCDIR_RIGHT:
        case SID_GLUE_ESCDIR_TOP:
        case SID_GLUE_ESCDIR_BOTTOM:
        case SID_GLUE_HORZALIGN_CENTER:
        case SID_GLUE_HORZALIGN_LEFT:
        case SID_GLUE_HORZALIGN_RIGHT:
        case SID_GLUE_VERTALIGN_CENTER:
        case SID_GLUE_VERTALIGN_TOP:
        case SID_GLUE_VERTALIGN_BOTTOM:
        {
            rtl::Reference<FuPoor> xFunc( GetCurrentFunction() );
            FuEditGluePoints* pFunc = dynamic_cast< FuEditGluePoints* >( xFunc.get() );

            if(pFunc)
                pFunc->ReceiveRequest(rReq);

            rReq.Done();
        }
        break;

        case SID_AUTOSPELL_CHECK:
        {
            bool bOnlineSpell;
            const SfxPoolItem* pItem;
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (pArgs && pArgs->HasItem(FN_PARAM_1, &pItem))
                bOnlineSpell = static_cast<const SfxBoolItem*>(pItem)->GetValue();
            else // Toggle
                bOnlineSpell = !GetDoc()->GetOnlineSpell();

            GetDoc()->SetOnlineSpell(bOnlineSpell);
            SdModule::SetAutoSpellProperty(bOnlineSpell, GetDoc()->GetDocumentType());

            ::Outliner* pOL = mpDrawView->GetTextEditOutliner();

            if (pOL)
            {
                EEControlBits nCntrl = pOL->GetControlWord();

                if (bOnlineSpell)
                    nCntrl |= EEControlBits::ONLINESPELLING;
                else
                    nCntrl &= ~EEControlBits::ONLINESPELLING;

                pOL->SetControlWord(nCntrl);
            }

            GetActiveWindow()->Invalidate();
            rReq.Done ();
        }
        break;

        case SID_TRANSLITERATE_SENTENCE_CASE:
        case SID_TRANSLITERATE_TITLE_CASE:
        case SID_TRANSLITERATE_TOGGLE_CASE:
        case SID_TRANSLITERATE_UPPER:
        case SID_TRANSLITERATE_LOWER:
        case SID_TRANSLITERATE_HALFWIDTH:
        case SID_TRANSLITERATE_FULLWIDTH:
        case SID_TRANSLITERATE_HIRAGANA:
        case SID_TRANSLITERATE_KATAKANA:
        {
            OutlinerView* pOLV = GetView()->GetTextEditOutlinerView();
            if( pOLV )
            {
                TransliterationFlags nType = TransliterationFlags::NONE;

                switch( nSId )
                {
                    case SID_TRANSLITERATE_SENTENCE_CASE:
                        nType = TransliterationFlags::SENTENCE_CASE;
                        break;
                    case SID_TRANSLITERATE_TITLE_CASE:
                        nType = TransliterationFlags::TITLE_CASE;
                        break;
                    case SID_TRANSLITERATE_TOGGLE_CASE:
                        nType = TransliterationFlags::TOGGLE_CASE;
                        break;
                    case SID_TRANSLITERATE_UPPER:
                        nType = TransliterationFlags::LOWERCASE_UPPERCASE;
                        break;
                    case SID_TRANSLITERATE_LOWER:
                        nType = TransliterationFlags::UPPERCASE_LOWERCASE;
                        break;
                    case SID_TRANSLITERATE_HALFWIDTH:
                        nType = TransliterationFlags::FULLWIDTH_HALFWIDTH;
                        break;
                    case SID_TRANSLITERATE_FULLWIDTH:
                        nType = TransliterationFlags::HALFWIDTH_FULLWIDTH;
                        break;
                    case SID_TRANSLITERATE_HIRAGANA:
                        nType = TransliterationFlags::KATAKANA_HIRAGANA;
                        break;
                    case SID_TRANSLITERATE_KATAKANA:
                        nType = TransliterationFlags::HIRAGANA_KATAKANA;
                        break;
                }

                pOLV->TransliterateText( nType );
            }

            rReq.Done();
        }
        break;

        // #UndoRedo#
        case SID_UNDO :
        {
            // moved implementation to BaseClass
            ImpSidUndo(rReq);
        }
        break;
        case SID_REDO :
        {
            // moved implementation to BaseClass
            ImpSidRedo(rReq);
        }
        break;

        default:
        break;
    }
}

void DrawViewShell::FuSupportRotate(SfxRequest const &rReq)
{
    if( rReq.GetSlot() != SID_TRANSLITERATE_ROTATE_CASE )
        return;

    ::sd::View* pView = GetView();

    if (!pView)
        return;

    OutlinerView* pOLV = pView->GetTextEditOutlinerView();

    if (!pOLV)
        return;
    TransliterationFlags transFlags = m_aRotateCase.getNextMode();
    if (TransliterationFlags::SENTENCE_CASE == transFlags)
    {
        OUString SelectedText = pOLV->GetSelected().trim();
        if (SelectedText.getLength() <= 2 || (SelectedText.indexOf(' ') < 0 && SelectedText.indexOf('\t') < 0))
            transFlags = m_aRotateCase.getNextMode();
    }
    pOLV->TransliterateText( transFlags );
}

void DrawViewShell::InsertURLField(const OUString& rURL, const OUString& rText,
                                   const OUString& rTarget, OUString const& rAltText,
                                   OUString const& rTitle)
{
    OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

    SvxURLField aURLField(rURL, rText, SvxURLFormat::Repr);
    aURLField.SetTargetFrame(rTarget);
    aURLField.SetName(rAltText);
    aURLField.SetTitle(rTitle);

    if (pOLV)
    {
        SvxFieldItem const aURLItem(aURLField, EE_FEATURE_FIELD);
        pOLV->InsertFieldAndSelect( aURLItem );
    }
    else
    {
        Outliner* pOutl = GetDoc()->GetInternalOutliner();
        pOutl->Init( OutlinerMode::TextObject );
        OutlinerMode nOutlMode = pOutl->GetOutlinerMode();

        pOutl->QuickInsertUrlField(aURLField, ESelection());
        std::optional<OutlinerParaObject> pOutlParaObject = pOutl->CreateParaObject();

        rtl::Reference<SdrRectObj> pRectObj = new SdrRectObj(
            GetView()->getSdrModelFromSdrView(), ::tools::Rectangle(), SdrObjKind::Text);

        pOutl->UpdateFields();
        pOutl->SetUpdateLayout( true );
        Size aSize(pOutl->CalcTextSize());
        pOutl->SetUpdateLayout( false );

        Point aPos;
        ::tools::Rectangle aRect(aPos, GetActiveWindow()->GetOutputSizePixel() );
        aPos = aRect.Center();
        aPos = GetActiveWindow()->PixelToLogic(aPos);

        if (aPos.getX() - (aSize.Width() / 2) >= 0)
            aPos.AdjustX( -(aSize.Width() / 2) );
        if (aPos.getY() - (aSize.Height() / 2) >= 0)
            aPos.AdjustY( -(aSize.Height() / 2) );

        ::tools::Rectangle aLogicRect(aPos, aSize);
        pRectObj->SetLogicRect(aLogicRect);
        pRectObj->SetOutlinerParaObject( std::move(pOutlParaObject) );
        mpDrawView->InsertObjectAtView(pRectObj.get(), *mpDrawView->GetSdrPageView());
        pOutl->Init( nOutlMode );
    }
}

void DrawViewShell::InsertURLButton(const OUString& rURL, const OUString& rText,
                                    const OUString& rTarget, const Point* pPos)
{
    bool bNewObj = true;

    const OUString sTargetURL( ::URIHelper::SmartRel2Abs( INetURLObject( GetDocSh()->GetMedium()->GetBaseURL() ), rURL, URIHelper::GetMaybeFileHdl(), true, false,
                                                                INetURLObject::EncodeMechanism::WasEncoded,
                                                                INetURLObject::DecodeMechanism::Unambiguous ) );
    const SdrMarkList&  rMarkList = mpDrawView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() > 0)
    {
        SdrObject* pMarkedObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        if( pMarkedObj ) try
        {
            // change first marked object
            if (SdrInventor::FmForm == pMarkedObj->GetObjInventor())
            {
                SdrUnoObj* pUnoCtrl = static_cast< SdrUnoObj* >( pMarkedObj );

                Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel(), UNO_SET_THROW );
                Reference< beans::XPropertySet > xPropSet( xControlModel, UNO_QUERY_THROW );

                bool bIsButton = pMarkedObj->GetObjIdentifier() == SdrObjKind::FormButton;
                if (!bIsButton && pMarkedObj->GetObjIdentifier() == SdrObjKind::UNO)
                {
                    const Reference<beans::XPropertySetInfo> xInfo(xPropSet->getPropertySetInfo());
                    bIsButton = xInfo.is() && xInfo->hasPropertyByName(u"ButtonType"_ustr)
                                && xInfo->hasPropertyByName(u"Label"_ustr)
                                && xInfo->hasPropertyByName(u"TargetURL"_ustr);
                }
                if (bIsButton)
                {
                    bNewObj = false;

                    xPropSet->setPropertyValue(u"Label"_ustr, Any(rText));
                    xPropSet->setPropertyValue(u"TargetURL"_ustr, Any(sTargetURL));

                    if (!rTarget.isEmpty())
                        xPropSet->setPropertyValue(u"TargetFrame"_ustr, Any(rTarget));

                    xPropSet->setPropertyValue(u"ButtonType"_ustr, Any(form::FormButtonType_URL));
#if HAVE_FEATURE_AVMEDIA
                    if (::avmedia::MediaWindow::isMediaURL(rURL, u""_ustr/*TODO?*/))
                    {
                        xPropSet->setPropertyValue(u"DispatchURLInternal"_ustr, Any(true));
                    }
#endif
                }
            }
            if (bNewObj)
            {
                // add url as interaction for first selected shape
                bNewObj = false;

                SdAnimationInfo* pInfo = SdDrawDocument::GetShapeUserData(*pMarkedObj, true);
                pInfo->meClickAction = presentation::ClickAction_DOCUMENT;
                pInfo->SetBookmark( sTargetURL );
                pMarkedObj->setHyperlink(rURL);
            }
        }
        catch( cpo::uno::Exception& )
        {
        }
    }

    if (!bNewObj)
        return;

    try
    {
        rtl::Reference<SdrUnoObj> pUnoCtrl = static_cast< SdrUnoObj* >(
            SdrObjFactory::MakeNewObject(
                GetView()->getSdrModelFromSdrView(),
                SdrInventor::FmForm,
                SdrObjKind::FormButton).get()); //,
                //mpDrawView->GetSdrPageView()->GetPage()));

        Reference< awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel(), uno::UNO_SET_THROW );
        Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY_THROW );

        xPropSet->setPropertyValue( u"Label"_ustr , Any( rText ) );
        xPropSet->setPropertyValue( u"TargetURL"_ustr , Any( sTargetURL ) );

        if( !rTarget.isEmpty() )
            xPropSet->setPropertyValue( u"TargetFrame"_ustr , Any( rTarget ) );

        xPropSet->setPropertyValue( u"ButtonType"_ustr , Any(  form::FormButtonType_URL ) );
#if HAVE_FEATURE_AVMEDIA
        if ( ::avmedia::MediaWindow::isMediaURL( rURL, u""_ustr/*TODO?*/ ) )
            xPropSet->setPropertyValue( u"DispatchURLInternal"_ustr , Any( true ) );
#endif

        Point aPos;

        if (pPos)
        {
            aPos = *pPos;
        }
        else
        {
            aPos = ::tools::Rectangle(aPos, GetActiveWindow()->GetOutputSizePixel()).Center();
            aPos = GetActiveWindow()->PixelToLogic(aPos);
        }

        Size aSize(4000, 1000);
        aPos.AdjustX( -(aSize.Width() / 2) );
        aPos.AdjustY( -(aSize.Height() / 2) );
        pUnoCtrl->SetLogicRect(::tools::Rectangle(aPos, aSize));

        SdrInsertFlags nOptions = SdrInsertFlags::SETDEFLAYER;

        OSL_ASSERT (GetViewShell()!=nullptr);
        SfxInPlaceClient* pIpClient = GetViewShell()->GetIPClient();
        if (pIpClient!=nullptr && pIpClient->IsObjectInPlaceActive())
        {
            nOptions |= SdrInsertFlags::DONTMARK;
        }

        mpDrawView->InsertObjectAtView(pUnoCtrl.get(), *mpDrawView->GetSdrPageView(), nOptions);
    }
    catch( cpo::uno::Exception& )
    {
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
