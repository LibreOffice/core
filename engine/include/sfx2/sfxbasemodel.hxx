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

#pragma once

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/document/XCmisDocument.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentRecovery2.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/document/XShapeEventBroadcaster.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/frame/XModel3.hpp>
#include <com/sun/star/util/XModifiable2.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/view/XPrintJobBroadcaster.hpp>
#include <com/sun/star/frame/XStorable2.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
//#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <cpo/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <cpo/uno/Any.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/script/XStarBasicAccess.hpp>

#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase.hxx>
#include <svl/lstner.hxx>

#include <memory>

class ErrCodeMsg;
class SfxMedium;
class   SfxObjectShell                      ;
class   SfxViewFrame;
struct  IMPL_SfxBaseModel_DataContainer     ;   // impl. struct to hold member of class SfxBaseModel

namespace sfx::intern {
    class ViewCreationGuard;
}

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::document { struct EventObject; }
namespace com::sun::star::frame { class XController2; }
namespace com::sun::star::ui { class XUIConfigurationManager2; }



/**_______________________________________________________________________________________________________
    @implements XChild
                XComponent
                document::XDocumentPropertiesSupplier
                rdf::XDocumentMetadataAccess
                XEventListener
                XModel
                XModifiable2
                XPrintable
                XStorable2
                document::XEventBroadcaster
                document::XEventsSupplier
                document::XEmbeddedScripts
                document::XScriptInvocationContext
                XCloseable
                XCloseBroadcaster

    @base       cppu::BaseMutex
                 SfxListener
*/

typedef ::cppu::WeakImplHelper  <   css::container::XChild
                                        ,   css::document::XDocumentPropertiesSupplier
                                        ,   css::document::XCmisDocument
                                        ,   css::rdf::XDocumentMetadataAccess
                                        ,   css::document::XDocumentRecovery2
                                        ,   css::document::XUndoManagerSupplier
                                        ,   css::document::XShapeEventBroadcaster
                                        ,   css::document::XDocumentEventBroadcaster
                                        ,   css::lang::XEventListener
                                        ,   css::document::XEventsSupplier
                                        ,   css::document::XEmbeddedScripts
                                        ,   css::document::XScriptInvocationContext
                                        ,   css::frame::XModel3
                                        ,   css::util::XModifiable2
                                        ,   css::view::XPrintable
                                        ,   css::view::XPrintJobBroadcaster
                                        ,   css::frame::XStorable2
                                        ,   css::frame::XLoadable
                                        ,   css::script::XStarBasicAccess
                                        ,   css::document::XViewDataSupplier
                                        ,   css::util::XCloseable           // => css::util::XCloseBroadcaster
                                        ,   css::datatransfer::XTransferable
                                        ,   css::document::XDocumentSubStorageSupplier
                                        ,   css::document::XStorageBasedDocument
                                        ,   css::script::provider::XScriptProviderSupplier
                                        ,   css::ui::XUIConfigurationManagerSupplier
                                        ,   css::embed::XVisualObject
                                        ,   css::lang::XUnoTunnel
                                        ,   css::frame::XModule
                                        ,   css::frame::XTitle
                                        ,   css::frame::XTitleChangeBroadcaster
                                        ,   css::frame::XUntitledNumbers
                                        >   SfxBaseModel_Base;

