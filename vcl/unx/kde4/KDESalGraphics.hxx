/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <memory>

#include <rtl/string.hxx>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>

#include <QImage>

/** handles graphics drawings requests and performs the needed drawing operations */
class KDESalGraphics : public X11SalGraphics
{
    std::unique_ptr<QImage> m_image;
    QRect lastPopupRect;

    public:
        /**
            What widgets can be drawn the native way.
            @param type Type of the widget.
            @param part Specification of the widget's part if it consists of more than one.
            @return true if the platform supports native drawing of the widget type defined by part.
        */
        virtual bool IsNativeControlSupported( ControlType type, ControlPart part ) override;

        /** Test whether the position is in the native widget.
            If the return value is TRUE, bIsInside contains information whether
            aPos was or was not inside the native widget specified by the
            type/part combination.
        */
        virtual bool hitTestNativeControl( ControlType type, ControlPart part,
                                        const Rectangle& rControlRegion, const Point& aPos,
                                        bool& rIsInside ) override;
        /** Draw the requested control described by part/nControlState.

            @param rControlRegion
            The bounding Rectangle of the complete control in VCL frame coordinates.

            @param aValue
            An optional value (tristate/numerical/string).

            @param aCaption
            A caption or title string (like button text etc.)
        */
        virtual bool drawNativeControl( ControlType type, ControlPart part,
                                        const Rectangle& rControlRegion, ControlState nControlState,
                                        const ImplControlValue& aValue,
                                        const OUString& aCaption ) override;

        /** Check if the bounding regions match.

            If the return value is TRUE, rNativeBoundingRegion
            contains the true bounding region covered by the control
            including any adornment, while rNativeContentRegion contains the area
            within the control that can be safely drawn into without drawing over
            the borders of the control.

            @param rControlRegion
            The bounding region of the control in VCL frame coordinates.

            @param aValue
            An optional value (tristate/numerical/string)

            @param aCaption
            A caption or title string (like button text etc.)
        */
        virtual bool getNativeControlRegion( ControlType type, ControlPart part,
                                            const Rectangle& rControlRegion, ControlState nControlState,
                                            const ImplControlValue& aValue,
                                            const OUString& aCaption,
                                            Rectangle &rNativeBoundingRegion, Rectangle &rNativeContentRegion ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
