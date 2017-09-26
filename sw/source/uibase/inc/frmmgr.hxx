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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_FRMMGR_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_FRMMGR_HXX

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

class   SwFormatCol;

const SwTwips   DFLT_WIDTH      = MM50 * 4;
const SwTwips   DFLT_HEIGHT     = MM50;

enum class Frmmgr_Type
{
    NONE    = 0x00,
    TEXT    = 0x01,
    GRF     = 0x02,
    OLE     = 0x04,
    ENVELP  = 0x10
};

class SW_DLLPUBLIC SwFlyFrameAttrMgr
{
    SfxItemSet  m_aSet;
    Point       m_aAbsPos;
    SwWrtShell* m_pOwnSh;

    bool    m_bAbsPos,
                m_bNewFrame;
    bool    m_bIsInVertical;
    // #mongolianlayout#
    bool    m_bIsInVerticalL2R;

    // internal calculation for borders
    SAL_DLLPRIVATE SwTwips           CalcTopSpace();
    SAL_DLLPRIVATE SwTwips           CalcBottomSpace();
    SAL_DLLPRIVATE SwTwips           CalcLeftSpace();
    SAL_DLLPRIVATE SwTwips           CalcRightSpace();

    SAL_DLLPRIVATE void UpdateFlyFrame_(); // post-treatment after insert or update

public:
    SwFlyFrameAttrMgr( bool bNew, SwWrtShell* pSh, Frmmgr_Type nType );

    //CopyCtor for dialogs to check the metrics
    SwFlyFrameAttrMgr( bool bNew, SwWrtShell *pSh, const SfxItemSet &rSet );

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

    void                SetHeightSizeType(SwFrameSize eType);

    // rotation
    void                SetRotation(sal_uInt16 nOld, sal_uInt16 nNew, const Size& rUnrotatedSize);

    // space to content
    void                SetLRSpace( long nLeft,
                                    long nRight );
    void                SetULSpace( long nTop,
                                    long nBottom );

    void                SetCol( const SwFormatCol &rCol);

    // change and query attributes
    void                UpdateAttrMgr();
    void                UpdateFlyFrame();

    // create new frame
    void                InsertFlyFrame();
    void                InsertFlyFrame(RndStdIds      eAnchorType,
                                   const Point    &rPos,
                                   const Size     &rSize);

    // check and change metrics
    void                ValidateMetrics(SvxSwFrameValidation& rVal,
                            const SwPosition* pToCharContentPos,
                            bool bOnlyPercentRefValue = false);

    void                DelAttr(sal_uInt16 nId);

    // reach out the set
    const SfxItemSet &GetAttrSet() const { return m_aSet; }
    SfxItemSet &GetAttrSet()       { return m_aSet; }
    void                     SetAttrSet(const SfxItemSet& rSet);

    inline const SwFormatVertOrient &GetVertOrient() const;
    inline const SwFormatHoriOrient &GetHoriOrient() const;
    inline const SvxShadowItem   &GetShadow() const;
    inline const SvxBoxItem      &GetBox() const;
    inline const SwFormatFrameSize    &GetFrameSize() const;

    long CalcWidthBorder()  { return CalcLeftSpace()+CalcRightSpace(); }
    long CalcHeightBorder() { return CalcTopSpace()+CalcBottomSpace(); }
};

inline const Size& SwFlyFrameAttrMgr::GetSize() const
{
    return static_cast<const SwFormatFrameSize&>(m_aSet.Get(RES_FRM_SIZE)).GetSize();
}

inline const SwFormatVertOrient &SwFlyFrameAttrMgr::GetVertOrient() const
{
    return static_cast<const SwFormatVertOrient&>(m_aSet.Get(RES_VERT_ORIENT));
}
inline const SwFormatHoriOrient &SwFlyFrameAttrMgr::GetHoriOrient() const
{
    return static_cast<const SwFormatHoriOrient &>(m_aSet.Get(RES_HORI_ORIENT));
}
inline const SwFormatFrameSize& SwFlyFrameAttrMgr::GetFrameSize() const
{
    return static_cast<const SwFormatFrameSize&>(m_aSet.Get(RES_FRM_SIZE));
}
inline const SvxShadowItem &SwFlyFrameAttrMgr::GetShadow() const
{
    return static_cast<const SvxShadowItem&>(m_aSet.Get(RES_SHADOW));
}
inline const SvxBoxItem &SwFlyFrameAttrMgr::GetBox() const
{
    return static_cast<const SvxBoxItem&>(m_aSet.Get(RES_BOX));
}
inline Point SwFlyFrameAttrMgr::GetPos() const
{
    return Point( GetHoriOrient().GetPos(), GetVertOrient().GetPos() );
}
inline RndStdIds SwFlyFrameAttrMgr::GetAnchor()  const
{
    return static_cast<const SwFormatAnchor&>(m_aSet.Get(RES_ANCHOR)).GetAnchorId();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
