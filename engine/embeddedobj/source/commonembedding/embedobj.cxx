/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_features.h>

#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/embed/ObjectSaveVetoException.hpp>
#include <com/sun/star/embed/StorageWrappedTargetException.hpp>
#include <com/sun/star/embed/UnreachableStateException.hpp>
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/embed/XWindowSupplier.hpp>
#include <com/sun/star/embed/StateChangeInProgressException.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <com/sun/star/embed/EmbedMisc.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/multicontainer2.hxx>
#include <comphelper/kit.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <officecfg/Office/Common.hxx>
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
#include <svl/documentlockfile.hxx>
#include <svl/msodocumentlockfile.hxx>
#endif
#include <tools/urlobj.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/ucbhelper.hxx>

#include <strings.hrc>
#include <vcl/svapp.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/weld.hxx>

#include <targetstatecontrol.hxx>

#include <commonembobj.hxx>
#include "embedobj.hxx"
#include <specialobject.hxx>
#include <array>

namespace {

// tdf#157943 / tdf#126742: locate the visible top-level frame loaded from
// sLinkURL, or null. Hidden frames are skipped (caller routes them to the
// warn path). Comparison mirrors LoadEnv::impl_searchAlreadyLoaded.
css::uno::Reference< css::frame::XFrame > findLinkSourceFrame(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    const OUString& sLinkURL )
{
    if ( sLinkURL.isEmpty() || INetURLObject( sLinkURL ).IsExoticProtocol() )
        return nullptr;

    try
    {
        css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( xContext );
        css::uno::Reference< css::container::XIndexAccess > xFrames = xDesktop->getFrames();
        if ( !xFrames.is() )
            return nullptr;

        const sal_Int32 nCount = xFrames->getCount();
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            try
            {
                css::uno::Reference< css::frame::XFrame > xFrame;
                xFrames->getByIndex( i ) >>= xFrame;
                if ( !xFrame.is() )
                    continue;

                OUString sFrameURL;
                css::uno::Reference< css::frame::XController > xController = xFrame->getController();
                if ( xController.is() )
                {
                    css::uno::Reference< css::frame::XModel > xModel = xController->getModel();
                    if ( xModel.is() )
                    {
                        // Skip hidden frames. Calling activate() / toFront()
                        // on one would either silently no-op or unexpectedly
                        // reveal a frame meant to stay invisible.
                        bool bHidden = false;
                        for ( const auto& rProp : xModel->getArgs() )
                        {
                            if ( rProp.Name == "Hidden" )
                            {
                                rProp.Value >>= bHidden;
                                break;
                            }
                        }
                        if ( bHidden )
                            continue;

                        sFrameURL = xModel->getURL();
                    }
                }
                else
                {
                    // load may be in progress - URL lives on the frame itself
                    css::uno::Reference< css::beans::XPropertySet > xFrameProps( xFrame, css::uno::UNO_QUERY );
                    if ( xFrameProps.is() )
                        xFrameProps->getPropertyValue( u"URL"_ustr ) >>= sFrameURL;
                }

                if ( sFrameURL.isEmpty() )
                    continue;

                if ( ::utl::UCBContentHelper::EqualURLs( sLinkURL, sFrameURL ) )
                    return xFrame;
            }
            catch ( const css::uno::Exception& )
            {
                // ignore individual frame errors and keep iterating
            }
        }
    }
    catch ( const css::uno::Exception& )
    {
    }
    return nullptr;
}

// Bring an already-loaded source document's frame to the front. Best-effort:
// failure here is harmless because the caller refuses the OLE activation
// regardless, which is the safe outcome.
void switchToExistingFrame( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    if ( !xFrame.is() )
        return;

    try
    {
        xFrame->activate();
        css::uno::Reference< css::awt::XTopWindow > xTopWindow( xFrame->getContainerWindow(), css::uno::UNO_QUERY );
        if ( xTopWindow.is() )
            xTopWindow->toFront();
    }
    catch ( const css::uno::Exception& )
    {
    }
}

