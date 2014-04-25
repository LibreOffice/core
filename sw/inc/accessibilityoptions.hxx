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
#ifndef INCLUDED_SW_INC_ACCESSIBILITYOPTIONS_HXX
#define INCLUDED_SW_INC_ACCESSIBILITYOPTIONS_HXX

#include <tools/solar.h>

struct SwAccessibilityOptions
{
    bool bIsAlwaysAutoColor         :1;
    bool bIsStopAnimatedText        :1;
    bool bIsStopAnimatedGraphics    :1;

    SwAccessibilityOptions() :
        bIsAlwaysAutoColor(false),
        bIsStopAnimatedText(false),
        bIsStopAnimatedGraphics(false) {}

    inline bool IsAlwaysAutoColor() const       { return bIsAlwaysAutoColor; }
    inline void SetAlwaysAutoColor( bool b )    { bIsAlwaysAutoColor = b; }

    inline bool IsStopAnimatedGraphics() const       { return bIsStopAnimatedText;}
    inline void SetStopAnimatedGraphics( bool b )    { bIsStopAnimatedText = b; }

    inline bool IsStopAnimatedText() const       { return bIsStopAnimatedGraphics; }
    inline void SetStopAnimatedText( bool b )    { bIsStopAnimatedGraphics = b;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
