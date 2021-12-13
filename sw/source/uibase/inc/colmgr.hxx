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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_COLMGR_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_COLMGR_HXX

#include <swdllapi.h>
#include <fmtclds.hxx>

#include <climits>

SW_DLLPUBLIC void FitToActualSize(SwFormatCol& rCol, sal_uInt16 nWidth);

class SW_DLLPUBLIC SwColMgr
{
public:
        // lActWidth is passed directly from the page dialogs edits
    SwColMgr(const SfxItemSet &rSet);
    ~SwColMgr();

    inline sal_uInt16   GetCount() const;
    void                SetCount(sal_uInt16 nCount, sal_uInt16 nGutterWidth);
    sal_uInt16          GetGutterWidth(sal_uInt16 nPos = USHRT_MAX) const;
    void                SetGutterWidth(sal_uInt16 nWidth, sal_uInt16 nPos = USHRT_MAX);

    sal_uInt16          GetColWidth(sal_uInt16 nIdx) const;
    void                SetColWidth(sal_uInt16 nIdx, sal_uInt16 nWidth);

    inline bool         IsAutoWidth() const;
    void                SetAutoWidth(bool bOn, sal_uInt16 lGutterWidth = 0);

    inline bool         HasLine() const;
    inline void         SetNoLine();

    void                SetLineWidthAndColor(SvxBorderLineStyle eStyle, sal_uLong nWidth, const Color& rCol);
    inline SvxBorderLineStyle    GetLineStyle() const;
    inline sal_uLong    GetLineWidth() const;
    inline const Color& GetLineColor() const;

    inline SwColLineAdj GetAdjust() const;
    inline void         SetAdjust(SwColLineAdj);

    short               GetLineHeightPercent() const;
    void                SetLineHeightPercent(short nPercent);

    inline void         NoCols();

    const SwFormatCol&  GetColumns() const { return m_aFormatCol; }

    void                SetActualWidth(sal_uInt16 nW);
    sal_uInt16          GetActualSize() const { return m_nWidth; }

private:
    SwFormatCol             m_aFormatCol;
    sal_uInt16              m_nWidth;
};

inline  sal_uInt16 SwColMgr::GetCount() const
{
    return m_aFormatCol.GetNumCols();
}

inline SvxBorderLineStyle SwColMgr::GetLineStyle() const
{
    return m_aFormatCol.GetLineStyle();
}
inline sal_uLong        SwColMgr::GetLineWidth() const
{
    return m_aFormatCol.GetLineWidth();
}

inline const Color& SwColMgr::GetLineColor() const
{
    return m_aFormatCol.GetLineColor();
}

inline  SwColLineAdj SwColMgr::GetAdjust() const
{
    return m_aFormatCol.GetLineAdj();
}

inline  void SwColMgr::SetAdjust(SwColLineAdj eAdj)
{
    m_aFormatCol.SetLineAdj(eAdj);
}
inline bool SwColMgr::IsAutoWidth() const
{
    return m_aFormatCol.IsOrtho();
}
inline void SwColMgr::SetAutoWidth(bool bOn, sal_uInt16 nGutterWidth)
{
    m_aFormatCol.SetOrtho(bOn, nGutterWidth, m_nWidth);
}

inline void SwColMgr::NoCols()
{
    m_aFormatCol.GetColumns().clear();
}
inline bool SwColMgr::HasLine() const
{
    return GetAdjust() != COLADJ_NONE;
}

inline void SwColMgr::SetNoLine()
{
    SetAdjust(COLADJ_NONE);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
