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

#include <sal/config.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>
#include <config_features.h>

#include <sfx2/sfxbasemodel.hxx>

#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/task/ErrorCodeRequest2.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XPrintJobListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/IllegalArgumentIOException.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/document/XStorageChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/script/provider/theMasterScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/ui/UIConfigurationManager.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/frame/XTransientDocumentsDocumentContentFactory.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/InvalidStateException.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <comphelper/enumhelper.hxx>
#include <comphelper/indexedpropertyvalues.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/string.hxx>

#include <cppuhelper/implbase.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/multicontainer2.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/grabbagitem.hxx>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/svborder.hxx>
#include <unotools/tempfile.hxx>
#include <osl/mutex.hxx>
#include <comphelper/errcode.hxx>
#include <vcl/filter/SvmWriter.hxx>
#include <vcl/salctype.hxx>
#include <vcl/gdimtf.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/servicehelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/transfer.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <sal/log.hxx>
#include <framework/configimporter.hxx>
#include <framework/titlehelper.hxx>
#include <comphelper/numberedcollection.hxx>
#include <unotools/ucbhelper.hxx>
#include <ucbhelper/content.hxx>

#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/viewfac.hxx>
#include <workwin.hxx>
#include <sfx2/signaturestate.hxx>
#include <sfx2/sfxuno.hxx>
#include <objshimp.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <basic/basmgr.hxx>
#include <sfx2/event.hxx>
#include <eventsupplier.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/strings.hrc>
#include <sfx2/app.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docstoragemodifylistener.hxx>
#include <sfx2/brokenpackageint.hxx>
#include "graphhelp.hxx"
#include <docundomanager.hxx>
#include <openurlhint.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/DocumentMetadataAccess.hxx>
#include "printhelper.hxx"
#include <sfx2/sfxresid.hxx>
#include <sfx2/filedlghelper.hxx>
#include <comphelper/profilezone.hxx>
#include <vcl/threadex.hxx>
#include <unotools/mediadescriptor.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>

//  namespaces


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::document::CmisProperty;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::frame::XController;
using ::com::sun::star::frame::XController2;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::io::IOException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::document::XDocumentRecovery;
using ::com::sun::star::document::XUndoManager;
using ::com::sun::star::document::XUndoAction;
using ::com::sun::star::frame::XModel;

namespace {

/** This Listener is used to get notified when the XDocumentProperties of the
    XModel change.
 */
class SfxDocInfoListener_Impl : public ::cppu::WeakImplHelper<
    util::XModifyListener >
{

public:
    SfxObjectShell& m_rShell;

    explicit SfxDocInfoListener_Impl( SfxObjectShell& i_rDoc )
        : m_rShell(i_rDoc)
    { };

    virtual void SAL_CALL disposing( const lang::EventObject& ) override;
    virtual void SAL_CALL modified( const lang::EventObject& ) override;
};

}

void SAL_CALL SfxDocInfoListener_Impl::modified( const lang::EventObject& )
{
    SolarMutexGuard aSolarGuard;

    // notify changes to the SfxObjectShell
    m_rShell.FlushDocInfo();
}

void SAL_CALL SfxDocInfoListener_Impl::disposing( const lang::EventObject& )
{
}


//  impl. declarations


struct IMPL_SfxBaseModel_DataContainer : public ::sfx2::IModifiableDocument
{
    // counter for SfxBaseModel instances created.
    static sal_Int64                                           g_nInstanceCounter       ;
    SfxObjectShellRef                                          m_pObjectShell           ;
    OUString                                                   m_sURL                   ;
    OUString                                                   m_sRuntimeUID            ;
    OUString                                                   m_aPreusedFilterName     ;
    comphelper::OInterfaceContainerHelper3<view::XPrintJobListener>  m_aPrintJobListeners;
    comphelper::OInterfaceContainerHelper3<lang::XEventListener>  m_aEventListeners;
    comphelper::OInterfaceContainerHelper3<util::XModifyListener>  m_aModifyListeners;
    comphelper::OInterfaceContainerHelper3<document::XEventListener>  m_aDocumentEventListeners1;
    comphelper::OInterfaceContainerHelper3<document::XDocumentEventListener>  m_aDocumentEventListeners2;
    comphelper::OInterfaceContainerHelper3<document::XStorageChangeListener>  m_aStorageChangeListeners;
    comphelper::OInterfaceContainerHelper3<util::XCloseListener>  m_aCloseListeners;
    std::unordered_map<css::uno::Reference< css::drawing::XShape >,
                       std::vector<css::uno::Reference< css::document::XShapeEventListener >>> maShapeListeners;
    Reference< XInterface >                                    m_xParent                ;
    Reference< frame::XController >                            m_xCurrent               ;
    Reference< document::XDocumentProperties >                 m_xDocumentProperties    ;
    Reference< script::XStarBasicAccess >                      m_xStarBasicAccess       ;
    Reference< container::XNameReplace >                       m_xEvents                ;
    Sequence< beans::PropertyValue>                            m_seqArguments           ;
    std::vector< Reference< frame::XController > >             m_seqControllers         ;
    Reference< container::XIndexAccess >                       m_contViewData           ;
    sal_uInt16                                                 m_nControllerLockCount   ;
    bool                                                       m_bClosed                ;
    bool                                                       m_bClosing               ;
    bool                                                       m_bSaving                ;
    bool                                                       m_bSuicide               ;
    bool                                                       m_bExternalTitle         ;
    bool                                                       m_bDisposing             ;
    Reference< view::XPrintable>                               m_xPrintable             ;
    Reference< ui::XUIConfigurationManager2 >                  m_xUIConfigurationManager;
    ::rtl::Reference< ::sfx2::DocumentStorageModifyListener >  m_pStorageModifyListen   ;
    OUString                                                   m_sModuleIdentifier      ;
    Reference< frame::XTitle >                                 m_xTitleHelper           ;
    Reference< frame::XUntitledNumbers >                       m_xNumberedControllers   ;
    Reference< rdf::XDocumentMetadataAccess>                   m_xDocumentMetadata      ;
    ::rtl::Reference< ::sfx2::DocumentUndoManager >            m_pDocumentUndoManager   ;
    Sequence< document::CmisProperty>                          m_cmisProperties         ;
    std::shared_ptr<SfxGrabBagItem>                            m_xGrabBagItem           ;
    std::optional<std::chrono::steady_clock::time_point>       m_oDirtyTimestamp        ;

    IMPL_SfxBaseModel_DataContainer( ::osl::Mutex& rMutex, SfxObjectShell* pObjectShell )
            :   m_pObjectShell          ( pObjectShell  )
            ,   m_aPrintJobListeners    ( rMutex    )
            ,   m_aEventListeners       ( rMutex    )
            ,   m_aModifyListeners      ( rMutex    )
            ,   m_aDocumentEventListeners1( rMutex  )
            ,   m_aDocumentEventListeners2( rMutex  )
            ,   m_aStorageChangeListeners ( rMutex  )
            ,   m_aCloseListeners       ( rMutex    )
            ,   m_nControllerLockCount  ( 0         )
            ,   m_bClosed               ( false     )
            ,   m_bClosing              ( false     )
            ,   m_bSaving               ( false     )
            ,   m_bSuicide              ( false     )
            ,   m_bExternalTitle        ( false     )
            ,   m_bDisposing            ( false     )
    {
        // increase global instance counter.
        ++g_nInstanceCounter;
        // set own Runtime UID
        m_sRuntimeUID = OUString::number( g_nInstanceCounter );
    }

    virtual ~IMPL_SfxBaseModel_DataContainer()
    {
    }

    // ::sfx2::IModifiableDocument
    virtual void storageIsModified() override
    {
        if ( m_pObjectShell.is() && !m_pObjectShell->IsModified() )
            m_pObjectShell->SetModified();
    }

    void impl_setDocumentProperties(
            const Reference< document::XDocumentProperties >& );

    Reference<rdf::XDocumentMetadataAccess> GetDMA()
    {
        if (!m_xDocumentMetadata.is())
        {
            OSL_ENSURE(m_pObjectShell.is(), "GetDMA: no object shell?");
            if (!m_pObjectShell.is())
            {
                return nullptr;
            }

            const Reference<XComponentContext> xContext(
                ::comphelper::getProcessComponentContext());
            const Reference<frame::XModel> xModel(
                m_pObjectShell->GetModel());
            const Reference<lang::XMultiComponentFactory> xMsf(
                xContext->getServiceManager());
            const Reference<frame::
                XTransientDocumentsDocumentContentFactory> xTDDCF(
                    xMsf->createInstanceWithContext(
                        "com.sun.star.frame.TransientDocumentsDocumentContentFactory",
                    xContext),
                UNO_QUERY_THROW);
            const Reference<ucb::XContent> xContent(
                xTDDCF->createDocumentContent(xModel) );
            OSL_ENSURE(xContent.is(), "GetDMA: cannot create DocumentContent");
            if (!xContent.is())
            {
                return nullptr;
            }
            OUString uri = xContent->getIdentifier()->getContentIdentifier();
            OSL_ENSURE(!uri.isEmpty(), "GetDMA: empty uri?");
            if (!uri.isEmpty() && !uri.endsWith("/"))
            {
                uri += "/";
            }

            m_xDocumentMetadata = new ::sfx2::DocumentMetadataAccess(
                xContext, *m_pObjectShell, uri);
        }
        return m_xDocumentMetadata;
    }

    Reference<rdf::XDocumentMetadataAccess> CreateDMAUninitialized()
    {
        return (m_pObjectShell.is())
            ? new ::sfx2::DocumentMetadataAccess(
                ::comphelper::getProcessComponentContext(), *m_pObjectShell)
            : nullptr;
    }

    void setModifiedForAutoSave(bool val)
    {
        if (val)
        {
            if (!m_oDirtyTimestamp)
                m_oDirtyTimestamp.emplace(std::chrono::steady_clock::now());
        }
        else
        {
            m_oDirtyTimestamp.reset();
        }
    }
};

// static member initialization.
sal_Int64 IMPL_SfxBaseModel_DataContainer::g_nInstanceCounter = 0;

namespace {

// Listener that forwards notifications from the PrintHelper to the "real" listeners
class SfxPrintHelperListener_Impl : public ::cppu::WeakImplHelper< view::XPrintJobListener >
{
public:
    IMPL_SfxBaseModel_DataContainer* m_pData;
    explicit SfxPrintHelperListener_Impl( IMPL_SfxBaseModel_DataContainer* pData )
        : m_pData( pData )
    {}

    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) override ;
    virtual void SAL_CALL printJobEvent( const view::PrintJobEvent& rEvent ) override;
};

}

void SAL_CALL SfxPrintHelperListener_Impl::disposing( const lang::EventObject& )
{
    m_pData->m_xPrintable = nullptr;
}

void SAL_CALL SfxPrintHelperListener_Impl::printJobEvent( const view::PrintJobEvent& rEvent )
{
    if ( m_pData->m_aPrintJobListeners.getLength() )
    {
        m_pData->m_aPrintJobListeners.notifyEach(&view::XPrintJobListener::printJobEvent, rEvent);
    }
}

namespace {

// SfxOwnFramesLocker ====================================================================================
// allows to lock all the frames related to the provided SfxObjectShell
class SfxOwnFramesLocker
{
    Sequence< Reference< frame::XFrame > > m_aLockedFrames;

    static vcl::Window* GetVCLWindow( const Reference< frame::XFrame >& xFrame );
public:
    explicit SfxOwnFramesLocker( SfxObjectShell const * ObjechShell );
    ~SfxOwnFramesLocker();
};

}

SfxOwnFramesLocker::SfxOwnFramesLocker( SfxObjectShell const * pObjectShell )
{
    if ( !pObjectShell )
        return;

    if ( comphelper::LibreOfficeKit::isForkedChild() )
        return; // no need to tweak UI when in the background

    for (   SfxViewFrame *pFrame = SfxViewFrame::GetFirst( pObjectShell );
            pFrame;
            pFrame = SfxViewFrame::GetNext( *pFrame, pObjectShell )
        )
    {
        SfxFrame& rSfxFrame = pFrame->GetFrame();
        try
        {
            // get vcl window related to the frame and lock it if it is still not locked
            const Reference< frame::XFrame >& xFrame = rSfxFrame.GetFrameInterface();
            vcl::Window* pWindow = GetVCLWindow( xFrame );
            if ( !pWindow )
                throw RuntimeException();

            if ( pWindow->IsEnabled() )
            {
                pWindow->Disable();

                try
                {
                    sal_Int32 nLen = m_aLockedFrames.getLength();
                    m_aLockedFrames.realloc( nLen + 1 );
                    m_aLockedFrames.getArray()[nLen] = xFrame;
                }
                catch( Exception& )
                {
                    pWindow->Enable();
                    throw;
                }
            }
        }
        catch( Exception& )
        {
            OSL_FAIL( "Not possible to lock the frame window!" );
        }
    }
}

SfxOwnFramesLocker::~SfxOwnFramesLocker()
{
    for ( auto& rFrame : asNonConstRange(m_aLockedFrames) )
    {
        try
        {
            if ( rFrame.is() )
            {
                // get vcl window related to the frame and unlock it
                vcl::Window* pWindow = GetVCLWindow( rFrame );
                if ( !pWindow )
                    throw RuntimeException();

                pWindow->Enable();

                rFrame.clear();
            }
        }
        catch( Exception& )
        {
            OSL_FAIL( "Can't unlock the frame window!" );
        }
    }
}

vcl::Window* SfxOwnFramesLocker::GetVCLWindow( const Reference< frame::XFrame >& xFrame )
{
    VclPtr<vcl::Window> pWindow;

    if ( xFrame.is() )
    {
        Reference< awt::XWindow > xWindow = xFrame->getContainerWindow();
        if ( xWindow.is() )
               pWindow = VCLUnoHelper::GetWindow( xWindow );
    }

    return pWindow;
}

namespace {

// SfxSaveGuard ====================================================================================
class SfxSaveGuard
{
    private:
        Reference< frame::XModel > m_xModel;
        IMPL_SfxBaseModel_DataContainer* m_pData;
        std::unique_ptr<SfxOwnFramesLocker> m_pFramesLock;

        SfxSaveGuard(SfxSaveGuard const &) = delete;
        void operator =(const SfxSaveGuard&) = delete;

    public:
        SfxSaveGuard(const Reference< frame::XModel >&             xModel                      ,
                           IMPL_SfxBaseModel_DataContainer* pData);
        ~SfxSaveGuard();
};

}

SfxSaveGuard::SfxSaveGuard(const Reference< frame::XModel >&             xModel                      ,
                                 IMPL_SfxBaseModel_DataContainer* pData)
    : m_xModel     ( xModel )
    , m_pData      ( pData )
{
    if ( m_pData->m_bClosed )
        throw lang::DisposedException("Object already disposed.");

    m_pData->m_bSaving = true;
    m_pFramesLock.reset(new SfxOwnFramesLocker( m_pData->m_pObjectShell.get() ));
}

SfxSaveGuard::~SfxSaveGuard()
{
    m_pFramesLock.reset();

    m_pData->m_bSaving = false;

    // m_bSuicide was set e.g. in case someone tried to close a document, while it was used for
    // storing at the same time. Further m_bSuicide was set to sal_True only if close(sal_True) was called.
    // So the ownership was delegated to the place where a veto exception was thrown.
    // Now we have to call close() again and delegate the ownership to the next one, which
    // can't accept that. Close(sal_False) can't work in this case. Because then the document will may be never closed...

    if ( !m_pData->m_bSuicide )
        return;

    // Reset this state. In case the new close() request is not accepted by someone else...
    // it's not a good idea to have two "owners" for close.-)
    m_pData->m_bSuicide = false;
    try
    {
        Reference< util::XCloseable > xClose(m_xModel, UNO_QUERY);
        if (xClose.is())
            xClose->close(true);
    }
    catch(const util::CloseVetoException&)
    {}
}

SfxBaseModel::SfxBaseModel( SfxObjectShell *pObjectShell )
: BaseMutex()
, m_pData( std::make_shared<IMPL_SfxBaseModel_DataContainer>( m_aMutex, pObjectShell ) )
, m_bSupportEmbeddedScripts( pObjectShell && pObjectShell->Get_Impl() && !pObjectShell->Get_Impl()->m_bNoBasicCapabilities )
, m_bSupportDocRecovery( pObjectShell && pObjectShell->Get_Impl() && pObjectShell->Get_Impl()->m_bDocRecoverySupport )
{
    if ( pObjectShell != nullptr )
    {
        StartListening( *pObjectShell ) ;
    }
}

//  destructor
SfxBaseModel::~SfxBaseModel()
{
}

//  XInterface
Any SAL_CALL SfxBaseModel::queryInterface( const uno::Type& rType )
{
    if  (   ( !m_bSupportEmbeddedScripts && rType.equals( cppu::UnoType<document::XEmbeddedScripts>::get() ) )
        ||  ( !m_bSupportDocRecovery && (rType.equals( cppu::UnoType<XDocumentRecovery>::get() ) || rType.equals( cppu::UnoType<XDocumentRecovery2>::get() )) )
        )
        return Any();

    return SfxBaseModel_Base::queryInterface( rType );
}


//  XTypeProvider


namespace
{
    void lcl_stripType( Sequence< uno::Type >& io_rTypes, const uno::Type& i_rTypeToStrip )
    {
        Sequence< uno::Type > aStrippedTypes( io_rTypes.getLength() - 1 );
        ::std::remove_copy_if(
            std::cbegin(io_rTypes),
            std::cend(io_rTypes),
            aStrippedTypes.getArray(),
            [&i_rTypeToStrip](const uno::Type& aType) { return aType == i_rTypeToStrip; }
        );
        io_rTypes = aStrippedTypes;
    }
}

Sequence< uno::Type > SAL_CALL SfxBaseModel::getTypes()
{
    Sequence< uno::Type > aTypes( SfxBaseModel_Base::getTypes() );

    if ( !m_bSupportEmbeddedScripts )
        lcl_stripType( aTypes, cppu::UnoType<document::XEmbeddedScripts>::get() );

    if ( !m_bSupportDocRecovery )
        lcl_stripType( aTypes, cppu::UnoType<XDocumentRecovery2>::get() );

    return aTypes;
}


//  XTypeProvider


Sequence< sal_Int8 > SAL_CALL SfxBaseModel::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


//  XStarBasicAccess

#if HAVE_FEATURE_SCRIPTING

