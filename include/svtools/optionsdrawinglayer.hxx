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

#include <svtools/svtdllapi.h>
#include <sal/types.h>

class Color;

namespace SvtOptionsDrawinglayer
{

/*-****************************************************************************************************
    @short      interface methods to get and set value of config key "org.openoffice.Office.Common/Drawinglayer/..."
    @descr      These options describe internal states to enable/disable features of installed office.

                GetStripeColorA()
                SetStripeColorA()       =>  Set first of two colors which overlay uses to draw stripes

                GetStripeColorB()
                SetStripeColorB()       =>  Set second of two colors which overlay uses to draw stripes

    @seealso    configuration package "org.openoffice.Office.Common/Drawinglayer"
*//*-*****************************************************************************************************/

SVT_DLLPUBLIC Color       GetStripeColorA();
SVT_DLLPUBLIC Color       GetStripeColorB();

// primitives
SVT_DLLPUBLIC bool        IsAntiAliasing();
SVT_DLLPUBLIC bool        IsSnapHorVerLinesToDiscrete();

SVT_DLLPUBLIC void        SetAntiAliasing( bool bOn, bool bTemporary );

// #i97672# selection settings
SVT_DLLPUBLIC sal_uInt16  GetTransparentSelectionPercent();
SVT_DLLPUBLIC sal_uInt16  GetSelectionMaximumLuminancePercent();

// get system highlight color, limited to the maximum allowed luminance
// (defined in GetSelectionMaximumLuminancePercent() in SvtOptionsDrawinglayer,
// combined with Application::GetSettings().GetStyleSettings().GetHighlightColor())
SVT_DLLPUBLIC Color getHilightColor();

} // namespace SvtOptionsDrawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
