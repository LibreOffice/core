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

#ifndef INCLUDED_VCL_INC_CONTROLCACHEKEY_HXX
#define INCLUDED_VCL_INC_CONTROLCACHEKEY_HXX

#include <tools/gen.hxx>
#include <vcl/salnativewidgets.hxx>

class ControlCacheKey
{
public:
    ControlType mnType;
    ControlPart mnPart;
    ControlState mnState;
    Size maSize;

    ControlCacheKey(ControlType nType, ControlPart nPart, ControlState nState, const Size& rSize)
        : mnType(nType)
        , mnPart(nPart)
        , mnState(nState)
        , maSize(rSize)
    {}

    bool operator==(ControlCacheKey const& aOther) const
    {
        return mnType == aOther.mnType
            && mnPart == aOther.mnPart
            && mnState == aOther.mnState
            && maSize.Width() == aOther.maSize.Width()
            && maSize.Height() == aOther.maSize.Height();
    }

    bool canCacheControl()
    {
        switch(mnType)
        {
            case ControlType::Checkbox:
            case ControlType::Radiobutton:
            case ControlType::ListNode:
            case ControlType::Slider:
            case ControlType::Progress:
            // FIXME: these guys have complex state hidden in ImplControlValue
            // structs which affects rendering, needs to be a and needs to be
            // part of the key to our cache.
            case ControlType::Spinbox:
            case ControlType::SpinButtons:
            case ControlType::TabItem:
                return false;

            case ControlType::Menubar:
                if (mnPart == ControlPart::Entire)
                    return false;
                break;

            default:
                break;
        }
        return true;
    }
};

#endif // INCLUDED_VCL_INC_CONTROLCACHEKEY_HXX