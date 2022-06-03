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

#ifndef INCLUDED_VCL_WINDOWSTATE_HXX
#define INCLUDED_VCL_WINDOWSTATE_HXX

#include <vcl/WindowPosSize.hxx>

namespace vcl
{
enum class WindowState
{
    NONE = 0x0000,
    Normal = 0x0001,
    Minimized = 0x0002,
    Maximized = 0x0004,
    // Rollup is no longer used, but the bit is retained because WindowData is serialized
    // from/to strings describing window state that are stored in a users config
    // Rollup = 0x0008,
    MaximizedHorz = 0x0010,
    MaximizedVert = 0x0020,
    FullScreen = 0x0040,
    SystemMask = 0xffff
};

enum class WindowDataMask
{
    NONE = 0x0000,
    X = 0x0001,
    Y = 0x0002,
    Width = 0x0004,
    Height = 0x0008,
    State = 0x0010,
    Minimized = 0x0020,
    Pos = X | Y,
    Size = Width | Height,
    PosSize = Pos | Size,
    PosSizeState = Pos | Size | State,
    All = X | Y | Width | Height | State | Minimized
};

class VCL_PLUGIN_PUBLIC WindowData final : public WindowPosSize
{
    WindowState m_nState;
    WindowDataMask m_nMask;

public:
    WindowData()
        : m_nState(WindowState::NONE)
        , m_nMask(WindowDataMask::NONE)
    {
    }
    WindowData(std::string_view rStr);

    // serialize values to a string (the original WindowState representation)
    OString toStr(bool bSave) const;

    void setState(WindowState nState) { m_nState = nState; }
    WindowState state() const { return m_nState; }
    WindowState& rState() { return m_nState; }

    void setMask(WindowDataMask nMask) { m_nMask = nMask; }
    WindowDataMask mask() const { return m_nMask; }
    WindowDataMask& rMask() { return m_nMask; }
};

} // namespace vcl

namespace o3tl
{
template <> struct typed_flags<vcl::WindowState> : is_typed_flags<vcl::WindowState, 0xffff>
{
};
template <> struct typed_flags<vcl::WindowDataMask> : is_typed_flags<vcl::WindowDataMask, 0x0f3f>
{
};
}

namespace vcl
{
inline std::ostream& operator<<(std::ostream& s, const WindowData& rData)
{
    if (rData.mask() & WindowDataMask::Width)
        s << rData.width() << "x";
    else
        s << "?x";
    if (rData.mask() & WindowDataMask::Height)
        s << rData.height() << "@(";
    else
        s << "?@(";
    if (rData.mask() & WindowDataMask::X)
        s << rData.x() << ",";
    else
        s << "?,";
    if (rData.mask() & WindowDataMask::Y)
        s << rData.y() << ")^";
    else
        s << "?)^";
    if (rData.mask() & WindowDataMask::State)
        s << "0x" << std::hex << static_cast<unsigned>(rData.state()) << std::dec;
    else
        s << "?";
    if (rData.savedPosSizeRefs())
        s << "_saved:" << rData.rSavedPosSize() << "^" << rData.savedPosSizeRefs();
    return s;
}

} // namespace vcl

#endif // INCLUDED_VCL_WINDOWSTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