static Reference< script::XStarBasicAccess > implGetStarBasicAccess( SfxObjectShell const * pObjectShell )
{
    Reference< script::XStarBasicAccess > xRet;

#if !HAVE_FEATURE_SCRIPTING
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

#endif

Reference< container::XNameContainer > SAL_CALL SfxBaseModel::getLibraryContainer()
{
#if !HAVE_FEATURE_SCRIPTING
    Reference< container::XNameContainer > dummy;

    return dummy;
#else
    SfxModelGuard aGuard( *this );

    Reference< script::XStarBasicAccess >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() && m_pData->m_pObjectShell.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell.get() );

    Reference< container::XNameContainer > xRet;
    if( rxAccess.is() )
        xRet = rxAccess->getLibraryContainer();
    return xRet;
#endif
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::createLibrary( const OUString& LibName, const OUString& Password,
    const OUString& ExternalSourceURL, const OUString& LinkTargetURL )
{
#if !HAVE_FEATURE_SCRIPTING
    (void) LibName;
    (void) Password;
    (void) ExternalSourceURL;
    (void) LinkTargetURL;
#else
    SfxModelGuard aGuard( *this );

    Reference< script::XStarBasicAccess >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() && m_pData->m_pObjectShell.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell.get() );

    if( rxAccess.is() )
        rxAccess->createLibrary( LibName, Password, ExternalSourceURL, LinkTargetURL );
#endif
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::addModule( const OUString& LibraryName, const OUString& ModuleName,
    const OUString& Language, const OUString& Source )
{
#if !HAVE_FEATURE_SCRIPTING
    (void) LibraryName;
    (void) ModuleName;
    (void) Language;
    (void) Source;
#else
    SfxModelGuard aGuard( *this );

    Reference< script::XStarBasicAccess >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() && m_pData->m_pObjectShell.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell.get() );

    if( rxAccess.is() )
        rxAccess->addModule( LibraryName, ModuleName, Language, Source );
#endif
}

/**___________________________________________________________________________________________________
    @seealso    XStarBasicAccess
*/
void SAL_CALL SfxBaseModel::addDialog( const OUString& LibraryName, const OUString& DialogName,
    const Sequence< sal_Int8 >& Data )
{
#if !HAVE_FEATURE_SCRIPTING
    (void) LibraryName;
    (void) DialogName;
    (void) Data;
#else
    SfxModelGuard aGuard( *this );

    Reference< script::XStarBasicAccess >& rxAccess = m_pData->m_xStarBasicAccess;
    if( !rxAccess.is() && m_pData->m_pObjectShell.is() )
        rxAccess = implGetStarBasicAccess( m_pData->m_pObjectShell.get() );

    if( rxAccess.is() )
        rxAccess->addDialog( LibraryName, DialogName, Data );
#endif
}


//  XChild


Reference< XInterface > SAL_CALL SfxBaseModel::getParent()
{
    SfxModelGuard aGuard( *this );

    return m_pData->m_xParent;
}


//  XChild


void SAL_CALL SfxBaseModel::setParent(const Reference< XInterface >& Parent)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    m_pData->m_xParent = Parent;
}


//  XChild


void SAL_CALL SfxBaseModel::dispose()
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    if  ( !m_pData->m_bClosed )
    {
        // gracefully accept wrong dispose calls instead of close call
        // and try to make it work (may be really disposed later!)
        try
        {
            close( true );
        }
        catch ( util::CloseVetoException& )
        {
        }

        return;
    }

    if  ( m_pData->m_bDisposing )
        return;
    m_pData->m_bDisposing = true;

    if ( m_pData->m_pStorageModifyListen.is() )
    {
        m_pData->m_pStorageModifyListen->dispose();
        m_pData->m_pStorageModifyListen = nullptr;
    }

    if ( m_pData->m_pDocumentUndoManager.is() )
    {
        m_pData->m_pDocumentUndoManager->disposing();
        m_pData->m_pDocumentUndoManager = nullptr;
    }

    lang::EventObject aEvent( static_cast<frame::XModel *>(this) );
    m_pData->m_aPrintJobListeners.disposeAndClear( aEvent );
    m_pData->m_aEventListeners.disposeAndClear( aEvent );
    m_pData->m_aModifyListeners.disposeAndClear( aEvent );
    m_pData->m_aDocumentEventListeners1.disposeAndClear( aEvent );
    m_pData->m_aDocumentEventListeners2.disposeAndClear( aEvent );
    m_pData->m_aStorageChangeListeners.disposeAndClear( aEvent );
    m_pData->m_aCloseListeners.disposeAndClear( aEvent );

    m_pData->m_xDocumentProperties.clear();

    m_pData->m_xDocumentMetadata.clear();

    if ( m_pData->m_pObjectShell.is() )
    {
        EndListening( *m_pData->m_pObjectShell );
    }

    m_pData->m_xCurrent.clear();
    m_pData->m_seqControllers.clear();

    // m_pData member must be set to zero before delete is called to
    // force disposed exception whenever someone tries to access our
    // instance while in the dtor.
    m_pData.reset();
}


//  XChild


void SAL_CALL SfxBaseModel::addEventListener( const Reference< lang::XEventListener >& aListener )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    m_pData->m_aEventListeners.addInterface( aListener );
}


//  XChild


void SAL_CALL SfxBaseModel::removeEventListener( const Reference< lang::XEventListener >& aListener )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    m_pData->m_aEventListeners.removeInterface( aListener );
}

void
IMPL_SfxBaseModel_DataContainer::impl_setDocumentProperties(
        const Reference< document::XDocumentProperties >& rxNewDocProps)
{
    m_xDocumentProperties.set(rxNewDocProps, UNO_SET_THROW);
    if (m_pObjectShell.is())
    {
        Reference<util::XModifyBroadcaster> const xMB(
            m_xDocumentProperties, UNO_QUERY_THROW);
        xMB->addModifyListener(new SfxDocInfoListener_Impl(*m_pObjectShell));
    }
}

// document::XDocumentPropertiesSupplier:
Reference< document::XDocumentProperties > SAL_CALL
SfxBaseModel::getDocumentProperties()
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    if ( !m_pData->m_xDocumentProperties.is() )
    {
        Reference< document::XDocumentProperties > xDocProps(
            document::DocumentProperties::create( ::comphelper::getProcessComponentContext() ) );
        m_pData->impl_setDocumentProperties(xDocProps);
    }

    return m_pData->m_xDocumentProperties;
}


//  lang::XEventListener


void SAL_CALL SfxBaseModel::disposing( const lang::EventObject& aObject )
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() )
        return;

    Reference< util::XModifyListener >  xMod( aObject.Source, UNO_QUERY );
    Reference< lang::XEventListener >  xListener( aObject.Source, UNO_QUERY );
    Reference< document::XEventListener >  xDocListener( aObject.Source, UNO_QUERY );

    if ( xMod.is() )
        m_pData->m_aModifyListeners.removeInterface( xMod );
    else if ( xListener.is() )
        m_pData->m_aEventListeners.removeInterface( xListener );
    else if ( xDocListener.is() )
        m_pData->m_aDocumentEventListeners1.removeInterface( xDocListener );
}


//  frame::XModel


sal_Bool SAL_CALL SfxBaseModel::attachResource( const   OUString&                   rURL    ,
                                                const   Sequence< beans::PropertyValue >&  rArgs   )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    if ( rURL.isEmpty() && rArgs.getLength() == 1 && rArgs[0].Name == "SetEmbedded" )
    {
        // allows to set a windowless document to EMBEDDED state
        // but _only_ before load() or initNew() methods
        if ( m_pData->m_pObjectShell.is() && !m_pData->m_pObjectShell->GetMedium() )
        {
            bool bEmb(false);
            if ( ( rArgs[0].Value >>= bEmb ) && bEmb )
                m_pData->m_pObjectShell->SetCreateMode_Impl( SfxObjectCreateMode::EMBEDDED );
        }

        return true;
    }

    if ( m_pData->m_pObjectShell.is() )
    {
        m_pData->m_sURL = rURL;

        SfxObjectShell* pObjectShell = m_pData->m_pObjectShell.get();

        Sequence< sal_Int32 > aWinExtent;
        for (const beans::PropertyValue & rProp : rArgs)
        {
            if (rProp.Name == "WinExtent" && (rProp.Value >>= aWinExtent) && ( aWinExtent.getLength() == 4 ) )
            {
                tools::Rectangle aVisArea( aWinExtent[0], aWinExtent[1], aWinExtent[2], aWinExtent[3] );
                aVisArea = OutputDevice::LogicToLogic(aVisArea, MapMode(MapUnit::Map100thMM), MapMode(pObjectShell->GetMapUnit()));
                pObjectShell->SetVisArea( aVisArea );
            }
            bool bBreakMacroSign = false;
            if ( rProp.Name == "BreakMacroSignature" && (rProp.Value >>= bBreakMacroSign) )
            {
                pObjectShell->BreakMacroSign_Impl( bBreakMacroSign );
            }
            bool bMacroEventRead = false;
            if ( rProp.Name == "MacroEventRead" && (rProp.Value >>= bMacroEventRead) && bMacroEventRead)
            {
                pObjectShell->SetMacroCallsSeenWhileLoading();
            }
        }
        Sequence<beans::PropertyValue> aStrippedArgs(rArgs.getLength());
        beans::PropertyValue* pStripped = aStrippedArgs.getArray();
        for (const beans::PropertyValue & rProp : rArgs)
        {
            if (rProp.Name == "WinExtent"
                || rProp.Name == "BreakMacroSignature"
                || rProp.Name == "MacroEventRead"
                || rProp.Name == "Stream"
                || rProp.Name == "InputStream"
                || rProp.Name == "URL"
                || rProp.Name == "Frame"
                || rProp.Name == "Password"
                || rProp.Name == "EncryptionData")
                continue;
            *pStripped++ = rProp;
        }
        aStrippedArgs.realloc(pStripped - aStrippedArgs.getArray());

        // TODO/LATER: all the parameters that are accepted by ItemSet of the DocShell must be removed here

        m_pData->m_seqArguments = aStrippedArgs;

        SfxMedium* pMedium = pObjectShell->GetMedium();
        if ( pMedium )
        {
            SfxAllItemSet aSet( pObjectShell->GetPool() );
            TransformParameters( SID_OPENDOC, rArgs, aSet );

            // the arguments are not allowed to reach the medium
            aSet.ClearItem( SID_FILE_NAME );
            aSet.ClearItem( SID_FILLFRAME );

            pMedium->GetItemSet().Put( aSet );
            const SfxStringItem* pItem = aSet.GetItem<SfxStringItem>(SID_FILTER_NAME, false);
            if ( pItem )
                pMedium->SetFilter(
                    pObjectShell->GetFactory().GetFilterContainer()->GetFilter4FilterName( pItem->GetValue() ) );

            const SfxStringItem* pTitleItem = aSet.GetItem<SfxStringItem>(SID_DOCINFO_TITLE, false);
            if ( pTitleItem )
            {
                SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pObjectShell );
                if ( pFrame )
                    pFrame->UpdateTitle();
            }
        }
    }

    return true ;
}


//  frame::XModel


OUString SAL_CALL SfxBaseModel::getURL()
{
    SfxModelGuard aGuard( *this );
    return m_pData->m_sURL ;
}


//  frame::XModel

Sequence< beans::PropertyValue > SAL_CALL SfxBaseModel::getArgs()
{
    return getArgs2({});
}

//  frame::XModel3

Sequence< beans::PropertyValue > SAL_CALL SfxBaseModel::getArgs2(const Sequence<OUString> & requestedArgsSeq )
{
    SfxModelGuard aGuard( *this );

    if (!SfxApplication::Get()) // tdf#113755
    {
        SAL_WARN("sfx.appl", "Unexpected operations on model");
        return m_pData->m_seqArguments;
    }

    std::set<std::u16string_view> requestedArgs;
    for (OUString const & s : requestedArgsSeq)
        requestedArgs.insert(s);

    if ( m_pData->m_pObjectShell.is() )
    {
        Sequence< beans::PropertyValue > seqArgsNew;
        Sequence< beans::PropertyValue > seqArgsOld;
        SfxAllItemSet aSet( m_pData->m_pObjectShell->GetPool() );

        // we need to know which properties are supported by the transformer
        // hopefully it is a temporary solution, I guess nonconvertable properties
        // should not be supported so then there will be only ItemSet from medium

        TransformItems( SID_OPENDOC, m_pData->m_pObjectShell->GetMedium()->GetItemSet(), seqArgsNew );
        TransformParameters( SID_OPENDOC, m_pData->m_seqArguments, aSet );
        TransformItems( SID_OPENDOC, aSet, seqArgsOld );

        sal_Int32 nNewLength = seqArgsNew.getLength();

        if (requestedArgs.empty() || requestedArgs.count(u"WinExtent"))
        {
            // "WinExtent" property should be updated always.
            // We can store it now to overwrite an old value
            // since it is not from ItemSet
            tools::Rectangle aTmpRect = m_pData->m_pObjectShell->GetVisArea( ASPECT_CONTENT );
            aTmpRect = OutputDevice::LogicToLogic(aTmpRect, MapMode(m_pData->m_pObjectShell->GetMapUnit()), MapMode(MapUnit::Map100thMM));

            Sequence< sal_Int32 > aRectSeq
            {
                o3tl::narrowing<int>(aTmpRect.Left()),
                o3tl::narrowing<int>(aTmpRect.Top()),
                o3tl::narrowing<int>(aTmpRect.IsWidthEmpty() ? aTmpRect.Left() : aTmpRect.Right()),
                o3tl::narrowing<int>(aTmpRect.IsHeightEmpty() ? aTmpRect.Top() : aTmpRect.Bottom())
            };

            seqArgsNew.realloc( ++nNewLength );
            auto pseqArgsNew = seqArgsNew.getArray();
            pseqArgsNew[ nNewLength - 1 ].Name = "WinExtent";
            pseqArgsNew[ nNewLength - 1 ].Value <<= aRectSeq;
        }

        if (requestedArgs.empty() || requestedArgs.count(u"PreusedFilterName"))
        {
            if ( !m_pData->m_aPreusedFilterName.isEmpty() )
            {
                seqArgsNew.realloc( ++nNewLength );
                auto pseqArgsNew = seqArgsNew.getArray();
                pseqArgsNew[ nNewLength - 1 ].Name = "PreusedFilterName";
                pseqArgsNew[ nNewLength - 1 ].Value <<= m_pData->m_aPreusedFilterName;
            }
        }

        if (requestedArgs.empty() || requestedArgs.count(u"DocumentBorder"))
        {
            SfxViewFrame* pFrame = SfxViewFrame::GetFirst( m_pData->m_pObjectShell.get() );
            if ( pFrame )
            {
                SvBorder aBorder = pFrame->GetBorderPixelImpl();

                Sequence< sal_Int32 > aBorderSeq
                {
                    o3tl::narrowing<int>(aBorder.Left()),
                    o3tl::narrowing<int>(aBorder.Top()),
                    o3tl::narrowing<int>(aBorder.Right()),
                    o3tl::narrowing<int>(aBorder.Bottom())
                };

                seqArgsNew.realloc( ++nNewLength );
                auto pseqArgsNew = seqArgsNew.getArray();
                pseqArgsNew[ nNewLength - 1 ].Name = "DocumentBorder";
                pseqArgsNew[ nNewLength - 1 ].Value <<= aBorderSeq;
            }
        }

        if (requestedArgs.empty())
        {
            // only the values that are not supported by the ItemSet must be cached here
            Sequence< beans::PropertyValue > aFinalCache;
            sal_Int32 nFinalLength = 0;

            for ( const auto& rOrg : std::as_const(m_pData->m_seqArguments) )
            {
                auto bNew = std::none_of(std::cbegin(seqArgsOld), std::cend(seqArgsOld),
                    [&rOrg](const beans::PropertyValue& rOld){ return rOld.Name == rOrg.Name; });
                if ( bNew )
                {
                    // the entity with this name should be new for seqArgsNew
                    // since it is not supported by transformer

                    seqArgsNew.realloc( ++nNewLength );
                    seqArgsNew.getArray()[ nNewLength - 1 ] = rOrg;

                    aFinalCache.realloc( ++nFinalLength );
                    aFinalCache.getArray()[ nFinalLength - 1 ] = rOrg;
                }
            }

            m_pData->m_seqArguments = aFinalCache;
        }

        return seqArgsNew;
    }

    return m_pData->m_seqArguments;
}

void SAL_CALL SfxBaseModel::setArgs(const Sequence<beans::PropertyValue>& aArgs)
{
    SfxModelGuard aGuard( *this );

    SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
    if (!pMedium)
    {
        throw util::InvalidStateException(
            "Medium could not be retrieved, unable to execute setArgs");
    }

    for (const auto& rArg : aArgs)
    {
        OUString sValue;
        bool bValue;
        bool ok = false;
        if (rArg.Name == "SuggestedSaveAsName")
        {
            if (rArg.Value >>= sValue)
            {
                pMedium->GetItemSet().Put(SfxStringItem(SID_SUGGESTEDSAVEASNAME, sValue));
                ok = true;
            }
        }
        else if (rArg.Name == "SuggestedSaveAsDir")
        {
            if (rArg.Value >>= sValue)
            {
                pMedium->GetItemSet().Put(SfxStringItem(SID_SUGGESTEDSAVEASDIR, sValue));
                ok = true;
            }
        }
        else if (rArg.Name == "LockContentExtraction")
        {
            if (rArg.Value >>= bValue)
            {
                pMedium->GetItemSet().Put(SfxBoolItem(SID_LOCK_CONTENT_EXTRACTION, bValue));
                ok = true;
            }
        }
        else if (rArg.Name == "LockExport")
        {
            if (rArg.Value >>= bValue)
            {
                pMedium->GetItemSet().Put(SfxBoolItem(SID_LOCK_EXPORT, bValue));
                ok = true;
            }
        }
        else if (rArg.Name == "LockPrint")
        {
            if (rArg.Value >>= bValue)
            {
                pMedium->GetItemSet().Put(SfxBoolItem(SID_LOCK_PRINT, bValue));
                ok = true;
            }
        }
        else if (rArg.Name == "LockSave")
        {
            if (rArg.Value >>= bValue)
            {
                pMedium->GetItemSet().Put(SfxBoolItem(SID_LOCK_SAVE, bValue));
                ok = true;
            }
        }
        else if (rArg.Name == "LockEditDoc")
        {
            if (rArg.Value >>= bValue)
            {
                pMedium->GetItemSet().Put(SfxBoolItem(SID_LOCK_EDITDOC, bValue));
                ok = true;
            }
        }
        else if (rArg.Name == "Replaceable")
        {
            if (rArg.Value >>= bValue)
            {
                pMedium->GetItemSet().Put(SfxBoolItem(SID_REPLACEABLE, bValue));
                ok = true;
            }
        }
        else if (rArg.Name == "EncryptionData")
        {
            pMedium->GetItemSet().Put(SfxUnoAnyItem(SID_ENCRYPTIONDATA, rArg.Value));
            ok = true;
        }
        if (!ok)
        {
            throw lang::IllegalArgumentException("Setting property not supported: " + rArg.Name,
                                                 comphelper::getProcessComponentContext(), 0);
        }
    }
}

