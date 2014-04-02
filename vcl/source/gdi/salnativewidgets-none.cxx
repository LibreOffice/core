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

#include <salgdi.hxx>

/****************************************************************
 *  Placeholder for no native widgets
 ***************************************************************/

/*
 * IsNativeControlSupported()
 *
 *  Returns true if the platform supports native
 *  drawing of the control defined by nPart
 */
bool SalGraphics::IsNativeControlSupported( ControlType, ControlPart )
{
    return false;
}

/*
 * HitTestNativeControl()
 *
 *  If the return value is true, bIsInside contains information whether
 *  aPos was or was not inside the native widget specified by the
 *  nType/nPart combination.
 */
bool SalGraphics::hitTestNativeControl( ControlType,
                              ControlPart,
                              const Rectangle&,
                              const Point&,
                              bool& )
{
    return false;
}

/*
 * DrawNativeControl()
 *
 *  Draws the requested control described by nPart/nState.
 *
 *  rControlRegion: The bounding region of the complete control in VCL frame coordinates.
 *  aValue:         An optional value (tristate/numerical/string)
 *  aCaption:   A caption or title string (like button text etc)
 */
bool SalGraphics::drawNativeControl(    ControlType,
                            ControlPart,
                            const Rectangle&,
                            ControlState,
                            const ImplControlValue&,
                            const OUString& )
{
    return false;
}

/*
 * GetNativeControlRegion()
 *
 *  If the return value is true, rNativeBoundingRegion
 *  contains the true bounding region covered by the control
 *  including any adornment, while rNativeContentRegion contains the area
 *  within the control that can be safely drawn into without drawing over
 *  the borders of the control.
 *
 *  rControlRegion: The bounding region of the control in VCL frame coordinates.
 *  aValue:     An optional value (tristate/numerical/string)
 *  aCaption:       A caption or title string (like button text etc)
 */
bool SalGraphics::getNativeControlRegion(  ControlType,
                                ControlPart,
                                const Rectangle&,
                                ControlState,
                                const ImplControlValue&,
                                const OUString&,
                                Rectangle &,
                                Rectangle & )
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
