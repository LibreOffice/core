/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: grfshex.cxx,v $
 * $Revision: 1.20 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <docary.hxx>
#ifndef _GRFSH_HXX
#include <grfsh.hxx>
#endif
#include <wrtsh.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <docary.hxx>
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif
#include <viewopt.hxx>
#include <swundo.hxx>
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif
#include <caption.hxx>
#define _SVSTDARR_STRINGSSORTDTOR
#include <svl/svstdarr.hxx>
#include <svtools/filter.hxx>
#include <svx/impgrf.hxx>
#include <svx/htmlmode.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <frmfmt.hxx>
#include <frmmgr.hxx>
#include <vcl/msgbox.hxx>
#ifndef _SVX_SVDOMEDIA_HXX
#include <svx/svdomedia.hxx>
#endif
#ifndef _SVX_SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif
#ifndef _SVX_SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
#include <SwStyleNameMapper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif

#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <avmedia/mediawindow.hxx>
#include <vcl/svapp.hxx>

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
