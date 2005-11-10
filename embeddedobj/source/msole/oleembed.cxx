/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: oleembed.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-10 16:12:49 $
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
#ifndef _COM_SUN_STAR_EMBED_STATECHANGEINPROGRESSEXCEPTION_HPP_
#include <com/sun/star/embed/StateChangeInProgressException.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif


#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#include <targetstatecontrol.hxx>

#include <olecomponent.hxx>

#include "ownview.hxx"

using namespace ::com::sun::star;

//----------------------------------------------
void OleEmbeddedObject::SwitchComponentToRunningState_Impl()
{
#ifdef WNT
    if ( m_pOleComponent )
    {
        try
        {
            m_pOleComponent->RunObject();
        }
        catch( embed::UnreachableStateException& )
        {
            GetRidOfComponent();
            throw;
        }
        catch( embed::WrongStateException& )
        {
            GetRidOfComponent();
            throw;
        }
    }
    else
#endif
    {
        throw embed::UnreachableStateException();
    }
}

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
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    // in case the object is already in requested state
    if ( m_nObjectState == nNewState )
        return;

#ifdef WNT
    if ( m_pOleComponent )
    {
        if ( m_nTargetState != -1 )
        {
            // means that the object is currently trying to reach the target state
            throw embed::StateChangeInProgressException( ::rtl::OUString(),
                                                        uno::Reference< uno::XInterface >(),
                                                        m_nTargetState );
        }

        TargetStateControl_Impl aControl( m_nTargetState, nNewState );

        // TODO: additional verbs can be a problem, since nobody knows how the object
        //       will behave after activation

        sal_Int32 nOldState = m_nObjectState;
        aGuard.clear();
        StateChangeNotification_Impl( sal_True, nOldState, nNewState );
        aGuard.reset();

        try
        {
            if ( nNewState == embed::EmbedStates::LOADED )
            {
                // This means just closing of the current object
                // If component can not be closed the object stays in loaded state
                // and it holds reference to "incomplete" component
                // If the object is switched to running state later
                // the component will become "complete"
                m_pOleComponent->CloseObject();
                // GetRidOfComponent();
                m_nObjectState = nNewState;
                aGuard.clear();
                StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState );
                aGuard.reset();
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

                    SwitchComponentToRunningState_Impl();
                    m_nObjectState = embed::EmbedStates::RUNNING;
                    aGuard.clear();
                    StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState );
                    aGuard.reset();

#ifdef WNT
                    if ( m_pOleComponent && m_bHasSizeToSet )
                    {
                        aGuard.clear();
                        try {
                            m_pOleComponent->SetExtent( m_aCachedSize, m_nCachedAspect );
                            m_bHasSizeToSet = sal_False;
                        }
                        catch( uno::Exception& ) {}
                        aGuard.reset();
                    }
#endif

                    if ( m_nObjectState == nNewState )
                        return;
                }

                // so now the object is either switched from Active to Running state or vise versa
                // the notification about object state change will be done asynchronously
                if ( m_nObjectState == embed::EmbedStates::RUNNING && nNewState == embed::EmbedStates::ACTIVE )
                {
                    // execute OPEN verb, if object does not reach active state it is an object's problem
                    aGuard.clear();
                    m_pOleComponent->ExecuteVerb( embed::EmbedVerbs::MS_OLEVERB_OPEN );
                    aGuard.reset();

#ifdef WNT
                    // some objects do not allow to set the size even in running state
                    if ( m_pOleComponent && m_bHasSizeToSet )
                    {
                        aGuard.clear();
                        try {
                            m_pOleComponent->SetExtent( m_aCachedSize, m_nCachedAspect );
                            m_bHasSizeToSet = sal_False;
                        }
                        catch( uno::Exception& ) {}
                        aGuard.reset();
                    }
#endif

                    m_nObjectState = nNewState;
                }
                else if ( m_nObjectState == embed::EmbedStates::ACTIVE && nNewState == embed::EmbedStates::RUNNING )
                {
                    aGuard.clear();
                    m_pOleComponent->CloseObject();
                    m_pOleComponent->RunObject(); // Should not fail, the object already was active
                    aGuard.reset();
                    m_nObjectState = nNewState;
                }
                else
                {
                    throw embed::UnreachableStateException();
                }
            }
            else
                throw embed::UnreachableStateException();
        }
        catch( uno::Exception& )
        {
            aGuard.clear();
            StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState );
            throw;
        }
    }
    else
