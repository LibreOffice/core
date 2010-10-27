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
#ifndef _SVX_TBXFORM_HXX
#define _SVX_TBXFORM_HXX

#include <sfx2/tbxctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/dialog.hxx>

#include <vcl/button.hxx>

//========================================================================
class SvxFmAbsRecWin : public NumericField
{
    SfxToolBoxControl*  m_pController;
        // for invalidating our content whe losing the focus
public:
    SvxFmAbsRecWin( Window* _pParent, SfxToolBoxControl* _pController );
    ~SvxFmAbsRecWin();

    virtual void KeyInput( const KeyEvent& rKeyEvt );
    virtual void LoseFocus();

protected:
    virtual void FirePosition( sal_Bool _bForce );
};


//========================================================================
class SvxFmConfigWin : public SfxPopupWindow
{
private:
    DECL_LINK( TbxSelectHdl, ToolBox* );

public:
    SvxFmConfigWin( USHORT nId, ResId aRIdWin, ResId aRIdTbx );
    ~SvxFmConfigWin();

    void            Update();
    virtual void    PopupModeEnd();
};


//========================================================================
class SvxFmTbxCtlConfig : public SfxToolBoxControl
{
private:
    UINT16 nLastSlot;

protected:
    using SfxToolBoxControl::Select;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlConfig( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxFmTbxCtlConfig() {}

    virtual void                Select( USHORT nModifier );
    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};

//========================================================================
class FixedText;
class SvxFmTbxCtlAbsRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlAbsRec( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxFmTbxCtlAbsRec();

    virtual Window* CreateItemWindow( Window* pParent );

    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
};

//========================================================================
class SvxFmTbxCtlRecText : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecText( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxFmTbxCtlRecText();

    virtual Window* CreateItemWindow( Window* pParent );
};

//========================================================================
class SvxFmTbxCtlRecFromText : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecFromText( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxFmTbxCtlRecFromText();

    virtual Window* CreateItemWindow( Window* pParent );
};

//========================================================================
class SvxFmTbxCtlRecTotal : public SfxToolBoxControl
{
    FixedText* pFixedText;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecTotal( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~SvxFmTbxCtlRecTotal();

    virtual Window* CreateItemWindow( Window* pParent );
    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
};

//========================================================================
class SvxFmTbxNextRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFmTbxNextRec( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
};

//========================================================================
class SvxFmTbxPrevRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFmTbxPrevRec( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
