/*************************************************************************
 *
 *  $RCSfile: viewhdl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tl $ $Date: 2001-07-19 08:13:04 $
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

#pragma hdrstop

#include <view.hxx>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDLISTENER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDNOTIFIER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer::clipboard;

// --------------------------- Clipboard EventListener ------------------

class SmClipboardChangeListener : public ::cppu::WeakImplHelper1<
    ::com::sun::star::datatransfer::clipboard::XClipboardListener >
{
    SmViewShell*    pView;

    // XEventListener
    virtual void SAL_CALL disposing(
            const lang::EventObject& rEventObject )
;//                         throw( UNO_NMSPC::RuntimeException );

    // XClipboardListener
    virtual void SAL_CALL changedContents(
            const datatransfer::clipboard::ClipboardEvent& rEventObject )
;//                                     throw( UNO_NMSPC::RuntimeException );

public:
    SmClipboardChangeListener( SmViewShell& rView ) : pView( &rView ) {}
    virtual ~SmClipboardChangeListener();

    void ViewDestroyed() { pView = 0; }

    void AddRemoveListener( BOOL bAdd );
};



// ------------------------- SmClipboardChangeListener ---------------------

SmClipboardChangeListener::~SmClipboardChangeListener()
{
}

void SAL_CALL SmClipboardChangeListener::disposing(
                                    const lang::EventObject& rEventObject )
{
}

void SAL_CALL SmClipboardChangeListener::changedContents(
                            const ClipboardEvent& rEventObject )
{
    if( pView )
    {
        {
        const ::vos::OGuard aGuard( Application::GetSolarMutex() );

        TransferableDataHelper aDataHelper( rEventObject.Contents );
        pView->bPasteState = aDataHelper.GetTransferable().is() &&
            ( aDataHelper.HasFormat( FORMAT_STRING ) ||
            aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBEDDED_OBJ ) ||
            (aDataHelper.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR )
                && aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE )));
        }

        SfxBindings& rBind = pView->GetViewFrame()->GetBindings();
        rBind.Invalidate( SID_PASTE );
    }
}

void SmClipboardChangeListener::AddRemoveListener( BOOL bAdd )
{
    try
    {
        do {
            Window* pW;
            Reference< XClipboard > xClipboard;
            if( pView && 0 != ( pW = pView->GetEditWindow()) )
                xClipboard = pW->GetClipboard();
            else
            {
                DBG_ASSERT( pView, "no SmViewShell" );

                Reference< lang::XMultiServiceFactory > xFact(
                                    ::comphelper::getProcessServiceFactory() );
                if( !xFact.is() )
                    break;
                xClipboard = Reference< XClipboard > ( xFact->createInstance(
                    ::rtl::OUString::createFromAscii(
                        "com.sun.star.datatransfer.clipboard.SystemClipboard" ) ),
                    UNO_QUERY );
            }

            if( !xClipboard.is() )
                break;

            Reference< XClipboardNotifier > xClpbrdNtfr( xClipboard, UNO_QUERY );
            if( xClpbrdNtfr.is() )
            {
                Reference< XClipboardListener > xClipEvtLstnr( this );
                if( bAdd )
                    xClpbrdNtfr->addClipboardListener( xClipEvtLstnr );
                else
                    xClpbrdNtfr->removeClipboardListener( xClipEvtLstnr );
            }
        }  while ( FALSE );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// ------------------------- SmViewShell ---------------------

void SmViewShell::AddRemoveClipboardListener( BOOL bAdd )
{
    if( bAdd && !xClipEvtLstnr.is() )
    {
        xClipEvtLstnr = pClipEvtLstnr = new SmClipboardChangeListener( *this );
        pClipEvtLstnr->AddRemoveListener( TRUE );
    }
    else if( !bAdd && xClipEvtLstnr.is() )
    {
        pClipEvtLstnr->AddRemoveListener( FALSE );
        pClipEvtLstnr->ViewDestroyed();
    }
}



