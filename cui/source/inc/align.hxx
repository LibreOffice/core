/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_ALIGN_HXX
#define _SVX_ALIGN_HXX

// include ---------------------------------------------------------------

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

