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

#ifndef INCLUDED_VCL_FRAMEPOSSIZE_HXX
#define INCLUDED_VCL_FRAMEPOSSIZE_HXX

#include <vcl/dllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <rtl/string.hxx>
#include <sal/types.h>
#include <tools/gen.hxx>

namespace vcl
{
/**
 * There are multiple ways to store the two different areas of a vcl::Window.
 * But this representation is hopefully less error prone from the used types
 * and more clear in what values in- or exclude the non-drawable window frame.
 *
 * There are especially two things to remember:
 *  * pos() is the top-left position of the window frame
 *  * size() returns just the drawable client area
 *
 * So these values actually don't represent any "real" geometry of either the
 * outer frame or the inner client area of the window. That's my reason for
 * naming the rectangle function posSize() instead of geometry(). Also to not
 * be confused with Qt's geometry() function. YMMV.
 *
 * LO already is supposed to use this schema. FWIW, the Qt documentation claims
 * "The differentiation is done in a way that covers the most common usage
 * transparently." AFAIK this is common for most/all platforms / UI toolkits.
 *
 * The API is kept largely overload free, as we can now use list-initialization.
 */
class VCL_PLUGIN_PUBLIC WindowPosSize
{
    // position of the window frames left-top corner
    sal_Int32 m_nX;
    sal_Int32 m_nY;
    // size of the client / drawable area, i.e. without decorations / borders
    sal_Int32 m_nWidth;
    sal_Int32 m_nHeight;

protected:
    WindowPosSize()
        : m_nX(0)
        , m_nY(0)
        , m_nWidth(1)
        , m_nHeight(1)
    {
    }

public:
    constexpr sal_Int32 x() const { return m_nX; }
    void setX(sal_Int32 nX) { m_nX = nX; }
    constexpr sal_Int32 y() const { return m_nY; }
    void setY(sal_Int32 nY) { m_nY = nY; }

    constexpr Point pos() const { return { m_nX, m_nY }; }
    void setPos(const Point& aPos)
    {
        setX(aPos.getX());
        setY(aPos.getY());
    }
    void move(sal_Int32 nDX, sal_Int32 nDY)
    {
        m_nX += nDX;
        m_nY += nDY;
    }

    constexpr sal_Int32 width() const { return m_nWidth; }
    void setWidth(sal_Int32 nWidth)
    {
        assert(nWidth >= 0);
        if (nWidth >= 0)
            m_nWidth = nWidth;
        else
            m_nWidth = 0;
    }

    constexpr sal_Int32 height() const { return m_nHeight; }
    void setHeight(sal_Int32 nHeight)
    {
        assert(nHeight >= 0);
        if (nHeight >= 0)
            m_nHeight = nHeight;
        else
            m_nHeight = 0;
    }

    constexpr Size size() const
    {
        return { static_cast<tools::Long>(m_nWidth), static_cast<tools::Long>(m_nHeight) };
    }
    void setSize(const Size& rSize)
    {
        setWidth(rSize.Width());
        setHeight(rSize.Height());
    }

    constexpr tools::Rectangle posSize() const { return { pos(), size() }; }
    void setPosSize(const tools::Rectangle& rRect)
    {
        setPos(rRect.GetPos());
        setSize(rRect.GetSize());
    }
    // because tools::Rectangle has the ambiguous (Point&, Point&) constructor, which we don't want here
    void setPosSize(const Point& rPos, const Size& rSize) { setPosSize({ rPos, rSize }); }
};

inline std::ostream& operator<<(std::ostream& s, const WindowPosSize& rPosSize)
{
    s << rPosSize.width() << "x" << rPosSize.height() << "@(" << rPosSize.x() << "," << rPosSize.y()
      << ")";
    return s;
}

} // namespace vcl

#endif // INCLUDED_VCL_FRAMEPOSSIZE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
