/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: align.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:04:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif

#ifndef SVX_DIALCONTROL_HXX
#include "dialcontrol.hxx"
#endif
#ifndef SVX_WRAPFIELD_HXX
#include "wrapfield.hxx"
#endif
#ifndef SVX_ORIENTHELPER_HXX
#include "orienthelper.hxx"
#endif
#ifndef _SVX_FRMDIRLBOX_HXX
#include "frmdirlbox.hxx"
#endif

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

