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
#include <unordered_map>

namespace vcl
{
std::shared_ptr<WidgetDefinitionPart> WidgetDefinition::getDefinition(ControlType eType,
                                                                      ControlPart ePart)
{
    auto aIterator = maDefinitions.find(ControlTypeAndPart(eType, ePart));

    if (aIterator != maDefinitions.end())
        return aIterator->second;
    return std::shared_ptr<WidgetDefinitionPart>();
}

std::vector<std::shared_ptr<WidgetDefinitionState>>
WidgetDefinitionPart::getStates(ControlType eType, ControlPart ePart, ControlState eState,
                                ImplControlValue const& rValue)
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
        {
            bAdd = false;
        }

        OString sExtra = "any";

        switch (eType)
        {
            case ControlType::TabItem:
            {
                auto const& rTabItemValue = static_cast<TabitemValue const&>(rValue);

                if (rTabItemValue.isLeftAligned() && rTabItemValue.isRightAligned()
                    && rTabItemValue.isFirst() && rTabItemValue.isLast())
                    sExtra = "first_last";
                else if (rTabItemValue.isLeftAligned() || rTabItemValue.isFirst())
                    sExtra = "first";
                else if (rTabItemValue.isRightAligned() || rTabItemValue.isLast())
                    sExtra = "last";
                else
                    sExtra = "middle";
            }
            break;
            case ControlType::ListHeader:
            {
                if (ePart == ControlPart::Arrow)
                {
                    if (rValue.getNumericVal() == 1)
                        sExtra = "down";
                    else
                        sExtra = "up";
                }
            }
            break;
            default:
                break;
        }

        if (state->msExtra != "any" && state->msExtra != sExtra)
        {
            bAdd = false;
        }

        if (bAdd)
            aStatesToAdd.push_back(state);
    }

    return aStatesToAdd;
}

WidgetDefinitionState::WidgetDefinitionState(OString const& sEnabled, OString const& sFocused,
                                             OString const& sPressed, OString const& sRollover,
                                             OString const& sDefault, OString const& sSelected,
                                             OString const& sButtonValue, OString const& sExtra)
    : msEnabled(sEnabled)
    , msFocused(sFocused)
    , msPressed(sPressed)
    , msRollover(sRollover)
    , msDefault(sDefault)
    , msSelected(sSelected)
    , msButtonValue(sButtonValue)
    , msExtra(sExtra)
{
}

void WidgetDefinitionState::addDrawRectangle(Color aStrokeColor, sal_Int32 nStrokeWidth,
                                             Color aFillColor, float fX1, float fY1, float fX2,
                                             float fY2, sal_Int32 nRx, sal_Int32 nRy)
{
    auto pCommand(std::make_shared<RectangleDrawCommand>());
    pCommand->maStrokeColor = aStrokeColor;
    pCommand->maFillColor = aFillColor;
    pCommand->mnStrokeWidth = nStrokeWidth;
    pCommand->mnRx = nRx;
    pCommand->mnRy = nRy;
    pCommand->mfX1 = fX1;
    pCommand->mfY1 = fY1;
    pCommand->mfX2 = fX2;
    pCommand->mfY2 = fY2;
    mpDrawCommands.push_back(std::move(pCommand));
}

void WidgetDefinitionState::addDrawCircle(Color aStrokeColor, sal_Int32 nStrokeWidth,
                                          Color aFillColor, float fX1, float fY1, float fX2,
                                          float fY2)
{
    auto pCommand(std::make_shared<CircleDrawCommand>());
    pCommand->maStrokeColor = aStrokeColor;
    pCommand->maFillColor = aFillColor;
    pCommand->mnStrokeWidth = nStrokeWidth;
    pCommand->mfX1 = fX1;
    pCommand->mfY1 = fY1;
    pCommand->mfX2 = fX2;
    pCommand->mfY2 = fY2;
    mpDrawCommands.push_back(std::move(pCommand));
}

void WidgetDefinitionState::addDrawLine(Color aStrokeColor, sal_Int32 nStrokeWidth, float fX1,
                                        float fY1, float fX2, float fY2)
{
    auto pCommand(std::make_shared<LineDrawCommand>());
    pCommand->maStrokeColor = aStrokeColor;
    pCommand->mnStrokeWidth = nStrokeWidth;
    pCommand->mfX1 = fX1;
    pCommand->mfY1 = fY1;
    pCommand->mfX2 = fX2;
    pCommand->mfY2 = fY2;
    mpDrawCommands.push_back(std::move(pCommand));
}

void WidgetDefinitionState::addDrawImage(OUString const& sSource)
{
    auto pCommand(std::make_shared<ImageDrawCommand>());
    pCommand->msSource = sSource;
    mpDrawCommands.push_back(std::move(pCommand));
}

void WidgetDefinitionState::addDrawExternal(OUString const& sSource)
{
    auto pCommand(std::make_unique<ExternalSourceDrawCommand>());
    pCommand->msSource = sSource;
    mpDrawCommands.push_back(std::move(pCommand));
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
