/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <widgetdraw/WidgetDefinition.hxx>

#include <sal/config.h>
#include <tools/stream.hxx>
#include <unordered_map>

namespace vcl
{
namespace
{
OString xmlControlPart(ControlPart ePart)
{
    switch (ePart)
    {
        case ControlPart::NONE:
            return "NONE";
        case ControlPart::Entire:
            return "Entire";
        case ControlPart::ListboxWindow:
            return "ListboxWindow";
        case ControlPart::Button:
            return "NONE";
        case ControlPart::ButtonUp:
            return "NONE";
        case ControlPart::ButtonDown:
            return "NONE";
        case ControlPart::ButtonLeft:
            return "NONE";
        case ControlPart::ButtonRight:
            return "NONE";
        case ControlPart::AllButtons:
            return "NONE";
        case ControlPart::SeparatorHorz:
            return "NONE";
        case ControlPart::SeparatorVert:
            return "NONE";
        case ControlPart::TrackHorzLeft:
            return "NONE";
        case ControlPart::TrackVertUpper:
            return "NONE";
        case ControlPart::TrackHorzRight:
            return "NONE";
        case ControlPart::TrackVertLower:
            return "NONE";
        case ControlPart::TrackHorzArea:
            return "NONE";
        case ControlPart::TrackVertArea:
            return "NONE";
        case ControlPart::Arrow:
            return "NONE";
        case ControlPart::ThumbHorz:
            return "NONE";
        case ControlPart::ThumbVert:
            return "NONE";
        case ControlPart::MenuItem:
            return "NONE";
        case ControlPart::MenuItemCheckMark:
            return "NONE";
        case ControlPart::MenuItemRadioMark:
            return "NONE";
        case ControlPart::Separator:
            return "NONE";
        case ControlPart::SubmenuArrow:
            return "NONE";
        case ControlPart::SubEdit:
            return "NONE";
        case ControlPart::DrawBackgroundHorz:
            return "NONE";
        case ControlPart::DrawBackgroundVert:
            return "NONE";
        case ControlPart::TabsDrawRtl:
            return "NONE";
        case ControlPart::HasBackgroundTexture:
            return "NONE";
        case ControlPart::HasThreeButtons:
            return "NONE";
        case ControlPart::BackgroundWindow:
            return "NONE";
        case ControlPart::BackgroundDialog:
            return "NONE";
        case ControlPart::Border:
            return "NONE";
        case ControlPart::Focus:
            return "FOCUS";

        default:
            break;
    }
    return "NONE";
}

} // end anonymous namespace

std::shared_ptr<WidgetDefinitionPart> WidgetDefinition::getPushButtonDefinition(ControlPart ePart)
{
    auto aIterator = maPushButtonDefinitions.find(xmlControlPart(ePart));

    if (aIterator != maPushButtonDefinitions.end())
        return aIterator->second;
    return std::shared_ptr<WidgetDefinitionPart>();
}

std::shared_ptr<WidgetDefinitionPart> WidgetDefinition::getRadioButtonDefinition(ControlPart ePart)
{
    auto aIterator = maRadioButtonDefinitions.find(xmlControlPart(ePart));

    if (aIterator != maRadioButtonDefinitions.end())
        return aIterator->second;
    return std::shared_ptr<WidgetDefinitionPart>();
}

std::shared_ptr<WidgetDefinitionPart> WidgetDefinition::getEditboxDefinition(ControlPart ePart)
{
    auto aIterator = maEditboxDefinitions.find(xmlControlPart(ePart));

    if (aIterator != maEditboxDefinitions.end())
        return aIterator->second;
    return std::shared_ptr<WidgetDefinitionPart>();
}

std::vector<std::shared_ptr<WidgetDefinitionState>>
WidgetDefinitionPart::getStates(ControlState eState, ImplControlValue const& rValue)
{
    std::vector<std::shared_ptr<WidgetDefinitionState>> aStatesToAdd;

    for (auto& state : maStates)
    {
        bool bAdd = true;

        if (state->msEnabled != "any"
            && !((state->msEnabled == "true" && eState & ControlState::ENABLED)
                 || (state->msEnabled == "false" && !(eState & ControlState::ENABLED))))
            bAdd = false;
        if (state->msFocused != "any"
            && !((state->msFocused == "true" && eState & ControlState::FOCUSED)
                 || (state->msFocused == "false" && !(eState & ControlState::FOCUSED))))
            bAdd = false;
        if (state->msPressed != "any"
            && !((state->msPressed == "true" && eState & ControlState::PRESSED)
                 || (state->msPressed == "false" && !(eState & ControlState::PRESSED))))
            bAdd = false;
        if (state->msRollover != "any"
            && !((state->msRollover == "true" && eState & ControlState::ROLLOVER)
                 || (state->msRollover == "false" && !(eState & ControlState::ROLLOVER))))
            bAdd = false;
        if (state->msDefault != "any"
            && !((state->msDefault == "true" && eState & ControlState::DEFAULT)
                 || (state->msDefault == "false" && !(eState & ControlState::DEFAULT))))
            bAdd = false;
        if (state->msSelected != "any"
            && !((state->msSelected == "true" && eState & ControlState::SELECTED)
                 || (state->msSelected == "false" && !(eState & ControlState::SELECTED))))
            bAdd = false;

        ButtonValue eButtonValue = rValue.getTristateVal();

        if (state->msButtonValue != "any"
            && !((state->msButtonValue == "true" && eButtonValue == ButtonValue::On)
                 || (state->msButtonValue == "false" && eButtonValue != ButtonValue::On)))
            bAdd = false;

        if (bAdd)
            aStatesToAdd.push_back(state);
    }

    return aStatesToAdd;
}

WidgetDefinitionState::WidgetDefinitionState(OString const& sEnabled, OString const& sFocused,
                                             OString const& sPressed, OString const& sRollover,
                                             OString const& sDefault, OString const& sSelected,
                                             OString const& sButtonValue)
    : msEnabled(sEnabled)
    , msFocused(sFocused)
    , msPressed(sPressed)
    , msRollover(sRollover)
    , msDefault(sDefault)
    , msSelected(sSelected)
    , msButtonValue(sButtonValue)
{
}

void WidgetDefinitionState::addDrawRectangle(Color aStrokeColor, sal_Int32 nStrokeWidth,
                                             Color aFillColor, sal_Int32 nRx, sal_Int32 nRy,
                                             sal_Int32 nMargin)
{
    std::shared_ptr<DrawCommand> pCommand(std::make_shared<RectangleDrawCommand>());
    pCommand->maStrokeColor = aStrokeColor;
    pCommand->maFillColor = aFillColor;
    pCommand->mnStrokeWidth = nStrokeWidth;
    pCommand->mnMargin = nMargin;
    RectangleDrawCommand& rRectCommand = static_cast<RectangleDrawCommand&>(*pCommand);
    rRectCommand.mnRx = nRx;
    rRectCommand.mnRy = nRy;
    mpDrawCommands.push_back(std::move(pCommand));
}

void WidgetDefinitionState::addDrawCircle(Color aStrokeColor, sal_Int32 nStrokeWidth,
                                          Color aFillColor, sal_Int32 nMargin)
{
    std::shared_ptr<DrawCommand> pCommand(std::make_shared<CircleDrawCommand>());
    pCommand->maStrokeColor = aStrokeColor;
    pCommand->maFillColor = aFillColor;
    pCommand->mnStrokeWidth = nStrokeWidth;
    pCommand->mnMargin = nMargin;
    mpDrawCommands.push_back(std::move(pCommand));
}

void WidgetDefinitionState::addDrawLine(Color aStrokeColor, sal_Int32 nStrokeWidth, float fX1,
                                        float fY1, float fX2, float fY2)
{
    std::shared_ptr<DrawCommand> pCommand(std::make_shared<LineDrawCommand>());
    pCommand->maStrokeColor = aStrokeColor;
    pCommand->mnStrokeWidth = nStrokeWidth;
    LineDrawCommand& rLineCommand = static_cast<LineDrawCommand&>(*pCommand);
    rLineCommand.mfX1 = fX1;
    rLineCommand.mfY1 = fY1;
    rLineCommand.mfX2 = fX2;
    rLineCommand.mfY2 = fY2;
    mpDrawCommands.push_back(std::move(pCommand));
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
