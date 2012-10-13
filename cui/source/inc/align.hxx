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
#ifndef _SVX_ALIGN_HXX
#define _SVX_ALIGN_HXX

#include <svx/orienthelper.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>
#include <svtools/valueset.hxx>
#include <svx/dialcontrol.hxx>
#include <svx/wrapfield.hxx>
#include <svx/frmdirlbox.hxx>

namespace svx {

// ============================================================================

class AlignmentTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;

public:
    virtual             ~AlignmentTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool    FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

private:
    explicit            AlignmentTabPage( Window* pParent, const SfxItemSet& rCoreSet );

    void                InitVsRefEgde();
    void                UpdateEnableControls();

    bool                HasAlignmentChanged( const SfxItemSet& rNew, sal_uInt16 nWhich ) const;

    DECL_LINK( UpdateEnableHdl, void* );

private:
    FixedLine           maFlAlignment;
    FixedText           maFtHorAlign;
    ListBox             maLbHorAlign;
    FixedText           maFtIndent;
    MetricField         maEdIndent;
    FixedText           maFtVerAlign;
    ListBox             maLbVerAlign;

    FixedLine           maFlOrient;
    DialControl         maCtrlDial;
    FixedText           maFtRotate;
    WrapField           maNfRotate;
    FixedText           maFtRefEdge;
    ValueSet            maVsRefEdge;
    TriStateBox         maCbStacked;
    TriStateBox         maCbAsianMode;
    OrientationHelper   maOrientHlp;

    FixedLine           maFlProperties;
    TriStateBox         maBtnWrap;
    TriStateBox         maBtnHyphen;
    TriStateBox         maBtnShrink;
    FixedText           maFtFrameDir;
    FrameDirListBox     maLbFrameDir;
};

// ============================================================================

} // namespace svx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