//  frame::XModel


void SAL_CALL SfxBaseModel::connectController( const Reference< frame::XController >& xController )
{
    SfxModelGuard aGuard( *this );
    OSL_PRECOND( xController.is(), "SfxBaseModel::connectController: invalid controller!" );
    if ( !xController.is() )
        return;

    m_pData->m_seqControllers.push_back(xController);

    if ( m_pData->m_seqControllers.size() == 1 )
    {
        SfxViewFrame* pViewFrame = SfxViewFrame::Get( xController, GetObjectShell() );
        ENSURE_OR_THROW( pViewFrame, "SFX document without SFX view!?" );
        pViewFrame->UpdateDocument_Impl();
        const OUString sDocumentURL = GetObjectShell()->GetMedium()->GetName();
        if ( !sDocumentURL.isEmpty() )
            SfxGetpApp()->Broadcast( SfxOpenUrlHint( sDocumentURL ) );
    }
}


//  frame::XModel


void SAL_CALL SfxBaseModel::disconnectController( const Reference< frame::XController >& xController )
{
    SfxModelGuard aGuard( *this );

    if ( m_pData->m_seqControllers.empty() )
        return;

    auto& vec = m_pData->m_seqControllers;
    std::erase(vec, xController);

    if ( xController == m_pData->m_xCurrent )
        m_pData->m_xCurrent.clear();
}

namespace
{
    class ControllerLockUndoAction : public ::cppu::WeakImplHelper< XUndoAction >
    {
    public:
        ControllerLockUndoAction( const Reference< XModel >& i_model, const bool i_undoIsUnlock )
            :m_xModel( i_model )
            ,m_bUndoIsUnlock( i_undoIsUnlock )
        {
        }

        // XUndoAction
        virtual OUString SAL_CALL getTitle() override;
        virtual void SAL_CALL undo(  ) override;
        virtual void SAL_CALL redo(  ) override;

    private:
        const Reference< XModel >   m_xModel;
        const bool                  m_bUndoIsUnlock;
    };

    OUString SAL_CALL ControllerLockUndoAction::getTitle()
    {
        // this action is intended to be used within an UndoContext only, so nobody will ever see this title ...
        return OUString();
    }

    void SAL_CALL ControllerLockUndoAction::undo(  )
    {
        if ( m_bUndoIsUnlock )
            m_xModel->unlockControllers();
        else
            m_xModel->lockControllers();
    }

    void SAL_CALL ControllerLockUndoAction::redo(  )
    {
        if ( m_bUndoIsUnlock )
            m_xModel->lockControllers();
        else
            m_xModel->unlockControllers();
    }
}


//  frame::XModel


void SAL_CALL SfxBaseModel::lockControllers()
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


//  frame::XModel


void SAL_CALL SfxBaseModel::unlockControllers()
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


//  frame::XModel


sal_Bool SAL_CALL SfxBaseModel::hasControllersLocked()
{
    SfxModelGuard aGuard( *this );
    return ( m_pData->m_nControllerLockCount != 0 ) ;
}


//  frame::XModel


Reference< frame::XController > SAL_CALL SfxBaseModel::getCurrentController()
{
    SfxModelGuard aGuard( *this );

    // get the last active controller of this model
    if ( m_pData->m_xCurrent.is() )
        return m_pData->m_xCurrent;

    // get the first controller of this model
    return !m_pData->m_seqControllers.empty() ? m_pData->m_seqControllers.front() : m_pData->m_xCurrent;
}


//  frame::XModel


void SAL_CALL SfxBaseModel::setCurrentController( const Reference< frame::XController >& xCurrentController )
{
    SfxModelGuard aGuard( *this );

    m_pData->m_xCurrent = xCurrentController;
}


//  frame::XModel


Reference< XInterface > SAL_CALL SfxBaseModel::getCurrentSelection()
{
    SfxModelGuard aGuard( *this );

    Reference< XInterface >     xReturn;
    Reference< frame::XController >    xController =   getCurrentController()      ;

    if ( xController.is() )
    {
        Reference< view::XSelectionSupplier >  xDocView( xController, UNO_QUERY );
        if ( xDocView.is() )
        {
            Any aSel = xDocView->getSelection();
            aSel >>= xReturn ;
        }
    }

    return xReturn ;
}


//  XModifiable2


sal_Bool SAL_CALL SfxBaseModel::disableSetModified()
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.is() )
        throw RuntimeException();

    bool bResult = m_pData->m_pObjectShell->IsEnableSetModified();
    m_pData->m_pObjectShell->EnableSetModified( false );

    return bResult;
}

sal_Bool SAL_CALL SfxBaseModel::enableSetModified()
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.is() )
        throw RuntimeException();

    bool bResult = m_pData->m_pObjectShell->IsEnableSetModified();
    m_pData->m_pObjectShell->EnableSetModified();

    return bResult;
}

sal_Bool SAL_CALL SfxBaseModel::isSetModifiedEnabled()
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.is() )
        throw RuntimeException();

    return m_pData->m_pObjectShell->IsEnableSetModified();
}


//  XModifiable


sal_Bool SAL_CALL SfxBaseModel::isModified()
{
    SfxModelGuard aGuard( *this );

    return m_pData->m_pObjectShell.is() && m_pData->m_pObjectShell->IsModified();
}


//  XModifiable


void SAL_CALL SfxBaseModel::setModified( sal_Bool bModified )
{
    SfxModelGuard aGuard( *this );

    if ( m_pData->m_pObjectShell.is() )
        m_pData->m_pObjectShell->SetModified(bModified);
}


//  XModifiable


void SAL_CALL SfxBaseModel::addModifyListener(const Reference< util::XModifyListener >& xListener)
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    m_pData->m_aModifyListeners.addInterface( xListener );
}


//  XModifiable


void SAL_CALL SfxBaseModel::removeModifyListener(const Reference< util::XModifyListener >& xListener)
{
    SfxModelGuard aGuard( *this );

    m_pData->m_aModifyListeners.removeInterface( xListener );
}


//  XCloseable


void SAL_CALL SfxBaseModel::close( sal_Bool bDeliverOwnership )
{
    SolarMutexGuard aGuard;
    if ( impl_isDisposed() || m_pData->m_bClosed || m_pData->m_bClosing )
        return;

    Reference< XInterface > xSelfHold( getXWeak() );
    lang::EventObject       aSource  ( getXWeak() );
    if (m_pData->m_aCloseListeners.getLength())
    {
        comphelper::OInterfaceIteratorHelper3 pIterator(m_pData->m_aCloseListeners);
        while (pIterator.hasMoreElements())
        {
            try
            {
                pIterator.next()->queryClosing( aSource, bDeliverOwnership );
            }
            catch( RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }

    if ( m_pData->m_bSaving )
    {
        if (bDeliverOwnership)
            m_pData->m_bSuicide = true;
        throw util::CloseVetoException(
                "Can not close while saving.",
                static_cast< util::XCloseable* >(this));
    }

    // no own objections against closing!
    m_pData->m_bClosing = true;
    if (m_pData->m_aCloseListeners.getLength())
    {
        comphelper::OInterfaceIteratorHelper3 pCloseIterator(m_pData->m_aCloseListeners);
        while (pCloseIterator.hasMoreElements())
        {
            try
            {
                pCloseIterator.next()->notifyClosing( aSource );
            }
            catch( RuntimeException& )
            {
                pCloseIterator.remove();
            }
        }
    }

    m_pData->m_bClosed = true;
    m_pData->m_bClosing = false;

    dispose();
}


//  XCloseBroadcaster


void SAL_CALL SfxBaseModel::addCloseListener( const Reference< util::XCloseListener >& xListener )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    m_pData->m_aCloseListeners.addInterface( xListener );
}


//  XCloseBroadcaster


void SAL_CALL SfxBaseModel::removeCloseListener( const Reference< util::XCloseListener >& xListener )
{
    SfxModelGuard aGuard( *this );

    m_pData->m_aCloseListeners.removeInterface( xListener );
}


//  XPrintable


Sequence< beans::PropertyValue > SAL_CALL SfxBaseModel::getPrinter()
{
    SfxModelGuard aGuard( *this );

    impl_getPrintHelper();
    return m_pData->m_xPrintable->getPrinter();
}

void SAL_CALL SfxBaseModel::setPrinter(const Sequence< beans::PropertyValue >& rPrinter)
{
    SfxModelGuard aGuard( *this );

    impl_getPrintHelper();
    m_pData->m_xPrintable->setPrinter( rPrinter );
}

void SAL_CALL SfxBaseModel::print(const Sequence< beans::PropertyValue >& rOptions)
{
    SfxModelGuard aGuard( *this );

    impl_getPrintHelper();

    // tdf#123728 Always print on main thread to avoid deadlocks
    vcl::solarthread::syncExecute([this, &rOptions]() { m_pData->m_xPrintable->print(rOptions); });
}

//  XStorable


sal_Bool SAL_CALL SfxBaseModel::hasLocation()
{
    SfxModelGuard aGuard( *this );

    return m_pData->m_pObjectShell.is() && m_pData->m_pObjectShell->HasName();
}


//  XStorable


OUString SAL_CALL SfxBaseModel::getLocation()
{
    SfxModelGuard aGuard( *this );

    if ( m_pData->m_pObjectShell.is() )
    {
        // TODO/LATER: is it correct that the shared document returns shared file location?
        if ( m_pData->m_pObjectShell->IsDocShared() )
            return m_pData->m_pObjectShell->GetSharedFileURL();
        else
            return m_pData->m_pObjectShell->GetMedium()->GetName();
    }

    return m_pData->m_sURL;
}


//  XStorable


sal_Bool SAL_CALL SfxBaseModel::isReadonly()
{
    SfxModelGuard aGuard( *this );

    return !m_pData->m_pObjectShell.is() || m_pData->m_pObjectShell->IsReadOnly();
}

//  XStorable2


void SAL_CALL SfxBaseModel::storeSelf( const    Sequence< beans::PropertyValue >&  aSeqArgs )
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.is() )
        return;

    SfxSaveGuard aSaveGuard(this, m_pData.get());

    bool bCheckIn = false;
    bool bOnMainThread = false;
    for ( const auto& rArg : aSeqArgs )
    {
        // check that only acceptable parameters are provided here
        if ( rArg.Name != "VersionComment" && rArg.Name != "Author"
          && rArg.Name != "DontTerminateEdit"
          && rArg.Name != "InteractionHandler" && rArg.Name != "StatusIndicator"
          && rArg.Name != "VersionMajor"
          && rArg.Name != "FailOnWarning"
          && rArg.Name != "CheckIn"
          && rArg.Name != "NoFileSync"
          && rArg.Name != "OnMainThread" )
        {
            const OUString aMessage( "Unexpected MediaDescriptor parameter: " + rArg.Name );
            throw lang::IllegalArgumentException( aMessage, Reference< XInterface >(), 1 );
        }
        else if ( rArg.Name == "CheckIn" )
        {
            rArg.Value >>= bCheckIn;
        }
        else if (rArg.Name == "OnMainThread")
        {
            rArg.Value >>= bOnMainThread;
        }
    }

    // Remove CheckIn property if needed
    sal_uInt16 nSlotId = SID_SAVEDOC;
    Sequence< beans::PropertyValue >  aArgs = aSeqArgs;
    if ( bCheckIn )
    {
        nSlotId = SID_CHECKIN;
        sal_Int32 nLength = aSeqArgs.getLength( );
        aArgs = Sequence< beans::PropertyValue >( nLength - 1 );
        std::copy_if(aSeqArgs.begin(), aSeqArgs.end(), aArgs.getArray(),
            [](const beans::PropertyValue& rProp) { return rProp.Name != "CheckIn"; });
    }

    std::optional<SfxAllItemSet> pParams(SfxGetpApp()->GetPool() );
    TransformParameters( nSlotId, aArgs, *pParams );

    SfxGetpApp()->NotifyEvent( SfxEventHint( SfxEventHintId::SaveDoc, GlobalEventConfig::GetEventName(GlobalEventId::SAVEDOC), m_pData->m_pObjectShell.get() ) );

    bool bRet = false;

    // TODO/LATER: let the embedded case of saving be handled more careful
    if ( m_pData->m_pObjectShell->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
    {
        // If this is an embedded object that has no URL based location it should be stored to own storage.
        // An embedded object can have a location based on URL in case it is a link, then it should be
        // stored in normal way.
        if ( !hasLocation() || getLocation().startsWith("private:") )
        {
            // actually in this very rare case only UI parameters have sense
            // TODO/LATER: should be done later, after integration of sb19
            bRet = m_pData->m_pObjectShell->DoSave()
                && m_pData->m_pObjectShell->DoSaveCompleted();
        }
        else
        {
            bRet = m_pData->m_pObjectShell->Save_Impl( &*pParams );
        }
    }
    else
    {
        // Tell the SfxMedium if we are in checkin instead of normal save
        m_pData->m_pObjectShell->GetMedium( )->SetInCheckIn( nSlotId == SID_CHECKIN );
        if (bOnMainThread)
            bRet = vcl::solarthread::syncExecute(
                [this, &pParams] { return m_pData->m_pObjectShell->Save_Impl(&*pParams); });
        else
            bRet = m_pData->m_pObjectShell->Save_Impl(&*pParams);
        m_pData->m_pObjectShell->GetMedium( )->SetInCheckIn( nSlotId != SID_CHECKIN );
    }

    pParams.reset();

    ErrCodeMsg nErrCode = m_pData->m_pObjectShell->GetErrorIgnoreWarning();
    m_pData->m_pObjectShell->ResetError();

    if ( bRet )
    {
        m_pData->m_aPreusedFilterName = GetMediumFilterName_Impl();

        SfxGetpApp()->NotifyEvent( SfxEventHint( SfxEventHintId::SaveDocDone, GlobalEventConfig::GetEventName(GlobalEventId::SAVEDOCDONE), m_pData->m_pObjectShell.get() ) );
    }
    else
    {
        if (!nErrCode)
            nErrCode = ERRCODE_IO_CANTWRITE;
        // write the contents of the logger to the file
        SfxGetpApp()->NotifyEvent( SfxEventHint( SfxEventHintId::SaveDocFailed, GlobalEventConfig::GetEventName(GlobalEventId::SAVEDOCFAILED), m_pData->m_pObjectShell.get() ) );

        throw task::ErrorCodeIOException(
            "SfxBaseModel::storeSelf: " + nErrCode.toString(),
            Reference< XInterface >(), sal_uInt32(nErrCode.GetCode()));
    }
}


//  XStorable


void SAL_CALL SfxBaseModel::store()
{
    comphelper::ProfileZone aZone("store");
    storeSelf( Sequence< beans::PropertyValue >() );
}


//  XStorable


void SAL_CALL SfxBaseModel::storeAsURL( const   OUString&                   rURL    ,
                                        const   Sequence< beans::PropertyValue >&  rArgs   )
{
    SfxModelGuard aGuard( *this );
    comphelper::ProfileZone aZone("storeAs");

    if ( !m_pData->m_pObjectShell.is() )
        return;

    SfxSaveGuard aSaveGuard(this, m_pData.get());

    utl::MediaDescriptor aDescriptor(rArgs);
    bool bOnMainThread = aDescriptor.getUnpackedValueOrDefault("OnMainThread", false);
    if (bOnMainThread)
    {
        vcl::solarthread::syncExecute([this, rURL, rArgs]() { impl_store(rURL, rArgs, false); });
    }
    else
    {
        impl_store(rURL, rArgs, false);
    }

    Sequence< beans::PropertyValue > aSequence ;
    TransformItems( SID_OPENDOC, m_pData->m_pObjectShell->GetMedium()->GetItemSet(), aSequence );
    attachResource( rURL, aSequence );

    loadCmisProperties( );

#if OSL_DEBUG_LEVEL > 0
    const SfxStringItem* pPasswdItem = m_pData->m_pObjectShell->GetMedium()->GetItemSet().GetItem(SID_PASSWORD, false);
    OSL_ENSURE( !pPasswdItem, "There should be no Password property in the document MediaDescriptor!" );
#endif
}


//  XUndoManagerSupplier

Reference< XUndoManager > SAL_CALL SfxBaseModel::getUndoManager(  )
{
    SfxModelGuard aGuard( *this );
    if ( !m_pData->m_pDocumentUndoManager.is() )
        m_pData->m_pDocumentUndoManager.set( new ::sfx2::DocumentUndoManager( *this ) );
    return m_pData->m_pDocumentUndoManager;
}


//  XStorable


void SAL_CALL SfxBaseModel::storeToURL( const   OUString&                   rURL    ,
                                        const   Sequence< beans::PropertyValue >&  rArgs   )
{
    SfxModelGuard aGuard( *this );
    comphelper::ProfileZone aZone("storeToURL");

    if ( !m_pData->m_pObjectShell.is() )
        return;

    SfxSaveGuard aSaveGuard(this, m_pData.get());
    try {
        utl::MediaDescriptor aDescriptor(rArgs);
        bool bOnMainThread = aDescriptor.getUnpackedValueOrDefault("OnMainThread", false);
        if (bOnMainThread)
            vcl::solarthread::syncExecute([this, rURL, rArgs]() { impl_store(rURL, rArgs, true); });
        else
            impl_store(rURL, rArgs, true);
    }
    catch (const uno::Exception &e)
    {
        // convert to the exception we announce in the throw
        // (eg. neon likes to throw InteractiveAugmentedIOException which
        // is not an io::IOException)
        throw io::IOException(e.Message, e.Context);
    }
}

sal_Bool SAL_CALL SfxBaseModel::wasModifiedSinceLastSave()
{
    SfxModelGuard aGuard( *this );
    return m_pData->m_oDirtyTimestamp.has_value();
}

void SAL_CALL SfxBaseModel::storeToRecoveryFile( const OUString& i_TargetLocation, const Sequence< PropertyValue >& i_MediaDescriptor )
{
    SfxModelGuard aGuard( *this );

    // delegate
    SfxSaveGuard aSaveGuard( this, m_pData.get() );
    impl_store( i_TargetLocation, i_MediaDescriptor, true );

    // no need for subsequent calls to storeToRecoveryFile, unless we're modified, again
    m_pData->setModifiedForAutoSave(false);
}