#endif
    {
        throw embed::UnreachableStateException();
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

#ifdef WNT
    if ( m_pOleComponent )
    {
        if ( m_nObjectState == embed::EmbedStates::LOADED )
        {
            // the list of supported verbs can be retrieved only when object is in running state
            throw embed::NeedsRunningStateException(); // TODO:
        }

        // the list of states can only be guessed based on standard verbs,
        // since there is no way to detect what additional verbs do
        return GetReachableStatesList_Impl( m_pOleComponent->GetVerbList() );
    }
    else
#endif
    {
        return uno::Sequence< sal_Int32 >();
    }
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
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

#ifdef WNT
    if ( m_pOleComponent )
    {
        sal_Int32 nOldState = m_nObjectState;

        // TODO/LATER detect target state here and do a notification
        // StateChangeNotification_Impl( sal_True, nOldState, nNewState );
        if ( m_nObjectState == embed::EmbedStates::LOADED )
        {
            // if the target object is in loaded state
            // it must be switched to running state to execute verb
            aGuard.clear();
            changeState( embed::EmbedStates::RUNNING );
            aGuard.reset();
        }

        try {
            if ( !m_pOleComponent )
                throw uno::RuntimeException();

            m_pOleComponent->ExecuteVerb( nVerbID );
        }
        catch( uno::Exception& )
        {
            aGuard.clear();
            StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState );
            throw;
        }

        // the following notification will be done asynchronously
        // StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState );
    }
    else
#endif
    {
        if ( nVerbID == -9 )
        {
            // the workaround verb to show the object in case no server is available
            if ( !m_pOwnView && m_xObjectStream.is() )
            {
                try {
                    uno::Reference< io::XSeekable > xSeekable( m_xObjectStream, uno::UNO_QUERY );
                    if ( xSeekable.is() )
                        xSeekable->seek( 0 );

                    m_pOwnView = new OwnView_Impl( m_xFactory, m_xObjectStream->getInputStream() );
                    m_pOwnView->acquire();
                }
                catch( uno::RuntimeException& )
                {
                    throw;
                }
                catch( uno::Exception& )
                {
                }
            }

            if ( !m_pOwnView || !m_pOwnView->Open() )
                throw embed::UnreachableStateException();
        }
        else
            throw embed::UnreachableStateException();
    }
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
    if ( m_pOleComponent )
    {
        // registry could be used in this case
        // if ( m_nObjectState == embed::EmbedStates::LOADED )
        // {
        //  // the list of supported verbs can be retrieved only when object is in running state
        //  throw embed::NeedsRunningStateException(); // TODO:
        // }

        return m_pOleComponent->GetVerbList();
    }
    else
#endif
    {
        return uno::Sequence< embed::VerbDescriptor >();
    }
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

    if ( m_xClientSite != xClient)
    {
        if ( m_nObjectState != embed::EmbedStates::LOADED && m_nObjectState != embed::EmbedStates::RUNNING )
            throw embed::WrongStateException(
                                    ::rtl::OUString::createFromAscii( "The client site can not be set currently!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

        m_xClientSite = xClient;
    }
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

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object must be in running state!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

#ifdef WNT
    if ( m_bGotStatus && m_nStatusAspect == nAspect )
        return m_nStatus;
    else if ( m_pOleComponent )
    {
        // OLE should allow to get status even in loaded state
        // if ( m_nObjectState == embed::EmbedStates::LOADED )
        //  changeState( m_nObjectState == embed::EmbedStates::RUNNING );

        m_nStatus = m_pOleComponent->GetMiscStatus( nAspect );
        m_nStatusAspect = nAspect;
        m_bGotStatus = sal_True;
        return m_nStatus;
    }
    else
#endif
    {
        return 0;
        // TODO/LATER: not sure that returning of a default value is better
        // throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Illegal call!\n" ),
        //                          uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }
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


