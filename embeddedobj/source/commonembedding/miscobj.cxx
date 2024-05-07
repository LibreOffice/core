/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <commonembobj.hxx>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <comphelper/multicontainer2.hxx>
#include <comphelper/storagehelper.hxx>

#include <cppuhelper/queryinterface.hxx>
#include <comphelper/mimeconfighelper.hxx>

#include <utility>
#include <vcl/weld.hxx>
#include <vcl/stdtext.hxx>
#include <strings.hrc>
#include <osl/file.hxx>
#include <comphelper/DirectoryHelper.hxx>

#include <vcl/svapp.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include "persistence.hxx"

#include <cassert>

using namespace ::com::sun::star;


OCommonEmbeddedObject::OCommonEmbeddedObject( uno::Reference< uno::XComponentContext > xContext,
                                                const uno::Sequence< beans::NamedValue >& aObjProps )
: m_bReadOnly( false )
, m_bDisposed( false )
, m_bClosed( false )
, m_nObjectState( -1 )
, m_nTargetState( -1 )
, m_nUpdateMode ( embed::EmbedUpdateModes::ALWAYS_UPDATE )
, m_xContext(std::move( xContext ))
, m_nMiscStatus( 0 )
, m_bEmbeddedScriptSupport( true )
, m_bDocumentRecoverySupport( true )
, m_bWaitSaveCompleted( false )
, m_bIsLinkURL( false )
, m_bLinkTempFileChanged( false )
, m_pLinkFile( )
, m_bOleUpdate( false )
, m_bInHndFunc( false )
, m_bLinkHasPassword( false )
, m_aLinkTempFile( )
, m_bHasClonedSize( false )
, m_nClonedMapUnit( 0 )
{
    CommonInit_Impl( aObjProps );
}


OCommonEmbeddedObject::OCommonEmbeddedObject(
        uno::Reference< uno::XComponentContext > xContext,
        const uno::Sequence< beans::NamedValue >& aObjProps,
        const uno::Sequence< beans::PropertyValue >& aMediaDescr,
        const uno::Sequence< beans::PropertyValue >& aObjectDescr )
: m_bReadOnly( false )
, m_bDisposed( false )
, m_bClosed( false )
, m_nObjectState( embed::EmbedStates::LOADED )
, m_nTargetState( -1 )
, m_nUpdateMode ( embed::EmbedUpdateModes::ALWAYS_UPDATE )
, m_xContext(std::move( xContext ))
, m_nMiscStatus( 0 )
, m_bEmbeddedScriptSupport( true )
, m_bDocumentRecoverySupport( true )
, m_bWaitSaveCompleted( false )
, m_bIsLinkURL( true )
, m_bLinkTempFileChanged( false )
, m_pLinkFile( )
, m_bOleUpdate( false )
, m_bInHndFunc( false )
, m_bLinkHasPassword( false )
, m_aLinkTempFile( )
, m_bHasClonedSize( false )
, m_nClonedMapUnit( 0 )
{
    // linked object has no own persistence so it is in loaded state starting from creation
    LinkInit_Impl( aObjProps, aMediaDescr, aObjectDescr );
}


void OCommonEmbeddedObject::CommonInit_Impl( const uno::Sequence< beans::NamedValue >& aObjectProps )
{
    OSL_ENSURE( m_xContext.is(), "No ServiceFactory is provided!" );
    if ( !m_xContext.is() )
        throw uno::RuntimeException();

    m_xDocHolder = new embeddedobj::DocumentHolder( m_xContext, this );

    // parse configuration entries
    // TODO/LATER: in future UI names can be also provided here
    for ( beans::NamedValue const & prop : aObjectProps )
    {
        if ( prop.Name == "ClassID" )
            prop.Value >>= m_aClassID;
        else if ( prop.Name == "ObjectDocumentServiceName" )
            prop.Value >>= m_aDocServiceName;
        else if ( prop.Name == "ObjectDocumentFilterName" )
            prop.Value >>= m_aPresetFilterName;
        else if ( prop.Name == "ObjectMiscStatus" )
            prop.Value >>= m_nMiscStatus;
        else if ( prop.Name == "ObjectVerbs" )
            prop.Value >>= m_aObjectVerbs;
    }

    if ( m_aClassID.getLength() != 16 /*|| !m_aDocServiceName.getLength()*/ )
        throw uno::RuntimeException(); // something goes really wrong

    // verbs table
    for (auto const& verb : m_aObjectVerbs)
    {
        if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_PRIMARY )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::UI_ACTIVE } );
        }
        else if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_SHOW )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::UI_ACTIVE } );
        }
        else if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_OPEN )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::ACTIVE } );
        }
        else if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_IPACTIVATE )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::INPLACE_ACTIVE } );
        }
        else if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_UIACTIVATE )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::UI_ACTIVE } );
        }
        else if ( verb.VerbID == embed::EmbedVerbs::MS_OLEVERB_HIDE )
        {
            m_aVerbTable.insert( { verb.VerbID, embed::EmbedStates::RUNNING } );
        }
    }
}


