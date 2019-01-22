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
        rWalker.next();
    }
    rWalker.parent();
}

void WidgetDefinitionReader::readPushButton(tools::XmlWalker& rWalker)
{
    rWalker.children();
    while (rWalker.isValid())
    {
        if (rWalker.name() == "part")
        {
            OString sPart = rWalker.attribute("value");
            std::shared_ptr<WidgetDefinition> pPart = std::make_shared<WidgetDefinition>();
            maPushButtonDefinitions.emplace(sPart, pPart);
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

                    std::shared_ptr<WidgetDefinitionState> pState
                        = std::make_shared<WidgetDefinitionState>(sEnabled, sFocused, sPressed,
                                                                  sRollover, sDefault, sSelected,
                                                                  sButtonValue);
                    pPart->maStates.push_back(pState);
                    readDrawingDefinition(rWalker, pState);
                }
                rWalker.next();
            }
            rWalker.parent();
        }
        rWalker.next();
    }
    rWalker.parent();
}

void WidgetDefinitionReader::readRadioButton(tools::XmlWalker& rWalker)
{
    rWalker.children();
    while (rWalker.isValid())
    {
        if (rWalker.name() == "part")
        {
            OString sPart = rWalker.attribute("value");
            std::shared_ptr<WidgetDefinition> pPart = std::make_shared<WidgetDefinition>();
            maRadioButtonDefinitions.emplace(sPart, pPart);
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
                    sButtonValue = sButtonValue.isEmpty() ? "any" : sButtonValue;
                    std::shared_ptr<WidgetDefinitionState> pState
                        = std::make_shared<WidgetDefinitionState>(sEnabled, sFocused, sPressed,
                                                                  sRollover, sDefault, sSelected,
                                                                  sButtonValue);
                    pPart->maStates.push_back(pState);
                    readDrawingDefinition(rWalker, pState);
                }
                rWalker.next();
            }
            rWalker.parent();
        }
        rWalker.next();
    }
    rWalker.parent();
}

bool WidgetDefinitionReader::read()
{
    if (!lcl_fileExists(m_rFilePath))
        return false;

    SvFileStream aFileStream(m_rFilePath, StreamMode::READ);

    std::unordered_map<OString, Color*> aStyleColorMap = {
        { "faceColor", &maFaceColor },
        { "checkedColor", &maCheckedColor },
        { "lightColor", &maLightColor },
        { "lightBorderColor", &maLightBorderColor },
        { "shadowColor", &maShadowColor },
        { "darkShadowColor", &maDarkShadowColor },
        { "buttonTextColor", &maButtonTextColor },
        { "buttonRolloverTextColor", &maButtonRolloverTextColor },
        { "radioCheckTextColor", &maRadioCheckTextColor },
        { "groupTextColor", &maGroupTextColor },
        { "labelTextColor", &maLabelTextColor },
        { "windowColor", &maWindowColor },
        { "windowTextColor", &maWindowTextColor },
        { "dialogColor", &maDialogColor },
        { "dialogTextColor", &maDialogTextColor },
        { "workspaceColor", &maWorkspaceColor },
        { "monoColor", &maMonoColor },
        { "fieldColor", &maFieldColor },
        { "fieldTextColor", &maFieldTextColor },
        { "fieldRolloverTextColor", &maFieldRolloverTextColor },
        { "activeColor", &maActiveColor },
        { "activeTextColor", &maActiveTextColor },
        { "activeBorderColor", &maActiveBorderColor },
        { "deactiveColor", &maDeactiveColor },
        { "deactiveTextColor", &maDeactiveTextColor },
        { "deactiveBorderColor", &maDeactiveBorderColor },
        { "menuColor", &maMenuColor },
        { "menuBarColor", &maMenuBarColor },
        { "menuBarRolloverColor", &maMenuBarRolloverColor },
        { "menuBorderColor", &maMenuBorderColor },
        { "menuTextColor", &maMenuTextColor },
        { "menuBarTextColor", &maMenuBarTextColor },
        { "menuBarRolloverTextColor", &maMenuBarRolloverTextColor },
        { "menuBarHighlightTextColor", &maMenuBarHighlightTextColor },
        { "menuHighlightColor", &maMenuHighlightColor },
        { "menuHighlightTextColor", &maMenuHighlightTextColor },
        { "highlightColor", &maHighlightColor },
        { "highlightTextColor", &maHighlightTextColor },
        { "activeTabColor", &maActiveTabColor },
        { "inactiveTabColor", &maInactiveTabColor },
        { "tabTextColor", &maTabTextColor },
        { "tabRolloverTextColor", &maTabRolloverTextColor },
        { "tabHighlightTextColor", &maTabHighlightTextColor },
        { "disableColor", &maDisableColor },
        { "helpColor", &maHelpColor },
        { "helpTextColor", &maHelpTextColor },
        { "linkColor", &maLinkColor },
        { "visitedLinkColor", &maVisitedLinkColor },
        { "toolTextColor", &maToolTextColor },
        { "fontColor", &maFontColor },
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
            readPushButton(aWalker);
        }
        else if (aWalker.name() == "radiobutton")
        {
            readRadioButton(aWalker);
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

std::shared_ptr<WidgetDefinition> WidgetDefinitionReader::getPushButtonDefinition(ControlPart ePart)
{
    auto aIterator = maPushButtonDefinitions.find(xmlControlPart(ePart));

    if (aIterator != maPushButtonDefinitions.end())
        return aIterator->second;
    return std::shared_ptr<WidgetDefinition>();
}

std::shared_ptr<WidgetDefinition>
WidgetDefinitionReader::getRadioButtonDefinition(ControlPart ePart)
{
    auto aIterator = maRadioButtonDefinitions.find(xmlControlPart(ePart));

    if (aIterator != maRadioButtonDefinitions.end())
        return aIterator->second;
    return std::shared_ptr<WidgetDefinition>();
}

std::vector<std::shared_ptr<WidgetDefinitionState>>
WidgetDefinition::getStates(ControlState eState, ImplControlValue const& rValue)
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
    std::unique_ptr<DrawCommand> pCommand(std::make_unique<CircleDrawCommand>());
    pCommand->maStrokeColor = aStrokeColor;
    pCommand->maFillColor = aFillColor;
    pCommand->mnStrokeWidth = nStrokeWidth;
    pCommand->mnMargin = nMargin;
    mpDrawCommands.push_back(std::move(pCommand));
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
