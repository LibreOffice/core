/*************************************************************************
 *
 *  $RCSfile: embedobj.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-18 12:47:05 $
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

#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDVERBS_HPP_
#include <com/sun/star/embed/EmbedVerbs.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDUPDATEMODES_HPP_
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDCLIENT_HPP_
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCLOSEBROADCASTER_HPP_
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif


#include "commonembobj.hxx"
#include "convert.hxx"
#include "intercept.hxx"


using namespace ::com::sun::star;

//----------------------------------------------
sal_Int32 OCommonEmbeddedObject::ConvertVerbToState_Impl( sal_Int32 nVerb )
{
    for ( sal_Int32 nInd = 0; nInd < m_aVerbTable.getLength(); nInd++ )
        if ( m_aVerbTable[nInd][0] == nVerb )
            return m_aVerbTable[nInd][1];

    throw lang::IllegalArgumentException(); // TODO: unexpected verb provided
}

//----------------------------------------------
void OCommonEmbeddedObject::SwitchStateTo_Impl( sal_Int32 nNextState )
{
    // TODO: may be needs interaction handler to detect wherether the object state
    //       can be changed even after errors

    if ( m_nObjectState == embed::EmbedStates::EMBED_LOADED )
    {
        if ( nNextState == embed::EmbedStates::EMBED_RUNNING )
        {
            if ( m_bIsLink )
            {
                m_pDocHolder->SetDocument( LoadLink_Impl(), m_bReadOnly );
            }
            else
            {
                // in case embedded object is in loaded state the contents must
                // be stored in the related storage and the storage
                // must be created already

                if ( !m_xObjectStorage.is() )
                    throw io::IOException(); //TODO: access denied

                m_pDocHolder->SetDocument( LoadDocumentFromStorage_Impl( m_xObjectStorage ), m_bReadOnly );
            }

            if ( !m_pDocHolder->GetDocument().is() )
                embed::UnreachableStateException(); //TODO: can't open document

            m_nObjectState = nNextState;
        }
        else
        {
            OSL_ENSURE( sal_False, "Unacceptable state switch!\n" );
            throw uno::RuntimeException(); // TODO
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::EMBED_RUNNING )
    {
        if ( nNextState == embed::EmbedStates::EMBED_LOADED )
        {
            // actually frame should not exist at this point
            m_pDocHolder->CloseDocument( sal_False, sal_False );

            m_nObjectState = nNextState;
        }
        else if ( nNextState == embed::EmbedStates::EMBED_ACTIVE )
        {
            if ( !m_xClientSite.is() )
                throw embed::WrongStateException(); //TODO: client site is not set!

            // create frame and load document in the frame
            m_pDocHolder->Show();

            m_xClientSite->onShowWindow( sal_True );

            m_nObjectState = nNextState;
        }
        else
        {
            OSL_ENSURE( sal_False, "Unacceptable state switch!\n" );
            throw uno::RuntimeException(); // TODO
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::EMBED_ACTIVE )
    {
        if ( nNextState == embed::EmbedStates::EMBED_RUNNING )
        {
            uno::Reference< util::XModifiable > xModif( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
            if ( !xModif.is() )
                throw uno::RuntimeException();

            if ( !m_xClientSite.is() )
                throw embed::WrongStateException(); //TODO: client site is not set!

            // store document if it is modified
            if ( xModif->isModified() )
            {
                try {
                    m_xClientSite->saveObject();
                }
                catch( embed::ObjectSaveVetoException& )
                {
                }
                catch( uno::Exception& e )
                {
                    throw embed::StorageWTException(
                        ::rtl::OUString::createFromAscii( "The client could not store the object!" ),
                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ),
                        uno::makeAny( e ) );
                }
            }

            m_pDocHolder->CloseFrame();

            m_xClientSite->onShowWindow( sal_False );
            // when Hide() method is fixed the frame will not be closed but hided
            // m_pDocHolder->Hide();

            m_nObjectState = nNextState;
        }
        else
        {
            OSL_ENSURE( sal_False, "Unacceptable state switch!\n" );
            throw uno::RuntimeException(); // TODO
        }
    }
    else
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object is in unacceptable state!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
}

//----------------------------------------------
uno::Sequence< sal_Int32 > OCommonEmbeddedObject::GetIntermediateStatesSequence_Impl( sal_Int32 nNewState )
{
    sal_Int32 nCurInd = 0;
    for ( nCurInd = 0; nCurInd < m_aAcceptedStates.getLength(); nCurInd++ )
        if ( m_aAcceptedStates[nCurInd] == m_nObjectState )
            break;

    if ( nCurInd == m_aAcceptedStates.getLength() )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object is in unacceptable state!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    sal_Int32 nDestInd = 0;
    for ( nDestInd = 0; nDestInd < m_aAcceptedStates.getLength(); nDestInd++ )
        if ( m_aAcceptedStates[nDestInd] == nNewState )
            break;

    if ( nDestInd == m_aAcceptedStates.getLength() )
        throw embed::UnreachableStateException(
            ::rtl::OUString::createFromAscii( "The state either not reachable, or the object allows the state only as an intermediate one!\n" ),
            uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ),
            m_nObjectState,
            nNewState );

    return m_pIntermediateStatesSeqs[nCurInd][nDestInd];
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::changeState( sal_Int32 nNewState )
        throw ( embed::UnreachableStateException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    // in case the object is already in requested state
    if ( m_nObjectState == nNewState )
        return;

    // retrieve sequence of states that should be passed to reach desired state
    uno::Sequence< sal_Int32 > aIntermediateStates = GetIntermediateStatesSequence_Impl( nNewState );

    for ( sal_Int32 nInd = 0; nInd < aIntermediateStates.getLength(); nInd++ )
        SwitchStateTo_Impl( aIntermediateStates[nInd] );

    SwitchStateTo_Impl( nNewState );
}

//----------------------------------------------
uno::Sequence< sal_Int32 > SAL_CALL OCommonEmbeddedObject::getReachableStates()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aAcceptedStates;
}

//----------------------------------------------
sal_Int32 SAL_CALL OCommonEmbeddedObject::getCurrentState()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return m_nObjectState;
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::doVerb( sal_Int32 nVerbID )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                embed::UnreachableStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    // for internal documents this call is just a duplicate of changeState
    changeState( ConvertVerbToState_Impl( nVerbID ) );
}

//----------------------------------------------
uno::Sequence< embed::VerbDescr > SAL_CALL OCommonEmbeddedObject::getSupportedVerbs()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    uno::Sequence< embed::VerbDescr > aResult;
    sal_Int32 nResLen = 0;

    // verbs list will be set on initialization depending from document type
    for ( sal_Int32 nStatesInd = 0; nStatesInd < m_aAcceptedStates.getLength(); nStatesInd++ )
        for ( sal_Int32 nVerbInd = 0; nVerbInd < m_aVerbTable.getLength(); nVerbInd++ )
            if ( m_aVerbTable[nVerbInd][1] == m_aAcceptedStates[nStatesInd] )
            {
                aResult.realloc( ++nResLen );
                // TODO: fill the whole structure
                aResult[nResLen-1].VerbID = m_aVerbTable[nVerbInd][0];
            }

    return aResult;
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::setClientSite(
                const uno::Reference< embed::XEmbeddedClient >& xClient )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    m_xClientSite = xClient;
}

//----------------------------------------------
uno::Reference< embed::XEmbeddedClient > SAL_CALL OCommonEmbeddedObject::getClientSite()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return m_xClientSite;
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::update()
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_nUpdateMode == embed::EmbedUpdateModes::EMBED_EXPLICIT_UPDATE )
    {
        // TODO: update view representation
    }
    else
    {
        // the object must be up to date
        OSL_ENSURE( m_nUpdateMode == embed::EmbedUpdateModes::EMBED_ALWAYS_UPDATE, "Unknown update mode!\n" );
    }
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::setUpdateMode( sal_Int32 nMode )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    OSL_ENSURE( nMode == embed::EmbedUpdateModes::EMBED_ALWAYS_UPDATE
                    || nMode == embed::EmbedUpdateModes::EMBED_EXPLICIT_UPDATE,
                "Unknown update mode!\n" );
    m_nUpdateMode = nMode;
}

//----------------------------------------------
sal_Int64 SAL_CALL OCommonEmbeddedObject::getStatus( sal_Int64 nAspect )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return 0;
    // TODO:
    // the status information must be filled in from configuration during object contruction
}

