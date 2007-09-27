/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colmgr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:55:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _COLMGR_HXX
#define _COLMGR_HXX

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif

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