// tdf#157943: any lock - foreign, hidden own frame, or stale own lock -
// would fail the OLE writeback at save time, so the caller refuses on
// any non-empty result. Visible-own-frame is filtered out earlier.
// On non-multiuser builds (Android/iOS) the svt::*DocumentLockFile
// symbols aren't linked, so the body short-circuits to "" - the caller
// just skips case 2 naturally without needing a separate gate.
OUString getSourceLockOwner( std::u16string_view sLinkURL )
{
    if ( sLinkURL.empty() || INetURLObject( sLinkURL ).IsExoticProtocol() )
        return u""_ustr;

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
    auto formatOwner = []( const LockFileEntry& aData, bool bMSO )
    {
        OUString sOwner = aData[LockFileComponent::OOOUSERNAME];
        if ( sOwner.isEmpty() )
            sOwner = aData[LockFileComponent::SYSUSERNAME];
        if ( sOwner.isEmpty() )
            return u""_ustr; // empty/corrupt lock data - treat as no lock
        if ( bMSO )
            sOwner += " (MS Office)";
        return sOwner;
    };

    // LO format (.~lock.*#)
    try
    {
        svt::DocumentLockFile aLockFile( sLinkURL );
        if ( OUString sOwner = formatOwner( aLockFile.GetLockData(), false );
             !sOwner.isEmpty() )
            return sOwner;
    }
    catch ( const css::uno::Exception& )
    {
    }

    // MSO format (~$*) - LO understands these via tdf#34171
    if ( svt::MSODocumentLockFile::IsMSOSupportedFileFormat( sLinkURL ) )
    {
        try
        {
            svt::MSODocumentLockFile aMSOLockFile( sLinkURL );
            if ( OUString sOwner = formatOwner( aMSOLockFile.GetLockData(), true );
                 !sOwner.isEmpty() )
                return sOwner;
        }
        catch ( const css::uno::Exception& )
        {
        }
    }
#endif

    return u""_ustr;
}

void showLinkSourceLockedDialog( const css::uno::Reference< css::awt::XWindow >& xClientWindow,
                                 std::u16string_view sLinkURL,
                                 std::u16string_view sOwner )
{
    std::locale aResLocale = Translate::Create( "emo" );
    OUString aMsg = Translate::get( STR_LINK_SOURCE_LOCKED, aResLocale );

    OUString aLinkURL( sLinkURL );
    OUString aSysPath = aLinkURL;
    osl::FileBase::getSystemPathFromFileURL( aLinkURL, aSysPath );
    aMsg = aMsg.replaceFirst( "%{filename}", aSysPath );
    aMsg = aMsg.replaceFirst( "%{owner}", sOwner );

    weld::Window* pParent = Application::GetFrameWeld( xClientWindow );
    std::shared_ptr< weld::MessageDialog > xQueryBox(
        Application::CreateMessageDialog( pParent,
            VclMessageType::Warning, VclButtonsType::Ok, aMsg ) );
    xQueryBox->runAsync( xQueryBox, []( sal_Int32 ) {} );
}

} // anonymous namespace

using namespace ::com::sun::star;

awt::Rectangle GetRectangleInterception( const awt::Rectangle& aRect1, const awt::Rectangle& aRect2 )
{
    awt::Rectangle aResult;

    OSL_ENSURE( aRect1.Width >= 0 && aRect2.Width >= 0 && aRect1.Height >= 0 && aRect2.Height >= 0,
                "Offset must not be less than zero!" );

    aResult.X = std::max(aRect1.X, aRect2.X);
    aResult.Y = std::max(aRect1.Y, aRect2.Y);

    sal_Int32 nRight1 = aRect1.X + aRect1.Width;
    sal_Int32 nBottom1 = aRect1.Y + aRect1.Height;
    sal_Int32 nRight2 = aRect2.X + aRect2.Width;
    sal_Int32 nBottom2 = aRect2.Y + aRect2.Height;
    aResult.Width = std::min( nRight1, nRight2 ) - aResult.X;
    aResult.Height = std::min( nBottom1, nBottom2 ) - aResult.Y;

    return aResult;
}