sal_Int64 SAL_CALL SfxBaseModel::getModifiedStateDuration()
{
    SfxModelGuard aGuard(*this);
    if (!m_pData->m_oDirtyTimestamp)
        return -1;
    auto ms = std::chrono::ceil<std::chrono::milliseconds>(std::chrono::steady_clock::now()
                                                           - *m_pData->m_oDirtyTimestamp);
    return ms.count();
}

void SAL_CALL SfxBaseModel::recoverFromFile( const OUString& i_SourceLocation, const OUString& i_SalvagedFile, const Sequence< PropertyValue >& i_MediaDescriptor )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    // delegate to our "load" method
    ::comphelper::NamedValueCollection aMediaDescriptor( i_MediaDescriptor );

    // our load implementation expects the SalvagedFile to be in the media descriptor
    OSL_ENSURE( !aMediaDescriptor.has( "SalvagedFile" ) || ( aMediaDescriptor.getOrDefault( "SalvagedFile", OUString() ) == i_SalvagedFile ),
        "SfxBaseModel::recoverFromFile: inconsistent information!" );
    aMediaDescriptor.put( "SalvagedFile", i_SalvagedFile );

    // similar for the to-be-loaded file
    OSL_ENSURE( !aMediaDescriptor.has( "URL" ) || ( aMediaDescriptor.getOrDefault( "URL", OUString() ) == i_SourceLocation ),
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


// XLoadable


void SAL_CALL SfxBaseModel::initNew()
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    if ( IsInitialized() )
        throw frame::DoubleInitializationException( OUString(), *this );

    // the object shell should exist always
    DBG_ASSERT( m_pData->m_pObjectShell.is(), "Model is useless without an ObjectShell" );
    if ( !m_pData->m_pObjectShell.is() )
        return;

    if( m_pData->m_pObjectShell->GetMedium() )
        throw frame::DoubleInitializationException();

    bool bRes = m_pData->m_pObjectShell->DoInitNew();
    ErrCodeMsg nErrCode = m_pData->m_pObjectShell->GetErrorIgnoreWarning() ?
                       m_pData->m_pObjectShell->GetErrorIgnoreWarning() : ERRCODE_IO_CANTCREATE;
    m_pData->m_pObjectShell->ResetError();

    if ( !bRes )
        throw task::ErrorCodeIOException(
            "SfxBaseModel::initNew: " + nErrCode.toString(),
            Reference< XInterface >(), sal_uInt32(nErrCode.GetCode()));
}

namespace {

OUString getFilterProvider( SfxMedium const & rMedium )
{
    const std::shared_ptr<const SfxFilter>& pFilter = rMedium.GetFilter();
    if (!pFilter)
        return OUString();

    return pFilter->GetProviderName();
}

void setUpdatePickList( SfxMedium* pMedium )
{
    if (!pMedium)
        return;

    bool bHidden = false;
    const SfxBoolItem* pHidItem = pMedium->GetItemSet().GetItem(SID_HIDDEN, false);
    if (pHidItem)
        bHidden = pHidItem->GetValue();

    pMedium->SetUpdatePickList(!bHidden);
}

}

void SAL_CALL SfxBaseModel::load(   const Sequence< beans::PropertyValue >& seqArguments )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    if ( IsInitialized() )
        throw frame::DoubleInitializationException( OUString(), *this );

    // the object shell should exist always
    DBG_ASSERT( m_pData->m_pObjectShell.is(), "Model is useless without an ObjectShell" );

    if (!m_pData->m_pObjectShell.is())
        return;

    if( m_pData->m_pObjectShell->GetMedium() )
        // if a Medium is present, the document is already initialized
        throw frame::DoubleInitializationException();

    SfxMedium* pMedium = new SfxMedium( seqArguments );

    ErrCodeMsg nError = ERRCODE_NONE;
    if (!getFilterProvider(*pMedium).isEmpty())
    {
        if (!m_pData->m_pObjectShell->DoLoadExternal(pMedium))
            nError = ERRCODE_IO_GENERAL;

        pMedium = handleLoadError(nError, pMedium);
        setUpdatePickList(pMedium);
        return;
    }

    OUString aFilterName;
    const SfxStringItem* pFilterNameItem = pMedium->GetItemSet().GetItem(SID_FILTER_NAME, false);
    if( pFilterNameItem )
        aFilterName = pFilterNameItem->GetValue();
    if( !m_pData->m_pObjectShell->GetFactory().GetFilterContainer()->GetFilter4FilterName( aFilterName ) )
    {
        // filtername is not valid
        delete pMedium;
        throw frame::IllegalArgumentIOException();
    }

    const SfxStringItem* pSalvageItem = pMedium->GetItemSet().GetItem(SID_DOC_SALVAGE, false);
    bool bSalvage = pSalvageItem != nullptr;

    // load document
    if ( !m_pData->m_pObjectShell->DoLoad(pMedium) )
        nError=ERRCODE_IO_GENERAL;

    // QUESTION: if the following happens outside of DoLoad, something important is missing there!
    Reference< task::XInteractionHandler > xHandler = pMedium->GetInteractionHandler();
    if( m_pData->m_pObjectShell->GetErrorCode() )
    {
        nError = m_pData->m_pObjectShell->GetErrorCode();
        if ( nError == ERRCODE_IO_BROKENPACKAGE && xHandler.is() )
        {
            const OUString aDocName( pMedium->GetURLObject().getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset ) );
            const SfxBoolItem* pRepairItem = pMedium->GetItemSet().GetItem(SID_REPAIRPACKAGE, false);
            if ( !pRepairItem || !pRepairItem->GetValue() )
            {
                RequestPackageReparation aRequest( aDocName );
                xHandler->handle( aRequest.GetRequest() );
                if( aRequest.isApproved() )
                {
                    // lok: we want to overwrite file in jail, so don't use template flag
                    bool bIsLOK = comphelper::LibreOfficeKit::isActive();
                    // broken package: try second loading and allow repair
                    pMedium->GetItemSet().Put( SfxBoolItem( SID_REPAIRPACKAGE, true ) );
                    pMedium->GetItemSet().Put( SfxBoolItem( SID_TEMPLATE, !bIsLOK ) );
                    pMedium->GetItemSet().Put( SfxStringItem( SID_DOCINFO_TITLE, aDocName ) );

                    // the error must be reset and the storage must be reopened in new mode
                    pMedium->ResetError();
                    pMedium->CloseStorage();
                    m_pData->m_pObjectShell->PrepareSecondTryLoad_Impl();
                    nError = ERRCODE_NONE;
                    if ( !m_pData->m_pObjectShell->DoLoad(pMedium) )
                        nError=ERRCODE_IO_GENERAL;
                    if (m_pData->m_pObjectShell->GetErrorCode())
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
        const SfxStringItem* pFilterItem = pMedium->GetItemSet().GetItem(SID_FILTER_NAME, false);
        SfxFilterMatcher& rMatcher = SfxGetpApp()->GetFilterMatcher();
        std::shared_ptr<const SfxFilter> pSetFilter = rMatcher.GetFilter4FilterName( pFilterItem->GetValue() );
        pMedium->SetFilter( pSetFilter );
        m_pData->m_pObjectShell->SetModified();
    }

    // TODO/LATER: maybe the mode should be retrieved from outside and the preused filter should not be set
    if ( m_pData->m_pObjectShell->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
    {
        const SfxStringItem* pFilterItem = pMedium->GetItemSet().GetItem(SID_FILTER_NAME, false);
        if ( pFilterItem )
            m_pData->m_aPreusedFilterName = pFilterItem->GetValue();
    }

    if ( !nError )
        nError = pMedium->GetErrorIgnoreWarning();

    m_pData->m_pObjectShell->ResetError();

    pMedium = handleLoadError(nError, pMedium);
    loadCmisProperties();
    setUpdatePickList(pMedium);

#if OSL_DEBUG_LEVEL > 0
    const SfxStringItem* pPasswdItem = pMedium->GetItemSet().GetItem(SID_PASSWORD, false);
    OSL_ENSURE( !pPasswdItem, "There should be no Password property in the document MediaDescriptor!" );
#endif
}


// XTransferable


Any SAL_CALL SfxBaseModel::getTransferData( const datatransfer::DataFlavor& aFlavor )
{
    SfxModelGuard aGuard( *this );

    Any aAny;

    if ( m_pData->m_pObjectShell.is() )
    {
        if ( aFlavor.MimeType == "application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"" )
        {
            if ( aFlavor.DataType != cppu::UnoType<Sequence< sal_Int8 >>::get() )
                throw datatransfer::UnsupportedFlavorException();

            TransferableObjectDescriptor aDesc;

            aDesc.maClassName = m_pData->m_pObjectShell->GetClassName();
            aDesc.maTypeName = aFlavor.HumanPresentableName;

            // TODO/LATER: ViewAspect needs to be sal_Int64
            aDesc.mnViewAspect = sal::static_int_cast< sal_uInt16 >( embed::Aspects::MSOLE_CONTENT );

            Size aSize = m_pData->m_pObjectShell->GetVisArea().GetSize();

            MapUnit aMapUnit = m_pData->m_pObjectShell->GetMapUnit();
            aDesc.maSize = OutputDevice::LogicToLogic(aSize, MapMode(aMapUnit), MapMode(MapUnit::Map100thMM));
            aDesc.maDragStartPos = Point();
            aDesc.maDisplayName.clear();

            SvMemoryStream aMemStm( 1024, 1024 );
            WriteTransferableObjectDescriptor( aMemStm, aDesc );
            aAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Tell() );
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-embed-source;windows_formatname=\"Star EMBS\"" )
        {
            if ( aFlavor.DataType != cppu::UnoType<Sequence< sal_Int8 >>::get() )
                throw datatransfer::UnsupportedFlavorException();

            try
            {
                utl::TempFileNamed aTmp;
                aTmp.EnableKillingFile();
                storeToURL( aTmp.GetURL(), Sequence < beans::PropertyValue >() );
                std::unique_ptr<SvStream> pStream(aTmp.GetStream( StreamMode::READ ));
                const sal_uInt32 nLen = pStream->TellEnd();
                Sequence< sal_Int8 > aSeq( nLen );
                pStream->ReadBytes(aSeq.getArray(), nLen);
                if( aSeq.hasElements() )
                    aAny <<= aSeq;
            }
            catch ( Exception& )
            {
            }
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"" )
        {
            if ( aFlavor.DataType != cppu::UnoType<Sequence< sal_Int8 >>::get() )
                throw datatransfer::UnsupportedFlavorException();


            std::shared_ptr<GDIMetaFile> xMetaFile =
                m_pData->m_pObjectShell->GetPreviewMetaFile( true );

            if (xMetaFile)
            {
                SvMemoryStream aMemStm( 65535, 65535 );
                aMemStm.SetVersion( SOFFICE_FILEFORMAT_CURRENT );

                SvmWriter aWriter( aMemStm );
                aWriter.Write( *xMetaFile );
                aAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ),
                                                aMemStm.TellEnd() );
            }
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\"" )
        {
            if ( aFlavor.DataType != cppu::UnoType<Sequence< sal_Int8 >>::get() )
                throw datatransfer::UnsupportedFlavorException();

            std::shared_ptr<GDIMetaFile> xMetaFile =
                m_pData->m_pObjectShell->GetPreviewMetaFile( true );

            if (xMetaFile)
            {
                SvMemoryStream aMemStm( 65535, 65535 );
                aMemStm.SetVersion( SOFFICE_FILEFORMAT_CURRENT );

                SvmWriter aWriter( aMemStm );
                aWriter.Write( *xMetaFile );
                aAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ),
                                                aMemStm.TellEnd() );
            }
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-emf;windows_formatname=\"Image EMF\"" )
        {
            if ( aFlavor.DataType == cppu::UnoType<Sequence< sal_Int8 >>::get() )
            {
                std::shared_ptr<GDIMetaFile> xMetaFile =
                    m_pData->m_pObjectShell->GetPreviewMetaFile( true );

                if (xMetaFile)
                {
                    std::unique_ptr<SvMemoryStream> xStream(
                        GraphicHelper::getFormatStrFromGDI_Impl(
                            xMetaFile.get(), ConvertDataFormat::EMF ) );
                    if (xStream)
                    {
                        xStream->SetVersion( SOFFICE_FILEFORMAT_CURRENT );
                        aAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( xStream->GetData() ),
                                                        xStream->TellEnd() );
                    }
                }
            }
            else if ( GraphicHelper::supportsMetaFileHandle_Impl()
              && aFlavor.DataType == cppu::UnoType<sal_uInt64>::get())
            {
                std::shared_ptr<GDIMetaFile> xMetaFile =
                    m_pData->m_pObjectShell->GetPreviewMetaFile( true );

                if (xMetaFile)
                {
                    aAny <<= reinterpret_cast< sal_uInt64 >(
                        GraphicHelper::getEnhMetaFileFromGDI_Impl( xMetaFile.get() ) );
                }
            }
            else
                throw datatransfer::UnsupportedFlavorException();
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"" )
        {
            if ( aFlavor.DataType == cppu::UnoType<Sequence< sal_Int8 >>::get() )
            {
                std::shared_ptr<GDIMetaFile> xMetaFile =
                    m_pData->m_pObjectShell->GetPreviewMetaFile( true );

                if (xMetaFile)
                {
                    std::unique_ptr<SvMemoryStream> xStream(
                        GraphicHelper::getFormatStrFromGDI_Impl(
                            xMetaFile.get(), ConvertDataFormat::WMF ) );

                    if (xStream)
                    {
                        xStream->SetVersion( SOFFICE_FILEFORMAT_CURRENT );
                        aAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( xStream->GetData() ),
                                                        xStream->TellEnd() );
                    }
                }
            }
            else if ( GraphicHelper::supportsMetaFileHandle_Impl()
              && aFlavor.DataType == cppu::UnoType<sal_uInt64>::get())
            {
                // means HGLOBAL handler to memory storage containing METAFILEPICT structure

                std::shared_ptr<GDIMetaFile> xMetaFile =
                    m_pData->m_pObjectShell->GetPreviewMetaFile( true );

                if (xMetaFile)
                {
                    Size aMetaSize = xMetaFile->GetPrefSize();
                    aAny <<= reinterpret_cast< sal_uInt64 >(
                        GraphicHelper::getWinMetaFileFromGDI_Impl(
                            xMetaFile.get(), aMetaSize ) );
                }
            }
            else
                throw datatransfer::UnsupportedFlavorException();
        }
        else if ( aFlavor.MimeType == "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" )
        {
            if ( aFlavor.DataType != cppu::UnoType<Sequence< sal_Int8 >>::get() )
                throw datatransfer::UnsupportedFlavorException();

            std::shared_ptr<GDIMetaFile> xMetaFile =
                m_pData->m_pObjectShell->GetPreviewMetaFile( true );

            if (xMetaFile)
            {
                std::unique_ptr<SvMemoryStream> xStream(
                    GraphicHelper::getFormatStrFromGDI_Impl(
                        xMetaFile.get(), ConvertDataFormat::BMP ) );

                if (xStream)
                {
                    xStream->SetVersion( SOFFICE_FILEFORMAT_CURRENT );
                    aAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( xStream->GetData() ),
                                                    xStream->TellEnd() );
                }
            }
        }
        else if ( aFlavor.MimeType == "image/png" )
        {
            if ( aFlavor.DataType != cppu::UnoType<Sequence< sal_Int8 >>::get() )
                throw datatransfer::UnsupportedFlavorException();

            std::shared_ptr<GDIMetaFile> xMetaFile =
                m_pData->m_pObjectShell->GetPreviewMetaFile( true );

            if (xMetaFile)
            {
                std::unique_ptr<SvMemoryStream> xStream(
                    GraphicHelper::getFormatStrFromGDI_Impl(
                        xMetaFile.get(), ConvertDataFormat::PNG ) );

                if (xStream)
                {
                    xStream->SetVersion( SOFFICE_FILEFORMAT_CURRENT );
                    aAny <<= Sequence< sal_Int8 >( static_cast< const sal_Int8* >( xStream->GetData() ),
                                                    xStream->TellEnd() );
                }
            }
        }
        else
            throw datatransfer::UnsupportedFlavorException();
    }

    return aAny;
}


// XTransferable


Sequence< datatransfer::DataFlavor > SAL_CALL SfxBaseModel::getTransferDataFlavors()
{
    SfxModelGuard aGuard( *this );

    const sal_Int32 nSuppFlavors = GraphicHelper::supportsMetaFileHandle_Impl() ? 10 : 8;
    Sequence< datatransfer::DataFlavor > aFlavorSeq( nSuppFlavors );
    auto pFlavorSeq = aFlavorSeq.getArray();

    pFlavorSeq[0].MimeType =
        "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"";
    pFlavorSeq[0].HumanPresentableName =  "GDIMetaFile";
    pFlavorSeq[0].DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();

    pFlavorSeq[1].MimeType =
        "application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\"";
    pFlavorSeq[1].HumanPresentableName = "GDIMetaFile";
    pFlavorSeq[1].DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();

    pFlavorSeq[2].MimeType =
        "application/x-openoffice-emf;windows_formatname=\"Image EMF\"" ;
    pFlavorSeq[2].HumanPresentableName = "Enhanced Windows MetaFile";
    pFlavorSeq[2].DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();

    pFlavorSeq[3].MimeType =
        "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"";
    pFlavorSeq[3].HumanPresentableName = "Windows MetaFile";
    pFlavorSeq[3].DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();

    pFlavorSeq[4].MimeType =
        "application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"";
    pFlavorSeq[4].HumanPresentableName = "Star Object Descriptor (XML)";
    pFlavorSeq[4].DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();

    pFlavorSeq[5].MimeType =
        "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
    pFlavorSeq[5].HumanPresentableName = "Star Embed Source (XML)";
    pFlavorSeq[5].DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();

    pFlavorSeq[6].MimeType =
        "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"";
    pFlavorSeq[6].HumanPresentableName = "Bitmap";
    pFlavorSeq[6].DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();

    pFlavorSeq[7].MimeType = "image/png";
    pFlavorSeq[7].HumanPresentableName = "PNG";
    pFlavorSeq[7].DataType = cppu::UnoType<Sequence< sal_Int8 >>::get();

    if ( nSuppFlavors == 10 )
    {
        pFlavorSeq[8].MimeType =
            "application/x-openoffice-emf;windows_formatname=\"Image EMF\"";
        pFlavorSeq[8].HumanPresentableName = "Enhanced Windows MetaFile";
        pFlavorSeq[8].DataType = cppu::UnoType<sal_uInt64>::get();

        pFlavorSeq[9].MimeType =
            "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"";
        pFlavorSeq[9].HumanPresentableName = "Windows MetaFile";
        pFlavorSeq[9].DataType = cppu::UnoType<sal_uInt64>::get();
    }

    return aFlavorSeq;
}


