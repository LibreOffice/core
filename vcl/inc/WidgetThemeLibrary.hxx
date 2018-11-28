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

#include <cstdint>

namespace vcl
{
struct WidgetDrawStyle
{
    uint32_t nSize;
    uint32_t maFaceColor;
    uint32_t maCheckedColor;
    uint32_t maLightColor;
    uint32_t maLightBorderColor;
    uint32_t maShadowColor;
    uint32_t maDarkShadowColor;
    uint32_t maButtonTextColor;
    uint32_t maButtonRolloverTextColor;
    uint32_t maRadioCheckTextColor;
    uint32_t maGroupTextColor;
    uint32_t maLabelTextColor;
    uint32_t maWindowColor;
    uint32_t maWindowTextColor;
    uint32_t maDialogColor;
    uint32_t maDialogTextColor;
    uint32_t maWorkspaceColor;
    uint32_t maMonoColor;
    uint32_t maFieldColor;
    uint32_t maFieldTextColor;
    uint32_t maFieldRolloverTextColor;
    uint32_t maActiveColor;
    uint32_t maActiveTextColor;
    uint32_t maActiveBorderColor;
    uint32_t maDeactiveColor;
    uint32_t maDeactiveTextColor;
    uint32_t maDeactiveBorderColor;
    uint32_t maMenuColor;
    uint32_t maMenuBarColor;
    uint32_t maMenuBarRolloverColor;
    uint32_t maMenuBorderColor;
    uint32_t maMenuTextColor;
    uint32_t maMenuBarTextColor;
    uint32_t maMenuBarRolloverTextColor;
    uint32_t maMenuBarHighlightTextColor;
    uint32_t maMenuHighlightColor;
    uint32_t maMenuHighlightTextColor;
    uint32_t maHighlightColor;
    uint32_t maHighlightTextColor;
    uint32_t maActiveTabColor;
    uint32_t maInactiveTabColor;
    uint32_t maTabTextColor;
    uint32_t maTabRolloverTextColor;
    uint32_t maTabHighlightTextColor;
    uint32_t maDisableColor;
    uint32_t maHelpColor;
    uint32_t maHelpTextColor;
    uint32_t maLinkColor;
    uint32_t maVisitedLinkColor;
    uint32_t maToolTextColor;
    uint32_t maFontColor;
};

struct ControlDrawParameters
{
    typedef struct _cairo cairo_t;
    ControlDrawParameters(cairo_t* i_pCairo, ControlPart i_ePart, ControlState i_eState)
        : nSize(sizeof(ControlDrawParameters))
        , pCairo(i_pCairo)
        , ePart(i_ePart)
        , eState(i_eState)
        , eButtonValue(ButtonValue::DontKnow)
        , bIsAction(false)
        , nValue(0)
    {
    }

    uint32_t nSize;
    cairo_t* pCairo;
    ControlPart ePart;
    ControlState eState;
    ButtonValue eButtonValue;
    bool bIsAction;
    int64_t nValue;
};

typedef struct WidgetThemeLibrary_t WidgetThemeLibrary;

typedef struct _rectangle
{
    long x, y;
    long width, height;
} rectangle_t;

struct WidgetThemeLibrary_t
{
    uint32_t nSize;

    bool (*isNativeControlSupported)(ControlType eType, ControlPart ePart);
    bool (*getRegion)(ControlType eType, ControlPart ePart, ControlState eState,
                      const rectangle_t& rBoundingControlRegion, rectangle_t& rNativeBoundingRegion,
                      rectangle_t& rNativeContentRegion);

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

    bool (*updateSettings)(WidgetDrawStyle& rStyle);
};

extern "C" vcl::WidgetThemeLibrary* CreateWidgetThemeLibrary();

} // end vcl namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
