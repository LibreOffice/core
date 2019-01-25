/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <widgetdraw/WidgetDefinitionReader.hxx>

#include <sal/config.h>
#include <osl/file.hxx>
#include <tools/stream.hxx>
#include <unordered_map>

namespace vcl
{
namespace
{
bool lcl_fileExists(OUString const& sFilename)
{
    osl::File aFile(sFilename);
    osl::FileBase::RC eRC = aFile.open(osl_File_OpenFlag_Read);
    return osl::FileBase::E_None == eRC;
}

int lcl_gethex(sal_Char aChar)
{
    if (aChar >= '0' && aChar <= '9')
        return aChar - '0';
    else if (aChar >= 'a' && aChar <= 'f')
        return aChar - 'a' + 10;
    else if (aChar >= 'A' && aChar <= 'F')
        return aChar - 'A' + 10;
    else
        return 0;
}

bool readColor(OString const& rString, Color& rColor)
{
    if (rString.getLength() != 7)
        return false;

    const sal_Char aChar(rString[0]);

    if (aChar != '#')
        return false;

    rColor.SetRed((lcl_gethex(rString[1]) << 4) | lcl_gethex(rString[2]));
    rColor.SetGreen((lcl_gethex(rString[3]) << 4) | lcl_gethex(rString[4]));
    rColor.SetBlue((lcl_gethex(rString[5]) << 4) | lcl_gethex(rString[6]));

    return true;
}

} // end anonymous namespace

WidgetDefinitionReader::WidgetDefinitionReader(OUString const& rFilePath)
    : m_rFilePath(rFilePath)
{
}

void WidgetDefinitionReader::readDrawingDefinition(tools::XmlWalker& rWalker,
                                                   std::shared_ptr<WidgetDefinitionState>& rpState)
{
    rWalker.children();
    while (rWalker.isValid())
    {
        if (rWalker.name() == "rect")
        {
            Color aStrokeColor;
            readColor(rWalker.attribute("stroke"), aStrokeColor);
            Color aFillColor;
            readColor(rWalker.attribute("fill"), aFillColor);
            OString sStrokeWidth = rWalker.attribute("stroke-width");
            sal_Int32 nStrokeWidth = -1;
            if (!sStrokeWidth.isEmpty())
                nStrokeWidth = sStrokeWidth.toInt32();

            sal_Int32 nRx = -1;
            OString sRx = rWalker.attribute("rx");
            if (!sRx.isEmpty())
                nRx = sRx.toInt32();

            sal_Int32 nRy = -1;
            OString sRy = rWalker.attribute("ry");
            if (!sRy.isEmpty())
                nRy = sRy.toInt32();

            sal_Int32 nMargin = 0;
            OString sMargin = rWalker.attribute("margin");
            if (!sMargin.isEmpty())
                nMargin = sMargin.toInt32();

            rpState->addDrawRectangle(aStrokeColor, nStrokeWidth, aFillColor, nRx, nRy, nMargin);
        }
        else if (rWalker.name() == "circ")
        {
            Color aStrokeColor;
            readColor(rWalker.attribute("stroke"), aStrokeColor);
            Color aFillColor;
            readColor(rWalker.attribute("fill"), aFillColor);
            OString sStrokeWidth = rWalker.attribute("stroke-width");
            sal_Int32 nStrokeWidth = -1;
            if (!sStrokeWidth.isEmpty())
                nStrokeWidth = sStrokeWidth.toInt32();

            sal_Int32 nMargin = 0;
            OString sMargin = rWalker.attribute("margin");
            if (!sMargin.isEmpty())
                nMargin = sMargin.toInt32();

            rpState->addDrawCircle(aStrokeColor, nStrokeWidth, aFillColor, nMargin);
        }
        else if (rWalker.name() == "line")
        {
            Color aStrokeColor;
            readColor(rWalker.attribute("stroke"), aStrokeColor);

            OString sStrokeWidth = rWalker.attribute("stroke-width");
            sal_Int32 nStrokeWidth = -1;
            if (!sStrokeWidth.isEmpty())
                nStrokeWidth = sStrokeWidth.toInt32();

            OString sX1 = rWalker.attribute("x1");
            float fX1 = sX1.isEmpty() ? -1.0 : sX1.toFloat();

            OString sY1 = rWalker.attribute("y1");
            float fY1 = sY1.isEmpty() ? -1.0 : sY1.toFloat();

            OString sX2 = rWalker.attribute("x2");
            float fX2 = sX2.isEmpty() ? -1.0 : sX2.toFloat();

            OString sY2 = rWalker.attribute("y2");
            float fY2 = sY2.isEmpty() ? -1.0 : sY2.toFloat();

            rpState->addDrawLine(aStrokeColor, nStrokeWidth, fX1, fY1, fX2, fY2);
        }

        rWalker.next();
    }
    rWalker.parent();
}

void WidgetDefinitionReader::readDefinition(
    tools::XmlWalker& rWalker,
    std::unordered_map<OString, std::shared_ptr<WidgetDefinitionPart>>& rPart)
{
    rWalker.children();
    while (rWalker.isValid())
    {
        if (rWalker.name() == "part")
        {
            OString sPart = rWalker.attribute("value");
            std::shared_ptr<WidgetDefinitionPart> pPart = std::make_shared<WidgetDefinitionPart>();
            rPart.emplace(sPart, pPart);
            readPart(rWalker, pPart);
        }
        rWalker.next();
    }
    rWalker.parent();
}

void WidgetDefinitionReader::readPart(tools::XmlWalker& rWalker,
                                      std::shared_ptr<WidgetDefinitionPart> rpPart)
{
    rWalker.children();
    while (rWalker.isValid())
    {
        if (rWalker.name() == "state")
        {
            OString sEnabled = rWalker.attribute("enabled");
            OString sFocused = rWalker.attribute("focused");
            OString sPressed = rWalker.attribute("pressed");
            OString sRollover = rWalker.attribute("rollover");
            OString sDefault = rWalker.attribute("default");
            OString sSelected = rWalker.attribute("selected");
            OString sButtonValue = rWalker.attribute("button-value");

            std::shared_ptr<WidgetDefinitionState> pState = std::make_shared<WidgetDefinitionState>(
                sEnabled, sFocused, sPressed, sRollover, sDefault, sSelected, sButtonValue);
            rpPart->maStates.push_back(pState);
            readDrawingDefinition(rWalker, pState);
        }
        rWalker.next();
    }
    rWalker.parent();
}

bool WidgetDefinitionReader::read(WidgetDefinition& rWidgetDefinition)
{
    if (!lcl_fileExists(m_rFilePath))
        return false;

    SvFileStream aFileStream(m_rFilePath, StreamMode::READ);

    std::unordered_map<OString, Color*> aStyleColorMap = {
        { "faceColor", &rWidgetDefinition.maFaceColor },
        { "checkedColor", &rWidgetDefinition.maCheckedColor },
        { "lightColor", &rWidgetDefinition.maLightColor },
        { "lightBorderColor", &rWidgetDefinition.maLightBorderColor },
        { "shadowColor", &rWidgetDefinition.maShadowColor },
        { "darkShadowColor", &rWidgetDefinition.maDarkShadowColor },
        { "buttonTextColor", &rWidgetDefinition.maButtonTextColor },
        { "buttonRolloverTextColor", &rWidgetDefinition.maButtonRolloverTextColor },
        { "radioCheckTextColor", &rWidgetDefinition.maRadioCheckTextColor },
        { "groupTextColor", &rWidgetDefinition.maGroupTextColor },
        { "labelTextColor", &rWidgetDefinition.maLabelTextColor },
        { "windowColor", &rWidgetDefinition.maWindowColor },
        { "windowTextColor", &rWidgetDefinition.maWindowTextColor },
        { "dialogColor", &rWidgetDefinition.maDialogColor },
        { "dialogTextColor", &rWidgetDefinition.maDialogTextColor },
        { "workspaceColor", &rWidgetDefinition.maWorkspaceColor },
        { "monoColor", &rWidgetDefinition.maMonoColor },
        { "fieldColor", &rWidgetDefinition.maFieldColor },
        { "fieldTextColor", &rWidgetDefinition.maFieldTextColor },
        { "fieldRolloverTextColor", &rWidgetDefinition.maFieldRolloverTextColor },
        { "activeColor", &rWidgetDefinition.maActiveColor },
        { "activeTextColor", &rWidgetDefinition.maActiveTextColor },
        { "activeBorderColor", &rWidgetDefinition.maActiveBorderColor },
        { "deactiveColor", &rWidgetDefinition.maDeactiveColor },
        { "deactiveTextColor", &rWidgetDefinition.maDeactiveTextColor },
        { "deactiveBorderColor", &rWidgetDefinition.maDeactiveBorderColor },
        { "menuColor", &rWidgetDefinition.maMenuColor },
        { "menuBarColor", &rWidgetDefinition.maMenuBarColor },
        { "menuBarRolloverColor", &rWidgetDefinition.maMenuBarRolloverColor },
        { "menuBorderColor", &rWidgetDefinition.maMenuBorderColor },
        { "menuTextColor", &rWidgetDefinition.maMenuTextColor },
        { "menuBarTextColor", &rWidgetDefinition.maMenuBarTextColor },
        { "menuBarRolloverTextColor", &rWidgetDefinition.maMenuBarRolloverTextColor },
        { "menuBarHighlightTextColor", &rWidgetDefinition.maMenuBarHighlightTextColor },
        { "menuHighlightColor", &rWidgetDefinition.maMenuHighlightColor },
        { "menuHighlightTextColor", &rWidgetDefinition.maMenuHighlightTextColor },
        { "highlightColor", &rWidgetDefinition.maHighlightColor },
        { "highlightTextColor", &rWidgetDefinition.maHighlightTextColor },
        { "activeTabColor", &rWidgetDefinition.maActiveTabColor },
        { "inactiveTabColor", &rWidgetDefinition.maInactiveTabColor },
        { "tabTextColor", &rWidgetDefinition.maTabTextColor },
        { "tabRolloverTextColor", &rWidgetDefinition.maTabRolloverTextColor },
        { "tabHighlightTextColor", &rWidgetDefinition.maTabHighlightTextColor },
        { "disableColor", &rWidgetDefinition.maDisableColor },
        { "helpColor", &rWidgetDefinition.maHelpColor },
        { "helpTextColor", &rWidgetDefinition.maHelpTextColor },
        { "linkColor", &rWidgetDefinition.maLinkColor },
        { "visitedLinkColor", &rWidgetDefinition.maVisitedLinkColor },
        { "toolTextColor", &rWidgetDefinition.maToolTextColor },
        { "fontColor", &rWidgetDefinition.maFontColor },
    };

    tools::XmlWalker aWalker;
    if (!aWalker.open(&aFileStream))
        return false;

    if (aWalker.name() != "widgets")
        return false;

    aWalker.children();
    while (aWalker.isValid())
    {
        if (aWalker.name() == "style")
        {
            aWalker.children();
            while (aWalker.isValid())
            {
                auto pair = aStyleColorMap.find(aWalker.name());
                if (pair != aStyleColorMap.end())
                {
                    readColor(aWalker.attribute("value"), *pair->second);
                }
                aWalker.next();
            }
            aWalker.parent();
        }
        else if (aWalker.name() == "pushbutton")
        {
            readDefinition(aWalker, rWidgetDefinition.maPushButtonDefinitions);
        }
        else if (aWalker.name() == "radiobutton")
        {
            readDefinition(aWalker, rWidgetDefinition.maRadioButtonDefinitions);
        }
        else if (aWalker.name() == "editbox")
        {
            readDefinition(aWalker, rWidgetDefinition.maEditboxDefinitions);
        }
        aWalker.next();
    }
    aWalker.parent();

    return true;
}

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
    mpDrawCommands.push_back(pCommand);
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
