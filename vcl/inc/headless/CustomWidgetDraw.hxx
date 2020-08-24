/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_CUSTOMWIDGETDRAW_HXX
#define INCLUDED_VCL_INC_CUSTOMWIDGETDRAW_HXX

#include <vcl/dllapi.h>
#include <WidgetDrawInterface.hxx>
#include <WidgetThemeLibrary.hxx>
#include <headless/svpgdi.hxx>
#include <vcl/settings.hxx>

namespace vcl
{
class CustomWidgetDraw final : public vcl::WidgetDrawInterface
{
private:
    static WidgetThemeLibrary* s_pWidgetImplementation;
    SvpSalGraphics& m_rGraphics;

public:
    CustomWidgetDraw(SvpSalGraphics& rGraphics);
    ~CustomWidgetDraw() override;

    bool isNativeControlSupported(ControlType eType, ControlPart ePart) override;

    bool hitTestNativeControl(ControlType eType, ControlPart ePart,
                              const tools::Rectangle& rBoundingControlRegion, const Point& aPos,
                              bool& rIsInside) override;

    bool drawNativeControl(ControlType eType, ControlPart ePart,
                           const tools::Rectangle& rBoundingControlRegion, ControlState eState,
                           const ImplControlValue& aValue, const OUString& aCaptions,
                           const Color& rBackgroundColor) override;

    bool getNativeControlRegion(ControlType eType, ControlPart ePart,
                                const tools::Rectangle& rBoundingControlRegion, ControlState eState,
                                const ImplControlValue& aValue, const OUString& aCaption,
                                tools::Rectangle& rNativeBoundingRegion,
                                tools::Rectangle& rNativeContentRegion) override;

    bool updateSettings(AllSettings& rSettings) override;
};

} // end vcl namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
