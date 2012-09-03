/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

#include <rtl/logfile.hxx>
#include <comphelper/componentcontext.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/storagehelper.hxx>


#include <targetstatecontrol.hxx>

#include <olecomponent.hxx>

#include "ownview.hxx"

using namespace ::com::sun::star;

#ifdef WNT
//----------------------------------------------
void OleEmbeddedObject::SwitchComponentToRunningState_Impl()
{
    if ( m_pOleComponent )
    {
        try
        {
            m_pOleComponent->RunObject();
        }
        catch( const embed::UnreachableStateException& )
        {
            GetRidOfComponent();
            throw;
        }
        catch( const embed::WrongStateException& )
        {
            GetRidOfComponent();
            throw;
        }
    }
    else
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
#endif
//----------------------------------------------
void OleEmbeddedObject::MoveListeners()
{
    if ( m_pInterfaceContainer )
    {
        // move state change listeners
        {
            ::cppu::OInterfaceContainerHelper* pStateChangeContainer =
                m_pInterfaceContainer->getContainer( ::getCppuType( ( const uno::Reference< embed::XStateChangeListener >*) NULL ) );
            if ( pStateChangeContainer != NULL )
            {
                uno::Reference< embed::XStateChangeBroadcaster > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
                if ( xWrappedObject.is() )
                {
                    ::cppu::OInterfaceIteratorHelper pIterator( *pStateChangeContainer );
                    while ( pIterator.hasMoreElements() )
                    {
                        try
                        {
                            xWrappedObject->addStateChangeListener( (embed::XStateChangeListener*)pIterator.next() );
                        }
                        catch( const uno::RuntimeException& )
                        {
                            pIterator.remove();
                        }
                    }
                }
            }
        }

        // move event listeners
        {
            ::cppu::OInterfaceContainerHelper* pEventContainer =
                m_pInterfaceContainer->getContainer( ::getCppuType( ( const uno::Reference< document::XEventListener >*) NULL ) );
            if ( pEventContainer != NULL )
            {
                uno::Reference< document::XEventBroadcaster > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
                if ( xWrappedObject.is() )
                {
                    ::cppu::OInterfaceIteratorHelper pIterator( *pEventContainer );
                    while ( pIterator.hasMoreElements() )
                    {
                        try
                        {
                            xWrappedObject->addEventListener( (document::XEventListener*)pIterator.next() );
                        }
                        catch( const uno::RuntimeException& )
                        {
                            pIterator.remove();
                        }
                    }
                }
            }
        }

        // move close listeners
        {
            ::cppu::OInterfaceContainerHelper* pCloseContainer =
                m_pInterfaceContainer->getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
            if ( pCloseContainer != NULL )
            {
                uno::Reference< util::XCloseBroadcaster > xWrappedObject( m_xWrappedObject, uno::UNO_QUERY );
                if ( xWrappedObject.is() )
                {
                    ::cppu::OInterfaceIteratorHelper pIterator( *pCloseContainer );
                    while ( pIterator.hasMoreElements() )
                    {
                        try
                        {
                            xWrappedObject->addCloseListener( (util::XCloseListener*)pIterator.next() );
                        }
                        catch( const uno::RuntimeException& )
                        {
                            pIterator.remove();
                        }
                    }
                }
            }
        }

        delete m_pInterfaceContainer;
        m_pInterfaceContainer = NULL;
    }
}

//----------------------------------------------
uno::Reference< embed::XStorage > OleEmbeddedObject::CreateTemporarySubstorage( ::rtl::OUString& o_aStorageName )
{
    uno::Reference< embed::XStorage > xResult;

    for ( sal_Int32 nInd = 0; nInd < 32000 && !xResult.is(); nInd++ )
    {
        ::rtl::OUString aName = ::rtl::OUString::valueOf( nInd );
        aName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TMPSTOR" ) );
        aName += m_aEntryName;
        if ( !m_xParentStorage->hasByName( aName ) )
        {
            xResult = m_xParentStorage->openStorageElement( aName, embed::ElementModes::READWRITE );
            o_aStorageName = aName;
        }
    }

    if ( !xResult.is() )
    {
        o_aStorageName = ::rtl::OUString();
        throw uno::RuntimeException();
    }

    return xResult;
}

//----------------------------------------------
::rtl::OUString OleEmbeddedObject::MoveToTemporarySubstream()
{
    ::rtl::OUString aResult;
    for ( sal_Int32 nInd = 0; nInd < 32000 && aResult.isEmpty(); nInd++ )
    {
        ::rtl::OUString aName = ::rtl::OUString::valueOf( nInd );
        aName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TMPSTREAM" ) );
        aName += m_aEntryName;
        if ( !m_xParentStorage->hasByName( aName ) )
        {
            m_xParentStorage->renameElement( m_aEntryName, aName );
            aResult = aName;
        }
    }

    if ( aResult.isEmpty() )
        throw uno::RuntimeException();

    return aResult;
}

//----------------------------------------------
sal_Bool OleEmbeddedObject::TryToConvertToOOo()
{
    sal_Bool bResult = sal_False;

    ::rtl::OUString aStorageName;
    ::rtl::OUString aTmpStreamName;
    sal_Int32 nStep = 0;

    if ( m_pOleComponent || m_bReadOnly )
        return sal_False;

    try
    {
        changeState( embed::EmbedStates::LOADED );

        // the stream must be seekable
        uno::Reference< io::XSeekable > xSeekable( m_xObjectStream, uno::UNO_QUERY_THROW );
        xSeekable->seek( 0 );
        ::rtl::OUString aFilterName = OwnView_Impl::GetFilterNameFromExtentionAndInStream( m_xFactory, ::rtl::OUString(), m_xObjectStream->getInputStream() );

        // use the solution only for OOXML format currently
        if ( !aFilterName.isEmpty()
          && ( aFilterName == "Calc MS Excel 2007 XML" || aFilterName == "Impress MS PowerPoint 2007 XML" || aFilterName == "MS Word 2007 XML" ) )
        {
            uno::Reference< container::XNameAccess > xFilterFactory(
                m_xFactory->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.FilterFactory" ) )),
                uno::UNO_QUERY_THROW );

            ::rtl::OUString aDocServiceName;
            uno::Any aFilterAnyData = xFilterFactory->getByName( aFilterName );
            uno::Sequence< beans::PropertyValue > aFilterData;
            if ( aFilterAnyData >>= aFilterData )
            {
                for ( sal_Int32 nInd = 0; nInd < aFilterData.getLength(); nInd++ )
                    if ( aFilterData[nInd].Name == "DocumentService" )
                        aFilterData[nInd].Value >>= aDocServiceName;
            }

            if ( !aDocServiceName.isEmpty() )
            {
                // create the model
                uno::Sequence< uno::Any > aArguments(1);
                aArguments[0] <<= beans::NamedValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EmbeddedObject" ) ), uno::makeAny( (sal_Bool)sal_True ));

                uno::Reference< util::XCloseable > xDocument( m_xFactory->createInstanceWithArguments( aDocServiceName, aArguments ), uno::UNO_QUERY_THROW );
                uno::Reference< frame::XLoadable > xLoadable( xDocument, uno::UNO_QUERY_THROW );
                uno::Reference< document::XStorageBasedDocument > xStorDoc( xDocument, uno::UNO_QUERY_THROW );

                // let the model behave as embedded one
                uno::Reference< frame::XModel > xModel( xDocument, uno::UNO_QUERY_THROW );
                uno::Sequence< beans::PropertyValue > aSeq( 1 );
                aSeq[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "SetEmbedded" ));
                aSeq[0].Value <<= sal_True;
                xModel->attachResource( ::rtl::OUString(), aSeq );

                // load the model from the stream
                uno::Sequence< beans::PropertyValue > aArgs( 5 );
                aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "HierarchicalDocumentName" ));
                aArgs[0].Value <<= m_aEntryName;
                aArgs[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ReadOnly" ));
                aArgs[1].Value <<= sal_True;
                aArgs[2].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
                aArgs[2].Value <<= aFilterName;
                aArgs[3].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "URL" ));
                aArgs[3].Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "private:stream" ));
                aArgs[4].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "InputStream" ));
                aArgs[4].Value <<= m_xObjectStream->getInputStream();

                xSeekable->seek( 0 );
                xLoadable->load( aArgs );

                // the model is successfuly loaded, create a new storage and store the model to the storage
                uno::Reference< embed::XStorage > xTmpStorage = CreateTemporarySubstorage( aStorageName );
                xStorDoc->storeToStorage( xTmpStorage, uno::Sequence< beans::PropertyValue >() );
                xDocument->close( sal_True );
                uno::Reference< beans::XPropertySet > xStorProps( xTmpStorage, uno::UNO_QUERY_THROW );
                ::rtl::OUString aMediaType;
                xStorProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "MediaType" )) ) >>= aMediaType;
                xTmpStorage->dispose();

                // look for the related embedded object factory
                ::comphelper::MimeConfigurationHelper aConfigHelper( m_xFactory );
                ::rtl::OUString aEmbedFactory;
                if ( !aMediaType.isEmpty() )
                    aEmbedFactory = aConfigHelper.GetFactoryNameByMediaType( aMediaType );

                if ( aEmbedFactory.isEmpty() )
                    throw uno::RuntimeException();

                uno::Reference< uno::XInterface > xFact = m_xFactory->createInstance( aEmbedFactory );

                uno::Reference< embed::XEmbedObjectCreator > xEmbCreator( xFact, uno::UNO_QUERY_THROW );

                // now the object should be adjusted to become the wrapper
                nStep = 1;
                uno::Reference< lang::XComponent > xComp( m_xObjectStream, uno::UNO_QUERY_THROW );
                xComp->dispose();
                m_xObjectStream = uno::Reference< io::XStream >();
                m_nObjectState = -1;

                nStep = 2;
                aTmpStreamName = MoveToTemporarySubstream();

                nStep = 3;
                m_xParentStorage->renameElement( aStorageName, m_aEntryName );

                nStep = 4;
                m_xWrappedObject.set( xEmbCreator->createInstanceInitFromEntry( m_xParentStorage, m_aEntryName, uno::Sequence< beans::PropertyValue >(), uno::Sequence< beans::PropertyValue >() ), uno::UNO_QUERY_THROW );

                bResult = sal_True; // the change is no more revertable
                try
                {
                    m_xParentStorage->removeElement( aTmpStreamName );
                }
                catch( const uno::Exception& )
                {
                    // the success of the removing is not so important
                }
            }
        }
    }
    catch( const uno::Exception& )
    {
        // repair the object if necessary
        switch( nStep )
        {
            case 4:
            case 3:
            if ( !aTmpStreamName.isEmpty() && aTmpStreamName != m_aEntryName )
                try
                {
                    if ( m_xParentStorage->hasByName( m_aEntryName ) )
                        m_xParentStorage->removeElement( m_aEntryName );
                    m_xParentStorage->renameElement( aTmpStreamName, m_aEntryName );
                }
                catch ( const uno::Exception& )
                {
                    try {
                        close( sal_True );
                    } catch( const uno::Exception& ) {}

                    m_xParentStorage->dispose(); // ??? the storage has information loss, it should be closed without commiting!
                    throw uno::RuntimeException(); // the repairing is not possible
                }
            case 2:
                try
                {
                    m_xObjectStream = m_xParentStorage->openStreamElement( m_aEntryName, m_bReadOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE );
                    m_nObjectState = embed::EmbedStates::LOADED;
                }
                catch( const uno::Exception& )
                {
                    try {
                        close( sal_True );
                    } catch( const uno::Exception& ) {}

                    throw uno::RuntimeException(); // the repairing is not possible
                }
                // no break as designed!

            case 1:
            case 0:
                if ( !aStorageName.isEmpty() )
                    try {
                        m_xParentStorage->removeElement( aStorageName );
                    } catch( const uno::Exception& ) { OSL_FAIL( "Can not remove temporary storage!" ); }
                break;
        }
    }

    if ( bResult )
    {
        // the conversion was done successfuly, now the additional initializations should happen

        MoveListeners();
        m_xWrappedObject->setClientSite( m_xClientSite );
        if ( m_xParent.is() )
        {
            uno::Reference< container::XChild > xChild( m_xWrappedObject, uno::UNO_QUERY );
            if ( xChild.is() )
                xChild->setParent( m_xParent );
        }

    }

    return bResult;
}

