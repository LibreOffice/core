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
#include "precompiled_vcl.hxx"

#include "vcl/svapp.hxx"

#include "xconnection.hxx"
#include "svdata.hxx"
#include "salinst.hxx"

namespace {

namespace css = com::sun::star;

}

namespace vcl
{
    class SolarMutexReleaser
    {
        sal_uLong mnReleased;
    public:
        SolarMutexReleaser()
        {
            mnReleased = Application::ReleaseSolarMutex();
        }

        ~SolarMutexReleaser()
        {
            if( mnReleased )
                Application::AcquireSolarMutex( mnReleased );
        }
    };
}

using namespace rtl;
using namespace osl;
using namespace vcl;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;


DisplayConnection::DisplayConnection()
{
    SalInstance::ConnectionIdentifierType eType;
    int nBytes;
    void* pBytes = ImplGetSVData()->mpDefInst->GetConnectionIdentifier( eType, nBytes );
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
{}

void DisplayConnection::start()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->SetEventCallback( this );
}

void DisplayConnection::terminate()
{
    ImplSVData* pSVData = ImplGetSVData();

    if( pSVData )
    {
        pSVData->mpDefInst->SetEventCallback( NULL );
    }

    SolarMutexReleaser aRel;

    MutexGuard aGuard( m_aMutex );
    Any aEvent;
    std::list< css::uno::Reference< XEventHandler > > aLocalList( m_aHandlers );
    for( ::std::list< css::uno::Reference< XEventHandler > >::const_iterator it = aLocalList.begin(); it != aLocalList.end(); ++it )
        (*it)->handleEvent( aEvent );
}

void SAL_CALL DisplayConnection::addEventHandler( const Any& /*window*/, const css::uno::Reference< XEventHandler >& handler, sal_Int32 /*eventMask*/ ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.push_back( handler );
}

void SAL_CALL DisplayConnection::removeEventHandler( const Any& /*window*/, const css::uno::Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aHandlers.remove( handler );
}

void SAL_CALL DisplayConnection::addErrorHandler( const css::uno::Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aErrorHandlers.push_back( handler );
}

void SAL_CALL DisplayConnection::removeErrorHandler( const css::uno::Reference< XEventHandler >& handler ) throw()
{
    MutexGuard aGuard( m_aMutex );

    m_aErrorHandlers.remove( handler );
}

Any SAL_CALL DisplayConnection::getIdentifier() throw()
{
    return m_aAny;
}

bool DisplayConnection::dispatchEvent( void* pData, int nBytes )
{
    SolarMutexReleaser aRel;

    Sequence< sal_Int8 > aSeq( (sal_Int8*)pData, nBytes );
    Any aEvent;
    aEvent <<= aSeq;
    ::std::list< css::uno::Reference< XEventHandler > > handlers;
    {
        MutexGuard aGuard( m_aMutex );
        handlers = m_aHandlers;
    }
    for( ::std::list< css::uno::Reference< XEventHandler > >::const_iterator it = handlers.begin(); it != handlers.end(); ++it )
        if( (*it)->handleEvent( aEvent ) )
            return true;
    return false;
}

bool DisplayConnection::dispatchErrorEvent( void* pData, int nBytes )
{
    SolarMutexReleaser aRel;

    Sequence< sal_Int8 > aSeq( (sal_Int8*)pData, nBytes );
    Any aEvent;
    aEvent <<= aSeq;
    ::std::list< css::uno::Reference< XEventHandler > > handlers;
    {
        MutexGuard aGuard( m_aMutex );
        handlers = m_aErrorHandlers;
    }
    for( ::std::list< css::uno::Reference< XEventHandler > >::const_iterator it = handlers.begin(); it != handlers.end(); ++it )
        if( (*it)->handleEvent( aEvent ) )
            return true;

    return false;
}
