/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xconnection.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 16:35:54 $
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

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <xconnection.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

using namespace rtl;
using namespace osl;
using namespace vcl;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;

DisplayConnection::DisplayConnection()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDisplayConnection = this;
    pSVData->mpDefInst->SetEventCallback( this, dispatchEvent );
    pSVData->mpDefInst->SetErrorEventCallback( this, dispatchErrorEvent );

    SalInstance::ConnectionIdentifierType eType;
    int nBytes;
    void* pBytes = pSVData->mpDefInst->GetConnectionIdentifier( eType, nBytes );
    switch( eType )
    {
        case SalInstance::AsciiCString:
            m_aAny <<= OUString::createFromAscii( (sal_Char*)pBytes );
            break;
        case SalInstance::Blob:
            m_aAny <<= Sequence< sal_Int8 >( (sal_Int8*)pBytes, nBytes );
            break;
    }
}

DisplayConnection::~DisplayConnection()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDisplayConnection = NULL;
    pSVData->mpDefInst->SetEventCallback( NULL, NULL );
    pSVData->mpDefInst->SetErrorEventCallback( NULL, NULL );
}


void SAL_CALL DisplayConnection::addEventHandler( const Any& /*window*/, const Reference< XEventHandler >& handler, sal_Int32 /*eventMask*/ ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.push_back( handler );
}

void SAL_CALL DisplayConnection::removeEventHandler( const Any& /*window*/, const Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.remove( handler );
}

void SAL_CALL DisplayConnection::addErrorHandler( const Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aErrorHandlers.push_back( handler );
}

void SAL_CALL DisplayConnection::removeErrorHandler( const Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aErrorHandlers.remove( handler );
}

Any SAL_CALL DisplayConnection::getIdentifier() throw()
{
    return m_aAny;
}

void DisplayConnection::dispatchDowningEvent()
{
    MutexGuard aGuard( m_aMutex );
    Any aEvent;
    std::list< Reference< XEventHandler > > aLocalList( m_aHandlers );
    for( ::std::list< Reference< XEventHandler > >::const_iterator it = aLocalList.begin(); it != aLocalList.end(); ++it )
        (*it)->handleEvent( aEvent );
}

bool DisplayConnection::dispatchEvent( void* pThis, void* pData, int nBytes )
{
    DisplayConnection* This = (DisplayConnection*)pThis;
    MutexGuard aGuard( This->m_aMutex );

    Sequence< sal_Int8 > aSeq( (sal_Int8*)pData, nBytes );
    Any aEvent;
    aEvent <<= aSeq;
    for( ::std::list< Reference< XEventHandler > >::const_iterator it = This->m_aHandlers.begin(); it != This->m_aHandlers.end(); ++it )
        if( (*it)->handleEvent( aEvent ) )
            return true;
    return false;
}

bool DisplayConnection::dispatchErrorEvent( void* pThis, void* pData, int nBytes )
{
    DisplayConnection* This = (DisplayConnection*)pThis;
    MutexGuard aGuard( This->m_aMutex );

    Sequence< sal_Int8 > aSeq( (sal_Int8*)pData, nBytes );
    Any aEvent;
    aEvent <<= aSeq;
    for( ::std::list< Reference< XEventHandler > >::const_iterator it = This->m_aErrorHandlers.begin(); it != This->m_aErrorHandlers.end(); ++it )
        if( (*it)->handleEvent( aEvent ) )
            return true;

    return false;
}
