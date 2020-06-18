/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/checksum.hxx>
#include <tools/gen.hxx>

namespace vcl::pdf
{
struct BitmapID
{
    Size m_aPixelSize;
    sal_Int32 m_nSize;
    BitmapChecksum m_nChecksum;
    BitmapChecksum m_nMaskChecksum;

    BitmapID()
        : m_nSize(0)
        , m_nChecksum(0)
        , m_nMaskChecksum(0)
    {
    }

    bool operator==(const BitmapID& rComp) const
    {
        return (m_aPixelSize == rComp.m_aPixelSize && m_nSize == rComp.m_nSize
                && m_nChecksum == rComp.m_nChecksum && m_nMaskChecksum == rComp.m_nMaskChecksum);
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
