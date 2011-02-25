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

    Beschreibung:
    -------------
    ToolBox-Controler fuer:
        Font-Name, Font-Hoehe, Font-Farbe
        Vorder-/Hintergrundfarbe /-Muster
        Rahmen, Linien
        (Absatz-)Vorlagen

    Bedienung:
    ----------
        SvxFontNameToolBoxControl
        -------------------------
        Item-Typ:       SvxFontItem
        Execute-Id:     SID_ATTR_CHAR_FONT
                        -> SvxFontItem
        Zusatzdaten
        aus DocShell:   SvxFontListItem (SID_ATTR_CHAR_FONTLIST)

        SvxFontHeightToolBoxControl
        ---------------------------
        Item-Typ:       SvxFontHeightItem
        Execute-Id:     SID_ATTR_CHAR_FONTHEIGHT
                        -> SvxFontHeightItem
        Zusatzdaten
        aus DocShell:   SvxFontListItem (SID_ATTR_CHAR_FONTLIST)

        SvxFontColorToolBoxControl
        --------------------------
        Item-Typ:       SvxFontColorItem
        Execute-Id:     SID_ATTR_CHAR_COLOR
                        -> SvxFontColorItem
        Zusatzdaten
        aus DocShell:   z.Zt. keine
                        in Zukunft: Farbpalette

        class SvxFontColorExtToolBoxControl
        -----------------------------------
        fuer die Zeichenfarbe
        Item-Typ        SvxColorItem
                    und SfxBoolItem
        Execute-Id      SID_ATTR_CHAR_COLOR
                    und SID_ATTR_CHAR_COLOR_EXT

        fuer den Zeichenhintergrund
        Item-Typ        SvxColorItem
                    und SfxBoolItem
        Execute-Id      SID_ATTR_CHAR_COLOR_BACKGROUND
                    und SID_ATTR_CHAR_COLOR_BACKGROUND_EXT

        SvxColorToolBoxControl
        --------------------------------
        Item-Typ:       SvxBrushItem
        Execute-Id:     SID_BACKGROUND_COLOR
                        -> SvxColorItem
        Zusatzdaten
        aus DocShell:   z.Zt. keine
                        in Zukunft: Farbpalette

        SvxPatternToolBoxControl
        ------------------------
        Item-Typ:       SvxBrushItem
        Execute-Id:     SID_BACKGROUND_PATTERN
                        -> SvxBrushItem
        Zusatzdaten
        aus DocShell:   z.Zt. keine
                        in Zukunft: Farbpalette
        Bemerkung:      Auswertung des BrushItems:
                        Die Brush-FillColor() wird als Schalter
                        missbraucht, um zu unterscheiden, ob ein
                        neuer Style oder eine neue Farbe
                        gesetzt wurde:

                        GetFillColor() == COL_BLACK -> GetStyle() auswerten,
                        GetFillColor() == COL_WHITE -> GetColor() auswerten

        SvxFrameToolBoxControl
        ----------------------
        Item-Typ:       SvxBoxItem
        Execute-Id:     SID_ATTR_BORDER
                        -> SvxBoxItem & SvxBoxInfoItem
        Zusatzdaten
        aus DocShell:   keine
        Bemerkung:      liefert je nach gewaehltem ValueSet-Item
                        nur SvxBoxItem oder zusaetzlich SvxBoxInfoItem
                        Wird dem Controller ein SfxUInt16Item mit einem
                        Wert != 0 geschickt, so wird der Paragraph-Mode
                        eingeschaltet, d.h. die letzte Zeile wird
                        ausgeblendet. Ein Wert == 0 schaltet wieder in
                        den Tabellenmodus.

        SvxFrameLineStyleToolBoxControl
        -------------------------------
        Item-Typ:       SvxLineItem
        Execute-Id:     SID_FRAME_LINESTYLE
                        -> SvxLineItem
        Zusatzdaten
        aus DocShell:   keine
        Bemerkung:      liefert ein SvxLineItem, welches eine SvxBorderLine
                        ohne Farbinformation liefert.

        SvxFrameLineColorToolBoxControl
        -------------------------------
        Item-Typ:       SvxColorItem
        Execute-Id:     SID_FRAME_LINECOLOR
                        -> SvxColorItem
        Zusatzdaten
        aus DocShell:   keine

        SvxStyleToolBoxControl
        ----------------------
        Item-Typ:       SfxTemplateItem
        Execute-Id:     SID_TEMPLATE_APPLY
                        -> StyleName (SfxStringItem)
                        -> eStyleFamily (SfxUInt16Item)
        Zusatzdaten
        aus DocShell:   keine
        Bemerkung:      Umschalten der Familie ueber Invalidate
                        an den Bindings (->SfxStyleControllerItem)

\*--------------------------------------------------------------*/

// include ---------------------------------------------------------------

// ITEMID_...-Defines im *.cxx

