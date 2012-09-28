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

#include <sfx2/sfxbasemodel.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XPrintJobListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/frame/IllegalArgumentIOException.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/frame/UntitledNumbersConst.hpp>
#include <com/sun/star/embed/XTransactionBroadcaster.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#include <com/sun/star/document/XStorageChangeListener.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/frame/XTransientDocumentsDocumentContentFactory.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <comphelper/enumhelper.hxx>  // can be removed when this is a "real" service

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/processfactory.hxx>  // can be removed when this is a "real" service
#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <basic/sbx.hxx>
#include <basic/sbuno.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/tempfile.hxx>
#include <osl/mutex.hxx>
#include <vcl/salctype.hxx>
#include <sot/clsids.hxx>
#include <sot/storinfo.hxx>
#include <comphelper/storagehelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/transfer.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <rtl/logfile.hxx>
#include <rtl/strbuf.hxx>
#include <framework/configimporter.hxx>
#include <framework/interaction.hxx>
#include <framework/titlehelper.hxx>
#include <comphelper/numberedcollection.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>

#include <sfx2/sfxbasecontroller.hxx>
#include "sfx2/viewfac.hxx"
#include "workwin.hxx"
#include <sfx2/signaturestate.hxx>
#include <sfx2/sfxuno.hxx>
#include <objshimp.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objuno.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/basmgr.hxx>
#include <sfx2/event.hxx>
#include <eventsupplier.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/app.hxx>
#include "appdata.hxx"
#include <sfx2/docfac.hxx>
#include <sfx2/fcontnr.hxx>
#include "sfx2/docstoragemodifylistener.hxx"
#include "sfx2/brokenpackageint.hxx"
#include "graphhelp.hxx"
#include "docundomanager.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/DocumentMetadataAccess.hxx>

#include <sfx2/sfxresid.hxx>

//________________________________________________________________________________________________________
//  namespaces
//________________________________________________________________________________________________________

namespace css = ::com::sun::star;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XController;
using ::com::sun::star::frame::XController2;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::io::IOException;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::document::XDocumentRecovery;
using ::com::sun::star::document::XUndoManager;
using ::com::sun::star::document::XUndoAction;
using ::com::sun::star::document::UndoFailedException;
using ::com::sun::star::frame::XModel;

/** This Listener is used to get notified when the XDocumentProperties of the
    XModel change.
 */
class SfxDocInfoListener_Impl : public ::cppu::WeakImplHelper1<
    ::com::sun::star::util::XModifyListener >
{

public:
    SfxObjectShell& m_rShell;

    SfxDocInfoListener_Impl( SfxObjectShell& i_rDoc )
        : m_rShell(i_rDoc)
    { };

    ~SfxDocInfoListener_Impl();

    virtual void SAL_CALL disposing( const lang::EventObject& )
        throw ( uno::RuntimeException );
    virtual void SAL_CALL modified( const lang::EventObject& )
        throw ( uno::RuntimeException );
};
SfxDocInfoListener_Impl::~SfxDocInfoListener_Impl()
{
}
void SAL_CALL SfxDocInfoListener_Impl::modified( const lang::EventObject& )
        throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;

    // notify changes to the SfxObjectShell
    m_rShell.FlushDocInfo();
}

void SAL_CALL SfxDocInfoListener_Impl::disposing( const lang::EventObject& )
    throw ( uno::RuntimeException )
{
}

//________________________________________________________________________________________________________
//  impl. declarations
//________________________________________________________________________________________________________


struct IMPL_SfxBaseModel_DataContainer : public ::sfx2::IModifiableDocument
{
    // counter for SfxBaseModel instances created.
    static sal_Int64                                        g_nInstanceCounter      ;
    SfxObjectShellRef                                       m_pObjectShell          ;
    ::rtl::OUString                                         m_sURL                  ;
    ::rtl::OUString                                         m_sRuntimeUID           ;
    ::rtl::OUString                                         m_aPreusedFilterName;
    ::cppu::OMultiTypeInterfaceContainerHelper              m_aInterfaceContainer   ;
    uno::Reference< uno::XInterface >                       m_xParent               ;
    uno::Reference< frame::XController >                    m_xCurrent              ;
    uno::Reference< document::XDocumentInfo >               m_xDocumentInfo         ;
    uno::Reference< document::XDocumentProperties >         m_xDocumentProperties;
    uno::Reference< script::XStarBasicAccess >              m_xStarBasicAccess      ;
    uno::Reference< container::XNameReplace >               m_xEvents               ;
    uno::Sequence< beans::PropertyValue>                    m_seqArguments          ;
    uno::Sequence< uno::Reference< frame::XController > >   m_seqControllers        ;
    uno::Reference< container::XIndexAccess >               m_contViewData          ;
    sal_uInt16                                              m_nControllerLockCount  ;
    sal_Bool                                                m_bClosed               ;
    sal_Bool                                                m_bClosing              ;
    sal_Bool                                                m_bSaving               ;
    sal_Bool                                                m_bSuicide              ;
    sal_Bool                                                m_bInitialized          ;
    sal_Bool                                                m_bExternalTitle        ;
    sal_Bool                                                m_bModifiedSinceLastSave;
    uno::Reference< com::sun::star::view::XPrintable>       m_xPrintable            ;
    uno::Reference< script::provider::XScriptProvider >     m_xScriptProvider;
    uno::Reference< ui::XUIConfigurationManager >           m_xUIConfigurationManager;
    ::rtl::Reference< ::sfx2::DocumentStorageModifyListener >   m_pStorageModifyListen;
    ::rtl::OUString                                         m_sModuleIdentifier;
    css::uno::Reference< css::frame::XTitle >               m_xTitleHelper;
    css::uno::Reference< css::frame::XUntitledNumbers >     m_xNumberedControllers;
    uno::Reference< rdf::XDocumentMetadataAccess>           m_xDocumentMetadata;
    ::rtl::Reference< ::sfx2::DocumentUndoManager >         m_pDocumentUndoManager;


    IMPL_SfxBaseModel_DataContainer( ::osl::Mutex& rMutex, SfxObjectShell* pObjectShell )
            :   m_pObjectShell          ( pObjectShell  )
            ,   m_aInterfaceContainer   ( rMutex        )
            ,   m_nControllerLockCount  ( 0             )
            ,   m_bClosed               ( sal_False     )
            ,   m_bClosing              ( sal_False     )
            ,   m_bSaving               ( sal_False     )
            ,   m_bSuicide              ( sal_False     )
            ,   m_bInitialized          ( sal_False     )
            ,   m_bExternalTitle        ( sal_False     )
            ,   m_bModifiedSinceLastSave( sal_False     )
            ,   m_pStorageModifyListen  ( NULL          )
            ,   m_xTitleHelper          ()
            ,   m_xNumberedControllers  ()
            ,   m_xDocumentMetadata     () // lazy
            ,   m_pDocumentUndoManager  ()
    {
        // increase global instance counter.
        ++g_nInstanceCounter;
        // set own Runtime UID
        m_sRuntimeUID = rtl::OUString::valueOf( g_nInstanceCounter );
    }

    virtual ~IMPL_SfxBaseModel_DataContainer()
    {
    }

    // ::sfx2::IModifiableDocument
    virtual void storageIsModified()
    {
        if ( m_pObjectShell.Is() && !m_pObjectShell->IsModified() )
            m_pObjectShell->SetModified( sal_True );
    }

    void impl_setDocumentProperties(
            const uno::Reference< document::XDocumentProperties >& );

    uno::Reference<rdf::XDocumentMetadataAccess> GetDMA()
    {
        if (!m_xDocumentMetadata.is())
        {
            OSL_ENSURE(m_pObjectShell, "GetDMA: no object shell?");
            if (!m_pObjectShell)
            {
                return 0;
            }

            const uno::Reference<uno::XComponentContext> xContext(
                ::comphelper::getProcessComponentContext());
            ::rtl::OUString uri;
            const uno::Reference<frame::XModel> xModel(
                m_pObjectShell->GetModel());
            const uno::Reference<lang::XMultiComponentFactory> xMsf(
                xContext->getServiceManager());
            const uno::Reference<frame::
                XTransientDocumentsDocumentContentFactory> xTDDCF(
                    xMsf->createInstanceWithContext(
                        ::rtl::OUString("com.sun.star.frame."
                            "TransientDocumentsDocumentContentFactory"),
                    xContext),
                uno::UNO_QUERY_THROW);
            const uno::Reference<ucb::XContent> xContent(
                xTDDCF->createDocumentContent(xModel) );
            OSL_ENSURE(xContent.is(), "GetDMA: cannot create DocumentContent");
            if (!xContent.is())
            {
                return 0;
            }
            uri = xContent->getIdentifier()->getContentIdentifier();
            OSL_ENSURE(!uri.isEmpty(), "GetDMA: empty uri?");
            if (!uri.isEmpty() && !uri.endsWithAsciiL("/", 1))
            {
                uri = uri + ::rtl::OUString("/");
            }

            m_xDocumentMetadata = new ::sfx2::DocumentMetadataAccess(
                xContext, *m_pObjectShell, uri);
        }
        return m_xDocumentMetadata;
    }

    uno::Reference<rdf::XDocumentMetadataAccess> CreateDMAUninitialized()
    {
        return (m_pObjectShell)
            ? new ::sfx2::DocumentMetadataAccess(
                ::comphelper::getProcessComponentContext(), *m_pObjectShell)
            : 0;
    }
};

// static member initialization.
sal_Int64 IMPL_SfxBaseModel_DataContainer::g_nInstanceCounter = 0;

// =======================================================================================================

// Listener that forwards notifications from the PrintHelper to the "real" listeners
class SfxPrintHelperListener_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::view::XPrintJobListener >
{
public:
    IMPL_SfxBaseModel_DataContainer* m_pData;
    SfxPrintHelperListener_Impl( IMPL_SfxBaseModel_DataContainer* pData )
        : m_pData( pData )
    {}

    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) throw ( uno::RuntimeException ) ;
    virtual void SAL_CALL printJobEvent( const view::PrintJobEvent& rEvent ) throw ( uno::RuntimeException);
};

void SAL_CALL SfxPrintHelperListener_Impl::disposing( const lang::EventObject& ) throw ( uno::RuntimeException )
{
    m_pData->m_xPrintable = 0;
}

void SAL_CALL SfxPrintHelperListener_Impl::printJobEvent( const view::PrintJobEvent& rEvent ) throw (uno::RuntimeException)
{
    ::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( ( const uno::Reference< view::XPrintJobListener >*) NULL ) );
    if ( pContainer!=NULL )
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
            ((view::XPrintJobListener*)pIterator.next())->printJobEvent( rEvent );
    }
}

// SfxOwnFramesLocker ====================================================================================
// allows to lock all the frames related to the provided SfxObjectShell
class SfxOwnFramesLocker
{
    uno::Sequence< uno::Reference< frame::XFrame > > m_aLockedFrames;

    Window* GetVCLWindow( const uno::Reference< frame::XFrame >& xFrame );
public:
    SfxOwnFramesLocker( SfxObjectShell* ObjechShell );
    ~SfxOwnFramesLocker();
    void UnlockFrames();
};

SfxOwnFramesLocker::SfxOwnFramesLocker( SfxObjectShell* pObjectShell )
{
    if ( !pObjectShell )
        return;

    for (   SfxViewFrame *pFrame = SfxViewFrame::GetFirst( pObjectShell );
            pFrame;
            pFrame = SfxViewFrame::GetNext( *pFrame, pObjectShell )
        )
    {
        SfxFrame& rSfxFrame = pFrame->GetFrame();
        try
        {
            // get vcl window related to the frame and lock it if it is still not locked
            uno::Reference< frame::XFrame > xFrame = rSfxFrame.GetFrameInterface();
            Window* pWindow = GetVCLWindow( xFrame );
            if ( !pWindow )
                throw uno::RuntimeException();

            if ( pWindow->IsEnabled() )
            {
                pWindow->Disable();

                try
                {
                    sal_Int32 nLen = m_aLockedFrames.getLength();
                    m_aLockedFrames.realloc( nLen + 1 );
                    m_aLockedFrames[nLen] = xFrame;
                }
                catch( uno::Exception& )
                {
                    pWindow->Enable();
                    throw;
                }
            }
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "Not possible to lock the frame window!\n" );
        }
    }
}

SfxOwnFramesLocker::~SfxOwnFramesLocker()
{
    UnlockFrames();
}

Window* SfxOwnFramesLocker::GetVCLWindow( const uno::Reference< frame::XFrame >& xFrame )
{
    Window* pWindow = NULL;

    if ( xFrame.is() )
    {
        uno::Reference< awt::XWindow > xWindow = xFrame->getContainerWindow();
        if ( xWindow.is() )
               pWindow = VCLUnoHelper::GetWindow( xWindow );
    }

    return pWindow;
}

void SfxOwnFramesLocker::UnlockFrames()
{
    for ( sal_Int32 nInd = 0; nInd < m_aLockedFrames.getLength(); nInd++ )
    {
        try
        {
            if ( m_aLockedFrames[nInd].is() )
            {
                // get vcl window related to the frame and unlock it
                Window* pWindow = GetVCLWindow( m_aLockedFrames[nInd] );
                if ( !pWindow )
                    throw uno::RuntimeException();

                pWindow->Enable();

                m_aLockedFrames[nInd] = uno::Reference< frame::XFrame >();
            }
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "Can't unlock the frame window!\n" );
        }
    }
}

// SfxSaveGuard ====================================================================================
class SfxSaveGuard
{
    private:
        uno::Reference< frame::XModel > m_xModel;
        IMPL_SfxBaseModel_DataContainer* m_pData;
        SfxOwnFramesLocker* m_pFramesLock;

    public:
        SfxSaveGuard(const uno::Reference< frame::XModel >&             xModel                      ,
                           IMPL_SfxBaseModel_DataContainer* pData                       ,
                           sal_Bool                         bRejectConcurrentSaveRequest);
        ~SfxSaveGuard();
};

SfxSaveGuard::SfxSaveGuard(const uno::Reference< frame::XModel >&             xModel                      ,
                                 IMPL_SfxBaseModel_DataContainer* pData                       ,
                                 sal_Bool                         bRejectConcurrentSaveRequest)
    : m_xModel     (xModel)
    , m_pData      (pData )
    , m_pFramesLock(0     )
{
    if ( m_pData->m_bClosed )
        throw ::com::sun::star::lang::DisposedException(
                ::rtl::OUString("Object already disposed."),
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >());

    if (
        bRejectConcurrentSaveRequest &&
        m_pData->m_bSaving
       )
        throw ::com::sun::star::io::IOException(
                ::rtl::OUString("Concurrent save requests on the same document are not possible."),
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >());

    m_pData->m_bSaving = sal_True;
    m_pFramesLock = new SfxOwnFramesLocker(m_pData->m_pObjectShell);
}

SfxSaveGuard::~SfxSaveGuard()
{
    SfxOwnFramesLocker* pFramesLock = m_pFramesLock;
    m_pFramesLock = 0;
    delete pFramesLock;

    m_pData->m_bSaving = sal_False;

    // m_bSuicide was set e.g. in case somewhere tried to close a document, while it was used for
    // storing at the same time. Further m_bSuicide was set to sal_True only if close(sal_True) was called.
    // So the owner ship was delegated to the place where a veto exception was thrown.
    // Now we have to call close() again and delegate the owner ship to the next one, which
    // cant accept that. Close(sal_False) cant work in this case. Because then the document will may be never closed ...

    if ( m_pData->m_bSuicide )
    {
        // Reset this state. In case the new close() request is not accepted by somehwere else ...
        // it's not a good idea to have two "owners" for close .-)
        m_pData->m_bSuicide = sal_False;
        try
        {
            uno::Reference< util::XCloseable > xClose(m_xModel, uno::UNO_QUERY);
            if (xClose.is())
                xClose->close(sal_True);
        }
        catch(const util::CloseVetoException&)
        {}
    }
}

// =======================================================================================================

//________________________________________________________________________________________________________
//  constructor
//________________________________________________________________________________________________________
DBG_NAME(sfx2_SfxBaseModel)
SfxBaseModel::SfxBaseModel( SfxObjectShell *pObjectShell )
: BaseMutex()
, m_pData( new IMPL_SfxBaseModel_DataContainer( m_aMutex, pObjectShell ) )
, m_bSupportEmbeddedScripts( pObjectShell && pObjectShell->Get_Impl() ? !pObjectShell->Get_Impl()->m_bNoBasicCapabilities : false )
, m_bSupportDocRecovery( pObjectShell && pObjectShell->Get_Impl() ? pObjectShell->Get_Impl()->m_bDocRecoverySupport : false )
{
    DBG_CTOR(sfx2_SfxBaseModel,NULL);
    if ( pObjectShell != NULL )
    {
        StartListening( *pObjectShell ) ;
    }
}

//________________________________________________________________________________________________________
//  destructor
//________________________________________________________________________________________________________

SfxBaseModel::~SfxBaseModel()
{
    DBG_DTOR(sfx2_SfxBaseModel,NULL);
}

//________________________________________________________________________________________________________
//  XInterface
//________________________________________________________________________________________________________

uno::Any SAL_CALL SfxBaseModel::queryInterface( const UNOTYPE& rType ) throw( uno::RuntimeException )
{
    if  (   ( !m_bSupportEmbeddedScripts && rType.equals( XEMBEDDEDSCRIPTS::static_type() ) )
        ||  ( !m_bSupportDocRecovery && rType.equals( XDocumentRecovery::static_type() ) )
        )
        return Any();

    return SfxBaseModel_Base::queryInterface( rType );
}

//________________________________________________________________________________________________________
//  XInterface
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::acquire() throw( )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::acquire() ;
}

//________________________________________________________________________________________________________
//  XInterface
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::release() throw( )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    // Forward to baseclass
    OWeakObject::release() ;
}

//________________________________________________________________________________________________________
//  XTypeProvider
//________________________________________________________________________________________________________

namespace
{
    void lcl_stripType( Sequence< Type >& io_rTypes, const Type& i_rTypeToStrip )
    {
        Sequence< UNOTYPE > aStrippedTypes( io_rTypes.getLength() - 1 );
        ::std::remove_copy_if(
            io_rTypes.getConstArray(),
            io_rTypes.getConstArray() + io_rTypes.getLength(),
            aStrippedTypes.getArray(),
            ::std::bind2nd( ::std::equal_to< Type >(), i_rTypeToStrip )
        );
        io_rTypes = aStrippedTypes;
    }
}