namespace
{
    using IntermediateStatesMap = std::array<std::array<uno::Sequence< sal_Int32 >, NUM_SUPPORTED_STATES>, NUM_SUPPORTED_STATES>;
    const IntermediateStatesMap & getIntermediateStatesMap()
    {
        static const IntermediateStatesMap map = [] () {
            IntermediateStatesMap tmp;

            // intermediate states
            // In the following table the first index points to starting state,
            // the second one to the target state, and the sequence referenced by
            // first two indexes contains intermediate states, that should be
            // passed by object to reach the target state.
            // If the sequence is empty that means that indirect switch from start
            // state to the target state is forbidden, only if direct switch is possible
            // the state can be reached.

            tmp[0][2] = { embed::EmbedStates::RUNNING };

            tmp[0][3] = { embed::EmbedStates::RUNNING,
                                                embed::EmbedStates::INPLACE_ACTIVE };

            tmp[0][4] = {embed::EmbedStates::RUNNING};

            tmp[1][3] = { embed::EmbedStates::INPLACE_ACTIVE };

            tmp[2][0] = { embed::EmbedStates::RUNNING };

            tmp[3][0] = { embed::EmbedStates::INPLACE_ACTIVE,
                                                embed::EmbedStates::RUNNING };

            tmp[3][1] = { embed::EmbedStates::INPLACE_ACTIVE };

            tmp[4][0] = { embed::EmbedStates::RUNNING };

            return tmp;
        }();
        return map;
    }

    // accepted states
    const css::uno::Sequence< sal_Int32 > & getAcceptedStates()
    {
        static const css::uno::Sequence< sal_Int32 > states {
            /* [0] */ embed::EmbedStates::LOADED,
                          /* [1] */ embed::EmbedStates::RUNNING,
                          /* [2] */ embed::EmbedStates::INPLACE_ACTIVE,
                          /* [3] */ embed::EmbedStates::UI_ACTIVE,
                          /* [4] */ embed::EmbedStates::ACTIVE };
        assert(states.getLength() == NUM_SUPPORTED_STATES);
        return states;
    }

}

sal_Int32 OCommonEmbeddedObject::ConvertVerbToState_Impl( sal_Int32 nVerb )
{
    auto it = m_aVerbTable.find( nVerb );
    if (it != m_aVerbTable.end())
        return it->second;

    throw lang::IllegalArgumentException(); // TODO: unexpected verb provided
}


void OCommonEmbeddedObject::Deactivate()
{
    uno::Reference< util::XModifiable > xModif( m_xDocHolder->GetComponent(), uno::UNO_QUERY );

    // no need to lock for the initialization
    uno::Reference< embed::XEmbeddedClient > xClientSite = m_xClientSite;
    if ( !xClientSite.is() )
        throw embed::WrongStateException(); //TODO: client site is not set!

    // tdf#131146 close frame before saving of the document
    // (during CloseFrame() call some changes could be detected not registered in util::XModifiable)
    m_xDocHolder->CloseFrame();

    // store document if it is modified
    if ( xModif.is() && xModif->isModified() )
    {
        try {
            xClientSite->saveObject();

            // tdf#141529 take note that an eventually used linked file
            // got changed/saved/written and that we need to copy it back if the
            // hosting file/document gets saved
            if(m_aLinkTempFile.is())
                m_bLinkTempFileChanged = true;
        }
        catch( const embed::ObjectSaveVetoException& )
        {
        }
        catch( const uno::Exception& )
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw embed::StorageWrappedTargetException(
                u"The client could not store the object!"_ustr,
                static_cast< ::cppu::OWeakObject* >( this ),
                anyEx );
        }
    }

    xClientSite->visibilityChanged( false );
}


