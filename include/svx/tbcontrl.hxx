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
#ifndef INCLUDED_SVX_TBCONTRL_HXX
#define INCLUDED_SVX_TBCONTRL_HXX

/*--------------------------------------------------------------*\

    Description:
    -------------
    ToolBox-Controller for:
        Font-Name, Font-Height, Font-Color
        Fore-/Background color /-patterns
        Frames, Lines
        (indentation-)templates

    Use:
    ----------
        SvxFontNameToolBoxControl
        -------------------------
        Item type:      SvxFontItem
        Execute-Id:     SID_ATTR_CHAR_FONT
                        -> SvxFontItem
        Additional information
        from DocShell:   SvxFontListItem (SID_ATTR_CHAR_FONTLIST)

        SvxColorToolBoxControl
        ----------------------
        Item type:      SvxColorItem
                        SvxLineItem
                        SfxBoolItem
                        XLineColorItem
                    and XFillColorItem

        for font color (writer, ...)
        Execute-Id      SID_ATTR_CHAR_COLOR2
                    and SID_ATTR_CHAR_COLOR_EXT

        for font color
        (calc/impress/draw and writer drawing objects)
        Execute-Id      SID_ATTR_CHAR_COLOR

        for character background color (writer)
        Execute-Id      SID_ATTR_CHAR_COLOR_BACKGROUND
                    and SID_ATTR_CHAR_COLOR_BACKGROUND_EXT

        for paragraph background color (writer)
        and cell background color (calc)
        Execute-Id      SID_BACKGROUND_COLOR

        for table/cell border color (writer, calc)
        Execute-Id      SID_FRAME_LINECOLOR

        for 3D extrusion
        Execute-Id      SID_EXTRUSION_3D_COLOR

        for line color
        Execute-Id      SID_ATTR_LINE_COLOR

        for area fill color
        Execute-Id      SID_ATTR_FILL_COLOR

        SvxPatternToolBoxControl
        ------------------------
        Item type:      SvxBrushItem
        Execute-Id:     SID_BACKGROUND_PATTERN
                        -> SvxBrushItem
        Additional information
        from DocShell:   presently none
                        in future: color palette
        Note:           Analysis of BrushItem:
                        Brush-FillColor() is misused as switch,
                        to distinguish whether a new style
                        or a new color has been set

                        GetFillColor() == COL_BLACK -> GetStyle() ,
                        GetFillColor() == COL_WHITE -> GetColor()

        SvxFrameToolBoxControl
        ----------------------
        Item type:      SvxBoxItem
        Execute-Id:     SID_ATTR_BORDER
                        -> SvxBoxItem & SvxBoxInfoItem
        Additional information
        from DocShell:  none
        Bemerkung:      provides dependent of chosen ValueSet-Item
                        only SvxBoxItem or additionally SvxBoxInfoItem
                        If the Controller ein SfxUInt16Item receives a
                        value != 0, paragraph mode will be switched on,
                        i.e. the last line will be hidden.
                        A value == 0 switches again to Tabel mode.

        SvxFrameLineStyleToolBoxControl
        -------------------------------
        Item type:      SvxLineItem
        Execute-Id:     SID_FRAME_LINESTYLE
                        -> SvxLineItem
        Additional information
        from DocShell:  none
        Bemerkung:      provides a SvxLineItem, which provides a SvxBorderLine
                        without color information.

        SvxStyleToolBoxControl
        ----------------------
        Item type:      SfxTemplateItem
        Execute-Id:     SID_TEMPLATE_APPLY
                        -> StyleName (SfxStringItem)
                        -> eStyleFamily (SfxUInt16Item)
        Additional information
        from DocShell:  none
        Bemerkung:      Switch family by Invalidate
                        at the Bindings (->SfxStyleControllerItem)

\*--------------------------------------------------------------*/

// ITEMID_...-Defines i *.cxx

#include <rsc/rscsfx.hxx>
#include <svl/lstner.hxx>
#include <sfx2/tbxctrl.hxx>
#include <svx/strarray.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <svx/PaletteManager.hxx>
#include <memory>