uno::Sequence< UNOTYPE > SAL_CALL SfxBaseModel::getTypes() throw( uno::RuntimeException )
{
    uno::Sequence< UNOTYPE > aTypes( SfxBaseModel_Base::getTypes() );

    if ( !m_bSupportEmbeddedScripts )
        lcl_stripType( aTypes, XEMBEDDEDSCRIPTS::static_type() );

    if ( !m_bSupportDocRecovery )
        lcl_stripType( aTypes, XDocumentRecovery::static_type() );

    return aTypes;
}

//________________________________________________________________________________________________________
//  XTypeProvider
//________________________________________________________________________________________________________

uno::Sequence< sal_Int8 > SAL_CALL SfxBaseModel::getImplementationId() throw( uno::RuntimeException )
{
    // Create one Id for all instances of this class.
    // Use ethernet address to do this! (sal_True)

    // Optimize this method
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pID is NULL - for the second call pID is different from NULL!
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        ::osl::MutexGuard aGuard( MUTEX::getGlobalMutex() ) ;

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pID == NULL )
        {
            // Create a new static ID ...
            static ::cppu::OImplementationId aID( sal_False ) ;
            // ... and set his address to static pointer!
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

//________________________________________________________________________________________________________
//  XStarBasicAccess
//________________________________________________________________________________________________________

uno::Reference< script::XStarBasicAccess > implGetStarBasicAccess( SfxObjectShell* pObjectShell )
{
    uno::Reference< script::XStarBasicAccess > xRet;

#ifdef DISABLE_SCRIPTING
    (void) pObjectShell;
#else
    if( pObjectShell )
    {
        BasicManager* pMgr = pObjectShell->GetBasicManager();
        xRet = getStarBasicAccess( pMgr );
    }
#endif
    return xRet;
}

uno::Reference< XNAMECONTAINER > SAL_CALL SfxBaseModel::getLibraryContainer() throw( uno::RuntimeException )
{
#ifdef DISABLE_SCRIPTING
    uno::Reference< XNAMECONTAINER > dummy;

    return dummy;
#else
    SfxModelGuard aGuard( *this );

    uno::Reference< script::XStarBasicAccess >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() && m_pData->m_pObjectShell.Is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    uno::Reference< XNAMECONTAINER > xRet;
    if( rxAccess.is() )
        xRet = rxAccess->getLibraryContainer();
    return xRet;
#endif
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::createLibrary( const ::rtl::OUString& LibName, const ::rtl::OUString& Password,
    const ::rtl::OUString& ExternalSourceURL, const ::rtl::OUString& LinkTargetURL )
        throw(ELEMENTEXISTEXCEPTION, uno::RuntimeException)
{
#ifdef DISABLE_SCRIPTING
    (void) LibName;
    (void) Password;
    (void) ExternalSourceURL;
    (void) LinkTargetURL;
#else
    SfxModelGuard aGuard( *this );

    uno::Reference< script::XStarBasicAccess >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() && m_pData->m_pObjectShell.Is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    if( rxAccess.is() )
        rxAccess->createLibrary( LibName, Password, ExternalSourceURL, LinkTargetURL );
#endif
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::addModule( const ::rtl::OUString& LibraryName, const ::rtl::OUString& ModuleName,
    const ::rtl::OUString& Language, const ::rtl::OUString& Source )
        throw( NOSUCHELEMENTEXCEPTION, uno::RuntimeException)
{
#ifdef DISABLE_SCRIPTING
    (void) LibraryName;
    (void) ModuleName;
    (void) Language;
    (void) Source;
#else
    SfxModelGuard aGuard( *this );

    uno::Reference< script::XStarBasicAccess >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() && m_pData->m_pObjectShell.Is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    if( rxAccess.is() )
        rxAccess->addModule( LibraryName, ModuleName, Language, Source );
#endif
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::addDialog( const ::rtl::OUString& LibraryName, const ::rtl::OUString& DialogName,
    const ::com::sun::star::uno::Sequence< sal_Int8 >& Data )
        throw(NOSUCHELEMENTEXCEPTION, uno::RuntimeException)
{
#ifdef DISABLE_SCRIPTING
    (void) LibraryName;
    (void) DialogName;
    (void) Data;
#else
    SfxModelGuard aGuard( *this );

    uno::Reference< script::XStarBasicAccess >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() && m_pData->m_pObjectShell.Is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell );

    if( rxAccess.is() )
        rxAccess->addDialog( LibraryName, DialogName, Data );
#endif
}


//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

uno::Reference< uno::XInterface > SAL_CALL SfxBaseModel::getParent() throw( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    return m_pData->m_xParent;
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::setParent(const uno::Reference< uno::XInterface >& Parent) throw(NOSUPPORTEXCEPTION, uno::RuntimeException)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    m_pData->m_xParent = Parent;
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    if  ( !m_pData->m_bClosed )
    {
        // gracefully accept wrong dispose calls instead of close call
        // and try to make it work (may be really disposed later!)
        try
        {
            close( sal_True );
        }
        catch ( com::sun::star::util::CloseVetoException& )
        {
        }

        return;
    }

    if ( m_pData->m_pStorageModifyListen.is() )
    {
        m_pData->m_pStorageModifyListen->dispose();
        m_pData->m_pStorageModifyListen = NULL;
    }

    if ( m_pData->m_pDocumentUndoManager.is() )
    {
        m_pData->m_pDocumentUndoManager->disposing();
        m_pData->m_pDocumentUndoManager = NULL;
    }

    lang::EventObject aEvent( (frame::XModel *)this );
    m_pData->m_aInterfaceContainer.disposeAndClear( aEvent );

    if ( m_pData->m_xDocumentInfo.is() )
    {
        // as long as an SfxObjectShell is assigned to an SfxBaseModel it is still existing here
        // so we can't dispose the shared DocumentInfoObject here
        m_pData->m_xDocumentInfo = 0;
    }

    m_pData->m_xDocumentProperties.clear();

    m_pData->m_xDocumentMetadata.clear();

    EndListening( *m_pData->m_pObjectShell );

    m_pData->m_xCurrent = uno::Reference< frame::XController > ();
    m_pData->m_seqControllers = uno::Sequence< uno::Reference< frame::XController > > () ;

    // m_pData member must be set to zero before 0delete is called to
    // force disposed exception whenever someone tries to access our
    // instance while in the dtor.
    IMPL_SfxBaseModel_DataContainer* pData = m_pData;
    m_pData = 0;
    delete pData;
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::addEventListener( const uno::Reference< XEVENTLISTENER >& aListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const uno::Reference< XEVENTLISTENER >*)0), aListener );
}

//________________________________________________________________________________________________________
//  XChild
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::removeEventListener( const uno::Reference< XEVENTLISTENER >& aListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const uno::Reference< XEVENTLISTENER >*)0), aListener );
}

//________________________________________________________________________________________________________
//  document::XDocumentInfoSupplier
//________________________________________________________________________________________________________

uno::Reference< document::XDocumentInfo > SAL_CALL SfxBaseModel::getDocumentInfo() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    if ( !m_pData->m_xDocumentInfo.is() )
    {
        // WARNING: this will only work if (when loading a document) the
        // document meta-data has already been read and completely written
        // into the XDocumentProperties at this point
        // ==> DO NOT call getDocumentInfo before document info has been read!
        uno::Reference< document::XDocumentInfo > xDocInfo =
            new SfxDocumentInfoObject;
        uno::Reference< document::XDocumentProperties > xDocProps =
            getDocumentProperties();
        uno::Sequence< uno::Any > args(1);
        args[0] <<= xDocProps;
        uno::Reference< lang::XInitialization > xInit(
            xDocInfo, uno::UNO_QUERY_THROW);
        try {
            xInit->initialize(args);
            ((SfxBaseModel*)this)->m_pData->m_xDocumentInfo = xDocInfo;
        } catch (uno::RuntimeException &) {
            throw;
        } catch (const uno::Exception & e) {
            throw lang::WrappedTargetRuntimeException(::rtl::OUString(
                "SfxBaseModel::getDocumentInfo: cannot initialize"), *this,
                uno::makeAny(e));
        }
        try {
            rtl::OUString aName("MediaType");
            uno::Reference < beans::XPropertySet > xSet(
                getDocumentStorage(), uno::UNO_QUERY_THROW );
            uno::Any aMediaType = xSet->getPropertyValue( aName );
            uno::Reference < beans::XPropertySet > xDocSet(
                m_pData->m_xDocumentInfo, uno::UNO_QUERY_THROW );
            xDocSet->setPropertyValue( aName, aMediaType );
        } catch (uno::Exception &) {
            //ignore
        }
    }

    return m_pData->m_xDocumentInfo;
}

void
IMPL_SfxBaseModel_DataContainer::impl_setDocumentProperties(
        const uno::Reference< document::XDocumentProperties >& rxNewDocProps)
{
    m_xDocumentProperties.set(rxNewDocProps, uno::UNO_QUERY_THROW);
    uno::Reference<util::XModifyBroadcaster> const xMB(m_xDocumentProperties,
            uno::UNO_QUERY_THROW);
    xMB->addModifyListener(new SfxDocInfoListener_Impl(*m_pObjectShell));
}

// document::XDocumentPropertiesSupplier:
uno::Reference< document::XDocumentProperties > SAL_CALL
SfxBaseModel::getDocumentProperties()
    throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    if ( !m_pData->m_xDocumentProperties.is() )
    {
        uno::Reference< document::XDocumentProperties > xDocProps(
            document::DocumentProperties::create( ::comphelper::getProcessComponentContext() ) );
        m_pData->impl_setDocumentProperties(xDocProps);
    }

    return m_pData->m_xDocumentProperties;
}


//________________________________________________________________________________________________________
//  XEVENTLISTENER
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::disposing( const lang::EventObject& aObject )
    throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    uno::Reference< XMODIFYLISTENER >  xMod( aObject.Source, uno::UNO_QUERY );
    uno::Reference< XEVENTLISTENER >  xListener( aObject.Source, uno::UNO_QUERY );
    uno::Reference< XDOCEVENTLISTENER >  xDocListener( aObject.Source, uno::UNO_QUERY );

    if ( xMod.is() )
        m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const uno::Reference< XMODIFYLISTENER >*)0), xMod );
    else if ( xListener.is() )
        m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const uno::Reference< XEVENTLISTENER >*)0), xListener );
    else if ( xDocListener.is() )
        m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const uno::Reference< XDOCEVENTLISTENER >*)0), xListener );
}

//________________________________________________________________________________________________________
//  frame::XModel
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::attachResource( const   ::rtl::OUString&                   rURL    ,
                                                const   uno::Sequence< beans::PropertyValue >&  rArgs   )
    throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    if ( rURL.isEmpty() && rArgs.getLength() == 1 && rArgs[0].Name == "SetEmbedded" )
    {
        // allows to set a windowless document to EMBEDDED state
        // but _only_ before load() or initNew() methods
        if ( m_pData->m_pObjectShell.Is() && !m_pData->m_pObjectShell->GetMedium() )
        {
            sal_Bool bEmb = sal_Bool();
            if ( ( rArgs[0].Value >>= bEmb ) && bEmb )
                m_pData->m_pObjectShell->SetCreateMode_Impl( SFX_CREATE_MODE_EMBEDDED );
        }

        return sal_True;
    }

    if ( m_pData->m_pObjectShell.Is() )
    {
        m_pData->m_sURL = rURL;

        SfxObjectShell* pObjectShell = m_pData->m_pObjectShell;

        ::comphelper::NamedValueCollection aArgs( rArgs );

        Sequence< sal_Int32 > aWinExtent;
        if ( ( aArgs.get( "WinExtent" ) >>= aWinExtent )&& ( aWinExtent.getLength() == 4 ) )
        {
            Rectangle aVisArea( aWinExtent[0], aWinExtent[1], aWinExtent[2], aWinExtent[3] );
            aVisArea = OutputDevice::LogicToLogic( aVisArea, MAP_100TH_MM, pObjectShell->GetMapUnit() );
            pObjectShell->SetVisArea( aVisArea );
        }

        sal_Bool bBreakMacroSign = sal_False;
        if ( aArgs.get( "BreakMacroSignature" ) >>= bBreakMacroSign )
        {
            pObjectShell->BreakMacroSign_Impl( bBreakMacroSign );
        }

        aArgs.remove( "WinExtent" );
        aArgs.remove( "BreakMacroSignature" );
        aArgs.remove( "Stream" );
        aArgs.remove( "InputStream" );
        aArgs.remove( "URL" );
        aArgs.remove( "Frame" );
        aArgs.remove( "Password" );
        aArgs.remove( "EncryptionData" );

        // TODO/LATER: all the parameters that are accepted by ItemSet of the DocShell must be removed here

        m_pData->m_seqArguments = aArgs.getPropertyValues();

        SfxMedium* pMedium = pObjectShell->GetMedium();
        if ( pMedium )
        {
            SfxAllItemSet aSet( pObjectShell->GetPool() );
            TransformParameters( SID_OPENDOC, rArgs, aSet );

            // the arguments are not allowed to reach the medium
            aSet.ClearItem( SID_FILE_NAME );
            aSet.ClearItem( SID_FILLFRAME );

            pMedium->GetItemSet()->Put( aSet );
            SFX_ITEMSET_ARG( &aSet, pItem, SfxStringItem, SID_FILTER_NAME, sal_False );
            if ( pItem )
                pMedium->SetFilter(
                    pObjectShell->GetFactory().GetFilterContainer()->GetFilter4FilterName( pItem->GetValue() ) );

            SFX_ITEMSET_ARG( &aSet, pTitleItem, SfxStringItem, SID_DOCINFO_TITLE, sal_False );
            if ( pTitleItem )
            {
                SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pObjectShell );
                if ( pFrame )
                    pFrame->UpdateTitle();
            }
        }
    }

    return sal_True ;
}

//________________________________________________________________________________________________________
//  frame::XModel
//________________________________________________________________________________________________________

::rtl::OUString SAL_CALL SfxBaseModel::getURL() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );
    return m_pData->m_sURL ;
}

//________________________________________________________________________________________________________
//  frame::XModel
//________________________________________________________________________________________________________

uno::Sequence< beans::PropertyValue > SAL_CALL SfxBaseModel::getArgs() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );
    if ( m_pData->m_pObjectShell.Is() )
    {
        uno::Sequence< beans::PropertyValue > seqArgsNew;
        uno::Sequence< beans::PropertyValue > seqArgsOld;
        SfxAllItemSet aSet( m_pData->m_pObjectShell->GetPool() );

        // we need to know which properties are supported by the transformer
        // hopefully it is a temporary solution, I guess nonconvertable properties
        // should not be supported so then there will be only ItemSet from medium

        TransformItems( SID_OPENDOC, *(m_pData->m_pObjectShell->GetMedium()->GetItemSet()), seqArgsNew );
        TransformParameters( SID_OPENDOC, m_pData->m_seqArguments, aSet );
        TransformItems( SID_OPENDOC, aSet, seqArgsOld );

        sal_Int32 nOrgLength = m_pData->m_seqArguments.getLength();
        sal_Int32 nOldLength = seqArgsOld.getLength();
        sal_Int32 nNewLength = seqArgsNew.getLength();

        // "WinExtent" property should be updated always.
        // We can store it now to overwrite an old value
        // since it is not from ItemSet
        Rectangle aTmpRect = m_pData->m_pObjectShell->GetVisArea( ASPECT_CONTENT );
        aTmpRect = OutputDevice::LogicToLogic( aTmpRect, m_pData->m_pObjectShell->GetMapUnit(), MAP_100TH_MM );

        Sequence< sal_Int32 > aRectSeq(4);
        aRectSeq[0] = aTmpRect.Left();
        aRectSeq[1] = aTmpRect.Top();
        aRectSeq[2] = aTmpRect.Right();
        aRectSeq[3] = aTmpRect.Bottom();

        seqArgsNew.realloc( ++nNewLength );
        seqArgsNew[ nNewLength - 1 ].Name = ::rtl::OUString("WinExtent");
        seqArgsNew[ nNewLength - 1 ].Value <<= aRectSeq;

        if ( !m_pData->m_aPreusedFilterName.isEmpty() )
        {
            seqArgsNew.realloc( ++nNewLength );
            seqArgsNew[ nNewLength - 1 ].Name = ::rtl::OUString("PreusedFilterName");
            seqArgsNew[ nNewLength - 1 ].Value <<= m_pData->m_aPreusedFilterName;
        }

        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( m_pData->m_pObjectShell );
        if ( pFrame )
        {
            SvBorder aBorder = pFrame->GetBorderPixelImpl( pFrame->GetViewShell() );

            Sequence< sal_Int32 > aBorderSeq(4);
            aBorderSeq[0] = aBorder.Left();
            aBorderSeq[1] = aBorder.Top();
            aBorderSeq[2] = aBorder.Right();
            aBorderSeq[3] = aBorder.Bottom();

            seqArgsNew.realloc( ++nNewLength );
            seqArgsNew[ nNewLength - 1 ].Name = ::rtl::OUString("DocumentBorder");
            seqArgsNew[ nNewLength - 1 ].Value <<= aBorderSeq;
        }

        // only the values that are not supported by the ItemSet must be cached here
        uno::Sequence< beans::PropertyValue > aFinalCache;
        sal_Int32 nFinalLength = 0;

        for ( sal_Int32 nOrg = 0; nOrg < nOrgLength; nOrg++ )
        {
             sal_Int32 nOldInd = 0;
            while ( nOldInd < nOldLength )
            {
                if ( m_pData->m_seqArguments[nOrg].Name.equals( seqArgsOld[nOldInd].Name ) )
                    break;
                nOldInd++;
            }

            if ( nOldInd == nOldLength )
            {
                // the entity with this name should be new for seqArgsNew
                // since it is not supported by transformer

                seqArgsNew.realloc( ++nNewLength );
                seqArgsNew[ nNewLength - 1 ] = m_pData->m_seqArguments[nOrg];

                aFinalCache.realloc( ++nFinalLength );
                aFinalCache[ nFinalLength - 1 ] = m_pData->m_seqArguments[nOrg];
            }
        }

        m_pData->m_seqArguments = aFinalCache;

        return seqArgsNew;
    }

    return m_pData->m_seqArguments;
}

