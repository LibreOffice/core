/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <tools/gen.hxx>
#include <vcl/salnativewidgets.hxx>

/* ScrollbarValue:
 *
 *   Value container for scrollbars.
 */
class ScrollbarValue final : public ImplControlValue
{
public:
    tools::Long mnMin;
    tools::Long mnMax;
    tools::Long mnCur;
    tools::Long mnVisibleSize;
    tools::Rectangle maThumbRect;
    tools::Rectangle maButton1Rect;
    tools::Rectangle maButton2Rect;
    ControlState mnButton1State;
    ControlState mnButton2State;
    ControlState mnThumbState;

    ScrollbarValue()
        : ImplControlValue(ControlType::Scrollbar, 0)
    {
        mnMin = 0;
        mnMax = 0;
        mnCur = 0;
        mnVisibleSize = 0;
        mnButton1State = ControlState::NONE;
        mnButton2State = ControlState::NONE;
        mnThumbState = ControlState::NONE;
    };
    virtual ~ScrollbarValue() override;
    virtual ScrollbarValue* clone() const override;

    ScrollbarValue(ScrollbarValue const&) = default;
    ScrollbarValue(ScrollbarValue&&) = default;
    ScrollbarValue& operator=(ScrollbarValue const&) = delete; // due to ImplControlValue
    ScrollbarValue& operator=(ScrollbarValue&&) = delete; // due to ImplControlValue
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
