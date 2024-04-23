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

#include <swtypes.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <tools/gen.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtanchr.hxx>
#include <swdllapi.h>

class   SwWrtShell;
struct  SvxSwFrameValidation;
struct  SwPosition;

class   SwFormatCol;
class SvGlobalName;

constexpr SwTwips DFLT_WIDTH = o3tl::toTwips(20, o3tl::Length::mm);
constexpr SwTwips DFLT_HEIGHT = o3tl::toTwips(5, o3tl::Length::mm);

enum class Frmmgr_Type
{
    NONE    = 0x00,
    TEXT    = 0x01,
    GRF     = 0x02,
    OLE     = 0x04,
    ENVELP  = 0x10
};

class SwFlyFrameAttrMgr
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
    SwTwips           CalcTopSpace();
    SwTwips           CalcBottomSpace();
    SwTwips           CalcLeftSpace();
    SwTwips           CalcRightSpace();

    void UpdateFlyFrame_(); // post-treatment after insert or update

public:
    SW_DLLPUBLIC SwFlyFrameAttrMgr( bool bNew, SwWrtShell* pSh, Frmmgr_Type nType, const SvGlobalName* pName );

    //CopyCtor for dialogs to check the metrics
    SW_DLLPUBLIC SwFlyFrameAttrMgr( bool bNew, SwWrtShell *pSh, SfxItemSet aSet );

    SW_DLLPUBLIC void SetAnchor(RndStdIds eId);
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
    void                SetRotation(Degree10 nOld, Degree10 nNew, const Size& rUnrotatedSize);

    // space to content
    void                SetLRSpace( tools::Long nLeft,
                                    tools::Long nRight );
    void                SetULSpace( tools::Long nTop,
                                    tools::Long nBottom );

    void                SetCol( const SwFormatCol &rCol);

    // change and query attributes
    void                UpdateAttrMgr();
    void                UpdateFlyFrame();

    // create new frame
    void                InsertFlyFrame();
    SW_DLLPUBLIC void   InsertFlyFrame(RndStdIds      eAnchorType,
                                   const Point    &rPos,
                                   const Size     &rSize);

    // check and change metrics
    SW_DLLPUBLIC void   ValidateMetrics(SvxSwFrameValidation& rVal,
                            const SwFormatAnchor* pToCharContentPos,
                            bool bOnlyPercentRefValue = false);

    void                DelAttr(sal_uInt16 nId);

    // reach out the set
    const SfxItemSet &GetAttrSet() const { return m_aSet; }
    SfxItemSet &GetAttrSet()       { return m_aSet; }
    void                     SetAttrSet(const SfxItemSet& rSet);
    void SetFrameSizeFromTable();
    /// Checks if we have a table selection and that covers exactly 1 full table.
    SW_DLLPUBLIC static const SwTableFormat* SingleTableSelected(SwWrtShell& rWrtShell);

    inline const SwFormatVertOrient &GetVertOrient() const;
    inline const SwFormatHoriOrient &GetHoriOrient() const;
    inline const SvxShadowItem   &GetShadow() const;
    inline const SvxBoxItem      &GetBox() const;
    inline const SwFormatFrameSize    &GetFrameSize() const;

    tools::Long CalcWidthBorder()  { return CalcLeftSpace()+CalcRightSpace(); }
    tools::Long CalcHeightBorder() { return CalcTopSpace()+CalcBottomSpace(); }
};

inline const Size& SwFlyFrameAttrMgr::GetSize() const
{
    return m_aSet.Get(RES_FRM_SIZE).GetSize();
}

inline const SwFormatVertOrient &SwFlyFrameAttrMgr::GetVertOrient() const
{
    return m_aSet.Get(RES_VERT_ORIENT);
}
inline const SwFormatHoriOrient &SwFlyFrameAttrMgr::GetHoriOrient() const
{
    return m_aSet.Get(RES_HORI_ORIENT);
}
inline const SwFormatFrameSize& SwFlyFrameAttrMgr::GetFrameSize() const
{
    return m_aSet.Get(RES_FRM_SIZE);
}
inline const SvxShadowItem &SwFlyFrameAttrMgr::GetShadow() const
{
    return m_aSet.Get(RES_SHADOW);
}
inline const SvxBoxItem &SwFlyFrameAttrMgr::GetBox() const
{
    return m_aSet.Get(RES_BOX);
}
inline Point SwFlyFrameAttrMgr::GetPos() const
{
    return Point( GetHoriOrient().GetPos(), GetVertOrient().GetPos() );
}
inline RndStdIds SwFlyFrameAttrMgr::GetAnchor()  const
{
    return m_aSet.Get(RES_ANCHOR).GetAnchorId();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
