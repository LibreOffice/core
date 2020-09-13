/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "emfphelperdata.hxx"
#include <vector>

namespace emfplushelper
{
const sal_uInt32 EmpPlusRectClamp = 0x00000000;
const sal_uInt32 EmpPlusBitmapClamp = 0x00000001;

struct EMFPImageAttributes : public EMFPObject
{
    sal_uInt32 wrapMode;
    Color clampColor;
    sal_uInt32 objectClamp;

    EMFPImageAttributes();

    virtual ~EMFPImageAttributes() override;

    void Read(SvStream& s);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
