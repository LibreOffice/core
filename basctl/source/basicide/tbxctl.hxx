/*************************************************************************
 *
 *  $RCSfile: tbxctl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:37 $
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
#ifndef _BASIDE_TBXCTL_HXX
#define _BASIDE_TBXCTL_HXX


#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif

#ifndef _SFXTBXMGR_HXX
#include <sfx2/tbxmgr.hxx>
#endif

#ifdef _BASIDE_POPUPWINDOWTBX

// class PopupWindowTbx --------------------------------------------------

class PopupWindowTbx : public SfxPopupWindow
{
private:
    SfxToolBoxManager   aTbx;
    Link                aSelectLink;

    DECL_LINK( SelectHdl, void* );

public:
    PopupWindowTbx( USHORT nId, ToolBoxAlign eAlign,
                    ResId aRIdWin, ResId aRIdTbx, SfxBindings& rBind );
    ~PopupWindowTbx();

    void                    StartSelection()
                                { aTbx.GetToolBox().StartSelection(); }
    void                    Update();

    virtual SfxPopupWindow* Clone() const;
    virtual void            PopupModeEnd();
};
#endif

//-------------------
// class TbxControls
//-------------------
class TbxControls : public SfxToolBoxControl
{
    USHORT                  nLastSlot;

protected:
    virtual void            StateChanged( USHORT nSID, SfxItemState eState,
                                          const SfxPoolItem* pState );
public:
    SFX_DECL_TOOLBOX_CONTROL();

    TbxControls(USHORT nId, ToolBox& rTbx, SfxBindings& rBind );
    ~TbxControls() {}

    virtual SfxPopupWindowType  GetPopupWindowType() const;
    virtual SfxPopupWindow*     CreatePopupWindow();

    void                        Select( USHORT nModifier );
};


#endif // _BASIDE_TBXCTL_HXX

