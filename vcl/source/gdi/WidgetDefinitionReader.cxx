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

OString getValueOrAny(OString const& rInputString)
{
    if (rInputString.isEmpty())
        return "any";
    return rInputString;
}

ControlPart xmlStringToControlPart(OString const& sPart)
{
    if (sPart.equalsIgnoreAsciiCase("NONE"))
        return ControlPart::NONE;
    else if (sPart.equalsIgnoreAsciiCase("Entire"))
        return ControlPart::Entire;
    else if (sPart.equalsIgnoreAsciiCase("ListboxWindow"))
        return ControlPart::ListboxWindow;
    else if (sPart.equalsIgnoreAsciiCase("Button"))
        return ControlPart::Button;
    else if (sPart.equalsIgnoreAsciiCase("ButtonUp"))
        return ControlPart::ButtonUp;
    else if (sPart.equalsIgnoreAsciiCase("ButtonDown"))
        return ControlPart::ButtonDown;
    else if (sPart.equalsIgnoreAsciiCase("ButtonLeft"))
        return ControlPart::ButtonLeft;
    else if (sPart.equalsIgnoreAsciiCase("ButtonRight"))
        return ControlPart::ButtonRight;
    else if (sPart.equalsIgnoreAsciiCase("AllButtons"))
        return ControlPart::AllButtons;
    else if (sPart.equalsIgnoreAsciiCase("SeparatorHorz"))
        return ControlPart::SeparatorHorz;
    else if (sPart.equalsIgnoreAsciiCase("SeparatorVert"))
        return ControlPart::SeparatorVert;
    else if (sPart.equalsIgnoreAsciiCase("TrackHorzLeft"))
        return ControlPart::TrackHorzLeft;
    else if (sPart.equalsIgnoreAsciiCase("TrackVertUpper"))
        return ControlPart::TrackVertUpper;
    else if (sPart.equalsIgnoreAsciiCase("TrackHorzRight"))
        return ControlPart::TrackHorzRight;
    else if (sPart.equalsIgnoreAsciiCase("TrackVertLower"))
        return ControlPart::TrackVertLower;
    else if (sPart.equalsIgnoreAsciiCase("TrackHorzArea"))
        return ControlPart::TrackHorzArea;
    else if (sPart.equalsIgnoreAsciiCase("TrackVertArea"))
        return ControlPart::TrackVertArea;
    else if (sPart.equalsIgnoreAsciiCase("Arrow"))
        return ControlPart::Arrow;
    else if (sPart.equalsIgnoreAsciiCase("ThumbHorz"))
        return ControlPart::ThumbHorz;
    else if (sPart.equalsIgnoreAsciiCase("ThumbVert"))
        return ControlPart::ThumbVert;
    else if (sPart.equalsIgnoreAsciiCase("MenuItem"))
        return ControlPart::MenuItem;
    else if (sPart.equalsIgnoreAsciiCase("MenuItemCheckMark"))
        return ControlPart::MenuItemCheckMark;
    else if (sPart.equalsIgnoreAsciiCase("MenuItemRadioMark"))
        return ControlPart::MenuItemRadioMark;
    else if (sPart.equalsIgnoreAsciiCase("Separator"))
        return ControlPart::Separator;
    else if (sPart.equalsIgnoreAsciiCase("SubmenuArrow"))
        return ControlPart::SubmenuArrow;
    else if (sPart.equalsIgnoreAsciiCase("SubEdit"))
        return ControlPart::SubEdit;
    else if (sPart.equalsIgnoreAsciiCase("DrawBackgroundHorz"))
        return ControlPart::DrawBackgroundHorz;
    else if (sPart.equalsIgnoreAsciiCase("DrawBackgroundVert"))
        return ControlPart::DrawBackgroundVert;
    else if (sPart.equalsIgnoreAsciiCase("TabsDrawRtl"))
        return ControlPart::TabsDrawRtl;
    else if (sPart.equalsIgnoreAsciiCase("HasBackgroundTexture"))
        return ControlPart::HasBackgroundTexture;
    else if (sPart.equalsIgnoreAsciiCase("HasThreeButtons"))
        return ControlPart::HasThreeButtons;
    else if (sPart.equalsIgnoreAsciiCase("BackgroundWindow"))
        return ControlPart::BackgroundWindow;
    else if (sPart.equalsIgnoreAsciiCase("BackgroundDialog"))
        return ControlPart::BackgroundDialog;
    else if (sPart.equalsIgnoreAsciiCase("Border"))
        return ControlPart::Border;
    else if (sPart.equalsIgnoreAsciiCase("Focus"))
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

WidgetDefinitionReader::WidgetDefinitionReader(OUString const& rDefinitionFile,
                                               OUString const& rResourcePath)
    : m_rDefinitionFile(rDefinitionFile)
    , m_rResourcePath(rResourcePath)
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

            OString sX1 = rWalker.attribute("x1");
            float fX1 = sX1.isEmpty() ? 0.0 : sX1.toFloat();

            OString sY1 = rWalker.attribute("y1");
            float fY1 = sY1.isEmpty() ? 0.0 : sY1.toFloat();

            OString sX2 = rWalker.attribute("x2");
            float fX2 = sX2.isEmpty() ? 1.0 : sX2.toFloat();

            OString sY2 = rWalker.attribute("y2");
            float fY2 = sY2.isEmpty() ? 1.0 : sY2.toFloat();

            rpState->addDrawCircle(aStrokeColor, nStrokeWidth, aFillColor, fX1, fY1, fX2, fY2);
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
        { "fontColor", &pStyle->maFontColor },
    };
    rWidgetDefinition.mpStyle = pStyle;

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
