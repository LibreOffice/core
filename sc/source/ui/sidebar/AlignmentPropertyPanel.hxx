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
#ifndef SC_PROPERTYPANEL_ALIGNMENT_HXX
#define SC_PROPERTYPANEL_ALIGNMENT_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/fixed.hxx>
#include <boost/scoped_ptr.hpp>
#include <editeng/svxenum.hxx>

class ToolBox;
class MetricField;
class MetricBox;
class CheckBox;
namespace svx { namespace sidebar { class SidebarDialControl; }}

namespace sc { namespace sidebar {

class AlignmentPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static AlignmentPropertyPanel* Create(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged(
        const DataChangedEvent& rEvent);

    virtual void HandleContextChange(
        const ::sfx2::sidebar::EnumContext aContext);

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled);

    SfxBindings* GetBindings();

private:
    //ui controls
    ToolBox*                                    mpTBHorizontal;
    ToolBox*                                    mpTBVertical;
    FixedText*                                  mpFTLeftIndent;
    MetricField*                                mpMFLeftIndent;
    CheckBox*                                   mpCBXWrapText;
    CheckBox*                                   mpCBXMergeCell;
    FixedText*                                  mpFtRotate;
    svx::sidebar::SidebarDialControl*           mpCtrlDial;
    MetricBox*                                  mpMtrAngle;
    CheckBox*                                   mpCbStacked;

    ::sfx2::sidebar::ControllerItem             maAlignHorControl;
    ::sfx2::sidebar::ControllerItem             maAlignVerControl;
    ::sfx2::sidebar::ControllerItem             maLeftIndentControl;
    ::sfx2::sidebar::ControllerItem             maMergeCellControl;
    ::sfx2::sidebar::ControllerItem             maWrapTextControl;
    ::sfx2::sidebar::ControllerItem             maAngleControl;
    ::sfx2::sidebar::ControllerItem             maStackControl;

    SvxCellHorJustify                           meHorAlignState;
    SvxCellVerJustify                           meVerAlignState;

    /// bitfield
    bool                                        mbMultiDisable : 1;

    cssu::Reference<css::frame::XFrame>         mxFrame;
    ::sfx2::sidebar::EnumContext                maContext;
    SfxBindings*                                mpBindings;

    DECL_LINK( TbxHorAlignSelectHdl, ToolBox* );
    DECL_LINK( TbxVerAlignSelectHdl, ToolBox* );
    DECL_LINK( MFLeftIndentMdyHdl, void * );
    DECL_LINK( CBOXMergnCellClkHdl, void * );
    DECL_LINK( CBOXWrapTextClkHdl, void * );
    DECL_LINK( AngleModifiedHdl, void * );
    DECL_LINK( RotationHdl, void * );
    DECL_LINK( ClickStackHdl, void * );

    // constructor/destuctor
    AlignmentPropertyPanel(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~AlignmentPropertyPanel();

    void Initialize();
    void UpdateHorAlign();
    void UpdateVerAlign();
    void FormatDegrees(double& dTmp);
};

} } // end of namespace ::sc::sidebar

#endif
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
