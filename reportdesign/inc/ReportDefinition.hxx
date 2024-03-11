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

#ifndef INCLUDED_REPORTDESIGN_INC_REPORTDEFINITION_HXX
#define INCLUDED_REPORTDESIGN_INC_REPORTDEFINITION_HXX

#include <config_options.h>
#include "dllapi.h"

#include "ReportHelperDefines.hxx"

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager2.hpp>

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <svx/unomod.hxx>

#include <memory>


namespace rptui
{
    class OReportModel;
}
namespace utl
{
    class MediaDescriptor;
}
namespace reportdesign
{
    class OReportComponentProperties;
    typedef cppu::PartialWeakComponentImplHelper<   css::report::XReportDefinition
                                                    ,   css::document::XEventBroadcaster
                                                    ,   css::document::XDocumentEventBroadcaster
                                                    ,   css::lang::XServiceInfo
                                                    ,   css::frame::XModule
                                                    ,   css::lang::XUnoTunnel
                                                    ,   css::util::XNumberFormatsSupplier
                                                    ,   css::frame::XTitle
                                                    ,   css::frame::XTitleChangeBroadcaster
                                                    ,   css::frame::XUntitledNumbers
                                                    ,   css::document::XDocumentPropertiesSupplier
                                                    ,   css::datatransfer::XTransferable
                                                    ,   css::document::XUndoManagerSupplier
                                                    ,   SvxUnoDrawMSFactory
                                                    >   ReportDefinitionBase;

    typedef ::cppu::PropertySetMixin< css::report::XReportDefinition > ReportDefinitionPropertySet;