// XTransferable


sal_Bool SAL_CALL SfxBaseModel::isDataFlavorSupported( const datatransfer::DataFlavor& aFlavor )
{
    SfxModelGuard aGuard( *this );

    if ( aFlavor.MimeType == "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"" )
    {
        if ( aFlavor.DataType == cppu::UnoType<Sequence< sal_Int8 >>::get() )
            return true;
    }
    else if ( aFlavor.MimeType == "application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\"" )
    {
        if ( aFlavor.DataType == cppu::UnoType<Sequence< sal_Int8 >>::get() )
            return true;
    }
    else if ( aFlavor.MimeType == "application/x-openoffice-emf;windows_formatname=\"Image EMF\"" )
    {
        if ( aFlavor.DataType == cppu::UnoType<Sequence< sal_Int8 >>::get() )
            return true;
        else if ( GraphicHelper::supportsMetaFileHandle_Impl()
          && aFlavor.DataType == cppu::UnoType<sal_uInt64>::get())
            return true;
    }
    else if ( aFlavor.MimeType == "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"" )
    {
        if ( aFlavor.DataType == cppu::UnoType<Sequence< sal_Int8 >>::get() )
            return true;
        else if ( GraphicHelper::supportsMetaFileHandle_Impl()
          && aFlavor.DataType == cppu::UnoType<sal_uInt64>::get())
            return true;
    }
    else if ( aFlavor.MimeType == "application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"" )
    {
        if ( aFlavor.DataType == cppu::UnoType<Sequence< sal_Int8 >>::get() )
            return true;
    }
    else if ( aFlavor.MimeType == "application/x-openoffice-embed-source;windows_formatname=\"Star EMBS\"" )
    {
        if ( aFlavor.DataType == cppu::UnoType<Sequence< sal_Int8 >>::get() )
            return true;
    }
    else if ( aFlavor.MimeType == "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" )
    {
        if ( aFlavor.DataType == cppu::UnoType<Sequence< sal_Int8 >>::get() )
            return true;
    }
    else if ( aFlavor.MimeType == "image/png" )
    {
        if ( aFlavor.DataType == cppu::UnoType<Sequence< sal_Int8 >>::get() )
            return true;
    }

    return false;
}


//  XEventsSupplier


Reference< container::XNameReplace > SAL_CALL SfxBaseModel::getEvents()
{
    SfxModelGuard aGuard( *this );

    if ( ! m_pData->m_xEvents.is() )
    {
        m_pData->m_xEvents = new SfxEvents_Impl( m_pData->m_pObjectShell.get(), this );
    }

    return m_pData->m_xEvents;
}


//  XEmbeddedScripts


Reference< script::XStorageBasedLibraryContainer > SAL_CALL SfxBaseModel::getBasicLibraries()
{
    SfxModelGuard aGuard( *this );

    Reference< script::XStorageBasedLibraryContainer > xBasicLibraries;
    if ( m_pData->m_pObjectShell.is() )
        xBasicLibraries.set(m_pData->m_pObjectShell->GetBasicContainer(), UNO_QUERY);
    return xBasicLibraries;
}

Reference< script::XStorageBasedLibraryContainer > SAL_CALL SfxBaseModel::getDialogLibraries()
{
    SfxModelGuard aGuard( *this );

    Reference< script::XStorageBasedLibraryContainer > xDialogLibraries;
    if ( m_pData->m_pObjectShell.is() )
        xDialogLibraries.set(m_pData->m_pObjectShell->GetDialogContainer(), UNO_QUERY);
    return xDialogLibraries;
}

sal_Bool SAL_CALL SfxBaseModel::getAllowMacroExecution()
{
    SfxModelGuard aGuard( *this );

    if ( m_pData->m_pObjectShell.is() )
        return m_pData->m_pObjectShell->AdjustMacroMode();
    return false;
}


//  XScriptInvocationContext


Reference< document::XEmbeddedScripts > SAL_CALL SfxBaseModel::getScriptContainer()
{
    SfxModelGuard aGuard( *this );

    Reference< document::XEmbeddedScripts > xDocumentScripts;

    try
    {
        Reference< frame::XModel > xDocument( this );
        xDocumentScripts.set( xDocument, UNO_QUERY );
        while ( !xDocumentScripts.is() && xDocument.is() )
        {
            Reference< container::XChild > xDocAsChild( xDocument, UNO_QUERY );
            if ( !xDocAsChild.is() )
            {
                xDocument = nullptr;
                break;
            }

            xDocument.set( xDocAsChild->getParent(), UNO_QUERY );
            xDocumentScripts.set( xDocument, UNO_QUERY );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("sfx.doc");
        xDocumentScripts = nullptr;
    }

    return xDocumentScripts;
}


//  XEventBroadcaster


void SAL_CALL SfxBaseModel::addEventListener( const Reference< document::XEventListener >& aListener )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    m_pData->m_aDocumentEventListeners1.addInterface( aListener );
}


//  XEventBroadcaster


void SAL_CALL SfxBaseModel::removeEventListener( const Reference< document::XEventListener >& aListener )
{
    SfxModelGuard aGuard( *this );

    m_pData->m_aDocumentEventListeners1.removeInterface( aListener );
}

//  XShapeEventBroadcaster

void SAL_CALL SfxBaseModel::addShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape, const Reference< document::XShapeEventListener >& xListener )
{
    assert(xShape.is() && "no shape?");
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    m_pData->maShapeListeners[xShape].push_back(xListener);
}


//  XShapeEventBroadcaster


void SAL_CALL SfxBaseModel::removeShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape, const Reference< document::XShapeEventListener >& xListener )
{
    SfxModelGuard aGuard( *this );

    auto it = m_pData->maShapeListeners.find(xShape);
    if (it != m_pData->maShapeListeners.end())
    {
        auto rVec = it->second;
        auto it2 = std::find(rVec.begin(), rVec.end(), xListener);
        if (it2 != rVec.end())
        {
            rVec.erase(it2);
            if (rVec.empty())
                m_pData->maShapeListeners.erase(it);
        }
    }
}

//  XDocumentEventBroadcaster


void SAL_CALL SfxBaseModel::addDocumentEventListener( const Reference< document::XDocumentEventListener >& aListener )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    m_pData->m_aDocumentEventListeners2.addInterface( aListener );
}


void SAL_CALL SfxBaseModel::removeDocumentEventListener( const Reference< document::XDocumentEventListener >& aListener )
{
    SfxModelGuard aGuard( *this );
    m_pData->m_aDocumentEventListeners2.removeInterface( aListener );
}


void SAL_CALL SfxBaseModel::notifyDocumentEvent( const OUString&, const Reference< frame::XController2 >&, const Any& )
{
    throw lang::NoSupportException("SfxBaseModel controls all the sent notifications itself!" );
}

Sequence<document::CmisProperty> SAL_CALL SfxBaseModel::getCmisProperties()
{
    if (impl_isDisposed())
        return Sequence<document::CmisProperty>();
    return m_pData->m_cmisProperties;
}

void SAL_CALL SfxBaseModel::setCmisProperties( const Sequence< document::CmisProperty >& _cmisproperties )
{
    m_pData->m_cmisProperties = _cmisproperties;
}

void SAL_CALL SfxBaseModel::updateCmisProperties( const Sequence< document::CmisProperty >& aProperties )
{
    SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
    if ( !pMedium )
        return;

    try
    {
        ::ucbhelper::Content aContent( pMedium->GetName( ),
            Reference<ucb::XCommandEnvironment>(),
            comphelper::getProcessComponentContext() );

        aContent.executeCommand( "updateProperties", uno::Any( aProperties ) );
        loadCmisProperties( );
    }
    catch (const Exception & e)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException( e.Message,
                        e.Context, anyEx );
    }

}

void SAL_CALL SfxBaseModel::checkOut(  )
{
    SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
    if ( !pMedium )
        return;

    try
    {
        ::ucbhelper::Content aContent( pMedium->GetName(),
            Reference<ucb::XCommandEnvironment>(),
            comphelper::getProcessComponentContext() );

        Any aResult = aContent.executeCommand( "checkout", Any( ) );
        OUString sURL;
        aResult >>= sURL;

        m_pData->m_pObjectShell->GetMedium( )->SetName( sURL );
        m_pData->m_pObjectShell->GetMedium( )->GetMedium_Impl( );
        m_pData->m_xDocumentProperties->setTitle( getTitle( ) );
        Sequence< beans::PropertyValue > aSequence ;
        TransformItems( SID_OPENDOC, pMedium->GetItemSet(), aSequence );
        attachResource( sURL, aSequence );

        // Reload the CMIS properties
        loadCmisProperties( );
    }
    catch ( const Exception & e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException( e.Message,
                        e.Context, anyEx );
    }
}

void SAL_CALL SfxBaseModel::cancelCheckOut(  )
{
    SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
    if ( !pMedium )
        return;

    try
    {
        ::ucbhelper::Content aContent( pMedium->GetName(),
            Reference<ucb::XCommandEnvironment>(),
            comphelper::getProcessComponentContext() );

        Any aResult = aContent.executeCommand( "cancelCheckout", Any( ) );
        OUString sURL;
        aResult >>= sURL;

        m_pData->m_pObjectShell->GetMedium( )->SetName( sURL );
    }
    catch ( const Exception & e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException( e.Message,
                        e.Context, anyEx );
    }
}

void SAL_CALL SfxBaseModel::checkIn( sal_Bool bIsMajor, const OUString& rMessage )
{
    SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
    if ( !pMedium )
        return;

    try
    {
        Sequence< beans::PropertyValue > aProps{
            comphelper::makePropertyValue("VersionMajor", bIsMajor),
            comphelper::makePropertyValue("VersionComment", rMessage),
            comphelper::makePropertyValue("CheckIn", true)
        };

        const OUString sName( pMedium->GetName( ) );
        storeSelf( aProps );

        // Refresh pMedium as it has probably changed during the storeSelf call
        pMedium = m_pData->m_pObjectShell->GetMedium( );
        const OUString sNewName( pMedium->GetName( ) );

        // URL has changed, update the document
        if ( sName != sNewName )
        {
            m_pData->m_xDocumentProperties->setTitle( getTitle( ) );
            Sequence< beans::PropertyValue > aSequence ;
            TransformItems( SID_OPENDOC, pMedium->GetItemSet(), aSequence );
            attachResource( sNewName, aSequence );

            // Reload the CMIS properties
            loadCmisProperties( );
        }
    }
    catch ( const Exception & e )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException( e.Message,
                        e.Context, anyEx );
    }
}

uno::Sequence< document::CmisVersion > SAL_CALL SfxBaseModel::getAllVersions( )
{
    uno::Sequence<document::CmisVersion> aVersions;
    if (impl_isDisposed())
        return aVersions;
    SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
    if ( pMedium )
    {
        try
        {
            ::ucbhelper::Content aContent( pMedium->GetName(),
                Reference<ucb::XCommandEnvironment>(),
                comphelper::getProcessComponentContext() );

            Any aResult = aContent.executeCommand( "getAllVersions", Any( ) );
            aResult >>= aVersions;
        }
        catch ( const Exception & e )
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw lang::WrappedTargetRuntimeException( e.Message,
                            e.Context, anyEx );
        }
    }
    return aVersions;
}

bool SfxBaseModel::getBoolPropertyValue( const OUString& rName )
{
    bool bValue = false;
    if ( m_pData->m_pObjectShell.is() )
    {
        SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
        if ( pMedium )
        {
            try
            {
                ::ucbhelper::Content aContent( pMedium->GetName( ),
                    utl::UCBContentHelper::getDefaultCommandEnvironment(),
                    comphelper::getProcessComponentContext() );
                Reference < beans::XPropertySetInfo > xProps = aContent.getProperties();
                if ( xProps->hasPropertyByName( rName ) )
                {
                    aContent.getPropertyValue( rName ) >>= bValue;
                }
            }
            catch ( const Exception & )
            {
                // Simply ignore it: it's likely the document isn't versionable in that case
                bValue = false;
            }
        }
    }
    return bValue;
}

sal_Bool SAL_CALL SfxBaseModel::isVersionable( )
{
    return getBoolPropertyValue( "IsVersionable" );
}

sal_Bool SAL_CALL SfxBaseModel::canCheckOut( )
{
    return getBoolPropertyValue( "CanCheckOut" );
}

sal_Bool SAL_CALL SfxBaseModel::canCancelCheckOut( )
{
    return getBoolPropertyValue( "CanCancelCheckOut" );
}

sal_Bool SAL_CALL SfxBaseModel::canCheckIn( )
{
    return getBoolPropertyValue( "CanCheckIn" );
}

void SfxBaseModel::loadCmisProperties( )
{
    SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
    if ( !pMedium )
        return;

    try
    {
        ::ucbhelper::Content aContent( pMedium->GetName( ),
            utl::UCBContentHelper::getDefaultCommandEnvironment(),
            comphelper::getProcessComponentContext() );
        Reference < beans::XPropertySetInfo > xProps = aContent.getProperties();
        static constexpr OUString aCmisProps( u"CmisProperties"_ustr );
        if ( xProps->hasPropertyByName( aCmisProps ) )
        {
            Sequence< document::CmisProperty> aCmisProperties;
            aContent.getPropertyValue( aCmisProps ) >>= aCmisProperties;
            setCmisProperties( aCmisProperties );
        }
    }
    catch (const ucb::ContentCreationException &)
    {
    }
    catch (const ucb::CommandAbortedException &)
    {
    }
}

SfxMedium* SfxBaseModel::handleLoadError( const ErrCodeMsg& rError, SfxMedium* pMedium )
{
    if (!rError)
    {
        // No error condition.
        return pMedium;
    }

    ErrCodeMsg nError = rError;
    bool bSilent = false;
    const SfxBoolItem* pSilentItem = pMedium->GetItemSet().GetItem(SID_SILENT, false);
    if( pSilentItem )
        bSilent = pSilentItem->GetValue();

    bool bWarning = nError.IsWarning();
    if ( nError != ERRCODE_IO_BROKENPACKAGE && !bSilent )
    {
        // broken package was handled already
        if ( SfxObjectShell::UseInteractionToHandleError(pMedium->GetInteractionHandler(), nError) && !bWarning)
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
        pMedium = nullptr;
    }

    if ( !bWarning )    // #i30711# don't abort loading if it's only a warning
    {
        nError = nError ? nError : ERRCODE_IO_CANTREAD;
        throw task::ErrorCodeIOException(
            "SfxBaseModel::handleLoadError: 0x" + nError.toString(),
            Reference< XInterface >(), sal_uInt32(nError.GetCode()));
    }
    else
        pMedium->SetWarningError(nError);

    return pMedium;
}


//  SfxListener


static void addTitle_Impl( Sequence < beans::PropertyValue >& rSeq, const OUString& rTitle )
{
    auto [begin, end] = asNonConstRange(rSeq);
    auto pProp = std::find_if(begin, end,
        [](const beans::PropertyValue& rProp) { return rProp.Name == "Title"; });
    if (pProp != end)
    {
        pProp->Value <<= rTitle;
    }
    else
    {
        sal_Int32 nCount = rSeq.getLength();
        rSeq.realloc( nCount+1 );
        auto& el = rSeq.getArray()[nCount];
        el.Name = "Title";
        el.Value <<= rTitle;
    }
}

void SfxBaseModel::Notify(          SfxBroadcaster& rBC     ,
                             const  SfxHint&        rHint   )
{
    if ( !m_pData )
        return;

    if ( &rBC != m_pData->m_pObjectShell.get() )
        return;

    if ( rHint.GetId() == SfxHintId::DocChanged )
        changing();
    else if (rHint.GetId() == SfxHintId::ThisIsAnSfxEventHint)
    {
        const SfxEventHint& rNamedHint = static_cast<const SfxEventHint&>(rHint);
        switch (rNamedHint.GetEventId())
        {
        case SfxEventHintId::StorageChanged:
        {
            if ( m_pData->m_xUIConfigurationManager.is()
              && m_pData->m_pObjectShell->GetCreateMode() != SfxObjectCreateMode::EMBEDDED )
            {
                Reference< embed::XStorage > xConfigStorage;
                static constexpr OUString aUIConfigFolderName( u"Configurations2"_ustr );

                xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, embed::ElementModes::READWRITE );
                if ( !xConfigStorage.is() )
                    xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, embed::ElementModes::READ );

                if ( xConfigStorage.is() || !m_pData->m_pObjectShell->GetStorage()->hasByName( aUIConfigFolderName ) )
                {
                    // the storage is different, since otherwise it could not be opened, so it must be exchanged
                    m_pData->m_xUIConfigurationManager->setStorage( xConfigStorage );
                }
                else
                {
                    OSL_FAIL( "Unexpected scenario!" );
                }
            }

            ListenForStorage_Impl( m_pData->m_pObjectShell->GetStorage() );
        }
        break;

        case SfxEventHintId::LoadFinished:
        {
            impl_getPrintHelper();
            ListenForStorage_Impl( m_pData->m_pObjectShell->GetStorage() );
            m_pData->setModifiedForAutoSave(false);
        }
        break;

        case SfxEventHintId::SaveAsDocDone:
        {
            m_pData->m_sURL = m_pData->m_pObjectShell->GetMedium()->GetName();

            Sequence< beans::PropertyValue > aArgs;
            TransformItems( SID_SAVEASDOC, m_pData->m_pObjectShell->GetMedium()->GetItemSet(), aArgs );
            addTitle_Impl( aArgs, m_pData->m_pObjectShell->GetTitle() );
            attachResource( m_pData->m_pObjectShell->GetMedium()->GetName(), aArgs );
        }
        break;

        case SfxEventHintId::DocCreated:
        {
            impl_getPrintHelper();
            m_pData->setModifiedForAutoSave(false);
        }
        break;

        case SfxEventHintId::ModifyChanged:
        {
            m_pData->setModifiedForAutoSave(isModified());
        }
        break;
        default: break;
        }

        Any aSupplement;
        if (const SfxPrintingHint* pPrintingHint = dynamic_cast<const SfxPrintingHint*>(&rHint))
            aSupplement <<= pPrintingHint->GetWhich();
        const SfxViewEventHint* pViewHint = dynamic_cast<const SfxViewEventHint*>(&rHint);
        postEvent_Impl( rNamedHint.GetEventName(), pViewHint ? pViewHint->GetController() : Reference< frame::XController2 >(), aSupplement );
    }
    else if ( rHint.GetId() == SfxHintId::TitleChanged )
    {
        addTitle_Impl( m_pData->m_seqArguments, m_pData->m_pObjectShell->GetTitle() );
        postEvent_Impl( GlobalEventConfig::GetEventName( GlobalEventId::TITLECHANGED ) );
    }
    else if ( rHint.GetId() == SfxHintId::ModeChanged )
    {
        postEvent_Impl( GlobalEventConfig::GetEventName( GlobalEventId::MODECHANGED ) );
    }
}


