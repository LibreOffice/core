/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _COLMGR_HXX
#define _COLMGR_HXX

#include "swdllapi.h"
#include <fmtclds.hxx>

SW_DLLPUBLIC void FitToActualSize(SwFmtCol& rCol, USHORT nWidth);

class SW_DLLPUBLIC SwColMgr
{
public:
        // lActWidth wird aus den Edits des Seitendialogs
        // direkt uebergeben
    SwColMgr(const SfxItemSet &rSet, USHORT nActWidth = USHRT_MAX);
    ~SwColMgr();


    inline USHORT       GetCount() const;
    void                SetCount(USHORT nCount, USHORT nGutterWidth);
    USHORT              GetGutterWidth(USHORT nPos = USHRT_MAX) const;
    void                SetGutterWidth(USHORT nWidth, USHORT nPos = USHRT_MAX);

    USHORT              GetColWidth(USHORT nIdx) const;
    void                SetColWidth(USHORT nIdx, USHORT nWidth);

    inline BOOL         IsAutoWidth() const;
    void                SetAutoWidth(BOOL bOn = TRUE, USHORT lGutterWidth = 0);

    inline BOOL         HasLine() const;
    inline void         SetNoLine();

    inline void         SetLineWidthAndColor(ULONG nWidth, const Color& rCol);
    inline ULONG        GetLineWidth() const;
    inline const Color& GetLineColor() const;

    inline SwColLineAdj GetAdjust() const;
    inline void         SetAdjust(SwColLineAdj);

    short               GetLineHeightPercent() const;
    void                SetLineHeightPercent(short nPercent);

    inline void         NoCols();
    void                Update();

    const SwFmtCol&     GetColumns() const { return aFmtCol; }

    void                SetActualWidth(USHORT nW);
    USHORT              GetActualSize() const { return nWidth; }


private:

    SwFmtCol            aFmtCol;
    USHORT              nWidth;
};

// INLINE METHODE --------------------------------------------------------
inline  USHORT SwColMgr::GetCount() const
{
    return aFmtCol.GetNumCols();
}

inline void         SwColMgr::SetLineWidthAndColor(ULONG nLWidth, const Color& rCol)
{
    aFmtCol.SetLineWidth(nLWidth);
    aFmtCol.SetLineColor(rCol);
}

inline ULONG        SwColMgr::GetLineWidth() const
{
    return aFmtCol.GetLineWidth();
}

inline const Color& SwColMgr::GetLineColor() const
{
    return aFmtCol.GetLineColor();
}

inline  SwColLineAdj SwColMgr::GetAdjust() const
{
    return aFmtCol.GetLineAdj();
}

inline  void SwColMgr::SetAdjust(SwColLineAdj eAdj)
{
    aFmtCol.SetLineAdj(eAdj);
}

inline BOOL SwColMgr::IsAutoWidth() const
{
    return aFmtCol.IsOrtho();
}

inline void SwColMgr::SetAutoWidth(BOOL bOn, USHORT nGutterWidth)
{
    aFmtCol.SetOrtho(bOn, nGutterWidth, nWidth);
}

inline void SwColMgr::NoCols()
{
    aFmtCol.GetColumns().DeleteAndDestroy(0, aFmtCol.GetColumns().Count());
}

inline BOOL SwColMgr::HasLine() const
{
    return GetAdjust() != COLADJ_NONE;
}

inline void SwColMgr::SetNoLine()
{
    SetAdjust(COLADJ_NONE);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
