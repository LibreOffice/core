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

#include <svtools/ctrlbox.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/fhgtitem.hxx>

//#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XSidebar.hpp>

#include <boost/scoped_ptr.hpp>
#include "TextCharacterSpacingPopup.hxx"
#include "TextUnderlinePopup.hxx"
#include <svx/sidebar/ColorPopup.hxx>
#include <vcl/vclenum.hxx>

class FloatingWindow;
class ToolBox;

namespace svx { namespace sidebar {

class SvxSBFontNameBox;
class PopupControl;
class PopupContainer;

class TextPropertyPanel
    : public Control,
      public ::sfx2::sidebar::IContextChangeReceiver,
      public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static TextPropertyPanel* Create (
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged (const DataChangedEvent& rEvent);
    ::sfx2::sidebar::ControllerItem& GetSpaceController();
    long GetSelFontSize();
    void SetSpacing(long nKern);
    void EndSpacingPopupMode (void);
    void EndUnderlinePopupMode (void);
    void SetFontColor (const String& rsColorName,const Color aColor);
    void SetBrushColor (const String& rsColorName,const Color aColor);
    void SetUnderline(FontUnderline eUnderline);
    Color& GetUnderlineColor();
    void SetDefaultUnderline(FontUnderline eUnderline);


    enum ColorType
    {
        FONT_COLOR = 1,
        BACK_COLOR = 2
    };

    virtual void HandleContextChange (
        const ::sfx2::sidebar::EnumContext aContext);


    // ControllerItem::ItemUpdateReceiverInterface

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled);

private:
    //ui controls
    ::boost::scoped_ptr<SvxSBFontNameBox> mpFontNameBox;
    FontSizeBox maFontSizeBox;
    ::boost::scoped_ptr<Window> mpToolBoxIncDecBackground;
    ::boost::scoped_ptr<ToolBox> mpToolBoxIncDec;
    ::boost::scoped_ptr<Window> mpToolBoxFontBackground;
    ::boost::scoped_ptr<ToolBox> mpToolBoxFont;
    ::boost::scoped_ptr<Window> mpToolBoxFontColorBackground;
    ::boost::scoped_ptr<ToolBox> mpToolBoxFontColor;
    ::boost::scoped_ptr<Window> mpToolBoxScriptBackground;
    ::boost::scoped_ptr<ToolBox> mpToolBoxScript;
    ::boost::scoped_ptr<Window> mpToolBoxScriptSwBackground;
    ::boost::scoped_ptr<ToolBox> mpToolBoxScriptSw;
    ::boost::scoped_ptr<Window> mpToolBoxSpacingBackground;
    ::boost::scoped_ptr<ToolBox> mpToolBoxSpacing;
    ::boost::scoped_ptr<Window> mpToolBoxHighlightBackground;
    ::boost::scoped_ptr<ToolBox> mpToolBoxHighlight;
    ::boost::scoped_ptr<ToolboxButtonColorUpdater> mpFontColorUpdater;
    ::boost::scoped_ptr<ToolboxButtonColorUpdater> mpHighlightUpdater;

    //control items
    ::sfx2::sidebar::ControllerItem maFontNameControl;
    ::sfx2::sidebar::ControllerItem maFontSizeControl;
    ::sfx2::sidebar::ControllerItem maWeightControl;
    ::sfx2::sidebar::ControllerItem maItalicControl;
    ::sfx2::sidebar::ControllerItem maUnderlineControl;
    ::sfx2::sidebar::ControllerItem maStrikeControl;
    ::sfx2::sidebar::ControllerItem maShadowControl;
    ::sfx2::sidebar::ControllerItem maFontColorControl;
    ::sfx2::sidebar::ControllerItem maScriptControlSw;
    ::sfx2::sidebar::ControllerItem maSuperScriptControl;
    ::sfx2::sidebar::ControllerItem maSubScriptControl;
    ::sfx2::sidebar::ControllerItem maSpacingControl;
    ::sfx2::sidebar::ControllerItem maHighlightControl;
    ::sfx2::sidebar::ControllerItem maSDFontGrow;
    ::sfx2::sidebar::ControllerItem maSDFontShrink;

    FontWeight                  meWeight;
    FontItalic                  meItalic;
    FontUnderline               meUnderline;
    Color                       meUnderlineColor;  //
    bool                        mbShadow;
    FontStrikeout               meStrike;
    bool mbWeightAvailable;
    bool mbPostureAvailable;
    Color                       maColor;
    bool mbColorAvailable;
    Color                       maBackColor;
    bool mbBackColorAvailable;
    ColorType meColorType;
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
    ColorPopup maFontColorPopup;
    ColorPopup maBrushColorPopup;

    cssu::Reference<css::frame::XFrame> mxFrame;
    ::sfx2::sidebar::EnumContext maContext;
    SfxBindings* mpBindings;

    TextPropertyPanel (
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~TextPropertyPanel (void);


    PopupControl* CreateCharacterSpacingControl (PopupContainer* pParent);
    PopupControl* CreateFontColorPopupControl (PopupContainer* pParent);
    PopupControl* CreateBrushColorPopupControl (PopupContainer* pParent);
    PopupControl* CreateUnderlinePopupControl (PopupContainer* pParent);
    DECL_LINK(SpacingClickHdl, ToolBox*);
    DECL_LINK(ToolBoxFontColorDropHdl, ToolBox *); //for new color picker
    DECL_LINK(ToolBoxHighlightDropHdl, ToolBox *);
    DECL_LINK(ToolBoxUnderlineClickHdl, ToolBox* );

    void Initialize (void);
    void SetupToolboxItems (void);
    void InitToolBoxFont();
    void InitToolBoxIncDec();
    void InitToolBoxFontColor();
    void InitToolBoxScript();
    void InitToolBoxHighlight();
    void InitToolBoxSpacing();

    DECL_LINK(FontSelHdl, FontNameBox *);
    DECL_LINK(FontSizeModifyHdl, FontSizeBox *);
    DECL_LINK(FontSizeSelHdl, FontSizeBox *);
    DECL_LINK(FontSizeLoseFocus, FontSizeBox *);
    DECL_LINK(ToolboxFontSelectHandler, ToolBox *);
    DECL_LINK(ToolboxIncDecSelectHdl, ToolBox *);
    DECL_LINK(ImplPopupModeEndHdl, FloatingWindow* );
    DECL_LINK(ToolBoxSwScriptSelectHdl, ToolBox *);
    DECL_LINK(ToolBoxScriptSelectHdl, ToolBox *);

    void UpdateItem (const sal_uInt16 nSlotId);
};

} } // end of namespace ::svx::sidebar

#endif
