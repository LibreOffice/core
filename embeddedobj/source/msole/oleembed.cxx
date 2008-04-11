/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: oleembed.cxx,v $
 * $Revision: 1.22 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_embeddedobj.hxx"

#include <oleembobj.hxx>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/NeedsRunningStateException.hpp>
#include <com/sun/star/embed/StateChangeInProgressException.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>


#include <com/sun/star/io/XSeekable.hpp>


#include <com/sun/star/lang/DisposedException.hpp>

#include <rtl/logfile.hxx>


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
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OleEmbeddedObject::changeState" );

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

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

                // the loaded state must be set before, because of notifications!
                m_nObjectState = nNewState;

                {
                    VerbExecutionControllerGuard aVerbGuard( m_aVerbExecutionController );
                    m_pOleComponent->CloseObject();
                }

                // GetRidOfComponent();
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
                            m_pOleComponent->SetExtent( m_aSizeToSet, m_nAspectToSet );
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
                            m_pOleComponent->SetExtent( m_aSizeToSet, m_nAspectToSet );
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
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OleEmbeddedObject::getReachableStates" );

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

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
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

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
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OleEmbeddedObject::doVerb" );

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

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

            // ==== the STAMPIT related solution =============================
            m_aVerbExecutionController.StartControlExecution();
            // ===============================================================

            m_pOleComponent->ExecuteVerb( nVerbID );

            // ==== the STAMPIT related solution =============================
            sal_Bool bModifiedOnExecution = m_aVerbExecutionController.EndControlExecution_WasModified();

            // this workaround is implemented for STAMPIT object
            // if object was modified during verb execution it is saved here
            if ( bModifiedOnExecution && m_pOleComponent->IsDirty() )
                SaveObject_Impl();
            // ===============================================================
        }
        catch( uno::Exception& )
        {
            // ==== the STAMPIT related solution =============================
            m_aVerbExecutionController.EndControlExecution_WasModified();
            // ===============================================================

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
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OleEmbeddedObject::getSupportedVerb" );

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
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
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

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
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

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
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

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
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    OSL_ENSURE( nMode == embed::EmbedUpdateModes::ALWAYS_UPDATE
                    || nMode == embed::EmbedUpdateModes::EXPLICIT_UPDATE,
                "Unknown update mode!\n" );
    m_nUpdateMode = nMode;
}

//----------------------------------------------
sal_Int64 SAL_CALL OleEmbeddedObject::getStatus( sal_Int64
#if defined WNT
    nAspect
#endif
)
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object must be in running state!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    sal_Int64 nResult = 0;

#ifdef WNT
    if ( m_bGotStatus && m_nStatusAspect == nAspect )
        nResult = m_nStatus;
    else if ( m_pOleComponent )
    {
        // OLE should allow to get status even in loaded state
        // if ( m_nObjectState == embed::EmbedStates::LOADED )
        //  changeState( m_nObjectState == embed::EmbedStates::RUNNING );

        m_nStatus = m_pOleComponent->GetMiscStatus( nAspect );
        m_nStatusAspect = nAspect;
        m_bGotStatus = sal_True;
        nResult = m_nStatus;
    }
#endif

    // this implementation needs size to be provided after object loading/creating to work in optimal way
    return ( nResult | embed::EmbedMisc::EMBED_NEEDSSIZEONLOAD );
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


