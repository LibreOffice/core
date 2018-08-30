/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_WIDGETTHEME_HXX
#define INCLUDED_VCL_INC_WIDGETTHEME_HXX

#include <cairo.h>
#include <vcl/dllapi.h>
#include <vcl/salnativewidgets.hxx>

namespace vcl
{
struct ControlDrawParameters
{
    ControlDrawParameters(cairo_t* i_pCairo, ControlPart i_ePart, ControlState i_eState)
        : pCairo(i_pCairo)
        , ePart(i_ePart)
        , eState(i_eState)
        , eButtonValue(ButtonValue::DontKnow)
        , nValue(0)
    {
    }

    cairo_t* pCairo;
    ControlPart ePart;
    ControlState eState;
    ButtonValue eButtonValue;
    sal_Int64 nValue;
};

class SAL_DLLPUBLIC_RTTI WidgetThemeLibrary
{
public:
    WidgetThemeLibrary();
    virtual ~WidgetThemeLibrary();

    virtual bool isNativeControlSupported(ControlType eType, ControlPart ePart);
    virtual bool getRegion(ControlType eType, ControlPart ePart, ControlState eState,
                           const tools::Rectangle& rBoundingControlRegion,
                           tools::Rectangle& rNativeBoundingRegion,
                           tools::Rectangle& rNativeContentRegion);

    virtual bool drawPushButton(ControlDrawParameters const& rParameters, long nWidth,
                                long nHeight);
    virtual bool drawRadiobutton(ControlDrawParameters const& rParameters, long nWidth,
                                 long nHeight);
    virtual bool drawCheckbox(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawCombobox(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawEditbox(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawScrollbar(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawSpinButtons(ControlDrawParameters const& rParameters, long nWidth,
                                 long nHeight);
    virtual bool drawSpinbox(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawTabItem(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawTabPane(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawTabHeader(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawTabBody(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawSlider(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawToolbar(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawProgress(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawWindowsBackground(ControlDrawParameters const& rParameters, long nWidth,
                                       long nHeight);
    virtual bool drawListbox(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawFrame(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawListNode(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawListNet(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    virtual bool drawListHeader(ControlDrawParameters const& rParameters, long nWidth,
                                long nHeight);
};

extern "C" vcl::WidgetThemeLibrary* CreateWidgetThemeLibrary();

} // end vcl namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