//  public impl.


void SfxBaseModel::NotifyModifyListeners_Impl() const
{
    if ( m_pData->m_aModifyListeners.getLength() )
    {
        lang::EventObject aEvent( static_cast<frame::XModel *>(const_cast<SfxBaseModel *>(this)) );
        m_pData->m_aModifyListeners.notifyEach( &util::XModifyListener::modified, aEvent );
    }

    // this notification here is done too generously, we cannot simply assume that we're really modified
    // now, but we need to check it ...
    m_pData->setModifiedForAutoSave(const_cast<SfxBaseModel*>(this)->isModified());
}

void SfxBaseModel::changing()
{
    SfxModelGuard aGuard( *this );

    // the notification should not be sent if the document can not be modified
    if ( !m_pData->m_pObjectShell.is() || !m_pData->m_pObjectShell->IsEnableSetModified() )
        return;

    NotifyModifyListeners_Impl();
}


//  public impl.


SfxObjectShell* SfxBaseModel::GetObjectShell() const
{
    return m_pData ? m_pData->m_pObjectShell.get() : nullptr;
}


//  public impl.


bool SfxBaseModel::IsInitialized() const
{
    if ( !m_pData || !m_pData->m_pObjectShell.is() )
    {
        OSL_FAIL( "SfxBaseModel::IsInitialized: this should have been caught earlier!" );
        return false;
    }

    return m_pData->m_pObjectShell->GetMedium() != nullptr;
}

void SfxBaseModel::MethodEntryCheck( const bool i_mustBeInitialized ) const
{
    if ( impl_isDisposed() )
        throw lang::DisposedException( OUString(), *const_cast< SfxBaseModel* >( this ) );
    if ( i_mustBeInitialized && !IsInitialized() )
        throw lang::NotInitializedException( OUString(), *const_cast< SfxBaseModel* >( this ) );
}

bool SfxBaseModel::impl_isDisposed() const
{
    return ( m_pData == nullptr ) ;
}


//  private impl.


OUString SfxBaseModel::GetMediumFilterName_Impl() const
{
    std::shared_ptr<const SfxFilter> pFilter;
    SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
    if ( pMedium )
        pFilter = pMedium->GetFilter();

    if ( pFilter )
        return pFilter->GetName();

    return OUString();
}

void SfxBaseModel::impl_store(  const   OUString&                   sURL            ,
                                const   Sequence< beans::PropertyValue >&  seqArguments    ,
                                        bool                        bSaveTo         )
{
    if( sURL.isEmpty() )
        throw frame::IllegalArgumentIOException();

    if (!m_pData->m_pObjectShell)
        return;

    ::comphelper::SequenceAsHashMap aArgHash(seqArguments);
    if ( !bSaveTo && !sURL.isEmpty()
      && !sURL.startsWith( "private:stream" )
      && ::utl::UCBContentHelper::EqualURLs( getLocation(), sURL ) )
    {
        // this is the same file URL as the current document location, try to use storeOwn if possible

        static constexpr OUString aFilterString( u"FilterName"_ustr  );
        const OUString aFilterName( aArgHash.getUnpackedValueOrDefault( aFilterString, OUString() ) );
        if ( !aFilterName.isEmpty() )
        {
            SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
            if ( pMedium )
            {
                const std::shared_ptr<const SfxFilter>& pFilter = pMedium->GetFilter();
                if ( pFilter && aFilterName == pFilter->GetFilterName() )
                {
                    // #i119366# - If the former file saving with password, do not trying in StoreSelf anyway...
                    bool bFormerPassword = false;
                    {
                        uno::Sequence< beans::NamedValue > aOldEncryptionData;
                        if (GetEncryptionData_Impl( &pMedium->GetItemSet(), aOldEncryptionData ))
                        {
                            bFormerPassword = true;
                        }
                    }
                    if ( !bFormerPassword )
                    {
                        aArgHash.erase( aFilterString );
                        aArgHash.erase( "URL" );

                        try
                        {
                            storeSelf( aArgHash.getAsConstPropertyValueList() );
                            return;
                        }
                        catch( const lang::IllegalArgumentException& )
                        {
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
                            // some additional arguments do not allow to use saving, SaveAs should be done
                            // but only for normal documents, the shared documents would be overwritten in this case
                            // that would mean an information loss
                            // TODO/LATER: need a new interaction for this case
                            if ( m_pData->m_pObjectShell->IsDocShared() )
                            {
                                uno::Sequence< beans::NamedValue > aNewEncryptionData = aArgHash.getUnpackedValueOrDefault("EncryptionData", uno::Sequence< beans::NamedValue >() );
                                if ( !aNewEncryptionData.hasElements() )
                                {
                                    aNewEncryptionData = ::comphelper::OStorageHelper::CreatePackageEncryptionData( aArgHash.getUnpackedValueOrDefault("Password", OUString()) );
                                }

                                uno::Sequence< beans::NamedValue > aOldEncryptionData;
                                (void)GetEncryptionData_Impl( &pMedium->GetItemSet(), aOldEncryptionData );

                                if ( !aOldEncryptionData.hasElements() && !aNewEncryptionData.hasElements() )
                                    throw;
                                else
                                {
                                    // if the password is changed a special error should be used in case of shared document
                                    throw task::ErrorCodeIOException("Can not change password for shared document.", uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_SFX_SHARED_NOPASSWORDCHANGE) );
                                }
                            }
#endif
                        }
                    }
                }
            }
        }
    }

    SfxGetpApp()->NotifyEvent( SfxEventHint( bSaveTo ? SfxEventHintId::SaveToDoc : SfxEventHintId::SaveAsDoc, GlobalEventConfig::GetEventName( bSaveTo ? GlobalEventId::SAVETODOC : GlobalEventId::SAVEASDOC ),
                                            m_pData->m_pObjectShell.get() ) );

    const OUString aFilterName(aArgHash.getUnpackedValueOrDefault("FilterName", OUString()));
    OUString aPassword, aPasswordToModify;
    if (!aArgHash.getUnpackedValueOrDefault("EncryptionData", Sequence<beans::NamedValue>())
             .hasElements())
        aPassword = aArgHash.getUnpackedValueOrDefault("Password", OUString());
    if (!aArgHash.getUnpackedValueOrDefault("ModifyPasswordInfo", Sequence<beans::PropertyValue>())
             .hasElements()
        && aArgHash.getUnpackedValueOrDefault("ModifyPasswordInfo", static_cast<sal_Int32>(0)) == 0)
        aPasswordToModify = aArgHash.getUnpackedValueOrDefault("PasswordToModify", OUString());
    aArgHash.erase("PasswordToModify");

    std::optional<SfxAllItemSet> pItemSet(SfxGetpApp()->GetPool());
    pItemSet->Put(SfxStringItem(SID_FILE_NAME, sURL));
    if ( bSaveTo )
        pItemSet->Put(SfxBoolItem(SID_SAVETO, true));

    if (!aFilterName.isEmpty() && (!aPassword.isEmpty() || !aPasswordToModify.isEmpty()))
        sfx2::SetPassword(SfxGetpApp()->GetFilterMatcher().GetFilter4FilterName(aFilterName),
                          &*pItemSet, aPassword, aPasswordToModify, false);

    TransformParameters(SID_SAVEASDOC, seqArguments, *pItemSet);

    const SfxBoolItem* pCopyStreamItem = pItemSet->GetItem<SfxBoolItem>(SID_COPY_STREAM_IF_POSSIBLE, false);

    if ( pCopyStreamItem && pCopyStreamItem->GetValue() && !bSaveTo )
    {
        throw frame::IllegalArgumentIOException(
                "CopyStreamIfPossible parameter is not acceptable for storeAsURL() call!" );
    }

    sal_uInt32 nModifyPasswordHash = 0;
    Sequence< beans::PropertyValue > aModifyPasswordInfo;
    const SfxUnoAnyItem* pModifyPasswordInfoItem = pItemSet->GetItem<SfxUnoAnyItem>(SID_MODIFYPASSWORDINFO, false);
    if ( pModifyPasswordInfoItem )
    {
        // it contains either a simple hash or a set of PropertyValues
        // TODO/LATER: the sequence of PropertyValue should replace the hash completely in future
        sal_Int32 nMPHTmp = 0;
        pModifyPasswordInfoItem->GetValue() >>= nMPHTmp;
        nModifyPasswordHash = static_cast<sal_uInt32>(nMPHTmp);
        pModifyPasswordInfoItem->GetValue() >>= aModifyPasswordInfo;
    }
    pItemSet->ClearItem(SID_MODIFYPASSWORDINFO);
    sal_uInt32 nOldModifyPasswordHash = m_pData->m_pObjectShell->GetModifyPasswordHash();
    m_pData->m_pObjectShell->SetModifyPasswordHash( nModifyPasswordHash );
    Sequence< beans::PropertyValue > aOldModifyPasswordInfo = m_pData->m_pObjectShell->GetModifyPasswordInfo();
    m_pData->m_pObjectShell->SetModifyPasswordInfo( aModifyPasswordInfo );

    // since saving a document modifies its DocumentProperties, the current
    // DocumentProperties must be saved on "SaveTo", so it can be restored
    // after saving
    bool bCopyTo =  bSaveTo ||
        m_pData->m_pObjectShell->GetCreateMode() == SfxObjectCreateMode::EMBEDDED;
    Reference<document::XDocumentProperties> xOldDocProps;
    if ( bCopyTo )
    {
        xOldDocProps = getDocumentProperties();
        const Reference<util::XCloneable> xCloneable(xOldDocProps,
            UNO_QUERY_THROW);
        const Reference<document::XDocumentProperties> xNewDocProps(
            xCloneable->createClone(), UNO_QUERY_THROW);
        m_pData->m_xDocumentProperties = xNewDocProps;
    }

    bool bRet = m_pData->m_pObjectShell->APISaveAs_Impl(sURL, *pItemSet, seqArguments);

    if ( bCopyTo )
    {
        // restore DocumentProperties if a copy was created
        m_pData->m_xDocumentProperties = xOldDocProps;
    }

    Reference < task::XInteractionHandler > xHandler;
    const SfxUnoAnyItem* pItem = pItemSet->GetItem<SfxUnoAnyItem>(SID_INTERACTIONHANDLER, false);
    if ( pItem )
        pItem->GetValue() >>= xHandler;

    pItemSet.reset();

    ErrCodeMsg nErrCode = m_pData->m_pObjectShell->GetErrorCode();
    if ( !bRet && !nErrCode )
    {
        SAL_WARN("sfx.doc", "Storing has failed, no error is set!");
        nErrCode = ERRCODE_IO_CANTWRITE;
    }
    m_pData->m_pObjectShell->ResetError();

    if ( bRet )
    {
        if ( nErrCode )
        {
            // must be a warning - use Interactionhandler if possible or abandon
            if ( xHandler.is() )
            {
                // TODO/LATER: a general way to set the error context should be available
                SfxErrorContext aEc( ERRCTX_SFX_SAVEASDOC, m_pData->m_pObjectShell->GetTitle() );

                task::ErrorCodeRequest2 aErrorCode(OUString(), uno::Reference<XInterface>(),
                    sal_Int32(sal_uInt32(nErrCode.GetCode())), nErrCode.GetArg1(), nErrCode.GetArg2(),
                    static_cast<sal_Int16>(nErrCode.GetDialogMask()));
                SfxMedium::CallApproveHandler( xHandler, Any( aErrorCode ), false );
            }
        }

        if ( !bSaveTo )
        {
            m_pData->m_aPreusedFilterName = GetMediumFilterName_Impl();
            m_pData->m_pObjectShell->SetModifyPasswordEntered();

            SfxGetpApp()->NotifyEvent( SfxEventHint( SfxEventHintId::SaveAsDocDone, GlobalEventConfig::GetEventName(GlobalEventId::SAVEASDOCDONE), m_pData->m_pObjectShell.get() ) );
        }
        else
        {
            m_pData->m_pObjectShell->SetModifyPasswordHash( nOldModifyPasswordHash );
            m_pData->m_pObjectShell->SetModifyPasswordInfo( aOldModifyPasswordInfo );

            SfxGetpApp()->NotifyEvent( SfxEventHint( SfxEventHintId::SaveToDocDone, GlobalEventConfig::GetEventName(GlobalEventId::SAVETODOCDONE), m_pData->m_pObjectShell.get() ) );
        }
    }
    else
    {
        m_pData->m_pObjectShell->SetModifyPasswordHash( nOldModifyPasswordHash );
        m_pData->m_pObjectShell->SetModifyPasswordInfo( aOldModifyPasswordInfo );


        SfxGetpApp()->NotifyEvent( SfxEventHint( bSaveTo ? SfxEventHintId::SaveToDocFailed : SfxEventHintId::SaveAsDocFailed, GlobalEventConfig::GetEventName( bSaveTo ? GlobalEventId::SAVETODOCFAILED : GlobalEventId::SAVEASDOCFAILED),
                                                m_pData->m_pObjectShell.get() ) );

        if ( comphelper::LibreOfficeKit::isActive() && SfxViewShell::Current() )
            SfxViewShell::Current()->libreOfficeKitViewCallback( LOK_CALLBACK_EXPORT_FILE, "ERROR"_ostr );

        std::stringstream aErrCode;
        aErrCode << nErrCode;
        throw task::ErrorCodeIOException(
            "SfxBaseModel::impl_store <" + sURL + "> failed: " + OUString::fromUtf8(aErrCode.str()),
            Reference< XInterface >(), sal_uInt32(nErrCode.GetCode()));
    }
}


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

    void operator()( const Reference<ListenerT>& listener ) const
    {
        try
        {
            (listener.get()->*m_pMethod)( m_rEvent );
        }
        catch( RuntimeException& )
        {
            // this exception is ignored to avoid problems with invalid listeners, the listener should be probably thrown away in future
        }
    }
};
} // anonymous namespace

void SfxBaseModel::postEvent_Impl( const OUString& aName, const Reference< frame::XController2 >& xController, const Any& supplement )
{
    // object already disposed?
    if ( impl_isDisposed() )
        return;

    // keep m_pData alive, if notified target would dispose the document
    std::shared_ptr<IMPL_SfxBaseModel_DataContainer> xKeepAlive(m_pData);

    // also make sure this object doesn't self-destruct while notifying
    rtl::Reference<SfxBaseModel> xHoldAlive(this);

    DBG_ASSERT( !aName.isEmpty(), "Empty event name!" );
    if (aName.isEmpty())
        return;

    if ( xKeepAlive->m_aDocumentEventListeners2.getLength() )
    {
        SAL_INFO("sfx.doc", "SfxDocumentEvent: " + aName);

        document::DocumentEvent aDocumentEvent( static_cast<frame::XModel*>(this), aName, xController, supplement );

        xKeepAlive->m_aDocumentEventListeners2.forEach(
            NotifySingleListenerIgnoreRE< document::XDocumentEventListener, document::DocumentEvent >(
                &document::XDocumentEventListener::documentEventOccured,
                aDocumentEvent ) );
    }

    if ( xKeepAlive->m_aDocumentEventListeners1.getLength() )
    {
        SAL_INFO("sfx.doc", "SfxEvent: " + aName);

        document::EventObject aEvent( static_cast<frame::XModel*>(this), aName );

        xKeepAlive->m_aDocumentEventListeners1.forEach(
            NotifySingleListenerIgnoreRE< document::XEventListener, document::EventObject >(
                &document::XEventListener::notifyEvent,
                aEvent ) );
    }

}

Reference < container::XIndexAccess > SAL_CALL SfxBaseModel::getViewData()
{
    SfxModelGuard aGuard( *this );

    if ( m_pData->m_pObjectShell.is() && !m_pData->m_contViewData.is() )
    {
        SfxViewFrame *pActFrame = SfxViewFrame::Current();
        if ( !pActFrame || pActFrame->GetObjectShell() != m_pData->m_pObjectShell.get() )
            pActFrame = SfxViewFrame::GetFirst( m_pData->m_pObjectShell.get() );

        if ( !pActFrame || !pActFrame->GetViewShell() )
            // currently no frame for this document at all or View is under construction
            return Reference < container::XIndexAccess >();

        m_pData->m_contViewData = new comphelper::IndexedPropertyValuesContainer();

        if ( !m_pData->m_contViewData.is() )
        {
            // error: no container class available!
            return Reference < container::XIndexAccess >();
        }

        Reference < container::XIndexContainer > xCont( m_pData->m_contViewData, UNO_QUERY );
        sal_Int32 nCount = 0;
        Sequence < beans::PropertyValue > aSeq;
        for ( SfxViewFrame *pFrame = SfxViewFrame::GetFirst( m_pData->m_pObjectShell.get() ); pFrame;
                pFrame = SfxViewFrame::GetNext( *pFrame, m_pData->m_pObjectShell.get() ) )
        {
            bool bIsActive = ( pFrame == pActFrame );
            pFrame->GetViewShell()->WriteUserDataSequence( aSeq );
            xCont->insertByIndex( bIsActive ? 0 : nCount, Any(aSeq) );
            nCount++;
        }
    }

    return m_pData->m_contViewData;
}

void SAL_CALL SfxBaseModel::setViewData( const Reference < container::XIndexAccess >& aData )
{
    SfxModelGuard aGuard( *this );

    m_pData->m_contViewData = aData;
}

/** calls all XEventListeners */
void SfxBaseModel::notifyEvent( const document::EventObject& aEvent ) const
{
    // object already disposed?
    if ( impl_isDisposed() )
        return;

    if( !m_pData->m_aDocumentEventListeners1.getLength() )

        return;

    comphelper::OInterfaceIteratorHelper3 aIt( m_pData->m_aDocumentEventListeners1 );
    while( aIt.hasMoreElements() )
    {
        try
        {
            aIt.next()->notifyEvent( aEvent );
        }
        catch( RuntimeException& )
        {
            aIt.remove();
        }
    }
    // for right now, we're only doing the event that this particular performance problem needed
    if (aEvent.EventName == "ShapeModified")
    {
        uno::Reference<drawing::XShape> xShape(aEvent.Source, uno::UNO_QUERY);
        if (xShape.is())
        {
            auto it = m_pData->maShapeListeners.find(xShape);
            if (it != m_pData->maShapeListeners.end())
                for (auto const & rListenerUnoRef : it->second)
                    rListenerUnoRef->notifyShapeEvent(aEvent);
        }
    }
}