//----------------------------------------------
void SAL_CALL OleEmbeddedObject::changeState( sal_Int32 nNewState )
        throw ( embed::UnreachableStateException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OleEmbeddedObject::changeState" );

    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->changeState( nNewState );
        return;
    }
    // end wrapping related part ====================

    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
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

                    if ( m_pOleComponent && m_bHasSizeToSet )
                    {
                        aGuard.clear();
                        try {
                            m_pOleComponent->SetExtent( m_aSizeToSet, m_nAspectToSet );
                            m_bHasSizeToSet = sal_False;
                        }
                        catch( const uno::Exception& ) {}
                        aGuard.reset();
                    }

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

    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->getReachableStates();
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
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
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->getCurrentState();
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    // TODO: Shouldn't we ask object? ( I guess no )
    return m_nObjectState;
}

namespace
{
    bool lcl_CopyStream(uno::Reference<io::XInputStream> xIn, uno::Reference<io::XOutputStream> xOut)
    {
        const sal_Int32 nChunkSize = 4096;
        uno::Sequence< sal_Int8 > aData(nChunkSize);
        sal_Int32 nTotalRead = 0;
        sal_Int32 nRead;
        do
        {
            nRead = xIn->readBytes(aData, nChunkSize);
            nTotalRead += nRead;
            xOut->writeBytes(aData);
        } while (nRead == nChunkSize);
        return nTotalRead != 0;
    }