//________________________________________________________________________________________________________
//  frame::XModel
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::connectController( const uno::Reference< frame::XController >& xController )
    throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );
    OSL_PRECOND( xController.is(), "SfxBaseModel::connectController: invalid controller!" );
    if ( !xController.is() )
        return;

    sal_uInt32 nOldCount = m_pData->m_seqControllers.getLength();
    uno::Sequence< uno::Reference< frame::XController > > aNewSeq( nOldCount + 1 );
    for ( sal_uInt32 n = 0; n < nOldCount; n++ )
        aNewSeq.getArray()[n] = m_pData->m_seqControllers.getConstArray()[n];
    aNewSeq.getArray()[nOldCount] = xController;
    m_pData->m_seqControllers = aNewSeq;

    if ( m_pData->m_seqControllers.getLength() == 1 )
    {
        SfxViewFrame* pViewFrame = SfxViewFrame::Get( xController, GetObjectShell() );
        ENSURE_OR_THROW( pViewFrame, "SFX document without SFX view!?" );
        bool bOldLock = pViewFrame->GetDispatcher()->IsLocked();
        pViewFrame->GetDispatcher()->Lock(sal_True);
        pViewFrame->UpdateDocument_Impl();
        pViewFrame->GetDispatcher()->Lock(bOldLock);
        const String sDocumentURL = GetObjectShell()->GetMedium()->GetName();
        if ( sDocumentURL.Len() )
            SFX_APP()->Broadcast( SfxStringHint( SID_OPENURL, sDocumentURL ) );
    }
}

//________________________________________________________________________________________________________
//  frame::XModel
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::disconnectController( const uno::Reference< frame::XController >& xController ) throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    sal_uInt32 nOldCount = m_pData->m_seqControllers.getLength();
    if ( !nOldCount )
        return;

    uno::Sequence< uno::Reference< frame::XController > > aNewSeq( nOldCount - 1 );
    for ( sal_uInt32 nOld = 0, nNew = 0; nOld < nOldCount; ++nOld )
    {
        if ( xController != m_pData->m_seqControllers.getConstArray()[nOld] )
        {
            aNewSeq.getArray()[nNew] = m_pData->m_seqControllers.getConstArray()[nOld];
            ++nNew;
        }
    }

    m_pData->m_seqControllers = aNewSeq;

    if ( xController == m_pData->m_xCurrent )
        m_pData->m_xCurrent = uno::Reference< frame::XController > ();
}

namespace
{
    typedef ::cppu::WeakImplHelper1< XUndoAction > ControllerLockUndoAction_Base;
    class ControllerLockUndoAction : public ControllerLockUndoAction_Base
    {
    public:
        ControllerLockUndoAction( const Reference< XModel >& i_model, const bool i_undoIsUnlock )
            :m_xModel( i_model )
            ,m_bUndoIsUnlock( i_undoIsUnlock )
        {
        }

        // XUndoAction
        virtual ::rtl::OUString SAL_CALL getTitle() throw (RuntimeException);
        virtual void SAL_CALL undo(  ) throw (UndoFailedException, RuntimeException);
        virtual void SAL_CALL redo(  ) throw (UndoFailedException, RuntimeException);

    private:
        const Reference< XModel >   m_xModel;
        const bool                  m_bUndoIsUnlock;
    };

    ::rtl::OUString SAL_CALL ControllerLockUndoAction::getTitle() throw (RuntimeException)
    {
        // this action is intended to be used within an UndoContext only, so nobody will ever see this title ...
        return ::rtl::OUString();
    }

    void SAL_CALL ControllerLockUndoAction::undo(  ) throw (UndoFailedException, RuntimeException)
    {
        if ( m_bUndoIsUnlock )
            m_xModel->unlockControllers();
        else
            m_xModel->lockControllers();
    }

    void SAL_CALL ControllerLockUndoAction::redo(  ) throw (UndoFailedException, RuntimeException)
    {
        if ( m_bUndoIsUnlock )
            m_xModel->lockControllers();
        else
            m_xModel->unlockControllers();
    }
}

//________________________________________________________________________________________________________
//  frame::XModel
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::lockControllers() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    ++m_pData->m_nControllerLockCount ;

    if  (   m_pData->m_pDocumentUndoManager.is()
        &&  m_pData->m_pDocumentUndoManager->isInContext()
        &&  !m_pData->m_pDocumentUndoManager->isLocked()
        )
    {
        m_pData->m_pDocumentUndoManager->addUndoAction( new ControllerLockUndoAction( this, true ) );
    }
}

//________________________________________________________________________________________________________
//  frame::XModel
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::unlockControllers() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    --m_pData->m_nControllerLockCount ;

    if  (   m_pData->m_pDocumentUndoManager.is()
        &&  m_pData->m_pDocumentUndoManager->isInContext()
        &&  !m_pData->m_pDocumentUndoManager->isLocked()
        )
    {
        m_pData->m_pDocumentUndoManager->addUndoAction( new ControllerLockUndoAction( this, false ) );
    }
}

//________________________________________________________________________________________________________
//  frame::XModel
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::hasControllersLocked() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );
    return ( m_pData->m_nControllerLockCount != 0 ) ;
}

//________________________________________________________________________________________________________
//  frame::XModel
//________________________________________________________________________________________________________

uno::Reference< frame::XController > SAL_CALL SfxBaseModel::getCurrentController() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    // get the last active controller of this model
    if ( m_pData->m_xCurrent.is() )
        return m_pData->m_xCurrent;

    // get the first controller of this model
    return m_pData->m_seqControllers.getLength() ? m_pData->m_seqControllers.getConstArray()[0] : m_pData->m_xCurrent;
}

//________________________________________________________________________________________________________
//  frame::XModel
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::setCurrentController( const uno::Reference< frame::XController >& xCurrentController )
        throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    m_pData->m_xCurrent = xCurrentController;
}

//________________________________________________________________________________________________________
//  frame::XModel
//________________________________________________________________________________________________________

uno::Reference< uno::XInterface > SAL_CALL SfxBaseModel::getCurrentSelection() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    uno::Reference< uno::XInterface >     xReturn;
    uno::Reference< frame::XController >    xController =   getCurrentController()      ;

    if ( xController.is() )
    {
        uno::Reference< view::XSelectionSupplier >  xDocView( xController, uno::UNO_QUERY );
        if ( xDocView.is() )
        {
            uno::Any xSel = xDocView->getSelection();
            xSel >>= xReturn ;
        }
    }

    return xReturn ;
}

//________________________________________________________________________________________________________
//  XModifiable2
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::disableSetModified() throw (::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.Is() )
        throw uno::RuntimeException();

    sal_Bool bResult = m_pData->m_pObjectShell->IsEnableSetModified();
    m_pData->m_pObjectShell->EnableSetModified( sal_False );

    return bResult;
}

sal_Bool SAL_CALL SfxBaseModel::enableSetModified() throw (::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.Is() )
        throw uno::RuntimeException();

    sal_Bool bResult = m_pData->m_pObjectShell->IsEnableSetModified();
    m_pData->m_pObjectShell->EnableSetModified( sal_True );

    return bResult;
}

sal_Bool SAL_CALL SfxBaseModel::isSetModifiedEnabled() throw (::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.Is() )
        throw uno::RuntimeException();

    return m_pData->m_pObjectShell->IsEnableSetModified();
}

//________________________________________________________________________________________________________
//  XModifiable
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::isModified() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->IsModified() : sal_False;
}

//________________________________________________________________________________________________________
//  XModifiable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::setModified( sal_Bool bModified )
        throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( m_pData->m_pObjectShell.Is() )
        m_pData->m_pObjectShell->SetModified(bModified);
}

//________________________________________________________________________________________________________
//  XModifiable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::addModifyListener(const uno::Reference< XMODIFYLISTENER >& xListener) throw( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const uno::Reference< XMODIFYLISTENER >*)0),xListener );
}

//________________________________________________________________________________________________________
//  XModifiable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::removeModifyListener(const uno::Reference< XMODIFYLISTENER >& xListener) throw( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const uno::Reference< XMODIFYLISTENER >*)0), xListener );
}

//____________________________________________________________________________________________________
//  XCloseable
//____________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::close( sal_Bool bDeliverOwnership ) throw (util::CloseVetoException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() || m_pData->m_bClosed || m_pData->m_bClosing )
        return;

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >(this) );
    lang::EventObject             aSource    (static_cast< ::cppu::OWeakObject*>(this));
    ::cppu::OInterfaceContainerHelper* pContainer = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                ((util::XCloseListener*)pIterator.next())->queryClosing( aSource, bDeliverOwnership );
            }
            catch( uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }

    if ( m_pData->m_bSaving )
    {
        if (bDeliverOwnership)
            m_pData->m_bSuicide = sal_True;
        throw util::CloseVetoException(
                ::rtl::OUString("Cant close while saving."),
                static_cast< ::com::sun::star::util::XCloseable* >(this));
    }

    // no own objections against closing!
    m_pData->m_bClosing = sal_True;
    pContainer = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pCloseIterator(*pContainer);
        while (pCloseIterator.hasMoreElements())
        {
            try
            {
                ((util::XCloseListener*)pCloseIterator.next())->notifyClosing( aSource );
            }
            catch( uno::RuntimeException& )
            {
                pCloseIterator.remove();
            }
        }
    }

    m_pData->m_bClosed = sal_True;
    m_pData->m_bClosing = sal_False;

    dispose();
}

//____________________________________________________________________________________________________
//  XCloseBroadcaster
//____________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::addCloseListener( const uno::Reference< XCLOSELISTENER >& xListener ) throw (uno::RuntimeException)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const uno::Reference< XCLOSELISTENER >*)0), xListener );
}

//____________________________________________________________________________________________________
//  XCloseBroadcaster
//____________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::removeCloseListener( const uno::Reference< XCLOSELISTENER >& xListener ) throw (uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const uno::Reference< XCLOSELISTENER >*)0), xListener );
}

//________________________________________________________________________________________________________
//  XPrintable
//________________________________________________________________________________________________________

uno::Sequence< beans::PropertyValue > SAL_CALL SfxBaseModel::getPrinter() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( impl_getPrintHelper() )
        return m_pData->m_xPrintable->getPrinter();
    else
        return uno::Sequence< beans::PropertyValue >();
}

void SAL_CALL SfxBaseModel::setPrinter(const uno::Sequence< beans::PropertyValue >& rPrinter)
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( impl_getPrintHelper() )
        m_pData->m_xPrintable->setPrinter( rPrinter );
}

void SAL_CALL SfxBaseModel::print(const uno::Sequence< beans::PropertyValue >& rOptions)
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( impl_getPrintHelper() )
        m_pData->m_xPrintable->print( rOptions );
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::hasLocation() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->HasName() : sal_False;
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

::rtl::OUString SAL_CALL SfxBaseModel::getLocation() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( m_pData->m_pObjectShell.Is() )
    {
        // TODO/LATER: is it correct that the shared document returns shared file location?
        if ( m_pData->m_pObjectShell->IsDocShared() )
            return m_pData->m_pObjectShell->GetSharedFileURL();
        else
            return ::rtl::OUString(m_pData->m_pObjectShell->GetMedium()->GetName());
    }

    return m_pData->m_sURL;
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

sal_Bool SAL_CALL SfxBaseModel::isReadonly() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    return m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->IsReadOnly() : sal_True;
}

//________________________________________________________________________________________________________
//  XStorable2
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::storeSelf( const    uno::Sequence< beans::PropertyValue >&  aSeqArgs )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException )
{
    RTL_LOGFILE_PRODUCT_CONTEXT( aPerfLog, "PERFORMANCE - SfxBaseModel::storeSelf" );

    SfxModelGuard aGuard( *this );

    if ( m_pData->m_pObjectShell.Is() )
    {
        m_pData->m_pObjectShell->AddLog( ::rtl::OUString( OSL_LOG_PREFIX "storeSelf"  ) );
        SfxSaveGuard aSaveGuard(this, m_pData, sal_False);

        for ( sal_Int32 nInd = 0; nInd < aSeqArgs.getLength(); nInd++ )
        {
            // check that only acceptable parameters are provided here
            if ( aSeqArgs[nInd].Name != "VersionComment" && aSeqArgs[nInd].Name != "Author"
              && aSeqArgs[nInd].Name != "InteractionHandler" && aSeqArgs[nInd].Name != "StatusIndicator" )
            {
                m_pData->m_pObjectShell->AddLog( ::rtl::OUString( OSL_LOG_PREFIX "unexpected parameter for storeSelf, might be no problem if SaveAs is executed."  ) );
                m_pData->m_pObjectShell->StoreLog();

                ::rtl::OUString aMessage( "Unexpected MediaDescriptor parameter: "  );
                aMessage += aSeqArgs[nInd].Name;
                throw lang::IllegalArgumentException( aMessage, uno::Reference< uno::XInterface >(), 1 );
            }
        }

        SfxAllItemSet *pParams = new SfxAllItemSet( SFX_APP()->GetPool() );
        TransformParameters( SID_SAVEDOC, aSeqArgs, *pParams );

        SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_SAVEDOC, GlobalEventConfig::GetEventName(STR_EVENT_SAVEDOC), m_pData->m_pObjectShell ) );

        sal_Bool bRet = sal_False;

        // TODO/LATER: let the embedded case of saving be handled more careful
        if ( m_pData->m_pObjectShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        {
            // If this is an embedded object that has no URL based location it should be stored to own storage.
            // An embedded object can have a location based on URL in case it is a link, then it should be
            // stored in normal way.
            if ( !hasLocation() || getLocation().compareToAscii( "private:", 8 ) == 0 )
            {
                // actually in this very rare case only UI parameters have sence
                // TODO/LATER: should be done later, after integration of sb19
                bRet = m_pData->m_pObjectShell->DoSave()
                    && m_pData->m_pObjectShell->DoSaveCompleted();
            }
            else
            {
                bRet = m_pData->m_pObjectShell->Save_Impl( pParams );
            }
        }
        else
            bRet = m_pData->m_pObjectShell->Save_Impl( pParams );

        DELETEZ( pParams );

        sal_uInt32 nErrCode = m_pData->m_pObjectShell->GetError() ? m_pData->m_pObjectShell->GetError()
                                                                    : ERRCODE_IO_CANTWRITE;
        m_pData->m_pObjectShell->ResetError();

        if ( bRet )
        {
            m_pData->m_pObjectShell->AddLog( ::rtl::OUString( OSL_LOG_PREFIX "successful saving."  ) );
            m_pData->m_aPreusedFilterName = GetMediumFilterName_Impl();

            SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_SAVEDOCDONE, GlobalEventConfig::GetEventName(STR_EVENT_SAVEDOCDONE), m_pData->m_pObjectShell ) );
        }
        else
        {
            m_pData->m_pObjectShell->AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Storing failed!"  ) );
            m_pData->m_pObjectShell->StoreLog();

            // write the contents of the logger to the file
            SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_SAVEDOCFAILED, GlobalEventConfig::GetEventName(STR_EVENT_SAVEDOCFAILED), m_pData->m_pObjectShell ) );

            throw task::ErrorCodeIOException( ::rtl::OUString(), uno::Reference< uno::XInterface >(), nErrCode );
        }
    }

}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::store() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    storeSelf( uno::Sequence< beans::PropertyValue >() );
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::storeAsURL( const   ::rtl::OUString&                   rURL    ,
                                        const   uno::Sequence< beans::PropertyValue >&  rArgs   )
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_PRODUCT_CONTEXT( aPerfLog, "PERFORMANCE - SfxBaseModel::storeAsURL" );

    SfxModelGuard aGuard( *this );

    if ( m_pData->m_pObjectShell.Is() )
    {
        m_pData->m_pObjectShell->AddLog( ::rtl::OUString( OSL_LOG_PREFIX "storeAsURL"  ) );
        SfxSaveGuard aSaveGuard(this, m_pData, sal_False);

        impl_store( rURL, rArgs, sal_False );

        uno::Sequence< beans::PropertyValue > aSequence ;
        TransformItems( SID_OPENDOC, *m_pData->m_pObjectShell->GetMedium()->GetItemSet(), aSequence );
        attachResource( rURL, aSequence );

#if OSL_DEBUG_LEVEL > 0
        SFX_ITEMSET_ARG( m_pData->m_pObjectShell->GetMedium()->GetItemSet(), pPasswdItem, SfxStringItem, SID_PASSWORD, sal_False);
        OSL_ENSURE( !pPasswdItem, "There should be no Password property in the document MediaDescriptor!" );
#endif
    }
}

//________________________________________________________________________________________________________
//  XUndoManagerSupplier
//________________________________________________________________________________________________________
Reference< XUndoManager > SAL_CALL SfxBaseModel::getUndoManager(  ) throw (RuntimeException)
{
    SfxModelGuard aGuard( *this );
    if ( !m_pData->m_pDocumentUndoManager.is() )
        m_pData->m_pDocumentUndoManager.set( new ::sfx2::DocumentUndoManager( *this ) );
    return m_pData->m_pDocumentUndoManager.get();
}

//________________________________________________________________________________________________________
//  XStorable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::storeToURL( const   ::rtl::OUString&                   rURL    ,
                                        const   uno::Sequence< beans::PropertyValue >&  rArgs   )
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( m_pData->m_pObjectShell.Is() )
    {
        m_pData->m_pObjectShell->AddLog( ::rtl::OUString( OSL_LOG_PREFIX "storeToURL"  ) );
        SfxSaveGuard aSaveGuard(this, m_pData, sal_False);
        impl_store( rURL, rArgs, sal_True );
    }
}

::sal_Bool SAL_CALL SfxBaseModel::wasModifiedSinceLastSave() throw ( RuntimeException )
{
    SfxModelGuard aGuard( *this );
    return m_pData->m_bModifiedSinceLastSave;
}

void SAL_CALL SfxBaseModel::storeToRecoveryFile( const ::rtl::OUString& i_TargetLocation, const Sequence< PropertyValue >& i_MediaDescriptor ) throw ( RuntimeException, IOException, WrappedTargetException )
{
    SfxModelGuard aGuard( *this );

    // delegate
    SfxSaveGuard aSaveGuard( this, m_pData, sal_False );
    impl_store( i_TargetLocation, i_MediaDescriptor, sal_True );

    // no need for subsequent calls to storeToRecoveryFile, unless we're modified, again
    m_pData->m_bModifiedSinceLastSave = sal_False;
}

void SAL_CALL SfxBaseModel::recoverFromFile( const ::rtl::OUString& i_SourceLocation, const ::rtl::OUString& i_SalvagedFile, const Sequence< PropertyValue >& i_MediaDescriptor ) throw ( RuntimeException, IOException, WrappedTargetException )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    // delegate to our "load" method
    ::comphelper::NamedValueCollection aMediaDescriptor( i_MediaDescriptor );

    // our load implementation expects the SalvagedFile to be in the media descriptor
    OSL_ENSURE( !aMediaDescriptor.has( "SalvagedFile" ) || ( aMediaDescriptor.getOrDefault( "SalvagedFile", ::rtl::OUString() ) == i_SalvagedFile ),
        "SfxBaseModel::recoverFromFile: inconsistent information!" );
    aMediaDescriptor.put( "SalvagedFile", i_SalvagedFile );

    // similar for the to-be-loaded file
    OSL_ENSURE( !aMediaDescriptor.has( "URL" ) || ( aMediaDescriptor.getOrDefault( "URL", ::rtl::OUString() ) == i_SourceLocation ),
        "SfxBaseModel::recoverFromFile: inconsistent information!" );
    aMediaDescriptor.put( "URL", i_SourceLocation );

    load( aMediaDescriptor.getPropertyValues() );

    // Note: The XDocumentRecovery interface specification requires us to do an attachResource after loading.
    // However, we will not do this here, as we know that our load implementation (respectively some method
    // called from there) already did so.
    // In particular, the load process might already have modified some elements of the media
    // descriptor, for instance the MacroExecMode (in case the user was involved to decide about it), and we do
    // not want to overwrite it with the "old" elements passed to this method here.
}

