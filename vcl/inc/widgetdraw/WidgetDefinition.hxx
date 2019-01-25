/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_WIDGETDEFINITION_HXX
#define INCLUDED_VCL_INC_WIDGETDEFINITION_HXX

#include <vcl/dllapi.h>
#include <memory>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <unordered_map>
#include <vector>
#include <vcl/salnativewidgets.hxx>

namespace vcl
{
enum class DrawCommandType
{
    RECTANGLE,
    CIRCLE,
    LINE
};

class VCL_DLLPUBLIC DrawCommand
{
public:
    DrawCommand(DrawCommandType aType)
        : maType(aType)
        , mnStrokeWidth(-1)
        , mnMargin(0)
    {
    }

    DrawCommandType maType;

    Color maStrokeColor;
    Color maFillColor;
    sal_Int32 mnStrokeWidth;
    sal_Int32 mnMargin;
};

class VCL_DLLPUBLIC RectangleDrawCommand : public DrawCommand
{
public:
    sal_Int32 mnRx;
    sal_Int32 mnRy;

    RectangleDrawCommand()
        : DrawCommand(DrawCommandType::RECTANGLE)
        , mnRx(0)
        , mnRy(0)
    {
    }
};

class VCL_DLLPUBLIC CircleDrawCommand : public DrawCommand
{
public:
    CircleDrawCommand()
        : DrawCommand(DrawCommandType::CIRCLE)
    {
    }
};

class VCL_DLLPUBLIC LineDrawCommand : public DrawCommand
{
public:
    float mfX1;
    float mfY1;
    float mfX2;
    float mfY2;

    LineDrawCommand()
        : DrawCommand(DrawCommandType::LINE)
    {
    }
};

class VCL_DLLPUBLIC WidgetDefinitionState
{
public:
    OString msEnabled;
    OString msFocused;
    OString msPressed;
    OString msRollover;
    OString msDefault;
    OString msSelected;
    OString msButtonValue;

    WidgetDefinitionState(OString const& sEnabled, OString const& sFocused, OString const& sPressed,
                          OString const& sRollover, OString const& sDefault,
                          OString const& sSelected, OString const& sButtonValue);

    std::vector<std::shared_ptr<DrawCommand>> mpDrawCommands;

    void addDrawRectangle(Color aStrokeColor, sal_Int32 nStrokeWidth, Color aFillColor,
                          sal_Int32 nRx, sal_Int32 nRy, sal_Int32 nMargin);
    void addDrawCircle(Color aStrokeColor, sal_Int32 nStrokeWidth, Color aFillColor,
                       sal_Int32 nMargin);
    void addDrawLine(Color aStrokeColor, sal_Int32 nStrokeWidth, float fX1, float fY1, float fX2,
                     float fY2);
};

class VCL_DLLPUBLIC WidgetDefinitionPart
{
public:
    std::vector<std::shared_ptr<WidgetDefinitionState>> getStates(ControlState eState,
                                                                  ImplControlValue const& rValue);

    std::vector<std::shared_ptr<WidgetDefinitionState>> maStates;
};

class VCL_DLLPUBLIC WidgetDefinition
{
public:
    Color maFaceColor;
    Color maCheckedColor;
    Color maLightColor;
    Color maLightBorderColor;
    Color maShadowColor;
    Color maDarkShadowColor;
    Color maButtonTextColor;
    Color maButtonRolloverTextColor;
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

    std::unordered_map<OString, std::shared_ptr<WidgetDefinitionPart>> maPushButtonDefinitions;
    std::unordered_map<OString, std::shared_ptr<WidgetDefinitionPart>> maRadioButtonDefinitions;
    std::unordered_map<OString, std::shared_ptr<WidgetDefinitionPart>> maEditboxDefinitions;

    std::shared_ptr<WidgetDefinitionPart> getPushButtonDefinition(ControlPart ePart);
    std::shared_ptr<WidgetDefinitionPart> getRadioButtonDefinition(ControlPart ePart);
    std::shared_ptr<WidgetDefinitionPart> getEditboxDefinition(ControlPart ePart);
};

} // end vcl namespace

#endif // INCLUDED_VCL_INC_WIDGETDEFINITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
