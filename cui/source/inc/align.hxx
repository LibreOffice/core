/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