void OCommonEmbeddedObject::LinkInit_Impl(
                                const uno::Sequence< beans::NamedValue >& aObjectProps,
                                const uno::Sequence< beans::PropertyValue >& aMediaDescr,
                                const uno::Sequence< beans::PropertyValue >& aObjectDescr )
{
    // setPersistance has no effect on own links, so the complete initialization must be done here

    for ( beans::PropertyValue const & prop : aMediaDescr )
        if ( prop.Name == "URL" )
            prop.Value >>= m_aLinkURL;
        else if ( prop.Name == "FilterName" )
            prop.Value >>= m_aLinkFilterName;

    OSL_ENSURE( m_aLinkURL.getLength() && m_aLinkFilterName.getLength(), "Filter and URL must be provided!" );

    m_bReadOnly = true;
    if ( m_aLinkFilterName.getLength() )
    {
        ::comphelper::MimeConfigurationHelper aHelper( m_xContext );
        OUString aExportFilterName = aHelper.GetExportFilterFromImportFilter( m_aLinkFilterName );
        m_bReadOnly = aExportFilterName != m_aLinkFilterName;
    }

    if(m_bIsLinkURL && !m_bReadOnly)
    {
        // tdf#141529 we have a linked OLE object. To prevent the original OLE
        // data to be changed each time the OLE gets changed (at deactivate), copy it to
        // a temporary file. That file will be changed on activated OLE changes then.
        // The moment the original gets changed itself will now be associated with the
        // file/document embedding the OLE being changed (see other additions to the
        // task-ID above)
        //
        // open OLE original data as read input file
        if ( comphelper::DirectoryHelper::fileExists( m_aLinkURL ) )
        {
            // create temporary file
            m_aLinkTempFile = io::TempFile::create( m_xContext );

            m_pLinkFile.reset( new FileChangedChecker( m_aLinkURL ) );
            handleLinkedOLE( CopyBackToOLELink::CopyLinkToTempInit );
        }
    }

    if(m_aLinkTempFile.is())
    {
        uno::Sequence< beans::PropertyValue > aAlternativeMediaDescr(aMediaDescr.getLength());
        auto aAlternativeMediaDescrRange = asNonConstRange(aAlternativeMediaDescr);

        for ( sal_Int32 a(0); a < aMediaDescr.getLength(); a++ )
        {
            const beans::PropertyValue& rSource(aMediaDescr[a]);
            beans::PropertyValue& rDestination(aAlternativeMediaDescrRange[a]);

            rDestination.Name = rSource.Name;
            if(rSource.Name == "URL")
                rDestination.Value <<= m_aLinkTempFile->getUri();
            else
                rDestination.Value = rSource.Value;
        }

        m_aDocMediaDescriptor = GetValuableArgs_Impl( aAlternativeMediaDescr, false );
    }
    else
    {
        m_aDocMediaDescriptor = GetValuableArgs_Impl( aMediaDescr, false );
    }

    uno::Reference< frame::XDispatchProviderInterceptor > xDispatchInterceptor;
    for ( beans::PropertyValue const & prop : aObjectDescr )
        if ( prop.Name == "OutplaceDispatchInterceptor" )
        {
            prop.Value >>= xDispatchInterceptor;
            break;
        }
        else if ( prop.Name == "Parent" )
        {
            prop.Value >>= m_xParent;
        }

    CommonInit_Impl( aObjectProps );

    if ( xDispatchInterceptor.is() )
        m_xDocHolder->SetOutplaceDispatchInterceptor( xDispatchInterceptor );
}


