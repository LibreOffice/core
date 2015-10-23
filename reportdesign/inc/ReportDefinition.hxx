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

#include "dllapi.h"

#include "ReportHelperDefines.hxx"

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
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
    class REPORTDESIGN_DLLPUBLIC OReportDefinition  :public ::cppu::BaseMutex
                                                    ,public ReportDefinitionBase
                                                    ,public ReportDefinitionPropertySet
                                                    ,public ::comphelper::IEmbeddedHelper
    {
    private:
        std::shared_ptr<OReportComponentProperties>                             m_aProps;
        std::shared_ptr<OReportDefinitionImpl>                                  m_pImpl;

    private:
        OReportDefinition(const OReportDefinition&) = delete;
        OReportDefinition& operator=(const OReportDefinition&) = delete;

        void setSection(     const OUString& _sProperty
                            ,const bool& _bOn
                            ,const OUString& _sName
                            ,css::uno::Reference< css::report::XSection>& _member);

        template <typename T> void set(  const OUString& _sProperty
                                        ,const T& _Value
                                        ,T& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(_Value), &l);
                _member = _Value;
            }
            l.notify();
        }

        void set(  const OUString& _sProperty
                  ,bool _Value
                  ,bool& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, css::uno::makeAny(_member), css::uno::makeAny(_Value), &l);
                _member = _Value;
            }
            l.notify();
        }

        /// write a single XML stream into the package
        bool WriteThroughComponent(
            /// the component we export
            const css::uno::Reference< css::lang::XComponent> & xComponent,
            const sal_Char* pStreamName,        /// the stream name
            const sal_Char* pServiceName,       /// service name of the component
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
            const sal_Char* pServiceName,
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

    protected:
        virtual ~OReportDefinition();

        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL disposing() override;
    public:
        explicit OReportDefinition(css::uno::Reference< css::uno::XComponentContext > const & _xContext);
        explicit OReportDefinition(css::uno::Reference< css::uno::XComponentContext > const & _xContext
                                  ,const css::uno::Reference< css::lang::XMultiServiceFactory > & _xFactory
                                  ,css::uno::Reference< css::drawing::XShape >& _xShape);

        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
            create(css::uno::Reference< css::uno::XComponentContext > const & xContext);

        css::uno::Reference< css::uno::XComponentContext > getContext();

        /** return the SdrModel of the real model
        *
        * \return
        */
        std::shared_ptr<rptui::OReportModel> getSdrModel() const;

        static std::shared_ptr<rptui::OReportModel> getSdrModel(const css::uno::Reference< css::report::XReportDefinition >& _xReportDefinition);
    private:
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )
        // css::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

        // css::beans::XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        // XReportDefinition
        virtual OUString SAL_CALL getMimeType() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMimeType( const OUString& _mimetype ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getCaption() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setCaption( const OUString& _caption ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int16 SAL_CALL getGroupKeepTogether() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setGroupKeepTogether( ::sal_Int16 _groupkeeptogether ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int16 SAL_CALL getPageHeaderOption() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPageHeaderOption( ::sal_Int16 _pageheaderoption ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int16 SAL_CALL getPageFooterOption() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPageFooterOption( ::sal_Int16 _pagefooteroption ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getCommand() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setCommand( const OUString& _command ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getCommandType() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setCommandType( ::sal_Int32 _commandtype ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getFilter() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFilter( const OUString& _filter ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getEscapeProcessing() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setEscapeProcessing( sal_Bool _escapeprocessing ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getActiveConnection() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setActiveConnection( const css::uno::Reference< css::sdbc::XConnection >& _activeconnection ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getDataSourceName() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setDataSourceName( const OUString& _datasourcename ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getReportHeaderOn() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setReportHeaderOn( sal_Bool _reportheaderon ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getReportFooterOn() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setReportFooterOn( sal_Bool _reportfooteron ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getPageHeaderOn() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPageHeaderOn( sal_Bool _pageheaderon ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getPageFooterOn() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPageFooterOn( sal_Bool _pagefooteron ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::report::XGroups > SAL_CALL getGroups() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getReportHeader() throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getPageHeader() throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getDetail() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getPageFooter() throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::report::XSection > SAL_CALL getReportFooter() throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::document::XEventBroadcaster > SAL_CALL getEventBroadcaster(  ) throw (css::lang::DisposedException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getAvailableMimeTypes(  ) throw (css::lang::DisposedException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XReportComponent
        REPORTCOMPONENT_HEADER()

        // XShape
        SHAPE_HEADER()

        // XShapeDescriptor
        virtual OUString SAL_CALL getShapeType(  ) throw (css::uno::RuntimeException, std::exception) override;

        //XFunctionsSupplier
        virtual css::uno::Reference< css::report::XFunctions > SAL_CALL getFunctions() throw (css::uno::RuntimeException, std::exception) override;

        // XCloneable
        virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw(css::uno::RuntimeException, std::exception) override
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw(css::uno::RuntimeException, std::exception) override
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }

        // XChild
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

        // XCloseBroadcaster
        virtual void SAL_CALL addCloseListener( const css::uno::Reference< css::util::XCloseListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;

        // XCloseable
        virtual void SAL_CALL close( sal_Bool DeliverOwnership ) throw (css::util::CloseVetoException, css::uno::RuntimeException, std::exception) override;

        // XModel
        virtual sal_Bool SAL_CALL attachResource( const OUString& URL, const css::uno::Sequence< css::beans::PropertyValue >& Arguments ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getURL(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getArgs(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL connectController( const css::uno::Reference< css::frame::XController >& Controller ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL disconnectController( const css::uno::Reference< css::frame::XController >& Controller ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL lockControllers(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL unlockControllers(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasControllersLocked(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::frame::XController > SAL_CALL getCurrentController(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setCurrentController( const css::uno::Reference< css::frame::XController >& Controller ) throw (css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getCurrentSelection(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XStorageBasedDocument
        virtual void SAL_CALL loadFromStorage( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescriptor ) throw (css::lang::IllegalArgumentException, css::frame::DoubleInitializationException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL storeToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Sequence< css::beans::PropertyValue >& aMediaDescriptor ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL switchToStorage( const css::uno::Reference< css::embed::XStorage >& xStorage ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::embed::XStorage > SAL_CALL getDocumentStorage(  ) throw (css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addStorageChangeListener( const css::uno::Reference< css::document::XStorageChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeStorageChangeListener( const css::uno::Reference< css::document::XStorageChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XViewDataSupplier
        virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getViewData(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setViewData( const css::uno::Reference< css::container::XIndexAccess >& Data ) throw (css::uno::RuntimeException, std::exception) override;

        // XLoadable
        virtual void SAL_CALL initNew(  ) throw (css::frame::DoubleInitializationException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL load( const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw (css::frame::DoubleInitializationException, css::io::IOException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XVisualObject
        virtual void SAL_CALL setVisualAreaSize( ::sal_Int64 nAspect, const css::awt::Size& aSize ) throw (css::lang::IllegalArgumentException, css::embed::WrongStateException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual css::awt::Size SAL_CALL getVisualAreaSize( ::sal_Int64 nAspect ) throw (css::lang::IllegalArgumentException, css::embed::WrongStateException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual css::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect ) throw (css::lang::IllegalArgumentException, css::embed::WrongStateException, css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getMapUnit( ::sal_Int64 nAspect ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XModifiable2
        virtual sal_Bool SAL_CALL disableSetModified(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL enableSetModified(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isSetModifiedEnabled(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XModifiable
        virtual sal_Bool SAL_CALL isModified(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setModified( sal_Bool bModified ) throw (css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

        // document::XEventBroadcaster
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::document::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::document::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XUIConfigurationManagerSupplier
        virtual css::uno::Reference< css::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XDocumentSubStorageSupplier
        virtual css::uno::Reference< css::embed::XStorage > SAL_CALL getDocumentSubStorage( const OUString& aStorageName, sal_Int32 nMode ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getDocumentSubStoragesNames(  ) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;

        // css::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;
        static css::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

        // SvxUnoDrawMSFactory
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
        css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

        // XStyleFamiliesSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getStyleFamilies(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XModule
        virtual void SAL_CALL setIdentifier( const OUString& Identifier ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getIdentifier(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XNumberFormatsSupplier
        virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getNumberFormatSettings(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::util::XNumberFormats > SAL_CALL getNumberFormats(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XTitle
        virtual OUString SAL_CALL getTitle(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setTitle( const OUString& sTitle ) throw (css::uno::RuntimeException, std::exception) override;

        // XTitleChangeBroadcaster
        virtual void SAL_CALL addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XUntitledNumbers
        virtual ::sal_Int32 SAL_CALL leaseNumber( const css::uno::Reference< css::uno::XInterface >& xComponent ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL releaseNumber( ::sal_Int32 nNumber ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL releaseNumberForComponent( const css::uno::Reference< css::uno::XInterface >& xComponent ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getUntitledPrefix(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XDocumentPropertiesSupplier
        virtual css::uno::Reference< css::document::XDocumentProperties > SAL_CALL getDocumentProperties(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XTransferable
        virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor ) throw (css::datatransfer::UnsupportedFlavorException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor ) throw (css::uno::RuntimeException, std::exception) override;

        // XUndoManagerSupplier
        virtual css::uno::Reference< css::document::XUndoManager > SAL_CALL getUndoManager(  ) throw (css::uno::RuntimeException, std::exception) override;

        // comphelper::IEmbeddedHelper
        virtual css::uno::Reference < css::embed::XStorage > getStorage() const override;
        virtual ::comphelper::EmbeddedObjectContainer& getEmbeddedObjectContainer() const override;
        virtual css::uno::Reference< css::task::XInteractionHandler > getInteractionHandler() const override;
        virtual bool isEnableSetModified() const override;

        css::uno::Reference< css::ui::XUIConfigurationManager2 > getUIConfigurationManager2(  ) throw (css::uno::RuntimeException);
      };

} // namespace reportdesign

#endif // INCLUDED_REPORTDESIGN_INC_REPORTDEFINITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
