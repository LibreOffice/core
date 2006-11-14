/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbxdraw.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 12:57:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _TBXDRAW_HXX
#define _TBXDRAW_HXX

// include ---------------------------------------------------------------

#ifndef _SFXTBXCTRL_HXX //autogen
#include <sfx2/tbxctrl.hxx>
#endif

// class SvxPopupWindowTbxMgr --------------------------------------------

class SvxPopupWindowTbxMgr : public SfxPopupWindow
{
    void*               pNiemalsDenPointerAufDieErzeugendeToolBoxMerken; // MI!
    ResId               aRIdWinTemp;
    ResId               aRIdTbxTemp;

    Link                aTbxClickHdl;


    DECL_LINK( TbxSelectHdl, ToolBox*);
    DECL_LINK( TbxClickHdl, ToolBox*);

protected:
    using SfxPopupWindow::StateChanged;

public:
    SvxPopupWindowTbxMgr(   USHORT nId,
                            SfxToolBoxControl* pTbxCtl,
                            ResId aRIdWin,
                            ResId aRIdTbx );
    SvxPopupWindowTbxMgr(   USHORT nId,
                            WindowAlign eAlign,
                            ResId aRIdWin,
                            ResId aRIdTbx );
    ~SvxPopupWindowTbxMgr();

    virtual void                StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pState);
    virtual SfxPopupWindow*     Clone() const;
    virtual void                PopupModeEnd();

    void                        StartSelection();

    // toolbox click or execute may chage the default selection in the tools toolbox
    virtual void                UserEvent( ULONG nEvent, void* pEventData );
};


#endif      // _TBX_DRAW_HXX