// important im tbxctrls.hxx created HeDaBu !!!
class SvxLineItem;
class SvxBoxInfoItem;
class SvxFontItem;
class SfxStyleControllerItem_Impl;
class SfxStyleSheetBasePool;
class SfxTemplateItem;
class SvxStyleBox_Impl;

namespace svx
{
    class ToolboxButtonColorUpdater;
}

class SVX_DLLPUBLIC SvxStyleToolBoxControl : public SfxToolBoxControl
{
    struct Impl;
    std::unique_ptr<Impl> pImpl;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxStyleToolBoxControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rToolBox);
    virtual ~SvxStyleToolBoxControl();

    virtual VclPtr<vcl::Window> CreateItemWindow(vcl::Window* pParent) override;

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;

    DECL_LINK_TYPED( VisibilityNotification, SvxStyleBox_Impl&, void );
protected:
    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& aArguments)
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XUpdatable
    virtual void SAL_CALL update()
            throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose()
            throw (css::uno::RuntimeException, std::exception) override;

private:

#define MAX_FAMILIES 5

    SfxStyleSheetBasePool* pStyleSheetPool;
    SfxStyleControllerItem_Impl* pBoundItems[MAX_FAMILIES];
    css::uno::Reference<css::lang::XComponent> m_xBoundItems[MAX_FAMILIES];
    SfxTemplateItem* pFamilyState[MAX_FAMILIES];
    sal_uInt16 nActFamily; // Id in the ToolBox = Position - 1

    SVX_DLLPRIVATE void Update();
    SVX_DLLPRIVATE void FillStyleBox();
    SVX_DLLPRIVATE void SelectStyle(const OUString& rStyleName);

friend class SfxStyleControllerItem_Impl;

    SVX_DLLPRIVATE void SetFamilyState(sal_uInt16 nIdx, const SfxTemplateItem* pItem);
    SVX_DLLPRIVATE SfxStyleFamily GetActFamily();
};

class SVX_DLLPUBLIC SvxFontNameToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFontNameToolBoxControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx);

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
    virtual VclPtr<vcl::Window> CreateItemWindow(vcl::Window* pParent) override;
};

class BorderColorStatus
{
    Color maColor;
    Color maTLBRColor;
    Color maBLTRColor;
public:
    BorderColorStatus();
    ~BorderColorStatus();
    void StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState);
    Color GetColor();
};

typedef std::function<void(const OUString&, const Color&)> ColorSelectFunction;
class SVX_DLLPUBLIC SvxColorToolBoxControl : public SfxToolBoxControl
{
    using SfxToolBoxControl::StateChanged;

    std::unique_ptr<svx::ToolboxButtonColorUpdater> m_xBtnUpdater;
    PaletteManager mPaletteManager;
    BorderColorStatus maBorderColorStatus;
    bool bSidebarType;
    ColorSelectFunction maColorSelectFunction;
    DECL_LINK_TYPED(SelectedHdl, const Color&, void);
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxColorToolBoxControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rToolBox);
    virtual ~SvxColorToolBoxControl();

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
    virtual VclPtr<SfxPopupWindow> CreatePopupWindow() override;
    virtual void Select(sal_uInt16 nSelectModifier) override;

    // XSubToolbarController
    virtual sal_Bool SAL_CALL opensSubToolbar() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updateImage() throw (css::uno::RuntimeException, std::exception) override;

    void setColorSelectFunction(const ColorSelectFunction& aColorSelectFunction);
};

class SVX_DLLPUBLIC SvxFrameToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFrameToolBoxControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rToolBox);

    virtual VclPtr<SfxPopupWindow> CreatePopupWindow() override;
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
};

class SVX_DLLPUBLIC SvxFrameLineStyleToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFrameLineStyleToolBoxControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rToolBox);

    virtual VclPtr<SfxPopupWindow> CreatePopupWindow() override;
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
};

class SVX_DLLPUBLIC SvxSimpleUndoRedoController : public SfxToolBoxControl
{
private:
    OUString aDefaultText;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxSimpleUndoRedoController(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rToolBox);
    virtual ~SvxSimpleUndoRedoController();

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
};

#endif // INCLUDED_SVX_TBCONTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
