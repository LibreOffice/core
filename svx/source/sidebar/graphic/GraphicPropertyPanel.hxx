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
#ifndef SVX_PROPERTYPANEL_GRAPHICPAGE_HXX
#define SVX_PROPERTYPANEL_GRAPHICPAGE_HXX

#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/fixed.hxx>
#include <boost/scoped_ptr.hpp>

class FixedText;
class MetricField;
class ListBox;
class FloatingWindow;


namespace svx { namespace sidebar {

class GraphicPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static GraphicPropertyPanel* Create(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged(
        const DataChangedEvent& rEvent);

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled);

    SfxBindings* GetBindings();

private:
    //ui controls
    MetricField*                                        mpMtrBrightness;
    MetricField*                                        mpMtrContrast;
    ListBox*                                            mpLBColorMode;
    MetricField*                                        mpMtrTrans;
    MetricField*                                        mpMtrRed;
    MetricField*                                        mpMtrGreen;
    MetricField*                                        mpMtrBlue;
    MetricField*                                        mpMtrGamma;

    ::sfx2::sidebar::ControllerItem                     maBrightControl;
    ::sfx2::sidebar::ControllerItem                     maContrastControl;
    ::sfx2::sidebar::ControllerItem                     maTransparenceControl;
    ::sfx2::sidebar::ControllerItem                     maRedControl;
    ::sfx2::sidebar::ControllerItem                     maGreenControl;
    ::sfx2::sidebar::ControllerItem                     maBlueControl;
    ::sfx2::sidebar::ControllerItem                     maGammaControl;
    ::sfx2::sidebar::ControllerItem                     maModeControl;

    String                                              msNormal;
    String                                              msBW;
    String                                              msGray;
    String                                              msWater;

    cssu::Reference<css::frame::XFrame>                 mxFrame;
    SfxBindings*                                        mpBindings;

    DECL_LINK( ModifyBrightnessHdl, void * );
    DECL_LINK( ModifyContrastHdl, void * );
    DECL_LINK( ModifyTransHdl, void * );
    DECL_LINK( ClickColorModeHdl, void * );
    DECL_LINK( ImplPopupModeEndHdl, FloatingWindow* );
    DECL_LINK( RedHdl, void*);
    DECL_LINK( GreenHdl, void*);
    DECL_LINK( BlueHdl, void*);
    DECL_LINK( GammaHdl, void*);

    // constructor/destuctor
    GraphicPropertyPanel(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~GraphicPropertyPanel();

    void SetupIcons(void);
    void Initialize();
};


} } // end of namespace ::svx::sidebar

#endif

// eof
