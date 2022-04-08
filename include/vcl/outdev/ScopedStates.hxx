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
/** Change the antialiasing to another value for the life-time the object */
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

/** Change the MapMode to another value for the life-time the object */
template <typename T> class ScopedMapMode
{
private:
    T& m_rOutputDevice;
    MapMode m_aOldMapMode;
    MapMode m_aNewMapMode;

public:
    ScopedMapMode(T& rOutputDevice, MapMode const& aNewMapMode)
        : m_rOutputDevice(rOutputDevice)
        , m_aOldMapMode(m_rOutputDevice.GetMapMode())
        , m_aNewMapMode(aNewMapMode)
    {
        if (m_aOldMapMode != m_aNewMapMode)
            m_rOutputDevice.SetMapMode(m_aNewMapMode);
    }

    ~ScopedMapMode()
    {
        if (m_aOldMapMode != m_aNewMapMode)
            m_rOutputDevice.SetMapMode(m_aOldMapMode);
    }
};
}

#endif // INCLUDED_VCL_SCOPEDSTATES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
