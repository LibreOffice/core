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
#ifndef INCLUDED_SVX_SOURCE_INC_TBXFORM_HXX
#define INCLUDED_SVX_SOURCE_INC_TBXFORM_HXX

#include <sfx2/tbxctrl.hxx>
#include <vcl/field.hxx>
#include <vcl/dialog.hxx>

#include <vcl/button.hxx>


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



class SvxFmConfigWin : public SfxPopupWindow
{
private:
    DECL_LINK( TbxSelectHdl, ToolBox* );

public:
    SvxFmConfigWin( sal_uInt16 nId, ResId aRIdWin, ResId aRIdTbx );
    ~SvxFmConfigWin();

    void            Update();
    virtual void    PopupModeEnd();
};



class SvxFmTbxCtlConfig : public SfxToolBoxControl
{
private:
    sal_uInt16 nLastSlot;

protected:
    using SfxToolBoxControl::Select;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlConfig( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxFmTbxCtlConfig() {}

    virtual void                Select( sal_uInt16 nModifier );
    virtual void                StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState );
    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();
};


class FixedText;
class SvxFmTbxCtlAbsRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlAbsRec( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxFmTbxCtlAbsRec();

    virtual Window* CreateItemWindow( Window* pParent );

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
};


class SvxFmTbxCtlRecText : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecText( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxFmTbxCtlRecText();

    virtual Window* CreateItemWindow( Window* pParent );
};


class SvxFmTbxCtlRecFromText : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecFromText( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxFmTbxCtlRecFromText();

    virtual Window* CreateItemWindow( Window* pParent );
};


class SvxFmTbxCtlRecTotal : public SfxToolBoxControl
{
    FixedText* pFixedText;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecTotal( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~SvxFmTbxCtlRecTotal();

    virtual Window* CreateItemWindow( Window* pParent );
    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
};


class SvxFmTbxNextRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFmTbxNextRec( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};


class SvxFmTbxPrevRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFmTbxPrevRec( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