/** returns true if someone added a XEventListener to this XEventBroadcaster */
bool SfxBaseModel::hasEventListeners() const
{
    return !impl_isDisposed()
        && ( m_pData->m_aDocumentEventListeners1.getLength() != 0
             || !m_pData->maShapeListeners.empty());
}

void SAL_CALL SfxBaseModel::addPrintJobListener( const Reference< view::XPrintJobListener >& xListener )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    impl_getPrintHelper();
    Reference < view::XPrintJobBroadcaster > xPJB( m_pData->m_xPrintable, UNO_QUERY );
    if ( xPJB.is() )
        xPJB->addPrintJobListener( xListener );
}

void SAL_CALL SfxBaseModel::removePrintJobListener( const Reference< view::XPrintJobListener >& xListener )
{
    SfxModelGuard aGuard( *this );

    impl_getPrintHelper();
    Reference < view::XPrintJobBroadcaster > xPJB( m_pData->m_xPrintable, UNO_QUERY );
    if ( xPJB.is() )
        xPJB->removePrintJobListener( xListener );
}

sal_Int64 SAL_CALL SfxBaseModel::getSomething( const Sequence< sal_Int8 >& aIdentifier )
{
    SvGlobalName aName( aIdentifier );
    if (aName == SvGlobalName( SFX_GLOBAL_CLASSID ))
    {
        SolarMutexGuard aGuard;
        SfxObjectShell *const pObjectShell(GetObjectShell());
        if (pObjectShell)
        {
            return comphelper::getSomething_cast(pObjectShell);
        }
    }

    return 0;
}


//  XDocumentSubStorageSupplier


void SfxBaseModel::ListenForStorage_Impl( const Reference< embed::XStorage >& xStorage )
{
    Reference< util::XModifiable > xModifiable( xStorage, UNO_QUERY );
    if ( xModifiable.is() )
    {
        if ( !m_pData->m_pStorageModifyListen.is() )
        {
            m_pData->m_pStorageModifyListen = new ::sfx2::DocumentStorageModifyListener( *m_pData, Application::GetSolarMutex() );
        }

        // no need to deregister the listening for old storage since it should be disposed automatically
        xModifiable->addModifyListener( m_pData->m_pStorageModifyListen );
    }
}

Reference< embed::XStorage > SAL_CALL SfxBaseModel::getDocumentSubStorage( const OUString& aStorageName, sal_Int32 nMode )
{
    SfxModelGuard aGuard( *this );

    Reference< embed::XStorage > xResult;
    if ( m_pData->m_pObjectShell.is() )
    {
        Reference< embed::XStorage > xStorage = m_pData->m_pObjectShell->GetStorage();
        if ( xStorage.is() )
        {
            try
            {
                xResult = xStorage->openStorageElement( aStorageName, nMode );
            }
            catch ( Exception& )
            {
            }
        }
    }

    return xResult;
}

Sequence< OUString > SAL_CALL SfxBaseModel::getDocumentSubStoragesNames()
{
    SfxModelGuard aGuard( *this );

    Sequence< OUString > aResult;
    bool bSuccess = false;
    if ( m_pData->m_pObjectShell.is() )
    {
        Reference < embed::XStorage > xStorage = m_pData->m_pObjectShell->GetStorage();
        if ( xStorage.is() )
        {
            const Sequence< OUString > aTemp = xStorage->getElementNames();
            sal_Int32 nResultSize = 0;
            for ( const auto& rName : aTemp )
            {
                if ( xStorage->isStorageElement( rName ) )
                {
                    aResult.realloc( ++nResultSize );
                    aResult.getArray()[ nResultSize - 1 ] = rName;
                }
            }

            bSuccess = true;
        }
    }

    if ( !bSuccess )
        throw io::IOException();

    return aResult;
}


//  XScriptProviderSupplier


Reference< script::provider::XScriptProvider > SAL_CALL SfxBaseModel::getScriptProvider()
{
    SfxModelGuard aGuard( *this );

    Reference< script::provider::XScriptProviderFactory > xScriptProviderFactory =
        script::provider::theMasterScriptProviderFactory::get( ::comphelper::getProcessComponentContext() );

    Reference< XScriptInvocationContext > xScriptContext( this );

    Reference< script::provider::XScriptProvider > xScriptProvider(
        xScriptProviderFactory->createScriptProvider( Any( xScriptContext ) ),
        UNO_SET_THROW );

    return xScriptProvider;
}


//  XUIConfigurationManagerSupplier


OUString const & SfxBaseModel::getRuntimeUID() const
{
    OSL_ENSURE( !m_pData->m_sRuntimeUID.isEmpty(),
                "SfxBaseModel::getRuntimeUID - ID is empty!" );
    return m_pData->m_sRuntimeUID;
}

bool SfxBaseModel::hasValidSignatures() const
{
    SolarMutexGuard aGuard;
    if ( m_pData->m_pObjectShell.is() )
        return ( m_pData->m_pObjectShell->ImplGetSignatureState() == SignatureState::OK );
    return false;
}

void SfxBaseModel::getGrabBagItem(css::uno::Any& rVal) const
{
    if (m_pData->m_xGrabBagItem)
        m_pData->m_xGrabBagItem->QueryValue(rVal);
    else
        rVal <<= uno::Sequence<beans::PropertyValue>();
}

void SfxBaseModel::setGrabBagItem(const css::uno::Any& rVal)
{
    if (!m_pData->m_xGrabBagItem)
        m_pData->m_xGrabBagItem = std::make_shared<SfxGrabBagItem>();

    m_pData->m_xGrabBagItem->PutValue(rVal, 0);
}

static void GetCommandFromSequence( OUString& rCommand, sal_Int32& nIndex, const Sequence< beans::PropertyValue >& rSeqPropValue )
{
    nIndex = -1;

    auto pPropValue = std::find_if(rSeqPropValue.begin(), rSeqPropValue.end(),
        [](const beans::PropertyValue& rPropValue) { return rPropValue.Name == "Command"; });
    if (pPropValue != rSeqPropValue.end())
    {
        pPropValue->Value >>= rCommand;
        nIndex = static_cast<sal_Int32>(std::distance(rSeqPropValue.begin(), pPropValue));
    }
}

static void ConvertSlotsToCommands( SfxObjectShell const * pDoc, Reference< container::XIndexContainer > const & rToolbarDefinition )
{
    if ( !pDoc )
        return;

    SfxModule*    pModule( pDoc->GetFactory().GetModule() );
    Sequence< beans::PropertyValue > aSeqPropValue;

    for ( sal_Int32 i = 0; i < rToolbarDefinition->getCount(); i++ )
    {
        if ( rToolbarDefinition->getByIndex( i ) >>= aSeqPropValue )
        {
            OUString aCommand;
            sal_Int32 nIndex( -1 );
            GetCommandFromSequence( aCommand, nIndex, aSeqPropValue );
            if ( nIndex >= 0 && aCommand.startsWith( "slot:" ) )
            {
                const sal_uInt16 nSlot = o3tl::toInt32(aCommand.subView( 5 ));

                // We have to replace the old "slot-Command" with our new ".uno:-Command"
                const SfxSlot* pSlot = pModule->GetSlotPool()->GetSlot( nSlot );
                if ( pSlot )
                {
                    aCommand = pSlot->GetCommand();
                    aSeqPropValue.getArray()[nIndex].Value <<= aCommand;
                    rToolbarDefinition->replaceByIndex( i, Any( aSeqPropValue ));
                }
            }
        }
    }
}

Reference< ui::XUIConfigurationManager > SAL_CALL SfxBaseModel::getUIConfigurationManager()
{
    return Reference< ui::XUIConfigurationManager >( getUIConfigurationManager2(), UNO_QUERY_THROW );
}

Reference< ui::XUIConfigurationManager2 > SfxBaseModel::getUIConfigurationManager2()
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_xUIConfigurationManager.is() )
    {
        Reference< ui::XUIConfigurationManager2 > xNewUIConfMan =
            ui::UIConfigurationManager::create( comphelper::getProcessComponentContext() );

        Reference< embed::XStorage > xConfigStorage;

        OUString aUIConfigFolderName( "Configurations2" );
        // First try to open with READWRITE and then READ
        xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, embed::ElementModes::READWRITE );
        if ( xConfigStorage.is() )
        {
            static constexpr OUString aMediaTypeProp( u"MediaType"_ustr );
            OUString aMediaType;
            Reference< beans::XPropertySet > xPropSet( xConfigStorage, UNO_QUERY );
            Any a = xPropSet->getPropertyValue( aMediaTypeProp );
            if ( !( a >>= aMediaType ) ||  aMediaType.isEmpty())
            {
                xPropSet->setPropertyValue( aMediaTypeProp, Any(OUString("application/vnd.sun.xml.ui.configuration")) );
            }
        }
        else
            xConfigStorage = getDocumentSubStorage( aUIConfigFolderName, embed::ElementModes::READ );

        // initialize ui configuration manager with document substorage
        xNewUIConfMan->setStorage( xConfigStorage );

        // embedded objects did not support local configuration data until OOo 3.0, so there's nothing to
        // migrate
        if ( m_pData->m_pObjectShell->GetCreateMode() != SfxObjectCreateMode::EMBEDDED )
        {
            // Import old UI configuration from OOo 1.x

            // Try to open with READ
            Reference< embed::XStorage > xOOo1ConfigStorage = getDocumentSubStorage( "Configurations", embed::ElementModes::READ );
            if ( xOOo1ConfigStorage.is() )
            {
                Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                std::vector< Reference< container::XIndexContainer > > rToolbars;

                bool bImported = framework::UIConfigurationImporterOOo1x::ImportCustomToolbars(
                                        xNewUIConfMan, rToolbars, xContext, xOOo1ConfigStorage );
                if ( bImported )
                {
                    SfxObjectShell* pObjShell = SfxBaseModel::GetObjectShell();

                    for ( size_t i = 0; i < rToolbars.size(); i++ )
                    {
                        const OUString sId(OUString::number( i + 1 ));
                        const OUString aCustomTbxName = "private:resource/toolbar/custom_OOo1x_" + sId;

                        Reference< container::XIndexContainer > xToolbar = rToolbars[i];
                        ConvertSlotsToCommands( pObjShell, xToolbar );
                        if ( !xNewUIConfMan->hasSettings( aCustomTbxName ))
                        {
                            // Set UIName for the toolbar with container property
                            Reference< beans::XPropertySet > xPropSet( xToolbar, UNO_QUERY );
                            if ( xPropSet.is() )
                            {
                                try
                                {
                                    xPropSet->setPropertyValue( "UIName", Any( "Toolbar " + sId ) );
                                }
                                catch ( beans::UnknownPropertyException& )
                                {
                                }
                            }

                            xNewUIConfMan->insertSettings( aCustomTbxName, xToolbar );
                            xNewUIConfMan->store();
                        }
                    }
                }
            }
        }

        m_pData->m_xUIConfigurationManager = xNewUIConfMan;
    }

    return m_pData->m_xUIConfigurationManager;
}


//  XVisualObject


void SAL_CALL SfxBaseModel::setVisualAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.is() )
        throw Exception("no object shell", nullptr); // TODO: error handling

    SfxViewFrame* pViewFrm = SfxViewFrame::GetFirst( m_pData->m_pObjectShell.get(), false );
    if ( pViewFrm && m_pData->m_pObjectShell->GetCreateMode() == SfxObjectCreateMode::EMBEDDED && !pViewFrm->GetFrame().IsInPlace() )
    {
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( pViewFrm->GetFrame().GetFrameInterface()->getContainerWindow() );
        Size aWinSize = pWindow->GetSizePixel();
        awt::Size aCurrent = getVisualAreaSize( nAspect );
        Size aDiff( aSize.Width-aCurrent.Width, aSize.Height-aCurrent.Height );
        aDiff = pViewFrm->GetViewShell()->GetWindow()->LogicToPixel( aDiff );
        aWinSize.AdjustWidth(aDiff.Width() );
        aWinSize.AdjustHeight(aDiff.Height() );
        pWindow->SetSizePixel( aWinSize );
    }
    else
    {
        tools::Rectangle aTmpRect = m_pData->m_pObjectShell->GetVisArea( ASPECT_CONTENT );
        aTmpRect.SetSize( Size( aSize.Width, aSize.Height ) );
        m_pData->m_pObjectShell->SetVisArea( aTmpRect );
    }
}

awt::Size SAL_CALL SfxBaseModel::getVisualAreaSize( sal_Int64 /*nAspect*/ )
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.is() )
        throw Exception("no object shell", nullptr); // TODO: error handling

    tools::Rectangle aTmpRect = m_pData->m_pObjectShell->GetVisArea( ASPECT_CONTENT );

    return awt::Size( aTmpRect.GetWidth(), aTmpRect.GetHeight() );
}


sal_Int32 SAL_CALL SfxBaseModel::getMapUnit( sal_Int64 /*nAspect*/ )
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.is() )
        throw Exception("no object shell", nullptr); // TODO: error handling

    return VCLUnoHelper::VCL2UnoEmbedMapUnit( m_pData->m_pObjectShell->GetMapUnit() );
}

embed::VisualRepresentation SAL_CALL SfxBaseModel::getPreferredVisualRepresentation( ::sal_Int64 /*nAspect*/ )
{
    SfxModelGuard aGuard( *this );

    datatransfer::DataFlavor aDataFlavor(
            "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"",
            "GDIMetaFile",
            cppu::UnoType<Sequence< sal_Int8 >>::get() );

    embed::VisualRepresentation aVisualRepresentation;
    aVisualRepresentation.Data = getTransferData( aDataFlavor );
    aVisualRepresentation.Flavor = aDataFlavor;

    return aVisualRepresentation;
}


//  XStorageBasedDocument


void SAL_CALL SfxBaseModel::loadFromStorage( const Reference< embed::XStorage >& xStorage,
                                             const Sequence< beans::PropertyValue >& aMediaDescriptor )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );
    if ( IsInitialized() )
        throw frame::DoubleInitializationException( OUString(), *this );

    // after i36090 is fixed the pool from object shell can be used
    // SfxAllItemSet aSet( m_pData->m_pObjectShell->GetPool() );
    SfxAllItemSet aSet( SfxGetpApp()->GetPool() );

    // the BaseURL is part of the ItemSet
    SfxMedium* pMedium = new SfxMedium( xStorage, OUString() );
    TransformParameters( SID_OPENDOC, aMediaDescriptor, aSet );
    pMedium->GetItemSet().Put( aSet );

    // allow to use an interactionhandler (if there is one)
    pMedium->UseInteractionHandler( true );

    const SfxBoolItem* pTemplateItem = aSet.GetItem<SfxBoolItem>(SID_TEMPLATE, false);
    bool bTemplate = pTemplateItem && pTemplateItem->GetValue();
    m_pData->m_pObjectShell->SetActivateEvent_Impl( bTemplate ? SfxEventHintId::CreateDoc : SfxEventHintId::OpenDoc );
    m_pData->m_pObjectShell->Get_Impl()->bOwnsStorage = false;

    // load document
    if ( !m_pData->m_pObjectShell->DoLoad(pMedium) )
    {
        ErrCodeMsg nError = m_pData->m_pObjectShell->GetErrorCode();
        nError = nError ? nError : ERRCODE_IO_CANTREAD;
        throw task::ErrorCodeIOException(
            "SfxBaseModel::loadFromStorage: " + nError.toString(),
            Reference< XInterface >(), sal_uInt32(nError.GetCode()));
    }
    loadCmisProperties( );
}

void SAL_CALL SfxBaseModel::storeToStorage( const Reference< embed::XStorage >& xStorage,
                                const Sequence< beans::PropertyValue >& aMediaDescriptor )
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.is() )
        throw io::IOException(); // TODO:

    auto xSet = std::make_shared<SfxAllItemSet>(m_pData->m_pObjectShell->GetPool());
    TransformParameters( SID_SAVEASDOC, aMediaDescriptor, *xSet );

    // TODO/LATER: maybe a special URL "private:storage" should be used
    const SfxStringItem* pItem = xSet->GetItem<SfxStringItem>(SID_FILTER_NAME, false);
    sal_Int32 nVersion = SOFFICE_FILEFORMAT_CURRENT;
    if( pItem )
    {
        std::shared_ptr<const SfxFilter> pFilter = SfxGetpApp()->GetFilterMatcher().GetFilter4FilterName( pItem->GetValue() );
        if ( pFilter && pFilter->UsesStorage() )
            nVersion = pFilter->GetVersion();
    }

    bool bSuccess = false;
    if ( xStorage == m_pData->m_pObjectShell->GetStorage() )
    {
        // storing to the own storage
        bSuccess = m_pData->m_pObjectShell->DoSave();
    }
    else
    {
        // TODO/LATER: if the provided storage has some data inside the storing might fail, probably the storage must be truncated
        // TODO/LATER: is it possible to have a template here?
        m_pData->m_pObjectShell->SetupStorage( xStorage, nVersion, false );

        // BaseURL is part of the ItemSet
        SfxMedium aMedium( xStorage, OUString(), xSet );
        aMedium.CanDisposeStorage_Impl( false );
        if ( aMedium.GetFilter() )
        {
            // storing without a valid filter will often crash
            bSuccess = m_pData->m_pObjectShell->DoSaveObjectAs( aMedium, true );
            m_pData->m_pObjectShell->DoSaveCompleted();
        }
    }

    ErrCodeMsg nError = m_pData->m_pObjectShell->GetErrorCode();
    m_pData->m_pObjectShell->ResetError();

    // the warnings are currently not transported
    if ( !bSuccess )
    {
        nError = nError ? nError : ERRCODE_IO_GENERAL;
        throw task::ErrorCodeIOException(
            "SfxBaseModel::storeToStorage: " + nError.toString(),
            Reference< XInterface >(), sal_uInt32(nError.GetCode()));
    }
}

void SAL_CALL SfxBaseModel::switchToStorage( const Reference< embed::XStorage >& xStorage )
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.is() )
        throw io::IOException(); // TODO:

    // the persistence should be switched only if the storage is different
    if ( xStorage != m_pData->m_pObjectShell->GetStorage() )
    {
        if ( !m_pData->m_pObjectShell->SwitchPersistence( xStorage ) )
        {
            ErrCodeMsg nError = m_pData->m_pObjectShell->GetErrorCode();
            nError = nError ? nError : ERRCODE_IO_GENERAL;
            throw task::ErrorCodeIOException(
                "SfxBaseModel::switchToStorage: " + nError.toString(),
                Reference< XInterface >(), sal_uInt32(nError.GetCode()));
        }
        else
        {
            // UICfgMgr has a reference to the old storage, update it
            getUIConfigurationManager2()->setStorage( xStorage );
        }
    }
    m_pData->m_pObjectShell->Get_Impl()->bOwnsStorage = false;
}

