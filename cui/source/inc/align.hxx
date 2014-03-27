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
#include <svx/wrapfield.hxx>
#include <svx/frmdirlbox.hxx>

namespace svx {



class AlignmentTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;

public:
    virtual             ~AlignmentTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    static sal_uInt16*  GetRanges();

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual int         DeactivatePage( SfxItemSet* pSet ) SAL_OVERRIDE;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

private:
    explicit            AlignmentTabPage( Window* pParent, const SfxItemSet& rCoreSet );

    void                InitVsRefEgde();
    void                UpdateEnableControls();

    bool                HasAlignmentChanged( const SfxItemSet& rNew, sal_uInt16 nWhich ) const;

    DECL_LINK( UpdateEnableHdl, void* );

private:
    ListBox*             m_pLbHorAlign;
    FixedText*           m_pFtIndent;
    MetricField*         m_pEdIndent;
    FixedText*           m_pFtVerAlign;
    ListBox*             m_pLbVerAlign;

    DialControl*         m_pCtrlDial;
    FixedText*           m_pFtRotate;
    WrapField*           m_pNfRotate;
    FixedText*           m_pFtRefEdge;
    ValueSet*            m_pVsRefEdge;
    TriStateBox*         m_pCbStacked;
    TriStateBox*         m_pCbAsianMode;
    OrientationHelper*   m_pOrientHlp;

    VclHBox*             m_pBoxDirection;
    TriStateBox*         m_pBtnWrap;
    TriStateBox*         m_pBtnHyphen;
    TriStateBox*         m_pBtnShrink;
    FrameDirListBox*     m_pLbFrameDir;

    // hidden labels/string
    FixedText*           m_pFtBotLock;
    FixedText*           m_pFtTopLock;
    FixedText*           m_pFtCelLock;
    FixedText*           m_pFtABCD;

    VclContainer*        m_pAlignmentFrame;
    VclContainer*        m_pOrientFrame;
    VclContainer*        m_pPropertiesFrame;
};



} // namespace svx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