//________________________________________________________________________________________________________
// XLoadable
//________________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::initNew()
        throw (::com::sun::star::frame::DoubleInitializationException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    if ( IsInitialized() )
        throw ::com::sun::star::frame::DoubleInitializationException( ::rtl::OUString(), *this );

    // the object shell should exist always
    DBG_ASSERT( m_pData->m_pObjectShell.Is(), "Model is useless without an ObjectShell" );
    if ( m_pData->m_pObjectShell.Is() )
    {
        if( m_pData->m_pObjectShell->GetMedium() )
            throw frame::DoubleInitializationException();

        sal_Bool bRes = m_pData->m_pObjectShell->DoInitNew( NULL );
        sal_uInt32 nErrCode = m_pData->m_pObjectShell->GetError() ?
                                    m_pData->m_pObjectShell->GetError() : ERRCODE_IO_CANTCREATE;
        m_pData->m_pObjectShell->ResetError();

        if ( !bRes )
            throw task::ErrorCodeIOException( ::rtl::OUString(), uno::Reference< uno::XInterface >(), nErrCode );
    }
}

namespace {

OUString getFilterProvider(const uno::Sequence<beans::PropertyValue>& rArgs)
{
    OUString aStr;
    for (sal_Int32 i = 0, n = rArgs.getLength(); i < n; ++i)
    {
        if (rArgs[i].Name == "FilterProvider")
        {
            rArgs[i].Value >>= aStr;
            return aStr;
        }
    }
    return aStr;
}

}

void SAL_CALL SfxBaseModel::load(   const uno::Sequence< beans::PropertyValue >& seqArguments )
        throw (::com::sun::star::frame::DoubleInitializationException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException,
               ::com::sun::star::uno::Exception)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    if ( IsInitialized() )
        throw ::com::sun::star::frame::DoubleInitializationException( ::rtl::OUString(), *this );

    // the object shell should exist always
    DBG_ASSERT( m_pData->m_pObjectShell.Is(), "Model is useless without an ObjectShell" );

    if ( m_pData->m_pObjectShell.Is() )
    {
        if( m_pData->m_pObjectShell->GetMedium() )
            // if a Medium is present, the document is already initialized
            throw frame::DoubleInitializationException();

        SfxMedium* pMedium = new SfxMedium( seqArguments );

        OUString aFilterProvider = getFilterProvider(seqArguments);
        if (!aFilterProvider.isEmpty())
        {
            if (!m_pData->m_pObjectShell->DoLoadExternal(pMedium, aFilterProvider))
                delete pMedium;

            pMedium->SetUpdatePickList(false);
            return;
        }

        String aFilterName;
        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pFilterNameItem, SfxStringItem, SID_FILTER_NAME, sal_False );
        if( pFilterNameItem )
            aFilterName = pFilterNameItem->GetValue();
        if( !m_pData->m_pObjectShell->GetFactory().GetFilterContainer()->GetFilter4FilterName( aFilterName ) )
        {
            // filtername is not valid
            delete pMedium;
            throw frame::IllegalArgumentIOException();
        }

        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False );
        sal_Bool bSalvage = pSalvageItem ? sal_True : sal_False;

        // load document
        sal_uInt32 nError = ERRCODE_NONE;
        if ( !m_pData->m_pObjectShell->DoLoad(pMedium) )
            nError=ERRCODE_IO_GENERAL;

        // QUESTION: if the following happens outside of DoLoad, something important is missing there!
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > xHandler = pMedium->GetInteractionHandler();
        if( m_pData->m_pObjectShell->GetErrorCode() )
        {
            nError = m_pData->m_pObjectShell->GetErrorCode();
            if ( nError == ERRCODE_IO_BROKENPACKAGE && xHandler.is() )
            {
                ::rtl::OUString aDocName = pMedium->GetURLObject().getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
                SFX_ITEMSET_ARG( pMedium->GetItemSet(), pRepairItem, SfxBoolItem, SID_REPAIRPACKAGE, sal_False );
                if ( !pRepairItem || !pRepairItem->GetValue() )
                {
                    RequestPackageReparation aRequest( aDocName );
                    xHandler->handle( aRequest.GetRequest() );
                    if( aRequest.isApproved() )
                    {
                        // broken package: try second loading and allow repair
                        pMedium->GetItemSet()->Put( SfxBoolItem( SID_REPAIRPACKAGE, sal_True ) );
                        pMedium->GetItemSet()->Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
                        pMedium->GetItemSet()->Put( SfxStringItem( SID_DOCINFO_TITLE, aDocName ) );

                        // the error must be reset and the storage must be reopened in new mode
                        pMedium->ResetError();
                        pMedium->CloseStorage();
                        m_pData->m_pObjectShell->PrepareSecondTryLoad_Impl();
                        if ( !m_pData->m_pObjectShell->DoLoad(pMedium) )
                            nError=ERRCODE_IO_GENERAL;
                        nError = m_pData->m_pObjectShell->GetErrorCode();
                    }
                }

                if ( nError == ERRCODE_IO_BROKENPACKAGE )
                {
                    // repair either not allowed or not successful
                    NotifyBrokenPackage aRequest( aDocName );
                    xHandler->handle( aRequest.GetRequest() );
                }
            }
        }

        if( m_pData->m_pObjectShell->IsAbortingImport() )
            nError = ERRCODE_ABORT;

        if( bSalvage )
        {
            // file recovery: restore original filter
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False );
            SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
            const SfxFilter* pSetFilter = rMatcher.GetFilter4FilterName( pFilterItem->GetValue() );
            pMedium->SetFilter( pSetFilter );
            m_pData->m_pObjectShell->SetModified(sal_True);
        }

        // TODO/LATER: may be the mode should be retrieved from outside and the preused filter should not be set
        if ( m_pData->m_pObjectShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        {
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False );
            if ( pFilterItem )
                m_pData->m_aPreusedFilterName = pFilterItem->GetValue();
        }

        if ( !nError )
            nError = pMedium->GetError();

        m_pData->m_pObjectShell->ResetError();

        if ( nError )
        {
            sal_Bool bSilent = sal_False;
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSilentItem, SfxBoolItem, SID_SILENT, sal_False);
            if( pSilentItem )
                bSilent = pSilentItem->GetValue();

              sal_Bool bWarning = ((nError & ERRCODE_WARNING_MASK) == ERRCODE_WARNING_MASK);
            if ( nError != ERRCODE_IO_BROKENPACKAGE && !bSilent )
            {
                // broken package was handled already
                if ( SfxObjectShell::UseInteractionToHandleError( xHandler, nError ) && !bWarning )
                {
                    // abort loading (except for warnings)
                       nError = ERRCODE_IO_ABORT;
                }
            }

            if ( m_pData->m_pObjectShell->GetMedium() != pMedium )
            {
                // for whatever reason document now has another medium
                OSL_FAIL("Document has rejected the medium?!");
                delete pMedium;
            }

            if ( !bWarning )    // #i30711# don't abort loading if it's only a warning
            {
                throw task::ErrorCodeIOException( ::rtl::OUString(),
                                                    uno::Reference< uno::XInterface >(),
                                                    nError ? nError : ERRCODE_IO_CANTREAD );
            }
        }

        sal_Bool bHidden = sal_False;
        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pHidItem, SfxBoolItem, SID_HIDDEN, sal_False);
        if ( pHidItem )
            bHidden = pHidItem->GetValue();

#if OSL_DEBUG_LEVEL > 0
        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pPasswdItem, SfxStringItem, SID_PASSWORD, sal_False);
        OSL_ENSURE( !pPasswdItem, "There should be no Password property in the document MediaDescriptor!" );
#endif
        // !TODO: will be done by Framework!
        pMedium->SetUpdatePickList( !bHidden );
    }
}

//________________________________________________________________________________________________________
// XTransferable
//________________________________________________________________________________________________________

uno::Any SAL_CALL SfxBaseModel::getTransferData( const DATAFLAVOR& aFlavor )
        throw (::com::sun::star::datatransfer::UnsupportedFlavorException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    uno::Any aAny;

    if ( m_pData->m_pObjectShell.Is() )
    {
        if ( aFlavor.MimeType == "application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"" )
        {
            if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            {
                TransferableObjectDescriptor aDesc;

                aDesc.maClassName = m_pData->m_pObjectShell->GetClassName();
                aDesc.maTypeName = aFlavor.HumanPresentableName;

                // TODO/LATER: ViewAspect needs to be sal_Int64
                aDesc.mnViewAspect = sal::static_int_cast< sal_uInt16 >( embed::Aspects::MSOLE_CONTENT );

                //TODO/LATER: status needs to become sal_Int64
                aDesc.mnOle2Misc = m_pData->m_pObjectShell->GetMiscStatus();
                Size aSize = m_pData->m_pObjectShell->GetVisArea().GetSize();

                MapUnit aMapUnit = m_pData->m_pObjectShell->GetMapUnit();
                aDesc.maSize = OutputDevice::LogicToLogic( aSize, aMapUnit, MAP_100TH_MM );
                aDesc.maDragStartPos = Point();
                aDesc.maDisplayName = String();
                aDesc.mbCanLink = sal_False;

                SvMemoryStream aMemStm( 1024, 1024 );
                aMemStm << aDesc;
                aAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Tell() );
            }
            else
                throw datatransfer::UnsupportedFlavorException();
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-embed-source;windows_formatname=\"Star EMBS\"" )
        {
            if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            {
                try
                {
                    utl::TempFile aTmp;
                    aTmp.EnableKillingFile( sal_True );
                    storeToURL( aTmp.GetURL(), uno::Sequence < beans::PropertyValue >() );
                    SvStream* pStream = aTmp.GetStream( STREAM_READ );
                    const sal_uInt32 nLen = pStream->Seek( STREAM_SEEK_TO_END );
                    ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( nLen );
                    pStream->Seek( STREAM_SEEK_TO_BEGIN );
                    pStream->Read( aSeq.getArray(),  nLen );
                    delete pStream;
                    if( aSeq.getLength() )
                        aAny <<= aSeq;
                }
                catch ( uno::Exception& )
                {
                }
            }
            else
                throw datatransfer::UnsupportedFlavorException();
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"" )
        {
            if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            {

                ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                    m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    SvMemoryStream aMemStm( 65535, 65535 );
                    aMemStm.SetVersion( SOFFICE_FILEFORMAT_CURRENT );

                    pMetaFile->Write( aMemStm );
                    aAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ),
                                                    aMemStm.Seek( STREAM_SEEK_TO_END ) );
                }
            }
            else
                throw datatransfer::UnsupportedFlavorException();
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\"" )
        {
            if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            {
                ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                    m_pData->m_pObjectShell->CreatePreviewMetaFile_Impl( sal_True );

                if ( pMetaFile )
                {
                    SvMemoryStream aMemStm( 65535, 65535 );
                    aMemStm.SetVersion( SOFFICE_FILEFORMAT_CURRENT );

                    pMetaFile->Write( aMemStm );
                    aAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ),
                                                    aMemStm.Seek( STREAM_SEEK_TO_END ) );
                }
            }
            else
                throw datatransfer::UnsupportedFlavorException();
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-emf;windows_formatname=\"Image EMF\"" )
        {
            if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            {
                ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                    m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    ::boost::shared_ptr<SvMemoryStream> pStream(
                        GraphicHelper::getFormatStrFromGDI_Impl(
                            pMetaFile.get(), CVT_EMF ) );
                    if ( pStream )
                    {
                        pStream->SetVersion( SOFFICE_FILEFORMAT_CURRENT );
                        aAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( pStream->GetData() ),
                                                        pStream->Seek( STREAM_SEEK_TO_END ) );
                    }
                }
            }
            else if ( GraphicHelper::supportsMetaFileHandle_Impl()
              && aFlavor.DataType == getCppuType( (const sal_uInt64*) 0 ) )
            {
                ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                    m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    aAny <<= reinterpret_cast< const sal_uInt64 >(
                        GraphicHelper::getEnhMetaFileFromGDI_Impl( pMetaFile.get() ) );
                }
            }
            else
                throw datatransfer::UnsupportedFlavorException();
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"" )
        {
            if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            {
                ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                    m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    ::boost::shared_ptr<SvMemoryStream> pStream(
                        GraphicHelper::getFormatStrFromGDI_Impl(
                            pMetaFile.get(), CVT_WMF ) );

                    if ( pStream )
                    {
                        pStream->SetVersion( SOFFICE_FILEFORMAT_CURRENT );
                        aAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( pStream->GetData() ),
                                                        pStream->Seek( STREAM_SEEK_TO_END ) );
                    }
                }
            }
            else if ( GraphicHelper::supportsMetaFileHandle_Impl()
              && aFlavor.DataType == getCppuType( (const sal_uInt64*) 0 ) )
            {
                // means HGLOBAL handler to memory storage containing METAFILEPICT structure

                ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                    m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    Size aMetaSize = pMetaFile->GetPrefSize();
                    aAny <<= reinterpret_cast< const sal_uInt64 >(
                        GraphicHelper::getWinMetaFileFromGDI_Impl(
                            pMetaFile.get(), aMetaSize ) );
                }
            }
            else
                throw datatransfer::UnsupportedFlavorException();
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" )
        {
            if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            {
                ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                    m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    ::boost::shared_ptr<SvMemoryStream> pStream(
                        GraphicHelper::getFormatStrFromGDI_Impl(
                            pMetaFile.get(), CVT_BMP ) );

                    if ( pStream )
                    {
                        pStream->SetVersion( SOFFICE_FILEFORMAT_CURRENT );
                        aAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( pStream->GetData() ),
                                                        pStream->Seek( STREAM_SEEK_TO_END ) );
                    }
                }
            }
            else
                throw datatransfer::UnsupportedFlavorException();
        }
        else if ( aFlavor.MimeType == "image/png" )
        {
            if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            {
                ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                    m_pData->m_pObjectShell->GetPreviewMetaFile( sal_True );

                if ( pMetaFile )
                {
                    ::boost::shared_ptr<SvMemoryStream> pStream(
                        GraphicHelper::getFormatStrFromGDI_Impl(
                            pMetaFile.get(), CVT_PNG ) );

                    if ( pStream )
                    {
                        pStream->SetVersion( SOFFICE_FILEFORMAT_CURRENT );
                        aAny <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( pStream->GetData() ),
                                                        pStream->Seek( STREAM_SEEK_TO_END ) );
                    }
                }
            }
            else
                throw datatransfer::UnsupportedFlavorException();
        }
        else
            throw datatransfer::UnsupportedFlavorException();
    }

    return aAny;
}

//________________________________________________________________________________________________________
// XTransferable
//________________________________________________________________________________________________________


