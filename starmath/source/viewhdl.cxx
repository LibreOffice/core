/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewhdl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:57:21 $
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
#include "precompiled_starmath.hxx"


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
            const lang::EventObject& rEventObject ) throw ( ::com::sun::star::uno::RuntimeException)
;//                         throw( UNO_NMSPC::RuntimeException );

    // XClipboardListener
    virtual void SAL_CALL changedContents(
            const datatransfer::clipboard::ClipboardEvent& rEventObject ) throw ( ::com::sun::star::uno::RuntimeException)
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
                                    const lang::EventObject& rEventObject ) throw ( ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SmClipboardChangeListener::changedContents(
                            const ClipboardEvent& rEventObject ) throw ( ::com::sun::star::uno::RuntimeException)
{
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if( pView )
    {
        {
            TransferableDataHelper aDataHelper( rEventObject.Contents );
            sal_Bool bHasTransferable = aDataHelper.GetTransferable().is();

            //! according to CD the above call to GetTransferable may create a (new)
            //! message loop and thus result in re-entrant code.
            //! Thus it was suggested to check 'pView' here again.
            if (pView)
            {
                pView->bPasteState = bHasTransferable &&
                    ( aDataHelper.HasFormat( FORMAT_STRING ) ||
                    aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBEDDED_OBJ ) ||
                    (aDataHelper.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR )
                        && aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE )));
            }
        }

        if (pView)
        {
            SfxBindings& rBind = pView->GetViewFrame()->GetBindings();
            rBind.Invalidate( SID_PASTE );
        }
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
    // AddRemoveListener and ViewDestroyed below should be an
    // atomic operation and must not be 'interrupted' by other code
    const ::vos::OGuard aGuard( Application::GetSolarMutex() );

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



