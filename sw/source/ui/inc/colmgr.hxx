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

SW_DLLPUBLIC void FitToActualSize(SwFmtCol& rCol, sal_uInt16 nWidth);

class SW_DLLPUBLIC SwColMgr
{
public:
        // lActWidth wird aus den Edits des Seitendialogs
        // direkt uebergeben
    SwColMgr(const SfxItemSet &rSet, sal_uInt16 nActWidth = USHRT_MAX);
    ~SwColMgr();


    inline sal_uInt16       GetCount() const;
    void                SetCount(sal_uInt16 nCount, sal_uInt16 nGutterWidth);
    sal_uInt16              GetGutterWidth(sal_uInt16 nPos = USHRT_MAX) const;
    void                SetGutterWidth(sal_uInt16 nWidth, sal_uInt16 nPos = USHRT_MAX);

    sal_uInt16              GetColWidth(sal_uInt16 nIdx) const;
    void                SetColWidth(sal_uInt16 nIdx, sal_uInt16 nWidth);

    inline sal_Bool         IsAutoWidth() const;
    void                SetAutoWidth(sal_Bool bOn = sal_True, sal_uInt16 lGutterWidth = 0);

    inline sal_Bool         HasLine() const;
    inline void         SetNoLine();

    void                SetLineWidthAndColor(SvxBorderStyle eStyle, sal_uLong nWidth, const Color& rCol);
    inline SvxBorderStyle   GetLineStyle() const;
    inline sal_uLong        GetLineWidth() const;
    inline const Color& GetLineColor() const;

    inline SwColLineAdj GetAdjust() const;
    inline void         SetAdjust(SwColLineAdj);

    short               GetLineHeightPercent() const;
    void                SetLineHeightPercent(short nPercent);

    inline void         NoCols();
    void                Update();

    const SwFmtCol&     GetColumns() const { return aFmtCol; }

    void                SetActualWidth(sal_uInt16 nW);
    sal_uInt16              GetActualSize() const { return nWidth; }


private:

    SwFmtCol            aFmtCol;
    sal_uInt16              nWidth;
};

// INLINE METHODE --------------------------------------------------------
inline  sal_uInt16 SwColMgr::GetCount() const
{
    return aFmtCol.GetNumCols();
}

inline SvxBorderStyle SwColMgr::GetLineStyle() const
{
    return aFmtCol.GetLineStyle();
}
inline sal_uLong        SwColMgr::GetLineWidth() const
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
inline sal_Bool SwColMgr::IsAutoWidth() const
{
    return aFmtCol.IsOrtho();
}
inline void SwColMgr::SetAutoWidth(sal_Bool bOn, sal_uInt16 nGutterWidth)
{
    aFmtCol.SetOrtho(bOn, nGutterWidth, nWidth);
}

inline void SwColMgr::NoCols()
{
    aFmtCol.GetColumns().DeleteAndDestroy(0, aFmtCol.GetColumns().Count());
}
inline sal_Bool SwColMgr::HasLine() const
{
    return GetAdjust() != COLADJ_NONE;
}

inline void SwColMgr::SetNoLine()
{
    SetAdjust(COLADJ_NONE);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
