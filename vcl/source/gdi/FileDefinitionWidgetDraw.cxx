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

#include <tools/stream.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapTools.hxx>

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
    OUString sDefinitionBasePath = lcl_getThemeDefinitionPath();
    WidgetDefinitionReader aReader(sDefinitionBasePath + "definition.xml", sDefinitionBasePath);
    aReader.read(m_aWidgetDefinition);

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maNWFData.mbNoFocusRects = true;
    pSVData->maNWFData.mbNoFocusRectsForFlatButtons = true;
    pSVData->maNWFData.mbNoActiveTabTextRaise = true;
    pSVData->maNWFData.mbCenteredTabs = true;
    pSVData->maNWFData.mbProgressNeedsErase = true;
    pSVData->maNWFData.mnStatusBarLowerRightOffset = 10;
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
            if (ePart == ControlPart::HasBackgroundTexture)
                return false;
            return true;
        case ControlType::Spinbox:
            if (ePart == ControlPart::AllButtons)
                return false;
            return true;
        case ControlType::SpinButtons:
            return false;
        case ControlType::TabItem:
        case ControlType::TabPane:
        case ControlType::TabHeader:
        case ControlType::TabBody:
            return true;
        case ControlType::Scrollbar:
            if (ePart == ControlPart::DrawBackgroundHorz
                || ePart == ControlPart::DrawBackgroundVert)
                return false;
            return true;
        case ControlType::Slider:
        case ControlType::Fixedline:
            return true;
        case ControlType::Toolbar:
        case ControlType::Menubar:
        case ControlType::MenuPopup:
            return false;
        case ControlType::Progress:
        case ControlType::IntroProgress:
            return true;
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
            case DrawCommandType::IMAGE:
            {
                auto const& rDrawCommand = static_cast<ImageDrawCommand const&>(*pDrawCommand);
                SvFileStream aFileStream(rDrawCommand.msSource, StreamMode::READ);
                BitmapEx aBitmap;
                vcl::bitmap::loadFromSvg(aFileStream, "", aBitmap, 1.0);
                long nImageWidth = aBitmap.GetSizePixel().Width();
                long nImageHeight = aBitmap.GetSizePixel().Height();
                SalTwoRect aTR(0, 0, nImageWidth, nImageHeight, nX, nY, nImageWidth, nImageHeight);
                rGraphics.DrawBitmap(aTR, *aBitmap.GetBitmap().ImplGetSalBitmap().get(),
                                     *aBitmap.GetAlpha().ImplGetSalBitmap().get(), nullptr);
            }
            break;
        }
    }
}

} // end anonymous namespace