OCommonEmbeddedObject::~OCommonEmbeddedObject()
{
    if ( !(m_pInterfaceContainer || m_xDocHolder.is()) )
        return;

    osl_atomic_increment(&m_refCount);
    if ( m_pInterfaceContainer )
    {
        try {
            lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );
            m_pInterfaceContainer->disposeAndClear( aSource );
        } catch( const uno::Exception& ) {}
        m_pInterfaceContainer.reset();
    }

    try {
        if ( m_xDocHolder.is() )
        {
            m_xDocHolder->CloseFrame();
            try {
                m_xDocHolder->CloseDocument( true, true );
            } catch ( const uno::Exception& ) {}
            m_xDocHolder->FreeOffice();

            m_xDocHolder.clear();
        }
    } catch( const uno::Exception& ) {}
}


void OCommonEmbeddedObject::requestPositioning( const awt::Rectangle& aRect )
{
    // the method is called in case object is inplace active and the object window was resized

    OSL_ENSURE( m_xClientSite.is(), "The client site must be set for inplace active object!" );
    if ( !m_xClientSite.is() )
        return;

    uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY );

    OSL_ENSURE( xInplaceClient.is(), "The client site must support XInplaceClient to allow inplace activation!" );
    if ( xInplaceClient.is() )
    {
        try {
            xInplaceClient->changedPlacement( aRect );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION( "embeddedobj", "Exception on request to resize!" );
        }
    }
}


void OCommonEmbeddedObject::PostEvent_Impl( const OUString& aEventName )
{
    if ( !m_pInterfaceContainer )
        return;

    comphelper::OInterfaceContainerHelper2* pIC = m_pInterfaceContainer->getContainer(
                                        cppu::UnoType<document::XEventListener>::get());
    if( !pIC )
        return;

    document::EventObject aEvent;
    aEvent.EventName = aEventName;
    aEvent.Source.set( static_cast< ::cppu::OWeakObject* >( this ) );
    // For now all the events are sent as object events
    // aEvent.Source = ( xSource.is() ? xSource
    //                       : uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ) );
    comphelper::OInterfaceIteratorHelper2 aIt( *pIC );
    while( aIt.hasMoreElements() )
    {
        try
        {
            static_cast<document::XEventListener *>(aIt.next())->notifyEvent( aEvent );
        }
        catch( const uno::RuntimeException& )
        {
            aIt.remove();
        }

        // the listener could dispose the object.
        if ( m_bDisposed )
            return;
    }
}


int OCommonEmbeddedObject::ShowMsgDialog(TranslateId Msg, const OUString& sFileName)
{
    std::locale aResLocale = Translate::Create( "emo" );
    OUString aMsg  = Translate::get( Msg, aResLocale );
    OUString aBtn  = Translate::get( BTN_OVERWRITE_TEXT, aResLocale );
    OUString aTemp = sFileName;

    osl::FileBase::getSystemPathFromFileURL( sFileName, aTemp );

    aMsg = aMsg.replaceFirst( "%{filename}", aTemp );
    weld::Window* pParent = Application::GetFrameWeld(m_xClientWindow);

    std::unique_ptr<weld::MessageDialog> xQueryBox (Application::CreateMessageDialog( pParent,
        VclMessageType::Warning, VclButtonsType::NONE, aMsg ) );
    xQueryBox->add_button( aBtn, RET_YES );
    xQueryBox->add_button( GetStandardText( StandardButtonType::Cancel ), RET_CANCEL );
    xQueryBox->set_default_response( RET_CANCEL );

    return xQueryBox->run();
}


