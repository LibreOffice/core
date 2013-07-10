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
#ifndef SVX_PROPERTYPANEL_POSIZEPAGE_HXX
#define SVX_PROPERTYPANEL_POSIZEPAGE_HXX

#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <boost/scoped_ptr.hpp>
#include <svx/rectenum.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <svl/poolitem.hxx>
#include <tools/fldunit.hxx>
#include <com/sun/star/ui/XSidebar.hpp>

class DialControl;
class SdrView;
class FixedText;
class MetricField;
class CheckBox;
class MetricBox;


namespace svx { namespace sidebar {

class SidebarDialControl;

class PosSizePropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static PosSizePropertyPanel* Create(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const cssu::Reference<css::ui::XSidebar>& rxSidebar);

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
    void ShowMenu (void);

private:
    //Position
    FixedText*        mpFtPosX;
    MetricField*      mpMtrPosX;
    FixedText*        mpFtPosY;
    MetricField*      mpMtrPosY;

    // size
    FixedText*        mpFtWidth;
    MetricField*      mpMtrWidth;
    FixedText*        mpFtHeight;
    MetricField*      mpMtrHeight;
    CheckBox*         mpCbxScale;

    //rotation
    FixedText*        mpFtAngle;
    MetricBox*        mpMtrAngle;

    //rotation control
    SidebarDialControl*  mpDial;

    //flip
    FixedText*        mpFtFlip;
    ToolBox*          mpFlipTbx;

    // Internal variables
    Rectangle                               maRect;
    const SdrView*                          mpView;
    sal_uInt32                              mlOldWidth;
    sal_uInt32                              mlOldHeight;
    RECT_POINT                              meRP;
    Point                                   maAnchorPos;    //anchor position
    long                                    mlRotX;
    long                                    mlRotY;
    Fraction                                maUIScale;
    SfxMapUnit                              mePoolUnit;
    FieldUnit                               meDlgUnit;

    // Controller Items
    ::sfx2::sidebar::ControllerItem         maTransfPosXControl;
    ::sfx2::sidebar::ControllerItem         maTransfPosYControl;
    ::sfx2::sidebar::ControllerItem         maTransfWidthControl;
    ::sfx2::sidebar::ControllerItem         maTransfHeightControl;

    ::sfx2::sidebar::ControllerItem         maSvxAngleControl;
    ::sfx2::sidebar::ControllerItem         maRotXControl;
    ::sfx2::sidebar::ControllerItem         maRotYControl;
    ::sfx2::sidebar::ControllerItem         maProPosControl;
    ::sfx2::sidebar::ControllerItem         maProSizeControl;
    ::sfx2::sidebar::ControllerItem         maAutoWidthControl;
    ::sfx2::sidebar::ControllerItem         maAutoHeightControl;
    ::sfx2::sidebar::ControllerItem         m_aMetricCtl;

    cssu::Reference< css::frame::XFrame >   mxFrame;
    ::sfx2::sidebar::EnumContext            maContext;
    SfxBindings*                            mpBindings;

    // to remember original positions for restoring these for different layouts
    Point                                   maFtWidthOrigPos;
    Point                                   maMtrWidthOrigPos;
    Point                                   maFtHeightOrigPos;
    Point                                   maMtrHeightOrigPos;
    Point                                   maCbxScaleOrigPos;
    Point                                   maFtAngleOrigPos;
    Point                                   maMtrAnglOrigPos;
    Point                                   maFlipTbxOrigPos;
    Point                                   maDialOrigPos;
    Point                                   maFtFlipOrigPos;

    /// bitfield
    bool                                    mbMtrPosXMirror : 1;
    bool                                    mbSizeProtected : 1;
    bool                                    mbPositionProtected : 1;
    bool                                    mbAutoWidth : 1;
    bool                                    mbAutoHeight : 1;
    bool                                    mbAdjustEnabled : 1;
    bool                                    mbIsFlip : 1;

    cssu::Reference<css::ui::XSidebar> mxSidebar;

    DECL_LINK( ChangePosXHdl, void * );
    DECL_LINK( ChangePosYHdl, void * );
    DECL_LINK( ChangeWidthHdl, void * );
    DECL_LINK( ChangeHeightHdl, void * );
    DECL_LINK( ClickAutoHdl, void * );
    DECL_LINK( AngleModifiedHdl, void * );
    DECL_LINK( RotationHdl, void * );
    DECL_LINK( FlipHdl, ToolBox * );

    void SetupIcons(void);
    void Initialize();
    void executePosX();
    void executePosY();
    void executeSize();

    // constructor/destuctor
    PosSizePropertyPanel(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const cssu::Reference<css::ui::XSidebar>& rxSidebar);
    virtual ~PosSizePropertyPanel();

    void MetricState( SfxItemState eState, const SfxPoolItem* pState );
    FieldUnit GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState );
    void DisableControls();
};


} } // end of namespace svx::sidebar



#endif // SVX_PROPERTYPANEL_POSIZEPAGE_HXX

// eof
