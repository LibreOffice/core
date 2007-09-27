/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tmplctrl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:48:57 $
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
#include "precompiled_sw.hxx"

// include ---------------------------------------------------------------


#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif

#include "wrtsh.hxx"
#include "view.hxx"
#include "swmodule.hxx"
#include "cmdid.h"
#include "docsh.hxx"
#include "tmplctrl.hxx"


// STATIC DATA -----------------------------------------------------------


SFX_IMPL_STATUSBAR_CONTROL( SwTemplateControl, SfxStringItem );

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

// class SvxZoomStatusBarControl ------------------------------------------

SwTemplateControl::SwTemplateControl( USHORT _nSlotId,
                                      USHORT _nId,
                                      StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb )
{
}

// -----------------------------------------------------------------------

SwTemplateControl::~SwTemplateControl()
{
}

// -----------------------------------------------------------------------

void SwTemplateControl::StateChanged(
    USHORT /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if( eState != SFX_ITEM_AVAILABLE || pState->ISA( SfxVoidItem ) )
        GetStatusBar().SetItemText( GetId(), String() );
    else if ( pState->ISA( SfxStringItem ) )
    {
        sTemplate = ((SfxStringItem*)pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), sTemplate );
    }
}

// -----------------------------------------------------------------------

void SwTemplateControl::Paint( const UserDrawEvent&  )
{
    GetStatusBar().SetItemText( GetId(), sTemplate );
}

// -----------------------------------------------------------------------

void SwTemplateControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU &&
            GetStatusBar().GetItemText( GetId() ).Len() )
    {
        CaptureMouse();
        TemplatePopup_Impl aPop;
        {
            SwView* pView = ::GetActiveView();
            SwWrtShell* pWrtShell;
            if( pView && 0 != (pWrtShell = pView->GetWrtShellPtr()) &&
                !pWrtShell->SwCrsrShell::HasSelection()&&
                !pWrtShell->IsSelFrmMode() &&
                !pWrtShell->IsObjSelected())
            {
                SfxStyleSheetBasePool* pPool = pView->GetDocShell()->
                                                            GetStyleSheetPool();
                pPool->SetSearchMask(SFX_STYLE_FAMILY_PAGE, SFXSTYLEBIT_ALL);
                if( pPool->Count() > 1 )
                {
                    USHORT nCount = 0;
                    SfxStyleSheetBase* pStyle = pPool->First();
                    while( pStyle )
                    {
                        nCount++;
                        aPop.InsertItem( nCount, pStyle->GetName() );
                        pStyle = pPool->Next();
                    }

                    aPop.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel());
                    USHORT nCurrId = aPop.GetCurId();
                    if( nCurrId != USHRT_MAX)
                    {
                        // sieht etwas umstaendlich aus, anders geht's aber nicht
                        pStyle = pPool->operator[]( nCurrId - 1 );
                        SfxStringItem aStyle( FN_SET_PAGE_STYLE, pStyle->GetName() );
                        pWrtShell->GetView().GetViewFrame()->GetDispatcher()->Execute(
                                    FN_SET_PAGE_STYLE,
                                    SFX_CALLMODE_SLOT|SFX_CALLMODE_RECORD,
                                    &aStyle, 0L );
                    }
                }
            }
        }
        ReleaseMouse();
    }
}



