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

        SvxFontColorToolBoxControl
        --------------------------
        Item type:      SvxFontColorItem
        Execute-Id:     SID_ATTR_CHAR_COLOR
                        -> SvxFontColorItem
        Additional information
        from DocShell:  presently none
                        in future: color palette

        class SvxColorExtToolBoxControl
        -----------------------------------
        Item type:      SvxColorItem
                    and SfxBoolItem

        for font color (writer, ...)
        Execute-Id      SID_ATTR_CHAR_COLOR2
                    and SID_ATTR_CHAR_COLOR_EXT

        for cell color (calc)
        Execute-Id      SID_ATTR_CHAR_COLOR

        for character background color (writer)
        Execute-Id      SID_ATTR_CHAR_COLOR_BACKGROUND
                    and SID_ATTR_CHAR_COLOR_BACKGROUND_EXT

        for paragraph background color (writer)
        Execute-Id      SID_BACKGROUND_COLOR

        for  cell background color (calc)
        Execute-Id      SID_ATTR_CHAR_COLOR_BACKGROUND

        for table/cell border color (writer, calc)
        Execute-Id      SID_FRAME_LINECOLOR

        SvxColorToolBoxControl
        --------------------------------
        Item type:      SvxBrushItem
        Execute-Id:     SID_BACKGROUND_COLOR
                        -> SvxColorItem

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

        SvxFrameLineColorToolBoxControl
        -------------------------------
        Item type:      SvxColorItem
        Execute-Id:     SID_FRAME_LINECOLOR
                        -> SvxColorItem
        Additional information
        from DocShell:   none

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

// important im tbxctrls.hxx created HeDaBu !!!
class SvxLineItem;
class SvxBoxInfoItem;
class SvxFontItem;
class SfxStyleControllerItem_Impl;
class SfxStyleSheetBasePool;
class SfxTemplateItem;

namespace svx
{
    class ToolboxButtonColorUpdater;
}



// class SvxStyleToolBoxControl ------------------------------------------

class SVX_DLLPUBLIC SvxStyleToolBoxControl : public SfxToolBoxControl
{
    struct Impl;
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxStyleToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SvxStyleToolBoxControl();

    virtual Window* CreateItemWindow( Window *pParent ) SAL_OVERRIDE;

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState ) SAL_OVERRIDE;

    DECL_LINK( VisibilityNotification, void* );
protected:
    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XUpdatable
    virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:

#define MAX_FAMILIES 5

    SfxStyleSheetBasePool*              pStyleSheetPool;
    SfxStyleControllerItem_Impl*        pBoundItems [MAX_FAMILIES];
    css::uno::Reference< css::lang::XComponent >
                                        m_xBoundItems[MAX_FAMILIES];
    SfxTemplateItem*                    pFamilyState[MAX_FAMILIES];
    sal_uInt16                          nActFamily; // Id in the ToolBox = Position - 1
    OUString                            aCurSel;
    Impl*                               pImpl;

    SVX_DLLPRIVATE void Update();
    SVX_DLLPRIVATE void FillStyleBox();
    SVX_DLLPRIVATE void     SelectStyle( const OUString& rStyleName );

friend class SfxStyleControllerItem_Impl;

    SVX_DLLPRIVATE void         SetFamilyState  ( sal_uInt16 nIdx, const SfxTemplateItem* pItem );
    SVX_DLLPRIVATE SfxStyleFamily   GetActFamily    ();
};


// class SvxFontNameToolBoxControl ---------------------------------------



class SVX_DLLPUBLIC SvxFontNameToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFontNameToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );

    virtual void    StateChanged        ( sal_uInt16 nSID, SfxItemState eState,
                                          const SfxPoolItem* pState ) SAL_OVERRIDE;
    virtual Window* CreateItemWindow    ( Window *pParent ) SAL_OVERRIDE;
};




// class SvxFontColorToolBoxControl --------------------------------------




class SVX_DLLPUBLIC SvxFontColorToolBoxControl : public SfxToolBoxControl
{
    ::svx::ToolboxButtonColorUpdater*   pBtnUpdater;
    Color                               mLastColor;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFontColorToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SvxFontColorToolBoxControl();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;
    virtual SfxPopupWindowType  GetPopupWindowType() const SAL_OVERRIDE;
    virtual SfxPopupWindow*     CreatePopupWindow() SAL_OVERRIDE;
};




// class SvxColorExtToolBoxControl --------------------------------------




class SVX_DLLPUBLIC SvxColorExtToolBoxControl : public SfxToolBoxControl
{
    using SfxToolBoxControl::StateChanged;

    ::svx::ToolboxButtonColorUpdater*   pBtnUpdater;
    Color                               mLastColor;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxColorExtToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SvxColorExtToolBoxControl();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;
    virtual SfxPopupWindowType  GetPopupWindowType() const SAL_OVERRIDE;
    virtual SfxPopupWindow*     CreatePopupWindow() SAL_OVERRIDE;
    virtual void                Select(sal_uInt16 nSelectModifier) SAL_OVERRIDE;
};




// class SvxColorToolBoxControl ------------------------------------------



class SVX_DLLPUBLIC SvxColorToolBoxControl : public SfxToolBoxControl
{
    ::svx::ToolboxButtonColorUpdater*   pBtnUpdater;
    Color                               mLastColor;

public:
    SvxColorToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SvxColorToolBoxControl();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;
    virtual SfxPopupWindowType  GetPopupWindowType() const SAL_OVERRIDE;
    virtual SfxPopupWindow*     CreatePopupWindow() SAL_OVERRIDE;
};




// class SvxFrameToolBoxControl ------------------------------------------



class SVX_DLLPUBLIC SvxFrameToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFrameToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );

    virtual SfxPopupWindowType  GetPopupWindowType() const SAL_OVERRIDE;
    virtual SfxPopupWindow*     CreatePopupWindow() SAL_OVERRIDE;
    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;

};




// class SvxLineStyleToolBoxControl --------------------------------------



class SVX_DLLPUBLIC SvxFrameLineStyleToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFrameLineStyleToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );

    virtual SfxPopupWindowType  GetPopupWindowType() const SAL_OVERRIDE;
    virtual SfxPopupWindow*     CreatePopupWindow() SAL_OVERRIDE;
    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;
};




// class SvxFrameLineColorToolBoxControl ---------------------------------



class SVX_DLLPUBLIC SvxFrameLineColorToolBoxControl : public SfxToolBoxControl
{
    ::svx::ToolboxButtonColorUpdater*   pBtnUpdater;
    Color                               mLastColor;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFrameLineColorToolBoxControl( sal_uInt16 nSlotId,
                                     sal_uInt16 nId,
                                     ToolBox& rTbx );
    virtual ~SvxFrameLineColorToolBoxControl();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;
    virtual SfxPopupWindowType  GetPopupWindowType() const SAL_OVERRIDE;
    virtual SfxPopupWindow*     CreatePopupWindow() SAL_OVERRIDE;
};

class SVX_DLLPUBLIC SvxSimpleUndoRedoController : public SfxToolBoxControl
{
private:
    OUString             aDefaultText;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxSimpleUndoRedoController( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    virtual ~SvxSimpleUndoRedoController();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState ) SAL_OVERRIDE;
};

#endif // INCLUDED_SVX_TBCONTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
