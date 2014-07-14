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
#include "precompiled_sw.hxx"

#include <docary.hxx>
#include <grfsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docary.hxx>
#include <textsh.hxx>
#include <viewopt.hxx>
#include <swundo.hxx>
#include <shells.hrc>
#include <caption.hxx>
#define _SVSTDARR_STRINGSSORTDTOR
#include <svl/svstdarr.hxx>
#include <svtools/filter.hxx>
#include <svx/htmlmode.hxx>
#include <docsh.hxx>
#include <frmfmt.hxx>
#include <frmmgr.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <SwStyleNameMapper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <poolfmt.hrc>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <avmedia/mediawindow.hxx>
#include <vcl/svapp.hxx>
#include <doc.hxx>
#include <svx/fmmodel.hxx>
#include <SwRewriter.hxx>
#include <comcore.hrc>
#include <drawdoc.hxx>

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
        const SfxStringItem* pStringItem = dynamic_cast< const SfxStringItem* >( &pReqArgs->Get( rReq.GetSlot() ) );

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

            SdrMediaObj* pObj = new SdrMediaObj(
                *rSh.SwFEShell::GetDoc()->GetOrCreateDrawModel(),
                basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aSize.getWidth(), aSize.getHeight(),
                    aPos.X(), aPos.Y()));

            pObj->setURL( aURL );
            rSh.EnterStdMode();
            rSh.SwFEShell::InsertDrawObj( *pObj, aPos );
            bRet = true;

            if( pWindow )
                pWindow->LeaveWait();
        }
    }

    return bRet;
}
