/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_WIN_SCOPED_SELECTED_GDI_HXX
#define INCLUDED_VCL_INC_WIN_SCOPED_SELECTED_GDI_HXX

#include <win/scoped_gdi.hxx>

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

#endif // INCLUDED_VCL_INC_WIN_SCOPED_SELECTED_GDI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
