/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

    inline void         SetLineWidthAndColor(sal_uLong nWidth, const Color& rCol);
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
inline void         SwColMgr::SetLineWidthAndColor(sal_uLong nLWidth, const Color& rCol)
{
    aFmtCol.SetLineWidth(nLWidth);
    aFmtCol.SetLineColor(rCol);
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