class SFX2_DLLPUBLIC SfxBaseModel   :   protected ::cppu::BaseMutex
                                    ,   public SfxBaseModel_Base
                                    ,   public SfxListener
{


//  public methods


public:


    //  constructor/destructor


    SfxBaseModel( SfxObjectShell *pObjectShell ) ;

    virtual ~SfxBaseModel() override ;


    //  XInterface


    /**___________________________________________________________________________________________________
        @short      give answer, if interface is supported
        @descr      The interfaces are searched by type.

        @seealso    XInterface

        @param      "rType" is the type of searched interface.

        @return     Any     information about found interface

        @onerror    A RuntimeException is thrown.
    */

    virtual cpo::uno::Any queryInterface( const cpo::uno::Type& rType ) override ;

    /**___________________________________________________________________________________________________
        @short      increment refcount
        @seealso    XInterface
        @seealso    release()
        @onerror    A RuntimeException is thrown.
    */

    virtual void acquire() noexcept override
    { OWeakObject::acquire(); }

    /**___________________________________________________________________________________________________
        @short      decrement refcount
        @seealso    XInterface
        @seealso    acquire()
        @onerror    A RuntimeException is thrown.
    */

    virtual void release() noexcept override
    { OWeakObject::release(); }


    //  XTypeProvider


    /**___________________________________________________________________________________________________
        @short      get information about supported interfaces
        @seealso    XTypeProvider
        @return     Sequence of types of all supported interfaces

        @onerror    A RuntimeException is thrown.
    */

    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override ;

    /**___________________________________________________________________________________________________
        @short      get implementation id
        @descr      This ID is necessary for UNO-caching. If there no ID, cache is disabled.
                    Another way, cache is enabled.

        @seealso    XTypeProvider
        @return     ID as Sequence of byte

        @onerror    A RuntimeException is thrown.
    */

    virtual cpo::uno::Sequence< sal_Int8 > getImplementationId() override ;


    //  XStarBasicAccess


    /**___________________________________________________________________________________________________
        @seealso    XStarBasicAccess
    */
    virtual css::uno::Reference< css::container::XNameContainer > getLibraryContainer() override;

    /**___________________________________________________________________________________________________
        @seealso    XStarBasicAccess
    */
    virtual void createLibrary( const OUString& LibName, const OUString& Password,
        const OUString& ExternalSourceURL, const OUString& LinkTargetURL ) override;

    /**___________________________________________________________________________________________________
        @seealso    XStarBasicAccess
    */
    virtual void addModule( const OUString& LibraryName, const OUString& ModuleName,
        const OUString& Language, const OUString& Source ) override;

    /**___________________________________________________________________________________________________
        @seealso    XStarBasicAccess
    */
    virtual void addDialog( const OUString& LibraryName, const OUString& DialogName,
        const cpo::uno::Sequence< sal_Int8 >& Data ) override;


    //  XChild


    virtual css::uno::Reference< css::uno::XInterface > getParent() override ;

    virtual void setParent( const css::uno::Reference< css::uno::XInterface >& xParent ) override;


    //  XComponent


    virtual void dispose() override;

    virtual void addEventListener(const css::uno::Reference< css::lang::XEventListener >& aListener) override;

    virtual void removeEventListener(const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XDocumentPropertiesSupplier
    virtual css::uno::Reference< css::document::XDocumentProperties >
        getDocumentProperties() override;


    //  XEventListener


    virtual void disposing( const css::lang::EventObject& aEvent ) override;


    //  XModel


    virtual bool attachResource(const OUString& sURL,
                                             const cpo::uno::Sequence< css::beans::PropertyValue >& aArgs) override;

    virtual OUString getURL() override;

    virtual cpo::uno::Sequence< css::beans::PropertyValue > getArgs() override;

    virtual void connectController( const css::uno::Reference< css::frame::XController >& xController ) override;

    virtual void disconnectController( const css::uno::Reference< css::frame::XController >& xController ) override;

    virtual void lockControllers() override;

    virtual void unlockControllers() override;

    virtual bool hasControllersLocked() override;

    virtual css::uno::Reference< css::frame::XController > getCurrentController() override;

    virtual void setCurrentController( const css::uno::Reference< css::frame::XController >& xController ) override;

    virtual css::uno::Reference< css::uno::XInterface > getCurrentSelection() override;


    //  XModel2

    virtual css::uno::Reference< css::container::XEnumeration > getControllers() override;

    virtual cpo::uno::Sequence< OUString > getAvailableViewControllerNames() override;

    virtual css::uno::Reference< css::frame::XController2 > createDefaultViewController(const css::uno::Reference< css::frame::XFrame >& Frame ) override;

    virtual css::uno::Reference< css::frame::XController2 > createViewController(const OUString&                                 ViewName       ,
                                                                                          const cpo::uno::Sequence< css::beans::PropertyValue >& Arguments      ,
                                                                                          const css::uno::Reference< css::frame::XFrame >&       Frame          ) override;

    virtual void setArgs(const cpo::uno::Sequence<css::beans::PropertyValue>& aArgs) override;


    //  XModel3

    virtual cpo::uno::Sequence< css::beans::PropertyValue > getArgs2( const cpo::uno::Sequence< OUString > & requestedArgs ) override;

    //  XModifiable2


    virtual bool disableSetModified(  ) override;
    virtual bool enableSetModified(  ) override;
    virtual bool isSetModifiedEnabled(  ) override;

    virtual bool isModified() override;

    virtual void setModified( bool bModified ) override;

    virtual void addModifyListener( const css::uno::Reference< css::util::XModifyListener >& xListener ) override ;

    virtual void removeModifyListener(const css::uno::Reference< css::util::XModifyListener > & xListener) override ;


    //  XCloseable


    virtual void close( bool bDeliverOwnership ) override;


    //  XCloseBroadcaster


    virtual void addCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) override;
    virtual void removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& xListener ) override;


    //  XPrintJobBroadcaster


    virtual void addPrintJobListener( const css::uno::Reference< css::view::XPrintJobListener >& xListener ) override;
    virtual void removePrintJobListener( const css::uno::Reference< css::view::XPrintJobListener >& xListener ) override;


    //  XPrintable


    virtual cpo::uno::Sequence< css::beans::PropertyValue > getPrinter() override;

    virtual void setPrinter( const cpo::uno::Sequence< css::beans::PropertyValue >& seqPrinter ) override;
    virtual void print( const cpo::uno::Sequence< css::beans::PropertyValue >& seqOptions ) override;


    //  XStorable2


    virtual void storeSelf( const  cpo::uno::Sequence< css::beans::PropertyValue >&   seqArguments    ) override;


    //  XStorable


    virtual bool hasLocation() override;

    virtual OUString getLocation() override;

    virtual bool isReadonly() override;

    virtual void store() override;

    virtual void storeAsURL(   const   OUString& sURL,
                                        const   cpo::uno::Sequence< css::beans::PropertyValue >&   seqArguments    ) override ;

    virtual void storeToURL(   const   OUString& sURL,
                                        const   cpo::uno::Sequence< css::beans::PropertyValue >&   seqArguments    ) override;

    SAL_DLLPRIVATE void
    impl_store(const OUString& sURL,
               const cpo::uno::Sequence<css::beans::PropertyValue>& seqArguments, bool bSaveTo);

    //  XLoadable


    virtual void initNew() override;

    virtual void load( const   cpo::uno::Sequence< css::beans::PropertyValue >&   seqArguments ) override;


    //  XDocumentSubStorageSupplier


    virtual css::uno::Reference< css::embed::XStorage > getDocumentSubStorage( const OUString& aStorageName, sal_Int32 nMode ) override;

    virtual cpo::uno::Sequence< OUString > getDocumentSubStoragesNames() override;


    //  XStorageBasedDocument


    virtual void loadFromStorage( const css::uno::Reference< css::embed::XStorage >& xStorage,
                                            const cpo::uno::Sequence< css::beans::PropertyValue >& aMediaDescriptor ) override;

    virtual void storeToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage,
                                            const cpo::uno::Sequence< css::beans::PropertyValue >& aMediaDescriptor ) override;

    virtual void switchToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage ) override;

    virtual css::uno::Reference< css::embed::XStorage > getDocumentStorage() override;

    virtual void addStorageChangeListener(
            const css::uno::Reference< css::document::XStorageChangeListener >& xListener ) override;

    virtual void removeStorageChangeListener(
            const css::uno::Reference< css::document::XStorageChangeListener >& xListener ) override;


    //  XVisualObject


    virtual void setVisualAreaSize( sal_Int64 nAspect, const css::awt::Size& aSize ) override;

    virtual css::awt::Size getVisualAreaSize( sal_Int64 nAspect ) override;

    virtual css::embed::VisualRepresentation getPreferredVisualRepresentation( ::sal_Int64 nAspect ) override;

    virtual sal_Int32 getMapUnit( sal_Int64 nAspect ) override;


    //  XScriptProviderSupplier


    virtual css::uno::Reference< css::script::provider::XScriptProvider > getScriptProvider() override;


    //  XUIConfigurationManagerSupplier
    virtual css::uno::Reference< css::ui::XUIConfigurationManager > getUIConfigurationManager() override;


    //  XTransferable


    virtual cpo::uno::Any getTransferData( const css::datatransfer::DataFlavor& aFlavor ) override;

    virtual cpo::uno::Sequence< css::datatransfer::DataFlavor > getTransferDataFlavors() override;

    virtual bool isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor ) override;


    //  XEventsSupplier


    /**___________________________________________________________________________________________________
        @descr      -   offers a list of event handlers which are be bound to events of
                        this object.
        @return     -   an Events object.
    */

    virtual css::uno::Reference< css::container::XNameReplace > getEvents() override;


    //  XEmbeddedScripts


    virtual css::uno::Reference< css::script::XStorageBasedLibraryContainer > getBasicLibraries() override;
    virtual css::uno::Reference< css::script::XStorageBasedLibraryContainer > getDialogLibraries() override;
    virtual bool getAllowMacroExecution() override;


    //  XScriptInvocationContext


    virtual css::uno::Reference< css::document::XEmbeddedScripts > getScriptContainer() override;


    //  document::XEventBroadcaster

    /**___________________________________________________________________________________________________
        @descr      -   registers the given XEventListener.
    */
    virtual void addEventListener( const css::uno::Reference< css::document::XEventListener >& xListener ) override;

    /**___________________________________________________________________________________________________
        @descr      -   unregisters the given XEventListener.
    */
    virtual void removeEventListener( const css::uno::Reference< css::document::XEventListener >& xListener ) override;


    //  document::XShapeEventBroadcaster

    /**___________________________________________________________________________________________________
        @descr      -   registers the given XEventListener.
    */
    virtual void addShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape, const css::uno::Reference< css::document::XShapeEventListener >& xListener ) override;

    /**___________________________________________________________________________________________________
        @descr      -   unregisters the given XEventListener.
    */
    virtual void removeShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape, const css::uno::Reference< css::document::XShapeEventListener >& xListener ) override;


    //  XDocumentEventBroadcaster

    virtual void addDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& Listener ) override;
    virtual void removeDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& Listener ) override;
    virtual void notifyDocumentEvent( const OUString& EventName, const css::uno::Reference< css::frame::XController2 >& ViewController, const cpo::uno::Any& Supplement ) override;


    //  XUnoTunnel


    virtual sal_Int64 getSomething( const cpo::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // css.frame.XModule
    virtual void setIdentifier(const OUString& sIdentifier) override;

    // css.frame.XModule
    virtual OUString getIdentifier() override;

    // css.frame.XTitle
    virtual OUString getTitle() override;

    // css.frame.XTitle
    virtual void setTitle( const OUString& sTitle ) override;

    // css.frame.XTitleChangeBroadcaster
    virtual void addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;

    // css.frame.XTitleChangeBroadcaster
    virtual void removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;

    // css.frame.XUntitledNumbers
    virtual ::sal_Int32 leaseNumber( const css::uno::Reference< css::uno::XInterface >& xComponent ) override;

    // css.frame.XUntitledNumbers
    virtual void releaseNumber( ::sal_Int32 nNumber ) override;

    // css.frame.XUntitledNumbers
    virtual void releaseNumberForComponent( const css::uno::Reference< css::uno::XInterface >& xComponent ) override;

    // css.frame.XUntitledNumbers
    virtual OUString getUntitledPrefix() override;

    // css.document.XDocumentRecovery
    virtual bool wasModifiedSinceLastSave() override;
    virtual void storeToRecoveryFile( const OUString& i_TargetLocation, const cpo::uno::Sequence< css::beans::PropertyValue >& i_MediaDescriptor ) override;
    virtual void recoverFromFile( const OUString& i_SourceLocation, const OUString& i_SalvagedFile, const cpo::uno::Sequence< css::beans::PropertyValue >& i_MediaDescriptor ) override;

    // css.document.XDocumentRecovery2
    virtual sal_Int64 getModifiedStateDuration() override;

    // css.document.XUndoManagerSupplier
    virtual css::uno::Reference< css::document::XUndoManager > getUndoManager(  ) override;


    // css::rdf::XNode:
    virtual OUString getStringValue() override;

    // css::rdf::XURI:
    virtual OUString getNamespace() override;
    virtual OUString getLocalName() override;

    // css::rdf::XRepositorySupplier:
    virtual css::uno::Reference<
        css::rdf::XRepository > getRDFRepository() override;

    // css::rdf::XDocumentMetadataAccess:
    virtual css::uno::Reference<
                css::rdf::XMetadatable >
        getElementByMetadataReference(
            const css::beans::StringPair & i_rReference) override;
    virtual css::uno::Reference<
                css::rdf::XMetadatable >
        getElementByURI(const css::uno::Reference<
            css::rdf::XURI > & i_xURI) override;
    virtual cpo::uno::Sequence< css::uno::Reference<
            css::rdf::XURI > > getMetadataGraphsWithType(
            const css::uno::Reference<
                css::rdf::XURI > & i_xType) override;
    virtual css::uno::Reference<
                css::rdf::XURI>
        addMetadataFile(const OUString & i_rFileName,
            const cpo::uno::Sequence<
                css::uno::Reference< css::rdf::XURI >
                > & i_rTypes) override;
    virtual css::uno::Reference<
                css::rdf::XURI>
        importMetadataFile(::sal_Int16 i_Format,
            const css::uno::Reference<
                css::io::XInputStream > & i_xInStream,
            const OUString & i_rFileName,
            const css::uno::Reference<
                css::rdf::XURI > & i_xBaseURI,
            const cpo::uno::Sequence<
                css::uno::Reference< css::rdf::XURI >
                > & i_rTypes) override;
    virtual void removeMetadataFile(
            const css::uno::Reference<
                css::rdf::XURI > & i_xGraphName) override;
    virtual void addContentOrStylesFile(
            const OUString & i_rFileName) override;
    virtual void removeContentOrStylesFile(
            const OUString & i_rFileName) override;

    virtual void loadMetadataFromStorage(
            const css::uno::Reference<
                css::embed::XStorage > & i_xStorage,
            const css::uno::Reference<
                css::rdf::XURI > & i_xBaseURI,
            const css::uno::Reference<
                css::task::XInteractionHandler> & i_xHandler) override;
    virtual void storeMetadataToStorage(
            const css::uno::Reference<
                css::embed::XStorage > & i_xStorage) override;
    virtual void loadMetadataFromMedium(
            const cpo::uno::Sequence<
                css::beans::PropertyValue > & i_rMedium) override;
    virtual void storeMetadataToMedium(
            const cpo::uno::Sequence<
                css::beans::PropertyValue > & i_rMedium) override;

    // XCmisDocument

    virtual cpo::uno::Sequence< css::document::CmisProperty >
            getCmisProperties() override;
    virtual void setCmisProperties(
            const cpo::uno::Sequence<
                css::document::CmisProperty >& _cmisproperties ) override;

    virtual void updateCmisProperties(
            const cpo::uno::Sequence<
                css::document::CmisProperty >& _cmisproperties ) override;

    virtual cpo::uno::Sequence< css::document::CmisVersion > getAllVersions ( ) override;

    virtual void checkOut(  ) override;
    virtual void cancelCheckOut(  ) override;
    virtual void checkIn( bool bIsMajor, const OUString & rMessage  ) override;

    virtual bool isVersionable( ) override;
    virtual bool canCheckOut( ) override;
    virtual bool canCancelCheckOut( ) override;
    virtual bool canCheckIn( ) override;

    /// @throws cpo::uno::RuntimeException
    bool getBoolPropertyValue( const OUString& rName );


    //  SfxListener


    void Notify(            SfxBroadcaster& aBC     ,
                    const   SfxHint&        aHint   ) override ;


    //  public IMPL?


    void changing() ;

    SfxObjectShell* GetObjectShell() const ;

    SAL_DLLPRIVATE bool impl_isDisposed() const ;
    bool IsInitialized() const;
    void MethodEntryCheck( const bool i_mustBeInitialized ) const;

    css::uno::Reference < css::container::XIndexAccess > getViewData() override;
    void setViewData( const css::uno::Reference < css::container::XIndexAccess >& aData ) override;

    /** calls all XEventListeners */
    void notifyEvent( const css::document::EventObject& aEvent ) const;

    /** returns true if someone added a XEventListener to this XEventBroadcaster */
    bool hasEventListeners() const;

protected:

    /* returns a unique id for the model that is valid as long as the document
       is loaded. The id is not saved across document close/reload. */
    OUString const & getRuntimeUID() const;

    /* returns true if the document signatures are valid, otherwise false */
    bool hasValidSignatures() const;

    /* GrabBagItem for interim interop purposes */
    void getGrabBagItem(cpo::uno::Any& rVal) const;

    void setGrabBagItem(const cpo::uno::Any& rVal);


//  private methods


private:
    /// @throws cpo::uno::RuntimeException
    css::uno::Reference< css::ui::XUIConfigurationManager2 > getUIConfigurationManager2();
    void impl_getPrintHelper();
    SAL_DLLPRIVATE void ListenForStorage_Impl( const css::uno::Reference< css::embed::XStorage >& xStorage );
    SAL_DLLPRIVATE OUString GetMediumFilterName_Impl() const;

    SAL_DLLPRIVATE void postEvent_Impl( const OUString& aName, const css::uno::Reference< css::frame::XController2 >& xController = css::uno::Reference< css::frame::XController2 >(), const cpo::uno::Any& aSupplement = cpo::uno::Any());

    SAL_DLLPRIVATE css::uno::Reference< css::frame::XTitle > impl_getTitleHelper ();
    SAL_DLLPRIVATE css::uno::Reference< css::frame::XUntitledNumbers > impl_getUntitledHelper ();

    SAL_DLLPRIVATE SfxViewFrame* FindOrCreateViewFrame_Impl(
                        const css::uno::Reference< css::frame::XFrame >& i_rFrame,
                        ::sfx::intern::ViewCreationGuard& i_rGuard
                    ) const;

    SAL_DLLPRIVATE void NotifyModifyListeners_Impl() const;

    SAL_DLLPRIVATE void loadCmisProperties();

    SAL_DLLPRIVATE SfxMedium* handleLoadError( const ErrCodeMsg& nError, SfxMedium* pMedium );


//  private variables and methods


private:

    std::shared_ptr<IMPL_SfxBaseModel_DataContainer> m_pData;
    // cannot be held in m_pData, since it needs to be accessed in non-threadsafe context
    const bool                          m_bSupportEmbeddedScripts;
    const bool                          m_bSupportDocRecovery;

} ; // class SfxBaseModel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