    //Dump the objects content to a tempfile, just the "CONTENTS" stream if
    //there is one for non-compound documents, otherwise the whole content.
    //On success a file is returned which must be removed by the caller
    rtl::OUString lcl_ExtractObject(::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory,
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > xObjectStream)
    {
        rtl::OUString sUrl;

        // the solution is only active for Unix systems
#ifndef WNT
        uno::Reference <beans::XPropertySet> xNativeTempFile(
            io::TempFile::create(comphelper::ComponentContext(xFactory).getUNOContext()),
            uno::UNO_QUERY_THROW);
        uno::Reference < io::XStream > xStream(xNativeTempFile, uno::UNO_QUERY_THROW);

        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[0] <<= xObjectStream;
        aArgs[1] <<= (sal_Bool)sal_True; // do not create copy
        uno::Reference< container::XNameContainer > xNameContainer(
            xFactory->createInstanceWithArguments(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.OLESimpleStorage")),
                aArgs ), uno::UNO_QUERY_THROW );

        uno::Reference< io::XStream > xCONTENTS;
        xNameContainer->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CONTENTS"))) >>= xCONTENTS;

        sal_Bool bCopied = xCONTENTS.is() && lcl_CopyStream(xCONTENTS->getInputStream(), xStream->getOutputStream());

        uno::Reference< io::XSeekable > xSeekableStor(xObjectStream, uno::UNO_QUERY);
        if (xSeekableStor.is())
            xSeekableStor->seek(0);

        if (!bCopied)
            bCopied = lcl_CopyStream(xObjectStream->getInputStream(), xStream->getOutputStream());

        if (bCopied)
        {
            xNativeTempFile->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RemoveFile")),
                uno::makeAny(sal_False));
            uno::Any aUrl = xNativeTempFile->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Uri")));
            aUrl >>= sUrl;

            xNativeTempFile = uno::Reference<beans::XPropertySet>();

            uno::Reference < ucb::XSimpleFileAccess2 > xSimpleFileAccess(
                    ucb::SimpleFileAccess::create( comphelper::ComponentContext(xFactory).getUNOContext() ) );

            xSimpleFileAccess->setReadOnly(sUrl, sal_True);
        }
        else
        {
            xNativeTempFile->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RemoveFile")),
                uno::makeAny(sal_True));
        }
