/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_WIDGETDRAWINTERFACE_HXX
#define INCLUDED_VCL_INC_WIDGETDRAWINTERFACE_HXX

#include <vcl/dllapi.h>
#include <vcl/salnativewidgets.hxx>
#include <vcl/settings.hxx>

namespace vcl
{
class VCL_PLUGIN_PUBLIC WidgetDrawInterface
{
public:
    virtual ~WidgetDrawInterface() COVERITY_NOEXCEPT_FALSE {}

    /**
     * Query the platform layer for native control support.
     *
     * @param [in] eType The widget type.
     * @param [in] ePart The part of the widget.
     * @return true if the platform supports native drawing of the widget type defined by part.
     */
    virtual inline bool isNativeControlSupported(ControlType eType, ControlPart ePart);

    /**
     * Query if a position is inside the native widget part.
     *
     * Mainly used for scrollbars.
     *
     * @param [in] eType The widget type.
     * @param [in] ePart The part of the widget.
     * @param [in] rBoundingControlRegion The bounding Rectangle of
                   the complete control in VCL frame coordinates.
     * @param [in] aPos The position to check the hit.
     * @param [out] rIsInside true, if \a aPos was inside the native widget.
     * @return true, if the query was successful.
     */
    virtual inline bool hitTestNativeControl(ControlType eType, ControlPart ePart,
                                             const tools::Rectangle& rBoundingControlRegion,
                                             const Point& aPos, bool& rIsInside);

    /**
     * Draw the requested control.
     *
     * @param [in] eType The widget type.
     * @param [in] ePart The part of the widget.
     * @param [in] rBoundingControlRegion The bounding rectangle of
     *             the complete control in VCL frame coordinates.
     * @param [in] eState The general state of the control (enabled, focused, etc.).
     * @param [in] aValue Addition control specific information.
     * @param [in] aCaption  A caption or title string (like button text etc.).
     * @param [in] rBackgroundColor Background color for the control (may be COL_AUTO)
     * @return true, if the control could be drawn.
     */
    virtual inline bool drawNativeControl(ControlType eType, ControlPart ePart,
                                          const tools::Rectangle& rBoundingControlRegion,
                                          ControlState eState, const ImplControlValue& aValue,
                                          const OUString& aCaptions, const Color& rBackgroundColor);

    /**
     * Get the native control regions for the control part.
     *
     * If the return value is true, \a rNativeBoundingRegion contains
     * the true bounding region covered by the control including any
     * adornment, while \a rNativeContentRegion contains the area
     * within the control that can be safely drawn into without drawing over
     * the borders of the control.
     *
     * @param [in] eType Type of the widget.
     * @param [in] ePart Specification of the widget's part if it consists of more than one.
     * @param [in] rBoundingControlRegion The bounding region of the control in VCL frame coordinates.
     * @param [in] eState The general state of the control (enabled, focused, etc.).
     * @param [in] aValue Addition control specific information.
     * @param [in] aCaption A caption or title string (like button text etc.).
     * @param [out] rNativeBoundingRegion The region covered by the control including any adornment.
     * @param [out] rNativeContentRegion The region within the control that can be safely drawn into.
     * @return true, if the regions are filled.
     */
    virtual inline bool getNativeControlRegion(ControlType eType, ControlPart ePart,
                                               const tools::Rectangle& rBoundingControlRegion,
                                               ControlState eState, const ImplControlValue& aValue,
                                               const OUString& aCaption,
                                               tools::Rectangle& rNativeBoundingRegion,
                                               tools::Rectangle& rNativeContentRegion);

    virtual inline bool updateSettings(AllSettings& rSettings);
};

bool WidgetDrawInterface::isNativeControlSupported(ControlType, ControlPart) { return false; }

bool WidgetDrawInterface::hitTestNativeControl(ControlType, ControlPart, const tools::Rectangle&,
                                               const Point&, bool&)
{
    return false;
}

bool WidgetDrawInterface::drawNativeControl(ControlType, ControlPart, const tools::Rectangle&,
                                            ControlState, const ImplControlValue&, const OUString&,
                                            const Color& /*rBackgroundColor*/)
{
    return false;
}

bool WidgetDrawInterface::getNativeControlRegion(ControlType, ControlPart, const tools::Rectangle&,
                                                 ControlState, const ImplControlValue&,
                                                 const OUString&, tools::Rectangle&,
                                                 tools::Rectangle&)
{
    return false;
}

bool WidgetDrawInterface::updateSettings(AllSettings&) { return false; }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
