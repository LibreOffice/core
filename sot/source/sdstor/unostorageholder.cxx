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
#include "precompiled_sot.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <comphelper/processfactory.hxx>

#include "unostorageholder.hxx"
#include <sot/storinfo.hxx>


using namespace ::com::sun::star;

UNOStorageHolder::UNOStorageHolder( SotStorage& aParentStorage,
                                    SotStorage& aStorage,
                                    uno::Reference< embed::XStorage > xStorage,
                                    ::utl::TempFile* pTempFile )
: m_pParentStorage( &aParentStorage )
, m_rSotStorage( &aStorage )
, m_xStorage( xStorage )
, m_pTempFile( pTempFile )
{
    OSL_ENSURE( m_xStorage.is() && m_pTempFile, "Wrong initialization!\n" );
    if ( !m_xStorage.is() || !m_pTempFile )
        throw uno::RuntimeException();

    uno::Reference< embed::XTransactionBroadcaster > xTrBroadcast( m_xStorage, uno::UNO_QUERY );
    if ( !xTrBroadcast.is() )
        throw uno::RuntimeException();

    xTrBroadcast->addTransactionListener( (embed::XTransactionListener*)this );
}

void UNOStorageHolder::InternalDispose()
{
    uno::Reference< embed::XTransactionBroadcaster > xTrBroadcast( m_xStorage, uno::UNO_QUERY );
    if ( xTrBroadcast.is() )
        xTrBroadcast->removeTransactionListener( (embed::XTransactionListener*)this );

    uno::Reference< lang::XComponent > xComponent( m_xStorage, uno::UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();
    m_xStorage = uno::Reference< embed::XStorage >();

    if ( m_pParentStorage )
        m_pParentStorage = NULL;

    if ( m_pTempFile )
    {
        delete m_pTempFile;
        m_pTempFile = NULL;
    }

    if ( m_rSotStorage.Is() )
        m_rSotStorage = NULL;
}

String UNOStorageHolder::GetStorageName()
{
    if ( m_rSotStorage.Is() )
        return m_rSotStorage->GetName();

    return String();
}

void SAL_CALL UNOStorageHolder::preCommit( const lang::EventObject& /*aEvent*/ )
        throw ( uno::Exception,
                uno::RuntimeException )
{
    // do nothing
}

void SAL_CALL UNOStorageHolder::commited( const lang::EventObject& /*aEvent*/ )
        throw ( uno::RuntimeException )
{
    ::utl::TempFile aTmpStorFile;
    if ( !aTmpStorFile.GetURL().Len() )
        throw uno::RuntimeException();

    uno::Reference< lang::XSingleServiceFactory > xStorageFactory(
            ::comphelper::getProcessServiceFactory()->createInstance(
                   ::rtl::OUString::createFromAscii( "com.sun.star.embed.StorageFactory" ) ),
            uno::UNO_QUERY );

    OSL_ENSURE( xStorageFactory.is(), "Can't create storage factory!\n" );
    if ( !xStorageFactory.is() )
        throw uno::RuntimeException();

    uno::Sequence< uno::Any > aArg( 2 );
    aArg[0] <<= ::rtl::OUString( aTmpStorFile.GetURL() );
    aArg[1] <<= embed::ElementModes::READWRITE;
    uno::Reference< embed::XStorage > xTempStorage( xStorageFactory->createInstanceWithArguments( aArg ), uno::UNO_QUERY );

    OSL_ENSURE( xTempStorage.is(), "Can't open storage!\n" );
    if ( !xTempStorage.is() )
        throw uno::RuntimeException();

    m_xStorage->copyToStorage( xTempStorage );

    uno::Reference< lang::XComponent > xComp( xTempStorage, uno::UNO_QUERY );
    if ( !xComp.is() )
        throw uno::RuntimeException();

    xComp->dispose();

    SotStorageRef rTempStorage = new SotStorage( sal_True, aTmpStorFile.GetURL(), STREAM_WRITE, STORAGE_TRANSACTED );
    if ( !rTempStorage.Is() || rTempStorage->GetError() != ERRCODE_NONE )
        throw uno::RuntimeException();

    SvStorageInfoList aSubStorInfoList;
    m_rSotStorage->FillInfoList( &aSubStorInfoList );
    for ( sal_uInt32 nInd = 0; nInd < aSubStorInfoList.Count(); nInd++ )
    {
        m_rSotStorage->Remove( aSubStorInfoList[nInd].GetName() );
        if ( m_rSotStorage->GetError() )
        {
            m_rSotStorage->ResetError();
            throw uno::RuntimeException();
        }
    }

    rTempStorage->CopyTo( m_rSotStorage );

    // CopyTo does not transport unknown media type
    // just workaround it
    uno::Any aMediaType;
    if ( rTempStorage->GetProperty( ::rtl::OUString::createFromAscii( "MediaType" ), aMediaType ) )
        m_rSotStorage->SetProperty( ::rtl::OUString::createFromAscii( "MediaType" ), aMediaType );

    m_rSotStorage->Commit();
}

void SAL_CALL UNOStorageHolder::preRevert( const lang::EventObject& /*aEvent*/ )
        throw ( uno::Exception,
                uno::RuntimeException )
{
    // do nothing
}

void SAL_CALL UNOStorageHolder::reverted( const lang::EventObject& /*aEvent*/ )
        throw ( uno::RuntimeException )
{
    // do nothing, since reverting of the duplicate storage just means
    // not to copy changes done for it to the original storage
}

void SAL_CALL UNOStorageHolder::disposing( const lang::EventObject& /*Source*/ )
        throw ( uno::RuntimeException )
{
    if ( m_pTempFile )
    {
        delete m_pTempFile;
        m_pTempFile = NULL;
    }

    if ( m_rSotStorage.Is() )
        m_rSotStorage = NULL;

    if ( m_pParentStorage )
    {
        SotStorage* pTmp = m_pParentStorage;
        m_pParentStorage = NULL;
        pTmp->RemoveUNOStorageHolder( this ); // this statement can lead to destruction of the holder
    }
}