void OCommonEmbeddedObject::StateChangeNotification_Impl( bool bBeforeChange, sal_Int32 nOldState, sal_Int32 nNewState ,::osl::ResettableMutexGuard& rGuard )
{
    if ( !m_pInterfaceContainer )
        return;

    comphelper::OInterfaceContainerHelper2* pContainer = m_pInterfaceContainer->getContainer(
                        cppu::UnoType<embed::XStateChangeListener>::get());
    if ( pContainer == nullptr )
        return;

    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );
    comphelper::OInterfaceIteratorHelper2 pIterator(*pContainer);

    // should be locked after the method is finished successfully
    rGuard.clear();

    while (pIterator.hasMoreElements())
    {
        try
        {
            if ( bBeforeChange )
                static_cast<embed::XStateChangeListener*>(pIterator.next())->changingState( aSource, nOldState, nNewState );
            else
                static_cast<embed::XStateChangeListener*>(pIterator.next())->stateChanged( aSource, nOldState, nNewState );
        }
        catch( const uno::Exception& )
        {
            // even if the listener complains ignore it for now
           }

        if ( m_bDisposed )
            return;
    }

    rGuard.reset();
}

void OCommonEmbeddedObject::SetInplaceActiveState()
{
    if ( !m_xClientSite.is() )
        throw embed::WrongStateException( u"client site not set, yet"_ustr, *this );

    uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY );
    if ( !xInplaceClient.is() || !xInplaceClient->canInplaceActivate() )
        throw embed::WrongStateException(); //TODO: can't activate inplace
    xInplaceClient->activatingInplace();

    uno::Reference< embed::XWindowSupplier > xClientWindowSupplier( xInplaceClient, uno::UNO_QUERY_THROW );

    m_xClientWindow = xClientWindowSupplier->getWindow();
    m_aOwnRectangle = xInplaceClient->getPlacement();
    m_aClipRectangle = xInplaceClient->getClipRectangle();
    awt::Rectangle aRectangleToShow = GetRectangleInterception( m_aOwnRectangle, m_aClipRectangle );

    // create own window based on the client window
    // place and resize the window according to the rectangles
    uno::Reference< awt::XWindowPeer > xClientWindowPeer( m_xClientWindow, uno::UNO_QUERY_THROW );

    // dispatch provider may not be provided
    uno::Reference< frame::XDispatchProvider > xContainerDP = xInplaceClient->getInplaceDispatchProvider();
    bool bOk = m_xDocHolder->ShowInplace( xClientWindowPeer, aRectangleToShow, xContainerDP );
    m_nObjectState = embed::EmbedStates::INPLACE_ACTIVE;
    if ( !bOk )
    {
        SwitchStateTo_Impl( embed::EmbedStates::RUNNING );
        throw embed::WrongStateException(); //TODO: can't activate inplace
    }
}

