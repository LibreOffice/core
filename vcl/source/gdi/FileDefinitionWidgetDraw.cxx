/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <FileDefinitionWidgetDraw.hxx>

#include <sal/config.h>
#include <svdata.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>

namespace vcl
{
namespace
{
OUString lcl_getThemeDefinitionPath()
{
    OUString sPath("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/theme_definitions/");
    rtl::Bootstrap::expandMacros(sPath);
    return sPath;
}

} // end anonymous namespace

FileDefinitionWidgetDraw::FileDefinitionWidgetDraw(SalGraphics& rGraphics)
    : m_rGraphics(rGraphics)
    , m_WidgetDefinitionReader(lcl_getThemeDefinitionPath() + "definition.xml")
{
    m_WidgetDefinitionReader.read();

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maNWFData.mbNoFocusRects = true;
    pSVData->maNWFData.mbNoFocusRectsForFlatButtons = true;
}

bool FileDefinitionWidgetDraw::isNativeControlSupported(ControlType eType, ControlPart /*ePart*/)
{
    switch (eType)
    {
        case ControlType::Generic:
        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
            return true;
        case ControlType::Checkbox:
        case ControlType::Combobox:
            return false;
        case ControlType::Editbox:
        case ControlType::EditboxNoBorder:
        case ControlType::MultilineEditbox:
            return true;
        case ControlType::Listbox:
        case ControlType::Spinbox:
        case ControlType::SpinButtons:
        case ControlType::TabItem:
        case ControlType::TabPane:
        case ControlType::TabHeader:
        case ControlType::TabBody:
        case ControlType::Scrollbar:
        case ControlType::Slider:
        case ControlType::Fixedline:
        case ControlType::Toolbar:
        case ControlType::Menubar:
        case ControlType::MenuPopup:
        case ControlType::Progress:
        case ControlType::IntroProgress:
        case ControlType::Tooltip:
        case ControlType::WindowBackground:
        case ControlType::Frame:
        case ControlType::ListNode:
        case ControlType::ListNet:
        case ControlType::ListHeader:
            return false;
    }

    return false;
}

bool FileDefinitionWidgetDraw::hitTestNativeControl(
    ControlType /*eType*/, ControlPart /*ePart*/,
    const tools::Rectangle& /*rBoundingControlRegion*/, const Point& /*aPos*/, bool& /*rIsInside*/)
{
    return false;
}

namespace
{
void munchDrawCommands(std::vector<std::shared_ptr<DrawCommand>> const& rDrawCommands,
                       SalGraphics& rGraphics, long nX, long nY, long nWidth, long nHeight)
{
    for (std::shared_ptr<DrawCommand> const& pDrawCommand : rDrawCommands)
    {
        switch (pDrawCommand->maType)
        {
            case DrawCommandType::RECTANGLE:
            {
                auto const& rRectDrawCommand
                    = static_cast<RectangleDrawCommand const&>(*pDrawCommand);
                Point aRectPoint(nX, nY);
                Size aRectSize(nWidth - 1, nHeight - 1);
                tools::Polygon aPolygon(tools::Rectangle(aRectPoint, aRectSize),
                                        rRectDrawCommand.mnRx, rRectDrawCommand.mnRy);

                basegfx::B2DPolygon aB2DPolygon(aPolygon.getB2DPolygon());
                rGraphics.SetLineColor(rRectDrawCommand.maStrokeColor);
                rGraphics.SetFillColor(rRectDrawCommand.maFillColor);
                rGraphics.DrawPolyPolygon(basegfx::B2DHomMatrix(),
                                          basegfx::B2DPolyPolygon(aB2DPolygon), 0.0f, nullptr);
            }
            break;
            case DrawCommandType::CIRCLE:
            {
                auto const& rCircleDrawCommand
                    = static_cast<CircleDrawCommand const&>(*pDrawCommand);
                Point aRectPoint(nX + 1 + rCircleDrawCommand.mnMargin,
                                 nY + 1 + rCircleDrawCommand.mnMargin);
                Size aRectSize(nWidth - 1 - 2 * rCircleDrawCommand.mnMargin,
                               nHeight - 1 - 2 * rCircleDrawCommand.mnMargin);

                tools::Rectangle aRectangle(aRectPoint, aRectSize);
                tools::Polygon aPolygon(aRectangle.Center(), aRectangle.GetWidth() >> 1,
                                        aRectangle.GetHeight() >> 1);

                basegfx::B2DPolygon aB2DPolygon(aPolygon.getB2DPolygon());
                rGraphics.SetLineColor(rCircleDrawCommand.maStrokeColor);
                rGraphics.SetFillColor(rCircleDrawCommand.maFillColor);
                rGraphics.DrawPolyPolygon(basegfx::B2DHomMatrix(),
                                          basegfx::B2DPolyPolygon(aB2DPolygon), 0.0f, nullptr);
            }
            break;
            case DrawCommandType::LINE:
            {
                auto const& rLineDrawCommand = static_cast<LineDrawCommand const&>(*pDrawCommand);
                Point aRectPoint(nX + 1 + rLineDrawCommand.mnMargin,
                                 nY + 1 + rLineDrawCommand.mnMargin);

                Size aRectSize(nWidth - 1 - 2 * rLineDrawCommand.mnMargin,
                               nHeight - 1 - 2 * rLineDrawCommand.mnMargin);

                rGraphics.SetFillColor();
                rGraphics.SetLineColor(rLineDrawCommand.maStrokeColor);

                basegfx::B2DPolygon aB2DPolygon{
                    { aRectPoint.X() + (aRectSize.Width() * rLineDrawCommand.mfX1),
                      aRectPoint.Y() + (aRectSize.Height() * rLineDrawCommand.mfY1) },
                    { aRectPoint.X() + (aRectSize.Width() * rLineDrawCommand.mfX2),
                      aRectPoint.Y() + (aRectSize.Height() * rLineDrawCommand.mfY2) },
                };

                rGraphics.DrawPolyLine(basegfx::B2DHomMatrix(), aB2DPolygon, 0.0f,
                                       basegfx::B2DVector(rLineDrawCommand.mnStrokeWidth,
                                                          rLineDrawCommand.mnStrokeWidth),
                                       basegfx::B2DLineJoin::Round, css::drawing::LineCap_ROUND,
                                       0.0f, false, nullptr);
            }
            break;
        }
    }
}

} // end anonymous namespace

bool FileDefinitionWidgetDraw::drawNativeControl(ControlType eType, ControlPart ePart,
                                                 const tools::Rectangle& rControlRegion,
                                                 ControlState eState,
                                                 const ImplControlValue& rValue,
                                                 const OUString& /*aCaptions*/)
{
    bool bOldAA = m_rGraphics.getAntiAliasB2DDraw();
    m_rGraphics.setAntiAliasB2DDraw(true);

    long nWidth = rControlRegion.GetWidth();
    long nHeight = rControlRegion.GetHeight();
    long nX = rControlRegion.Left() + 1;
    long nY = rControlRegion.Top() + 1;

    bool bOK = false;

    switch (eType)
    {
        case ControlType::Generic:
        {
        }
        break;
        case ControlType::Pushbutton:
        {
            std::shared_ptr<WidgetDefinition> pDefinition
                = m_WidgetDefinitionReader.getPushButtonDefinition(ePart);
            if (pDefinition)
            {
                auto aStates = pDefinition->getStates(eState, rValue);
                if (!aStates.empty())
                {
                    std::shared_ptr<WidgetDefinitionState> pState = aStates.back();
                    {
                        munchDrawCommands(pState->mpDrawCommands, m_rGraphics, nX, nY, nWidth,
                                          nHeight);
                        bOK = true;
                    }
                }
            }
        }
        break;
        case ControlType::Radiobutton:
        {
            std::shared_ptr<WidgetDefinition> pDefinition
                = m_WidgetDefinitionReader.getRadioButtonDefinition(ePart);
            if (pDefinition)
            {
                std::shared_ptr<WidgetDefinitionState> pState
                    = pDefinition->getStates(eState, rValue).back();
                {
                    munchDrawCommands(pState->mpDrawCommands, m_rGraphics, nX, nY, nWidth, nHeight);
                    bOK = true;
                }
            }
        }
        break;
        case ControlType::Checkbox:
        case ControlType::Combobox:
            break;
        case ControlType::Editbox:
        case ControlType::EditboxNoBorder:
        case ControlType::MultilineEditbox:
        {
            std::shared_ptr<WidgetDefinition> pDefinition
                = m_WidgetDefinitionReader.getEditboxDefinition(ePart);
            if (pDefinition)
            {
                std::shared_ptr<WidgetDefinitionState> pState
                    = pDefinition->getStates(eState, rValue).back();
                {
                    munchDrawCommands(pState->mpDrawCommands, m_rGraphics, nX, nY, nWidth, nHeight);
                    bOK = true;
                }
            }
        }
        break;
        case ControlType::Listbox:
        case ControlType::Spinbox:
        case ControlType::SpinButtons:
        case ControlType::TabItem:
        case ControlType::TabPane:
        case ControlType::TabBody:
        case ControlType::Scrollbar:
        case ControlType::Slider:
        case ControlType::Fixedline:
        case ControlType::Toolbar:
        case ControlType::Menubar:
            break;
        case ControlType::MenuPopup:
            break;
        case ControlType::Progress:
        case ControlType::IntroProgress:
            break;
        case ControlType::Tooltip:
            break;
        case ControlType::WindowBackground:
        case ControlType::Frame:
        case ControlType::ListNode:
        case ControlType::ListNet:
        case ControlType::ListHeader:
        default:
            break;
    }

    m_rGraphics.setAntiAliasB2DDraw(bOldAA);

    return bOK;
}

bool FileDefinitionWidgetDraw::getNativeControlRegion(
    ControlType /*eType*/, ControlPart /*ePart*/,
    const tools::Rectangle& /*rBoundingControlRegion*/, ControlState /*eState*/,
    const ImplControlValue& /*aValue*/, const OUString& /*aCaption*/,
    tools::Rectangle& /*rNativeBoundingRegion*/, tools::Rectangle& /*rNativeContentRegion*/)
{
    return false;
}

bool FileDefinitionWidgetDraw::updateSettings(AllSettings& rSettings)
{
    StyleSettings aStyleSet = rSettings.GetStyleSettings();

    aStyleSet.SetFaceColor(m_WidgetDefinitionReader.maFaceColor);
    aStyleSet.SetCheckedColor(m_WidgetDefinitionReader.maCheckedColor);
    aStyleSet.SetLightColor(m_WidgetDefinitionReader.maLightColor);
    aStyleSet.SetLightBorderColor(m_WidgetDefinitionReader.maLightBorderColor);
    aStyleSet.SetShadowColor(m_WidgetDefinitionReader.maShadowColor);
    aStyleSet.SetDarkShadowColor(m_WidgetDefinitionReader.maDarkShadowColor);
    aStyleSet.SetButtonTextColor(m_WidgetDefinitionReader.maButtonTextColor);
    aStyleSet.SetButtonRolloverTextColor(m_WidgetDefinitionReader.maButtonRolloverTextColor);
    aStyleSet.SetRadioCheckTextColor(m_WidgetDefinitionReader.maRadioCheckTextColor);
    aStyleSet.SetGroupTextColor(m_WidgetDefinitionReader.maGroupTextColor);
    aStyleSet.SetLabelTextColor(m_WidgetDefinitionReader.maLabelTextColor);
    aStyleSet.SetWindowColor(m_WidgetDefinitionReader.maWindowColor);
    aStyleSet.SetWindowTextColor(m_WidgetDefinitionReader.maWindowTextColor);
    aStyleSet.SetDialogColor(m_WidgetDefinitionReader.maDialogColor);
    aStyleSet.SetDialogTextColor(m_WidgetDefinitionReader.maDialogTextColor);
    aStyleSet.SetWorkspaceColor(m_WidgetDefinitionReader.maWorkspaceColor);
    aStyleSet.SetMonoColor(m_WidgetDefinitionReader.maMonoColor);
    aStyleSet.SetFieldColor(m_WidgetDefinitionReader.maFieldColor);
    aStyleSet.SetFieldTextColor(m_WidgetDefinitionReader.maFieldTextColor);
    aStyleSet.SetFieldRolloverTextColor(m_WidgetDefinitionReader.maFieldRolloverTextColor);
    aStyleSet.SetActiveColor(m_WidgetDefinitionReader.maActiveColor);
    aStyleSet.SetActiveTextColor(m_WidgetDefinitionReader.maActiveTextColor);
    aStyleSet.SetActiveBorderColor(m_WidgetDefinitionReader.maActiveBorderColor);
    aStyleSet.SetDeactiveColor(m_WidgetDefinitionReader.maDeactiveColor);
    aStyleSet.SetDeactiveTextColor(m_WidgetDefinitionReader.maDeactiveTextColor);
    aStyleSet.SetDeactiveBorderColor(m_WidgetDefinitionReader.maDeactiveBorderColor);
    aStyleSet.SetMenuColor(m_WidgetDefinitionReader.maMenuColor);
    aStyleSet.SetMenuBarColor(m_WidgetDefinitionReader.maMenuBarColor);
    aStyleSet.SetMenuBarRolloverColor(m_WidgetDefinitionReader.maMenuBarRolloverColor);
    aStyleSet.SetMenuBorderColor(m_WidgetDefinitionReader.maMenuBorderColor);
    aStyleSet.SetMenuTextColor(m_WidgetDefinitionReader.maMenuTextColor);
    aStyleSet.SetMenuBarTextColor(m_WidgetDefinitionReader.maMenuBarTextColor);
    aStyleSet.SetMenuBarRolloverTextColor(m_WidgetDefinitionReader.maMenuBarRolloverTextColor);
    aStyleSet.SetMenuBarHighlightTextColor(m_WidgetDefinitionReader.maMenuBarHighlightTextColor);
    aStyleSet.SetMenuHighlightColor(m_WidgetDefinitionReader.maMenuHighlightColor);
    aStyleSet.SetMenuHighlightTextColor(m_WidgetDefinitionReader.maMenuHighlightTextColor);
    aStyleSet.SetHighlightColor(m_WidgetDefinitionReader.maHighlightColor);
    aStyleSet.SetHighlightTextColor(m_WidgetDefinitionReader.maHighlightTextColor);
    aStyleSet.SetActiveTabColor(m_WidgetDefinitionReader.maActiveTabColor);
    aStyleSet.SetInactiveTabColor(m_WidgetDefinitionReader.maInactiveTabColor);
    aStyleSet.SetTabTextColor(m_WidgetDefinitionReader.maTabTextColor);
    aStyleSet.SetTabRolloverTextColor(m_WidgetDefinitionReader.maTabRolloverTextColor);
    aStyleSet.SetTabHighlightTextColor(m_WidgetDefinitionReader.maTabHighlightTextColor);
    aStyleSet.SetDisableColor(m_WidgetDefinitionReader.maDisableColor);
    aStyleSet.SetHelpColor(m_WidgetDefinitionReader.maHelpColor);
    aStyleSet.SetHelpTextColor(m_WidgetDefinitionReader.maHelpTextColor);
    aStyleSet.SetLinkColor(m_WidgetDefinitionReader.maLinkColor);
    aStyleSet.SetVisitedLinkColor(m_WidgetDefinitionReader.maVisitedLinkColor);
    aStyleSet.SetToolTextColor(m_WidgetDefinitionReader.maToolTextColor);
    aStyleSet.SetFontColor(m_WidgetDefinitionReader.maFontColor);

    rSettings.SetStyleSettings(aStyleSet);

    return true;
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
