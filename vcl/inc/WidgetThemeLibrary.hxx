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

/**
 * This file expects cairo.h and the definition of
 * the other types used here to be defined before
 * including it.
 */

namespace vcl
{
struct WidgetDrawStyle
{
    Color maFaceColor;
    Color maLightColor;
    Color maLightBorderColor;
    Color maShadowColor;
    Color maDarkShadowColor;
    Color maHighlightColor;
    Color maHighlightTextColor;
    Color maActiveTabColor;
    Color maInactiveTabColor;
    Color maWindowColor;
    Color maWindowTextColor;
    Color maDialogColor;
    Color maDialogTextColor;
};

struct ControlDrawParameters
{
    ControlDrawParameters(cairo_t* i_pCairo, ControlPart i_ePart, ControlState i_eState)
        : pCairo(i_pCairo)
        , ePart(i_ePart)
        , eState(i_eState)
        , eButtonValue(ButtonValue::DontKnow)
        , bIsStock(false)
        , nValue(0)
    {
    }

    cairo_t* pCairo;
    ControlPart ePart;
    ControlState eState;
    ButtonValue eButtonValue;
    bool bIsStock;
    int64_t nValue;
};

#ifndef SAL_DLLPUBLIC_RTTI
#define SAL_DLLPUBLIC_RTTI
#endif

typedef struct _WidgetThemeLibrary WidgetThemeLibrary;

struct _WidgetThemeLibrary
{
    uint32_t nSize;

    bool (*isNativeControlSupported)(ControlType eType, ControlPart ePart);
    bool (*getRegion)(ControlType eType, ControlPart ePart, ControlState eState,
                      const tools::Rectangle& rBoundingControlRegion,
                      tools::Rectangle& rNativeBoundingRegion,
                      tools::Rectangle& rNativeContentRegion);

    bool (*drawPushButton)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawRadiobutton)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawCheckbox)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawCombobox)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawEditbox)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawScrollbar)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawSpinButtons)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawSpinbox)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawTabItem)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawTabPane)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawTabHeader)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawTabBody)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawSlider)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawFixedline)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawToolbar)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawProgress)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawWindowsBackground)(ControlDrawParameters const& rParameters, long nWidth,
                                  long nHeight);
    bool (*drawListbox)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawFrame)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawListNode)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawListNet)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);
    bool (*drawListHeader)(ControlDrawParameters const& rParameters, long nWidth, long nHeight);

    bool (*updateSettings)(WidgetDrawStyle& rSettings);
};

extern "C" vcl::WidgetThemeLibrary* CreateWidgetThemeLibrary();

} // end vcl namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