void OCommonEmbeddedObject::SwitchStateTo_Impl( sal_Int32 nNextState )
{
    // TODO: may be needs interaction handler to detect whether the object state
    //         can be changed even after errors

    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        if ( nNextState == embed::EmbedStates::RUNNING )
        {
            // after the object reaches the running state the cloned size is not necessary any more
            m_bHasClonedSize = false;

            if ( m_bIsLinkURL )
            {
                m_xDocHolder->SetComponent( LoadLink_Impl(), m_bReadOnly );
            }
            else
            {
                if ( !dynamic_cast<OSpecialEmbeddedObject*>(this) )
                {
                    // in case embedded object is in loaded state the contents must
                    // be stored in the related storage and the storage
                    // must be created already
                    if ( !m_xObjectStorage.is() )
                        throw io::IOException(); //TODO: access denied

                    m_xDocHolder->SetComponent( LoadDocumentFromStorage_Impl(), m_bReadOnly );
                }
                else
                {
                    // objects without persistence will be initialized internally
                    uno::Sequence < uno::Any > aArgs{ uno::Any(
                        uno::Reference < embed::XEmbeddedObject >( this )) };
                    uno::Reference< util::XCloseable > xDocument(
                            m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( GetDocumentServiceName(), aArgs, m_xContext),
                            uno::UNO_QUERY );

                    uno::Reference < container::XChild > xChild( xDocument, uno::UNO_QUERY );
                    if ( xChild.is() )
                        xChild->setParent( m_xParent );

                    m_xDocHolder->SetComponent( xDocument, m_bReadOnly );
                }
            }

            if ( !m_xDocHolder->GetComponent().is() )
                throw embed::UnreachableStateException(); //TODO: can't open document

            m_nObjectState = nNextState;
        }
        else
        {
            SAL_WARN( "embeddedobj.common", "Unacceptable state switch!" );
            throw uno::RuntimeException(u"invalid next state, only RUNNING state allowed"_ustr); // TODO
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::RUNNING )
    {
        if ( nNextState == embed::EmbedStates::LOADED )
        {
            m_nClonedMapUnit = m_xDocHolder->GetMapUnit( embed::Aspects::MSOLE_CONTENT );
            m_bHasClonedSize = m_xDocHolder->GetExtent( embed::Aspects::MSOLE_CONTENT, &m_aClonedSize );

            // actually frame should not exist at this point
            m_xDocHolder->CloseDocument( false, false );

            m_nObjectState = nNextState;
        }
        else
        {
            if ( nNextState == embed::EmbedStates::INPLACE_ACTIVE )
            {
                SetInplaceActiveState();
            }
            else if ( nNextState == embed::EmbedStates::ACTIVE )
            {
                if ( !m_xClientSite.is() )
                    throw embed::WrongStateException(); //TODO: client site is not set!

                // create frame and load document in the frame
                m_xDocHolder->Show();

                m_xClientSite->visibilityChanged( true );
                m_nObjectState = nNextState;
            }
            else
            {
                SAL_WARN( "embeddedobj.common", "Unacceptable state switch!" );
                throw uno::RuntimeException(u"invalid next state,only LOADED/INPLACE_ACTIVE/ACTIVE allowed"_ustr); // TODO
            }
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::INPLACE_ACTIVE )
    {
        if ( nNextState == embed::EmbedStates::RUNNING )
        {
            uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY_THROW );

            m_xClientSite->visibilityChanged( true );

            xInplaceClient->deactivatedInplace();
            Deactivate();
            m_nObjectState = nNextState;
        }
        else if ( nNextState == embed::EmbedStates::UI_ACTIVE )
        {
            if ( !(m_nMiscStatus & embed::EmbedMisc::MS_EMBED_NOUIACTIVATE) )
            {
                uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY_THROW );
                // TODO:
                uno::Reference< css::frame::XLayoutManager > xContainerLM =
                            xInplaceClient->getLayoutManager();
                if ( !xContainerLM.is() )
                    throw embed::WrongStateException(); //TODO: can't activate UI
                // dispatch provider may not be provided
                uno::Reference< frame::XDispatchProvider > xContainerDP = xInplaceClient->getInplaceDispatchProvider();

                // get the container module name
                OUString aModuleName;
                try
                {
                    uno::Reference< embed::XComponentSupplier > xCompSupl( m_xClientSite, uno::UNO_QUERY_THROW );
                    uno::Reference< uno::XInterface > xContDoc( xCompSupl->getComponent(), uno::UNO_QUERY_THROW );

                    uno::Reference< frame::XModuleManager2 > xManager( frame::ModuleManager::create( m_xContext ) );

                    aModuleName = xManager->identify( xContDoc );
                }
                catch( const uno::Exception& )
                {}

                if (!comphelper::COKit::isActive())
                {
                    // if currently another object is UIactive it will be deactivated; usually this will activate the LM of
                    // the container. Locking the LM will prevent flicker.
                    xContainerLM->lock();
                    xInplaceClient->activatingUI();
                    bool bOk = m_xDocHolder->ShowUI( xContainerLM, xContainerDP, aModuleName );
                    xContainerLM->unlock();

                    if ( bOk )
                    {
                        m_nObjectState = nNextState;
                        m_xDocHolder->ResizeHatchWindow();
                    }
                    else
                    {
                        xInplaceClient->deactivatedUI();
                        throw embed::WrongStateException(); //TODO: can't activate UI
                    }
                }
            }
        }
        else
        {
            SAL_WARN( "embeddedobj.common", "Unacceptable state switch!" );
            throw uno::RuntimeException(u"invalid next state,only RUNNING/UI_ACTIVE allowed"_ustr); // TODO
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::ACTIVE )
    {
        if ( nNextState == embed::EmbedStates::RUNNING )
        {
            Deactivate();
            m_nObjectState = nNextState;
        }
        else
        {
            SAL_WARN( "embeddedobj.common", "Unacceptable state switch!" );
            throw uno::RuntimeException(u"invalid next state, only RUNNING state allowed"_ustr); // TODO
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::UI_ACTIVE )
    {
        if ( nNextState == embed::EmbedStates::INPLACE_ACTIVE )
        {
            uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY_THROW );
            uno::Reference< css::frame::XLayoutManager > xContainerLM =
                        xInplaceClient->getLayoutManager();

            bool bOk = false;
            if ( xContainerLM.is() )
                bOk = m_xDocHolder->HideUI( xContainerLM );

            if ( !bOk )
                throw embed::WrongStateException(); //TODO: can't activate UI
            m_nObjectState = nNextState;
            m_xDocHolder->ResizeHatchWindow();
            xInplaceClient->deactivatedUI();
        }
    }
    else
        throw embed::WrongStateException( u"The object is in unacceptable state!"_ustr,
                                          static_cast< ::cppu::OWeakObject* >(this) );
}


