/*************************************************************************
 *
 *  $RCSfile: oleembed.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-13 17:01:13 $
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

#include <oleembobj.hxx>

#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDVERBS_HPP_
#include <com/sun/star/embed/EmbedVerbs.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ENTRYINITMODES_HPP_
#include <com/sun/star/embed/EntryInitModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDUPDATEMODES_HPP_
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_NEEDSRUNNINGSTATEEXCEPTION_HPP_
#include <com/sun/star/embed/NeedsRunningStateException.hpp>
#endif



#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#include <olecomponent.hxx>

using namespace ::com::sun::star;

//----------------------------------------------
uno::Sequence< sal_Int32 > OleEmbeddedObject::GetReachableStatesList_Impl(
                                                        const uno::Sequence< embed::VerbDescr >& aVerbList )
{
    // TODO: find the sequence
    return uno::Sequence< sal_Int32 >();
}

//----------------------------------------------
uno::Sequence< sal_Int32 > OleEmbeddedObject::GetIntermediateVerbsSequence_Impl( sal_Int32 nNewState )
{
    // TODO: find the sequence
    OSL_ENSURE( m_nObjectState != embed::EmbedStates::EMBED_LOADED, "Loaded object is switched to running state without verbs using!" );

    // actually there will be only one verb
    if ( m_nObjectState == embed::EmbedStates::EMBED_RUNNING && nNewState == embed::EmbedStates::EMBED_ACTIVE )
    {
        uno::Sequence< sal_Int32 > aVerbs( 1 );
        aVerbs[0] = embed::EmbedVerbs::MS_OLEVERB_OPEN;
    }

    return uno::Sequence< sal_Int32 >();
}

//----------------------------------------------
void SAL_CALL OleEmbeddedObject::changeState( sal_Int32 nNewState )
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

    if ( !m_pOleComponent )
        throw uno::RuntimeException();

    // TODO: additional verbs can be a problem, since nobody knows how the object
    //       will behave after activation

    if ( nNewState == embed::EmbedStates::EMBED_LOADED )
    {
        // This means just closing of the current object
        SaveObject_Impl();
        m_pOleComponent->CloseObject();
    }
    else
    {
        if ( m_nObjectState == embed::EmbedStates::EMBED_LOADED )
        {
            // if the target object is in loaded state and a different state is specified
            // as a new one the object first must be switched to running state.

            m_pOleComponent->RunObject();
            m_nObjectState = embed::EmbedStates::EMBED_RUNNING;
            if ( m_nObjectState == nNewState )
                return;
        }

        // so now the object is either switched from Active to Running state or vise versa
        if ( m_nObjectState == embed::EmbedStates::EMBED_RUNNING && nNewState == embed::EmbedStates::EMBED_ACTIVE )
        {
            // execute OPEN verb, if object does not reach active state it is an object's problem
            m_pOleComponent->ExecuteVerb( embed::EmbedVerbs::MS_OLEVERB_OPEN );
        }
        else if ( m_nObjectState == embed::EmbedStates::EMBED_ACTIVE && nNewState == embed::EmbedStates::EMBED_RUNNING )
        {
            m_pOleComponent->CloseObject();
            m_pOleComponent->RunObject();
        }
        else
        {
            OSL_ENSURE( sal_False, "Unreachable code executed!" );
        }
    }
}

//----------------------------------------------
uno::Sequence< sal_Int32 > SAL_CALL OleEmbeddedObject::getReachableStates()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( !m_pOleComponent )
        throw uno::RuntimeException();

    if ( m_nObjectState == embed::EmbedStates::EMBED_LOADED )
    {
        // the list of supported verbs can be retrieved only when object is in running state
        throw embed::NeedsRunningStateException(); // TODO:
    }

    // the list of states can only be guessed based on standard verbs,
    // since there is no way to detect what additional verbs do
    return GetReachableStatesList_Impl( m_pOleComponent->GetVerbList() );
}

//----------------------------------------------
sal_Int32 SAL_CALL OleEmbeddedObject::getCurrentState()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    // TODO: Shouldn't we ask object? ( I guess no )
    return m_nObjectState;
}

//----------------------------------------------
void SAL_CALL OleEmbeddedObject::doVerb( sal_Int32 nVerbID )
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

    if ( !m_pOleComponent )
        throw uno::RuntimeException();

    if ( m_nObjectState == embed::EmbedStates::EMBED_LOADED )
    {
        // if the target object is in loaded state
        // it must be switched to running state to execute verb

        m_pOleComponent->RunObject();
        m_nObjectState = embed::EmbedStates::EMBED_RUNNING;
    }

    m_pOleComponent->ExecuteVerb( nVerbID );
}

//----------------------------------------------
uno::Sequence< embed::VerbDescr > SAL_CALL OleEmbeddedObject::getSupportedVerbs()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_nObjectState == embed::EmbedStates::EMBED_LOADED )
    {
        // the list of supported verbs can be retrieved only when object is in running state
        throw embed::NeedsRunningStateException(); // TODO:
    }

    return m_pOleComponent->GetVerbList();
}

//----------------------------------------------
void SAL_CALL OleEmbeddedObject::setClientSite(
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
uno::Reference< embed::XEmbeddedClient > SAL_CALL OleEmbeddedObject::getClientSite()
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
void SAL_CALL OleEmbeddedObject::update()
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
void SAL_CALL OleEmbeddedObject::setUpdateMode( sal_Int32 nMode )
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
sal_Int64 SAL_CALL OleEmbeddedObject::getStatus( sal_Int64 nAspect )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::EMBED_LOADED )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object must be in running state!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return m_pOleComponent->GetMiscStatus();
}

