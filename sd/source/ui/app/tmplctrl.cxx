/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tmplctrl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:46:24 $
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
#include "precompiled_sd.hxx"

// include ---------------------------------------------------------------


#include <vcl/menu.hxx>
#include <vcl/status.hxx>
#include <svtools/style.hxx>
#include <svtools/stritem.hxx>
#include <sfx2/dispatch.hxx>

#include "tmplctrl.hxx"
#include "ViewShellBase.hxx"
#include "drawdoc.hxx"
#include "sdattr.hrc"
#include "app.hrc"

SFX_IMPL_STATUSBAR_CONTROL( SdTemplateControl, SfxStringItem );

// class TemplatePopup_Impl --------------------------------------------------

class TemplatePopup_Impl : public PopupMenu
{
public:
    TemplatePopup_Impl();

    USHORT          GetCurId() const { return nCurId; }

private:
    USHORT          nCurId;

    virtual void    Select();
};

// -----------------------------------------------------------------------

TemplatePopup_Impl::TemplatePopup_Impl() :
    PopupMenu(),
    nCurId(USHRT_MAX)
{
}

// -----------------------------------------------------------------------

void TemplatePopup_Impl::Select()
{
    nCurId = GetCurItemId();
}

// class SdTemplateControl ------------------------------------------

SdTemplateControl::SdTemplateControl( USHORT _nSlotId,
                                      USHORT _nId,
                                      StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb )
{
}

// -----------------------------------------------------------------------

SdTemplateControl::~SdTemplateControl()
{
}

// -----------------------------------------------------------------------

void SdTemplateControl::StateChanged(
    USHORT /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if( eState != SFX_ITEM_AVAILABLE || pState->ISA( SfxVoidItem ) )
        GetStatusBar().SetItemText( GetId(), String() );
    else if ( pState->ISA( SfxStringItem ) )
    {
        msTemplate = ((SfxStringItem*)pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), msTemplate );
    }
}

// -----------------------------------------------------------------------

void SdTemplateControl::Paint( const UserDrawEvent&  )
{
    GetStatusBar().SetItemText( GetId(), msTemplate );
}

// -----------------------------------------------------------------------

void SdTemplateControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU && GetStatusBar().GetItemText( GetId() ).Len() )
    {
        SfxViewFrame* pViewFrame = SfxViewFrame::Current();

        sd::ViewShellBase* pViewShellBase = sd::ViewShellBase::GetViewShellBase( pViewFrame );
        if( !pViewShellBase )
            return;

        SdDrawDocument* pDoc = pViewShellBase->GetDocument();
        if( !pDoc )
            return;

        CaptureMouse();
        TemplatePopup_Impl aPop;
        {
            const USHORT nMasterCount = pDoc->GetMasterSdPageCount(PK_STANDARD);

            USHORT nCount = 0;
            for( USHORT nPage = 0; nPage < nMasterCount; ++nPage )
            {
                SdPage* pMaster = pDoc->GetMasterSdPage(nPage, PK_STANDARD);
                if( pMaster )
                    aPop.InsertItem( ++nCount, pMaster->GetName() );
            }
            aPop.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel());

            USHORT nCurrId = aPop.GetCurId()-1;
            if( nCurrId < nMasterCount )
            {
                SdPage* pMaster = pDoc->GetMasterSdPage(nCurrId, PK_STANDARD);
                SfxStringItem aStyle( ATTR_PRESLAYOUT_NAME, pMaster->GetName() );
                pViewFrame->GetDispatcher()->Execute(SID_PRESENTATION_LAYOUT,SFX_CALLMODE_SLOT, &aStyle, 0L );
                pViewFrame->GetBindings().Invalidate(SID_PRESENTATION_LAYOUT);
                pViewFrame->GetBindings().Invalidate(SID_STATUS_LAYOUT);
            }
        }

        ReleaseMouse();
    }
}



