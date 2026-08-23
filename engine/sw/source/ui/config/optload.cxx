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

#include <officecfg/Office/Writer.hxx>
#include <comphelper/string.hxx>
#include <swtypes.hxx>
#include <uiitems.hxx>
#include <modcfg.hxx>
#include <swmodule.hxx>
#include <usrpref.hxx>
#include <wrtsh.hxx>
#include <linkenum.hxx>
#include <uitool.hxx>
#include <view.hxx>

#include <strings.hrc>
#include <optload.hrc>
#include <cmdid.h>

#include <optload.hxx>
#include <sfx2/htmlmode.hxx>
#include <fldmgr.hxx>
#include <poolfmt.hxx>
#include <expfld.hxx>

#include <svtools/insdlg.hxx>
#include <svtools/unitconv.hxx>
#include <tools/resary.hxx>
#include <vcl/settings.hxx>

#include <comphelper/classids.hxx>
#include <unotools/configmgr.hxx>
#include <SwStyleNameMapper.hxx>
#include <numrule.hxx>

#include <doc.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/eitem.hxx>

using namespace ::com::sun::star;

static bool lcl_isPropertyReadOnly(const SwCapObjType eType, const CapConfigProp ePropType, const SvGlobalName& rOleId)
{
    bool bReadOnly = false;

    switch (ePropType)
    {
        case PROP_CAP_OBJECT_ENABLE:
        {
            switch (eType)
            {
                case TABLE_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Table::Enable::isReadOnly();
                    break;
                case FRAME_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Frame::Enable::isReadOnly();
                    break;
                case GRAPHIC_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Graphic::Enable::isReadOnly();
                    break;
                case OLE_CAP:
                {
                    if (rOleId == SvGlobalName(SO3_SC_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Calc::Enable::isReadOnly();
                    } else if (rOleId == SvGlobalName(SO3_SIMPRESS_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Impress::Enable::isReadOnly();
                    } else if (rOleId == SvGlobalName(SO3_SCH_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Chart::Enable::isReadOnly();
                    } else if (rOleId == SvGlobalName(SO3_SM_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Formula::Enable::isReadOnly();
                    } else if (rOleId == SvGlobalName(SO3_SDRAW_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Draw::Enable::isReadOnly();
                    } else if (rOleId == SvGlobalName(SO3_OUT_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::OLEMisc::Enable::isReadOnly();
                    } else {
                        // Should not reach it.
                    }
                }
                break;
                default:
                    break;
            }
        }
        break;
        case PROP_CAP_OBJECT_CATEGORY:
        {
            switch (eType)
            {
                case TABLE_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Table::Settings::Category::isReadOnly();
                    break;
                case FRAME_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Frame::Settings::Category::isReadOnly();
                    break;
                case GRAPHIC_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Graphic::Settings::Category::isReadOnly();
                    break;
                case OLE_CAP:
                {
                    if (rOleId == SvGlobalName(SO3_SC_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Calc::Settings::Category::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SIMPRESS_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Impress::Settings::Category::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SCH_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Chart::Settings::Category::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SM_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Formula::Settings::Category::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SDRAW_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Draw::Settings::Category::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_OUT_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::OLEMisc::Settings::Category::isReadOnly();
                    }
                    else {
                        // Should not reach it.
                    }
                }
                break;
                default:
                    break;
            }
        }
        break;
        case PROP_CAP_OBJECT_NUMBERING:
        {
            switch (eType)
            {
                case TABLE_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Table::Settings::Numbering::isReadOnly();
                    break;
                case FRAME_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Frame::Settings::Numbering::isReadOnly();
                    break;
                case GRAPHIC_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Graphic::Settings::Numbering::isReadOnly();
                    break;
                case OLE_CAP:
                {
                    if (rOleId == SvGlobalName(SO3_SC_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Calc::Settings::Numbering::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SIMPRESS_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Impress::Settings::Numbering::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SCH_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Chart::Settings::Numbering::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SM_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Formula::Settings::Numbering::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SDRAW_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Draw::Settings::Numbering::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_OUT_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::OLEMisc::Settings::Numbering::isReadOnly();
                    }
                    else {
                        // Should not reach it.
                    }
                }
                break;
                default:
                    break;
            }
        }
        break;
        case PROP_CAP_OBJECT_NUMBERINGSEPARATOR:
        {
            switch (eType)
            {
                case TABLE_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Table::Settings::NumberingSeparator::isReadOnly();
                    break;
                case FRAME_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Frame::Settings::NumberingSeparator::isReadOnly();
                    break;
                case GRAPHIC_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Graphic::Settings::NumberingSeparator::isReadOnly();
                    break;
                case OLE_CAP:
                {
                    if (rOleId == SvGlobalName(SO3_SC_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Calc::Settings::NumberingSeparator::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SIMPRESS_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Impress::Settings::NumberingSeparator::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SCH_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Chart::Settings::NumberingSeparator::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SM_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Formula::Settings::NumberingSeparator::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SDRAW_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Draw::Settings::NumberingSeparator::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_OUT_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::OLEMisc::Settings::NumberingSeparator::isReadOnly();
                    }
                    else {
                        // Should not reach it.
                    }
                }
                break;
                default:
                    break;
            }
        }
        break;
        case PROP_CAP_OBJECT_CAPTIONTEXT:
        {
            switch (eType)
            {
                case TABLE_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Table::Settings::CaptionText::isReadOnly();
                    break;
                case FRAME_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Frame::Settings::CaptionText::isReadOnly();
                    break;
                case GRAPHIC_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Graphic::Settings::CaptionText::isReadOnly();
                    break;
                case OLE_CAP:
                {
                    if (rOleId == SvGlobalName(SO3_SC_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Calc::Settings::CaptionText::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SIMPRESS_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Impress::Settings::CaptionText::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SCH_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Chart::Settings::CaptionText::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SM_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Formula::Settings::CaptionText::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SDRAW_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Draw::Settings::CaptionText::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_OUT_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::OLEMisc::Settings::CaptionText::isReadOnly();
                    }
                    else {
                        // Should not reach it.
                    }
                }
                break;
                default:
                    break;
            }
        }
        break;
        case PROP_CAP_OBJECT_DELIMITER:
        {
            switch (eType)
            {
                case TABLE_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Table::Settings::Delimiter::isReadOnly();
                    break;
                case FRAME_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Frame::Settings::Delimiter::isReadOnly();
                    break;
                case GRAPHIC_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Graphic::Settings::Delimiter::isReadOnly();
                    break;
                case OLE_CAP:
                {
                    if (rOleId == SvGlobalName(SO3_SC_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Calc::Settings::Delimiter::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SIMPRESS_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Impress::Settings::Delimiter::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SCH_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Chart::Settings::Delimiter::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SM_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Formula::Settings::Delimiter::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SDRAW_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Draw::Settings::Delimiter::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_OUT_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::OLEMisc::Settings::Delimiter::isReadOnly();
                    }
                    else {
                        // Should not reach it.
                    }
                }
                break;
                default:
                    break;
            }
        }
        break;
        case PROP_CAP_OBJECT_LEVEL:
        {
            switch (eType)
            {
                case TABLE_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Table::Settings::Level::isReadOnly();
                    break;
                case FRAME_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Frame::Settings::Level::isReadOnly();
                    break;
                case GRAPHIC_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Graphic::Settings::Level::isReadOnly();
                    break;
                case OLE_CAP:
                {
                    if (rOleId == SvGlobalName(SO3_SC_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Calc::Settings::Level::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SIMPRESS_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Impress::Settings::Level::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SCH_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Chart::Settings::Level::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SM_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Formula::Settings::Level::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SDRAW_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Draw::Settings::Level::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_OUT_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::OLEMisc::Settings::Level::isReadOnly();
                    }
                    else {
                        // Should not reach it.
                    }
                }
                break;
                default:
                    break;
            }
        }
        break;
        case PROP_CAP_OBJECT_POSITION:
        {
            switch (eType)
            {
                case TABLE_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Table::Settings::Position::isReadOnly();
                    break;
                case FRAME_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Frame::Settings::Position::isReadOnly();
                    break;
                case GRAPHIC_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Graphic::Settings::Position::isReadOnly();
                    break;
                case OLE_CAP:
                {
                    if (rOleId == SvGlobalName(SO3_SC_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Calc::Settings::Position::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SIMPRESS_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Impress::Settings::Position::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SCH_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Chart::Settings::Position::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SM_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Formula::Settings::Position::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SDRAW_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Draw::Settings::Position::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_OUT_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::OLEMisc::Settings::Position::isReadOnly();
                    }
                    else {
                        // Should not reach it.
                    }
                }
                break;
                default:
                    break;
            }
        }
        break;
        case PROP_CAP_OBJECT_CHARACTERSTYLE:
        {
            switch (eType)
            {
                case TABLE_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Table::Settings::CharacterStyle::isReadOnly();
                    break;
                case FRAME_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Frame::Settings::CharacterStyle::isReadOnly();
                    break;
                case GRAPHIC_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Graphic::Settings::CharacterStyle::isReadOnly();
                    break;
                case OLE_CAP:
                {
                    if (rOleId == SvGlobalName(SO3_SC_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Calc::Settings::CharacterStyle::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SIMPRESS_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Impress::Settings::CharacterStyle::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SCH_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Chart::Settings::CharacterStyle::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SM_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Formula::Settings::CharacterStyle::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SDRAW_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Draw::Settings::CharacterStyle::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_OUT_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::OLEMisc::Settings::CharacterStyle::isReadOnly();
                    }
                    else {
                        // Should not reach it.
                    }
                }
                break;
                default:
                    break;
            }
        }
        break;
        case PROP_CAP_OBJECT_APPLYATTRIBUTES:
        {
            switch (eType)
            {
                case GRAPHIC_CAP:
                    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Graphic::Settings::ApplyAttributes::isReadOnly();
                    break;
                case OLE_CAP:
                {
                    if (rOleId == SvGlobalName(SO3_SC_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Calc::Settings::ApplyAttributes::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SIMPRESS_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Impress::Settings::ApplyAttributes::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SCH_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Chart::Settings::ApplyAttributes::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SM_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Formula::Settings::ApplyAttributes::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_SDRAW_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Draw::Settings::ApplyAttributes::isReadOnly();
                    }
                    else if (rOleId == SvGlobalName(SO3_OUT_CLASSID)) {
                        bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::OLEMisc::Settings::ApplyAttributes::isReadOnly();
                    }
                    else {
                        // Should not reach it.
                    }
                }
                break;
                default:
                    break;
            }
        }
        break;
        default:
            break;
    }

    return bReadOnly;
}

SwCaptionOptDlg::SwCaptionOptDlg(weld::Window* pParent, const SfxItemSet& rSet)
    : SfxSingleTabDialogController(pParent, &rSet, u"modules/swriter/ui/captiondialog.ui"_ustr,
                                   u"CaptionDialog"_ustr)
{
    // create TabPage
    SetTabPage(SwCaptionOptPage::Create(get_content_area(), this, &rSet));
}

SwCaptionPreview::SwCaptionPreview()
    : mbFontInitialized(false)
{
}

void SwCaptionPreview::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();
    Wallpaper aBack(rSettings.GetWindowColor());
    rRenderContext.SetBackground(aBack);
    rRenderContext.SetFillColor(aBack.GetColor());
    rRenderContext.SetLineColor(aBack.GetColor());
    rRenderContext.SetTextColor(rSettings.GetWindowTextColor());

    if (!mbFontInitialized)
    {
        maFont = rRenderContext.GetFont();
        maFont.SetFontHeight(maFont.GetFontHeight() * 120 / 100);
        mbFontInitialized = true;
    }
    rRenderContext.SetFont(maFont);
}

void SwCaptionPreview::SetPreviewText(const OUString& rText)
{
    if (rText != maText)
    {
        maText = rText;
        Invalidate();
    }
}

void SwCaptionPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(106 , 20), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
}

void SwCaptionPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    ApplySettings(rRenderContext);

    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), GetOutputSizePixel()));
    rRenderContext.DrawText(Point(4, 6), maText);
}

IMPL_LINK(SwCaptionOptPage, TextFilterHdl, OUString&, rTest, bool)
{
    rTest = m_aTextFilter.filter(rTest);
    return true;
}

SwCaptionOptPage::SwCaptionOptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/optcaptionpage.ui"_ustr, u"OptCaptionPage"_ustr, &rSet)
    , m_sSWTable(SwResId(STR_CAPTION_TABLE))
    , m_sSWFrame(SwResId(STR_CAPTION_FRAME))
    , m_sSWGraphic(SwResId(STR_CAPTION_GRAPHIC))
    , m_sOLE(SwResId(STR_CAPTION_OLE))
    , m_sBegin(SwResId(STR_CAPTION_BEGINNING))
    , m_sEnd(SwResId(STR_CAPTION_END))
    , m_sAbove(SwResId(STR_CAPTION_ABOVE))
    , m_sBelow(SwResId(STR_CAPTION_BELOW))
    , m_sNone(SwResId(SW_STR_NONE))
    , m_nPrevSelectedEntry(-1)
    , m_pMgr(new SwFieldMgr)
    , m_bHTMLMode(false)
    , m_aTextFilter(m_sNone)
    , m_xCheckLB(m_xBuilder->weld_tree_view(u"objects"_ustr))
    , m_xLbCaptionOrder(m_xBuilder->weld_combo_box(u"captionorder"_ustr))
    , m_xLbCaptionOrderImg(m_xBuilder->weld_widget(u"lockcaptionorder"_ustr))
    , m_xSettingsGroup(m_xBuilder->weld_widget(u"settings"_ustr))
    , m_xCategoryBox(m_xBuilder->weld_combo_box(u"category"_ustr))
    , m_xCategoryBoxImg(m_xBuilder->weld_widget(u"lockcategory"_ustr))
    , m_xFormatText(m_xBuilder->weld_label(u"numberingft"_ustr))
    , m_xFormatBox(m_xBuilder->weld_combo_box(u"numbering"_ustr))
    , m_xFormatBoxImg(m_xBuilder->weld_widget(u"locknumbering"_ustr))
    , m_xNumberingSeparatorFT(m_xBuilder->weld_label(u"numseparatorft"_ustr))
    , m_xNumberingSeparatorED(m_xBuilder->weld_entry(u"numseparator"_ustr))
    , m_xNumberingSeparatorImg(m_xBuilder->weld_widget(u"locknumseparator"_ustr))
    , m_xTextText(m_xBuilder->weld_label(u"separatorft"_ustr))
    , m_xTextEdit(m_xBuilder->weld_entry(u"separator"_ustr))
    , m_xTextEditImg(m_xBuilder->weld_widget(u"lockseparator"_ustr))
    , m_xPosBox(m_xBuilder->weld_combo_box(u"position"_ustr))
    , m_xPosBoxImg(m_xBuilder->weld_widget(u"lockposition"_ustr))
    , m_xNumCapt(m_xBuilder->weld_widget(u"numcaption"_ustr))
    , m_xLbLevel(m_xBuilder->weld_combo_box(u"level"_ustr))
    , m_xLbLevelImg(m_xBuilder->weld_widget(u"locklevel"_ustr))
    , m_xEdDelim(m_xBuilder->weld_entry(u"chapseparator"_ustr))
    , m_xEdDelimImg(m_xBuilder->weld_widget(u"lockchapseparator"_ustr))
    , m_xCategory(m_xBuilder->weld_widget(u"categoryformat"_ustr))
    , m_xCharStyleLB(m_xBuilder->weld_combo_box(u"charstyle"_ustr))
    , m_xCharStyleImg(m_xBuilder->weld_widget(u"lockcharstyle"_ustr))
    , m_xApplyBorderCB(m_xBuilder->weld_check_button(u"applyborder"_ustr))
    , m_xApplyBorderImg(m_xBuilder->weld_widget(u"lockapplyborder"_ustr))
    , m_xPreview(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, m_aPreview))
{
    m_xCategoryBox->connect_entry_insert_text(LINK(this, SwCaptionOptPage, TextFilterHdl));

    m_xCheckLB->enable_toggle_buttons(weld::ColumnToggleType::Check);

    SwStyleNameMapper::FillUIName(SwPoolFormatId::COLL_LABEL_ABB, m_sIllustration);
    SwStyleNameMapper::FillUIName(SwPoolFormatId::COLL_LABEL_TABLE, m_sTable);
    SwStyleNameMapper::FillUIName(SwPoolFormatId::COLL_LABEL_FRAME, m_sText);
    SwStyleNameMapper::FillUIName(SwPoolFormatId::COLL_LABEL_DRAWING, m_sDrawing);


    // m_xFormatBox
    sal_uInt16 nSelFormat = SVX_NUM_ARABIC;
    SwWrtShell* pSh = ::GetActiveWrtShell();

    if (pSh)
    {
        for ( auto i = m_pMgr->GetFieldTypeCount(); i; )
        {
            SwFieldType* pFieldType = m_pMgr->GetFieldType(SwFieldIds::Unknown, --i);
            if (!pFieldType->GetName().isEmpty()
                && pFieldType->GetName() == m_xCategoryBox->get_active_text())
            {
                nSelFormat = o3tl::narrowing<sal_uInt16>(static_cast<SwSetExpFieldType*>(pFieldType)->GetSeqFormat());
                break;
            }
        }

        ::FillCharStyleListBox( *m_xCharStyleLB, pSh->GetView().GetDocShell(), true, true );
    }

    const sal_uInt16 nCount = m_pMgr->GetFormatCount(SwFieldTypesEnum::Sequence, false);
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        const sal_uInt16 nFormatId = m_pMgr->GetFormatId(SwFieldTypesEnum::Sequence, i);
        m_xFormatBox->append(OUString::number(nFormatId), m_pMgr->GetFormatStr(SwFieldTypesEnum::Sequence, i));
        if (nFormatId == nSelFormat)
            m_xFormatBox->set_active(i);
    }

    for (int i = 0; i < MAXLEVEL; ++i)
        m_xLbLevel->append_text(OUString::number(i + 1));

    sal_Unicode nLvl = MAXLEVEL;
    OUString sDelim(u": "_ustr);

    if (pSh)
    {
        SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(m_pMgr->GetFieldType(
                                            SwFieldIds::SetExp, m_xCategoryBox->get_active_text() ));
        if( pFieldType )
        {
            sDelim = pFieldType->GetDelimiter();
            nLvl = pFieldType->GetOutlineLvl();
        }
    }

    m_xLbLevel->set_active(nLvl < MAXLEVEL ? nLvl + 1 : 0);
    m_xEdDelim->set_text(sDelim);

    m_xCategoryBox->connect_changed(LINK(this, SwCaptionOptPage, ModifyComboHdl));
    Link<weld::Entry&,void> aLk = LINK(this, SwCaptionOptPage, ModifyEntryHdl);
    m_xNumberingSeparatorED->connect_changed(aLk);
    m_xTextEdit->connect_changed(aLk);

    m_xCategoryBox->connect_changed(LINK(this, SwCaptionOptPage, SelectHdl));
    m_xFormatBox->connect_changed(LINK(this, SwCaptionOptPage, SelectListBoxHdl));

    m_xLbCaptionOrder->connect_changed(LINK(this, SwCaptionOptPage, OrderHdl));

    m_xCheckLB->connect_selection_changed(LINK(this, SwCaptionOptPage, ShowEntryHdl));
    m_xCheckLB->connect_toggled(LINK(this, SwCaptionOptPage, ToggleEntryHdl));
}

SwCaptionOptPage::~SwCaptionOptPage()
{
    DelUserData();
    m_pMgr.reset();
    m_xPreview.reset();
}

std::unique_ptr<SfxTabPage> SwCaptionOptPage::Create(weld::Container* pPage, weld::DialogController* pController,
                                            const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwCaptionOptPage>(pPage, pController, *rAttrSet);
}

OUString SwCaptionOptPage::GetAllStrings()
{
    OUStringBuffer sAllStrings;
    OUString labels[] = { u"label1"_ustr,         u"label13"_ustr,     u"label2"_ustr,  u"label7"_ustr,  u"numberingft"_ustr,
                          u"numseparatorft"_ustr, u"separatorft"_ustr, u"label18"_ustr, u"label11"_ustr, u"label4"_ustr,
                          u"label6"_ustr,         u"label10"_ustr,     u"label3"_ustr };

    for (const auto& label : labels)
    {
        if (const auto pString = m_xBuilder->weld_label(label))
            sAllStrings.append(pString->get_label() + " ");
    }

    if (const auto pString = m_xBuilder->weld_check_button(u"applyborder"_ustr))
        sAllStrings.append(pString->get_label() + " ");

    return sAllStrings.toString().replaceAll("_", "");
}

bool SwCaptionOptPage::FillItemSet( SfxItemSet* )
{
    bool bRet = false;
    SwModuleOptions* pModOpt = SwModule::get()->GetModuleConfig();

    SaveEntry(m_xCheckLB->get_selected_index());    // apply current entry

    int nCheckCount = 0;
    for (int i = 0, nCount = m_xCheckLB->n_children(); i < nCount; ++i)
    {
        if (m_xCheckLB->get_toggle(i) == TRISTATE_TRUE)
            ++nCheckCount;
        InsCaptionOpt* pData = weld::fromId<InsCaptionOpt*>(m_xCheckLB->get_id(i));
        bRet |= pModOpt->SetCapOption(m_bHTMLMode, pData);
    }

    pModOpt->SetInsWithCaption(m_bHTMLMode, nCheckCount > 0);

    int nPos = m_xLbCaptionOrder->get_active();
    pModOpt->SetCaptionOrderNumberingFirst(nPos == 1);

    return bRet;
}

void SwCaptionOptPage::Reset( const SfxItemSet* rSet)
{
    if(const SfxUInt16Item* pItem = rSet->GetItemIfSet(SID_HTML_MODE, false))
    {
        m_bHTMLMode = 0 != (pItem->GetValue() & HTMLMODE_ON);
    }

    DelUserData();
    m_xCheckLB->clear();   // remove all entries

    // Writer objects
    int nPos = 0;
    bool bReadOnly = false;
    m_xCheckLB->append();
    m_xCheckLB->set_toggle(nPos, TRISTATE_FALSE);
    m_xCheckLB->set_text(nPos, m_sSWTable, 0);
    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Table::Enable::isReadOnly();
    m_xCheckLB->set_sensitive(nPos, !bReadOnly);
    SetOptions(nPos++, TABLE_CAP);
    m_xCheckLB->append();
    m_xCheckLB->set_toggle(nPos, TRISTATE_FALSE);
    m_xCheckLB->set_text(nPos, m_sSWFrame, 0);
    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Frame::Enable::isReadOnly();
    m_xCheckLB->set_sensitive(nPos, !bReadOnly);
    SetOptions(nPos++, FRAME_CAP);
    m_xCheckLB->append();
    m_xCheckLB->set_toggle(nPos, TRISTATE_FALSE);
    m_xCheckLB->set_text(nPos, m_sSWGraphic, 0);
    bReadOnly = officecfg::Office::Writer::Insert::Caption::WriterObject::Graphic::Enable::isReadOnly();
    m_xCheckLB->set_sensitive(nPos, !bReadOnly);
    SetOptions(nPos++, GRAPHIC_CAP);

    // get Productname and -version
    const OUString sWithoutVersion( utl::ConfigManager::getProductName() );
    const OUString sComplete(
        sWithoutVersion + " " +
        utl::ConfigManager::getProductVersion() );

    SvObjectServerList aObjS;
    aObjS.FillInsertObjects();
    aObjS.Remove( SvGlobalName( SO3_SW_CLASSID ) ); // remove Writer-ID

    for ( sal_uLong i = 0; i < aObjS.Count(); ++i )
    {
        const SvGlobalName &rOleId = aObjS[i].GetClassName();
        OUString sClass;
        if (rOleId == SvGlobalName(SO3_OUT_CLASSID))
            sClass = m_sOLE;
        else
            sClass = aObjS[i].GetHumanName();
        // don't show product version
        sClass = sClass.replaceFirst( sComplete, sWithoutVersion );
        m_xCheckLB->append();
        m_xCheckLB->set_toggle(nPos, TRISTATE_FALSE);
        m_xCheckLB->set_text(nPos, sClass, 0);
        if (rOleId == SvGlobalName(SO3_SC_CLASSID)) {
            bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Calc::Enable::isReadOnly();
        } else if (rOleId == SvGlobalName(SO3_SDRAW_CLASSID)) {
            bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Draw::Enable::isReadOnly();
        } else if (rOleId == SvGlobalName(SO3_SM_CLASSID)) {
            bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Formula::Enable::isReadOnly();
        } else if (rOleId == SvGlobalName(SO3_SCH_CLASSID)) {
            bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Chart::Enable::isReadOnly();
        } else if (rOleId == SvGlobalName(SO3_SIMPRESS_CLASSID)) {
            bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::Impress::Enable::isReadOnly();
        } else if (rOleId == SvGlobalName(SO3_OUT_CLASSID)) {
            bReadOnly = officecfg::Office::Writer::Insert::Caption::OfficeObject::OLEMisc::Enable::isReadOnly();
        }
        else {
            //
        }
        m_xCheckLB->set_sensitive(nPos, !bReadOnly);
        SetOptions( nPos++, OLE_CAP, &rOleId );
    }
    m_xLbCaptionOrder->set_active(
        SwModule::get()->GetModuleConfig()->IsCaptionOrderNumberingFirst() ? 1 : 0);
    m_xLbCaptionOrder->set_sensitive(!officecfg::Office::Writer::Insert::Caption::CaptionOrderNumberingFirst::isReadOnly());
    m_xLbCaptionOrderImg->set_visible(officecfg::Office::Writer::Insert::Caption::CaptionOrderNumberingFirst::isReadOnly());
    m_xCheckLB->select(0);
    ShowEntryHdl(*m_xCheckLB);
}

void SwCaptionOptPage::SetOptions(const sal_uLong nPos,
        const SwCapObjType eObjType, const SvGlobalName *pOleId)
{
    SwModuleOptions* pModOpt = SwModule::get()->GetModuleConfig();
    const InsCaptionOpt* pOpt = pModOpt->GetCapOption(m_bHTMLMode, eObjType, pOleId);

    if (pOpt)
    {
        InsCaptionOpt* pIns = new InsCaptionOpt(*pOpt);
        m_xCheckLB->set_id(nPos, weld::toId(pIns));
        m_xCheckLB->set_toggle(nPos, pOpt->UseCaption() ? TRISTATE_TRUE : TRISTATE_FALSE);
    }
    else
    {
        InsCaptionOpt* pIns = new InsCaptionOpt(eObjType, pOleId);
        m_xCheckLB->set_id(nPos, weld::toId(pIns));
    }
}

void SwCaptionOptPage::DelUserData()
{
    for (int i = 0, nCount = m_xCheckLB->n_children(); i < nCount; ++i)
    {
        delete weld::fromId<InsCaptionOpt*>(m_xCheckLB->get_id(i));
        m_xCheckLB->set_id(i, u"0"_ustr);
    }
}

void SwCaptionOptPage::UpdateEntry(int nSelEntry)
{
    if (nSelEntry != -1)
    {
        bool bChecked = m_xCheckLB->get_toggle(nSelEntry) == TRISTATE_TRUE;

        m_xSettingsGroup->set_sensitive(bChecked);
        bool bNumSep = bChecked && m_xLbCaptionOrder->get_active() == 1;
        m_xNumberingSeparatorED->set_sensitive( bNumSep );
        m_xNumberingSeparatorFT->set_sensitive( bNumSep );

        m_xNumCapt->set_sensitive(bChecked);
        m_xCategory->set_sensitive(bChecked);
        m_xPreview->set_sensitive(bChecked);


        InsCaptionOpt* pOpt = weld::fromId<InsCaptionOpt*>(m_xCheckLB->get_id(nSelEntry));

        m_xCategoryBox->clear();
        m_xCategoryBox->append_text(m_sNone);
        if (::GetActiveWrtShell())
        {
            const size_t nCount = m_pMgr->GetFieldTypeCount();

            for (size_t i = 0; i < nCount; ++i)
            {
                SwFieldType *pType = m_pMgr->GetFieldType( SwFieldIds::Unknown, i );
                if( pType->Which() == SwFieldIds::SetExp &&
                    static_cast<SwSetExpFieldType *>( pType)->GetType() & SwGetSetExpType::Sequence )
                {
                    m_xCategoryBox->append_text(pType->GetName().toString());
                }
            }
        }
        else
        {
            m_xCategoryBox->append_text(m_sIllustration.toString());
            m_xCategoryBox->append_text(m_sTable.toString());
            m_xCategoryBox->append_text(m_sText.toString());
            m_xCategoryBox->append_text(m_sDrawing.toString());
        }

        if (!pOpt->GetCategory().isEmpty())
        {
            if (m_xCategoryBox->find_text(pOpt->GetCategory()) == -1)
                m_xCategoryBox->insert_text(0, pOpt->GetCategory());
            m_xCategoryBox->set_active_text(pOpt->GetCategory());
        }
        else
            m_xCategoryBox->set_active_text(m_sNone);

        if (m_xCategoryBox->get_active_text().isEmpty())
        {
            sal_Int32 nPos = 0;
            switch(pOpt->GetObjType())
            {
                case OLE_CAP:
                case GRAPHIC_CAP:       nPos = 1;   break;
                case TABLE_CAP:         nPos = 2;   break;
                case FRAME_CAP:         nPos = 3;   break;
            }
            m_xCategoryBox->set_active(nPos);
        }

        bool bReadOnly = lcl_isPropertyReadOnly(pOpt->GetObjType(), PROP_CAP_OBJECT_CATEGORY, pOpt->GetOleId());
        m_xCategoryBox->set_sensitive(bChecked && !bReadOnly);
        m_xCategoryBoxImg->set_visible(bReadOnly);

        bReadOnly = lcl_isPropertyReadOnly(pOpt->GetObjType(), PROP_CAP_OBJECT_NUMBERING, pOpt->GetOleId());
        m_xFormatBox->set_sensitive(bChecked && !bReadOnly);
        m_xFormatBoxImg->set_visible(bReadOnly);

        bReadOnly = lcl_isPropertyReadOnly(pOpt->GetObjType(), PROP_CAP_OBJECT_NUMBERINGSEPARATOR, pOpt->GetOleId());
        m_xNumberingSeparatorED->set_sensitive(bNumSep && !bReadOnly);
        m_xNumberingSeparatorImg->set_visible(bReadOnly);

        bReadOnly = lcl_isPropertyReadOnly(pOpt->GetObjType(), PROP_CAP_OBJECT_CAPTIONTEXT, pOpt->GetOleId());
        m_xTextEdit->set_sensitive(bChecked && !bReadOnly);
        m_xTextEditImg->set_visible(bReadOnly);

        bReadOnly = lcl_isPropertyReadOnly(pOpt->GetObjType(), PROP_CAP_OBJECT_DELIMITER, pOpt->GetOleId());
        m_xPosBox->set_sensitive(bChecked && !bReadOnly);
        m_xPosBoxImg->set_visible(bReadOnly);

        bReadOnly = lcl_isPropertyReadOnly(pOpt->GetObjType(), PROP_CAP_OBJECT_LEVEL, pOpt->GetOleId());
        m_xLbLevel->set_sensitive(bChecked && !bReadOnly);
        m_xLbLevelImg->set_visible(bReadOnly);

        bReadOnly = lcl_isPropertyReadOnly(pOpt->GetObjType(), PROP_CAP_OBJECT_POSITION, pOpt->GetOleId());
        m_xEdDelim->set_sensitive(bChecked && !bReadOnly);
        m_xEdDelimImg->set_visible(bReadOnly);

        bReadOnly = lcl_isPropertyReadOnly(pOpt->GetObjType(), PROP_CAP_OBJECT_CHARACTERSTYLE, pOpt->GetOleId());
        m_xCharStyleLB->set_sensitive(bChecked && !bReadOnly);
        m_xCharStyleImg->set_visible(bReadOnly);

        for (sal_Int32 i = 0; i < m_xFormatBox->get_count(); i++)
        {
            if (pOpt->GetNumType() == m_xFormatBox->get_id(i).toUInt32())
            {
                m_xFormatBox->set_active(i);
                break;
            }
        }
        m_xTextEdit->set_text(pOpt->GetCaption());

        m_xPosBox->clear();
        switch (pOpt->GetObjType())
        {
            case GRAPHIC_CAP:
            case TABLE_CAP:
            case OLE_CAP:
                m_xPosBox->append_text(m_sAbove);
                m_xPosBox->append_text(m_sBelow);
                break;
            case FRAME_CAP:
                m_xPosBox->append_text(m_sBegin);
                m_xPosBox->append_text(m_sEnd);
                break;
        }
        m_xPosBox->set_active(pOpt->GetPos());

        sal_Int32 nLevelPos = ( pOpt->GetLevel() < MAXLEVEL ) ? pOpt->GetLevel() + 1 : 0;
        m_xLbLevel->set_active(nLevelPos);
        m_xEdDelim->set_text(pOpt->GetSeparator());
        m_xNumberingSeparatorED->set_text(pOpt->GetNumSeparator());
        if (!pOpt->GetCharacterStyle().isEmpty())
            m_xCharStyleLB->set_active_text(pOpt->GetCharacterStyle().toString());
        else
            m_xCharStyleLB->set_active(0);

        bReadOnly = lcl_isPropertyReadOnly(pOpt->GetObjType(), PROP_CAP_OBJECT_APPLYATTRIBUTES, pOpt->GetOleId());
        m_xApplyBorderCB->set_sensitive(m_xCategoryBox->get_sensitive() && !bReadOnly &&
                pOpt->GetObjType() != TABLE_CAP && pOpt->GetObjType() != FRAME_CAP );
        m_xApplyBorderImg->set_visible(bReadOnly);
        m_xApplyBorderCB->set_active(pOpt->CopyAttributes());
    }

    ModifyHdl();
}

IMPL_LINK_NOARG(SwCaptionOptPage, ShowEntryHdl, weld::TreeView&, void)
{
    if (m_nPrevSelectedEntry != -1)
        SaveEntry(m_nPrevSelectedEntry);
    UpdateEntry(m_xCheckLB->get_selected_index());
    m_nPrevSelectedEntry = m_xCheckLB->get_selected_index();
}

IMPL_LINK(SwCaptionOptPage, ToggleEntryHdl, const weld::TreeView::iter_col&, rRowCol, void)
{
    UpdateEntry(m_xCheckLB->get_iter_index_in_parent(rRowCol.first));
}

void SwCaptionOptPage::SaveEntry(int nEntry)
{
    if (nEntry == -1)
        return;

    InsCaptionOpt* pOpt = weld::fromId<InsCaptionOpt*>(m_xCheckLB->get_id(nEntry));

    pOpt->UseCaption() = m_xCheckLB->get_toggle(nEntry) == TRISTATE_TRUE;
    const OUString aName(m_xCategoryBox->get_active_text());
    if (aName == m_sNone)
        pOpt->SetCategory(u""_ustr);
    else
        pOpt->SetCategory(comphelper::string::strip(aName, ' '));
    pOpt->SetNumType(m_xFormatBox->get_active_id().toUInt32());
    pOpt->SetCaption(m_xTextEdit->get_sensitive() ? m_xTextEdit->get_text() : OUString() );
    pOpt->SetPos(m_xPosBox->get_active());
    int nPos = m_xLbLevel->get_active();
    sal_Int32 nLevel = (nPos > 0) ? nPos - 1 : MAXLEVEL;
    pOpt->SetLevel(nLevel);
    pOpt->SetSeparator(m_xEdDelim->get_text());
    pOpt->SetNumSeparator(m_xNumberingSeparatorED->get_text());
    if (m_xCharStyleLB->get_active() == -1)
        pOpt->SetCharacterStyle(UIName(u""_ustr));
    else
        pOpt->SetCharacterStyle(UIName(m_xCharStyleLB->get_active_text()));
    pOpt->CopyAttributes() = m_xApplyBorderCB->get_active();
}

void SwCaptionOptPage::ModifyHdl()
{
    const OUString sFieldTypeName = m_xCategoryBox->get_active_text();

    if (SfxSingleTabDialogController* pDlg = dynamic_cast<SfxSingleTabDialogController*>(GetDialogController()))
        pDlg->GetOKButton().set_sensitive(!sFieldTypeName.isEmpty());
    bool bEnable = m_xCategoryBox->get_sensitive() && sFieldTypeName != m_sNone;

    m_xFormatText->set_sensitive(bEnable);
    m_xFormatBox->set_sensitive(bEnable);
    m_xTextText->set_sensitive(bEnable);
    m_xTextEdit->set_sensitive(bEnable);

    InvalidatePreview();
}

IMPL_LINK_NOARG(SwCaptionOptPage, ModifyEntryHdl, weld::Entry&, void)
{
    ModifyHdl();
}

IMPL_LINK_NOARG(SwCaptionOptPage, ModifyComboHdl, weld::ComboBox&, void)
{
    ModifyHdl();
}

IMPL_LINK_NOARG(SwCaptionOptPage, SelectHdl, weld::ComboBox&, void)
{
    InvalidatePreview();
}

IMPL_LINK_NOARG(SwCaptionOptPage, SelectListBoxHdl, weld::ComboBox&, void)
{
    InvalidatePreview();
}

IMPL_LINK(SwCaptionOptPage, OrderHdl, weld::ComboBox&, rBox, void)
{
    InvalidatePreview();

    int nSelEntry = m_xCheckLB->get_selected_index();
    bool bChecked = false;
    if (nSelEntry != -1)
    {
        bChecked = m_xCheckLB->get_toggle(nSelEntry) == TRISTATE_TRUE;
    }

    int nPos = rBox.get_active();
    m_xNumberingSeparatorFT->set_sensitive(bChecked && nPos == 1);
    m_xNumberingSeparatorED->set_sensitive(bChecked && nPos == 1);
}

void SwCaptionOptPage::InvalidatePreview()
{
    OUString aStr;

    if (m_xCategoryBox->get_active_text() != m_sNone)
    {
        //#i61007# order of captions
        bool bOrderNumberingFirst = m_xLbCaptionOrder->get_active() == 1;
        // number
        const sal_uInt16 nNumFormat = m_xFormatBox->get_active_id().toUInt32();
        if (SVX_NUM_NUMBER_NONE != nNumFormat)
        {
            //#i61007# order of captions
            if( !bOrderNumberingFirst )
            {
                // category
                aStr += m_xCategoryBox->get_active_text() + " ";
            }

            if (SwWrtShell *pSh = ::GetActiveWrtShell())
            {
                SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(m_pMgr->GetFieldType(
                                                SwFieldIds::SetExp, m_xCategoryBox->get_active_text() ));
                if( pFieldType && pFieldType->GetOutlineLvl() < MAXLEVEL )
                {
                    sal_uInt8 nLvl = pFieldType->GetOutlineLvl();
                    SwNumberTree::tNumberVector aNumVector;
                    for( sal_uInt8 i = 0; i <= nLvl; ++i )
                        aNumVector.push_back(1);

                    const OUString sNumber( pSh->GetOutlineNumRule()->MakeNumString(
                                                            aNumVector, false ));
                    if( !sNumber.isEmpty() )
                        aStr += sNumber + pFieldType->GetDelimiter();
                }
            }

            switch( nNumFormat )
            {
                case SVX_NUM_CHARS_UPPER_LETTER:    aStr += "A"; break;
                case SVX_NUM_CHARS_UPPER_LETTER_N:  aStr += "A"; break;
                case SVX_NUM_CHARS_LOWER_LETTER:    aStr += "a"; break;
                case SVX_NUM_CHARS_LOWER_LETTER_N:  aStr += "a"; break;
                case SVX_NUM_ROMAN_UPPER:           aStr += "I"; break;
                case SVX_NUM_ROMAN_LOWER:           aStr += "i"; break;
                //case ARABIC:
                default:                    aStr += "1"; break;
            }
        }
        //#i61007# order of captions
        if( bOrderNumberingFirst )
        {
            aStr += m_xNumberingSeparatorED->get_text() + m_xCategoryBox->get_active_text();
        }
        aStr += m_xTextEdit->get_text();
    }
    m_aPreview.SetPreviewText(aStr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
