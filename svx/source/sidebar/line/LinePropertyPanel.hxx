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
#ifndef SVX_PROPERTYPANEL_LINEPAGE_HXX
#define SVX_PROPERTYPANEL_LINEPAGE_HXX

#include <svx/xdash.hxx>
#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <svx/sidebar/ColorPopup.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <svx/xtable.hxx>
#include "LineWidthPopup.hxx"


namespace svx { class ToolboxButtonColorUpdater; }
class SvxLineColorPage;
class SvxLineStylePage;
class SvxLineWidthPage;
class XLineStyleItem;
class XLineDashItem;
class XLineStartItem;
class XLineEndItem;
class XLineEndList;
class XDashList;
class ListBox;
class ToolBox;
class FloatingWindow;

namespace {
    #define SIDEBAR_LINE_WIDTH_GLOBAL_VALUE "PopupPanel_LineWidth"
} //end of anonymous namespace

namespace svx { namespace sidebar {

class PopupContainer;
class ColorPopup;
class LineWidthControl;


class LinePropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
private:
    friend class ::SvxLineStylePage;
    friend class ::SvxLineWidthPage;

public:
    static LinePropertyPanel* Create(
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

    void SetWidth(long nWidth);
    void SetWidthIcon(int n);
    void SetWidthIcon();

    void EndLineWidthPopupMode (void);

private:
    //ui controls
    FixedText*                        mpFTWidth;
    ToolBox*                          mpTBWidth;
    FixedText*                        mpFTColor;
    ToolBox*                          mpTBColor;
    FixedText*                        mpFTStyle;
    ListBox*                          mpLBStyle;
    FixedText*                        mpFTTrancparency;
    MetricField*                      mpMFTransparent;
    FixedText*                        mpFTArrow;
    ListBox*                          mpLBStart;
    ListBox*                          mpLBEnd;
    FixedText*                        mpFTEdgeStyle;
    ListBox*                          mpLBEdgeStyle;
    FixedText*                        mpFTCapStyle;
    ListBox*                          mpLBCapStyle;

    //ControllerItem
    ::sfx2::sidebar::ControllerItem                         maStyleControl;
    ::sfx2::sidebar::ControllerItem                         maDashControl;
    ::sfx2::sidebar::ControllerItem                         maWidthControl;
    ::sfx2::sidebar::ControllerItem                         maColorControl;
    ::sfx2::sidebar::ControllerItem                         maStartControl;
    ::sfx2::sidebar::ControllerItem                         maEndControl;
    ::sfx2::sidebar::ControllerItem                         maLineEndListControl;
    ::sfx2::sidebar::ControllerItem                         maLineStyleListControl;
    ::sfx2::sidebar::ControllerItem                         maTransControl;
    ::sfx2::sidebar::ControllerItem                         maEdgeStyle;
    ::sfx2::sidebar::ControllerItem                         maCapStyle;

    Color                                                   maColor;
    ::boost::scoped_ptr< ::svx::ToolboxButtonColorUpdater > mpColorUpdater;
    ::boost::scoped_ptr< XLineStyleItem >                   mpStyleItem;
    ::boost::scoped_ptr< XLineDashItem >                    mpDashItem;
    sal_uInt16                                              mnTrans;
    SfxMapUnit                                              meMapUnit;
    sal_Int32                                               mnWidthCoreValue;
    XLineEndListRef                                         mxLineEndList;
    XDashListRef                                            mxLineStyleList;
    ::boost::scoped_ptr< XLineStartItem >                   mpStartItem;
    ::boost::scoped_ptr< XLineEndItem >                     mpEndItem;

    //popup windows
    ColorPopup maColorPopup;
    LineWidthPopup maLineWidthPopup;

    // images from ressource
    Image                                                   maIMGColor;
    Image                                                   maIMGNone;

    // multi-images
    ::boost::scoped_array<Image>                            mpIMGWidthIcon;

    cssu::Reference< css::frame::XFrame >                   mxFrame;
    SfxBindings*                                            mpBindings;

    /// bitfield
    bool                mbColorAvailable : 1;
    bool                mbWidthValuable : 1;

    void SetupIcons(void);
    void Initialize();
    void FillLineEndList();
    void FillLineStyleList();
    void SelectEndStyle(bool bStart);
    void SelectLineStyle();

    DECL_LINK(ImplPopupModeEndHdl, FloatingWindow* );
    DECL_LINK(ImplWidthPopupModeEndHdl, FloatingWindow* );
    DECL_LINK(ToolboxColorSelectHdl, ToolBox*);
    DECL_LINK(ChangeLineStyleHdl, void*);
    DECL_LINK(ToolboxWidthSelectHdl, ToolBox*);
    DECL_LINK(ChangeTransparentHdl , void *);
    DECL_LINK(ChangeStartHdl, void *);
    DECL_LINK(ChangeEndHdl, void *);
    DECL_LINK(ChangeEdgeStyleHdl, void *);
    DECL_LINK(ChangeCapStyleHdl, void *);

    // constructor/destuctor
    LinePropertyPanel(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~LinePropertyPanel(void);

    void SetColor(
        const OUString& rsColorName,
        const Color aColor);

    PopupControl* CreateColorPopupControl (PopupContainer* pParent);
    PopupControl* CreateLineWidthPopupControl (PopupContainer* pParent);
};

} } // end of namespace svx::sidebar

#endif

// eof