uno::Sequence< DATAFLAVOR > SAL_CALL SfxBaseModel::getTransferDataFlavors()
        throw (::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    sal_Int32 nSuppFlavors = GraphicHelper::supportsMetaFileHandle_Impl() ? 10 : 8;
    uno::Sequence< DATAFLAVOR > aFlavorSeq( nSuppFlavors );

    aFlavorSeq[0].MimeType =
        ::rtl::OUString( "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\""  );
    aFlavorSeq[0].HumanPresentableName = ::rtl::OUString( "GDIMetaFile"  );
    aFlavorSeq[0].DataType = getCppuType( (const Sequence< sal_Int8 >*) 0 );

    aFlavorSeq[1].MimeType =
        ::rtl::OUString( "application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\""  );
    aFlavorSeq[1].HumanPresentableName = ::rtl::OUString( "GDIMetaFile"  );
    aFlavorSeq[1].DataType = getCppuType( (const Sequence< sal_Int8 >*) 0 );

    aFlavorSeq[2].MimeType =
        ::rtl::OUString( "application/x-openoffice-emf;windows_formatname=\"Image EMF\""  );
    aFlavorSeq[2].HumanPresentableName = ::rtl::OUString( "Enhanced Windows MetaFile"  );
    aFlavorSeq[2].DataType = getCppuType( (const Sequence< sal_Int8 >*) 0 );

    aFlavorSeq[3].MimeType =
        ::rtl::OUString( "application/x-openoffice-wmf;windows_formatname=\"Image WMF\""  );
    aFlavorSeq[3].HumanPresentableName = ::rtl::OUString( "Windows MetaFile"  );
    aFlavorSeq[3].DataType = getCppuType( (const Sequence< sal_Int8 >*) 0 );

    aFlavorSeq[4].MimeType =
        ::rtl::OUString( "application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"" );
    aFlavorSeq[4].HumanPresentableName = ::rtl::OUString( "Star Object Descriptor (XML)"  );
    aFlavorSeq[4].DataType = getCppuType( (const Sequence< sal_Int8 >*) 0 );

    aFlavorSeq[5].MimeType =
        ::rtl::OUString( "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\""  );
    aFlavorSeq[5].HumanPresentableName = ::rtl::OUString( "Star Embed Source (XML)"  );
    aFlavorSeq[5].DataType = getCppuType( (const Sequence< sal_Int8 >*) 0 );

    aFlavorSeq[6].MimeType =
        ::rtl::OUString( "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\""  );
    aFlavorSeq[6].HumanPresentableName = ::rtl::OUString( "Bitmap"  );
    aFlavorSeq[6].DataType = getCppuType( (const Sequence< sal_Int8 >*) 0 );

    aFlavorSeq[7].MimeType =
        ::rtl::OUString( "image/png"  );
    aFlavorSeq[7].HumanPresentableName = ::rtl::OUString( "PNG"  );
    aFlavorSeq[7].DataType = getCppuType( (const Sequence< sal_Int8 >*) 0 );

    if ( nSuppFlavors == 10 )
    {
        aFlavorSeq[8].MimeType =
            ::rtl::OUString( "application/x-openoffice-emf;windows_formatname=\"Image EMF\""  );
        aFlavorSeq[8].HumanPresentableName = ::rtl::OUString( "Enhanced Windows MetaFile"  );
        aFlavorSeq[8].DataType = getCppuType( (const sal_uInt64*) 0 );

        aFlavorSeq[9].MimeType =
            ::rtl::OUString( "application/x-openoffice-wmf;windows_formatname=\"Image WMF\""  );
        aFlavorSeq[9].HumanPresentableName = ::rtl::OUString( "Windows MetaFile"  );
        aFlavorSeq[9].DataType = getCppuType( (const sal_uInt64*) 0 );
    }

    return aFlavorSeq;
}

//________________________________________________________________________________________________________
// XTransferable
//________________________________________________________________________________________________________


sal_Bool SAL_CALL SfxBaseModel::isDataFlavorSupported( const DATAFLAVOR& aFlavor )
        throw (::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( aFlavor.MimeType == "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"" )
    {
        if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            return sal_True;
    }
    else if ( aFlavor.MimeType == "application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\"" )
    {
        if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            return sal_True;
    }
    else if ( aFlavor.MimeType == "application/x-openoffice-emf;windows_formatname=\"Image EMF\"" )
    {
        if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            return sal_True;
        else if ( GraphicHelper::supportsMetaFileHandle_Impl()
          && aFlavor.DataType == getCppuType( (const sal_uInt64*) 0 ) )
            return sal_True;
    }
    else if ( aFlavor.MimeType == "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"" )
    {
        if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            return sal_True;
        else if ( GraphicHelper::supportsMetaFileHandle_Impl()
          && aFlavor.DataType == getCppuType( (const sal_uInt64*) 0 ) )
            return sal_True;
    }
    else if ( aFlavor.MimeType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"")) )
    {
        if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            return sal_True;
    }
    else if ( aFlavor.MimeType == "application/x-openoffice-embed-source;windows_formatname=\"Star EMBS\"" )
    {
        if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            return sal_True;
    }
    else if ( aFlavor.MimeType == "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" )
    {
        if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            return sal_True;
    }
    else if ( aFlavor.MimeType == "image/png" )
    {
        if ( aFlavor.DataType == getCppuType( (const Sequence< sal_Int8 >*) 0 ) )
            return sal_True;
    }

    return sal_False;
}


//--------------------------------------------------------------------------------------------------------
//  XEventsSupplier
//--------------------------------------------------------------------------------------------------------

uno::Reference< container::XNameReplace > SAL_CALL SfxBaseModel::getEvents() throw( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    if ( ! m_pData->m_xEvents.is() )
    {
        m_pData->m_xEvents = new SfxEvents_Impl( m_pData->m_pObjectShell, this );
    }

    return m_pData->m_xEvents;
}

//--------------------------------------------------------------------------------------------------------
//  XEmbeddedScripts
//--------------------------------------------------------------------------------------------------------

uno::Reference< script::XStorageBasedLibraryContainer > SAL_CALL SfxBaseModel::getBasicLibraries() throw (RuntimeException)
{
    SfxModelGuard aGuard( *this );

    uno::Reference< script::XStorageBasedLibraryContainer > xBasicLibraries;
    if ( m_pData->m_pObjectShell )
        xBasicLibraries.set( m_pData->m_pObjectShell->GetBasicContainer(), UNO_QUERY_THROW );
    return xBasicLibraries;
}

uno::Reference< script::XStorageBasedLibraryContainer > SAL_CALL SfxBaseModel::getDialogLibraries() throw (RuntimeException)
{
    SfxModelGuard aGuard( *this );

    uno::Reference< script::XStorageBasedLibraryContainer > xDialogLibraries;
    if ( m_pData->m_pObjectShell )
        xDialogLibraries.set( m_pData->m_pObjectShell->GetDialogContainer(), UNO_QUERY_THROW );
    return xDialogLibraries;
}

::sal_Bool SAL_CALL SfxBaseModel::getAllowMacroExecution() throw (RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( m_pData->m_pObjectShell )
        return m_pData->m_pObjectShell->AdjustMacroMode( String(), false );
    return sal_False;
}

//--------------------------------------------------------------------------------------------------------
//  XScriptInvocationContext
//--------------------------------------------------------------------------------------------------------

Reference< document::XEmbeddedScripts > SAL_CALL SfxBaseModel::getScriptContainer() throw (RuntimeException)
{
    SfxModelGuard aGuard( *this );

    Reference< document::XEmbeddedScripts > xDocumentScripts;

    try
    {
        Reference< frame::XModel > xDocument( this );
        xDocumentScripts.set( xDocument, uno::UNO_QUERY );
        while ( !xDocumentScripts.is() && xDocument.is() )
        {
            Reference< container::XChild > xDocAsChild( xDocument, uno::UNO_QUERY );
            if ( !xDocAsChild.is() )
            {
                xDocument = NULL;
                break;
            }

            xDocument.set( xDocAsChild->getParent(), uno::UNO_QUERY );
            xDocumentScripts.set( xDocument, uno::UNO_QUERY );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
        xDocumentScripts = NULL;
    }

    return xDocumentScripts;
}

//--------------------------------------------------------------------------------------------------------
//  XEventBroadcaster
//--------------------------------------------------------------------------------------------------------

void SAL_CALL SfxBaseModel::addEventListener( const uno::Reference< XDOCEVENTLISTENER >& aListener ) throw( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const uno::Reference< XDOCEVENTLISTENER >*)0), aListener );
}

//--------------------------------------------------------------------------------------------------------
//  XEventBroadcaster
//--------------------------------------------------------------------------------------------------------

void SAL_CALL SfxBaseModel::removeEventListener( const uno::Reference< XDOCEVENTLISTENER >& aListener ) throw( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const uno::Reference< XDOCEVENTLISTENER >*)0), aListener );
}

//--------------------------------------------------------------------------------------------------------
//  XDocumentEventBroadcaster
//--------------------------------------------------------------------------------------------------------
// ---------------------------------
void SAL_CALL SfxBaseModel::addDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& aListener )
    throw ( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const uno::Reference< document::XDocumentEventListener >*)0), aListener );
}

// ---------------------------------
void SAL_CALL SfxBaseModel::removeDocumentEventListener( const uno::Reference< document::XDocumentEventListener >& aListener )
    throw ( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );
    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const uno::Reference< document::XDocumentEventListener >*)0), aListener );
}

// ---------------------------------
void SAL_CALL SfxBaseModel::notifyDocumentEvent( const ::rtl::OUString&, const uno::Reference< frame::XController2 >&, const uno::Any& )
    throw ( lang::IllegalArgumentException, lang::NoSupportException, uno::RuntimeException )
{
    throw lang::NoSupportException( ::rtl::OUString( "SfxBaseModel controlls all the sent notifications itself!"  ), uno::Reference< uno::XInterface >() );
}

//________________________________________________________________________________________________________
//  SfxListener
//________________________________________________________________________________________________________

void addTitle_Impl( Sequence < ::com::sun::star::beans::PropertyValue >& rSeq, const ::rtl::OUString& rTitle )
{
    sal_Int32 nCount = rSeq.getLength();
    sal_Int32 nArg;

    for ( nArg = 0; nArg < nCount; nArg++ )
    {
        ::com::sun::star::beans::PropertyValue& rProp = rSeq[nArg];
        if ( rProp.Name == "Title" )
        {
            rProp.Value <<= rTitle;
            break;
        }
    }

    if ( nArg == nCount )
    {
        rSeq.realloc( nCount+1 );
        rSeq[nCount].Name = ::rtl::OUString( "Title" );
        rSeq[nCount].Value <<= rTitle;
    }
}

void SfxBaseModel::Notify(          SfxBroadcaster& rBC     ,
                             const  SfxHint&        rHint   )
{
    if ( !m_pData )
        return;

    if ( &rBC == m_pData->m_pObjectShell )
    {
        SfxSimpleHint* pSimpleHint = PTR_CAST( SfxSimpleHint, &rHint );
        if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DOCCHANGED )
            changing();

        SfxEventHint* pNamedHint = PTR_CAST( SfxEventHint, &rHint );
        if ( pNamedHint )
        {

            switch ( pNamedHint->GetEventId() )
            {
            case SFX_EVENT_STORAGECHANGED:
            {
                if ( m_pData->m_xUIConfigurationManager.is()
                  && m_pData->m_pObjectShell->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                {
                    uno::Reference< XSTORAGE > xConfigStorage;
                    rtl::OUString aUIConfigFolderName( "Configurations2" );

                    xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, com::sun::star::embed::ElementModes::READWRITE );
                    if ( !xConfigStorage.is() )
                        xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, com::sun::star::embed::ElementModes::READ );

                    if ( xConfigStorage.is() || !m_pData->m_pObjectShell->GetStorage()->hasByName( aUIConfigFolderName ) )
                    {
                        // the storage is different, since otherwise it could not be opened, so it must be exchanged
                        Reference< ui::XUIConfigurationStorage > xUIConfigStorage( m_pData->m_xUIConfigurationManager, uno::UNO_QUERY );
                        xUIConfigStorage->setStorage( xConfigStorage );
                    }
                    else
                    {
                        OSL_FAIL( "Unexpected scenario!\n" );
                    }
                }

                ListenForStorage_Impl( m_pData->m_pObjectShell->GetStorage() );
            }
            break;

            case SFX_EVENT_LOADFINISHED:
            {
                impl_getPrintHelper();
                ListenForStorage_Impl( m_pData->m_pObjectShell->GetStorage() );
                m_pData->m_bModifiedSinceLastSave = sal_False;
            }
            break;

            case SFX_EVENT_SAVEASDOCDONE:
            {
                m_pData->m_sURL = m_pData->m_pObjectShell->GetMedium()->GetName();

                SfxItemSet *pSet = m_pData->m_pObjectShell->GetMedium()->GetItemSet();
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
                ::rtl::OUString aTitle = m_pData->m_pObjectShell->GetTitle();
                TransformItems( SID_SAVEASDOC, *pSet, aArgs );
                addTitle_Impl( aArgs, aTitle );
                attachResource( m_pData->m_pObjectShell->GetMedium()->GetName(), aArgs );
            }
            break;

            case SFX_EVENT_DOCCREATED:
            {
                impl_getPrintHelper();
                m_pData->m_bModifiedSinceLastSave = sal_False;
            }
            break;

            case SFX_EVENT_MODIFYCHANGED:
            {
                m_pData->m_bModifiedSinceLastSave = isModified();
            }
            break;
            }


            SfxViewEventHint* pViewHint = PTR_CAST( SfxViewEventHint, &rHint );
            postEvent_Impl( pNamedHint->GetEventName(), pViewHint ? pViewHint->GetController() : uno::Reference< frame::XController2 >() );
        }

        if ( pSimpleHint )
        {
            if ( pSimpleHint->GetId() == SFX_HINT_TITLECHANGED )
            {
                ::rtl::OUString aTitle = m_pData->m_pObjectShell->GetTitle();
                addTitle_Impl( m_pData->m_seqArguments, aTitle );
                postEvent_Impl( GlobalEventConfig::GetEventName( STR_EVENT_TITLECHANGED ) );
            }
            if ( pSimpleHint->GetId() == SFX_HINT_MODECHANGED )
            {
                postEvent_Impl( GlobalEventConfig::GetEventName( STR_EVENT_MODECHANGED ) );
            }
        }
    }
}

//________________________________________________________________________________________________________
//  public impl.
//________________________________________________________________________________________________________

void SfxBaseModel::NotifyModifyListeners_Impl() const
{
    ::cppu::OInterfaceContainerHelper* pIC = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType((const uno::Reference< XMODIFYLISTENER >*)0) );
    if ( pIC )
    {
        lang::EventObject aEvent( (frame::XModel *)this );
        pIC->notifyEach( &util::XModifyListener::modified, aEvent );
    }

    // this notification here is done too generously, we cannot simply assume that we're really modified
    // now, but we need to check it ...
    m_pData->m_bModifiedSinceLastSave = const_cast< SfxBaseModel* >( this )->isModified();
}

void SfxBaseModel::changing()
{
    SfxModelGuard aGuard( *this );

    // the notification should not be sent if the document can not be modified
    if ( !m_pData->m_pObjectShell.Is() || !m_pData->m_pObjectShell->IsEnableSetModified() )
        return;

    NotifyModifyListeners_Impl();
}

//________________________________________________________________________________________________________
//  public impl.
//________________________________________________________________________________________________________

SfxObjectShell* SfxBaseModel::GetObjectShell() const
{
    return m_pData ? (SfxObjectShell*) m_pData->m_pObjectShell : 0;
}

//________________________________________________________________________________________________________
//  public impl.
//________________________________________________________________________________________________________

sal_Bool SfxBaseModel::IsInitialized() const
{
    if ( !m_pData || !m_pData->m_pObjectShell )
    {
        OSL_FAIL( "SfxBaseModel::IsInitialized: this should have been caught earlier!" );
        return sal_False;
    }

    return m_pData->m_pObjectShell->GetMedium() != NULL;
}

void SfxBaseModel::MethodEntryCheck( const bool i_mustBeInitialized ) const
{
    if ( impl_isDisposed() )
        throw ::com::sun::star::lang::DisposedException( ::rtl::OUString(), *const_cast< SfxBaseModel* >( this ) );
    if ( i_mustBeInitialized && !IsInitialized() )
        throw ::com::sun::star::lang::NotInitializedException( ::rtl::OUString(), *const_cast< SfxBaseModel* >( this ) );
}

sal_Bool SfxBaseModel::impl_isDisposed() const
{
    return ( m_pData == NULL ) ;
}

//________________________________________________________________________________________________________
//  private impl.
//________________________________________________________________________________________________________

::rtl::OUString SfxBaseModel::GetMediumFilterName_Impl()
{
    const SfxFilter* pFilter = NULL;
    SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
    if ( pMedium )
        pFilter = pMedium->GetFilter();

    if ( pFilter )
        return pFilter->GetName();

    return ::rtl::OUString();
}

