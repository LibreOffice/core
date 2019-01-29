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

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

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
{
    WidgetDefinitionReader aReader(lcl_getThemeDefinitionPath() + "definition.xml");
    aReader.read(m_aWidgetDefinition);

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maNWFData.mbNoFocusRects = true;
    pSVData->maNWFData.mbNoFocusRectsForFlatButtons = true;
}

bool FileDefinitionWidgetDraw::isNativeControlSupported(ControlType eType, ControlPart ePart)
{
    switch (eType)
    {
        case ControlType::Generic:
        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
            return true;
        case ControlType::Combobox:
            if (ePart == ControlPart::HasBackgroundTexture)
                return false;
            return true;
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

                basegfx::B2DRectangle rRect(
                    nX + (nWidth * rRectDrawCommand.mfX1), nY + (nHeight * rRectDrawCommand.mfY1),
                    nX + (nWidth * rRectDrawCommand.mfX2), nY + (nHeight * rRectDrawCommand.mfY2));

                basegfx::B2DPolygon aB2DPolygon = basegfx::utils::createPolygonFromRect(
                    rRect, rRectDrawCommand.mnRx / rRect.getWidth() * 2.0,
                    rRectDrawCommand.mnRy / rRect.getHeight() * 2.0);

                rGraphics.SetLineColor();
                rGraphics.SetFillColor(rRectDrawCommand.maFillColor);
                rGraphics.DrawPolyPolygon(basegfx::B2DHomMatrix(),
                                          basegfx::B2DPolyPolygon(aB2DPolygon), 0.0f, nullptr);
                rGraphics.SetLineColor(rRectDrawCommand.maStrokeColor);
                rGraphics.SetFillColor();
                rGraphics.DrawPolyLine(basegfx::B2DHomMatrix(), aB2DPolygon, 0.0f,
                                       basegfx::B2DVector(rRectDrawCommand.mnStrokeWidth,
                                                          rRectDrawCommand.mnStrokeWidth),
                                       basegfx::B2DLineJoin::Round, css::drawing::LineCap_ROUND,
                                       0.0f, false, nullptr);
            }
            break;
            case DrawCommandType::CIRCLE:
            {
                auto const& rCircleDrawCommand
                    = static_cast<CircleDrawCommand const&>(*pDrawCommand);

                basegfx::B2DRectangle rRect(nX + (nWidth * rCircleDrawCommand.mfX1),
                                            nY + (nHeight * rCircleDrawCommand.mfY1),
                                            nX + (nWidth * rCircleDrawCommand.mfX2),
                                            nY + (nHeight * rCircleDrawCommand.mfY2));

                basegfx::B2DPolygon aB2DPolygon = basegfx::utils::createPolygonFromEllipse(
                    rRect.getCenter(), rRect.getWidth() / 2.0, rRect.getHeight() / 2.0);

                rGraphics.SetLineColor(rCircleDrawCommand.maStrokeColor);
                rGraphics.SetFillColor(rCircleDrawCommand.maFillColor);
                rGraphics.DrawPolyPolygon(basegfx::B2DHomMatrix(),
                                          basegfx::B2DPolyPolygon(aB2DPolygon), 0.0f, nullptr);
            }
            break;
            case DrawCommandType::LINE:
            {
                auto const& rLineDrawCommand = static_cast<LineDrawCommand const&>(*pDrawCommand);
                Point aRectPoint(nX + 1, nY + 1);

                Size aRectSize(nWidth - 1, nHeight - 1);

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

    long nWidth = rControlRegion.GetWidth() - 1;
    long nHeight = rControlRegion.GetHeight() - 1;
    long nX = rControlRegion.Left();
    long nY = rControlRegion.Top();

    bool bOK = false;

    switch (eType)
    {
        case ControlType::Generic:
        {
        }
        break;
        case ControlType::Pushbutton:
        {
            std::shared_ptr<WidgetDefinitionPart> pPart
                = m_aWidgetDefinition.getDefinition(eType, ePart);
            if (pPart)
            {
                auto aStates = pPart->getStates(eState, rValue);
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
            std::shared_ptr<WidgetDefinitionPart> pPart
                = m_aWidgetDefinition.getDefinition(eType, ePart);
            if (pPart)
            {
                std::shared_ptr<WidgetDefinitionState> pState
                    = pPart->getStates(eState, rValue).back();
                {
                    munchDrawCommands(pState->mpDrawCommands, m_rGraphics, nX, nY, nWidth, nHeight);
                    bOK = true;
                }
            }
        }
        break;
        case ControlType::Checkbox:
        {
            std::shared_ptr<WidgetDefinitionPart> pPart
                = m_aWidgetDefinition.getDefinition(eType, ePart);
            if (pPart)
            {
                auto aStates = pPart->getStates(eState, rValue);
                if (!aStates.empty())
                {
                    std::shared_ptr<WidgetDefinitionState> pState = aStates.back();
                    munchDrawCommands(pState->mpDrawCommands, m_rGraphics, nX, nY, nWidth, nHeight);
                    bOK = true;
                }
            }
        }
        break;
        case ControlType::Combobox:
        {
            std::shared_ptr<WidgetDefinitionPart> pPart
                = m_aWidgetDefinition.getDefinition(eType, ePart);
            if (pPart)
            {
                auto aStates = pPart->getStates(eState, rValue);
                if (!aStates.empty())
                {
                    std::shared_ptr<WidgetDefinitionState> pState = aStates.back();
                    munchDrawCommands(pState->mpDrawCommands, m_rGraphics, nX, nY, nWidth, nHeight);
                    bOK = true;
                }
            }
        }
        break;
        case ControlType::Editbox:
        case ControlType::EditboxNoBorder:
        case ControlType::MultilineEditbox:
        {
            std::shared_ptr<WidgetDefinitionPart> pPart
                = m_aWidgetDefinition.getDefinition(eType, ePart);
            if (pPart)
            {
                std::shared_ptr<WidgetDefinitionState> pState
                    = pPart->getStates(eState, rValue).back();
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
    ControlType eType, ControlPart /*ePart*/, const tools::Rectangle& /*rBoundingControlRegion*/,
    ControlState /*eState*/, const ImplControlValue& /*aValue*/, const OUString& /*aCaption*/,
    tools::Rectangle& /*rNativeBoundingRegion*/, tools::Rectangle& rNativeContentRegion)
{
    switch (eType)
    {
        case ControlType::Checkbox:
            rNativeContentRegion = tools::Rectangle(Point(), Size(48, 32));
            return true;
        case ControlType::Radiobutton:
            rNativeContentRegion = tools::Rectangle(Point(), Size(32, 32));
            return true;
        default:
            break;
    }

    return false;
}

bool FileDefinitionWidgetDraw::updateSettings(AllSettings& rSettings)
{
    StyleSettings aStyleSet = rSettings.GetStyleSettings();

    aStyleSet.SetFaceColor(m_aWidgetDefinition.maFaceColor);
    aStyleSet.SetCheckedColor(m_aWidgetDefinition.maCheckedColor);
    aStyleSet.SetLightColor(m_aWidgetDefinition.maLightColor);
    aStyleSet.SetLightBorderColor(m_aWidgetDefinition.maLightBorderColor);
    aStyleSet.SetShadowColor(m_aWidgetDefinition.maShadowColor);
    aStyleSet.SetDarkShadowColor(m_aWidgetDefinition.maDarkShadowColor);
    aStyleSet.SetButtonTextColor(m_aWidgetDefinition.maButtonTextColor);
    aStyleSet.SetButtonRolloverTextColor(m_aWidgetDefinition.maButtonRolloverTextColor);
    aStyleSet.SetRadioCheckTextColor(m_aWidgetDefinition.maRadioCheckTextColor);
    aStyleSet.SetGroupTextColor(m_aWidgetDefinition.maGroupTextColor);
    aStyleSet.SetLabelTextColor(m_aWidgetDefinition.maLabelTextColor);
    aStyleSet.SetWindowColor(m_aWidgetDefinition.maWindowColor);
    aStyleSet.SetWindowTextColor(m_aWidgetDefinition.maWindowTextColor);
    aStyleSet.SetDialogColor(m_aWidgetDefinition.maDialogColor);
    aStyleSet.SetDialogTextColor(m_aWidgetDefinition.maDialogTextColor);
    aStyleSet.SetWorkspaceColor(m_aWidgetDefinition.maWorkspaceColor);
    aStyleSet.SetMonoColor(m_aWidgetDefinition.maMonoColor);
    aStyleSet.SetFieldColor(m_aWidgetDefinition.maFieldColor);
    aStyleSet.SetFieldTextColor(m_aWidgetDefinition.maFieldTextColor);
    aStyleSet.SetFieldRolloverTextColor(m_aWidgetDefinition.maFieldRolloverTextColor);
    aStyleSet.SetActiveColor(m_aWidgetDefinition.maActiveColor);
    aStyleSet.SetActiveTextColor(m_aWidgetDefinition.maActiveTextColor);
    aStyleSet.SetActiveBorderColor(m_aWidgetDefinition.maActiveBorderColor);
    aStyleSet.SetDeactiveColor(m_aWidgetDefinition.maDeactiveColor);
    aStyleSet.SetDeactiveTextColor(m_aWidgetDefinition.maDeactiveTextColor);
    aStyleSet.SetDeactiveBorderColor(m_aWidgetDefinition.maDeactiveBorderColor);
    aStyleSet.SetMenuColor(m_aWidgetDefinition.maMenuColor);
    aStyleSet.SetMenuBarColor(m_aWidgetDefinition.maMenuBarColor);
    aStyleSet.SetMenuBarRolloverColor(m_aWidgetDefinition.maMenuBarRolloverColor);
    aStyleSet.SetMenuBorderColor(m_aWidgetDefinition.maMenuBorderColor);
    aStyleSet.SetMenuTextColor(m_aWidgetDefinition.maMenuTextColor);
    aStyleSet.SetMenuBarTextColor(m_aWidgetDefinition.maMenuBarTextColor);
    aStyleSet.SetMenuBarRolloverTextColor(m_aWidgetDefinition.maMenuBarRolloverTextColor);
    aStyleSet.SetMenuBarHighlightTextColor(m_aWidgetDefinition.maMenuBarHighlightTextColor);
    aStyleSet.SetMenuHighlightColor(m_aWidgetDefinition.maMenuHighlightColor);
    aStyleSet.SetMenuHighlightTextColor(m_aWidgetDefinition.maMenuHighlightTextColor);
    aStyleSet.SetHighlightColor(m_aWidgetDefinition.maHighlightColor);
    aStyleSet.SetHighlightTextColor(m_aWidgetDefinition.maHighlightTextColor);
    aStyleSet.SetActiveTabColor(m_aWidgetDefinition.maActiveTabColor);
    aStyleSet.SetInactiveTabColor(m_aWidgetDefinition.maInactiveTabColor);
    aStyleSet.SetTabTextColor(m_aWidgetDefinition.maTabTextColor);
    aStyleSet.SetTabRolloverTextColor(m_aWidgetDefinition.maTabRolloverTextColor);
    aStyleSet.SetTabHighlightTextColor(m_aWidgetDefinition.maTabHighlightTextColor);
    aStyleSet.SetDisableColor(m_aWidgetDefinition.maDisableColor);
    aStyleSet.SetHelpColor(m_aWidgetDefinition.maHelpColor);
    aStyleSet.SetHelpTextColor(m_aWidgetDefinition.maHelpTextColor);
    aStyleSet.SetLinkColor(m_aWidgetDefinition.maLinkColor);
    aStyleSet.SetVisitedLinkColor(m_aWidgetDefinition.maVisitedLinkColor);
    aStyleSet.SetToolTextColor(m_aWidgetDefinition.maToolTextColor);
    aStyleSet.SetFontColor(m_aWidgetDefinition.maFontColor);

    rSettings.SetStyleSettings(aStyleSet);

    return true;
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
