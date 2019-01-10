/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_WIDGETDEFINITIONREADER_HXX
#define INCLUDED_VCL_INC_WIDGETDEFINITIONREADER_HXX

#include <vcl/dllapi.h>
#include <memory>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>

namespace vcl
{
class VCL_DLLPUBLIC WidgetDefinitionReader
{
private:
    OUString m_rFilePath;

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

    WidgetDefinitionReader(OUString const& rFilePath);
    bool read();
};

} // end vcl namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
