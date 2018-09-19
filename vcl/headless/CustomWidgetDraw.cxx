/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <headless/CustomWidgetDraw.hxx>
#include <vcl/salnativewidgets.hxx>
#include <sal/main.h>
#include <sal/config.h>
#include <rtl/bootstrap.hxx>
#include <tools/svlibrary.h>
#include <osl/module.hxx>
#include <cairo.h>
#include <svdata.hxx>

namespace vcl
{
WidgetThemeLibrary* CustomWidgetDraw::s_pWidgetImplementation = nullptr;

CustomWidgetDraw::CustomWidgetDraw(SvpSalGraphics& rGraphics)
    : m_rGraphics(rGraphics)
{
    if (!s_pWidgetImplementation)
    {
        OUString aUrl("${LO_LIB_DIR}/" SVLIBRARY("vcl_widget_theme"));
        rtl::Bootstrap::expandMacros(aUrl);
        osl::Module aLibrary;
        aLibrary.load(aUrl, SAL_LOADMODULE_GLOBAL);
        auto fCreateWidgetThemeLibraryFunction
            = reinterpret_cast<vcl::WidgetThemeLibrary*(SAL_CALL*)()>(
                aLibrary.getFunctionSymbol("CreateWidgetThemeLibrary"));
        aLibrary.release();

        if (fCreateWidgetThemeLibraryFunction)
            s_pWidgetImplementation = (*fCreateWidgetThemeLibraryFunction)();

        // Init
        if (s_pWidgetImplementation)
        {
            ImplSVData* pSVData = ImplGetSVData();
            pSVData->maNWFData.mbNoFocusRects = true;
            pSVData->maNWFData.mbNoFocusRectsForFlatButtons = true;
        }
    }
}

CustomWidgetDraw::~CustomWidgetDraw() {}

bool CustomWidgetDraw::isNativeControlSupported(ControlType eType, ControlPart ePart)
{
    if (!s_pWidgetImplementation)
        return false;
    return s_pWidgetImplementation->isNativeControlSupported(eType, ePart);
}

bool CustomWidgetDraw::hitTestNativeControl(ControlType /*eType*/, ControlPart /*ePart*/,
                                            const tools::Rectangle& /*rBoundingControlRegion*/,
                                            const Point& /*aPos*/, bool& /*rIsInside*/)
{
    return false;
}

bool CustomWidgetDraw::drawNativeControl(ControlType eType, ControlPart ePart,
                                         const tools::Rectangle& rControlRegion,
                                         ControlState eState, const ImplControlValue& rValue,
                                         const OUString& /*aCaptions*/)
{
    if (!s_pWidgetImplementation)
        return false;

    bool bOldAA = m_rGraphics.getAntiAliasB2DDraw();
    m_rGraphics.setAntiAliasB2DDraw(true);

    cairo_t* pCairoContext = m_rGraphics.getCairoContext(false);
    m_rGraphics.clipRegion(pCairoContext);

    cairo_translate(pCairoContext, rControlRegion.Left(), rControlRegion.Top());

    long nWidth = rControlRegion.GetWidth();
    long nHeight = rControlRegion.GetHeight();

    bool bOK = false;

    ControlDrawParameters aParameters{ pCairoContext, ePart, eState };

    switch (eType)
    {
        case ControlType::Generic:
        {
        }
        break;
        case ControlType::Pushbutton:
        {
            const PushButtonValue* pPushButtonValue = static_cast<const PushButtonValue*>(&rValue);
            if (pPushButtonValue)
                aParameters.bIsStock = pPushButtonValue->mbIsStock;
            bOK = s_pWidgetImplementation->drawPushButton(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::Radiobutton:
        {
            aParameters.eButtonValue = rValue.getTristateVal();
            bOK = s_pWidgetImplementation->drawRadiobutton(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::Checkbox:
        {
            aParameters.eButtonValue = rValue.getTristateVal();
            bOK = s_pWidgetImplementation->drawCheckbox(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::Combobox:
        {
            bOK = s_pWidgetImplementation->drawCombobox(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::Editbox:
        {
            bOK = s_pWidgetImplementation->drawEditbox(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::EditboxNoBorder:
        {
            bOK = s_pWidgetImplementation->drawEditbox(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::MultilineEditbox:
        {
            bOK = s_pWidgetImplementation->drawEditbox(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::Listbox:
        {
            bOK = s_pWidgetImplementation->drawListbox(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::Spinbox:
        {
            if (rValue.getType() == ControlType::SpinButtons)
            {
                const SpinbuttonValue* pSpinVal = static_cast<const SpinbuttonValue*>(&rValue);

                ControlPart upBtnPart = pSpinVal->mnUpperPart;
                ControlState upBtnState = pSpinVal->mnUpperState;

                ControlPart downBtnPart = pSpinVal->mnLowerPart;
                ControlState downBtnState = pSpinVal->mnLowerState;
                {
                    ControlDrawParameters aParametersUp{ pCairoContext, upBtnPart, upBtnState };
                    cairo_save(pCairoContext);
                    cairo_translate(pCairoContext,
                                    pSpinVal->maUpperRect.Left() - rControlRegion.Left(),
                                    pSpinVal->maUpperRect.Top() - rControlRegion.Top());
                    bOK = s_pWidgetImplementation->drawSpinbox(aParametersUp,
                                                               pSpinVal->maUpperRect.GetWidth(),
                                                               pSpinVal->maUpperRect.GetHeight());
                    cairo_restore(pCairoContext);
                }

                if (bOK)
                {
                    ControlDrawParameters aParametersDown{ pCairoContext, downBtnPart,
                                                           downBtnState };
                    cairo_save(pCairoContext);
                    cairo_translate(pCairoContext,
                                    pSpinVal->maLowerRect.Left() - rControlRegion.Left(),
                                    pSpinVal->maLowerRect.Top() - rControlRegion.Top());
                    bOK = s_pWidgetImplementation->drawSpinbox(aParametersDown,
                                                               pSpinVal->maLowerRect.GetWidth(),
                                                               pSpinVal->maLowerRect.GetHeight());
                    cairo_restore(pCairoContext);
                }
            }
            else
            {
                bOK = s_pWidgetImplementation->drawSpinbox(aParameters, nWidth, nHeight);
            }
        }
        break;
        case ControlType::SpinButtons:
        {
            bOK = s_pWidgetImplementation->drawSpinButtons(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::TabItem:
        {
            bOK = s_pWidgetImplementation->drawTabItem(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::TabPane:
        {
            bOK = s_pWidgetImplementation->drawTabPane(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::TabHeader:
        {
            bOK = s_pWidgetImplementation->drawTabHeader(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::TabBody:
        {
            bOK = s_pWidgetImplementation->drawTabBody(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::Scrollbar:
        {
            bOK = s_pWidgetImplementation->drawScrollbar(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::Slider:
        {
            cairo_save(pCairoContext);
            bOK = s_pWidgetImplementation->drawSlider(aParameters, nWidth, nHeight);
            cairo_restore(pCairoContext);

            if (bOK)
            {
                const SliderValue* pSliderValue = static_cast<const SliderValue*>(&rValue);

                ControlDrawParameters aParametersButton{ pCairoContext, ControlPart::Button,
                                                         eState | pSliderValue->mnThumbState };
                cairo_save(pCairoContext);
                cairo_translate(pCairoContext,
                                pSliderValue->maThumbRect.Left() - rControlRegion.Left(),
                                pSliderValue->maThumbRect.Top() - rControlRegion.Top());
                bOK = s_pWidgetImplementation->drawSlider(aParametersButton,
                                                          pSliderValue->maThumbRect.GetWidth(),
                                                          pSliderValue->maThumbRect.GetHeight());
                cairo_restore(pCairoContext);
            }
        }
        break;
        case ControlType::Fixedline:
        {
            bOK = s_pWidgetImplementation->drawFixedline(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::Toolbar:
        {
            bOK = s_pWidgetImplementation->drawToolbar(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::Menubar:
            break;
        case ControlType::MenuPopup:
            break;
        case ControlType::Progress:
        {
            aParameters.nValue = rValue.getNumericVal();
            bOK = s_pWidgetImplementation->drawProgress(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::IntroProgress:
            break;
        case ControlType::Tooltip:
            break;
        case ControlType::WindowBackground:
        {
            bOK = s_pWidgetImplementation->drawWindowsBackground(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::Frame:
        {
            bOK = s_pWidgetImplementation->drawFrame(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::ListNode:
        {
            aParameters.eButtonValue = rValue.getTristateVal();
            bOK = s_pWidgetImplementation->drawListNode(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::ListNet:
        {
            bOK = s_pWidgetImplementation->drawListNet(aParameters, nWidth, nHeight);
        }
        break;
        case ControlType::ListHeader:
        {
            bOK = s_pWidgetImplementation->drawListHeader(aParameters, nWidth, nHeight);
        }
        break;
    }

    basegfx::B2DRange aExtents(rControlRegion.Left(), rControlRegion.Top(), rControlRegion.Right(),
                               rControlRegion.Bottom());

    m_rGraphics.releaseCairoContext(pCairoContext, true, aExtents);

    m_rGraphics.setAntiAliasB2DDraw(bOldAA);

    return bOK;
}

bool CustomWidgetDraw::getNativeControlRegion(
    ControlType eType, ControlPart ePart, const tools::Rectangle& rBoundingControlRegion,
    ControlState eState, const ImplControlValue& /*aValue*/, const OUString& /*aCaption*/,
    tools::Rectangle& rNativeBoundingRegion, tools::Rectangle& rNativeContentRegion)
{
    if (!s_pWidgetImplementation)
        return false;

    return s_pWidgetImplementation->getRegion(eType, ePart, eState, rBoundingControlRegion,
                                              rNativeBoundingRegion, rNativeContentRegion);
}

bool CustomWidgetDraw::updateSettings(AllSettings& rSettings)
{
    if (!s_pWidgetImplementation)
        return false;

    WidgetDrawStyle aStyle;

    if (s_pWidgetImplementation->updateSettings(aStyle))
    {
        StyleSettings aStyleSet = rSettings.GetStyleSettings();

        aStyleSet.SetFaceColor(aStyle.maFaceColor);
        aStyleSet.SetCheckedColor(Color(0xCC, 0xCC, 0xCC));
        aStyleSet.SetLightColor(aStyle.maLightColor);
        aStyleSet.SetLightBorderColor(aStyle.maLightBorderColor);
        aStyleSet.SetShadowColor(aStyle.maShadowColor);
        aStyleSet.SetDarkShadowColor(aStyle.maDarkShadowColor);
        aStyleSet.SetButtonTextColor(Color(COL_BLACK));
        aStyleSet.SetButtonRolloverTextColor(Color(COL_BLACK));
        aStyleSet.SetRadioCheckTextColor(Color(COL_BLACK));
        aStyleSet.SetGroupTextColor(Color(COL_BLACK));
        aStyleSet.SetLabelTextColor(Color(COL_BLACK));
        aStyleSet.SetWindowColor(aStyle.maWindowColor);
        aStyleSet.SetWindowTextColor(aStyle.maWindowTextColor);
        aStyleSet.SetDialogColor(aStyle.maDialogColor);
        aStyleSet.SetDialogTextColor(aStyle.maDialogTextColor);
        aStyleSet.SetWorkspaceColor(Color(0xDF, 0xDF, 0xDE));
        aStyleSet.SetMonoColor(Color(COL_BLACK));
        aStyleSet.SetFieldColor(Color(COL_WHITE));
        aStyleSet.SetFieldTextColor(Color(COL_BLACK));
        aStyleSet.SetFieldRolloverTextColor(Color(COL_BLACK));
        aStyleSet.SetActiveColor(Color(COL_BLUE));
        aStyleSet.SetActiveTextColor(Color(COL_WHITE));
        aStyleSet.SetActiveBorderColor(Color(COL_LIGHTGRAY));
        aStyleSet.SetDeactiveColor(Color(COL_GRAY));
        aStyleSet.SetDeactiveTextColor(Color(COL_LIGHTGRAY));
        aStyleSet.SetDeactiveBorderColor(Color(COL_LIGHTGRAY));
        aStyleSet.SetMenuColor(Color(COL_LIGHTGRAY));
        aStyleSet.SetMenuBarColor(Color(COL_LIGHTGRAY));
        aStyleSet.SetMenuBarRolloverColor(Color(COL_BLUE));
        aStyleSet.SetMenuBorderColor(Color(COL_LIGHTGRAY));
        aStyleSet.SetMenuTextColor(Color(COL_BLACK));
        aStyleSet.SetMenuBarTextColor(Color(COL_BLACK));
        aStyleSet.SetMenuBarRolloverTextColor(Color(COL_WHITE));
        aStyleSet.SetMenuBarHighlightTextColor(Color(COL_WHITE));
        aStyleSet.SetMenuHighlightColor(Color(COL_BLUE));
        aStyleSet.SetMenuHighlightTextColor(Color(COL_WHITE));
        aStyleSet.SetHighlightColor(aStyle.maHighlightColor);
        aStyleSet.SetHighlightTextColor(aStyle.maHighlightTextColor);
        aStyleSet.SetActiveTabColor(aStyle.maActiveTabColor);
        aStyleSet.SetInactiveTabColor(aStyle.maInactiveTabColor);
        aStyleSet.SetTabTextColor(Color(COL_BLACK));
        aStyleSet.SetTabRolloverTextColor(Color(COL_BLACK));
        aStyleSet.SetTabHighlightTextColor(Color(COL_BLACK));
        aStyleSet.SetDisableColor(Color(COL_GRAY));
        aStyleSet.SetHelpColor(Color(0xFF, 0xFF, 0xE0));
        aStyleSet.SetHelpTextColor(Color(COL_BLACK));
        aStyleSet.SetLinkColor(Color(COL_BLUE));
        aStyleSet.SetVisitedLinkColor(Color(0x00, 0x00, 0xCC));
        aStyleSet.SetToolTextColor(Color(COL_BLACK));
        aStyleSet.SetFontColor(Color(COL_BLACK));

        rSettings.SetStyleSettings(aStyleSet);

        return true;
    }

    return false;
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
