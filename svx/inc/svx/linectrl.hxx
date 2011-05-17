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
#ifndef _SVX_LINECTRL_HXX
#define _SVX_LINECTRL_HXX


#include <svtools/valueset.hxx>
#include <svl/lstner.hxx>
#include <sfx2/tbxctrl.hxx>
#include "svx/svxdllapi.h"

class XLineStyleItem;
class XLineDashItem;
class SvxLineBox;
class SvxMetricField;
class SvxColorBox;
class XLineEndList;

//========================================================================
// SvxLineStyleController:
//========================================================================

class SVX_DLLPUBLIC SvxLineStyleToolBoxControl : public SfxToolBoxControl
{
private:
    XLineStyleItem*     pStyleItem;
    XLineDashItem*      pDashItem;

    sal_Bool                bUpdate;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxLineStyleToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxLineStyleToolBoxControl();

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    void                Update( const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );
};

//========================================================================
// SvxLineWidthController:
//========================================================================

class SVX_DLLPUBLIC SvxLineWidthToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxLineWidthToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxLineWidthToolBoxControl();

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );
};

//========================================================================
// SvxLineColorController:
//========================================================================

class SVX_DLLPUBLIC SvxLineColorToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxLineColorToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxLineColorToolBoxControl();

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                      const SfxPoolItem* pState );
    void                Update( const SfxPoolItem* pState );
    virtual Window*     CreateItemWindow( Window *pParent );
};

//========================================================================
// class SvxLineEndWindow
//========================================================================
class SvxLineEndWindow : public SfxPopupWindow
{
    using FloatingWindow::StateChanged;

private:
    XLineEndList*   pLineEndList;
    ValueSet        aLineEndSet;
    sal_uInt16          nCols;
    sal_uInt16          nLines;
    sal_uIntPtr             nLineEndWidth;
    Size            aBmpSize;
    sal_Bool            bPopupMode;
    bool            mbInResize;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;


    DECL_LINK( SelectHdl, void * );
    void            FillValueSet();
    void            SetSize();
    void            implInit();

protected:
    virtual void    Resizing( Size& rSize );
    virtual void    Resize();
    virtual sal_Bool    Close();
    virtual void    PopupModeEnd();

    /** This function is called when the window gets the focus.  It grabs
        the focus to the line ends value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus (void);

public:
    SvxLineEndWindow( sal_uInt16 nId,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                      const String& rWndTitle );
    SvxLineEndWindow( sal_uInt16 nId,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                      Window* pParentWindow,
                      const String& rWndTitle );
    ~SvxLineEndWindow();

    void            StartSelection();

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    virtual SfxPopupWindow* Clone() const;
};

//========================================================================
// class SvxColorToolBoxControl
//========================================================================

class SVX_DLLPUBLIC SvxLineEndToolBoxControl : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxLineEndToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxLineEndToolBoxControl();

    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
