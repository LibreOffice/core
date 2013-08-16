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
#ifndef _SVX_SWPOSSIZETABPAGE_HXX
#define _SVX_SWPOSSIZETABPAGE_HXX

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
class SvxSwPosSizeTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;

    MetricField*  m_pWidthMF;
    MetricField*  m_pHeightMF;
    CheckBox*     m_pKeepRatioCB;

    RadioButton*  m_pToPageRB;
    RadioButton*  m_pToParaRB;
    RadioButton*  m_pToCharRB;
    RadioButton*  m_pAsCharRB;
    RadioButton*  m_pToFrameRB;

    TriStateBox*  m_pPositionCB;
    TriStateBox*  m_pSizeCB;

    VclContainer* m_pPosFrame;
    FixedText*    m_pHoriFT;
    ListBox*      m_pHoriLB;
    FixedText*    m_pHoriByFT;
    MetricField*  m_pHoriByMF;
    FixedText*    m_pHoriToFT;
    ListBox*      m_pHoriToLB;

    CheckBox*     m_pHoriMirrorCB;

    FixedText*    m_pVertFT;
    ListBox*      m_pVertLB;
    FixedText*    m_pVertByFT;
    MetricField*  m_pVertByMF;
    FixedText*    m_pVertToFT;
    ListBox*      m_pVertToLB;

    CheckBox*     m_pFollowCB;

    SvxSwFrameExample* m_pExampleWN;

    Link        m_aValidateLink;

    //'string provider'
    SvxSwFramePosString m_aFramePosString;

    Rectangle           m_aRect; //size of all selected objects
    Rectangle           m_aWorkArea;
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
    sal_uInt16  m_nHtmlMode;
    bool    m_bHtmlMode;
    bool    m_bAtHoriPosModified;
    bool    m_bAtVertPosModified;
    bool    m_bIsVerticalFrame;
    bool    m_bPositioningDisabled;
    bool    m_bIsMultiSelection;
    bool    m_bIsInRightToLeft;



    DECL_LINK(RangeModifyHdl, void *);
    DECL_LINK(AnchorTypeHdl, void *);
    DECL_LINK( PosHdl, ListBox * );
    DECL_LINK( RelHdl, ListBox * );
    DECL_LINK(MirrorHdl, void *);
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK(ProtectHdl, void *);

    void            InitPos(short nAnchorType, sal_uInt16 nH, sal_uInt16 nHRel,
                            sal_uInt16 nV,  sal_uInt16 nVRel,
                            long   nX,  long   nY);
    sal_uInt16          GetMapPos(FrmMap *pMap, ListBox &rAlignLB);
    short           GetAlignment(FrmMap *pMap, sal_uInt16 nMapPos, ListBox &rAlignLB, ListBox &rRelationLB);
    short           GetRelation(FrmMap *pMap, ListBox &rRelationLB);
    short           GetAnchorType(bool* pbHasChanged = 0);
    sal_uLong           FillRelLB(FrmMap *pMap, sal_uInt16 nLBSelPos, sal_uInt16 nAlign, sal_uInt16 nRel, ListBox &rLB, FixedText &rFT);
    sal_uInt16          FillPosLB(FrmMap *pMap, sal_uInt16 nAlign, const sal_uInt16 _nRel, ListBox &rLB);

    void            UpdateExample();

    void setOptimalFrmWidth();
    void setOptimalRelWidth();

public:
    SvxSwPosSizeTabPage( Window* pParent, const SfxItemSet& rInAttrs  );
    ~SvxSwPosSizeTabPage();

    static SfxTabPage* Create( Window*, const SfxItemSet& );
    static sal_uInt16*     GetRanges();

    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual int  DeactivatePage( SfxItemSet* pSet );

    void    EnableAnchorTypes(sal_uInt16 nAnchorEnable);

    void SetValidateFramePosLink( const Link& rLink )
            {m_aValidateLink = rLink;}

    void SetView( const SdrView* pSdrView );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
