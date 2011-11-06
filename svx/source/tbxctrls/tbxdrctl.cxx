/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#include <tools/ref.hxx>
#include <tools/shl.hxx>
#include <svl/aeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/imagemgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/toolbox.hxx>

#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#include "svx/tbxctl.hxx"
#include "svx/tbxdraw.hxx"
#include "svx/tbxcolor.hxx"
#include "tbxdraw.hrc"
#include <com/sun/star/frame/XLayoutManager.hpp>

SFX_IMPL_TOOLBOX_CONTROL(SvxTbxCtlDraw, SfxAllEnumItem);

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

// -----------------------------------------------------------------------

SvxTbxCtlDraw::SvxTbxCtlDraw( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :

    SfxToolBoxControl( nSlotId, nId, rTbx ),

    m_sToolboxName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/drawbar" ) )

{
    rTbx.SetItemBits( nId, TIB_CHECKABLE | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

// -----------------------------------------------------------------------

void SvxTbxCtlDraw::StateChanged( sal_uInt16 nSID, SfxItemState eState,
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
        sal_Bool bCheck = sal_False;
        if ( xLayoutMgr->isElementVisible( m_sToolboxName ) )
        {
            xLayoutMgr->hideElement( m_sToolboxName );
            xLayoutMgr->destroyElement( m_sToolboxName );
        }
        else
        {
            bCheck = sal_True;
            xLayoutMgr->createElement( m_sToolboxName );
            xLayoutMgr->showElement( m_sToolboxName );
        }

        GetToolBox().CheckItem( GetId(), bCheck );
    }
}

// -----------------------------------------------------------------------

void SvxTbxCtlDraw::Select( sal_Bool )
{
    toggleToolbox();
}

