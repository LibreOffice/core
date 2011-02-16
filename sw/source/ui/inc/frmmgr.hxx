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
#ifndef _FRMMGR_HXX
#define _FRMMGR_HXX

#include "swtypes.hxx"
#include "frmatr.hxx"
#include <editeng/svxenum.hxx>
#include <tools/gen.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <fmtornt.hxx>
#include <fmtanchr.hxx>
#include "swdllapi.h"

class   SwWrtShell;
struct  SvxSwFrameValidation;
struct  SwPosition;

class   SwFmt;
class   SwFmtCol;

const SwTwips   DFLT_WIDTH      = MM50 * 4;
const SwTwips   DFLT_HEIGHT     = MM50;

#define FULL_ATTRSET    0xffff

#define FRMMGR_TYPE_NONE    0x00
#define FRMMGR_TYPE_TEXT    0x01
#define FRMMGR_TYPE_GRF     0x02
#define FRMMGR_TYPE_OLE     0x04
#define FRMMGR_TYPE_LABEL   0x08
#define FRMMGR_TYPE_ENVELP  0x10

class SW_DLLPUBLIC SwFlyFrmAttrMgr
{
    SfxItemSet  aSet;
    Point       aAbsPos;
    SwWrtShell  *pOwnSh;

    sal_Bool        bAbsPos,
                bNewFrm;
    sal_Bool        bIsInVertical;
    // --> OD 2009-09-01 #mongolianlayout#
    sal_Bool        bIsInVerticalL2R;
    // <--

    // interne Verrechnung fuer Umrandung
    SW_DLLPRIVATE SwTwips           CalcTopSpace();
    SW_DLLPRIVATE SwTwips           CalcBottomSpace();
    SW_DLLPRIVATE SwTwips           CalcLeftSpace();
    SW_DLLPRIVATE SwTwips           CalcRightSpace();

    SW_DLLPRIVATE void _UpdateFlyFrm(); //Nacharbeit nach Einfuegen oder Update

public:
    SwFlyFrmAttrMgr( sal_Bool bNew, SwWrtShell* pSh, sal_uInt8 nType );

    //CopyCtor fuer die Dialoge, zum Pruefen der Metrics
    SwFlyFrmAttrMgr( sal_Bool bNew, SwWrtShell *pSh, const SfxItemSet &rSet );

    inline SwWrtShell*  GetShell() { return pOwnSh; }

    void                SetAnchor(RndStdIds eId);
    inline RndStdIds    GetAnchor()  const;

    void                SetHorzOrientation(sal_Int16 eOrient);
    void                SetVertOrientation(sal_Int16 eOrient);

    // Absolute Position
    void                SetAbsPos(const Point& rLPoint);

    // Relative Position vom Anker
    void                SetPos(const Point& rLPoint);
    inline Point        GetPos() const;

    // Groesse
    void                SetSize(const Size& rLSize);
    inline const Size&  GetSize() const;

    inline sal_uInt16       GetHeightPercent() const;

    void                SetHeightSizeType(SwFrmSize eType);

    // Abstand zum Inhalt
    void                SetLRSpace( long nLeft  = LONG_MAX,
                                    long nRight = LONG_MAX );
    void                SetULSpace( long nTop   = LONG_MAX,
                                    long nBottom= LONG_MAX );

    void                SetCol( const SwFmtCol &rCol);

    // Attribute aendern und erfragen
    void                UpdateAttrMgr();
    void                UpdateFlyFrm();

    // neuen Rahmen erzeugen
    sal_Bool                InsertFlyFrm();
    void                InsertFlyFrm(RndStdIds      eAnchorType,
                                   const Point    &rPos,
                                   const Size     &rSize,
                                   sal_Bool           bAbsPos = sal_False);

    // Metriken pruefen und  aendern
    void                ValidateMetrics(SvxSwFrameValidation& rVal,
                            const SwPosition* pToCharCntntPos,
                            sal_Bool bOnlyPercentRefValue = sal_False);

    void                DelAttr(sal_uInt16 nId);

    // Set rausreichen
    inline const SfxItemSet &GetAttrSet() const { return aSet; }
    inline       SfxItemSet &GetAttrSet()       { return aSet; }
    void                     SetAttrSet(const SfxItemSet& rSet);

    inline const SwFmtVertOrient &GetVertOrient() const;
    inline const SwFmtHoriOrient &GetHoriOrient() const;
    inline const SvxShadowItem   &GetShadow() const;
    inline const SvxBoxItem      &GetBox() const;
    inline const SwFmtSurround   &GetSurround() const;
    inline const SwFmtFrmSize    &GetFrmSize() const;

    long CalcWidthBorder()  { return CalcLeftSpace()+CalcRightSpace(); }
    long CalcHeightBorder() { return CalcTopSpace()+CalcBottomSpace(); }
};

inline const Size& SwFlyFrmAttrMgr::GetSize() const
{
    return ((SwFmtFrmSize&)aSet.Get(RES_FRM_SIZE)).GetSize();
}

inline const SwFmtVertOrient &SwFlyFrmAttrMgr::GetVertOrient() const
{
    return ((SwFmtVertOrient&)aSet.Get(RES_VERT_ORIENT));
}
inline const SwFmtHoriOrient &SwFlyFrmAttrMgr::GetHoriOrient() const
{
    return ((SwFmtHoriOrient &)aSet.Get(RES_HORI_ORIENT));
}
inline const SwFmtFrmSize& SwFlyFrmAttrMgr::GetFrmSize() const
{
    return ((SwFmtFrmSize&)aSet.Get(RES_FRM_SIZE));
}
inline const SvxShadowItem &SwFlyFrmAttrMgr::GetShadow() const
{
    return ((SvxShadowItem&)aSet.Get(RES_SHADOW));
}
inline const SvxBoxItem &SwFlyFrmAttrMgr::GetBox() const
{
    return ((SvxBoxItem&)aSet.Get(RES_BOX));
}
inline const SwFmtSurround &SwFlyFrmAttrMgr::GetSurround() const
{
    return ((SwFmtSurround&)aSet.Get(RES_SURROUND));
}

inline Point SwFlyFrmAttrMgr::GetPos() const
{
    return Point( GetHoriOrient().GetPos(), GetVertOrient().GetPos() );
}

inline RndStdIds SwFlyFrmAttrMgr::GetAnchor()  const
{
    return ((SwFmtAnchor&)aSet.Get(RES_ANCHOR)).GetAnchorId();
}

inline sal_uInt16 SwFlyFrmAttrMgr::GetHeightPercent() const
{
    return GetFrmSize().GetHeightPercent();
}

#endif