void SfxBaseModel::impl_store(  const   ::rtl::OUString&                   sURL            ,
                                const   uno::Sequence< beans::PropertyValue >&  seqArguments    ,
                                        sal_Bool                    bSaveTo         )
{
    if( sURL.isEmpty() )
        throw frame::IllegalArgumentIOException();

    sal_Bool bSaved = sal_False;
    if ( !bSaveTo && m_pData->m_pObjectShell && !sURL.isEmpty()
      && sURL.compareToAscii( "private:stream", 14 ) != COMPARE_EQUAL
      && ::utl::UCBContentHelper::EqualURLs( getLocation(), sURL ) )
    {
        // this is the same file URL as the current document location, try to use storeOwn if possible

        ::comphelper::SequenceAsHashMap aArgHash( seqArguments );
        ::rtl::OUString aFilterString( "FilterName"  );
        ::rtl::OUString aFilterName = aArgHash.getUnpackedValueOrDefault( aFilterString, ::rtl::OUString() );
        if ( !aFilterName.isEmpty() )
        {
            SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
            if ( pMedium )
            {
                const SfxFilter* pFilter = pMedium->GetFilter();
                if ( pFilter && aFilterName.equals( pFilter->GetFilterName() ) )
                {
                    aArgHash.erase( aFilterString );
                    aArgHash.erase( ::rtl::OUString( "URL"  ) );

                    try
                    {
                        storeSelf( aArgHash.getAsConstPropertyValueList() );
                        bSaved = sal_True;
                    }
                    catch( const lang::IllegalArgumentException& )
                    {
                        // some additional arguments do not allow to use saving, SaveAs should be done
                        // but only for normal documents, the shared documents would be overwritten in this case
                        // that would mean an information loss
                        // TODO/LATER: need a new interaction for this case
                        if ( m_pData->m_pObjectShell->IsDocShared() )
                        {
                            m_pData->m_pObjectShell->AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Can't store shared document!"  ) );
                            m_pData->m_pObjectShell->StoreLog();

                            uno::Sequence< beans::NamedValue > aNewEncryptionData = aArgHash.getUnpackedValueOrDefault( ::rtl::OUString( "EncryptionData"  ), uno::Sequence< beans::NamedValue >() );
                            if ( !aNewEncryptionData.getLength() )
                            {
                                ::rtl::OUString aNewPassword = aArgHash.getUnpackedValueOrDefault( ::rtl::OUString( "Password"  ), ::rtl::OUString() );
                                aNewEncryptionData = ::comphelper::OStorageHelper::CreatePackageEncryptionData( aNewPassword );
                            }

                            uno::Sequence< beans::NamedValue > aOldEncryptionData;
                            GetEncryptionData_Impl( pMedium->GetItemSet(), aOldEncryptionData );

                            if ( !aOldEncryptionData.getLength() && !aNewEncryptionData.getLength() )
                                throw;
                            else
                            {
                                // if the password is changed a special error should be used in case of shared document
                                throw task::ErrorCodeIOException( ::rtl::OUString( "Cant change password for shared document."  ), uno::Reference< uno::XInterface >(), ERRCODE_SFX_SHARED_NOPASSWORDCHANGE );
                            }
                        }
                    }
                }
            }
        }
    }

    if ( !bSaved && m_pData->m_pObjectShell )
    {
        SFX_APP()->NotifyEvent( SfxEventHint( bSaveTo ? SFX_EVENT_SAVETODOC : SFX_EVENT_SAVEASDOC, GlobalEventConfig::GetEventName( bSaveTo ? STR_EVENT_SAVETODOC : STR_EVENT_SAVEASDOC ),
                                                m_pData->m_pObjectShell ) );

        SfxAllItemSet *aParams = new SfxAllItemSet( SFX_APP()->GetPool() );
        aParams->Put( SfxStringItem( SID_FILE_NAME, String(sURL) ) );
        if ( bSaveTo )
            aParams->Put( SfxBoolItem( SID_SAVETO, sal_True ) );

        TransformParameters( SID_SAVEASDOC, seqArguments, *aParams );

        SFX_ITEMSET_ARG( aParams, pCopyStreamItem, SfxBoolItem, SID_COPY_STREAM_IF_POSSIBLE, sal_False );

        if ( pCopyStreamItem && pCopyStreamItem->GetValue() && !bSaveTo )
        {
            m_pData->m_pObjectShell->AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Misuse of CopyStreamIfPossible!"  ) );
            m_pData->m_pObjectShell->StoreLog();

            throw frame::IllegalArgumentIOException(
                    ::rtl::OUString( "CopyStreamIfPossible parameter is not acceptable for storeAsURL() call!"),
                    uno::Reference< uno::XInterface >() );
        }

        sal_uInt32 nModifyPasswordHash = 0;
        uno::Sequence< beans::PropertyValue > aModifyPasswordInfo;
        SFX_ITEMSET_ARG( aParams, pModifyPasswordInfoItem, SfxUnoAnyItem, SID_MODIFYPASSWORDINFO, sal_False );
        if ( pModifyPasswordInfoItem )
        {
            // it contains either a simple hash or a set of PropertyValues
            // TODO/LATER: the sequence of PropertyValue should replace the hash completely in future
            sal_Int32 nMPHTmp = 0;
            pModifyPasswordInfoItem->GetValue() >>= nMPHTmp;
            nModifyPasswordHash = (sal_uInt32)nMPHTmp;
            pModifyPasswordInfoItem->GetValue() >>= aModifyPasswordInfo;
        }
        aParams->ClearItem( SID_MODIFYPASSWORDINFO );
        sal_uInt32 nOldModifyPasswordHash = m_pData->m_pObjectShell->GetModifyPasswordHash();
        m_pData->m_pObjectShell->SetModifyPasswordHash( nModifyPasswordHash );
        uno::Sequence< beans::PropertyValue > aOldModifyPasswordInfo = m_pData->m_pObjectShell->GetModifyPasswordInfo();
        m_pData->m_pObjectShell->SetModifyPasswordInfo( aModifyPasswordInfo );

        // since saving a document modifies its DocumentInfo, the current
        // DocumentInfo must be saved on "SaveTo", so it can be restored
        // after saving
        sal_Bool bCopyTo =  bSaveTo ||
            m_pData->m_pObjectShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED;
        uno::Reference<document::XDocumentProperties> xOldDocProps;
        uno::Reference<document::XDocumentInfo> xOldDocInfo;
        if ( bCopyTo )
        {
            xOldDocProps = getDocumentProperties();
            if (m_pData->m_xDocumentInfo.is())
            {
                xOldDocInfo = getDocumentInfo();
                const Reference<util::XCloneable> xCloneable(xOldDocInfo,
                    UNO_QUERY_THROW);
                const Reference<document::XDocumentInfo> xNewDocInfo(
                    xCloneable->createClone(), UNO_QUERY_THROW);
                const Reference<document::XDocumentPropertiesSupplier> xDPS(
                    xNewDocInfo, UNO_QUERY_THROW);
                const Reference<document::XDocumentProperties> xNewDocProps(
                    xDPS->getDocumentProperties());
                m_pData->m_xDocumentProperties = xNewDocProps;
                m_pData->m_xDocumentInfo = xNewDocInfo;
            }
            else // try not to create DocumentInfo if it does not exist...
            {
                const Reference<util::XCloneable> xCloneable(xOldDocProps,
                    UNO_QUERY_THROW);
                const Reference<document::XDocumentProperties> xNewDocProps(
                    xCloneable->createClone(), UNO_QUERY_THROW);
                m_pData->m_xDocumentProperties = xNewDocProps;
            }
        }

        sal_Bool bRet = m_pData->m_pObjectShell->APISaveAs_Impl( sURL, aParams );

        if ( bCopyTo )
        {
            // restore DocumentInfo if a copy was created
            m_pData->m_xDocumentProperties = xOldDocProps;
            m_pData->m_xDocumentInfo = xOldDocInfo;
        }

        uno::Reference < task::XInteractionHandler > xHandler;
        SFX_ITEMSET_ARG( aParams, pItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, sal_False);
        if ( pItem )
            pItem->GetValue() >>= xHandler;

        DELETEZ( aParams );

        sal_uInt32 nErrCode = m_pData->m_pObjectShell->GetErrorCode();
        if ( !bRet && !nErrCode )
        {
            m_pData->m_pObjectShell->AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Storing has failed, no error is set!"  ) );
            nErrCode = ERRCODE_IO_CANTWRITE;
        }
        m_pData->m_pObjectShell->ResetError();

        if ( bRet )
        {
            if ( nErrCode )
            {
                // must be a warning - use Interactionhandler if possible or abandone
                if ( xHandler.is() )
                {
                    // TODO/LATER: a general way to set the error context should be available
                    SfxErrorContext aEc( ERRCTX_SFX_SAVEASDOC, m_pData->m_pObjectShell->GetTitle() );

                    ::com::sun::star::task::ErrorCodeRequest aErrorCode;
                    aErrorCode.ErrCode = nErrCode;
                    SfxMedium::CallApproveHandler( xHandler, uno::makeAny( aErrorCode ), sal_False );
                }
            }

            m_pData->m_pObjectShell->AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Storing succeeded!"  ) );
            if ( !bSaveTo )
            {
                m_pData->m_aPreusedFilterName = GetMediumFilterName_Impl();
                m_pData->m_pObjectShell->SetModifyPasswordEntered();

                SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_SAVEASDOCDONE, GlobalEventConfig::GetEventName(STR_EVENT_SAVEASDOCDONE), m_pData->m_pObjectShell ) );
            }
            else
            {
                m_pData->m_pObjectShell->SetModifyPasswordHash( nOldModifyPasswordHash );
                m_pData->m_pObjectShell->SetModifyPasswordInfo( aOldModifyPasswordInfo );

                SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_SAVETODOCDONE, GlobalEventConfig::GetEventName(STR_EVENT_SAVETODOCDONE), m_pData->m_pObjectShell ) );
            }
        }
        else
        {
            // let the logring be stored to the related file
            m_pData->m_pObjectShell->AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Storing failed!"  ) );
            m_pData->m_pObjectShell->StoreLog();

            m_pData->m_pObjectShell->SetModifyPasswordHash( nOldModifyPasswordHash );
            m_pData->m_pObjectShell->SetModifyPasswordInfo( aOldModifyPasswordInfo );


            SFX_APP()->NotifyEvent( SfxEventHint( bSaveTo ? SFX_EVENT_SAVETODOCFAILED : SFX_EVENT_SAVEASDOCFAILED, GlobalEventConfig::GetEventName( bSaveTo ? STR_EVENT_SAVETODOCFAILED : STR_EVENT_SAVEASDOCFAILED),
                                                    m_pData->m_pObjectShell ) );

            throw task::ErrorCodeIOException( ::rtl::OUString(), uno::Reference< uno::XInterface >(), nErrCode );
        }
    }
}

//********************************************************************************************************
namespace {
template< typename ListenerT, typename EventT >
class NotifySingleListenerIgnoreRE
{
private:
    typedef void ( SAL_CALL ListenerT::*NotificationMethod )( const EventT& );
    NotificationMethod  m_pMethod;
    const EventT&       m_rEvent;
public:
    NotifySingleListenerIgnoreRE( NotificationMethod method, const EventT& event ) : m_pMethod( method ), m_rEvent( event ) { }

    void operator()( const uno::Reference<ListenerT>& listener ) const
    {
        try
        {
            (listener.get()->*m_pMethod)( m_rEvent );
        }
        catch( uno::RuntimeException& )
        {
            // this exception is ignored to avoid problems with invalid listeners, the listener should be probably thrown away in future
        }
    }
};
} // anonymous namespace

void SfxBaseModel::postEvent_Impl( const ::rtl::OUString& aName, const uno::Reference< frame::XController2 >& xController )
{
    // object already disposed?
    if ( impl_isDisposed() )
        return;

    DBG_ASSERT( !aName.isEmpty(), "Empty event name!" );
    if (aName.isEmpty())
        return;

    ::cppu::OInterfaceContainerHelper* pIC =
        m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( (const uno::Reference< document::XDocumentEventListener >*)0 ) );
    if ( pIC )
    {
        SAL_INFO("sfx2", "SfxDocumentEvent: " + aName);

        document::DocumentEvent aDocumentEvent( (frame::XModel*)this, aName, xController, uno::Any() );

        pIC->forEach< document::XDocumentEventListener, NotifySingleListenerIgnoreRE< document::XDocumentEventListener, document::DocumentEvent > >(
            NotifySingleListenerIgnoreRE< document::XDocumentEventListener, document::DocumentEvent >(
                &document::XDocumentEventListener::documentEventOccured,
                aDocumentEvent ) );
    }

    pIC = m_pData->m_aInterfaceContainer.getContainer( ::getCppuType( (const uno::Reference< document::XEventListener >*)0 ) );
    if ( pIC )
    {
        SAL_INFO("sfx2", "SfxEvent: " + aName);

        document::EventObject aEvent( (frame::XModel*)this, aName );

        pIC->forEach< document::XEventListener, NotifySingleListenerIgnoreRE< document::XEventListener, document::EventObject > >(
            NotifySingleListenerIgnoreRE< document::XEventListener, document::EventObject >(
                &document::XEventListener::notifyEvent,
                aEvent ) );
    }

}

uno::Reference < container::XIndexAccess > SAL_CALL SfxBaseModel::getViewData() throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( m_pData->m_pObjectShell.Is() && !m_pData->m_contViewData.is() )
    {
        SfxViewFrame *pActFrame = SfxViewFrame::Current();
        if ( !pActFrame || pActFrame->GetObjectShell() != m_pData->m_pObjectShell )
            pActFrame = SfxViewFrame::GetFirst( m_pData->m_pObjectShell );

        if ( !pActFrame || !pActFrame->GetViewShell() )
            // currently no frame for this document at all or View is under construction
            return uno::Reference < container::XIndexAccess >();

        m_pData->m_contViewData = Reference < container::XIndexAccess >(
                ::comphelper::getProcessServiceFactory()->createInstance(
                DEFINE_CONST_UNICODE("com.sun.star.document.IndexedPropertyValues") ),
                uno::UNO_QUERY );

        if ( !m_pData->m_contViewData.is() )
        {
            // error: no container class available!
            return uno::Reference < container::XIndexAccess >();
        }

        uno::Reference < container::XIndexContainer > xCont( m_pData->m_contViewData, uno::UNO_QUERY );
        sal_Int32 nCount = 0;
        uno::Sequence < beans::PropertyValue > aSeq;
        ::com::sun::star::uno::Any aAny;
        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst( m_pData->m_pObjectShell ); pFrame;
                pFrame = SfxViewFrame::GetNext( *pFrame, m_pData->m_pObjectShell ) )
        {
            sal_Bool bIsActive = ( pFrame == pActFrame );
            pFrame->GetViewShell()->WriteUserDataSequence( aSeq );
            aAny <<= aSeq;
            xCont->insertByIndex( bIsActive ? 0 : nCount, aAny );
            nCount++;
        }
    }

    return m_pData->m_contViewData;
}

void SAL_CALL SfxBaseModel::setViewData( const uno::Reference < container::XIndexAccess >& aData ) throw(::com::sun::star::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    m_pData->m_contViewData = aData;
}

/** calls all XEventListeners */
void SfxBaseModel::notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) const
{
    // object already disposed?
    if ( impl_isDisposed() )
        return;

    ::cppu::OInterfaceContainerHelper* pIC = m_pData->m_aInterfaceContainer.getContainer(
                                        ::getCppuType((const uno::Reference< XDOCEVENTLISTENER >*)0) );
    if( pIC )

    {
        ::cppu::OInterfaceIteratorHelper aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            try
            {
                ((XDOCEVENTLISTENER *)aIt.next())->notifyEvent( aEvent );
            }
            catch( uno::RuntimeException& )
            {
                aIt.remove();
            }
        }
    }
}

/** returns true if someone added a XEventListener to this XEventBroadcaster */
sal_Bool SfxBaseModel::hasEventListeners() const
{
    return !impl_isDisposed() && (NULL != m_pData->m_aInterfaceContainer.getContainer( ::getCppuType((const uno::Reference< XDOCEVENTLISTENER >*)0) ) );
}

void SAL_CALL SfxBaseModel::addPrintJobListener( const uno::Reference< view::XPrintJobListener >& xListener ) throw (uno::RuntimeException)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    if ( impl_getPrintHelper() )
    {
        uno::Reference < view::XPrintJobBroadcaster > xPJB( m_pData->m_xPrintable, uno::UNO_QUERY );
        if ( xPJB.is() )
            xPJB->addPrintJobListener( xListener );
    }
}

void SAL_CALL SfxBaseModel::removePrintJobListener( const uno::Reference< view::XPrintJobListener >& xListener ) throw (uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( impl_getPrintHelper() )
    {
        uno::Reference < view::XPrintJobBroadcaster > xPJB( m_pData->m_xPrintable, uno::UNO_QUERY );
        if ( xPJB.is() )
            xPJB->removePrintJobListener( xListener );
    }
}

// simple declaration of class SvObject is enough
// the corresponding <so3/iface.hxx> cannon be included because it provides
// declaration of class SvBorder that conflicts with ../../inc/viewfrm.hxx
class SvObject;
sal_Int64 SAL_CALL SfxBaseModel::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException)
{
    SvGlobalName aName( aIdentifier );
    if ((aName == SvGlobalName( SO3_GLOBAL_CLASSID )) ||
        (aName == SvGlobalName( SFX_GLOBAL_CLASSID )))
    {
        SolarMutexGuard aGuard;
        SfxObjectShell *const pObjectShell(GetObjectShell());
        if (pObjectShell)
        {
            // SO3_GLOBAL_CLASSID is apparently used by binfilter :(
            if ( aName == SvGlobalName( SO3_GLOBAL_CLASSID ) )
                 return (sal_Int64)(sal_IntPtr)(SvObject*) pObjectShell;
            else if ( aName == SvGlobalName( SFX_GLOBAL_CLASSID ) )
                 return (sal_Int64)(sal_IntPtr)(SfxObjectShell*) pObjectShell;
        }
    }

    return 0;
}

//____________________________________________________________________________________________________
//  XDocumentSubStorageSupplier
//____________________________________________________________________________________________________

void SfxBaseModel::ListenForStorage_Impl( const uno::Reference< embed::XStorage >& xStorage )
{
    uno::Reference< util::XModifiable > xModifiable( xStorage, uno::UNO_QUERY );
    if ( xModifiable.is() )
    {
        if ( !m_pData->m_pStorageModifyListen.is() )
        {
            m_pData->m_pStorageModifyListen = new ::sfx2::DocumentStorageModifyListener( *m_pData, Application::GetSolarMutex() );
        }

        // no need to deregister the listening for old storage since it should be disposed automatically
        xModifiable->addModifyListener( m_pData->m_pStorageModifyListen.get() );
    }
}

uno::Reference< XSTORAGE > SAL_CALL SfxBaseModel::getDocumentSubStorage( const ::rtl::OUString& aStorageName, sal_Int32 nMode )
    throw ( uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    uno::Reference< XSTORAGE > xResult;
    if ( m_pData->m_pObjectShell.Is() )
    {
        uno::Reference< embed::XStorage > xStorage = m_pData->m_pObjectShell->GetStorage();
        if ( xStorage.is() )
        {
            try
            {
                xResult = xStorage->openStorageElement( aStorageName, nMode );
            }
            catch ( uno::Exception& )
            {
            }
        }
    }

    return xResult;
}

Sequence< ::rtl::OUString > SAL_CALL SfxBaseModel::getDocumentSubStoragesNames()
    throw ( io::IOException,
            RuntimeException )
{
    SfxModelGuard aGuard( *this );

    Sequence< ::rtl::OUString > aResult;
    sal_Bool bSuccess = sal_False;
    if ( m_pData->m_pObjectShell.Is() )
    {
        uno::Reference < embed::XStorage > xStorage = m_pData->m_pObjectShell->GetStorage();
        uno::Reference < container::XNameAccess > xAccess( xStorage, uno::UNO_QUERY );
        if ( xAccess.is() )
        {
            Sequence< ::rtl::OUString > aTemp = xAccess->getElementNames();
            sal_Int32 nResultSize = 0;
            for ( sal_Int32 n = 0; n < aTemp.getLength(); n++ )
            {
                if ( xStorage->isStorageElement( aTemp[n] ) )
                {
                    aResult.realloc( ++nResultSize );
                    aResult[ nResultSize - 1 ] = aTemp[n];
                }
            }

            bSuccess = sal_True;
        }
    }

    if ( !bSuccess )
        throw io::IOException();

       return aResult;
}

//____________________________________________________________________________________________________
//  XScriptProviderSupplier
//____________________________________________________________________________________________________


uno::Reference< script::provider::XScriptProvider > SAL_CALL SfxBaseModel::getScriptProvider()
    throw ( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    uno::Reference< script::provider::XScriptProvider > xScriptProvider;

    ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
    Reference< script::provider::XScriptProviderFactory > xScriptProviderFactory(
        aContext.getSingleton( "com.sun.star.script.provider.theMasterScriptProviderFactory" ), uno::UNO_QUERY_THROW );

    try
    {
        Reference< XScriptInvocationContext > xScriptContext( this );
        xScriptProvider.set( xScriptProviderFactory->createScriptProvider( makeAny( xScriptContext ) ), uno::UNO_SET_THROW );
    }
    catch( const uno::RuntimeException& )
    {
        throw;
    }
    catch( const lang::IllegalArgumentException& )
    {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString(),
            *this,
            ::cppu::getCaughtException()
        );
    }

    return xScriptProvider;
}

//____________________________________________________________________________________________________
//  XUIConfigurationManagerSupplier
//____________________________________________________________________________________________________

rtl::OUString SfxBaseModel::getRuntimeUID() const
{
    OSL_ENSURE( !m_pData->m_sRuntimeUID.isEmpty(),
                "SfxBaseModel::getRuntimeUID - ID is empty!" );
    return m_pData->m_sRuntimeUID;
}

sal_Bool SfxBaseModel::hasValidSignatures() const
{
    SolarMutexGuard aGuard;
    if ( m_pData->m_pObjectShell.Is() )
        return ( m_pData->m_pObjectShell->ImplGetSignatureState( sal_False ) == SIGNATURESTATE_SIGNATURES_OK );
    return sal_False;
}

static void GetCommandFromSequence( rtl::OUString& rCommand, sal_Int32& nIndex, const uno::Sequence< beans::PropertyValue >& rSeqPropValue )
{
    rtl::OUString aCommand;
    nIndex = -1;

    for ( sal_Int32 i = 0; i < rSeqPropValue.getLength(); i++ )
    {
        if ( rSeqPropValue[i].Name == "Command" )
        {
            rSeqPropValue[i].Value >>= rCommand;
            nIndex = i;
            return;
        }
    }
}

static void ConvertSlotsToCommands( SfxObjectShell* pDoc, uno::Reference< container::XIndexContainer >& rToolbarDefinition )
{
    if ( pDoc )
    {
        Any           aAny;
        SfxModule*    pModule( pDoc->GetFactory().GetModule() );
        rtl::OUString aSlotCmd( "slot:" );
        rtl::OUString aUnoCmd( ".uno:" );
        uno::Sequence< beans::PropertyValue > aSeqPropValue;

        for ( sal_Int32 i = 0; i < rToolbarDefinition->getCount(); i++ )
        {
            sal_Int32 nIndex( -1 );
            rtl::OUString aCommand;

            if ( rToolbarDefinition->getByIndex( i ) >>= aSeqPropValue )
            {
                GetCommandFromSequence( aCommand, nIndex, aSeqPropValue );
                if ( nIndex >= 0 && ( aCommand.indexOf( aSlotCmd ) == 0 ))
                {
                    rtl::OUString aSlot( aCommand.copy( 5 ));

                    // We have to replace the old "slot-Command" with our new ".uno:-Command"
                    const SfxSlot* pSlot = pModule->GetSlotPool()->GetSlot( sal_uInt16( aSlot.toInt32() ));
                    if ( pSlot )
                    {
                        rtl::OUStringBuffer aStrBuf( aUnoCmd );
                        aStrBuf.appendAscii( pSlot->GetUnoName() );

                        aCommand = aStrBuf.makeStringAndClear();
                        aSeqPropValue[nIndex].Value <<= aCommand;
                        rToolbarDefinition->replaceByIndex( i, Any( aSeqPropValue ));
                    }
                }
            }
        }
    }
}