#else
        (void) xFactory;
        (void) xObjectStream;
#endif
        return sUrl;
    }
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

    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->doVerb( nVerbID );
        return;
    }
    // end wrapping related part ====================

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
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

    }
    else
#endif
    {
        if ( nVerbID == -9 )
        {
            // the workaround verb to show the object in case no server is available

            // if it is possible, the object will be converted to OOo format
            if ( !m_bTriedConversion )
            {
                m_bTriedConversion = sal_True;
                if ( TryToConvertToOOo() )
                {
                    changeState( embed::EmbedStates::UI_ACTIVE );
                    return;
                }
            }

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
            {
                //Make a RO copy and see if the OS can find something to at
                //least display the content for us
                if (m_aTempDumpURL.isEmpty())
                    m_aTempDumpURL = lcl_ExtractObject(m_xFactory, m_xObjectStream);

                if (!m_aTempDumpURL.isEmpty())
                {
                    uno::Reference< ::com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
                        ::com::sun::star::system::SystemShellExecute::create(comphelper::ComponentContext(m_xFactory).getUNOContext()) );
                    xSystemShellExecute->execute(m_aTempDumpURL, ::rtl::OUString(), ::com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY);
                }
                else
                    throw embed::UnreachableStateException();
            }
        }
        else
        {

            throw embed::UnreachableStateException();
        }
    }
}