void OCommonEmbeddedObject::handleLinkedOLE( CopyBackToOLELink eState )
{
    // do not refresh and autosave at the same time
    // when refresh all, then get both Link and Ole Update, in this case ignore OLE-refresh
    if ( m_bInHndFunc || m_bOleUpdate || !m_aLinkTempFile.is() )
        return;

    m_bInHndFunc = true;

    bool bLnkFileChg = m_pLinkFile->hasFileChanged( false );
    bool bTmpFileChg = m_bLinkTempFileChanged;


    if ( eState != CopyBackToOLELink::CopyLinkToTempInit && !bLnkFileChg && !bTmpFileChg )
    {
        // no changes
        eState = CopyBackToOLELink::NoCopy;
    }
    else if ( ( eState == CopyBackToOLELink::CopyTempToLink ) && bLnkFileChg && !bTmpFileChg )
    {
        // Save pressed,  but the Link-file is changed, but not the temp-file
        // in this case update the object with new link data
        eState = CopyBackToOLELink::CopyLinkToTempRefresh;
    }
    else if ( ( eState == CopyBackToOLELink::CopyTempToLink ) && bLnkFileChg && bTmpFileChg )
    {
        // Save pressed,  but the Link-file is changed, question to user for overwrite
        if ( ShowMsgDialog(STR_OVERWRITE_LINK, m_aLinkURL) == RET_CANCEL )
            eState = CopyBackToOLELink::NoCopy;
    }
    else if ( ( eState == CopyBackToOLELink::CopyLinkToTemp ) && bTmpFileChg )
    {
        // Refresh pressed,  but the Temp-file is changed, question to user for overwrite
        // it is not important it has bLnkFileChg, always overwrite the temp-file
        if ( ShowMsgDialog( STR_OVERWRITE_TEMP, m_aLinkURL ) == RET_CANCEL )
            eState = CopyBackToOLELink::NoCopy;
    }

    auto writeFile = [ this ]( const OUString& SrcName, const OUString& DesName )
    {
        uno::Reference < ucb::XSimpleFileAccess2 > xWriteAccess( ucb::SimpleFileAccess::create( m_xContext ) );
        uno::Reference < ucb::XSimpleFileAccess > xReadAccess( ucb::SimpleFileAccess::create( m_xContext ) );

        try
        {
            uno::Reference < io::XInputStream > xInStream( xReadAccess->openFileRead (SrcName ) );

            // This is *needed* since OTempFileService calls OTempFileService::readBytes which
            // ensures the SvStream mpStream gets/is opened, *but* also sets the mnCachedPos from
            // OTempFileService which still points to the end-of-file (from write-cc'ing).
            uno::Reference < io::XSeekable > xSeek( xInStream, uno::UNO_QUERY_THROW );
            xSeek->seek( 0 );

            xWriteAccess->writeFile( DesName, xInStream );
            m_bLinkTempFileChanged = false;
            // store the new timestamp
            m_pLinkFile->hasFileChanged();
        }
        catch ( const uno::Exception& ex )
        {
            OUString aMsg;
            osl::FileBase::getSystemPathFromFileURL( SrcName, aMsg );
            aMsg = ex.Message + "\n\n" + aMsg;
            weld::Window* pParent = Application::GetFrameWeld(m_xClientWindow);
            std::unique_ptr<weld::MessageDialog> xQueryBox( Application::CreateMessageDialog( pParent,
                                                     VclMessageType::Error, VclButtonsType::Ok, aMsg ) );

            xQueryBox->run();
        }
    };

    switch ( eState )
    {
        case CopyBackToOLELink::NoCopy:
            break;
        case CopyBackToOLELink::CopyLinkToTemp: // copy Link-File to Temp-File   (Refresh)
        case CopyBackToOLELink::CopyLinkToTempInit: //create temp file
            writeFile( m_aLinkURL, m_aLinkTempFile->getUri() );
            break;
        case CopyBackToOLELink::CopyTempToLink: // copy Temp-File to Link-File   (Save)
            // tdf#141529 if we have a changed copy of the original OLE data we now
            // need to write it back 'over' the original OLE data
            writeFile( m_aLinkTempFile->getUri(), m_aLinkURL );
            break;
        case CopyBackToOLELink::CopyLinkToTempRefresh: // need a Refresh not save
            // do nothing
            break;
        default:
            break;
    }

    m_bInHndFunc = false;
}


