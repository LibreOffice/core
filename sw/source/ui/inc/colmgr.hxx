/*************************************************************************
 *
 *  $RCSfile: colmgr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _COLMGR_HXX
#define _COLMGR_HXX

#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif

void FitToActualSize(SwFmtCol& rCol, USHORT nWidth);

class SwColMgr
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
inline void         SwColMgr::SetLineWidthAndColor(ULONG nWidth, const Color& rCol)
{
    aFmtCol.SetLineWidth(nWidth);
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
