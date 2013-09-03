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
#ifndef _SVX_LINECTRL_HXX
#define _SVX_LINECTRL_HXX


#include <svtools/valueset.hxx>
#include <svl/lstner.hxx>
#include <sfx2/tbxctrl.hxx>
#include <svx/svxdllapi.h>
#include <svx/xtable.hxx>

class XLineStyleItem;
class XLineDashItem;
class SvxLineBox;
class SvxMetricField;
class SvxColorBox;

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
    XLineEndListRef pLineEndList;
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
                      const OUString& rWndTitle );
    SvxLineEndWindow( sal_uInt16 nId,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                      Window* pParentWindow,
                      const OUString& rWndTitle );
    ~SvxLineEndWindow();

    void            StartSelection();

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
    virtual SfxPopupWindow* Clone() const;
};

//========================================================================
// class SvxLineEndToolBoxControl
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
