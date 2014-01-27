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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTPROPERTYPANEL_HXX

#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/EnumContext.hxx>

#include <svtools/ctrlbox.hxx>
#include <editeng/fhgtitem.hxx>

#include <com/sun/star/ui/XSidebar.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>

#include <boost/scoped_ptr.hpp>
#include "TextCharacterSpacingPopup.hxx"
#include "TextUnderlinePopup.hxx"
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/vclenum.hxx>

class FloatingWindow;
class ToolBox;

namespace svx { namespace sidebar {

class SvxSBFontNameBox;
class PopupControl;
class PopupContainer;

class TextPropertyPanel
    : public PanelLayout,
      public ::sfx2::sidebar::IContextChangeReceiver,
      public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static TextPropertyPanel* Create (
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const ::sfx2::sidebar::EnumContext& rContext);

    virtual void DataChanged (const DataChangedEvent& rEvent);

    ::sfx2::sidebar::ControllerItem& GetSpaceController();
    long GetSelFontSize();
    void SetSpacing(long nKern);
    void EndSpacingPopupMode (void);
    void EndUnderlinePopupMode (void);
    void SetUnderline(FontUnderline eUnderline);
    Color& GetUnderlineColor();


    virtual void HandleContextChange (
        const ::sfx2::sidebar::EnumContext aContext);


    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled);

private:
    //ui controls
    SvxSBFontNameBox* mpFontNameBox;
    FontSizeBox* mpFontSizeBox;
    ToolBox* mpToolBoxFont;
    ToolBox* mpToolBoxIncDec;
    ToolBox* mpToolBoxScript;
    ToolBox* mpToolBoxSpacing;
    ToolBox* mpToolBoxFontColor;

    //control items
    ::sfx2::sidebar::ControllerItem maFontNameControl;
    ::sfx2::sidebar::ControllerItem maFontSizeControl;
    ::sfx2::sidebar::ControllerItem maUnderlineControl;
    ::sfx2::sidebar::ControllerItem maSpacingControl;

    FontUnderline               meUnderline;
    Color                       meUnderlineColor;
    bool                        mbKernAvailable;
    bool                        mbKernLBAvailable;
    long                        mlKerning;
    SvxFontHeightItem*          mpHeightItem;

    bool mbFocusOnFontSizeCtrl;
    TextCharacterSpacingPopup maCharSpacePopup;
    TextUnderlinePopup maUnderlinePopup;

    ::sfx2::sidebar::EnumContext maContext;
    SfxBindings* mpBindings;

    TextPropertyPanel (
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const ::sfx2::sidebar::EnumContext& rContext);
    virtual ~TextPropertyPanel (void);


    PopupControl* CreateCharacterSpacingControl (PopupContainer* pParent);
    PopupControl* CreateUnderlinePopupControl (PopupContainer* pParent);
    DECL_LINK(SpacingClickHdl, ToolBox*);
    DECL_LINK(ToolBoxUnderlineClickHdl, ToolBox* );

    void Initialize (void);
    void SetupToolboxItems (void);
    void InitToolBoxFont();
    void InitToolBoxSpacing();

    DECL_LINK(FontSelHdl, FontNameBox *);
    DECL_LINK(FontSizeModifyHdl, FontSizeBox *);
    DECL_LINK(FontSizeSelHdl, FontSizeBox *);
    DECL_LINK(FontSizeLoseFocus, FontSizeBox *);
    DECL_LINK(ToolboxFontSelectHandler, ToolBox *);

    void UpdateFontColorToolbox (bool bWriterText);
};

} } // end of namespace ::svx::sidebar

#endif
