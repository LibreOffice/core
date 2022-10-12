/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <utility>
#include <widgetdraw/WidgetDefinitionReader.hxx>

#include <sal/config.h>
#include <osl/file.hxx>
#include <tools/stream.hxx>
#include <o3tl/string_view.hxx>
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

int lcl_gethex(char aChar)
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

    const char aChar(rString[0]);

    if (aChar != '#')
        return false;

    rColor.SetRed((lcl_gethex(rString[1]) << 4) | lcl_gethex(rString[2]));
    rColor.SetGreen((lcl_gethex(rString[3]) << 4) | lcl_gethex(rString[4]));
    rColor.SetBlue((lcl_gethex(rString[5]) << 4) | lcl_gethex(rString[6]));

    return true;
}

bool readSetting(OString const& rInputString, OString& rOutputString)
{
    if (!rInputString.isEmpty())
        rOutputString = rInputString;
    return true;
}

OString getValueOrAny(OString const& rInputString)
{
    if (rInputString.isEmpty())
        return "any";
    return rInputString;
}

ControlPart xmlStringToControlPart(std::string_view sPart)
{
    if (o3tl::equalsIgnoreAsciiCase(sPart, "NONE"))
        return ControlPart::NONE;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "Entire"))
        return ControlPart::Entire;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "ListboxWindow"))
        return ControlPart::ListboxWindow;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "Button"))
        return ControlPart::Button;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "ButtonUp"))
        return ControlPart::ButtonUp;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "ButtonDown"))
        return ControlPart::ButtonDown;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "ButtonLeft"))
        return ControlPart::ButtonLeft;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "ButtonRight"))
        return ControlPart::ButtonRight;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "AllButtons"))
        return ControlPart::AllButtons;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "SeparatorHorz"))
        return ControlPart::SeparatorHorz;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "SeparatorVert"))
        return ControlPart::SeparatorVert;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "TrackHorzLeft"))
        return ControlPart::TrackHorzLeft;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "TrackVertUpper"))
        return ControlPart::TrackVertUpper;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "TrackHorzRight"))
        return ControlPart::TrackHorzRight;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "TrackVertLower"))
        return ControlPart::TrackVertLower;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "TrackHorzArea"))
        return ControlPart::TrackHorzArea;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "TrackVertArea"))
        return ControlPart::TrackVertArea;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "Arrow"))
        return ControlPart::Arrow;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "ThumbHorz"))
        return ControlPart::ThumbHorz;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "ThumbVert"))
        return ControlPart::ThumbVert;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "MenuItem"))
        return ControlPart::MenuItem;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "MenuItemCheckMark"))
        return ControlPart::MenuItemCheckMark;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "MenuItemRadioMark"))
        return ControlPart::MenuItemRadioMark;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "Separator"))
        return ControlPart::Separator;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "SubmenuArrow"))
        return ControlPart::SubmenuArrow;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "SubEdit"))
        return ControlPart::SubEdit;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "DrawBackgroundHorz"))
        return ControlPart::DrawBackgroundHorz;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "DrawBackgroundVert"))
        return ControlPart::DrawBackgroundVert;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "TabsDrawRtl"))
        return ControlPart::TabsDrawRtl;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "HasBackgroundTexture"))
        return ControlPart::HasBackgroundTexture;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "HasThreeButtons"))
        return ControlPart::HasThreeButtons;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "BackgroundWindow"))
        return ControlPart::BackgroundWindow;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "BackgroundDialog"))
        return ControlPart::BackgroundDialog;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "Border"))
        return ControlPart::Border;
    else if (o3tl::equalsIgnoreAsciiCase(sPart, "Focus"))
        return ControlPart::Focus;
    return ControlPart::NONE;
}

