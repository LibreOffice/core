/*************************************************************************
 *
 *  $RCSfile: WinClipbImpl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-02 15:37:30 $
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

#ifndef _MTAOLECLIPB_H_
#include <systools\win32\MtaOleClipb.h>
#endif

#ifndef _APNDATAOBJECT_HXX_
#include "APNDataObject.hxx"
#endif

#ifndef _WINCLIPBOARD_HXX_
#include "WinClipboard.hxx"
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_RENDERINGCAPABILITIES_HPP_
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
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

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CWinClipbImpl::CWinClipbImpl( const OUString& aClipboardName, CWinClipboard* theWinClipboard ) :
    m_itsName( aClipboardName ),
    m_pClipboardContent( NULL ),
    m_pWinClipboard( theWinClipboard ),
    m_bInDispose( sal_False )
{
    OSL_ASSERT( NULL != m_pWinClipboard );

    // necessary to reassociate from
    // the static callback function
    s_pCWinClipbImpl = this;
}

//------------------------------------------------------------------------
// getContent
//------------------------------------------------------------------------

Reference< XTransferable > SAL_CALL CWinClipbImpl::getContents( ) throw( RuntimeException )
{
    if ( NULL != m_pClipboardContent )
    {
        //return Reference< XTransferable >( m_pClipbContent->m_rXTDataSource );
        //return m_pClipboardContent->m_rXTDataSource;
        return Reference< XTransferable >( );
    }
    else
    {
        // get the current dataobject from clipboard
        IDataObjectPtr pIDataObject;
        MTAGetClipboard( &pIDataObject );

        // create an apartment neutral dataobject and initialize it with a
        // com smart pointer to the IDataObject from clipboard
        IDataObjectPtr pIDo = ( new CAPNDataObject( pIDataObject ) );

        CDTransObjFactory objFactory;

        // remeber pIDo destroys itself due to the smart pointer
        //return Reference< XTransferable >( static_cast< XTransferable* >( new CDOTransferable( pIDo ) ) );
        return objFactory.createTransferableFromDataObj( m_pWinClipboard->m_SrvMgr, pIDo );
    }
}

//------------------------------------------------------------------------
// setContent
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::setContents( const Reference< XTransferable >& xTransferable,
                                          const Reference< XClipboardOwner >& xClipboardOwner )
                                          throw( RuntimeException )
{
    CDTransObjFactory objFactory;

    IDataObjectPtr pIDataObj( objFactory.createDataObjFromTransferable(
        m_pWinClipboard->m_SrvMgr , xTransferable ) );

    MTASetClipboard( pIDataObj );
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
    MTAFlushClipboard( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::registerClipboardViewer( ) const
{
    if ( m_pWinClipboard->hasClipboardListener( ) )
        MTARegisterClipboardViewer( CWinClipbImpl::onClipboardContentChanged );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::unregisterClipboardViewer( ) const
{
    if ( !m_pWinClipboard->hasClipboardListener( ) )
        MTARegisterClipboardViewer( NULL );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CWinClipbImpl::dispose() throw( RuntimeException )
{
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CALLBACK CWinClipbImpl::onClipboardContentChanged( void )
{
    // reassocition to instance through static member
    s_pCWinClipbImpl->m_pWinClipboard->notifyAllClipboardListener( );
}

//------------------------------------------------------------------------
// returns wether we are in dispose or not, is important for the
// XTDataObject so that we ignore the "Link" format during FlushClipboard
//------------------------------------------------------------------------

sal_Bool SAL_CALL CWinClipbImpl::isInDispose( ) const
{
    return m_bInDispose;
}
