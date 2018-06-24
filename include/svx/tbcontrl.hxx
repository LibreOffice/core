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
        Note:           provides, depending on chosen ValueSet-Item,
                        only SvxBoxItem or additionally SvxBoxInfoItem
                        If the Controller in SfxUInt16Item receives a
                        value != 0, paragraph mode will be switched on,
                        i.e. the last line will be hidden.
                        A value == 0 switches again to Table mode.

        SvxFrameLineStyleToolBoxControl
        -------------------------------
        Item type:      SvxLineItem
        Execute-Id:     SID_FRAME_LINESTYLE
                        -> SvxLineItem
        Additional information
        from DocShell:  none
        Note:           provides a SvxLineItem, which provides a SvxBorderLine
                        without color information.

        SvxStyleToolBoxControl
        ----------------------
        Item type:      SfxTemplateItem
        Execute-Id:     SID_TEMPLATE_APPLY
                        -> StyleName (SfxStringItem)
                        -> eStyleFamily (SfxUInt16Item)
        Additional information
        from DocShell:  none
        Note:           Switch family by Invalidate
                        at the Bindings (->SfxStyleControllerItem)

\*--------------------------------------------------------------*/

// ITEMID_... defines in the *.cxx

#include <svl/style.hxx>
#include <svl/lstner.hxx>
#include <sfx2/tbxctrl.hxx>
#include <svx/strarray.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <svtools/popupwindowcontroller.hxx>
#include <svx/colorwindow.hxx>
#include <memory>

// important in the tbxctrls.hxx created with HeDaBu !!!
class SvxLineItem;
class SvxBoxInfoItem;
class SvxFontItem;
class SfxStyleControllerItem_Impl;
class SfxStyleSheetBasePool;
class SfxTemplateItem;
class SvxStyleBox_Impl;
class PaletteManager;

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
    virtual ~SvxStyleToolBoxControl() override;

    virtual VclPtr<vcl::Window> CreateItemWindow(vcl::Window* pParent) override;

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;

    DECL_LINK( VisibilityNotification, SvxStyleBox_Impl&, void );
protected:
    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& aArguments) override;

    // XUpdatable
    virtual void SAL_CALL update() override;

    // XComponent
    virtual void SAL_CALL dispose() override;

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

typedef std::function<void(const OUString&, const NamedColor&)> ColorSelectFunction;

class SVX_DLLPUBLIC SvxColorToolBoxControl : public cppu::ImplInheritanceHelper< svt::PopupWindowController,
                                                                                 css::frame::XSubToolbarController >
{
    std::unique_ptr<svx::ToolboxButtonColorUpdater> m_xBtnUpdater;
    std::shared_ptr<PaletteManager> m_xPaletteManager;
    BorderColorStatus m_aBorderColorStatus;
    bool m_bSplitButton;
    sal_uInt16 m_nSlotId;
    ColorSelectFunction m_aColorSelectFunction;
    DECL_LINK(SelectedHdl, const NamedColor&, void);
public:
    explicit SvxColorToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext );
    virtual ~SvxColorToolBoxControl() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence<css::uno::Any>& rArguments ) override;

    // XUpdatable
    virtual void SAL_CALL update() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 nSelectModifier ) override;

    using svt::ToolboxController::createPopupWindow;
    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XSubToolbarController
    virtual sal_Bool SAL_CALL opensSubToolbar() override;
    virtual OUString SAL_CALL getSubToolbarName() override;
    virtual void SAL_CALL functionSelected( const OUString& rCommand ) override;
    virtual void SAL_CALL updateImage() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void setColorSelectFunction(const ColorSelectFunction& aColorSelectFunction);
    void EnsurePaletteManager();
};

class SVX_DLLPUBLIC SvxSimpleUndoRedoController : public SfxToolBoxControl
{
private:
    OUString aDefaultText;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxSimpleUndoRedoController(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rToolBox);
    virtual ~SvxSimpleUndoRedoController() override;

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) override;
};

class SVX_DLLPUBLIC SvxCurrencyToolBoxControl : public svt::PopupWindowController
{
private:
    OUString     m_aFormatString;
    LanguageType m_eLanguage;
    sal_uInt32   m_nFormatKey;
public:
    static void GetCurrencySymbols( std::vector<OUString>& rList, bool bFlag,
                                    std::vector<sal_uInt16>& rCurrencyList );

    explicit SvxCurrencyToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext );
    virtual ~SvxCurrencyToolBoxControl() override;

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 nSelectModifier ) override;

    using svt::ToolboxController::createPopupWindow;
    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& rArguments ) override;
};

#endif // INCLUDED_SVX_TBCONTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