uno::Sequence< sal_Int32 > const & OCommonEmbeddedObject::GetIntermediateStatesSequence_Impl( sal_Int32 nNewState )
{
    sal_Int32 nCurInd = 0;
    auto & rAcceptedStates = getAcceptedStates();
    for ( nCurInd = 0; nCurInd < rAcceptedStates.getLength(); nCurInd++ )
        if ( rAcceptedStates[nCurInd] == m_nObjectState )
            break;

    if ( nCurInd == rAcceptedStates.getLength() )
        throw embed::WrongStateException( u"The object is in unacceptable state!"_ustr,
                                          static_cast< ::cppu::OWeakObject* >(this) );

    sal_Int32 nDestInd = 0;
    for ( nDestInd = 0; nDestInd < rAcceptedStates.getLength(); nDestInd++ )
        if ( rAcceptedStates[nDestInd] == nNewState )
            break;

    if ( nDestInd == rAcceptedStates.getLength() )
        throw embed::UnreachableStateException(
            u"The state either not reachable, or the object allows the state only as an intermediate one!"_ustr,
            static_cast< ::cppu::OWeakObject* >(this),
            m_nObjectState,
            nNewState );

    return getIntermediateStatesMap()[nCurInd][nDestInd];
}


void SAL_CALL OCommonEmbeddedObject::changeState( sal_Int32 nNewState )
{
    if ( officecfg::Office::Common::Security::Scripting::DisableActiveContent::get()
        && nNewState != embed::EmbedStates::LOADED )
        throw embed::UnreachableStateException();
    // To avoid deadlock, take the SolarMutex before m_aMutex, as changeState's body eventually
    // reaches VCL (e.g. toolbar disposal in DocumentHolder::ShowUI -> ToolbarController::disposing)
    // which acquires SolarMutex, while VCL-side callers like SwView::SetVisArea ->
    // SfxInPlaceClient::VisAreaChanged -> OCommonEmbeddedObject::setObjectRectangles already hold
    // SolarMutex and then want m_aMutex:
    SolarMutexGuard solarGuard;
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( u"The object has no persistence!"_ustr,
                                          static_cast< ::cppu::OWeakObject* >(this) );

    sal_Int32 nOldState = m_nObjectState;

    if ( m_nTargetState != -1 )
    {
        // means that the object is currently trying to reach the target state
        throw embed::StateChangeInProgressException( OUString(),
                                                    uno::Reference< uno::XInterface >(),
                                                    m_nTargetState );
    }
    else
    {
        TargetStateControl_Impl aControl( m_nTargetState, nNewState );

        // in case the object is already in requested state
        if ( m_nObjectState == nNewState )
        {
            // if active object is activated again, bring its window to top
            if ( m_nObjectState == embed::EmbedStates::ACTIVE )
                m_xDocHolder->Show();

            return;
        }

        // retrieve sequence of states that should be passed to reach desired state
        uno::Sequence< sal_Int32 > aIntermediateStates = GetIntermediateStatesSequence_Impl( nNewState );

        // notify listeners that the object is going to change the state
        StateChangeNotification_Impl( true, nOldState, nNewState,aGuard );

        try {
            for (sal_Int32 state : aIntermediateStates)
                SwitchStateTo_Impl( state );

            SwitchStateTo_Impl( nNewState );
        }
        catch( const uno::Exception& )
        {
            if ( nOldState != m_nObjectState )
                // notify listeners that the object has changed the state
                StateChangeNotification_Impl( false, nOldState, m_nObjectState, aGuard );

            throw;
        }
    }

    // notify listeners that the object has changed the state
    StateChangeNotification_Impl( false, nOldState, nNewState, aGuard );

    // let the object window be shown
    if ( nNewState == embed::EmbedStates::UI_ACTIVE || nNewState == embed::EmbedStates::INPLACE_ACTIVE )
        PostEvent_Impl( u"OnVisAreaChanged"_ustr );
}


