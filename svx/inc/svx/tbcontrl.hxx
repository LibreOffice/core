/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_TBCONTRL_HXX
#define _SVX_TBCONTRL_HXX

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
        Bemerkung:      provides dependant of chosen ValueSet-Item
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
#include "svx/svxdllapi.h"

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
//########################################################################

//========================================================================
// class SvxStyleToolBoxControl ------------------------------------------
//========================================================================
class SVX_DLLPUBLIC SvxStyleToolBoxControl : public SfxToolBoxControl
{
    struct Impl;
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxStyleToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxStyleToolBoxControl();

    virtual Window* CreateItemWindow( Window *pParent );

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );

    DECL_LINK( VisibilityNotification, void* );
protected:
    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XUpdatable
    virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

private:

#define MAX_FAMILIES 5

    SfxStyleSheetBasePool*              pStyleSheetPool;
    SfxStyleControllerItem_Impl*        pBoundItems [MAX_FAMILIES];
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > m_xBoundItems[MAX_FAMILIES];
    SfxTemplateItem*                    pFamilyState[MAX_FAMILIES];
    sal_uInt16                              nActFamily; // Id in the ToolBox = Position - 1
    String                              aCurSel;
    Impl*                               pImpl;

    SVX_DLLPRIVATE void Update();
    SVX_DLLPRIVATE void FillStyleBox();
    SVX_DLLPRIVATE void     SelectStyle( const String& rStyleName );

friend class SfxStyleControllerItem_Impl;

    SVX_DLLPRIVATE void         SetFamilyState  ( sal_uInt16 nIdx, const SfxTemplateItem* pItem );
    SVX_DLLPRIVATE SfxStyleFamily   GetActFamily    ();
};

//========================================================================
// class SvxFontNameToolBoxControl ---------------------------------------
//========================================================================


class SVX_DLLPUBLIC SvxFontNameToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFontNameToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );

    virtual void    StateChanged        ( sal_uInt16 nSID, SfxItemState eState,
                                          const SfxPoolItem* pState );
    virtual Window* CreateItemWindow    ( Window *pParent );
};



//========================================================================
// class SvxFontColorToolBoxControl --------------------------------------
//========================================================================



class SVX_DLLPUBLIC SvxFontColorToolBoxControl : public SfxToolBoxControl
{
    ::svx::ToolboxButtonColorUpdater*   pBtnUpdater;
    Color                               mLastColor;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFontColorToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxFontColorToolBoxControl();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};



//========================================================================
// class SvxColorExtToolBoxControl --------------------------------------
//========================================================================



class SVX_DLLPUBLIC SvxColorExtToolBoxControl : public SfxToolBoxControl
{
    using SfxToolBoxControl::StateChanged;
    using SfxToolBoxControl::Select;

    //  SfxStatusForwarder          aForward;
    ::svx::ToolboxButtonColorUpdater*   pBtnUpdater;
    Color                               mLastColor;
    sal_Bool                            bChoiceFromPalette;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxColorExtToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxColorExtToolBoxControl();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                Select( sal_Bool bMod1 = sal_False );
};



//========================================================================
// class SvxColorToolBoxControl ------------------------------------------
//========================================================================


class SVX_DLLPUBLIC SvxColorToolBoxControl : public SfxToolBoxControl
{
    ::svx::ToolboxButtonColorUpdater*   pBtnUpdater;
    Color                               mLastColor;

public:
    SvxColorToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxColorToolBoxControl();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};



//========================================================================
// class SvxFrameToolBoxControl ------------------------------------------
//========================================================================


class SVX_DLLPUBLIC SvxFrameToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFrameToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );

};



//========================================================================
// class SvxLineStyleToolBoxControl --------------------------------------
//========================================================================


class SVX_DLLPUBLIC SvxFrameLineStyleToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFrameLineStyleToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
};



//========================================================================
// class SvxFrameLineColorToolBoxControl ---------------------------------
//========================================================================


class SVX_DLLPUBLIC SvxFrameLineColorToolBoxControl : public SfxToolBoxControl
{
    ::svx::ToolboxButtonColorUpdater*   pBtnUpdater;
    Color                               mLastColor;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFrameLineColorToolBoxControl( sal_uInt16 nSlotId,
                                     sal_uInt16 nId,
                                     ToolBox& rTbx );
    ~SvxFrameLineColorToolBoxControl();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};

class SVX_DLLPUBLIC SvxSimpleUndoRedoController : public SfxToolBoxControl
{
private:
    ::rtl::OUString             aDefaultText;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxSimpleUndoRedoController( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxSimpleUndoRedoController();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
};

#endif // #ifndef _SVX_TBCONTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
