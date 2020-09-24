/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cairo.h>
#include <headless/CustomWidgetDraw.hxx>
#include <sal/config.h>
#include <rtl/bootstrap.hxx>
#include <tools/svlibrary.h>
#include <osl/module.hxx>
#include <svdata.hxx>

namespace vcl
{
WidgetThemeLibrary* CustomWidgetDraw::s_pWidgetImplementation = nullptr;

CustomWidgetDraw::CustomWidgetDraw(SvpSalGraphics& rGraphics)
    : m_rGraphics(rGraphics)
{
#ifndef DISABLE_DYNLOADING
    static bool s_bMissingLibrary = false;
    if (s_pWidgetImplementation || s_bMissingLibrary)
        return;

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
    else
        s_bMissingLibrary = true;
#endif
}

CustomWidgetDraw::~CustomWidgetDraw() {}

bool CustomWidgetDraw::isNativeControlSupported(ControlType eType, ControlPart ePart)
{
    return s_pWidgetImplementation
           && s_pWidgetImplementation->isNativeControlSupported(eType, ePart);
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
                                         const OUString& /*aCaptions*/,
                                         const Color& /*rBackgroundColor*/)
{
    if (!s_pWidgetImplementation)
        return false;

    bool bOldAA = m_rGraphics.getAntiAlias();
    m_rGraphics.setAntiAlias(true);

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
                aParameters.bIsAction = pPushButtonValue->mbIsAction;
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

    m_rGraphics.setAntiAlias(bOldAA);

    return bOK;
}

bool CustomWidgetDraw::getNativeControlRegion(
    ControlType eType, ControlPart ePart, const tools::Rectangle& rBoundingControlRegion,
    ControlState eState, const ImplControlValue& /*aValue*/, const OUString& /*aCaption*/,
    tools::Rectangle& rNativeBoundingRegion, tools::Rectangle& rNativeContentRegion)
{
    // Translate to POD rectangle and back.
    const rectangle_t aRegion
        = { rBoundingControlRegion.getX(), rBoundingControlRegion.getY(),
            rBoundingControlRegion.GetWidth(), rBoundingControlRegion.GetHeight() };
    if (s_pWidgetImplementation)
    {
        rectangle_t aNativeBoundingRegion;
        rectangle_t aNativeContentRegion;
        s_pWidgetImplementation->getRegion(eType, ePart, eState, aRegion, aNativeBoundingRegion,
                                           aNativeContentRegion);

        rNativeBoundingRegion
            = tools::Rectangle(aNativeBoundingRegion.x, aNativeBoundingRegion.y,
                               aNativeBoundingRegion.width, aNativeBoundingRegion.height);
        rNativeContentRegion
            = tools::Rectangle(aNativeBoundingRegion.x, aNativeBoundingRegion.y,
                               aNativeBoundingRegion.width, aNativeBoundingRegion.height);
    }

    return false;
}

bool CustomWidgetDraw::updateSettings(AllSettings& rSettings)
{
    if (!s_pWidgetImplementation)
        return false;

    WidgetDrawStyle aStyle;
    aStyle.nSize = sizeof(WidgetDrawStyle);

    if (s_pWidgetImplementation->updateSettings(aStyle))
    {
        StyleSettings aStyleSet = rSettings.GetStyleSettings();

        aStyleSet.SetFaceColor(aStyle.maFaceColor);
        aStyleSet.SetCheckedColor(aStyle.maCheckedColor);
        aStyleSet.SetLightColor(aStyle.maLightColor);
        aStyleSet.SetLightBorderColor(aStyle.maLightBorderColor);
        aStyleSet.SetShadowColor(aStyle.maShadowColor);
        aStyleSet.SetDarkShadowColor(aStyle.maDarkShadowColor);
        aStyleSet.SetDefaultButtonTextColor(aStyle.maDefaultButtonTextColor);
        aStyleSet.SetButtonTextColor(aStyle.maButtonTextColor);
        aStyleSet.SetDefaultActionButtonTextColor(aStyle.maDefaultActionButtonTextColor);
        aStyleSet.SetActionButtonTextColor(aStyle.maActionButtonTextColor);
        aStyleSet.SetFlatButtonTextColor(aStyle.maFlatButtonTextColor);
        aStyleSet.SetDefaultButtonRolloverTextColor(aStyle.maDefaultButtonRolloverTextColor);
        aStyleSet.SetButtonRolloverTextColor(aStyle.maButtonRolloverTextColor);
        aStyleSet.SetDefaultActionButtonRolloverTextColor(
            aStyle.maDefaultActionButtonRolloverTextColor);
        aStyleSet.SetActionButtonRolloverTextColor(aStyle.maActionButtonRolloverTextColor);
        aStyleSet.SetFlatButtonRolloverTextColor(aStyle.maFlatButtonRolloverTextColor);
        aStyleSet.SetDefaultButtonPressedRolloverTextColor(
            aStyle.maDefaultButtonPressedRolloverTextColor);
        aStyleSet.SetButtonPressedRolloverTextColor(aStyle.maButtonPressedRolloverTextColor);
        aStyleSet.SetDefaultActionButtonPressedRolloverTextColor(
            aStyle.maDefaultActionButtonPressedRolloverTextColor);
        aStyleSet.SetActionButtonPressedRolloverTextColor(
            aStyle.maActionButtonPressedRolloverTextColor);
        aStyleSet.SetFlatButtonPressedRolloverTextColor(
            aStyle.maFlatButtonPressedRolloverTextColor);
        aStyleSet.SetRadioCheckTextColor(aStyle.maRadioCheckTextColor);
        aStyleSet.SetGroupTextColor(aStyle.maGroupTextColor);
        aStyleSet.SetLabelTextColor(aStyle.maLabelTextColor);
        aStyleSet.SetWindowColor(aStyle.maWindowColor);
        aStyleSet.SetWindowTextColor(aStyle.maWindowTextColor);
        aStyleSet.SetDialogColor(aStyle.maDialogColor);
        aStyleSet.SetDialogTextColor(aStyle.maDialogTextColor);
        aStyleSet.SetWorkspaceColor(aStyle.maWorkspaceColor);
        aStyleSet.SetMonoColor(aStyle.maMonoColor);
        aStyleSet.SetFieldColor(Color(aStyle.maFieldColor));
        aStyleSet.SetFieldTextColor(aStyle.maFieldTextColor);
        aStyleSet.SetFieldRolloverTextColor(aStyle.maFieldRolloverTextColor);
        aStyleSet.SetActiveColor(aStyle.maActiveColor);
        aStyleSet.SetActiveTextColor(aStyle.maActiveTextColor);
        aStyleSet.SetActiveBorderColor(aStyle.maActiveBorderColor);
        aStyleSet.SetDeactiveColor(aStyle.maDeactiveColor);
        aStyleSet.SetDeactiveTextColor(aStyle.maDeactiveTextColor);
        aStyleSet.SetDeactiveBorderColor(aStyle.maDeactiveBorderColor);
        aStyleSet.SetMenuColor(aStyle.maMenuColor);
        aStyleSet.SetMenuBarColor(aStyle.maMenuBarColor);
        aStyleSet.SetMenuBarRolloverColor(aStyle.maMenuBarRolloverColor);
        aStyleSet.SetMenuBorderColor(aStyle.maMenuBorderColor);
        aStyleSet.SetMenuTextColor(aStyle.maMenuTextColor);
        aStyleSet.SetMenuBarTextColor(aStyle.maMenuBarTextColor);
        aStyleSet.SetMenuBarRolloverTextColor(aStyle.maMenuBarRolloverTextColor);
        aStyleSet.SetMenuBarHighlightTextColor(aStyle.maMenuBarHighlightTextColor);
        aStyleSet.SetMenuHighlightColor(aStyle.maMenuHighlightColor);
        aStyleSet.SetMenuHighlightTextColor(aStyle.maMenuHighlightTextColor);
        aStyleSet.SetHighlightColor(aStyle.maHighlightColor);
        aStyleSet.SetHighlightTextColor(aStyle.maHighlightTextColor);
        aStyleSet.SetActiveTabColor(aStyle.maActiveTabColor);
        aStyleSet.SetInactiveTabColor(aStyle.maInactiveTabColor);
        aStyleSet.SetTabTextColor(aStyle.maTabTextColor);
        aStyleSet.SetTabRolloverTextColor(aStyle.maTabRolloverTextColor);
        aStyleSet.SetTabHighlightTextColor(aStyle.maTabHighlightTextColor);
        aStyleSet.SetDisableColor(aStyle.maDisableColor);
        aStyleSet.SetHelpColor(aStyle.maHelpColor);
        aStyleSet.SetHelpTextColor(aStyle.maHelpTextColor);
        aStyleSet.SetLinkColor(aStyle.maLinkColor);
        aStyleSet.SetVisitedLinkColor(aStyle.maVisitedLinkColor);
        aStyleSet.SetToolTextColor(aStyle.maToolTextColor);
        aStyleSet.SetFontColor(aStyle.maFontColor);

        rSettings.SetStyleSettings(aStyleSet);

        return true;
    }

    return false;
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