//----------------------------------------------
uno::Sequence< embed::VerbDescriptor > SAL_CALL OleEmbeddedObject::getSupportedVerbs()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OleEmbeddedObject::getSupportedVerb" );

    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->getSupportedVerbs();
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
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
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->setClientSite( xClient );
        return;
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_xClientSite != xClient)
    {
        if ( m_nObjectState != embed::EmbedStates::LOADED && m_nObjectState != embed::EmbedStates::RUNNING )
            throw embed::WrongStateException(
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The client site can not be set currently!\n" )),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

        m_xClientSite = xClient;
    }
}

//----------------------------------------------
uno::Reference< embed::XEmbeddedClient > SAL_CALL OleEmbeddedObject::getClientSite()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->getClientSite();
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_xClientSite;
}

//----------------------------------------------
void SAL_CALL OleEmbeddedObject::update()
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->update();
        return;
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
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
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->setUpdateMode( nMode );
        return;
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    OSL_ENSURE( nMode == embed::EmbedUpdateModes::ALWAYS_UPDATE
                    || nMode == embed::EmbedUpdateModes::EXPLICIT_UPDATE,
                "Unknown update mode!\n" );
    m_nUpdateMode = nMode;
}

//----------------------------------------------
sal_Int64 SAL_CALL OleEmbeddedObject::getStatus( sal_Int64
    nAspect
)
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->getStatus( nAspect );
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object must be in running state!\n" )),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    sal_Int64 nResult = 0;

#ifdef WNT
    if ( m_bGotStatus && m_nStatusAspect == nAspect )
        nResult = m_nStatus;
    else if ( m_pOleComponent )
    {

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
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->setContainerName( sName );
        return;
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    m_aContainerName = sName;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
