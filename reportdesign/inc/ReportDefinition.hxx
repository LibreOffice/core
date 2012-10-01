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

#ifndef REPORTDESIGN_API_REPORTDEFINITION_HXX
#define REPORTDESIGN_API_REPORTDEFINITION_HXX

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

#include <comphelper/embeddedobjectcontainer.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/propertysetmixin.hxx>
#include <svx/unomod.hxx>

#ifndef INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_13
#define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_13
#define COMPHELPER_IMPLBASE_INTERFACE_NUMBER 13
#include <comphelper/implbase_var.hxx>
#endif

#include <boost/shared_ptr.hpp>


namespace rptui
{
    class OReportModel;
}
namespace comphelper
{
    class MediaDescriptor;
}
namespace reportdesign
{
    class OReportComponentProperties;
    typedef ::comphelper::PartialWeakComponentImplHelper13 <   ::com::sun::star::report::XReportDefinition
                                                    ,   ::com::sun::star::document::XEventBroadcaster
                                                    ,   ::com::sun::star::lang::XServiceInfo
                                                    ,   ::com::sun::star::frame::XModule
                                                    ,   ::com::sun::star::lang::XUnoTunnel
                                                    ,   ::com::sun::star::util::XNumberFormatsSupplier
                                                    ,   ::com::sun::star::frame::XTitle
                                                    ,   ::com::sun::star::frame::XTitleChangeBroadcaster
                                                    ,   ::com::sun::star::frame::XUntitledNumbers
                                                    ,   ::com::sun::star::document::XDocumentPropertiesSupplier
                                                    ,   ::com::sun::star::datatransfer::XTransferable
                                                    ,   ::com::sun::star::document::XUndoManagerSupplier
                                                    ,   SvxUnoDrawMSFactory
                                                    >   ReportDefinitionBase;

    typedef ::cppu::PropertySetMixin< ::com::sun::star::report::XReportDefinition > ReportDefinitionPropertySet;

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
        ::boost::shared_ptr<OReportComponentProperties>                             m_aProps;
        ::boost::shared_ptr<OReportDefinitionImpl>                                  m_pImpl;

    private:
        OReportDefinition(const OReportDefinition&);
        OReportDefinition& operator=(const OReportDefinition&);

        void setSection(     const ::rtl::OUString& _sProperty
                            ,const sal_Bool& _bOn
                            ,const ::rtl::OUString& _sName
                            ,::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _member);

