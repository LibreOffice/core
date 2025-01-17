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
#include <o3tl/numeric.hxx>
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

bool readColor(OString const& rString, Color& rColor)
{
    if (rString.getLength() != 7)
        return false;

    const char aChar(rString[0]);

    if (aChar != '#')
        return false;

    rColor.SetRed(o3tl::convertToHex<sal_Int32>(rString[1], rString[2]));
    rColor.SetGreen(o3tl::convertToHex<sal_Int32>(rString[3], rString[4]));
    rColor.SetBlue(o3tl::convertToHex<sal_Int32>(rString[5], rString[6]));

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
        return "any"_ostr;
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

bool getControlTypeForXmlString(std::string_view rString, ControlType& reType)
{
    static std::unordered_map<std::string_view, ControlType> aPartMap = {
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
        { "levelbar", ControlType::LevelBar },
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

    auto const aIterator = aPartMap.find(rString);
    if (aIterator != aPartMap.end())
    {
        reType = aIterator->second;
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
            readColor(rWalker.attribute("stroke"_ostr), aStrokeColor);
            Color aFillColor;
            readColor(rWalker.attribute("fill"_ostr), aFillColor);
            OString sStrokeWidth = rWalker.attribute("stroke-width"_ostr);
            sal_Int32 nStrokeWidth = -1;
            if (!sStrokeWidth.isEmpty())
                nStrokeWidth = sStrokeWidth.toInt32();

            sal_Int32 nRx = -1;
            OString sRx = rWalker.attribute("rx"_ostr);
            if (!sRx.isEmpty())
                nRx = sRx.toInt32();

            sal_Int32 nRy = -1;
            OString sRy = rWalker.attribute("ry"_ostr);
            if (!sRy.isEmpty())
                nRy = sRy.toInt32();

            OString sX1 = rWalker.attribute("x1"_ostr);
            float fX1 = sX1.isEmpty() ? 0.0 : sX1.toFloat();

            OString sY1 = rWalker.attribute("y1"_ostr);
            float fY1 = sY1.isEmpty() ? 0.0 : sY1.toFloat();

            OString sX2 = rWalker.attribute("x2"_ostr);
            float fX2 = sX2.isEmpty() ? 1.0 : sX2.toFloat();

            OString sY2 = rWalker.attribute("y2"_ostr);
            float fY2 = sY2.isEmpty() ? 1.0 : sY2.toFloat();

            rpState->addDrawRectangle(aStrokeColor, nStrokeWidth, aFillColor, fX1, fY1, fX2, fY2,
                                      nRx, nRy);
        }
        else if (rWalker.name() == "line")
        {
            Color aStrokeColor;
            readColor(rWalker.attribute("stroke"_ostr), aStrokeColor);

            OString sStrokeWidth = rWalker.attribute("stroke-width"_ostr);
            sal_Int32 nStrokeWidth = -1;
            if (!sStrokeWidth.isEmpty())
                nStrokeWidth = sStrokeWidth.toInt32();

            OString sX1 = rWalker.attribute("x1"_ostr);
            float fX1 = sX1.isEmpty() ? -1.0 : sX1.toFloat();

            OString sY1 = rWalker.attribute("y1"_ostr);
            float fY1 = sY1.isEmpty() ? -1.0 : sY1.toFloat();

            OString sX2 = rWalker.attribute("x2"_ostr);
            float fX2 = sX2.isEmpty() ? -1.0 : sX2.toFloat();

            OString sY2 = rWalker.attribute("y2"_ostr);
            float fY2 = sY2.isEmpty() ? -1.0 : sY2.toFloat();

            rpState->addDrawLine(aStrokeColor, nStrokeWidth, fX1, fY1, fX2, fY2);
        }
        else if (rWalker.name() == "image")
        {
            OString sSource = rWalker.attribute("source"_ostr);
            rpState->addDrawImage(m_rResourcePath
                                  + OStringToOUString(sSource, RTL_TEXTENCODING_UTF8));
        }
        else if (rWalker.name() == "external")
        {
            OString sSource = rWalker.attribute("source"_ostr);
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
            OString sPart = rWalker.attribute("value"_ostr);
            ControlPart ePart = xmlStringToControlPart(sPart);

            std::shared_ptr<WidgetDefinitionPart> pPart = std::make_shared<WidgetDefinitionPart>();

            OString sWidth = rWalker.attribute("width"_ostr);
            if (!sWidth.isEmpty())
            {
                sal_Int32 nWidth = sWidth.isEmpty() ? 0 : sWidth.toInt32();
                pPart->mnWidth = nWidth;
            }

            OString sHeight = rWalker.attribute("height"_ostr);
            if (!sHeight.isEmpty())
            {
                sal_Int32 nHeight = sHeight.isEmpty() ? 0 : sHeight.toInt32();
                pPart->mnHeight = nHeight;
            }

            OString sMarginHeight = rWalker.attribute("margin-height"_ostr);
            if (!sMarginHeight.isEmpty())
            {
                sal_Int32 nMarginHeight = sMarginHeight.isEmpty() ? 0 : sMarginHeight.toInt32();
                pPart->mnMarginHeight = nMarginHeight;
            }

            OString sMarginWidth = rWalker.attribute("margin-width"_ostr);
            if (!sMarginWidth.isEmpty())
            {
                sal_Int32 nMarginWidth = sMarginWidth.isEmpty() ? 0 : sMarginWidth.toInt32();
                pPart->mnMarginWidth = nMarginWidth;
            }

            OString sOrientation = rWalker.attribute("orientation"_ostr);
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
                                      const std::shared_ptr<WidgetDefinitionPart>& rpPart)
{
    rWalker.children();
    while (rWalker.isValid())
    {
        if (rWalker.name() == "state")
        {
            OString sEnabled = getValueOrAny(rWalker.attribute("enabled"_ostr));
            OString sFocused = getValueOrAny(rWalker.attribute("focused"_ostr));
            OString sPressed = getValueOrAny(rWalker.attribute("pressed"_ostr));
            OString sRollover = getValueOrAny(rWalker.attribute("rollover"_ostr));
            OString sDefault = getValueOrAny(rWalker.attribute("default"_ostr));
            OString sSelected = getValueOrAny(rWalker.attribute("selected"_ostr));
            OString sButtonValue = getValueOrAny(rWalker.attribute("button-value"_ostr));
            OString sExtra = getValueOrAny(rWalker.attribute("extra"_ostr));

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

    rWidgetDefinition.mpStyle = std::make_shared<WidgetDefinitionStyle>();

    std::unordered_map<std::string_view, Color*> aStyleColorMap = {
        { "faceColor", &rWidgetDefinition.mpStyle->maFaceColor },
        { "checkedColor", &rWidgetDefinition.mpStyle->maCheckedColor },
        { "lightColor", &rWidgetDefinition.mpStyle->maLightColor },
        { "lightBorderColor", &rWidgetDefinition.mpStyle->maLightBorderColor },
        { "shadowColor", &rWidgetDefinition.mpStyle->maShadowColor },
        { "darkShadowColor", &rWidgetDefinition.mpStyle->maDarkShadowColor },
        { "buttonTextColor", &rWidgetDefinition.mpStyle->maButtonTextColor },
        { "defaultActionButtonTextColor",
          &rWidgetDefinition.mpStyle->maDefaultActionButtonTextColor },
        { "actionButtonTextColor", &rWidgetDefinition.mpStyle->maActionButtonTextColor },
        { "actionButtonRolloverTextColor",
          &rWidgetDefinition.mpStyle->maActionButtonRolloverTextColor },
        { "buttonRolloverTextColor", &rWidgetDefinition.mpStyle->maButtonRolloverTextColor },
        { "radioCheckTextColor", &rWidgetDefinition.mpStyle->maRadioCheckTextColor },
        { "groupTextColor", &rWidgetDefinition.mpStyle->maGroupTextColor },
        { "labelTextColor", &rWidgetDefinition.mpStyle->maLabelTextColor },
        { "windowColor", &rWidgetDefinition.mpStyle->maWindowColor },
        { "windowTextColor", &rWidgetDefinition.mpStyle->maWindowTextColor },
        { "dialogColor", &rWidgetDefinition.mpStyle->maDialogColor },
        { "dialogTextColor", &rWidgetDefinition.mpStyle->maDialogTextColor },
        { "workspaceColor", &rWidgetDefinition.mpStyle->maWorkspaceColor },
        { "monoColor", &rWidgetDefinition.mpStyle->maMonoColor },
        { "fieldColor", &rWidgetDefinition.mpStyle->maFieldColor },
        { "fieldTextColor", &rWidgetDefinition.mpStyle->maFieldTextColor },
        { "fieldRolloverTextColor", &rWidgetDefinition.mpStyle->maFieldRolloverTextColor },
        { "activeColor", &rWidgetDefinition.mpStyle->maActiveColor },
        { "activeTextColor", &rWidgetDefinition.mpStyle->maActiveTextColor },
        { "activeBorderColor", &rWidgetDefinition.mpStyle->maActiveBorderColor },
        { "deactiveColor", &rWidgetDefinition.mpStyle->maDeactiveColor },
        { "deactiveTextColor", &rWidgetDefinition.mpStyle->maDeactiveTextColor },
        { "deactiveBorderColor", &rWidgetDefinition.mpStyle->maDeactiveBorderColor },
        { "menuColor", &rWidgetDefinition.mpStyle->maMenuColor },
        { "menuBarColor", &rWidgetDefinition.mpStyle->maMenuBarColor },
        { "menuBarRolloverColor", &rWidgetDefinition.mpStyle->maMenuBarRolloverColor },
        { "menuBorderColor", &rWidgetDefinition.mpStyle->maMenuBorderColor },
        { "menuTextColor", &rWidgetDefinition.mpStyle->maMenuTextColor },
        { "menuBarTextColor", &rWidgetDefinition.mpStyle->maMenuBarTextColor },
        { "menuBarRolloverTextColor", &rWidgetDefinition.mpStyle->maMenuBarRolloverTextColor },
        { "menuBarHighlightTextColor", &rWidgetDefinition.mpStyle->maMenuBarHighlightTextColor },
        { "menuHighlightColor", &rWidgetDefinition.mpStyle->maMenuHighlightColor },
        { "menuHighlightTextColor", &rWidgetDefinition.mpStyle->maMenuHighlightTextColor },
        { "highlightColor", &rWidgetDefinition.mpStyle->maHighlightColor },
        { "highlightTextColor", &rWidgetDefinition.mpStyle->maHighlightTextColor },
        { "activeTabColor", &rWidgetDefinition.mpStyle->maActiveTabColor },
        { "inactiveTabColor", &rWidgetDefinition.mpStyle->maInactiveTabColor },
        { "tabTextColor", &rWidgetDefinition.mpStyle->maTabTextColor },
        { "tabRolloverTextColor", &rWidgetDefinition.mpStyle->maTabRolloverTextColor },
        { "tabHighlightTextColor", &rWidgetDefinition.mpStyle->maTabHighlightTextColor },
        { "disableColor", &rWidgetDefinition.mpStyle->maDisableColor },
        { "helpColor", &rWidgetDefinition.mpStyle->maHelpColor },
        { "helpTextColor", &rWidgetDefinition.mpStyle->maHelpTextColor },
        { "linkColor", &rWidgetDefinition.mpStyle->maLinkColor },
        { "visitedLinkColor", &rWidgetDefinition.mpStyle->maVisitedLinkColor },
        { "toolTextColor", &rWidgetDefinition.mpStyle->maToolTextColor },
    };

    rWidgetDefinition.mpSettings = std::make_shared<WidgetDefinitionSettings>();

    std::unordered_map<std::string_view, OString*> aSettingMap = {
        { "noActiveTabTextRaise", &rWidgetDefinition.mpSettings->msNoActiveTabTextRaise },
        { "centeredTabs", &rWidgetDefinition.mpSettings->msCenteredTabs },
        { "listBoxEntryMargin", &rWidgetDefinition.mpSettings->msListBoxEntryMargin },
        { "defaultFontSize", &rWidgetDefinition.mpSettings->msDefaultFontSize },
        { "titleHeight", &rWidgetDefinition.mpSettings->msTitleHeight },
        { "floatTitleHeight", &rWidgetDefinition.mpSettings->msFloatTitleHeight },
        { "listBoxPreviewDefaultLogicWidth",
          &rWidgetDefinition.mpSettings->msListBoxPreviewDefaultLogicWidth },
        { "listBoxPreviewDefaultLogicHeight",
          &rWidgetDefinition.mpSettings->msListBoxPreviewDefaultLogicHeight },
    };

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
                    readColor(aWalker.attribute("value"_ostr), *pair->second);
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
                    readSetting(aWalker.attribute("value"_ostr), *pair->second);
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
