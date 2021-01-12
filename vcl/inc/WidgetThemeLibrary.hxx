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
#include <tools/color.hxx>

namespace vcl
{
struct WidgetDrawStyle
{
    uint32_t nSize;
    Color maFaceColor;
    Color maCheckedColor;
    Color maLightColor;
    Color maLightBorderColor;
    Color maShadowColor;
    Color maDarkShadowColor;
    Color maDefaultButtonTextColor;
    Color maButtonTextColor;
    Color maDefaultActionButtonTextColor;
    Color maActionButtonTextColor;
    Color maFlatButtonTextColor;
    Color maDefaultButtonRolloverTextColor;
    Color maButtonRolloverTextColor;
    Color maDefaultActionButtonRolloverTextColor;
    Color maActionButtonRolloverTextColor;
    Color maFlatButtonRolloverTextColor;
    Color maDefaultButtonPressedRolloverTextColor;
    Color maButtonPressedRolloverTextColor;
    Color maDefaultActionButtonPressedRolloverTextColor;
    Color maActionButtonPressedRolloverTextColor;
    Color maFlatButtonPressedRolloverTextColor;
    Color maRadioCheckTextColor;
    Color maGroupTextColor;
    Color maLabelTextColor;
    Color maWindowColor;
    Color maWindowTextColor;
    Color maDialogColor;
    Color maDialogTextColor;
    Color maWorkspaceColor;
    Color maMonoColor;
    Color maFieldColor;
    Color maFieldTextColor;
    Color maFieldRolloverTextColor;
    Color maActiveColor;
    Color maActiveTextColor;
    Color maActiveBorderColor;
    Color maDeactiveColor;
    Color maDeactiveTextColor;
    Color maDeactiveBorderColor;
    Color maMenuColor;
    Color maMenuBarColor;
    Color maMenuBarRolloverColor;
    Color maMenuBorderColor;
    Color maMenuTextColor;
    Color maMenuBarTextColor;
    Color maMenuBarRolloverTextColor;
    Color maMenuBarHighlightTextColor;
    Color maMenuHighlightColor;
    Color maMenuHighlightTextColor;
    Color maHighlightColor;
    Color maHighlightTextColor;
    Color maActiveTabColor;
    Color maInactiveTabColor;
    Color maTabTextColor;
    Color maTabRolloverTextColor;
    Color maTabHighlightTextColor;
    Color maDisableColor;
    Color maHelpColor;
    Color maHelpTextColor;
    Color maLinkColor;
    Color maVisitedLinkColor;
    Color maToolTextColor;
    Color maFontColor;
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
    tools::Long x, y;
    tools::Long width, height;
} rectangle_t;

struct WidgetThemeLibrary_t
{
    uint32_t nSize;

    bool (*isNativeControlSupported)(ControlType eType, ControlPart ePart);
    bool (*getRegion)(ControlType eType, ControlPart ePart, ControlState eState,
                      const rectangle_t& rBoundingControlRegion, rectangle_t& rNativeBoundingRegion,
                      rectangle_t& rNativeContentRegion);

    bool (*drawPushButton)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                           tools::Long nHeight);
    bool (*drawRadiobutton)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                            tools::Long nHeight);
    bool (*drawCheckbox)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                         tools::Long nHeight);
    bool (*drawCombobox)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                         tools::Long nHeight);
    bool (*drawEditbox)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                        tools::Long nHeight);
    bool (*drawScrollbar)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                          tools::Long nHeight);
    bool (*drawSpinButtons)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                            tools::Long nHeight);
    bool (*drawSpinbox)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                        tools::Long nHeight);
    bool (*drawTabItem)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                        tools::Long nHeight);
    bool (*drawTabPane)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                        tools::Long nHeight);
    bool (*drawTabHeader)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                          tools::Long nHeight);
    bool (*drawTabBody)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                        tools::Long nHeight);
    bool (*drawSlider)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                       tools::Long nHeight);
    bool (*drawFixedline)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                          tools::Long nHeight);
    bool (*drawToolbar)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                        tools::Long nHeight);
    bool (*drawProgress)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                         tools::Long nHeight);
    bool (*drawWindowsBackground)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                                  tools::Long nHeight);
    bool (*drawListbox)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                        tools::Long nHeight);
    bool (*drawFrame)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                      tools::Long nHeight);
    bool (*drawListNode)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                         tools::Long nHeight);
    bool (*drawListNet)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                        tools::Long nHeight);
    bool (*drawListHeader)(ControlDrawParameters const& rParameters, tools::Long nWidth,
                           tools::Long nHeight);

    bool (*updateSettings)(WidgetDrawStyle& rStyle);
};

extern "C" vcl::WidgetThemeLibrary* CreateWidgetThemeLibrary();

} // end vcl namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