#include <rsc/rscsfx.hxx>
#include <svl/lstner.hxx>
#include <sfx2/tbxctrl.hxx>
#include <svx/strarray.hxx>
#include "svx/svxdllapi.h"

#include <com/sun/star/awt/FontDescriptor.hpp>

// wichtig im mit HeDaBu erzeugtem tbxctrls.hxx!!!
class SvxLineItem;
class SvxBoxInfoItem;
class SvxFontItem;
class SvxFontHeightItem;
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

    SvxStyleToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxStyleToolBoxControl();

    virtual Window* CreateItemWindow( Window *pParent );

    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
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
    USHORT                              nActFamily; // Id in der ToolBox = Position - 1
    USHORT                              nActFilter; // FilterIdx
    USHORT                              nAppFilter; // Filter, den die App gesetzt hat
    String                              aCurSel;
    BOOL                                bListening;
    Impl*                               pImpl;

    SVX_DLLPRIVATE void Update();
    SVX_DLLPRIVATE void FillStyleBox();
    SVX_DLLPRIVATE void     SelectStyle( const String& rStyleName );

friend class SfxStyleControllerItem_Impl;

    SVX_DLLPRIVATE void         SetFamilyState  ( USHORT nIdx, const SfxTemplateItem* pItem );
    SVX_DLLPRIVATE SfxStyleFamily   GetActFamily    ();
};

//========================================================================
// class SvxFontNameToolBoxControl ---------------------------------------
//========================================================================


class SVX_DLLPUBLIC SvxFontNameToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFontNameToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );

    virtual void    StateChanged        ( USHORT nSID, SfxItemState eState,
                                          const SfxPoolItem* pState );
    virtual Window* CreateItemWindow    ( Window *pParent );
};



//========================================================================
// class SvxFontHeightToolBoxControl -------------------------------------
//========================================================================

/*
class SvxFontSizeItem;
class SvxFontItem;
class SvxFontSizeBox_Impl;

class SVX_DLLPUBLIC SvxFontHeightToolBoxControl : public SfxToolBoxControl
{
    SvxFontSizeBox_Impl* pBox;                          // ItemWindow f"ur quick-access
    ::com::sun::star::awt::FontDescriptor aCurrentFont; // Kopie des aktuellen Fonts

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFontHeightToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxFontHeightToolBoxControl();

    // new controller API
    // XStatusListener
    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual Window*     CreateItemWindow( Window *pParent );
};
*/


//========================================================================
// class SvxFontColorToolBoxControl --------------------------------------
//========================================================================



class SVX_DLLPUBLIC SvxFontColorToolBoxControl : public SfxToolBoxControl
{
    ::svx::ToolboxButtonColorUpdater*   pBtnUpdater;
    Color                               mLastColor;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFontColorToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxFontColorToolBoxControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};



//========================================================================
// class SvxFontColorExtToolBoxControl --------------------------------------
//========================================================================



class SVX_DLLPUBLIC SvxFontColorExtToolBoxControl : public SfxToolBoxControl
{
    using SfxToolBoxControl::StateChanged;
    using SfxToolBoxControl::Select;

    //  SfxStatusForwarder          aForward;
    ::svx::ToolboxButtonColorUpdater*   pBtnUpdater;
    Color                               mLastColor;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFontColorExtToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxFontColorExtToolBoxControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                Select( BOOL bMod1 = FALSE );

};



//========================================================================
// class SvxColorToolBoxControl ------------------------------------------
//========================================================================


class SVX_DLLPUBLIC SvxColorToolBoxControl : public SfxToolBoxControl
{
    ::svx::ToolboxButtonColorUpdater*   pBtnUpdater;
    Color                               mLastColor;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxColorToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxColorToolBoxControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
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
    SvxFrameToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );

};



//========================================================================
// class SvxLineStyleToolBoxControl --------------------------------------
//========================================================================


class SVX_DLLPUBLIC SvxFrameLineStyleToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFrameLineStyleToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
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
    SvxFrameLineColorToolBoxControl( USHORT nSlotId,
                                     USHORT nId,
                                     ToolBox& rTbx );
    ~SvxFrameLineColorToolBoxControl();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};



class SvxReloadControllerItem_Impl;
class SvxReloadControllerItem : public SfxToolBoxControl
{
private:
    SvxReloadControllerItem_Impl* pImpl;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxReloadControllerItem( USHORT nSlotId,
                             USHORT nId,
                             ToolBox& rTbx );
    ~SvxReloadControllerItem();

protected:
    virtual void StateChanged( USHORT nSID, SfxItemState eState,
                               const SfxPoolItem* pState );
};

class SVX_DLLPUBLIC SvxSimpleUndoRedoController : public SfxToolBoxControl
{
private:
    ::rtl::OUString             aDefaultText;

public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxSimpleUndoRedoController( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxSimpleUndoRedoController();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
};

#endif // #ifndef _SVX_TBCONTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
