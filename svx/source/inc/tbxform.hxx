/*************************************************************************
 *
 *  $RCSfile: tbxform.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_TBXFORM_HXX
#define _SVX_TBXFORM_HXX

#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif

#ifndef _SFXTBXMGR_HXX //autogen
#include <sfx2/tbxmgr.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

//========================================================================

class FmInputRecordNoDialog : public ModalDialog
{
public:
    FixedText       m_aLabel;
    NumericField    m_aRecordNo;
    OKButton        m_aOk;
    CancelButton    m_aCancel;

public:
    FmInputRecordNoDialog(Window * pParent);

    void SetValue(double dNew) { m_aRecordNo.SetValue(dNew); }
    long GetValue() const { return m_aRecordNo.GetValue(); }
};

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
    virtual void FirePosition();
};


//========================================================================
class SvxFmConfigWin : public SfxPopupWindow
{
private:
    SfxToolBoxManager aTbx;
    Link aSelectLink;

    DECL_LINK( TbxSelectHdl, ToolBox* );

public:
    SvxFmConfigWin( USHORT nId, ResId aRIdWin, ResId aRIdTbx, SfxBindings& rBindings );
    ~SvxFmConfigWin();

    void            StartSelection()
                        { aTbx.GetToolBox().StartSelection(); }
    void            Update();
    virtual void    PopupModeEnd();
};


//========================================================================
class SvxFmTbxCtlConfig : public SfxToolBoxControl
{
private:
    UINT16 nLastSlot;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlConfig( USHORT nId, ToolBox& rTbx, SfxBindings& );
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

    SvxFmTbxCtlAbsRec( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
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

    SvxFmTbxCtlRecText( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
    ~SvxFmTbxCtlRecText();

    virtual Window* CreateItemWindow( Window* pParent );
};

//========================================================================
class SvxFmTbxCtlRecFromText : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecFromText( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
    ~SvxFmTbxCtlRecFromText();

    virtual Window* CreateItemWindow( Window* pParent );
};

//========================================================================
class SvxFmTbxCtlRecTotal : public SfxToolBoxControl
{
    FixedText* pFixedText;

public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlRecTotal( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
    ~SvxFmTbxCtlRecTotal();

    virtual Window* CreateItemWindow( Window* pParent );
    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
};

/*
//========================================================================
class SvxFmTbxCtlFilterText : public SfxToolBoxControl
{
    FixedText* pFixedText;
    XubString aText;
public:
    SFX_DECL_TOOLBOX_CONTROL();

    SvxFmTbxCtlFilterText( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
    ~SvxFmTbxCtlFilterText();

    virtual Window* CreateItemWindow( Window* pParent );
    virtual void    StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState );
};
*/

//========================================================================
class SvxFmTbxNextRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFmTbxNextRec( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
};

//========================================================================
class SvxFmTbxPrevRec : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    SvxFmTbxPrevRec( USHORT nId, ToolBox& rTbx, SfxBindings& rBindings );
};


#endif

