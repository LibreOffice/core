/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <editeng/editengdllapi.h>
#include <editeng/outliner.hxx>

class EDITENG_DLLPUBLIC URLFieldHelper
{
public:
    static void RemoveURLField(Outliner* pOutl, OutlinerView* pOLV);
    static bool IsCursorAtURLField(OutlinerView* pOLV);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
