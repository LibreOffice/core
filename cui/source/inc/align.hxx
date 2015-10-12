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
#ifndef INCLUDED_CUI_SOURCE_INC_ALIGN_HXX
#define INCLUDED_CUI_SOURCE_INC_ALIGN_HXX

// list box indexes
#define ALIGNDLG_HORALIGN_STD       0
#define ALIGNDLG_HORALIGN_LEFT      1
#define ALIGNDLG_HORALIGN_CENTER    2
#define ALIGNDLG_HORALIGN_RIGHT     3
#define ALIGNDLG_HORALIGN_BLOCK     4
#define ALIGNDLG_HORALIGN_FILL      5
#define ALIGNDLG_HORALIGN_DISTRIBUTED 6

#define ALIGNDLG_VERALIGN_STD         0
#define ALIGNDLG_VERALIGN_TOP         1
#define ALIGNDLG_VERALIGN_MID         2
#define ALIGNDLG_VERALIGN_BOTTOM      3
#define ALIGNDLG_VERALIGN_BLOCK       4
#define ALIGNDLG_VERALIGN_DISTRIBUTED 5


#include <svx/orienthelper.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>
#include <svtools/valueset.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/frmdirlbox.hxx>

namespace svx {



class AlignmentTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;
    friend class VclPtr<AlignmentTabPage>;
    static const sal_uInt16 s_pRanges[];

public:
    virtual             ~AlignmentTabPage();
    virtual void        dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );
    static const sal_uInt16*  GetRanges() { return s_pRanges; }

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual sfxpg       DeactivatePage( SfxItemSet* pSet ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

private:
    explicit            AlignmentTabPage( vcl::Window* pParent, const SfxItemSet& rCoreSet );

    void                InitVsRefEgde();
    void                UpdateEnableControls();

    bool                HasAlignmentChanged( const SfxItemSet& rNew, sal_uInt16 nWhich ) const;

    DECL_LINK_TYPED( UpdateEnableHdl, ListBox&, void );
    DECL_LINK_TYPED( UpdateEnableClickHdl, Button*, void );

private:
    VclPtr<ListBox>             m_pLbHorAlign;
    VclPtr<FixedText>           m_pFtIndent;
    VclPtr<MetricField>         m_pEdIndent;
    VclPtr<FixedText>           m_pFtVerAlign;
    VclPtr<ListBox>             m_pLbVerAlign;

    VclPtr<DialControl>         m_pCtrlDial;
    VclPtr<FixedText>           m_pFtRotate;
    VclPtr<NumericField>        m_pNfRotate;
    VclPtr<FixedText>           m_pFtRefEdge;
    VclPtr<ValueSet>            m_pVsRefEdge;
    VclPtr<TriStateBox>         m_pCbStacked;
    VclPtr<TriStateBox>         m_pCbAsianMode;
    OrientationHelper*   m_pOrientHlp;

    VclPtr<VclHBox>             m_pBoxDirection;
    VclPtr<TriStateBox>         m_pBtnWrap;
    VclPtr<TriStateBox>         m_pBtnHyphen;
    VclPtr<TriStateBox>         m_pBtnShrink;
    VclPtr<FrameDirListBox>     m_pLbFrameDir;

    // hidden labels/string
    VclPtr<FixedText>           m_pFtBotLock;
    VclPtr<FixedText>           m_pFtTopLock;
    VclPtr<FixedText>           m_pFtCelLock;
    VclPtr<FixedText>           m_pFtABCD;

    VclPtr<VclContainer>        m_pAlignmentFrame;
    VclPtr<VclContainer>        m_pOrientFrame;
    VclPtr<VclContainer>        m_pPropertiesFrame;
};



}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