uno::Reference< ui::XUIConfigurationManager > SAL_CALL SfxBaseModel::getUIConfigurationManager()
        throw ( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_xUIConfigurationManager.is() )
    {
        uno::Reference< ui::XUIConfigurationManager > xNewUIConfMan(
            ::comphelper::getProcessServiceFactory()->createInstance(
                ::rtl::OUString("com.sun.star.ui.UIConfigurationManager")),
                uno::UNO_QUERY );

        Reference< ui::XUIConfigurationStorage > xUIConfigStorage( xNewUIConfMan, uno::UNO_QUERY );
        if ( xUIConfigStorage.is() )
        {
            uno::Reference< XSTORAGE > xConfigStorage;

            rtl::OUString aUIConfigFolderName( "Configurations2" );
            // First try to open with READWRITE and then READ
            xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, embed::ElementModes::READWRITE );
            if ( xConfigStorage.is() )
            {
                rtl::OUString aMediaTypeProp( "MediaType" );
                rtl::OUString aUIConfigMediaType(
                        "application/vnd.sun.xml.ui.configuration"  );
                rtl::OUString aMediaType;
                uno::Reference< beans::XPropertySet > xPropSet( xConfigStorage, uno::UNO_QUERY );
                Any a = xPropSet->getPropertyValue( aMediaTypeProp );
                if ( !( a >>= aMediaType ) ||  aMediaType.isEmpty())
                {
                    a <<= aUIConfigMediaType;
                    xPropSet->setPropertyValue( aMediaTypeProp, a );
                }
            }
            else
                xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, embed::ElementModes::READ );

            // initialize ui configuration manager with document substorage
            xUIConfigStorage->setStorage( xConfigStorage );

            // embedded objects did not support local configuration data until OOo 3.0, so there's nothing to
            // migrate
            if ( m_pData->m_pObjectShell->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
            {
                // Import old UI configuration from OOo 1.x
                uno::Reference< XSTORAGE > xOOo1ConfigStorage;
                rtl::OUString         aOOo1UIConfigFolderName( "Configurations" );

                // Try to open with READ
                xOOo1ConfigStorage = getDocumentSubStorage( aOOo1UIConfigFolderName, embed::ElementModes::READ );
                if ( xOOo1ConfigStorage.is() )
                {
                    uno::Reference< lang::XMultiServiceFactory > xServiceMgr( ::comphelper::getProcessServiceFactory() );
                    uno::Sequence< uno::Reference< container::XIndexContainer > > rToolbars;

                    sal_Bool bImported = framework::UIConfigurationImporterOOo1x::ImportCustomToolbars(
                                            xNewUIConfMan, rToolbars, xServiceMgr, xOOo1ConfigStorage );
                    if ( bImported )
                    {
                        SfxObjectShell* pObjShell = SfxBaseModel::GetObjectShell();

                        rtl::OUString aNum( "private:resource/toolbar/custom_OOo1x_" );
                        rtl::OUString aTitle( "Toolbar " );
                        for ( sal_Int32 i = 0; i < rToolbars.getLength(); i++ )
                        {
                            rtl::OUString aCustomTbxName = aNum + rtl::OUString::valueOf( i + 1 );
                            rtl::OUString aCustomTbxTitle = aTitle + rtl::OUString::valueOf( i + 1 );

                            uno::Reference< container::XIndexContainer > xToolbar = rToolbars[i];
                            ConvertSlotsToCommands( pObjShell, xToolbar );
                            if ( !xNewUIConfMan->hasSettings( aCustomTbxName ))
                            {
                                // Set UIName for the toolbar with container property
                                uno::Reference< beans::XPropertySet > xPropSet( xToolbar, UNO_QUERY );
                                if ( xPropSet.is() )
                                {
                                    try
                                    {
                                        rtl::OUString aPropName( "UIName" );
                                        Any           aAny( aCustomTbxTitle );
                                        xPropSet->setPropertyValue( aPropName, aAny );
                                    }
                                    catch ( beans::UnknownPropertyException& )
                                    {
                                    }
                                }

                                uno::Reference< container::XIndexAccess > xToolbarData( xToolbar, uno::UNO_QUERY );
                                xNewUIConfMan->insertSettings( aCustomTbxName, xToolbarData );
                                uno::Reference< ui::XUIConfigurationPersistence > xPersist( xNewUIConfMan, uno::UNO_QUERY );
                                xPersist->store();
                            }
                        }
                    }
                }
            }
        }

        m_pData->m_xUIConfigurationManager = xNewUIConfMan;
    }

    return m_pData->m_xUIConfigurationManager;
}

//____________________________________________________________________________________________________
//  XVisualObject
//____________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::setVisualAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.Is() )
        throw uno::Exception(); // TODO: error handling

    SfxViewFrame* pViewFrm = SfxViewFrame::GetFirst( m_pData->m_pObjectShell, sal_False );
    if ( pViewFrm && m_pData->m_pObjectShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED && !pViewFrm->GetFrame().IsInPlace() )
    {
        Window* pWindow = VCLUnoHelper::GetWindow( pViewFrm->GetFrame().GetFrameInterface()->getContainerWindow() );
        Size aWinSize = pWindow->GetSizePixel();
        awt::Size aCurrent = getVisualAreaSize( nAspect );
        Size aDiff( aSize.Width-aCurrent.Width, aSize.Height-aCurrent.Height );
        aDiff = pViewFrm->GetViewShell()->GetWindow()->LogicToPixel( aDiff );
        aWinSize.Width() += aDiff.Width();
        aWinSize.Height() += aDiff.Height();
        pWindow->SetSizePixel( aWinSize );
    }
    else
    {
        Rectangle aTmpRect = m_pData->m_pObjectShell->GetVisArea( ASPECT_CONTENT );
        aTmpRect.SetSize( Size( aSize.Width, aSize.Height ) );
        m_pData->m_pObjectShell->SetVisArea( aTmpRect );
    }
}

awt::Size SAL_CALL SfxBaseModel::getVisualAreaSize( sal_Int64 /*nAspect*/ )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.Is() )
        throw uno::Exception(); // TODO: error handling

    Rectangle aTmpRect = m_pData->m_pObjectShell->GetVisArea( ASPECT_CONTENT );

    return awt::Size( aTmpRect.GetWidth(), aTmpRect.GetHeight() );
}


sal_Int32 SAL_CALL SfxBaseModel::getMapUnit( sal_Int64 /*nAspect*/ )
        throw ( uno::Exception,
                uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.Is() )
        throw uno::Exception(); // TODO: error handling

    return VCLUnoHelper::VCL2UnoEmbedMapUnit( m_pData->m_pObjectShell->GetMapUnit() );
}

embed::VisualRepresentation SAL_CALL SfxBaseModel::getPreferredVisualRepresentation( ::sal_Int64 /*nAspect*/ )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    datatransfer::DataFlavor aDataFlavor(
            ::rtl::OUString("application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"" ),
            ::rtl::OUString("GDIMetaFile"),
            ::getCppuType( (const uno::Sequence< sal_Int8 >*) NULL ) );

    embed::VisualRepresentation aVisualRepresentation;
    aVisualRepresentation.Data = getTransferData( aDataFlavor );
    aVisualRepresentation.Flavor = aDataFlavor;

    return aVisualRepresentation;
}

//____________________________________________________________________________________________________
//  XStorageBasedDocument
//____________________________________________________________________________________________________

void SAL_CALL SfxBaseModel::loadFromStorage( const uno::Reference< XSTORAGE >& xStorage,
                                             const uno::Sequence< beans::PropertyValue >& aMediaDescriptor )
    throw ( lang::IllegalArgumentException,
            frame::DoubleInitializationException,
            IOEXCEPTION,
            EXCEPTION,
            uno::RuntimeException )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    if ( IsInitialized() )
        throw ::com::sun::star::frame::DoubleInitializationException( ::rtl::OUString(), *this );

    // after i36090 is fixed the pool from object shell can be used
    // SfxAllItemSet aSet( m_pData->m_pObjectShell->GetPool() );
    SfxAllItemSet aSet( SFX_APP()->GetPool() );

    // the BaseURL is part of the ItemSet
    SfxMedium* pMedium = new SfxMedium( xStorage, String() );
    TransformParameters( SID_OPENDOC, aMediaDescriptor, aSet );
    pMedium->GetItemSet()->Put( aSet );

    // allow to use an interactionhandler (if there is one)
    pMedium->UseInteractionHandler( sal_True );

    SFX_ITEMSET_ARG( &aSet, pTemplateItem, SfxBoolItem, SID_TEMPLATE, sal_False);
    sal_Bool bTemplate = pTemplateItem && pTemplateItem->GetValue();
    m_pData->m_pObjectShell->SetActivateEvent_Impl( bTemplate ? SFX_EVENT_CREATEDOC : SFX_EVENT_OPENDOC );
    m_pData->m_pObjectShell->Get_Impl()->bOwnsStorage = sal_False;

    // load document
    if ( !m_pData->m_pObjectShell->DoLoad(pMedium) )
    {
        sal_uInt32 nError = m_pData->m_pObjectShell->GetErrorCode();
        throw task::ErrorCodeIOException( ::rtl::OUString(),
                                            uno::Reference< uno::XInterface >(),
                                            nError ? nError : ERRCODE_IO_CANTREAD );
    }
}

void SAL_CALL SfxBaseModel::storeToStorage( const uno::Reference< XSTORAGE >& xStorage,
                                const uno::Sequence< beans::PropertyValue >& aMediaDescriptor )
    throw ( lang::IllegalArgumentException,
            IOEXCEPTION,
            EXCEPTION,
            uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    uno::Reference< XSTORAGE > xResult;
    if ( !m_pData->m_pObjectShell.Is() )
        throw IOEXCEPTION(); // TODO:

    SfxAllItemSet aSet( m_pData->m_pObjectShell->GetPool() );
    TransformParameters( SID_SAVEASDOC, aMediaDescriptor, aSet );

    // TODO/LATER: may be a special URL "private:storage" should be used
    SFX_ITEMSET_ARG( &aSet, pItem, SfxStringItem, SID_FILTER_NAME, sal_False );
    sal_Int32 nVersion = SOFFICE_FILEFORMAT_CURRENT;
    if( pItem )
    {
        String aFilterName = pItem->GetValue();
        const SfxFilter* pFilter = SFX_APP()->GetFilterMatcher().GetFilter4FilterName( aFilterName );
        if ( pFilter && pFilter->UsesStorage() )
            nVersion = pFilter->GetVersion();
    }

    sal_Bool bSuccess = sal_False;
    if ( xStorage == m_pData->m_pObjectShell->GetStorage() )
    {
        // storing to the own storage
        bSuccess = m_pData->m_pObjectShell->DoSave();
    }
    else
    {
        // TODO/LATER: if the provided storage has some data inside the storing might fail, probably the storage must be truncated
        // TODO/LATER: is it possible to have a template here?
        m_pData->m_pObjectShell->SetupStorage( xStorage, nVersion, sal_False );

        // BaseURL is part of the ItemSet
        SfxMedium aMedium( xStorage, String(), &aSet );
        aMedium.CanDisposeStorage_Impl( sal_False );
        if ( aMedium.GetFilter() )
        {
            // storing without a valid filter will often crash
            bSuccess = m_pData->m_pObjectShell->DoSaveObjectAs( aMedium, sal_True );
            m_pData->m_pObjectShell->DoSaveCompleted( NULL );
        }
    }

    sal_uInt32 nError = m_pData->m_pObjectShell->GetErrorCode();
    m_pData->m_pObjectShell->ResetError();

    // the warnings are currently not transported
    if ( !bSuccess )
    {
        throw task::ErrorCodeIOException( ::rtl::OUString(),
                                            uno::Reference< uno::XInterface >(),
                                            nError ? nError : ERRCODE_IO_GENERAL );
    }
}

void SAL_CALL SfxBaseModel::switchToStorage( const uno::Reference< XSTORAGE >& xStorage )
        throw ( lang::IllegalArgumentException,
                IOEXCEPTION,
                EXCEPTION,
                uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    uno::Reference< XSTORAGE > xResult;
    if ( !m_pData->m_pObjectShell.Is() )
        throw IOEXCEPTION(); // TODO:

    // the persistence should be switched only if the storage is different
    if ( xStorage != m_pData->m_pObjectShell->GetStorage() )
    {
        if ( !m_pData->m_pObjectShell->SwitchPersistance( xStorage ) )
        {
            sal_uInt32 nError = m_pData->m_pObjectShell->GetErrorCode();
            throw task::ErrorCodeIOException( ::rtl::OUString(),
                                                uno::Reference< uno::XInterface >(),
                                                nError ? nError : ERRCODE_IO_GENERAL );
        }
        else
        {
            // UICfgMgr has a reference to the old storage, update it
            uno::Reference< ui::XUIConfigurationStorage > xUICfgMgrStorage( getUIConfigurationManager(), uno::UNO_QUERY );
            if ( xUICfgMgrStorage.is() )
                xUICfgMgrStorage->setStorage( xStorage );
        }
    }
    m_pData->m_pObjectShell->Get_Impl()->bOwnsStorage = sal_False;
}

uno::Reference< XSTORAGE > SAL_CALL SfxBaseModel::getDocumentStorage()
        throw ( IOEXCEPTION,
                EXCEPTION,
                uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    uno::Reference< XSTORAGE > xResult;
    if ( !m_pData->m_pObjectShell.Is() )
        throw IOEXCEPTION(); // TODO

    return m_pData->m_pObjectShell->GetStorage();
}

void SAL_CALL SfxBaseModel::addStorageChangeListener(
            const uno::Reference< document::XStorageChangeListener >& xListener )
        throw ( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    m_pData->m_aInterfaceContainer.addInterface(
                                    ::getCppuType((const uno::Reference< document::XStorageChangeListener >*)0), xListener );
}

void SAL_CALL SfxBaseModel::removeStorageChangeListener(
            const uno::Reference< document::XStorageChangeListener >& xListener )
        throw ( uno::RuntimeException )
{
    SfxModelGuard aGuard( *this );

    m_pData->m_aInterfaceContainer.removeInterface(
                                    ::getCppuType((const uno::Reference< document::XStorageChangeListener >*)0), xListener );
}

#include "printhelper.hxx"
bool SfxBaseModel::impl_getPrintHelper()
{
    if ( m_pData->m_xPrintable.is() )
        return true;
    m_pData->m_xPrintable = new SfxPrintHelper();
    uno::Reference < lang::XInitialization > xInit( m_pData->m_xPrintable, uno::UNO_QUERY );
    uno::Sequence < uno::Any > aValues(1);
    aValues[0] <<= uno::Reference < frame::XModel > (static_cast< frame::XModel* >(this), uno::UNO_QUERY );
    xInit->initialize( aValues );
    uno::Reference < view::XPrintJobBroadcaster > xBrd( m_pData->m_xPrintable, uno::UNO_QUERY );
    xBrd->addPrintJobListener( new SfxPrintHelperListener_Impl( m_pData ) );
    return true;
}

//=============================================================================
// css.frame.XModule
 void SAL_CALL SfxBaseModel::setIdentifier(const ::rtl::OUString& Identifier)
    throw (css::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );
    m_pData->m_sModuleIdentifier = Identifier;
}

//=============================================================================
// css.frame.XModule
 ::rtl::OUString SAL_CALL SfxBaseModel::getIdentifier()
    throw (css::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );
    if (!m_pData->m_sModuleIdentifier.isEmpty())
        return m_pData->m_sModuleIdentifier;
    if (m_pData->m_pObjectShell)
        return m_pData->m_pObjectShell->GetFactory().GetDocumentServiceName();
    return ::rtl::OUString();
}

//=============================================================================
css::uno::Reference< css::frame::XTitle > SfxBaseModel::impl_getTitleHelper ()
{
    SfxModelGuard aGuard( *this );

    if ( ! m_pData->m_xTitleHelper.is ())
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR   = ::comphelper::getProcessServiceFactory ();
        const ::rtl::OUString SERVICENAME_DESKTOP("com.sun.star.frame.Desktop");
        css::uno::Reference< css::frame::XUntitledNumbers >    xDesktop(xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::frame::XModel >              xThis   (static_cast< css::frame::XModel* >(this), css::uno::UNO_QUERY_THROW);

        ::framework::TitleHelper* pHelper = new ::framework::TitleHelper(xSMGR);
        m_pData->m_xTitleHelper = css::uno::Reference< css::frame::XTitle >(static_cast< ::cppu::OWeakObject* >(pHelper), css::uno::UNO_QUERY_THROW);
        pHelper->setOwner                   (xThis   );
        pHelper->connectWithUntitledNumbers (xDesktop);
    }

    return m_pData->m_xTitleHelper;
}

//=============================================================================
css::uno::Reference< css::frame::XUntitledNumbers > SfxBaseModel::impl_getUntitledHelper ()
{
    SfxModelGuard aGuard( *this );

    if ( ! m_pData->m_xNumberedControllers.is ())
    {
        css::uno::Reference< css::frame::XModel > xThis   (static_cast< css::frame::XModel* >(this), css::uno::UNO_QUERY_THROW);
        ::comphelper::NumberedCollection*         pHelper = new ::comphelper::NumberedCollection();

        m_pData->m_xNumberedControllers = css::uno::Reference< css::frame::XUntitledNumbers >(static_cast< ::cppu::OWeakObject* >(pHelper), css::uno::UNO_QUERY_THROW);

        pHelper->setOwner          (xThis);
        pHelper->setUntitledPrefix (::rtl::OUString(" : "));
    }

    return m_pData->m_xNumberedControllers;
}

