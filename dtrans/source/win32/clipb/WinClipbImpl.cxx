/*************************************************************************
 *
 *  $RCSfile: WinClipbImpl.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 14:37:20 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _WINCLIPBIMPL_HXX_
#include "WinClipbImpl.hxx"
#endif

#ifndef _DTOBJFACTORY_HXX_
#include "..\..\inc\DtObjFactory.hxx"
#endif

#ifndef _APNDATAOBJECT_HXX_
#include "..\dtobj\APNDataObject.hxx"
#endif

#ifndef _WINCLIPBOARD_HXX_
#include "WinClipboard.hxx"
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_RENDERINGCAPABILITIES_HPP_
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#endif

#ifndef _XNOTIFYINGDATAOBJECT_HXX_
#include "..\dtobj\XNotifyingDataObject.hxx"
#endif

#include <windows.h>
#include <ole2.h>
#include <objidl.h>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace rtl;
using namespace osl;
using namespace std;
using namespace cppu;

using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::datatransfer::clipboard::RenderingCapabilities;

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

// definition of static members
CWinClipbImpl* CWinClipbImpl::s_pCWinClipbImpl = NULL;
osl::Mutex     CWinClipbImpl::s_aMutex;

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CWinClipbImpl::CWinClipbImpl( const OUString& aClipboardName, CWinClipboard* theWinClipboard ) :
    m_itsName( aClipboardName ),
    m_pWinClipboard( theWinClipboard ),
    m_pCurrentClipContent( NULL )
{
    OSL_ASSERT( NULL != m_pWinClipboard );

    // necessary to reassociate from
    // the static callback function
    s_pCWinClipbImpl = this;
    registerClipboardViewer( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CWinClipbImpl::~CWinClipbImpl( )
{
    ClearableMutexGuard aGuard( s_aMutex );
    s_pCWinClipbImpl = NULL;
    aGuard.clear( );

    unregisterClipboardViewer( );
}

//------------------------------------------------------------------------
// getContent
//------------------------------------------------------------------------

Reference< XTransferable > SAL_CALL CWinClipbImpl::getContents( ) throw( RuntimeException )
{
    // use the shotcut or create a transferable from
    // system clipboard
    ClearableMutexGuard aGuard( m_ClipContentMutex );

    if ( NULL != m_pCurrentClipContent )
    {
        return m_pCurrentClipContent->m_XTransferable;
    }

    // release the mutex, so that the variable may be
    // changed by other threads
    aGuard.clear( );

    Reference< XTransferable > rClipContent;

    // get the current dataobject from clipboard
    IDataObjectPtr pIDataObject;
    HRESULT hr = m_MtaOleClipboard.getClipboard( &pIDataObject );

    if ( SUCCEEDED( hr ) )
    {
        // create an apartment neutral dataobject and initialize it with a
        // com smart pointer to the IDataObject from clipboard
        IDataObjectPtr pIDo( new CAPNDataObject( pIDataObject ) );

        CDTransObjFactory objFactory;

        // remeber pIDo destroys itself due to the smart pointer
        rClipContent = objFactory.createTransferableFromDataObj( m_pWinClipboard->m_SrvMgr, pIDo );
    }

    return rClipContent;
}

//------------------------------------------------------------------------
// setContent
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::setContents(
    const Reference< XTransferable >& xTransferable,
    const Reference< XClipboardOwner >& xClipboardOwner )
    throw( RuntimeException )
{
    CDTransObjFactory objFactory;
    IDataObjectPtr    pIDataObj;

    if ( xTransferable.is( ) )
    {
        ClearableMutexGuard aGuard( m_ClipContentMutex );

        m_pCurrentClipContent = new CXNotifyingDataObject(
            objFactory.createDataObjFromTransferable( m_pWinClipboard->m_SrvMgr , xTransferable ),
            xTransferable,
            xClipboardOwner,
            this );

        aGuard.clear( );

        pIDataObj = IDataObjectPtr( m_pCurrentClipContent );
    }

    m_MtaOleClipboard.setClipboard( pIDataObj );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

OUString SAL_CALL CWinClipbImpl::getName(  ) throw( RuntimeException )
{
    return m_itsName;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Int8 SAL_CALL CWinClipbImpl::getRenderingCapabilities(  ) throw( RuntimeException )
{
    return ( Delayed | Persistant );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::flushClipboard( ) throw( RuntimeException )
{
    // sollte eigentlich hier stehen: ClearableMutexGuard aGuard( m_ClipContentMutex );
    // geht aber nicht, da FlushClipboard zurückruft und das DataObject
    // freigibt und damit würde es einen Deadlock in onReleaseDataObject geben
    // FlushClipboard muß synchron sein, damit das runterfahren ggf. erst weitergeht,
    // wenn alle Clipboard-Formate gerendert wurden
    // die Abfrage ist nötig, damit nur geflusht wird, wenn wir wirklich Clipboardowner
    // sind (ich weiss nicht genau was passiert, wenn man flusht und nicht Clipboard
    // owner ist).
    // eventuell kann man aber die Abfrage in den Clipboard STA Thread verlagern, indem
    // man sich dort das DataObject merkt und vor dem flushen OleIsCurrentClipboard ruft

    if ( NULL != m_pCurrentClipContent )
        m_MtaOleClipboard.flushClipboard( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::registerClipboardViewer( )
{
    m_MtaOleClipboard.registerClipViewer( CWinClipbImpl::onClipboardContentChanged );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::unregisterClipboardViewer( )
{
    m_MtaOleClipboard.registerClipViewer( NULL );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::dispose() throw( RuntimeException )
{
    OSL_ENSURE( !m_pCurrentClipContent, "Clipboard was not flushed before shutdown!" );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void WINAPI CWinClipbImpl::onClipboardContentChanged( void )
{
    MutexGuard aGuard( s_aMutex );

    // reassocition to instance through static member
    if ( NULL != s_pCWinClipbImpl )
        s_pCWinClipbImpl->m_pWinClipboard->notifyAllClipboardListener( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::onReleaseDataObject( CXNotifyingDataObject* theCaller )
{
    OSL_ASSERT( NULL != theCaller );

    if ( theCaller )
        theCaller->lostOwnership( );

    // if the current caller is the one we currently
    // hold, then set it to NULL because an external
    // source must be the clipboardowner now
    MutexGuard aGuard( m_ClipContentMutex );

    if ( m_pCurrentClipContent == theCaller )
        m_pCurrentClipContent = NULL;
}