uno::Any SAL_CALL OCommonEmbeddedObject::queryInterface( const uno::Type& rType )
{
    uno::Any aReturn;

    if ( rType == cppu::UnoType<embed::XEmbeddedObject>::get() )
    {
        void * p = static_cast< embed::XEmbeddedObject * >( this );
        return uno::Any( &p, rType );
    }
    else if (rType == cppu::UnoType<embed::XEmbedPersist2>::get())
    {
        void* p = static_cast<embed::XEmbedPersist2*>(this);
        return uno::Any(&p, rType);
    }
    else if (rType == cppu::UnoType<lang::XServiceInfo>::get())
    {
        void* p = static_cast<lang::XServiceInfo*>(this);
        return uno::Any(&p, rType);
    }
    else if (rType == cppu::UnoType<lang::XInitialization>::get())
    {
        void* p = static_cast<lang::XInitialization*>(this);
        return uno::Any(&p, rType);
    }
    else if (rType == cppu::UnoType<lang::XTypeProvider>::get())
    {
        void* p = static_cast<lang::XTypeProvider*>(this);
        return uno::Any(&p, rType);
    }
    else
        aReturn = ::cppu::queryInterface(
                    rType,
                    static_cast< embed::XInplaceObject* >( this ),
                    static_cast< embed::XVisualObject* >( this ),
                    static_cast< embed::XCommonEmbedPersist* >( static_cast< embed::XEmbedPersist* >( this ) ),
                    static_cast< embed::XEmbedPersist* >( this ),
                    static_cast< embed::XLinkageSupport* >( this ),
                    static_cast< embed::XStateChangeBroadcaster* >( this ),
                    static_cast< embed::XClassifiedObject* >( this ),
                    static_cast< embed::XComponentSupplier* >( this ),
                    static_cast< util::XCloseable* >( this ),
                    static_cast< container::XChild* >( this ),
                    static_cast< chart2::XDefaultSizeTransmitter* >( this ),
                    static_cast< document::XEventBroadcaster* >( this ) );

    if ( aReturn.hasValue() )
        return aReturn;
    else
        return ::cppu::OWeakObject::queryInterface( rType ) ;

}


void SAL_CALL OCommonEmbeddedObject::acquire()
        noexcept
{
    ::cppu::OWeakObject::acquire() ;
}


void SAL_CALL OCommonEmbeddedObject::release()
        noexcept
{
    ::cppu::OWeakObject::release() ;
}


uno::Sequence< sal_Int8 > SAL_CALL OCommonEmbeddedObject::getClassID()
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    return m_aClassID;
}

OUString SAL_CALL OCommonEmbeddedObject::getClassName()
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    return m_aClassName;
}

void SAL_CALL OCommonEmbeddedObject::setClassInfo(
                const uno::Sequence< sal_Int8 >& /*aClassID*/, const OUString& /*aClassName*/ )
{
    // the object class info can not be changed explicitly
    throw lang::NoSupportException(); //TODO:
}


uno::Reference< util::XCloseable > SAL_CALL OCommonEmbeddedObject::getComponent()
{
    SolarMutexGuard aGuard;
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    // add an exception
    if ( m_nObjectState == -1 )
    {
        // the object is still not loaded
        throw uno::RuntimeException( u"Can't store object without persistence!"_ustr,
                                     static_cast< ::cppu::OWeakObject* >(this) );
    }

    return m_xDocHolder->GetComponent();
}


void SAL_CALL OCommonEmbeddedObject::addStateChangeListener( const uno::Reference< embed::XStateChangeListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new comphelper::OMultiTypeInterfaceContainerHelper2( m_aMutex ));

    m_pInterfaceContainer->addInterface( cppu::UnoType<embed::XStateChangeListener>::get(),
                                                        xListener );
}


void SAL_CALL OCommonEmbeddedObject::removeStateChangeListener(
                    const uno::Reference< embed::XStateChangeListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<embed::XStateChangeListener>::get(),
                                                xListener );
}