//=============================================================================
// css.frame.XTitle
::rtl::OUString SAL_CALL SfxBaseModel::getTitle()
    throw (css::uno::RuntimeException)
{
    // SYNCHRONIZED ->
    SfxModelGuard aGuard( *this );

    ::rtl::OUString aResult = impl_getTitleHelper()->getTitle ();
    if ( !m_pData->m_bExternalTitle && m_pData->m_pObjectShell )
    {
        SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
        if ( pMedium )
        {
            try {
                ::ucbhelper::Content aContent( pMedium->GetName(),
                    uno::Reference<ucb::XCommandEnvironment>(),
                    comphelper::getProcessComponentContext() );
                const uno::Reference < beans::XPropertySetInfo > xProps
                     = aContent.getProperties();
                if ( xProps.is() )
                {
                    ::rtl::OUString aServerTitle( "TitleOnServer" );
                    if ( xProps->hasPropertyByName( aServerTitle ) )
                    {
                        uno::Any aAny = aContent.getPropertyValue( aServerTitle );
                        aAny >>= aResult;
                    }
                }
            }
            catch (const ucb::ContentCreationException &)
            {
            }
            catch (const ucb::CommandAbortedException &)
            {
            }
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pRepairedDocItem, SfxBoolItem, SID_REPAIRPACKAGE, sal_False );
            if ( pRepairedDocItem && pRepairedDocItem->GetValue() )
                aResult += SfxResId(STR_REPAIREDDOCUMENT).toString();
        }

        if ( m_pData->m_pObjectShell->IsReadOnlyUI() || (pMedium && pMedium->IsReadOnly()) )
            aResult += SfxResId(STR_READONLY).toString();
        else if ( m_pData->m_pObjectShell->IsDocShared() )
            aResult += SfxResId(STR_SHARED).toString();

        if ( m_pData->m_pObjectShell->GetDocumentSignatureState() == SIGNATURESTATE_SIGNATURES_OK )
            aResult += SfxResId(RID_XMLSEC_DOCUMENTSIGNED).toString();
    }

    return aResult;
}

//=============================================================================
// css.frame.XTitle
void SAL_CALL SfxBaseModel::setTitle( const ::rtl::OUString& sTitle )
    throw (css::uno::RuntimeException)
{
    // SYNCHRONIZED ->
    SfxModelGuard aGuard( *this );

    impl_getTitleHelper()->setTitle (sTitle);
    m_pData->m_bExternalTitle = sal_True;
}

//=============================================================================
// css.frame.XTitleChangeBroadcaster
void SAL_CALL SfxBaseModel::addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener )
    throw (css::uno::RuntimeException)
{
    // SYNCHRONIZED ->
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper(), css::uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (xListener);
}

//=============================================================================
// css.frame.XTitleChangeBroadcaster
void SAL_CALL SfxBaseModel::removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener )
    throw (css::uno::RuntimeException)
{
    // SYNCHRONIZED ->
    SfxModelGuard aGuard( *this );

    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper(), css::uno::UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->removeTitleChangeListener (xListener);
}

//=============================================================================
// css.frame.XUntitledNumbers
::sal_Int32 SAL_CALL SfxBaseModel::leaseNumber( const css::uno::Reference< css::uno::XInterface >& xComponent )
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    SfxModelGuard aGuard( *this );

    return impl_getUntitledHelper ()->leaseNumber (xComponent);
}

//=============================================================================
// css.frame.XUntitledNumbers
void SAL_CALL SfxBaseModel::releaseNumber( ::sal_Int32 nNumber )
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    SfxModelGuard aGuard( *this );
    impl_getUntitledHelper ()->releaseNumber (nNumber);
}

//=============================================================================
// css.frame.XUntitledNumbers
void SAL_CALL SfxBaseModel::releaseNumberForComponent( const css::uno::Reference< css::uno::XInterface >& xComponent )
    throw (css::lang::IllegalArgumentException,
           css::uno::RuntimeException         )
{
    SfxModelGuard aGuard( *this );
    impl_getUntitledHelper ()->releaseNumberForComponent (xComponent);
}

//=============================================================================
// css.frame.XUntitledNumbers
::rtl::OUString SAL_CALL SfxBaseModel::getUntitledPrefix()
    throw (css::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );
    return impl_getUntitledHelper ()->getUntitledPrefix ();
}

//=============================================================================
// css::frame::XModel2
css::uno::Reference< css::container::XEnumeration > SAL_CALL SfxBaseModel::getControllers()
    throw (css::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    sal_Int32 c = m_pData->m_seqControllers.getLength();
    sal_Int32 i = 0;
    css::uno::Sequence< css::uno::Any > lEnum(c);
    for (i=0; i<c; ++i)
        lEnum[i] <<= m_pData->m_seqControllers[i];

    ::comphelper::OAnyEnumeration*                      pEnum = new ::comphelper::OAnyEnumeration(lEnum);
    css::uno::Reference< css::container::XEnumeration > xEnum(static_cast< css::container::XEnumeration* >(pEnum), css::uno::UNO_QUERY_THROW);
    return xEnum;
}

//=============================================================================
// css::frame::XModel2
css::uno::Sequence< ::rtl::OUString > SAL_CALL SfxBaseModel::getAvailableViewControllerNames()
    throw (css::uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    const SfxObjectFactory& rDocumentFactory = GetObjectShell()->GetFactory();
    const sal_Int16 nViewFactoryCount = rDocumentFactory.GetViewFactoryCount();

    Sequence< ::rtl::OUString > aViewNames( nViewFactoryCount );
    for ( sal_Int16 nViewNo = 0; nViewNo < nViewFactoryCount; ++nViewNo )
        aViewNames[nViewNo] = rDocumentFactory.GetViewFactory( nViewNo ).GetAPIViewName();
    return aViewNames;
}

//=============================================================================
// css::frame::XModel2
css::uno::Reference< css::frame::XController2 > SAL_CALL SfxBaseModel::createDefaultViewController( const css::uno::Reference< css::frame::XFrame >& i_rFrame )
    throw (css::uno::RuntimeException         ,
           css::lang::IllegalArgumentException,
           css::uno::Exception                )
{
    SfxModelGuard aGuard( *this );

    const SfxObjectFactory& rDocumentFactory = GetObjectShell()->GetFactory();
    const ::rtl::OUString sDefaultViewName = rDocumentFactory.GetViewFactory( 0 ).GetAPIViewName();

    aGuard.clear();

    return createViewController( sDefaultViewName, Sequence< PropertyValue >(), i_rFrame );
}

//=============================================================================
namespace sfx { namespace intern {

    /** a class which, in its dtor, cleans up variuos objects (well, at the moment only the frame) collected during
        the creation of a document view, unless the creation was successful.
    */
    class SAL_DLLPRIVATE ViewCreationGuard
    {
    public:
        ViewCreationGuard()
            :m_bSuccess( false )
        {
        }

        ~ViewCreationGuard()
        {
            if ( !m_bSuccess )
                impl_closeAll();
        }

        void takeFrameOwnership( SfxFrame* i_pFrame )
        {
            OSL_PRECOND( !m_aWeakFrame, "ViewCreationGuard::takeFrameOwnership: already have a frame!" );
            OSL_PRECOND( i_pFrame != NULL, "ViewCreationGuard::takeFrameOwnership: invalid frame!" );
            m_aWeakFrame = i_pFrame;
        }

        void    releaseAll()
        {
            m_bSuccess = true;
        }

    private:
        void impl_closeAll()
        {
            if ( m_aWeakFrame && !m_aWeakFrame->GetCurrentDocument() )
            {
                m_aWeakFrame->SetFrameInterface_Impl( NULL );
                m_aWeakFrame->DoClose();
            }
        }

    private:
        bool            m_bSuccess;
        SfxFrameWeak    m_aWeakFrame;
    };
} }

//=============================================================================
SfxViewFrame* SfxBaseModel::FindOrCreateViewFrame_Impl( const Reference< XFrame >& i_rFrame, ::sfx::intern::ViewCreationGuard& i_rGuard ) const
{
    SfxViewFrame* pViewFrame = NULL;
    for (   pViewFrame = SfxViewFrame::GetFirst( GetObjectShell(), sal_False );
            pViewFrame;
            pViewFrame= SfxViewFrame::GetNext( *pViewFrame, GetObjectShell(), sal_False )
        )
    {
        if ( pViewFrame->GetFrame().GetFrameInterface() == i_rFrame )
            break;
    }
    if ( !pViewFrame )
    {
    #if OSL_DEBUG_LEVEL > 0
        for (   SfxFrame* pCheckFrame = SfxFrame::GetFirst();
                pCheckFrame;
                pCheckFrame = SfxFrame::GetNext( *pCheckFrame )
             )
        {
            if ( pCheckFrame->GetFrameInterface() == i_rFrame )
            {
                if  (   ( pCheckFrame->GetCurrentViewFrame() != NULL )
                    ||  ( pCheckFrame->GetCurrentDocument() != NULL )
                    )
                    // Note that it is perfectly letgitimate that during loading into an XFrame which already contains
                    // a document, there exist two SfxFrame instances bound to this XFrame - the old one, which will be
                    // destroyed later, and the new one, which we're going to create
                    continue;

                OSL_FAIL( "SfxBaseModel::FindOrCreateViewFrame_Impl: there already is an SfxFrame for the given XFrame, but no view in it!" );
                    // nowadays, we're the only instance allowed to create an SfxFrame for an XFrame, so this case here should not happen
                break;
            }
        }
    #endif

        SfxFrame* pTargetFrame = SfxFrame::Create( i_rFrame );
        ENSURE_OR_THROW( pTargetFrame, "could not create an SfxFrame" );
        i_rGuard.takeFrameOwnership( pTargetFrame );

        // prepare it
        pTargetFrame->PrepareForDoc_Impl( *GetObjectShell() );

        // create view frame
        pViewFrame = new SfxViewFrame( *pTargetFrame, GetObjectShell() );
    }
    return pViewFrame;
}

//=============================================================================
// css::frame::XModel2
css::uno::Reference< css::frame::XController2 > SAL_CALL SfxBaseModel::createViewController(
        const ::rtl::OUString& i_rViewName, const Sequence< PropertyValue >& i_rArguments, const Reference< XFrame >& i_rFrame )
    throw (css::uno::RuntimeException         ,
           css::lang::IllegalArgumentException,
           css::uno::Exception                )
{
    SfxModelGuard aGuard( *this );

    if ( !i_rFrame.is() )
        throw css::lang::IllegalArgumentException( ::rtl::OUString(), *this, 3 );

    // find the proper SFX view factory
    SfxViewFactory* pViewFactory = GetObjectShell()->GetFactory().GetViewFactoryByViewName( i_rViewName );
    if ( !pViewFactory )
        throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );

    // determine previous shell (used in some special cases)
    Reference< XController > xPreviousController( i_rFrame->getController() );
    const Reference< XModel > xMe( this );
    if  (   ( xPreviousController.is() )
        &&  ( xMe != xPreviousController->getModel() )
        )
    {
        xPreviousController.clear();
    }
    SfxViewShell* pOldViewShell = SfxViewShell::Get( xPreviousController );
    OSL_ENSURE( !xPreviousController.is() || ( pOldViewShell != NULL ),
        "SfxBaseModel::createViewController: invalid old controller!" );

    // a guard which will clean up in case of failure
    ::sfx::intern::ViewCreationGuard aViewCreationGuard;

    // determine the ViewFrame belonging to the given XFrame
    SfxViewFrame* pViewFrame = FindOrCreateViewFrame_Impl( i_rFrame, aViewCreationGuard );
    OSL_POSTCOND( pViewFrame, "SfxBaseModel::createViewController: no frame?" );

    // delegate to SFX' view factory
    pViewFrame->GetBindings().ENTERREGISTRATIONS();
    SfxViewShell* pViewShell = pViewFactory->CreateInstance( pViewFrame, pOldViewShell );
    pViewFrame->GetBindings().LEAVEREGISTRATIONS();
    ENSURE_OR_THROW( pViewShell, "invalid view shell provided by factory" );

    // by setting the ViewShell it is prevented that disposing the Controller will destroy this ViewFrame also
    pViewFrame->GetDispatcher()->SetDisableFlags( 0 );
    pViewFrame->SetViewShell_Impl( pViewShell );

    // remember ViewID
    pViewFrame->SetCurViewId_Impl( pViewFactory->GetOrdinal() );

    // ensure a default controller, if the view shell did not provide an own implementation
    if ( !pViewShell->GetController().is() )
        pViewShell->SetController( new SfxBaseController( pViewShell ) );

    // pass the creation arguments to the controller
    SfxBaseController* pBaseController = pViewShell->GetBaseController_Impl();
    ENSURE_OR_THROW( pBaseController, "invalid controller implementation!" );
    pBaseController->SetCreationArguments_Impl( i_rArguments );

    // some initial view settings, coming from our most recent attachResource call
    ::comphelper::NamedValueCollection aDocumentLoadArgs( getArgs() );
    if ( aDocumentLoadArgs.getOrDefault( "ViewOnly", false ) )
        pViewFrame->GetFrame().SetMenuBarOn_Impl( sal_False );

    const sal_Int16 nPluginMode = aDocumentLoadArgs.getOrDefault( "PluginMode", sal_Int16( 0 ) );
    if ( nPluginMode == 1 )
    {
        pViewFrame->ForceOuterResize_Impl( sal_False );
        pViewFrame->GetBindings().HidePopups( sal_True );

        SfxFrame& rFrame = pViewFrame->GetFrame();
        // MBA: layoutmanager of inplace frame starts locked and invisible
        rFrame.GetWorkWindow_Impl()->MakeVisible_Impl( sal_False );
        rFrame.GetWorkWindow_Impl()->Lock_Impl( sal_True );

        rFrame.GetWindow().SetBorderStyle( WINDOW_BORDER_NOBORDER );
        pViewFrame->GetWindow().SetBorderStyle( WINDOW_BORDER_NOBORDER );
    }

    // tell the guard we were successful
    aViewCreationGuard.releaseAll();

    // outta gere
    return pBaseController;
}

//=============================================================================
// RDF DocumentMetadataAccess

// ::com::sun::star::rdf::XRepositorySupplier:
uno::Reference< rdf::XRepository > SAL_CALL
SfxBaseModel::getRDFRepository() throw (uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->getRDFRepository();
}

// ::com::sun::star::rdf::XNode:
::rtl::OUString SAL_CALL
SfxBaseModel::getStringValue() throw (uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->getStringValue();
}

// ::com::sun::star::rdf::XURI:
::rtl::OUString SAL_CALL
SfxBaseModel::getNamespace() throw (uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->getNamespace();
}

::rtl::OUString SAL_CALL
SfxBaseModel::getLocalName() throw (uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->getLocalName();
}

// ::com::sun::star::rdf::XDocumentMetadataAccess:
uno::Reference< rdf::XMetadatable > SAL_CALL
SfxBaseModel::getElementByMetadataReference(
    const ::com::sun::star::beans::StringPair & i_rReference)
throw (uno::RuntimeException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->getElementByMetadataReference(i_rReference);
}

uno::Reference< rdf::XMetadatable > SAL_CALL
SfxBaseModel::getElementByURI(const uno::Reference< rdf::XURI > & i_xURI)
throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->getElementByURI(i_xURI);
}

uno::Sequence< uno::Reference< rdf::XURI > > SAL_CALL
SfxBaseModel::getMetadataGraphsWithType(
    const uno::Reference<rdf::XURI> & i_xType)
throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->getMetadataGraphsWithType(i_xType);
}

uno::Reference<rdf::XURI> SAL_CALL
SfxBaseModel::addMetadataFile(const ::rtl::OUString & i_rFileName,
    const uno::Sequence < uno::Reference< rdf::XURI > > & i_rTypes)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    container::ElementExistException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->addMetadataFile(i_rFileName, i_rTypes);
}

uno::Reference<rdf::XURI> SAL_CALL
SfxBaseModel::importMetadataFile(::sal_Int16 i_Format,
    const uno::Reference< io::XInputStream > & i_xInStream,
    const ::rtl::OUString & i_rFileName,
    const uno::Reference< rdf::XURI > & i_xBaseURI,
    const uno::Sequence < uno::Reference< rdf::XURI > > & i_rTypes)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    datatransfer::UnsupportedFlavorException,
    container::ElementExistException, rdf::ParseException, io::IOException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->importMetadataFile(i_Format,
        i_xInStream, i_rFileName, i_xBaseURI, i_rTypes);
}

void SAL_CALL
SfxBaseModel::removeMetadataFile(
    const uno::Reference< rdf::XURI > & i_xGraphName)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    container::NoSuchElementException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->removeMetadataFile(i_xGraphName);
}

void SAL_CALL
SfxBaseModel::addContentOrStylesFile(const ::rtl::OUString & i_rFileName)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    container::ElementExistException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->addContentOrStylesFile(i_rFileName);
}

void SAL_CALL
SfxBaseModel::removeContentOrStylesFile(const ::rtl::OUString & i_rFileName)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    container::NoSuchElementException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->removeContentOrStylesFile(i_rFileName);
}

void SAL_CALL
SfxBaseModel::loadMetadataFromStorage(
    uno::Reference< embed::XStorage > const & i_xStorage,
    uno::Reference<rdf::XURI> const & i_xBaseURI,
    uno::Reference<task::XInteractionHandler> const & i_xHandler)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    lang::WrappedTargetException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(
        m_pData->CreateDMAUninitialized());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    try {
        xDMA->loadMetadataFromStorage(i_xStorage, i_xBaseURI, i_xHandler);
    } catch (lang::IllegalArgumentException &) {
        throw; // not initialized
    } catch (uno::Exception &) {
        // UGLY: if it's a RuntimeException, we can't be sure DMA is initialzed
        m_pData->m_xDocumentMetadata = xDMA;
        throw;
    }
    m_pData->m_xDocumentMetadata = xDMA;

}

void SAL_CALL
SfxBaseModel::storeMetadataToStorage(
    uno::Reference< embed::XStorage > const & i_xStorage)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    lang::WrappedTargetException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->storeMetadataToStorage(i_xStorage);
}

void SAL_CALL
SfxBaseModel::loadMetadataFromMedium(
    const uno::Sequence< beans::PropertyValue > & i_rMedium)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    lang::WrappedTargetException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(
        m_pData->CreateDMAUninitialized());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    try {
        xDMA->loadMetadataFromMedium(i_rMedium);
    } catch (lang::IllegalArgumentException &) {
        throw; // not initialized
    } catch (uno::Exception &) {
        // UGLY: if it's a RuntimeException, we can't be sure DMA is initialzed
        m_pData->m_xDocumentMetadata = xDMA;
        throw;
    }
    m_pData->m_xDocumentMetadata = xDMA;
}

void SAL_CALL
SfxBaseModel::storeMetadataToMedium(
    const uno::Sequence< beans::PropertyValue > & i_rMedium)
throw (uno::RuntimeException, lang::IllegalArgumentException,
    lang::WrappedTargetException)
{
    SfxModelGuard aGuard( *this );

    const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw uno::RuntimeException( ::rtl::OUString(
            "model has no document metadata"), *this );
    }

    return xDMA->storeMetadataToMedium(i_rMedium);
}

// =====================================================================================================================
// = SfxModelSubComponent
// =====================================================================================================================

SfxModelSubComponent::~SfxModelSubComponent()
{
}

void SfxModelSubComponent::disposing()
{
    // nothing to do here
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