    struct OReportDefinitionImpl;
    /** \class OReportDefinition Defines the implementation of a \interface com:::sun::star::report::XReportDefinition
     * \ingroup reportdesign_api
     *
     */
    class OReportDefinition  final :public ::cppu::BaseMutex
                                                    ,public ReportDefinitionBase
                                                    ,public ReportDefinitionPropertySet
                                                    ,public ::comphelper::IEmbeddedHelper
    {
        std::shared_ptr<OReportComponentProperties>                             m_aProps;
        std::shared_ptr<OReportDefinitionImpl>                                  m_pImpl;

        OReportDefinition(const OReportDefinition&) = delete;
        OReportDefinition& operator=(const OReportDefinition&) = delete;

        void setSection(     const OUString& _sProperty
                            ,bool _bOn
                            ,const OUString& _sName
                            ,css::uno::Reference< css::report::XSection>& _member);

        template <typename T> void set(  const OUString& _sProperty
                                        ,const T& Value
                                        ,T& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, css::uno::Any(_member), css::uno::Any(Value), &l);
                _member = Value;
            }
            l.notify();
        }

        void set(  const OUString& _sProperty
                  ,bool Value
                  ,bool& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, css::uno::Any(_member), css::uno::Any(Value), &l);
                _member = Value;
            }
            l.notify();
        }

        /// write a single XML stream into the package
        bool WriteThroughComponent(
            /// the component we export
            const css::uno::Reference< css::lang::XComponent> & xComponent,
            const char* pStreamName,        /// the stream name
            const char* pServiceName,       /// service name of the component
            /// the argument (XInitialization)
            const css::uno::Sequence< css::uno::Any> & rArguments,
            /// output descriptor
            const css::uno::Sequence< css::beans::PropertyValue> & rMediaDesc,
            const css::uno::Reference< css::embed::XStorage >& _xStorageToSaveTo);

        /// write a single output stream
        /// (to be called either directly or by WriteThroughComponent(...))
        bool WriteThroughComponent(
            const css::uno::Reference< css::io::XOutputStream> & xOutputStream,
            const css::uno::Reference< css::lang::XComponent> & xComponent,
            const char* pServiceName,
            const css::uno::Sequence< css::uno::Any> & rArguments,
            const css::uno::Sequence< css::beans::PropertyValue> & rMediaDesc);

        void notifyEvent(const OUString& _sEventName);
        void init();
        void fillArgs(utl::MediaDescriptor& _aDescriptor);

        css::uno::Reference< css::frame::XTitle >             impl_getTitleHelper_throw();
        css::uno::Reference< css::frame::XUntitledNumbers >   impl_getUntitledHelper_throw();

        /** loads the report definition from the given storage
            @precond
                our mutex is locked
            @throws
        */
        void impl_loadFromStorage_nolck_throw(
            const css::uno::Reference< css::embed::XStorage >& _rxStorage,
            const css::uno::Sequence< css::beans::PropertyValue >& _rArguments
        );

        virtual ~OReportDefinition() override;

        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL disposing() override;
    public:
        explicit OReportDefinition(css::uno::Reference< css::uno::XComponentContext > const & _xContext);
        explicit OReportDefinition(css::uno::Reference< css::uno::XComponentContext > const & _xContext
                                  ,const css::uno::Reference< css::lang::XMultiServiceFactory > & _xFactory
                                  ,css::uno::Reference< css::drawing::XShape >& _xShape);

        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
        /// @throws css::uno::RuntimeException
        static OUString getImplementationName_Static();
        static css::uno::Reference< css::uno::XInterface >
            create(css::uno::Reference< css::uno::XComponentContext > const & xContext);

        css::uno::Reference< css::uno::XComponentContext > getContext();

    private:
        /** abstract SdrModel provider */
        virtual SdrModel& getSdrModelFromUnoModel() const override;

    public:
        //TTTT Needed? Or same as above?
        static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();
        UNLESS_MERGELIBS_MORE(REPORTDESIGN_DLLPUBLIC) static std::shared_ptr<rptui::OReportModel> getSdrModel(const css::uno::Reference< css::report::XReportDefinition >& _xReportDefinition);

    private:
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )
        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        // XReportDefinition
        virtual OUString SAL_CALL getMimeType() override;
        virtual void SAL_CALL setMimeType( const OUString& _mimetype ) override;
        virtual OUString SAL_CALL getCaption() override;
        virtual void SAL_CALL setCaption( const OUString& _caption ) override;
        virtual ::sal_Int16 SAL_CALL getGroupKeepTogether() override;
        virtual void SAL_CALL setGroupKeepTogether( ::sal_Int16 _groupkeeptogether ) override;
        virtual ::sal_Int16 SAL_CALL getPageHeaderOption() override;
        virtual void SAL_CALL setPageHeaderOption( ::sal_Int16 _pageheaderoption ) override;
        virtual ::sal_Int16 SAL_CALL getPageFooterOption() override;
        virtual void SAL_CALL setPageFooterOption( ::sal_Int16 _pagefooteroption ) override;
        virtual OUString SAL_CALL getCommand() override;
        virtual void SAL_CALL setCommand( const OUString& _command ) override;
        virtual ::sal_Int32 SAL_CALL getCommandType() override;
        virtual void SAL_CALL setCommandType( ::sal_Int32 _commandtype ) override;
        virtual OUString SAL_CALL getFilter() override;
        virtual void SAL_CALL setFilter( const OUString& _filter ) override;
        virtual sal_Bool SAL_CALL getEscapeProcessing() override;
        virtual void SAL_CALL setEscapeProcessing( sal_Bool _escapeprocessing ) override;
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getActiveConnection() override;
        virtual void SAL_CALL setActiveConnection( const css::uno::Reference< css::sdbc::XConnection >& _activeconnection ) override;
        virtual OUString SAL_CALL getDataSourceName() override;
        virtual void SAL_CALL setDataSourceName( const OUString& _datasourcename ) override;
        virtual sal_Bool SAL_CALL getReportHeaderOn() override;
        virtual void SAL_CALL setReportHeaderOn( sal_Bool _reportheaderon ) override;
        virtual sal_Bool SAL_CALL getReportFooterOn() override;
        virtual void SAL_CALL setReportFooterOn( sal_Bool _reportfooteron ) override;
        virtual sal_Bool SAL_CALL getPageHeaderOn() override;
        virtual void SAL_CALL setPageHeaderOn( sal_Bool _pageheaderon ) override;
        virtual sal_Bool SAL_CALL getPageFooterOn() override;
        virtual void SAL_CALL setPageFooterOn( sal_Bool _pagefooteron ) override;
        virtual css::uno::Reference< css::report::XGroups > SAL_CALL getGroups() override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getReportHeader() override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getPageHeader() override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getDetail() override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getPageFooter() override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getReportFooter() override;
        virtual css::uno::Reference< css::document::XEventBroadcaster > SAL_CALL getEventBroadcaster(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAvailableMimeTypes(  ) override;

        // XReportComponent
        REPORTCOMPONENT_HEADER()

        // XShape
        SHAPE_HEADER()

        // XShapeDescriptor
        virtual OUString SAL_CALL getShapeType(  ) override;

        //XFunctionsSupplier
        virtual css::uno::Reference< css::report::XFunctions > SAL_CALL getFunctions() override;

        // XCloneable
        virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;

        // XComponent
        virtual void SAL_CALL dispose() override;
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }

        // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

        // XCloseBroadcaster
        virtual void SAL_CALL addCloseListener( const css::uno::Reference< css::util::XCloseListener >& Listener ) override;
        virtual void SAL_CALL removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& Listener ) override;

        // XCloseable
        virtual void SAL_CALL close( sal_Bool DeliverOwnership ) override;

        // XModel
        virtual sal_Bool SAL_CALL attachResource( const OUString& URL, const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) override;
        virtual OUString SAL_CALL getURL(  ) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getArgs(  ) override;
        virtual void SAL_CALL connectController( const css::uno::Reference< css::frame::XController >& Controller ) override;
        virtual void SAL_CALL disconnectController( const css::uno::Reference< css::frame::XController >& Controller ) override;
        virtual void SAL_CALL lockControllers(  ) override;
        virtual void SAL_CALL unlockControllers(  ) override;
        virtual sal_Bool SAL_CALL hasControllersLocked(  ) override;
        virtual css::uno::Reference< css::frame::XController > SAL_CALL getCurrentController(  ) override;
        virtual void SAL_CALL setCurrentController( const css::uno::Reference< css::frame::XController >& Controller ) override;
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getCurrentSelection(  ) override;

        // XStorageBasedDocument
        virtual void SAL_CALL loadFromStorage( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescriptor ) override;
        virtual void SAL_CALL storeToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescriptor ) override;
        virtual void SAL_CALL switchToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage ) override;
        virtual css::uno::Reference< css::embed::XStorage > SAL_CALL getDocumentStorage(  ) override;
        virtual void SAL_CALL addStorageChangeListener( const css::uno::Reference< css::document::XStorageChangeListener >& xListener ) override;
        virtual void SAL_CALL removeStorageChangeListener( const css::uno::Reference< css::document::XStorageChangeListener >& xListener ) override;

        // XViewDataSupplier
        virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getViewData(  ) override;
        virtual void SAL_CALL setViewData( const css::uno::Reference< css::container::XIndexAccess >& Data ) override;

        // XLoadable
        virtual void SAL_CALL initNew(  ) override;
        virtual void SAL_CALL load( const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) override;

        // XVisualObject
        virtual void SAL_CALL setVisualAreaSize( ::sal_Int64 nAspect, const css::awt::Size& aSize ) override;
        virtual css::awt::Size SAL_CALL getVisualAreaSize( ::sal_Int64 nAspect ) override;
        virtual css::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect ) override;
        virtual ::sal_Int32 SAL_CALL getMapUnit( ::sal_Int64 nAspect ) override;

        // XModifiable2
        virtual sal_Bool SAL_CALL disableSetModified(  ) override;
        virtual sal_Bool SAL_CALL enableSetModified(  ) override;
        virtual sal_Bool SAL_CALL isSetModifiedEnabled(  ) override;

        // XModifiable
        virtual sal_Bool SAL_CALL isModified(  ) override;
        virtual void SAL_CALL setModified( sal_Bool bModified ) override;

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
        virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

        // document::XEventBroadcaster
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::document::XEventListener >& aListener ) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::document::XEventListener >& aListener ) override;

        // document::XDocumentEventBroadcaster
        virtual void SAL_CALL addDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& rListener ) override;
        virtual void SAL_CALL removeDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& rListener ) override;
        virtual void SAL_CALL notifyDocumentEvent( const OUString& rEventName, const css::uno::Reference< css::frame::XController2 >& rViewController, const css::uno::Any& rSupplement ) override;

        // XUIConfigurationManagerSupplier
        virtual css::uno::Reference< css::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager(  ) override;

        // XDocumentSubStorageSupplier
        virtual css::uno::Reference< css::embed::XStorage > SAL_CALL getDocumentSubStorage( const OUString& aStorageName, sal_Int32 nMode ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getDocumentSubStoragesNames(  ) override;

        // css::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

        // SvxUnoDrawMSFactory
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) override;
        css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() override;

        // XStyleFamiliesSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getStyleFamilies(  ) override;

        // XModule
        virtual void SAL_CALL setIdentifier( const OUString& Identifier ) override;
        virtual OUString SAL_CALL getIdentifier(  ) override;

        // XNumberFormatsSupplier
        virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getNumberFormatSettings(  ) override;
        virtual css::uno::Reference< css::util::XNumberFormats > SAL_CALL getNumberFormats(  ) override;

        // XTitle
        virtual OUString SAL_CALL getTitle(  ) override;
        virtual void SAL_CALL setTitle( const OUString& sTitle ) override;

        // XTitleChangeBroadcaster
        virtual void SAL_CALL addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;
        virtual void SAL_CALL removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) override;

        // XUntitledNumbers
        virtual ::sal_Int32 SAL_CALL leaseNumber( const css::uno::Reference< css::uno::XInterface >& xComponent ) override;
        virtual void SAL_CALL releaseNumber( ::sal_Int32 nNumber ) override;
        virtual void SAL_CALL releaseNumberForComponent( const css::uno::Reference< css::uno::XInterface >& xComponent ) override;
        virtual OUString SAL_CALL getUntitledPrefix(  ) override;

        // XDocumentPropertiesSupplier
        virtual css::uno::Reference< css::document::XDocumentProperties > SAL_CALL getDocumentProperties(  ) override;

        // XTransferable
        virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor ) override;
        virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) override;
        virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor ) override;

        // XUndoManagerSupplier
        virtual css::uno::Reference< css::document::XUndoManager > SAL_CALL getUndoManager(  ) override;

        // comphelper::IEmbeddedHelper
        virtual css::uno::Reference < css::embed::XStorage > getStorage() const override;
        virtual ::comphelper::EmbeddedObjectContainer& getEmbeddedObjectContainer() const override;
        virtual css::uno::Reference< css::task::XInteractionHandler > getInteractionHandler() const override;
        virtual bool isEnableSetModified() const override;
        virtual OUString getDocumentBaseURL() const override;

        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::ui::XUIConfigurationManager2 > getUIConfigurationManager2(  );
      };

} // namespace reportdesign

#endif // INCLUDED_REPORTDESIGN_INC_REPORTDEFINITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
