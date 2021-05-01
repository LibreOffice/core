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

#include <basegfx/color/bcolor.hxx>

namespace svgio::svgreader
    {
        class SvgPaint
        {
        private:
            basegfx::BColor     maColor;

            bool                mbSet : 1;
            bool                mbOn : 1;
            bool                mbCurrent : 1;

        public:
            SvgPaint(const basegfx::BColor& rColor = basegfx::BColor(0.0, 0.0, 0.0), bool bSet = false, bool bOn = false, bool bCurrent = false)
            :   maColor(rColor),
                mbSet(bSet),
                mbOn(bOn),
                mbCurrent(bCurrent)
            {
            }

            const basegfx::BColor& getBColor() const { return maColor; }
            bool isSet() const { return mbSet; }
            bool isOn() const { return mbOn; }
            bool isCurrent() const { return mbCurrent; }
        };

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