bool getControlTypeForXmlString(OString const& rString, ControlType& reType)
{
    static std::unordered_map<OString, ControlType> aPartMap = {
        { "pushbutton", ControlType::Pushbutton },
        { "radiobutton", ControlType::Radiobutton },
        { "checkbox", ControlType::Checkbox },
        { "combobox", ControlType::Combobox },
        { "editbox", ControlType::Editbox },
        { "listbox", ControlType::Listbox },
        { "scrollbar", ControlType::Scrollbar },
        { "spinbox", ControlType::Spinbox },
        { "slider", ControlType::Slider },
        { "fixedline", ControlType::Fixedline },
        { "progress", ControlType::Progress },
        { "tabitem", ControlType::TabItem },
        { "tabheader", ControlType::TabHeader },
        { "tabpane", ControlType::TabPane },
        { "tabbody", ControlType::TabBody },
        { "frame", ControlType::Frame },
        { "windowbackground", ControlType::WindowBackground },
        { "toolbar", ControlType::Toolbar },
        { "listnode", ControlType::ListNode },
        { "listnet", ControlType::ListNet },
        { "listheader", ControlType::ListHeader },
        { "menubar", ControlType::Menubar },
        { "menupopup", ControlType::MenuPopup },
        { "tooltip", ControlType::Tooltip },
    };

    auto const& rIterator = aPartMap.find(rString);
    if (rIterator != aPartMap.end())
    {
        reType = rIterator->second;
        return true;
    }
    return false;
}

} // end anonymous namespace

WidgetDefinitionReader::WidgetDefinitionReader(OUString aDefinitionFile, OUString aResourcePath)
    : m_rDefinitionFile(std::move(aDefinitionFile))
    , m_rResourcePath(std::move(aResourcePath))
{
}

void WidgetDefinitionReader::readDrawingDefinition(
    tools::XmlWalker& rWalker, const std::shared_ptr<WidgetDefinitionState>& rpState)
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

            OString sX1 = rWalker.attribute("x1");
            float fX1 = sX1.isEmpty() ? 0.0 : sX1.toFloat();

            OString sY1 = rWalker.attribute("y1");
            float fY1 = sY1.isEmpty() ? 0.0 : sY1.toFloat();

            OString sX2 = rWalker.attribute("x2");
            float fX2 = sX2.isEmpty() ? 1.0 : sX2.toFloat();

            OString sY2 = rWalker.attribute("y2");
            float fY2 = sY2.isEmpty() ? 1.0 : sY2.toFloat();

            rpState->addDrawRectangle(aStrokeColor, nStrokeWidth, aFillColor, fX1, fY1, fX2, fY2,
                                      nRx, nRy);
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
        else if (rWalker.name() == "image")
        {
            OString sSource = rWalker.attribute("source");
            rpState->addDrawImage(m_rResourcePath
                                  + OStringToOUString(sSource, RTL_TEXTENCODING_UTF8));
        }
        else if (rWalker.name() == "external")
        {
            OString sSource = rWalker.attribute("source");
            rpState->addDrawExternal(m_rResourcePath
                                     + OStringToOUString(sSource, RTL_TEXTENCODING_UTF8));
        }
        rWalker.next();
    }
    rWalker.parent();
}