        template <typename T> void set(  const ::rtl::OUString& _sProperty
                                        ,const T& _Value
                                        ,T& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, ::com::sun::star::uno::makeAny(_member), ::com::sun::star::uno::makeAny(_Value), &l);
                _member = _Value;
            }
            l.notify();
        }

        /// write a single XML stream into the package
        sal_Bool WriteThroughComponent(
            /// the component we export
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XComponent> & xComponent,
            const sal_Char* pStreamName,        /// the stream name
            const sal_Char* pServiceName,       /// service name of the component
            /// the argument (XInitialization)
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Any> & rArguments,
            /// output descriptor
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue> & rMediaDesc,
            sal_Bool bPlainStream
            , const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _xStorageToSaveTo);          /// neither compress nor encrypt

        /// write a single output stream
        /// (to be called either directly or by WriteThroughComponent(...))
        sal_Bool WriteThroughComponent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::io::XOutputStream> & xOutputStream,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XComponent> & xComponent,
            const sal_Char* pServiceName,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Any> & rArguments,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue> & rMediaDesc);

        void notifyEvent(const ::rtl::OUString& _sEventName);
        void init();
        void fillArgs(::comphelper::MediaDescriptor& _aDescriptor);

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitle >             impl_getTitleHelper_throw();
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XUntitledNumbers >   impl_getUntitledHelper_throw();

        /** loads the report definition from the given storage
            @precond
                our mutex is locked
            @throws
        */
        void impl_loadFromStorage_nolck_throw(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArguments
        );

    protected:
        virtual ~OReportDefinition();

        /** this function is called upon disposing the component
        */
        virtual void SAL_CALL disposing();
    public:
        explicit OReportDefinition(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext);
        explicit OReportDefinition(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext
                                  ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & _xFactory
                                  ,::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _xShape);

        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getContext();

        /** return the SdrModel of the real model
        *
        * \return
        */
        ::boost::shared_ptr<rptui::OReportModel> getSdrModel() const;

        static ::boost::shared_ptr<rptui::OReportModel> getSdrModel(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _xReportDefinition);
    private:
        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )
        // ::com::sun::star::lang::XServiceInfo
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XReportDefinition
        virtual ::rtl::OUString SAL_CALL getMimeType() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMimeType( const ::rtl::OUString& _mimetype ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getCaption() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCaption( const ::rtl::OUString& _caption ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getGroupKeepTogether() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setGroupKeepTogether( ::sal_Int16 _groupkeeptogether ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getPageHeaderOption() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPageHeaderOption( ::sal_Int16 _pageheaderoption ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int16 SAL_CALL getPageFooterOption() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPageFooterOption( ::sal_Int16 _pagefooteroption ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getCommand() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCommand( const ::rtl::OUString& _command ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getCommandType() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCommandType( ::sal_Int32 _commandtype ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getFilter() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFilter( const ::rtl::OUString& _filter ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getEscapeProcessing() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setEscapeProcessing( ::sal_Bool _escapeprocessing ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getActiveConnection() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setActiveConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _activeconnection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getDataSourceName() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDataSourceName( const ::rtl::OUString& _datasourcename ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getReportHeaderOn() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setReportHeaderOn( ::sal_Bool _reportheaderon ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getReportFooterOn() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setReportFooterOn( ::sal_Bool _reportfooteron ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getPageHeaderOn() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPageHeaderOn( ::sal_Bool _pageheaderon ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getPageFooterOn() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPageFooterOn( ::sal_Bool _pagefooteron ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroups > SAL_CALL getGroups() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > SAL_CALL getReportHeader() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > SAL_CALL getPageHeader() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > SAL_CALL getDetail() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > SAL_CALL getPageFooter() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > SAL_CALL getReportFooter() throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventBroadcaster > SAL_CALL getEventBroadcaster(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableMimeTypes(  ) throw (::com::sun::star::lang::DisposedException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XReportComponent
        REPORTCOMPONENT_HEADER()

        // XShape
        SHAPE_HEADER()

        // XShapeDescriptor
        virtual ::rtl::OUString SAL_CALL getShapeType(  ) throw (::com::sun::star::uno::RuntimeException);

        //XFunctionsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunctions > SAL_CALL getFunctions() throw (::com::sun::star::uno::RuntimeException);

        // XCloneable
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException)
        {
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException)
        {
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }

        // XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

        // XCloseBroadcaster
        virtual void SAL_CALL addCloseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeCloseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

        // XCloseable
        virtual void SAL_CALL close( ::sal_Bool DeliverOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);

        // XModel
        virtual ::sal_Bool SAL_CALL attachResource( const ::rtl::OUString& URL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getURL(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getArgs(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL connectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& Controller ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL disconnectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& Controller ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL lockControllers(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL unlockControllers(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasControllersLocked(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL getCurrentController(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCurrentController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& Controller ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getCurrentSelection(  ) throw (::com::sun::star::uno::RuntimeException);

        // XStorageBasedDocument
        virtual void SAL_CALL loadFromStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescriptor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::frame::DoubleInitializationException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL storeToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescriptor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL switchToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL getDocumentStorage(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addStorageChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeStorageChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XStorageChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XViewDataSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getViewData(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setViewData( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& Data ) throw (::com::sun::star::uno::RuntimeException);

        // XLoadable
        virtual void SAL_CALL initNew(  ) throw (::com::sun::star::frame::DoubleInitializationException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL load( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments ) throw (::com::sun::star::frame::DoubleInitializationException, ::com::sun::star::io::IOException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XVisualObject
        virtual void SAL_CALL setVisualAreaSize( ::sal_Int64 nAspect, const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Size SAL_CALL getVisualAreaSize( ::sal_Int64 nAspect ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getMapUnit( ::sal_Int64 nAspect ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XModifiable2
        virtual ::sal_Bool SAL_CALL disableSetModified(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL enableSetModified(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isSetModifiedEnabled(  ) throw (::com::sun::star::uno::RuntimeException);

        // XModifiable
        virtual ::sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setModified( ::sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // document::XEventBroadcaster
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // document::XEventListener
        virtual void SAL_CALL notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XUIConfigurationManagerSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager(  ) throw (::com::sun::star::uno::RuntimeException);

        // XDocumentSubStorageSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL getDocumentSubStorage( const ::rtl::OUString& aStorageName, sal_Int32 nMode ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getDocumentSubStoragesNames(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

        // SvxUnoDrawMSFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

        // XStyleFamiliesSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getStyleFamilies(  ) throw (::com::sun::star::uno::RuntimeException);

        // XModule
        virtual void SAL_CALL setIdentifier( const ::rtl::OUString& Identifier ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getIdentifier(  ) throw (::com::sun::star::uno::RuntimeException);

        // XNumberFormatsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getNumberFormatSettings(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > SAL_CALL getNumberFormats(  ) throw (::com::sun::star::uno::RuntimeException);

        // XTitle
        virtual ::rtl::OUString SAL_CALL getTitle(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTitle( const ::rtl::OUString& sTitle ) throw (::com::sun::star::uno::RuntimeException);

        // XTitleChangeBroadcaster
        virtual void SAL_CALL addTitleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitleChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeTitleChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTitleChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XUntitledNumbers
        virtual ::sal_Int32 SAL_CALL leaseNumber( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xComponent ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL releaseNumber( ::sal_Int32 nNumber ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL releaseNumberForComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xComponent ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getUntitledPrefix(  ) throw (::com::sun::star::uno::RuntimeException);

        // XDocumentPropertiesSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties > SAL_CALL getDocumentProperties(  ) throw (::com::sun::star::uno::RuntimeException);

        // XTransferable
        virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw (::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw (::com::sun::star::uno::RuntimeException);

        // XUndoManagerSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XUndoManager > SAL_CALL getUndoManager(  ) throw (::com::sun::star::uno::RuntimeException);

        // comphelper::IEmbeddedHelper
        virtual com::sun::star::uno::Reference < com::sun::star::embed::XStorage > getStorage() const;
        virtual ::comphelper::EmbeddedObjectContainer& getEmbeddedObjectContainer() const;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > getInteractionHandler() const;
        virtual bool isEnableSetModified() const;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getContext() const;
      };
// =============================================================================
} // namespace reportdesign
// =============================================================================
#endif // REPORTDESIGN_API_REPORTDEFINITION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
