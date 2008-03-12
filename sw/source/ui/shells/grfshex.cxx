/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: grfshex.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:54:12 $
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



#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _GRFSH_HXX
#include <grfsh.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif
#ifndef _CAPTION_HXX
#include <caption.hxx>
#endif
#define _SVSTDARR_STRINGSSORTDTOR
#include <svtools/svstdarr.hxx>
#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX
#include <svx/impgrf.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_SVDOMEDIA_HXX
#include <svx/svdomedia.hxx>
#endif
#ifndef _SVX_SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif
#ifndef _SVX_SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_LISTBOXCONTROLACTIONS_HPP_
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#endif
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif

#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/stritem.hxx>
#include <avmedia/mediawindow.hxx>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

// -> #111827#
#include <SwRewriter.hxx>
#include <undobj.hxx>
#include <comcore.hrc>
// <- #111827#

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::sfx2;
using ::rtl::OUString;

bool SwTextShell::InsertMediaDlg( SfxRequest& rReq )
{
    ::rtl::OUString     aURL;
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    Window*             pWindow = &GetView().GetViewFrame()->GetWindow();
    bool                bAPI = false, bRet = false;

    if( pReqArgs )
    {
        const SfxStringItem* pStringItem = PTR_CAST( SfxStringItem, &pReqArgs->Get( rReq.GetSlot() ) );

        if( pStringItem )
        {
            aURL = pStringItem->GetValue();
            bAPI = aURL.getLength();
        }
    }

    if( bAPI || ::avmedia::MediaWindow::executeMediaURLDialog( pWindow, aURL ) )
    {
        Size aPrefSize;

        if( pWindow )
            pWindow->EnterWait();

        if( !::avmedia::MediaWindow::isMediaURL( aURL, true, &aPrefSize ) )
        {
            if( pWindow )
                pWindow->LeaveWait();

            if( !bAPI )
                ::avmedia::MediaWindow::executeFormatErrorBox( pWindow );
        }
        else
        {
            SwWrtShell& rSh = GetShell();

            if( !rSh.HasDrawView() )
                rSh.MakeDrawView();

            Size            aDocSz( rSh.GetDocSize() );
               const SwRect&    rVisArea = rSh.VisArea();
            Point           aPos( rVisArea.Center() );
            Size            aSize;

            if( rVisArea.Width() > aDocSz.Width())
                aPos.X() = aDocSz.Width() / 2 + rVisArea.Left();

            if(rVisArea.Height() > aDocSz.Height())
                aPos.Y() = aDocSz.Height() / 2 + rVisArea.Top();

            if( aPrefSize.Width() && aPrefSize.Height() )
            {
                if( pWindow )
                    aSize = pWindow->PixelToLogic( aPrefSize, MAP_TWIP );
                else
                    aSize = Application::GetDefaultDevice()->PixelToLogic( aPrefSize, MAP_TWIP );
            }
            else
                aSize = Size( 2835, 2835 );

            SdrMediaObj* pObj = new SdrMediaObj( Rectangle( aPos, aSize ) );

            pObj->setURL( aURL );
            rSh.EnterStdMode();
            rSh.SwFEShell::Insert( *pObj, 0, 0, &aPos );
            bRet = true;

            if( pWindow )
                pWindow->LeaveWait();
        }
    }

    return bRet;
}
