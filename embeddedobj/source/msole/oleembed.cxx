/*************************************************************************
 *
 *  $RCSfile: oleembed.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 17:53:50 $
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
                                                        const uno::Sequence< embed::VerbDescriptor >& aVerbList )
{
    uno::Sequence< sal_Int32 > aStates(2);
    aStates[0] = embed::EmbedStates::LOADED;
    aStates[1] = embed::EmbedStates::RUNNING;
    for ( sal_Int32 nInd = 0; nInd < aVerbList.getLength(); nInd++ )
        if ( aVerbList[nInd].VerbID == embed::EmbedVerbs::MS_OLEVERB_OPEN )
        {
            aStates.realloc(3);
            aStates[2] = embed::EmbedStates::ACTIVE;
        }

    return aStates;
}

//----------------------------------------------
uno::Sequence< sal_Int32 > OleEmbeddedObject::GetIntermediateVerbsSequence_Impl( sal_Int32 nNewState )
{
    OSL_ENSURE( m_nObjectState != embed::EmbedStates::LOADED, "Loaded object is switched to running state without verbs using!" );

    // actually there will be only one verb
    if ( m_nObjectState == embed::EmbedStates::RUNNING && nNewState == embed::EmbedStates::ACTIVE )
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

#ifdef WNT
    // TODO: additional verbs can be a problem, since nobody knows how the object
    //       will behave after activation

    if ( nNewState == embed::EmbedStates::LOADED )
    {
        // This means just closing of the current object
        // If component can not be closed the object stays in loaded state
        // and it holds reference to "incomplete" component
        // If the object is switched to running state later
        // the component will become "complete"
        GetRidOfComponent();
    }
    else if ( nNewState == embed::EmbedStates::RUNNING || nNewState == embed::EmbedStates::ACTIVE )
    {
        if ( m_nObjectState == embed::EmbedStates::LOADED )
        {
            // if the target object is in loaded state and a different state is specified
            // as a new one the object first must be switched to running state.

            // the component can exist already in nonrunning state
            // it can be created during loading to detect type of object
            CreateOleComponentAndLoad_Impl( m_pOleComponent );

            m_pOleComponent->RunObject();
            m_nObjectState = embed::EmbedStates::RUNNING;
            if ( m_nObjectState == nNewState )
                return;
        }

        // so now the object is either switched from Active to Running state or vise versa
        if ( m_nObjectState == embed::EmbedStates::RUNNING && nNewState == embed::EmbedStates::ACTIVE )
        {
            // execute OPEN verb, if object does not reach active state it is an object's problem
            m_pOleComponent->ExecuteVerb( embed::EmbedVerbs::MS_OLEVERB_OPEN );
        }
        else if ( m_nObjectState == embed::EmbedStates::ACTIVE && nNewState == embed::EmbedStates::RUNNING )
        {
            m_pOleComponent->CloseObject();
            m_pOleComponent->RunObject();
        }
        else
        {
            throw embed::UnreachableStateException();
        }
    }
    else
#endif
        throw embed::UnreachableStateException();

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

#ifdef WNT
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        // the list of supported verbs can be retrieved only when object is in running state
        throw embed::NeedsRunningStateException(); // TODO:
    }

    if ( !m_pOleComponent )
        throw uno::RuntimeException();

    // the list of states can only be guessed based on standard verbs,
    // since there is no way to detect what additional verbs do
    return GetReachableStatesList_Impl( m_pOleComponent->GetVerbList() );
#else
    return uno::Sequence< sal_Int32 >();
#endif
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

#ifdef WNT
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        // if the target object is in loaded state
        // it must be switched to running state to execute verb

        // the component can exist already in noncomplete state
        // it can be created during loading
        CreateOleComponentAndLoad_Impl( m_pOleComponent );
        m_pOleComponent->RunObject();
        m_nObjectState = embed::EmbedStates::RUNNING;
    }

    if ( !m_pOleComponent )
        throw uno::RuntimeException();

    m_pOleComponent->ExecuteVerb( nVerbID );
#else
    throw embed::UnreachableStateException();
#endif
}

//----------------------------------------------
uno::Sequence< embed::VerbDescriptor > SAL_CALL OleEmbeddedObject::getSupportedVerbs()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
#ifdef WNT
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        // the list of supported verbs can be retrieved only when object is in running state
        throw embed::NeedsRunningStateException(); // TODO:
    }

    if ( !m_pOleComponent )
        throw uno::RuntimeException();

    return m_pOleComponent->GetVerbList();
#else
    return uno::Sequence< embed::VerbDescriptor >();
#endif
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

    if ( m_nUpdateMode == embed::EmbedUpdateModes::EXPLICIT_UPDATE )
    {
        // TODO: update view representation
    }
    else
    {
        // the object must be up to date
        OSL_ENSURE( m_nUpdateMode == embed::EmbedUpdateModes::ALWAYS_UPDATE, "Unknown update mode!\n" );
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

    OSL_ENSURE( nMode == embed::EmbedUpdateModes::ALWAYS_UPDATE
                    || nMode == embed::EmbedUpdateModes::EXPLICIT_UPDATE,
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

    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::LOADED )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object must be in running state!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

#ifdef WNT
    if ( !m_pOleComponent )
        throw uno::RuntimeException();

    return m_pOleComponent->GetMiscStatus( nAspect );
#else
    throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Illegal call!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
#endif
}

//----------------------------------------------
void SAL_CALL OleEmbeddedObject::setContainerName( const ::rtl::OUString& sName )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    m_aContainerName = sName;
}


