/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_WIN_SCOPED_GDI_HXX
#define INCLUDED_VCL_INC_WIN_SCOPED_GDI_HXX

#include <win/svsys.h>
#include <win/wincomp.hxx>
#include <win/saldata.hxx>

#include <memory>

template <typename H, auto DeleterFunc> struct GDIDeleter
{
    using pointer = H;
    void operator()(H h) { DeleterFunc(h); }
};

template <typename H, auto DeleterFunc>
using ScopedGDI = std::unique_ptr<H, GDIDeleter<H, DeleterFunc>>;

using ScopedHBRUSH = ScopedGDI<HBRUSH, DeleteBrush>;
using ScopedHRGN = ScopedGDI<HRGN, DeleteRegion>;
using ScopedHDC = ScopedGDI<HDC, DeleteDC>;
using ScopedHPEN = ScopedGDI<HPEN, DeletePen>;
using ScopedHFONT = ScopedGDI<HFONT, DeleteFont>;
using ScopedHBITMAP = ScopedGDI<HBITMAP, DeleteBitmap>;

template <typename ScopedH, auto SelectorFunc> class ScopedSelectedGDI
{
public:
    ScopedSelectedGDI(HDC hDC, typename ScopedH::pointer h)
        : m_hDC(hDC)
        , m_hSelectedH(h)
        , m_hOrigH(SelectorFunc(hDC, h))
    {
    }

    ~ScopedSelectedGDI() { SelectorFunc(m_hDC, m_hOrigH); }

private:
    HDC m_hDC;
    ScopedH m_hSelectedH;
    typename ScopedH::pointer m_hOrigH;
};

using ScopedSelectedHPEN = ScopedSelectedGDI<ScopedHPEN, SelectPen>;
using ScopedSelectedHFONT = ScopedSelectedGDI<ScopedHFONT, SelectFont>;
using ScopedSelectedHBRUSH = ScopedSelectedGDI<ScopedHBRUSH, SelectBrush>;

template <sal_uLong ID> class ScopedCachedHDC
{
public:
    explicit ScopedCachedHDC(HBITMAP hBitmap)
        : m_hDC(ImplGetCachedDC(ID, hBitmap))
    {
    }

    ~ScopedCachedHDC() { ImplReleaseCachedDC(ID); }

    HDC get() const { return m_hDC; }

private:
    HDC m_hDC;
};

#endif // INCLUDED_VCL_INC_WIN_SCOPED_GDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
