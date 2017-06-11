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
#ifndef INCLUDED_CUI_SOURCE_INC_SWPOSSIZETABPAGE_HXX
#define INCLUDED_CUI_SOURCE_INC_SWPOSSIZETABPAGE_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <svx/swframeposstrings.hxx>
#include <svx/swframeexample.hxx>

// SvxSwPosSizeTabPage - position and size page for Writer drawing objects
struct FrmMap;
class SdrView;
struct SvxSwFrameValidation;
enum class SvxAnchorIds;

class SvxSwPosSizeTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;

    VclPtr<MetricField>  m_pWidthMF;
    VclPtr<MetricField>  m_pHeightMF;
    VclPtr<CheckBox>     m_pKeepRatioCB;

    VclPtr<RadioButton>  m_pToPageRB;
    VclPtr<RadioButton>  m_pToParaRB;
    VclPtr<RadioButton>  m_pToCharRB;
    VclPtr<RadioButton>  m_pAsCharRB;
    VclPtr<RadioButton>  m_pToFrameRB;

    VclPtr<TriStateBox>  m_pPositionCB;
    VclPtr<TriStateBox>  m_pSizeCB;

    VclPtr<VclContainer> m_pPosFrame;
    VclPtr<FixedText>    m_pHoriFT;
    VclPtr<ListBox>      m_pHoriLB;
    VclPtr<FixedText>    m_pHoriByFT;
    VclPtr<MetricField>  m_pHoriByMF;
    VclPtr<FixedText>    m_pHoriToFT;
    VclPtr<ListBox>      m_pHoriToLB;

    VclPtr<CheckBox>     m_pHoriMirrorCB;

    VclPtr<FixedText>    m_pVertFT;
    VclPtr<ListBox>      m_pVertLB;
    VclPtr<FixedText>    m_pVertByFT;
    VclPtr<MetricField>  m_pVertByMF;
    VclPtr<FixedText>    m_pVertToFT;
    VclPtr<ListBox>      m_pVertToLB;

    VclPtr<CheckBox>     m_pFollowCB;

    VclPtr<SvxSwFrameExample> m_pExampleWN;

    Link<SvxSwFrameValidation&,void> m_aValidateLink;

    ::tools::Rectangle           m_aRect; //size of all selected objects
    ::tools::Rectangle           m_aWorkArea;
    Point               m_aAnchorPos;

    FrmMap* m_pVMap;
    FrmMap* m_pHMap;
    const SdrView*      m_pSdrView;

    // initial values
    short   m_nOldH;
    short   m_nOldHRel;
    short   m_nOldV;
    short   m_nOldVRel;

    double  m_fWidthHeightRatio; //width-to-height ratio to support the KeepRatio button
    bool    m_bHtmlMode;
    bool    m_bIsVerticalFrame;
    bool    m_bPositioningDisabled;
    bool    m_bIsMultiSelection;
    bool    m_bIsInRightToLeft;


    DECL_LINK(RangeModifyHdl, Control&, void);
    DECL_LINK(RangeModifyClickHdl, Button*, void);
    DECL_LINK(AnchorTypeHdl, Button*, void);
    DECL_LINK( PosHdl, ListBox&, void );
    DECL_LINK( RelHdl, ListBox&, void );
    DECL_LINK(MirrorHdl, Button*, void);
    DECL_LINK( ModifyHdl, Edit&, void );
    DECL_LINK(ProtectHdl, Button*, void);

    void            InitPos(RndStdIds nAnchorType, sal_uInt16 nH, sal_uInt16 nHRel,
                            sal_uInt16 nV,  sal_uInt16 nVRel,
                            long   nX,  long   nY);
    static sal_uInt16   GetMapPos(FrmMap *pMap, ListBox &rAlignLB);
    static short        GetAlignment(FrmMap *pMap, sal_uInt16 nMapPos, ListBox &rAlignLB, ListBox &rRelationLB);
    static short        GetRelation(FrmMap *pMap, ListBox &rRelationLB);
    RndStdIds           GetAnchorType(bool* pbHasChanged = nullptr);
    void                FillRelLB(FrmMap *pMap, sal_uInt16 nLBSelPos, sal_uInt16 nAlign, sal_uInt16 nRel, ListBox &rLB, FixedText &rFT);
    sal_uInt16          FillPosLB(FrmMap *pMap, sal_uInt16 nAlign, const sal_uInt16 _nRel, ListBox &rLB);

    void            UpdateExample();

    void setOptimalFrmWidth();
    void setOptimalRelWidth();

public:
    SvxSwPosSizeTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxSwPosSizeTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window*, const SfxItemSet* );
    static const sal_uInt16*     GetRanges();

    virtual bool FillItemSet( SfxItemSet* ) override;
    virtual void Reset( const SfxItemSet * ) override;

    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    EnableAnchorTypes(SvxAnchorIds nAnchorEnable);

    void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink )
            {m_aValidateLink = rLink;}

    void SetView( const SdrView* pSdrView );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
