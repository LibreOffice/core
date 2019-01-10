/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <FileDefinitionWidgetDraw.hxx>

#include <sal/config.h>
#include <svdata.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>

namespace vcl
{
namespace
{
OUString lcl_getClassificationUserPath()
{
    OUString sPath("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER
                   "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/user/");
    rtl::Bootstrap::expandMacros(sPath);
    return sPath;
}

} // end anonymous namespace

FileDefinitionWidgetDraw::FileDefinitionWidgetDraw(SalGraphics& rGraphics)
    : m_rGraphics(rGraphics)
    , m_WidgetDefinitionReader(lcl_getClassificationUserPath() + "definition.xml")
{
    m_WidgetDefinitionReader.read();

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maNWFData.mbNoFocusRects = true;
    pSVData->maNWFData.mbNoFocusRectsForFlatButtons = true;
}

bool FileDefinitionWidgetDraw::isNativeControlSupported(ControlType /*eType*/,
                                                        ControlPart /*ePart*/)
{
    return false;
}

bool FileDefinitionWidgetDraw::hitTestNativeControl(
    ControlType /*eType*/, ControlPart /*ePart*/,
    const tools::Rectangle& /*rBoundingControlRegion*/, const Point& /*aPos*/, bool& /*rIsInside*/)
{
    return false;
}

bool FileDefinitionWidgetDraw::drawNativeControl(ControlType /*eType*/, ControlPart /*ePart*/,
                                                 const tools::Rectangle& /*rControlRegion*/,
                                                 ControlState /*eState*/,
                                                 const ImplControlValue& /*rValue*/,
                                                 const OUString& /*aCaptions*/)
{
    (void)m_rGraphics; // avoid unused warning
    return false;
}

bool FileDefinitionWidgetDraw::getNativeControlRegion(
    ControlType /*eType*/, ControlPart /*ePart*/,
    const tools::Rectangle& /*rBoundingControlRegion*/, ControlState /*eState*/,
    const ImplControlValue& /*aValue*/, const OUString& /*aCaption*/,
    tools::Rectangle& /*rNativeBoundingRegion*/, tools::Rectangle& /*rNativeContentRegion*/)
{
    return false;
}

bool FileDefinitionWidgetDraw::updateSettings(AllSettings& rSettings)
{
    StyleSettings aStyleSet = rSettings.GetStyleSettings();

    aStyleSet.SetFaceColor(m_WidgetDefinitionReader.maFaceColor);
    aStyleSet.SetCheckedColor(m_WidgetDefinitionReader.maCheckedColor);
    aStyleSet.SetLightColor(m_WidgetDefinitionReader.maLightColor);
    aStyleSet.SetLightBorderColor(m_WidgetDefinitionReader.maLightBorderColor);
    aStyleSet.SetShadowColor(m_WidgetDefinitionReader.maShadowColor);
    aStyleSet.SetDarkShadowColor(m_WidgetDefinitionReader.maDarkShadowColor);
    aStyleSet.SetButtonTextColor(m_WidgetDefinitionReader.maButtonTextColor);
    aStyleSet.SetButtonRolloverTextColor(m_WidgetDefinitionReader.maButtonRolloverTextColor);
    aStyleSet.SetRadioCheckTextColor(m_WidgetDefinitionReader.maRadioCheckTextColor);
    aStyleSet.SetGroupTextColor(m_WidgetDefinitionReader.maGroupTextColor);
    aStyleSet.SetLabelTextColor(m_WidgetDefinitionReader.maLabelTextColor);
    aStyleSet.SetWindowColor(m_WidgetDefinitionReader.maWindowColor);
    aStyleSet.SetWindowTextColor(m_WidgetDefinitionReader.maWindowTextColor);
    aStyleSet.SetDialogColor(m_WidgetDefinitionReader.maDialogColor);
    aStyleSet.SetDialogTextColor(m_WidgetDefinitionReader.maDialogTextColor);
    aStyleSet.SetWorkspaceColor(m_WidgetDefinitionReader.maWorkspaceColor);
    aStyleSet.SetMonoColor(m_WidgetDefinitionReader.maMonoColor);
    aStyleSet.SetFieldColor(m_WidgetDefinitionReader.maFieldColor);
    aStyleSet.SetFieldTextColor(m_WidgetDefinitionReader.maFieldTextColor);
    aStyleSet.SetFieldRolloverTextColor(m_WidgetDefinitionReader.maFieldRolloverTextColor);
    aStyleSet.SetActiveColor(m_WidgetDefinitionReader.maActiveColor);
    aStyleSet.SetActiveTextColor(m_WidgetDefinitionReader.maActiveTextColor);
    aStyleSet.SetActiveBorderColor(m_WidgetDefinitionReader.maActiveBorderColor);
    aStyleSet.SetDeactiveColor(m_WidgetDefinitionReader.maDeactiveColor);
    aStyleSet.SetDeactiveTextColor(m_WidgetDefinitionReader.maDeactiveTextColor);
    aStyleSet.SetDeactiveBorderColor(m_WidgetDefinitionReader.maDeactiveBorderColor);
    aStyleSet.SetMenuColor(m_WidgetDefinitionReader.maMenuColor);
    aStyleSet.SetMenuBarColor(m_WidgetDefinitionReader.maMenuBarColor);
    aStyleSet.SetMenuBarRolloverColor(m_WidgetDefinitionReader.maMenuBarRolloverColor);
    aStyleSet.SetMenuBorderColor(m_WidgetDefinitionReader.maMenuBorderColor);
    aStyleSet.SetMenuTextColor(m_WidgetDefinitionReader.maMenuTextColor);
    aStyleSet.SetMenuBarTextColor(m_WidgetDefinitionReader.maMenuBarTextColor);
    aStyleSet.SetMenuBarRolloverTextColor(m_WidgetDefinitionReader.maMenuBarRolloverTextColor);
    aStyleSet.SetMenuBarHighlightTextColor(m_WidgetDefinitionReader.maMenuBarHighlightTextColor);
    aStyleSet.SetMenuHighlightColor(m_WidgetDefinitionReader.maMenuHighlightColor);
    aStyleSet.SetMenuHighlightTextColor(m_WidgetDefinitionReader.maMenuHighlightTextColor);
    aStyleSet.SetHighlightColor(m_WidgetDefinitionReader.maHighlightColor);
    aStyleSet.SetHighlightTextColor(m_WidgetDefinitionReader.maHighlightTextColor);
    aStyleSet.SetActiveTabColor(m_WidgetDefinitionReader.maActiveTabColor);
    aStyleSet.SetInactiveTabColor(m_WidgetDefinitionReader.maInactiveTabColor);
    aStyleSet.SetTabTextColor(m_WidgetDefinitionReader.maTabTextColor);
    aStyleSet.SetTabRolloverTextColor(m_WidgetDefinitionReader.maTabRolloverTextColor);
    aStyleSet.SetTabHighlightTextColor(m_WidgetDefinitionReader.maTabHighlightTextColor);
    aStyleSet.SetDisableColor(m_WidgetDefinitionReader.maDisableColor);
    aStyleSet.SetHelpColor(m_WidgetDefinitionReader.maHelpColor);
    aStyleSet.SetHelpTextColor(m_WidgetDefinitionReader.maHelpTextColor);
    aStyleSet.SetLinkColor(m_WidgetDefinitionReader.maLinkColor);
    aStyleSet.SetVisitedLinkColor(m_WidgetDefinitionReader.maVisitedLinkColor);
    aStyleSet.SetToolTextColor(m_WidgetDefinitionReader.maToolTextColor);
    aStyleSet.SetFontColor(m_WidgetDefinitionReader.maFontColor);

    rSettings.SetStyleSettings(aStyleSet);

    return true;
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