uno::Sequence< sal_Int32 > SAL_CALL OCommonEmbeddedObject::getReachableStates()
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( u"The object has no persistence!"_ustr,
                                           static_cast< ::cppu::OWeakObject* >(this) );

    return getAcceptedStates();
}


sal_Int32 SAL_CALL OCommonEmbeddedObject::getCurrentState()
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( u"The object has no persistence!"_ustr,
                                          static_cast< ::cppu::OWeakObject* >(this) );

    return m_nObjectState;
}


void SAL_CALL OCommonEmbeddedObject::doVerb( sal_Int32 nVerbID )
{
    SolarMutexGuard aSolarGuard;
        //TODO: a gross hack to avoid deadlocks when this is called from the
        // outside and OCommonEmbeddedObject::changeState, with m_aMutex locked,
        // calls into framework code that tries to lock the solar mutex, while
        // another thread (through Window::ImplCallPaint, say) calls
        // OCommonEmbeddedObject::getComponent with the solar mutex locked and
        // then tries to lock m_aMutex (see fdo#56818); the alternative would be
        // to get locking done right in this class, but that looks like a
        // daunting task

    osl::ClearableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( u"The object has no persistence!"_ustr,
                                          static_cast< ::cppu::OWeakObject* >(this) );

    // for internal documents this call is just a duplicate of changeState
    sal_Int32 nNewState = -1;
    try
    {
        nNewState = ConvertVerbToState_Impl( nVerbID );
    }
    catch( const uno::Exception& )
    {}

    if ( nNewState == -1 )
    {
        // TODO/LATER: Save Copy as... verb ( -8 ) is implemented by container
        // TODO/LATER: check if the verb is a supported one and if it is produce related operation
    }
    else
    {
        // tdf#157943 / tdf#126742: refuse user-initiated activation of an
        // OLE link whose source is in use - either navigate to the visible
        // frame (matches MSO) or warn on any lock (foreign, hidden own
        // frame, or stale). Gated to visible-state verbs so internal
        // RUNNING transitions aren't redirected. Skipped in headless.
        if ( m_bIsLinkURL
             && ( nNewState == embed::EmbedStates::INPLACE_ACTIVE
                  || nNewState == embed::EmbedStates::UI_ACTIVE
                  || nNewState == embed::EmbedStates::ACTIVE )
             && !Application::IsHeadlessModeEnabled() )
        {
            // snapshot under m_aMutex so the framework calls below run on
            // stable values even if breakLink/reload races on another thread
            const OUString aLinkURL = m_aLinkURL;
            const auto xClientWindow = m_xClientWindow;
            const auto xContext = m_xContext;
            aGuard.clear();

            // 1. Source loaded as a visible frame here: bring it to front.
            // Matches MSO's "navigate to existing editor" behaviour.
            if ( auto xExistingFrame = findLinkSourceFrame( xContext, aLinkURL );
                 xExistingFrame.is() )
            {
                switchToExistingFrame( xExistingFrame );
                return;
            }

            // 2. Lock file present (foreign, hidden own frame, or stale
            // own lock from a crash) - all fail writeback at save time.
            // On non-multiuser builds getSourceLockOwner returns "" and
            // this branch is naturally skipped - no #if needed here.
            if ( OUString sLockOwner = getSourceLockOwner( aLinkURL );
                 !sLockOwner.isEmpty() )
            {
                showLinkSourceLockedDialog( xClientWindow, aLinkURL, sLockOwner );
                return;
            }
        }
        else
        {
            aGuard.clear();
        }

        changeState( nNewState );
    }
}


