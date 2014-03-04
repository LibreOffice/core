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
#ifndef INCLUDED_SW_SOURCE_UI_INC_FRMMGR_HXX
#define INCLUDED_SW_SOURCE_UI_INC_FRMMGR_HXX

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
    SfxItemSet  m_aSet;
    Point       m_aAbsPos;
    SwWrtShell* m_pOwnSh;

    sal_Bool    m_bAbsPos,
                m_bNewFrm;
    sal_Bool    m_bIsInVertical;
    // --> OD 2009-09-01 #mongolianlayout#
    sal_Bool    m_bIsInVerticalL2R;

    // internal calculation for borders
    SAL_DLLPRIVATE SwTwips           CalcTopSpace();
    SAL_DLLPRIVATE SwTwips           CalcBottomSpace();
    SAL_DLLPRIVATE SwTwips           CalcLeftSpace();
    SAL_DLLPRIVATE SwTwips           CalcRightSpace();

    SAL_DLLPRIVATE void _UpdateFlyFrm(); // post-treatment after insert or update

public:
    SwFlyFrmAttrMgr( sal_Bool bNew, SwWrtShell* pSh, sal_uInt8 nType );

    //CopyCtor for dialogs to check the metrics
    SwFlyFrmAttrMgr( sal_Bool bNew, SwWrtShell *pSh, const SfxItemSet &rSet );

    inline SwWrtShell*  GetShell() { return m_pOwnSh; }

    void                SetAnchor(RndStdIds eId);
    inline RndStdIds    GetAnchor()  const;

    void                SetHorzOrientation(sal_Int16 eOrient);
    void                SetVertOrientation(sal_Int16 eOrient);

    // absolute position
    void                SetAbsPos(const Point& rLPoint);

    // anchor's relative position
    void                SetPos(const Point& rLPoint);
    inline Point        GetPos() const;

    // size
    void                SetSize(const Size& rLSize);
    inline const Size&  GetSize() const;

    inline sal_uInt16       GetHeightPercent() const;

    void                SetHeightSizeType(SwFrmSize eType);

    // space to content
    void                SetLRSpace( long nLeft  = LONG_MAX,
                                    long nRight = LONG_MAX );
    void                SetULSpace( long nTop   = LONG_MAX,
                                    long nBottom= LONG_MAX );

    void                SetCol( const SwFmtCol &rCol);

    // change and query attributes
    void                UpdateAttrMgr();
    void                UpdateFlyFrm();

    // create new frame
    sal_Bool                InsertFlyFrm();
    void                InsertFlyFrm(RndStdIds      eAnchorType,
                                   const Point    &rPos,
                                   const Size     &rSize,
                                   sal_Bool           bAbsPos = sal_False);

    // check and change metrics
    void                ValidateMetrics(SvxSwFrameValidation& rVal,
                            const SwPosition* pToCharCntntPos,
                            bool bOnlyPercentRefValue = false);

    void                DelAttr(sal_uInt16 nId);

    // reach out the set
    inline const SfxItemSet &GetAttrSet() const { return m_aSet; }
    inline       SfxItemSet &GetAttrSet()       { return m_aSet; }
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
    return ((SwFmtFrmSize&)m_aSet.Get(RES_FRM_SIZE)).GetSize();
}

inline const SwFmtVertOrient &SwFlyFrmAttrMgr::GetVertOrient() const
{
    return ((SwFmtVertOrient&)m_aSet.Get(RES_VERT_ORIENT));
}
inline const SwFmtHoriOrient &SwFlyFrmAttrMgr::GetHoriOrient() const
{
    return ((SwFmtHoriOrient &)m_aSet.Get(RES_HORI_ORIENT));
}
inline const SwFmtFrmSize& SwFlyFrmAttrMgr::GetFrmSize() const
{
    return ((SwFmtFrmSize&)m_aSet.Get(RES_FRM_SIZE));
}
inline const SvxShadowItem &SwFlyFrmAttrMgr::GetShadow() const
{
    return ((SvxShadowItem&)m_aSet.Get(RES_SHADOW));
}
inline const SvxBoxItem &SwFlyFrmAttrMgr::GetBox() const
{
    return ((SvxBoxItem&)m_aSet.Get(RES_BOX));
}
inline const SwFmtSurround &SwFlyFrmAttrMgr::GetSurround() const
{
    return ((SwFmtSurround&)m_aSet.Get(RES_SURROUND));
}

inline Point SwFlyFrmAttrMgr::GetPos() const
{
    return Point( GetHoriOrient().GetPos(), GetVertOrient().GetPos() );
}

inline RndStdIds SwFlyFrmAttrMgr::GetAnchor()  const
{
    return ((SwFmtAnchor&)m_aSet.Get(RES_ANCHOR)).GetAnchorId();
}

inline sal_uInt16 SwFlyFrmAttrMgr::GetHeightPercent() const
{
    return GetFrmSize().GetHeightPercent();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