bool FileDefinitionWidgetDraw::resolveDefinition(ControlType eType, ControlPart ePart,
                                                 ControlState eState,
                                                 const ImplControlValue& rValue, long nX, long nY,
                                                 long nWidth, long nHeight)
{
    bool bOK = false;
    auto const& pPart = m_aWidgetDefinition.getDefinition(eType, ePart);
    if (pPart)
    {
        auto const& aStates = pPart->getStates(eState, rValue);
        if (!aStates.empty())
        {
            // use last defined state
            auto const& pState = aStates.back();
            {
                munchDrawCommands(pState->mpDrawCommands, m_rGraphics, nX, nY, nWidth, nHeight);
                bOK = true;
            }
        }
    }
    return bOK;
}

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
            /*bool bIsAction = false;
            const PushButtonValue* pPushButtonValue = static_cast<const PushButtonValue*>(&rValue);
            if (pPushButtonValue)
                bIsAction = pPushButtonValue->mbIsAction;*/

            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Radiobutton:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Checkbox:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Combobox:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Editbox:
        case ControlType::EditboxNoBorder:
        case ControlType::MultilineEditbox:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Listbox:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Spinbox:
        {
            if (rValue.getType() == ControlType::SpinButtons)
            {
                const SpinbuttonValue* pSpinVal = static_cast<const SpinbuttonValue*>(&rValue);

                {
                    ControlPart eUpButtonPart = pSpinVal->mnUpperPart;
                    ControlState eUpButtonState = pSpinVal->mnUpperState;

                    long nUpperX = pSpinVal->maUpperRect.Left();
                    long nUpperY = pSpinVal->maUpperRect.Top();
                    long nUpperWidth = pSpinVal->maUpperRect.GetWidth() - 1;
                    long nUpperHeight = pSpinVal->maUpperRect.GetHeight() - 1;

                    bOK = resolveDefinition(eType, eUpButtonPart, eUpButtonState,
                                            ImplControlValue(), nUpperX, nUpperY, nUpperWidth,
                                            nUpperHeight);
                }

                if (bOK)
                {
                    ControlPart eDownButtonPart = pSpinVal->mnLowerPart;
                    ControlState eDownButtonState = pSpinVal->mnLowerState;

                    long nLowerX = pSpinVal->maLowerRect.Left();
                    long nLowerY = pSpinVal->maLowerRect.Top();
                    long nLowerWidth = pSpinVal->maLowerRect.GetWidth() - 1;
                    long nLowerHeight = pSpinVal->maLowerRect.GetHeight() - 1;

                    bOK = resolveDefinition(eType, eDownButtonPart, eDownButtonState,
                                            ImplControlValue(), nLowerX, nLowerY, nLowerWidth,
                                            nLowerHeight);
                }
            }
            else
            {
                bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
            }
        }
        break;
        case ControlType::SpinButtons:
            break;
        case ControlType::TabItem:
        case ControlType::TabHeader:
        case ControlType::TabPane:
        case ControlType::TabBody:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Scrollbar:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Slider:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);

            if (bOK)
            {
                const SliderValue* pSliderValue = static_cast<const SliderValue*>(&rValue);

                long nThumbX = pSliderValue->maThumbRect.Left();
                long nThumbY = pSliderValue->maThumbRect.Top();
                long nThumbWidth = pSliderValue->maThumbRect.GetWidth() - 1;
                long nThumbHeight = pSliderValue->maThumbRect.GetHeight() - 1;

                bOK = resolveDefinition(eType, ControlPart::Button,
                                        eState | pSliderValue->mnThumbState, rValue, nThumbX,
                                        nThumbY, nThumbWidth, nThumbHeight);
            }
        }
        break;
        case ControlType::Fixedline:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Toolbar:
        case ControlType::Menubar:
            break;
        case ControlType::MenuPopup:
            break;
        case ControlType::Progress:
        case ControlType::IntroProgress:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
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
    ControlType eType, ControlPart ePart, const tools::Rectangle& rBoundingControlRegion,
    ControlState /*eState*/, const ImplControlValue& /*aValue*/, const OUString& /*aCaption*/,
    tools::Rectangle& rNativeBoundingRegion, tools::Rectangle& rNativeContentRegion)
{
    switch (eType)
    {
        case ControlType::Spinbox:
        {
            Size aButtonSize(32, 32);
            Point aLocation(rBoundingControlRegion.TopLeft());

            if (ePart == ControlPart::ButtonUp)
            {
                rNativeContentRegion = tools::Rectangle(
                    Point(aLocation.X() + rBoundingControlRegion.GetWidth() - aButtonSize.Width(),
                          aLocation.Y()),
                    aButtonSize);
                rNativeBoundingRegion = rNativeContentRegion;
                return true;
            }
            else if (ePart == ControlPart::ButtonDown)
            {
                rNativeContentRegion
                    = tools::Rectangle(Point(aLocation.X() + rBoundingControlRegion.GetWidth()
                                                 - (2 * aButtonSize.Width()),
                                             aLocation.Y()),
                                       aButtonSize);
                rNativeBoundingRegion = rNativeContentRegion;
                return true;
            }
            else if (ePart == ControlPart::SubEdit)
            {
                rNativeContentRegion = tools::Rectangle(
                    aLocation, Size(rBoundingControlRegion.GetWidth() - (2 * aButtonSize.Width()),
                                    aButtonSize.Height()));
                rNativeBoundingRegion = rNativeContentRegion;
                return true;
            }
            else if (ePart == ControlPart::Entire)
            {
                rNativeContentRegion = tools::Rectangle(
                    aLocation, Size(rBoundingControlRegion.GetWidth(), aButtonSize.Height()));
                rNativeBoundingRegion = rNativeContentRegion;
                return true;
            }
        }
        break;
        case ControlType::Checkbox:
            rNativeContentRegion = tools::Rectangle(Point(), Size(44, 26));
            return true;
        case ControlType::Radiobutton:
            rNativeContentRegion = tools::Rectangle(Point(), Size(32, 32));
            return true;
        case ControlType::TabItem:
        {
            rNativeBoundingRegion = tools::Rectangle(rBoundingControlRegion.TopLeft(),
                                                     Size(rBoundingControlRegion.GetWidth() + 20,
                                                          rBoundingControlRegion.GetHeight() + 6));
            rNativeContentRegion = rNativeBoundingRegion;
            return true;
        }
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
