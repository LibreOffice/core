/*************************************************************************
 *
 *  $RCSfile: tbxdrctl.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-18 15:35:54 $
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

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#include <tools/ref.hxx>
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _AEITEM_HXX
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#include <sfx2/imagemgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/toolbox.hxx>

#pragma hdrstop

#include "dialmgr.hxx"
#include "dialogs.hrc"

#include "tbxctl.hxx"
#include "tbxdraw.hxx"
#include "tbxcolor.hxx"
#include "tbxdraw.hrc"

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <drafts/com/sun/star/frame/XLayoutManager.hpp>
#endif

SFX_IMPL_TOOLBOX_CONTROL(SvxTbxCtlDraw, SfxAllEnumItem);

using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::frame;

// -----------------------------------------------------------------------

SvxTbxCtlDraw::SvxTbxCtlDraw( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :

    SfxToolBoxControl( nSlotId, nId, rTbx ),

    m_sToolboxName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/drawbar" ) )

{
    rTbx.SetItemBits( nId, TIB_CHECKABLE | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

// -----------------------------------------------------------------------

void SvxTbxCtlDraw::StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), ( eState != SFX_ITEM_DISABLED ) );
    SfxToolBoxControl::StateChanged( nSID, eState, pState );

    Reference< XLayoutManager > xLayoutMgr = getLayoutManager();
    if ( xLayoutMgr.is() )
        GetToolBox().CheckItem(
            GetId(), xLayoutMgr->isElementVisible( m_sToolboxName ) != sal_False );
}

// -----------------------------------------------------------------------

SfxPopupWindowType SvxTbxCtlDraw::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

void SvxTbxCtlDraw::toggleToolbox()
{
    Reference< XLayoutManager > xLayoutMgr = getLayoutManager();
    if ( xLayoutMgr.is() )
    {
        BOOL bCheck = FALSE;
        if ( xLayoutMgr->isElementVisible( m_sToolboxName ) )
        {
            xLayoutMgr->hideElement( m_sToolboxName );
            xLayoutMgr->destroyElement( m_sToolboxName );
        }
        else
        {
            bCheck = TRUE;
            xLayoutMgr->createElement( m_sToolboxName );
            xLayoutMgr->showElement( m_sToolboxName );
            ::com::sun::star::awt::Point aPos;
            xLayoutMgr->dockWindow( m_sToolboxName,
                ::drafts::com::sun::star::ui::DockingArea_DOCKINGAREA_BOTTOM, aPos );
        }

        GetToolBox().CheckItem( GetId(), bCheck );
    }
}

// -----------------------------------------------------------------------

void SvxTbxCtlDraw::Select( BOOL bMod1 )
{
    toggleToolbox();
}

