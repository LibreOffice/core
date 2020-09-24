/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_SCOPEDSTATES_HXX
#define INCLUDED_VCL_SCOPEDSTATES_HXX

#include <vcl/outdev.hxx>

namespace vcl
{
class ScopedAntialiasing
{
private:
    OutputDevice& m_rOutputDevice;
    AntialiasingFlags m_nPreviousAAState;

public:
    ScopedAntialiasing(OutputDevice& rOutputDevice, bool bAAState)
        : m_rOutputDevice(rOutputDevice)
        , m_nPreviousAAState(m_rOutputDevice.GetAntialiasing())
    {
        if (bAAState)
            rOutputDevice.SetAntialiasing(m_nPreviousAAState | AntialiasingFlags::Enable);
        else
            rOutputDevice.SetAntialiasing(m_nPreviousAAState & ~AntialiasingFlags::Enable);
    }

    ~ScopedAntialiasing() { m_rOutputDevice.SetAntialiasing(m_nPreviousAAState); }
};
}

#endif // INCLUDED_VCL_SCOPEDSTATES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
