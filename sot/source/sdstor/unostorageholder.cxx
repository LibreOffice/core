/*************************************************************************
 *
 *  $RCSfile: unostorageholder.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 16:53:01 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTIONBROADCASTER_HPP_
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#include <comphelper/processfactory.hxx>

#include "unostorageholder.hxx"


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

void SAL_CALL UNOStorageHolder::preCommit( const lang::EventObject& aEvent )
        throw ( uno::Exception,
                uno::RuntimeException )
{
    // do nothing
}

void SAL_CALL UNOStorageHolder::commited( const lang::EventObject& aEvent )
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
    aArg[1] <<= embed::ElementModes::ELEMENT_READWRITE;
    uno::Reference< embed::XStorage > xTempStorage( xStorageFactory->createInstanceWithArguments( aArg ), uno::UNO_QUERY );

    OSL_ENSURE( xTempStorage.is(), "Can't open storage!\n" );
    if ( !xTempStorage.is() )
        throw uno::RuntimeException();

    m_xStorage->copyToStorage( xTempStorage );

    uno::Reference< lang::XComponent > xComp( xTempStorage, uno::UNO_QUERY );
    if ( !xComp.is() )
        throw uno::RuntimeException();

    xComp->dispose();

    SotStorageRef rTempStorage = new SotStorage( TRUE, aTmpStorFile.GetURL(), STREAM_WRITE, STORAGE_TRANSACTED );
    if ( !rTempStorage.Is() || rTempStorage->GetError() != ERRCODE_NONE )
        throw uno::RuntimeException();

    rTempStorage->CopyTo( m_rSotStorage );

    // CopyTo does not transport unknown media type
    // just workaround it
    uno::Any aMediaType;
    if ( rTempStorage->GetProperty( ::rtl::OUString::createFromAscii( "MediaType" ), aMediaType ) )
        m_rSotStorage->SetProperty( ::rtl::OUString::createFromAscii( "MediaType" ), aMediaType );

    m_rSotStorage->Commit();
}

void SAL_CALL UNOStorageHolder::preRevert( const lang::EventObject& aEvent )
        throw ( uno::Exception,
                uno::RuntimeException )
{
    // do nothing
}

void SAL_CALL UNOStorageHolder::reverted( const lang::EventObject& aEvent )
        throw ( uno::RuntimeException )
{
    // do nothing, since reverting of the duplicate storage just means
    // not to copy changes done for it to the original storage
}

void SAL_CALL UNOStorageHolder::disposing( const lang::EventObject& Source )
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


