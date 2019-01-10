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
#include <tools/XmlWalker.hxx>
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
        aWalker.next();
    }
    aWalker.parent();

    return true;
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
