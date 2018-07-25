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
    }
}

CustomWidgetDraw::~CustomWidgetDraw() {}

bool CustomWidgetDraw::isNativeControlSupported(ControlType eType, ControlPart /*ePart*/)
{
    switch (eType)
    {
        case ControlType::Pushbutton:
            return true;
        default:
            break;
    }

    return false;
}

bool CustomWidgetDraw::hitTestNativeControl(ControlType /*eType*/, ControlPart /*ePart*/,
                                            const tools::Rectangle& /*rBoundingControlRegion*/,
                                            const Point& /*aPos*/, bool& /*rIsInside*/)
{
    //printf ("CustomWidgetDraw::hitTestNativeControl\n");
    return false;
}

bool CustomWidgetDraw::drawNativeControl(ControlType eType, ControlPart ePart,
                                         const tools::Rectangle& rControlRegion,
                                         ControlState /*eState*/,
                                         const ImplControlValue& /*aValue*/,
                                         const OUString& /*aCaptions*/)
{
    printf("CustomWidgetDraw::drawNativeControl %d %d\n", sal_Int32(eType), sal_Int32(ePart));
    if (s_pWidgetImplementation == nullptr)
        return false;

    switch (eType)
    {
        case ControlType::Generic:
            break;
        case ControlType::Pushbutton:
        {
            printf("drawNativeControl -- ControlType::Pushbutton\n");

            cairo_t* pCairoContext = m_rGraphics.getCairoContext(false);
            m_rGraphics.clipRegion(pCairoContext);
            cairo_translate(pCairoContext, rControlRegion.Left(), rControlRegion.Top());

            long nX = 0;
            long nY = 0;
            long nWidth = rControlRegion.GetWidth();
            long nHeight = rControlRegion.GetHeight();

            if (ePart == ControlPart::Focus)
                s_pWidgetImplementation->drawPushButtonFocus(pCairoContext, nX, nY, nWidth,
                                                             nHeight);
            else
                s_pWidgetImplementation->drawPushButton(pCairoContext, nX, nY, nWidth, nHeight);

            cairo_destroy(pCairoContext); // unref

            return true;
        }
        break;
        case ControlType::Radiobutton:
            break;
        case ControlType::Checkbox:
            break;
        case ControlType::Combobox:
            break;
        case ControlType::Editbox:
            break;
        case ControlType::EditboxNoBorder:
            break;
        case ControlType::MultilineEditbox:
            break;
        case ControlType::Listbox:
            break;
        case ControlType::Spinbox:
            break;
        case ControlType::SpinButtons:
            break;
        case ControlType::TabItem:
            break;
        case ControlType::TabPane:
            break;
        case ControlType::TabHeader:
            break;
        case ControlType::TabBody:
            break;
        case ControlType::Scrollbar:
            break;
        case ControlType::Slider:
            break;
        case ControlType::Fixedline:
            break;
        case ControlType::Toolbar:
            break;
        case ControlType::Menubar:
            break;
        case ControlType::MenuPopup:
            break;
        case ControlType::Progress:
            break;
        case ControlType::IntroProgress:
            break;
        case ControlType::Tooltip:
            break;
        case ControlType::WindowBackground:
            break;
        case ControlType::Frame:
            break;
        case ControlType::ListNode:
            break;
        case ControlType::ListNet:
            break;
        case ControlType::ListHeader:
            break;
    }
    return false;
}

bool CustomWidgetDraw::getNativeControlRegion(ControlType /*eType*/, ControlPart /*ePart*/,
                                              const tools::Rectangle& /*rBoundingControlRegion*/,
                                              ControlState /*eState*/,
                                              const ImplControlValue& /*aValue*/,
                                              const OUString& /*aCaption*/,
                                              tools::Rectangle& /*rNativeBoundingRegion*/,
                                              tools::Rectangle& /*rNativeContentRegion*/)
{
    //printf ("CustomWidgetDraw::getNativeControlRegion\n");
    return false;
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