Reference< embed::XStorage > SAL_CALL SfxBaseModel::getDocumentStorage()
{
    SfxModelGuard aGuard( *this );

    if ( !m_pData->m_pObjectShell.is() )
        throw io::IOException(); // TODO

    return m_pData->m_pObjectShell->GetStorage();
}

void SAL_CALL SfxBaseModel::addStorageChangeListener(
            const Reference< document::XStorageChangeListener >& xListener )
{
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    m_pData->m_aStorageChangeListeners.addInterface( xListener );
}

void SAL_CALL SfxBaseModel::removeStorageChangeListener(
            const Reference< document::XStorageChangeListener >& xListener )
{
    SfxModelGuard aGuard( *this );

    m_pData->m_aStorageChangeListeners.removeInterface( xListener );
}

void SfxBaseModel::impl_getPrintHelper()
{
    if ( m_pData->m_xPrintable.is() )
        return;
    m_pData->m_xPrintable = new SfxPrintHelper();
    Reference < lang::XInitialization > xInit( m_pData->m_xPrintable, UNO_QUERY );
    xInit->initialize( { Any(Reference < frame::XModel > (this)) } );
    Reference < view::XPrintJobBroadcaster > xBrd( m_pData->m_xPrintable, UNO_QUERY );
    xBrd->addPrintJobListener( new SfxPrintHelperListener_Impl( m_pData.get() ) );
}


// css.frame.XModule
 void SAL_CALL SfxBaseModel::setIdentifier(const OUString& Identifier)
{
    SfxModelGuard aGuard( *this );
    m_pData->m_sModuleIdentifier = Identifier;
}


// css.frame.XModule
 OUString SAL_CALL SfxBaseModel::getIdentifier()
{
    SfxModelGuard aGuard( *this );
    if (!m_pData->m_sModuleIdentifier.isEmpty())
        return m_pData->m_sModuleIdentifier;
    if (m_pData->m_pObjectShell.is())
        return m_pData->m_pObjectShell->GetFactory().GetDocumentServiceName();
    return OUString();
}


Reference< frame::XTitle > SfxBaseModel::impl_getTitleHelper ()
{
    SfxModelGuard aGuard( *this );

    if ( ! m_pData->m_xTitleHelper.is ())
    {
        Reference< XComponentContext >     xContext = ::comphelper::getProcessComponentContext();
        Reference< frame::XUntitledNumbers >    xDesktop( frame::Desktop::create(xContext), UNO_QUERY_THROW);

        m_pData->m_xTitleHelper = new ::framework::TitleHelper(xContext, Reference< frame::XModel >(this), xDesktop);
    }

    return m_pData->m_xTitleHelper;
}


Reference< frame::XUntitledNumbers > SfxBaseModel::impl_getUntitledHelper ()
{
    SfxModelGuard aGuard( *this );

    if ( ! m_pData->m_xNumberedControllers.is ())
    {
        rtl::Reference<::comphelper::NumberedCollection> pHelper = new ::comphelper::NumberedCollection();
        m_pData->m_xNumberedControllers = pHelper;
        pHelper->setOwner          (Reference< frame::XModel >(this));
        pHelper->setUntitledPrefix (" : ");
    }

    return m_pData->m_xNumberedControllers;
}


// css.frame.XTitle
OUString SAL_CALL SfxBaseModel::getTitle()
{
    // SYNCHRONIZED ->
    SfxModelGuard aGuard( *this );

    OUString aResult = impl_getTitleHelper()->getTitle ();
    if ( !m_pData->m_bExternalTitle && m_pData->m_pObjectShell )
    {
        SfxMedium* pMedium = m_pData->m_pObjectShell->GetMedium();
        if ( pMedium )
        {
            try {
                ::ucbhelper::Content aContent( pMedium->GetName(),
                    utl::UCBContentHelper::getDefaultCommandEnvironment(),
                    comphelper::getProcessComponentContext() );
                const Reference < beans::XPropertySetInfo > xProps
                     = aContent.getProperties();
                if ( xProps.is() )
                {
                    static constexpr OUString aServerTitle( u"TitleOnServer"_ustr );
                    if ( xProps->hasPropertyByName( aServerTitle ) )
                    {
                        Any aAny = aContent.getPropertyValue( aServerTitle );
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
            const SfxBoolItem* pRepairedDocItem = pMedium->GetItemSet().GetItem(SID_REPAIRPACKAGE, false);
            if ( pRepairedDocItem && pRepairedDocItem->GetValue() )
                aResult += SfxResId(STR_REPAIREDDOCUMENT);
        }

        if ( m_pData->m_pObjectShell->IsReadOnlyUI() || (pMedium && pMedium->IsReadOnly()) )
            aResult += SfxResId(STR_READONLY);
        else if ( m_pData->m_pObjectShell->IsDocShared() )
            aResult += SfxResId(STR_SHARED);

        if ( m_pData->m_pObjectShell->GetDocumentSignatureState() == SignatureState::OK )
            aResult += SfxResId(RID_XMLSEC_DOCUMENTSIGNED);
    }

    return aResult;
}


// css.frame.XTitle
void SAL_CALL SfxBaseModel::setTitle( const OUString& sTitle )
{
    // SYNCHRONIZED ->
    SfxModelGuard aGuard( *this );

    impl_getTitleHelper()->setTitle (sTitle);
    m_pData->m_bExternalTitle = true;
}


// css.frame.XTitleChangeBroadcaster
void SAL_CALL SfxBaseModel::addTitleChangeListener( const Reference< frame::XTitleChangeListener >& xListener )
{
    // SYNCHRONIZED ->
    SfxModelGuard aGuard( *this, SfxModelGuard::E_INITIALIZING );

    Reference< frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper(), UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (xListener);
}


// css.frame.XTitleChangeBroadcaster
void SAL_CALL SfxBaseModel::removeTitleChangeListener( const Reference< frame::XTitleChangeListener >& xListener )
{
    // SYNCHRONIZED ->
    SfxModelGuard aGuard( *this );

    Reference< frame::XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper(), UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->removeTitleChangeListener (xListener);
}


// css.frame.XUntitledNumbers
::sal_Int32 SAL_CALL SfxBaseModel::leaseNumber( const Reference< XInterface >& xComponent )
{
    SfxModelGuard aGuard( *this );

    return impl_getUntitledHelper ()->leaseNumber (xComponent);
}


// css.frame.XUntitledNumbers
void SAL_CALL SfxBaseModel::releaseNumber( ::sal_Int32 nNumber )
{
    SfxModelGuard aGuard( *this );
    impl_getUntitledHelper ()->releaseNumber (nNumber);
}


// css.frame.XUntitledNumbers
void SAL_CALL SfxBaseModel::releaseNumberForComponent( const Reference< XInterface >& xComponent )
{
    SfxModelGuard aGuard( *this );
    impl_getUntitledHelper ()->releaseNumberForComponent (xComponent);
}


// css.frame.XUntitledNumbers
OUString SAL_CALL SfxBaseModel::getUntitledPrefix()
{
    SfxModelGuard aGuard( *this );
    return impl_getUntitledHelper ()->getUntitledPrefix ();
}


// frame::XModel2
Reference< container::XEnumeration > SAL_CALL SfxBaseModel::getControllers()
{
    SfxModelGuard aGuard( *this );

    sal_Int32 c = m_pData->m_seqControllers.size();
    Sequence< Any > lEnum(c);
    std::transform(m_pData->m_seqControllers.begin(), m_pData->m_seqControllers.end(),
                   lEnum.getArray(), [](const auto& x) { return css::uno::Any(x); });

    return new ::comphelper::OAnyEnumeration(lEnum);
}


// frame::XModel2
Sequence< OUString > SAL_CALL SfxBaseModel::getAvailableViewControllerNames()
{
    SfxModelGuard aGuard( *this );

    const SfxObjectFactory& rDocumentFactory = GetObjectShell()->GetFactory();
    const sal_Int16 nViewFactoryCount = rDocumentFactory.GetViewFactoryCount();

    Sequence< OUString > aViewNames( nViewFactoryCount );
    auto aViewNamesRange = asNonConstRange(aViewNames);
    for ( sal_Int16 nViewNo = 0; nViewNo < nViewFactoryCount; ++nViewNo )
        aViewNamesRange[nViewNo] = rDocumentFactory.GetViewFactory( nViewNo ).GetAPIViewName();
    return aViewNames;
}


// frame::XModel2
Reference< frame::XController2 > SAL_CALL SfxBaseModel::createDefaultViewController( const Reference< frame::XFrame >& i_rFrame )
{
    SfxModelGuard aGuard( *this );

    const SfxObjectFactory& rDocumentFactory = GetObjectShell()->GetFactory();
    const OUString sDefaultViewName = rDocumentFactory.GetViewFactory().GetAPIViewName();

    aGuard.clear();

    return createViewController( sDefaultViewName, Sequence< PropertyValue >(), i_rFrame );
}


namespace sfx::intern {

    /** a class which, in its dtor, cleans up various objects (well, at the moment only the frame) collected during
        the creation of a document view, unless the creation was successful.
    */
    class ViewCreationGuard
    {
    public:
        ViewCreationGuard()
            :m_bSuccess( false )
        {
        }

        ~ViewCreationGuard()
        {
            if ( !m_bSuccess && m_aWeakFrame && !m_aWeakFrame->GetCurrentDocument() )
            {
                m_aWeakFrame->SetFrameInterface_Impl( nullptr );
                m_aWeakFrame->DoClose();
            }
        }

        void takeFrameOwnership( SfxFrame* i_pFrame )
        {
            OSL_PRECOND( !m_aWeakFrame, "ViewCreationGuard::takeFrameOwnership: already have a frame!" );
            OSL_PRECOND( i_pFrame != nullptr, "ViewCreationGuard::takeFrameOwnership: invalid frame!" );
            m_aWeakFrame = i_pFrame;
        }

        void    releaseAll()
        {
            m_bSuccess = true;
        }

    private:
        bool             m_bSuccess;
        SfxFrameWeakRef  m_aWeakFrame;
    };
}


SfxViewFrame* SfxBaseModel::FindOrCreateViewFrame_Impl( const Reference< XFrame >& i_rFrame, ::sfx::intern::ViewCreationGuard& i_rGuard ) const
{
    SfxViewFrame* pViewFrame = nullptr;
    for (   pViewFrame = SfxViewFrame::GetFirst( GetObjectShell(), false );
            pViewFrame;
            pViewFrame= SfxViewFrame::GetNext( *pViewFrame, GetObjectShell(), false )
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
                if  (   ( pCheckFrame->GetCurrentViewFrame() != nullptr )
                    ||  ( pCheckFrame->GetCurrentDocument() != nullptr )
                    )
                    // Note that it is perfectly legitimate that during loading into an XFrame which already contains
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


// frame::XModel2
Reference< frame::XController2 > SAL_CALL SfxBaseModel::createViewController(
        const OUString& i_rViewName, const Sequence< PropertyValue >& i_rArguments, const Reference< XFrame >& i_rFrame )
{
    SfxModelGuard aGuard( *this );

    if ( !i_rFrame.is() )
        throw lang::IllegalArgumentException( OUString(), *this, 3 );

    // find the proper SFX view factory
    SfxViewFactory* pViewFactory = GetObjectShell()->GetFactory().GetViewFactoryByViewName( i_rViewName );
    if ( !pViewFactory )
        throw IllegalArgumentException( OUString(), *this, 1 );

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
    OSL_ENSURE( !xPreviousController.is() || ( pOldViewShell != nullptr ),
        "SfxBaseModel::createViewController: invalid old controller!" );

    // a guard which will clean up in case of failure
    ::sfx::intern::ViewCreationGuard aViewCreationGuard;

    // determine the ViewFrame belonging to the given XFrame
    SfxViewFrame* pViewFrame = FindOrCreateViewFrame_Impl( i_rFrame, aViewCreationGuard );
    assert(pViewFrame && "SfxBaseModel::createViewController: no frame");

    // delegate to SFX' view factory
    pViewFrame->GetBindings().ENTERREGISTRATIONS();
    SfxViewShell* pViewShell = pViewFactory->CreateInstance(*pViewFrame, pOldViewShell);
    pViewFrame->GetBindings().LEAVEREGISTRATIONS();
    ENSURE_OR_THROW( pViewShell, "invalid view shell provided by factory" );

    // by setting the ViewShell it is prevented that disposing the Controller will destroy this ViewFrame also
    pViewFrame->GetDispatcher()->SetDisableFlags( SfxDisableFlags::NONE );
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
    ::comphelper::NamedValueCollection aDocumentLoadArgs( getArgs2( { "ViewOnly", "PluginMode" } ) );
    if ( aDocumentLoadArgs.getOrDefault( "ViewOnly", false ) )
        pViewFrame->GetFrame().SetMenuBarOn_Impl( false );

    const sal_Int16 nPluginMode = aDocumentLoadArgs.getOrDefault( "PluginMode", sal_Int16( 0 ) );
    if ( nPluginMode == 1 )
    {
        pViewFrame->ForceOuterResize_Impl();
        pViewFrame->GetBindings().HidePopups();

        SfxFrame& rFrame = pViewFrame->GetFrame();
        // MBA: layoutmanager of inplace frame starts locked and invisible
        rFrame.GetWorkWindow_Impl()->MakeVisible_Impl( false );
        rFrame.GetWorkWindow_Impl()->Lock_Impl( true );

        rFrame.GetWindow().SetBorderStyle( WindowBorderStyle::NOBORDER );
        pViewFrame->GetWindow().SetBorderStyle( WindowBorderStyle::NOBORDER );
    }

    // tell the guard we were successful
    aViewCreationGuard.releaseAll();

    // outta here
    return pBaseController;
}


// RDF DocumentMetadataAccess

// rdf::XRepositorySupplier:
Reference< rdf::XRepository > SAL_CALL
SfxBaseModel::getRDFRepository()
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->getRDFRepository();
}

// rdf::XNode:
OUString SAL_CALL
SfxBaseModel::getStringValue()
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->getStringValue();
}

// rdf::XURI:
OUString SAL_CALL
SfxBaseModel::getNamespace()
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->getNamespace();
}

OUString SAL_CALL
SfxBaseModel::getLocalName()
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->getLocalName();
}

// rdf::XDocumentMetadataAccess:
Reference< rdf::XMetadatable > SAL_CALL
SfxBaseModel::getElementByMetadataReference(
    const beans::StringPair & i_rReference)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->getElementByMetadataReference(i_rReference);
}

Reference< rdf::XMetadatable > SAL_CALL
SfxBaseModel::getElementByURI(const Reference< rdf::XURI > & i_xURI)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->getElementByURI(i_xURI);
}

Sequence< Reference< rdf::XURI > > SAL_CALL
SfxBaseModel::getMetadataGraphsWithType(
    const Reference<rdf::XURI> & i_xType)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->getMetadataGraphsWithType(i_xType);
}

Reference<rdf::XURI> SAL_CALL
SfxBaseModel::addMetadataFile(const OUString & i_rFileName,
    const Sequence < Reference< rdf::XURI > > & i_rTypes)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->addMetadataFile(i_rFileName, i_rTypes);
}

Reference<rdf::XURI> SAL_CALL
SfxBaseModel::importMetadataFile(::sal_Int16 i_Format,
    const Reference< io::XInputStream > & i_xInStream,
    const OUString & i_rFileName,
    const Reference< rdf::XURI > & i_xBaseURI,
    const Sequence < Reference< rdf::XURI > > & i_rTypes)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->importMetadataFile(i_Format,
        i_xInStream, i_rFileName, i_xBaseURI, i_rTypes);
}

void SAL_CALL
SfxBaseModel::removeMetadataFile(
    const Reference< rdf::XURI > & i_xGraphName)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->removeMetadataFile(i_xGraphName);
}

void SAL_CALL
SfxBaseModel::addContentOrStylesFile(const OUString & i_rFileName)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->addContentOrStylesFile(i_rFileName);
}

void SAL_CALL
SfxBaseModel::removeContentOrStylesFile(const OUString & i_rFileName)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->removeContentOrStylesFile(i_rFileName);
}

void SAL_CALL
SfxBaseModel::loadMetadataFromStorage(
    Reference< embed::XStorage > const & i_xStorage,
    Reference<rdf::XURI> const & i_xBaseURI,
    Reference<task::XInteractionHandler> const & i_xHandler)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(
        m_pData->CreateDMAUninitialized());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    try {
        xDMA->loadMetadataFromStorage(i_xStorage, i_xBaseURI, i_xHandler);
    } catch (lang::IllegalArgumentException &) {
        throw; // not initialized
    } catch (Exception &) {
        // UGLY: if it's a RuntimeException, we can't be sure DMA is initialized
        m_pData->m_xDocumentMetadata = xDMA;
        throw;
    }
    m_pData->m_xDocumentMetadata = xDMA;

}

void SAL_CALL
SfxBaseModel::storeMetadataToStorage(
    Reference< embed::XStorage > const & i_xStorage)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->storeMetadataToStorage(i_xStorage);
}

void SAL_CALL
SfxBaseModel::loadMetadataFromMedium(
    const Sequence< beans::PropertyValue > & i_rMedium)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(
        m_pData->CreateDMAUninitialized());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    try {
        xDMA->loadMetadataFromMedium(i_rMedium);
    } catch (lang::IllegalArgumentException &) {
        throw; // not initialized
    } catch (Exception &) {
        // UGLY: if it's a RuntimeException, we can't be sure DMA is initialized
        m_pData->m_xDocumentMetadata = xDMA;
        throw;
    }
    m_pData->m_xDocumentMetadata = xDMA;
}

void SAL_CALL
SfxBaseModel::storeMetadataToMedium(
    const Sequence< beans::PropertyValue > & i_rMedium)
{
    SfxModelGuard aGuard( *this );

    const Reference<rdf::XDocumentMetadataAccess> xDMA(m_pData->GetDMA());
    if (!xDMA.is()) {
        throw RuntimeException( "model has no document metadata", *this );
    }

    return xDMA->storeMetadataToMedium(i_rMedium);
}


// = SfxModelSubComponent


SfxModelSubComponent::~SfxModelSubComponent()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
