/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbxdrctl.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 13:22:32 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

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

#include "dialmgr.hxx"
#include "dialogs.hrc"

#include "tbxctl.hxx"
#include "tbxdraw.hxx"
#include "tbxcolor.hxx"
#include "tbxdraw.hrc"

#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif

SFX_IMPL_TOOLBOX_CONTROL(SvxTbxCtlDraw, SfxAllEnumItem);

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

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
        }

        GetToolBox().CheckItem( GetId(), bCheck );
    }
}

// -----------------------------------------------------------------------

void SvxTbxCtlDraw::Select( BOOL )
{
    toggleToolbox();
}

