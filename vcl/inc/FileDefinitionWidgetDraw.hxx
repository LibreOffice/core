/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_FILEDEFINITIONWIDGETDRAW_HXX
#define INCLUDED_VCL_INC_FILEDEFINITIONWIDGETDRAW_HXX

#include <vcl/dllapi.h>
#include "widgetdraw/WidgetDefinitionReader.hxx"
#include "salgdi.hxx"
#include "WidgetDrawInterface.hxx"

namespace vcl
{
class FileDefinitionWidgetDraw : public vcl::WidgetDrawInterface
{
private:
    SalGraphics& m_rGraphics;
    std::shared_ptr<WidgetDefinition> m_pWidgetDefinition;

    bool resolveDefinition(ControlType eType, ControlPart ePart, ControlState eState,
                           const ImplControlValue& rValue, long nX, long nY, long nWidth,
                           long nHeight);

public:
    FileDefinitionWidgetDraw(SalGraphics& rGraphics);

    bool isNativeControlSupported(ControlType eType, ControlPart ePart) override;

    bool hitTestNativeControl(ControlType eType, ControlPart ePart,
                              const tools::Rectangle& rBoundingControlRegion, const Point& aPos,
                              bool& rIsInside) override;

    bool drawNativeControl(ControlType eType, ControlPart ePart,
                           const tools::Rectangle& rBoundingControlRegion, ControlState eState,
                           const ImplControlValue& aValue, const OUString& aCaptions) override;

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
