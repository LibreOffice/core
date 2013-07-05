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
#ifndef SVX_SIDEBAR_TEXT_PROPERTY_PAGE_HXX
#define SVX_SIDEBAR_TEXT_PROPERTY_PAGE_HXX

#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <sfx2/sidebar/GridLayouter.hxx>

#include <svtools/ctrlbox.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/fhgtitem.hxx>

#include <com/sun/star/ui/XSidebar.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>

#include <boost/scoped_ptr.hpp>
#include "TextCharacterSpacingPopup.hxx"
#include "TextUnderlinePopup.hxx"
#include <svx/sidebar/ColorPopup.hxx>
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
    void SetDefaultUnderline(FontUnderline eUnderline);


    virtual void HandleContextChange (
        const ::sfx2::sidebar::EnumContext aContext);


    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled);

    // Inherited from vcl Window.
    virtual void Resize (void);

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
    ::sfx2::sidebar::ControllerItem maWeightControl;
    ::sfx2::sidebar::ControllerItem maItalicControl;
    ::sfx2::sidebar::ControllerItem maUnderlineControl;
    ::sfx2::sidebar::ControllerItem maStrikeControl;
    ::sfx2::sidebar::ControllerItem maShadowControl;
    ::sfx2::sidebar::ControllerItem maScriptControlSw;
    ::sfx2::sidebar::ControllerItem maSuperScriptControl;
    ::sfx2::sidebar::ControllerItem maSubScriptControl;
    ::sfx2::sidebar::ControllerItem maSpacingControl;
    ::sfx2::sidebar::ControllerItem maSDFontGrow;
    ::sfx2::sidebar::ControllerItem maSDFontShrink;

    FontWeight                  meWeight;
    FontItalic                  meItalic;
    FontUnderline               meUnderline;
    Color                       meUnderlineColor;
    bool                        mbShadow;
    FontStrikeout               meStrike;
    bool mbWeightAvailable;
    bool mbPostureAvailable;
    SvxEscapement               meEscape;  //for sw
    bool                        mbSuper;
    bool                        mbSub;
    bool                        mbKernAvailable;
    bool                        mbKernLBAvailable;
    long                        mlKerning;
    SvxFontHeightItem*          mpHeightItem;

    const FontList* mpFontList;
    bool mbMustDelete;
    bool mbFocusOnFontSizeCtrl;
    TextCharacterSpacingPopup maCharSpacePopup;
    TextUnderlinePopup maUnderlinePopup;

    ::sfx2::sidebar::EnumContext maContext;
    SfxBindings* mpBindings;
    ::sfx2::sidebar::GridLayouter maLayouter;

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
    void InitToolBoxIncDec();
    void InitToolBoxScript();
    void InitToolBoxSpacing();

    DECL_LINK(FontSelHdl, FontNameBox *);
    DECL_LINK(FontSizeModifyHdl, FontSizeBox *);
    DECL_LINK(FontSizeSelHdl, FontSizeBox *);
    DECL_LINK(FontSizeLoseFocus, FontSizeBox *);
    DECL_LINK(ToolboxFontSelectHandler, ToolBox *);
    DECL_LINK(ToolboxIncDecSelectHdl, ToolBox *);
    DECL_LINK(ToolBoxScriptSelectHdl, ToolBox *);

    void UpdateItem (const sal_uInt16 nSlotId);

    /** Depending on the given context make one of the toolboxes
        mpToolBoxFontColor and mpToolBoxFontColorSW visible.  Both
        occupy the same space.
    */
    void UpdateFontColorToolbox (
        const ::sfx2::sidebar::EnumContext aContext);

    bool isWriter();
};

} } // end of namespace ::svx::sidebar

#endif
