/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: align.hxx,v $
 * $Revision: 1.7 $
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

// include ---------------------------------------------------------------


#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <sfx2/tabdlg.hxx>
#include <svtools/valueset.hxx>
#include "dialcontrol.hxx"
#include "wrapfield.hxx"
#include "orienthelper.hxx"
#include <svx/frmdirlbox.hxx>

namespace svx {

// ============================================================================

class AlignmentTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;

public:
    virtual             ~AlignmentTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    static USHORT*      GetRanges();

    virtual void        Reset( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

private:
    explicit            AlignmentTabPage( Window* pParent, const SfxItemSet& rCoreSet );

    void                InitVsRefEgde();
    void                UpdateEnableControls();

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