void SAL_CALL OCommonEmbeddedObject::close( sal_Bool bDeliverOwnership )
{
    SolarMutexGuard aGuard;
    if ( m_bClosed )
        throw lang::DisposedException(); // TODO

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >( this ) );
    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );

    if ( m_pInterfaceContainer )
    {
        comphelper::OInterfaceContainerHelper2* pContainer =
            m_pInterfaceContainer->getContainer( cppu::UnoType<util::XCloseListener>::get());
        if ( pContainer != nullptr )
        {
            comphelper::OInterfaceIteratorHelper2 pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    static_cast<util::XCloseListener*>(pIterator.next())->queryClosing( aSource, bDeliverOwnership );
                }
                catch( const uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }

        pContainer = m_pInterfaceContainer->getContainer(
                                    cppu::UnoType<util::XCloseListener>::get());
        if ( pContainer != nullptr )
        {
            comphelper::OInterfaceIteratorHelper2 pCloseIterator(*pContainer);
            while (pCloseIterator.hasMoreElements())
            {
                try
                {
                    static_cast<util::XCloseListener*>(pCloseIterator.next())->notifyClosing( aSource );
                }
                catch( const uno::RuntimeException& )
                {
                    pCloseIterator.remove();
                }
            }
        }

        m_pInterfaceContainer->disposeAndClear( aSource );
        m_pInterfaceContainer.reset();
    }

    m_bDisposed = true; // the object is disposed now for outside

    // it is possible that the document can not be closed, in this case if the argument is false
    // the exception will be thrown otherwise in addition to exception the object must register itself
    // as termination listener and listen for document events

    if ( m_xDocHolder.is() )
    {
        m_xDocHolder->CloseFrame();

        try {
            m_xDocHolder->CloseDocument( bDeliverOwnership, bDeliverOwnership );
        }
        catch( const uno::Exception& )
        {
            if ( bDeliverOwnership )
            {
                m_xDocHolder.clear();
                m_bClosed = true;
            }

            throw;
        }

        m_xDocHolder->FreeOffice();

        m_xDocHolder.clear();
    }

    // TODO: for now the storage will be disposed by the object, but after the document
    // will use the storage, the storage will be disposed by the document and recreated by the object
    if ( m_xObjectStorage.is() )
    {
        try {
            m_xObjectStorage->dispose();
        } catch ( const uno::Exception& ) {}

        m_xObjectStorage.clear();
        m_xRecoveryStorage.clear();
    }

    m_bClosed = true; // the closing succeeded
}


void SAL_CALL OCommonEmbeddedObject::addCloseListener( const uno::Reference< util::XCloseListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new comphelper::OMultiTypeInterfaceContainerHelper2(m_aMutex));

    m_pInterfaceContainer->addInterface( cppu::UnoType<util::XCloseListener>::get(), xListener );
}


void SAL_CALL OCommonEmbeddedObject::removeCloseListener( const uno::Reference< util::XCloseListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<util::XCloseListener>::get(),
                                                xListener );
}


void SAL_CALL OCommonEmbeddedObject::addEventListener( const uno::Reference< document::XEventListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new comphelper::OMultiTypeInterfaceContainerHelper2(m_aMutex));

    m_pInterfaceContainer->addInterface( cppu::UnoType<document::XEventListener>::get(), xListener );
}


void SAL_CALL OCommonEmbeddedObject::removeEventListener( const uno::Reference< document::XEventListener >& xListener )
{
    SolarMutexGuard aGuard;
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<document::XEventListener>::get(),
                                                xListener );
}

OUString SAL_CALL OCommonEmbeddedObject::getImplementationName()
{
    return u"com.sun.star.comp.embed.OCommonEmbeddedObject"_ustr;
}

sal_Bool SAL_CALL OCommonEmbeddedObject::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<OUString> SAL_CALL OCommonEmbeddedObject::getSupportedServiceNames()
{
    return { u"com.sun.star.comp.embed.OCommonEmbeddedObject"_ustr };
}

uno::Sequence<uno::Type> SAL_CALL OCommonEmbeddedObject::getTypes()
{
    static const uno::Sequence<uno::Type> aTypes{
        cppu::UnoType<embed::XEmbeddedObject>::get(),
        cppu::UnoType<embed::XEmbedPersist2>::get(),
        cppu::UnoType<embed::XLinkageSupport>::get(),
        cppu::UnoType<embed::XInplaceObject>::get(),
        cppu::UnoType<container::XChild>::get(),
        cppu::UnoType<chart2::XDefaultSizeTransmitter>::get(),
        cppu::UnoType<lang::XServiceInfo>::get(),
        cppu::UnoType<lang::XInitialization>::get(),
        cppu::UnoType<lang::XTypeProvider>::get(),
    };
    return aTypes;
}

uno::Sequence<sal_Int8> SAL_CALL OCommonEmbeddedObject::getImplementationId()
{
    return uno::Sequence<sal_Int8>();
}

void SAL_CALL OCommonEmbeddedObject::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    if (!rArguments.hasElements())
    {
        return;
    }

    comphelper::SequenceAsHashMap aMap(rArguments[0]);
    auto it = aMap.find(u"ReadOnly"_ustr);
    if (it != aMap.end())
    {
        it->second >>= m_bReadOnly;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