uno::Sequence< embed::VerbDescriptor > SAL_CALL OCommonEmbeddedObject::getSupportedVerbs()
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( u"The object has no persistence!"_ustr,
                                          static_cast< ::cppu::OWeakObject* >(this) );

    return m_aObjectVerbs;
}


void SAL_CALL OCommonEmbeddedObject::setClientSite(
                const uno::Reference< embed::XEmbeddedClient >& xClient )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_xClientSite != xClient)
    {
        if ( m_nObjectState != embed::EmbedStates::LOADED && m_nObjectState != embed::EmbedStates::RUNNING )
            throw embed::WrongStateException(
                                    u"The client site can not be set currently!"_ustr,
                                     static_cast< ::cppu::OWeakObject* >(this) );

        m_xClientSite = xClient;
    }
}


uno::Reference< embed::XEmbeddedClient > SAL_CALL OCommonEmbeddedObject::getClientSite()
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( u"The object has no persistence!"_ustr,
                                          static_cast< ::cppu::OWeakObject* >(this) );

    return m_xClientSite;
}


void SAL_CALL OCommonEmbeddedObject::update()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( u"The object has no persistence!"_ustr,
                                          static_cast< ::cppu::OWeakObject* >(this) );

    PostEvent_Impl( u"OnVisAreaChanged"_ustr );
}


void SAL_CALL OCommonEmbeddedObject::setUpdateMode( sal_Int32 nMode )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( u"The object has no persistence!"_ustr,
                                          static_cast< ::cppu::OWeakObject* >(this) );

    OSL_ENSURE( nMode == embed::EmbedUpdateModes::ALWAYS_UPDATE
                    || nMode == embed::EmbedUpdateModes::EXPLICIT_UPDATE,
                "Unknown update mode!" );
    m_nUpdateMode = nMode;
}


sal_Int64 SAL_CALL OCommonEmbeddedObject::getStatus( sal_Int64 )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_nMiscStatus;
}


void SAL_CALL OCommonEmbeddedObject::setContainerName( const OUString& sName )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    m_aContainerName = sName;
}

void OCommonEmbeddedObject::SetOleState(bool bIsOleUpdate)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    m_bOleUpdate = bIsOleUpdate;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL OCommonEmbeddedObject::getParent()
{
    return m_xParent;
}

void SAL_CALL OCommonEmbeddedObject::setParent( const css::uno::Reference< css::uno::XInterface >& xParent )
{
    m_xParent = xParent;
    if ( m_nObjectState != -1 && m_nObjectState != embed::EmbedStates::LOADED )
    {
        uno::Reference < container::XChild > xChild( m_xDocHolder->GetComponent(), uno::UNO_QUERY );
        if ( xChild.is() )
            xChild->setParent( xParent );
    }
}

// XDefaultSizeTransmitter
void SAL_CALL OCommonEmbeddedObject::setDefaultSize( const css::awt::Size& rSize_100TH_MM )
{
    //#i103460# charts do not necessarily have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this method
    m_aDefaultSizeForChart_In_100TH_MM = rSize_100TH_MM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