void WidgetDefinitionReader::readDefinition(tools::XmlWalker& rWalker,
                                            WidgetDefinition& rWidgetDefinition, ControlType eType)
{
    rWalker.children();
    while (rWalker.isValid())
    {
        if (rWalker.name() == "part")
        {
            OString sPart = rWalker.attribute("value");
            ControlPart ePart = xmlStringToControlPart(sPart);

            std::shared_ptr<WidgetDefinitionPart> pPart = std::make_shared<WidgetDefinitionPart>();

            OString sWidth = rWalker.attribute("width");
            if (!sWidth.isEmpty())
            {
                sal_Int32 nWidth = sWidth.isEmpty() ? 0 : sWidth.toInt32();
                pPart->mnWidth = nWidth;
            }

            OString sHeight = rWalker.attribute("height");
            if (!sHeight.isEmpty())
            {
                sal_Int32 nHeight = sHeight.isEmpty() ? 0 : sHeight.toInt32();
                pPart->mnHeight = nHeight;
            }

            OString sMarginHeight = rWalker.attribute("margin-height");
            if (!sMarginHeight.isEmpty())
            {
                sal_Int32 nMarginHeight = sMarginHeight.isEmpty() ? 0 : sMarginHeight.toInt32();
                pPart->mnMarginHeight = nMarginHeight;
            }

            OString sMarginWidth = rWalker.attribute("margin-width");
            if (!sMarginWidth.isEmpty())
            {
                sal_Int32 nMarginWidth = sMarginWidth.isEmpty() ? 0 : sMarginWidth.toInt32();
                pPart->mnMarginWidth = nMarginWidth;
            }

            OString sOrientation = rWalker.attribute("orientation");
            if (!sOrientation.isEmpty())
            {
                pPart->msOrientation = sOrientation;
            }

            rWidgetDefinition.maDefinitions.emplace(ControlTypeAndPart(eType, ePart), pPart);
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
            OString sEnabled = getValueOrAny(rWalker.attribute("enabled"));
            OString sFocused = getValueOrAny(rWalker.attribute("focused"));
            OString sPressed = getValueOrAny(rWalker.attribute("pressed"));
            OString sRollover = getValueOrAny(rWalker.attribute("rollover"));
            OString sDefault = getValueOrAny(rWalker.attribute("default"));
            OString sSelected = getValueOrAny(rWalker.attribute("selected"));
            OString sButtonValue = getValueOrAny(rWalker.attribute("button-value"));
            OString sExtra = getValueOrAny(rWalker.attribute("extra"));

            std::shared_ptr<WidgetDefinitionState> pState = std::make_shared<WidgetDefinitionState>(
                sEnabled, sFocused, sPressed, sRollover, sDefault, sSelected, sButtonValue, sExtra);

            rpPart->maStates.push_back(pState);
            readDrawingDefinition(rWalker, pState);
        }
        rWalker.next();
    }
    rWalker.parent();
}

