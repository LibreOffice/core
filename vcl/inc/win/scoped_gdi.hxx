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

#include <memory>

struct HBRUSHDeleter
{
    using pointer = HBRUSH;
    void operator()(HBRUSH hBrush) { DeleteBrush(hBrush); }
};

struct HRGNDeleter
{
    using pointer = HRGN;
    void operator()(HRGN hRgn) { DeleteRegion(hRgn); }
};

struct HDCDeleter
{
    using pointer = HDC;
    void operator()(HDC hDC) { DeleteDC(hDC); }
};

struct HPENDeleter
{
    using pointer = HPEN;
    void operator()(HPEN hPen) { DeletePen(hPen); }
};

struct HFONTDeleter
{
    using pointer = HFONT;
    void operator()(HFONT hFont) { DeleteFont(hFont); }
};

struct HBITMAPDeleter
{
    using pointer = HBITMAP;
    void operator()(HBITMAP hBitmap) { DeleteBitmap(hBitmap); }
};

using ScopedHBRUSH = std::unique_ptr<HBRUSH, HBRUSHDeleter>;
using ScopedHRGN = std::unique_ptr<HRGN, HRGNDeleter>;
using ScopedHDC = std::unique_ptr<HDC, HDCDeleter>;
using ScopedHPEN = std::unique_ptr<HPEN, HPENDeleter>;
using ScopedHFONT = std::unique_ptr<HFONT, HFONTDeleter>;
using ScopedHBITMAP = std::unique_ptr<HBITMAP, HBITMAPDeleter>;

class ScopedSelectedHPEN
{
public:
    ScopedSelectedHPEN(HDC hDC, HPEN hPen)
        : m_hDC(hDC)
        , m_hOrigPen(SelectPen(hDC, hPen))
        , m_hSelectedPen(hPen)
    {
    }

    ~ScopedSelectedHPEN() { SelectPen(m_hDC, m_hOrigPen); }

private:
    HDC m_hDC;
    HPEN m_hOrigPen;
    ScopedHPEN m_hSelectedPen;
};

class ScopedSelectedHFONT
{
public:
    ScopedSelectedHFONT(HDC hDC, HFONT hFont)
        : m_hDC(hDC)
        , m_hOrigFont(SelectFont(hDC, hFont))
        , m_hSelectedFont(hFont)
    {
    }

    ~ScopedSelectedHFONT() { SelectFont(m_hDC, m_hOrigFont); }

private:
    HDC m_hDC;
    HFONT m_hOrigFont;
    ScopedHFONT m_hSelectedFont;
};

class ScopedSelectedHBRUSH
{
public:
    ScopedSelectedHBRUSH(HDC hDC, HBRUSH hBrush)
        : m_hDC(hDC)
        , m_hOrigBrush(SelectBrush(hDC, hBrush))
        , m_hSelectedBrush(hBrush)
    {
    }

    ~ScopedSelectedHBRUSH() { SelectBrush(m_hDC, m_hOrigBrush); }

private:
    HDC m_hDC;
    HBRUSH m_hOrigBrush;
    ScopedHBRUSH m_hSelectedBrush;
};

#endif // INCLUDED_VCL_INC_WIN_SCOPED_GDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