bool WidgetDefinitionReader::read(WidgetDefinition& rWidgetDefinition)
{
    if (!lcl_fileExists(m_rDefinitionFile))
        return false;

    auto pStyle = std::make_shared<WidgetDefinitionStyle>();

    std::unordered_map<OString, Color*> aStyleColorMap = {
        { "faceColor", &pStyle->maFaceColor },
        { "checkedColor", &pStyle->maCheckedColor },
        { "lightColor", &pStyle->maLightColor },
        { "lightBorderColor", &pStyle->maLightBorderColor },
        { "shadowColor", &pStyle->maShadowColor },
        { "darkShadowColor", &pStyle->maDarkShadowColor },
        { "buttonTextColor", &pStyle->maButtonTextColor },
        { "defaultActionButtonTextColor", &pStyle->maDefaultActionButtonTextColor },
        { "actionButtonTextColor", &pStyle->maActionButtonTextColor },
        { "actionButtonRolloverTextColor", &pStyle->maActionButtonRolloverTextColor },
        { "buttonRolloverTextColor", &pStyle->maButtonRolloverTextColor },
        { "radioCheckTextColor", &pStyle->maRadioCheckTextColor },
        { "groupTextColor", &pStyle->maGroupTextColor },
        { "labelTextColor", &pStyle->maLabelTextColor },
        { "windowColor", &pStyle->maWindowColor },
        { "windowTextColor", &pStyle->maWindowTextColor },
        { "dialogColor", &pStyle->maDialogColor },
        { "dialogTextColor", &pStyle->maDialogTextColor },
        { "workspaceColor", &pStyle->maWorkspaceColor },
        { "monoColor", &pStyle->maMonoColor },
        { "fieldColor", &pStyle->maFieldColor },
        { "fieldTextColor", &pStyle->maFieldTextColor },
        { "fieldRolloverTextColor", &pStyle->maFieldRolloverTextColor },
        { "activeColor", &pStyle->maActiveColor },
        { "activeTextColor", &pStyle->maActiveTextColor },
        { "activeBorderColor", &pStyle->maActiveBorderColor },
        { "deactiveColor", &pStyle->maDeactiveColor },
        { "deactiveTextColor", &pStyle->maDeactiveTextColor },
        { "deactiveBorderColor", &pStyle->maDeactiveBorderColor },
        { "menuColor", &pStyle->maMenuColor },
        { "menuBarColor", &pStyle->maMenuBarColor },
        { "menuBarRolloverColor", &pStyle->maMenuBarRolloverColor },
        { "menuBorderColor", &pStyle->maMenuBorderColor },
        { "menuTextColor", &pStyle->maMenuTextColor },
        { "menuBarTextColor", &pStyle->maMenuBarTextColor },
        { "menuBarRolloverTextColor", &pStyle->maMenuBarRolloverTextColor },
        { "menuBarHighlightTextColor", &pStyle->maMenuBarHighlightTextColor },
        { "menuHighlightColor", &pStyle->maMenuHighlightColor },
        { "menuHighlightTextColor", &pStyle->maMenuHighlightTextColor },
        { "highlightColor", &pStyle->maHighlightColor },
        { "highlightTextColor", &pStyle->maHighlightTextColor },
        { "activeTabColor", &pStyle->maActiveTabColor },
        { "inactiveTabColor", &pStyle->maInactiveTabColor },
        { "tabTextColor", &pStyle->maTabTextColor },
        { "tabRolloverTextColor", &pStyle->maTabRolloverTextColor },
        { "tabHighlightTextColor", &pStyle->maTabHighlightTextColor },
        { "disableColor", &pStyle->maDisableColor },
        { "helpColor", &pStyle->maHelpColor },
        { "helpTextColor", &pStyle->maHelpTextColor },
        { "linkColor", &pStyle->maLinkColor },
        { "visitedLinkColor", &pStyle->maVisitedLinkColor },
        { "toolTextColor", &pStyle->maToolTextColor },
    };

    rWidgetDefinition.mpStyle = pStyle;

    auto pSettings = std::make_shared<WidgetDefinitionSettings>();

    std::unordered_map<OString, OString*> aSettingMap = {
        { "noActiveTabTextRaise", &pSettings->msNoActiveTabTextRaise },
        { "centeredTabs", &pSettings->msCenteredTabs },
        { "listBoxEntryMargin", &pSettings->msListBoxEntryMargin },
        { "defaultFontSize", &pSettings->msDefaultFontSize },
        { "titleHeight", &pSettings->msTitleHeight },
        { "floatTitleHeight", &pSettings->msFloatTitleHeight },
        { "listBoxPreviewDefaultLogicWidth", &pSettings->msListBoxPreviewDefaultLogicWidth },
        { "listBoxPreviewDefaultLogicHeight", &pSettings->msListBoxPreviewDefaultLogicHeight },
    };

    rWidgetDefinition.mpSettings = pSettings;

    SvFileStream aFileStream(m_rDefinitionFile, StreamMode::READ);

    tools::XmlWalker aWalker;
    if (!aWalker.open(&aFileStream))
        return false;

    if (aWalker.name() != "widgets")
        return false;

    aWalker.children();
    while (aWalker.isValid())
    {
        ControlType eType;
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
        if (aWalker.name() == "settings")
        {
            aWalker.children();
            while (aWalker.isValid())
            {
                auto pair = aSettingMap.find(aWalker.name());
                if (pair != aSettingMap.end())
                {
                    readSetting(aWalker.attribute("value"), *pair->second);
                }
                aWalker.next();
            }
            aWalker.parent();
        }
        else if (getControlTypeForXmlString(aWalker.name(), eType))
        {
            readDefinition(aWalker, rWidgetDefinition, eType);
        }
        aWalker.next();
    }
    aWalker.parent();

    return true;
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
