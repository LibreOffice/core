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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_CLICKABLEIMAGE_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_CLICKABLEIMAGE_HXX

#include "FormComponent.hxx"
#include "EventThread.hxx"
#include "imgprod.hxx"
#include <tools/link.hxx>
#include <comphelper/propmultiplex.hxx>
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/form/XApproveActionListener.hpp>
#include <com/sun/star/form/XApproveActionBroadcaster.hpp>
#include <com/sun/star/form/submission/XSubmissionSupplier.hpp>
#include <com/sun/star/form/submission/XSubmission.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/graphic/XGraphicObject.hpp>
#include <cppuhelper/implbase3.hxx>


class SfxMedium;


namespace frm
{


    class OImageProducerThread_Impl;
    class ControlFeatureInterception;

    // OClickableImageBaseModel

    typedef ::cppu::ImplHelper3 <   ::com::sun::star::form::XImageProducerSupplier
                                ,   ::com::sun::star::awt::XImageProducer
                                ,   ::com::sun::star::form::submission::XSubmissionSupplier
                                >   OClickableImageBaseModel_Base;

    class OClickableImageBaseModel  :public OClickableImageBaseModel_Base
                                    ,public OControlModel
                                    ,public OPropertyChangeListener
    {
    protected:
        ::com::sun::star::form::FormButtonType  m_eButtonType; // Type of the button (push, submit, reset)
        OUString                         m_sTargetURL;         // URL for the URL button
        OUString                         m_sTargetFrame;       // TargetFrame to open

        // ImageProducer stuff
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer>    m_xProducer;
        // Store the image in a graphic object to make it accesible via graphic cache using graphic ID.
        ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphicObject > m_xGraphicObject;
        SfxMedium*                              m_pMedium;     // Download medium
        ImageProducer*                          m_pProducer;
        bool                                m_bDispatchUrlInternal; // property: is not allowed to set : 1
        bool                                m_bDownloading : 1;     // Is a download in progress?
        bool                                m_bProdStarted : 1;

        // XSubmission stuff
        ::com::sun::star::uno::Reference< ::com::sun::star::form::submission::XSubmission >
                                                m_xSubmissionDelegate;

        DECL_LINK_TYPED( DownloadDoneLink, void*, void );

        inline ImageProducer* GetImageProducer() { return m_pProducer; }

        void StartProduction();
        void SetURL(const OUString& rURL);
        void DataAvailable();
        void DownloadDone();

        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;
        inline bool isDispatchUrlInternal() const { return m_bDispatchUrlInternal; }
        inline void     setDispatchUrlInternal(bool _bDispatch) { m_bDispatchUrlInternal = _bDispatch; }

    public:
        OClickableImageBaseModel(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory,
            const OUString& _rUnoControlModelTypeName,
            const OUString& _rDefault
        );
        DECLARE_DEFAULT_CLONE_CTOR( OClickableImageBaseModel )
        virtual ~OClickableImageBaseModel();

        // UNO Binding
        DECLARE_UNO3_AGG_DEFAULTS(OClickableImageBaseModel, OControlModel)
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        // OComponentHelper
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

        // ::com::sun::star::form::XImageProducerSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer> SAL_CALL getImageProducer() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return m_xProducer; }

        // OPropertySetHelper
        virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

        virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
            throw(::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;

        using ::cppu::OPropertySetHelper::getFastPropertyValue;

        // OPropertyChangeListener
        virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent&) throw(::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

        // XPropertyState
        virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const SAL_OVERRIDE;

        // XImageProducer
        virtual void SAL_CALL addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL startProduction(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XSubmissionSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::submission::XSubmission > SAL_CALL getSubmission() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setSubmission( const ::com::sun::star::uno::Reference< ::com::sun::star::form::submission::XSubmission >& _submission ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XEventListener
        using OControlModel::disposing;

    public:
        struct GuardAccess { friend class ImageModelMethodGuard; private: GuardAccess() { } };
        ::osl::Mutex&   getMutex( GuardAccess ) { return m_aMutex; }
        ImageProducer*  getImageProducer( GuardAccess ) { return m_pProducer; }

    protected:
        using OControlModel::getMutex;

        void implConstruct();

        // to be called from within the cloning-ctor of your derived class
        void implInitializeImageURL( );

        DECL_LINK_TYPED( OnImageImportDone, ::Graphic*, void );
    };

    class ImageModelMethodGuard : public ::osl::MutexGuard
    {
    private:
        typedef ::osl::MutexGuard   GuardBase;

    public:
        ImageModelMethodGuard( OClickableImageBaseModel& _rModel )
            :GuardBase( _rModel.getMutex( OClickableImageBaseModel::GuardAccess() ) )
        {
            if ( NULL == _rModel.getImageProducer( OClickableImageBaseModel::GuardAccess() ) )
                throw ::com::sun::star::lang::DisposedException(
                    OUString(),
                    static_cast< ::com::sun::star::form::XImageProducerSupplier* >( &_rModel )
                );
        }
    };


    // OClickableImageBaseControl

    typedef ::cppu::ImplHelper3 <   ::com::sun::star::form::XApproveActionBroadcaster
                                ,   ::com::sun::star::form::submission::XSubmission
                                ,   ::com::sun::star::frame::XDispatchProviderInterception
                                >   OClickableImageBaseControl_BASE;

    class OClickableImageBaseControl    :public OClickableImageBaseControl_BASE
                                        ,public OControl
    {
        friend class OImageProducerThread_Impl;

    private:
        OImageProducerThread_Impl*          m_pThread;
        ::cppu::OInterfaceContainerHelper   m_aSubmissionVetoListeners;
        ::std::unique_ptr< ControlFeatureInterception >
                                            m_pFeatureInterception;

    protected:
        ::cppu::OInterfaceContainerHelper m_aApproveActionListeners;
        ::cppu::OInterfaceContainerHelper m_aActionListeners;
        OUString m_aActionCommand;

        // XSubmission
        virtual void SAL_CALL submit(  ) throw (::com::sun::star::util::VetoException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL submitWithInteraction( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& aHandler ) throw (::com::sun::star::util::VetoException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL addSubmissionVetoListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::submission::XSubmissionVetoListener >& listener ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeSubmissionVetoListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::submission::XSubmissionVetoListener >& listener ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XEventListener
        using OControl::disposing;

    public:
        OClickableImageBaseControl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory,
            const OUString& _aService);
        virtual ~OClickableImageBaseControl();

    protected:
        // UNO Binding
        DECLARE_UNO3_AGG_DEFAULTS(OClickableImageBaseControl, OControl)
        virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // OComponentHelper
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

        // ::com::sun::star::form::XApproveActionBroadcaster
        virtual void SAL_CALL addApproveActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XApproveActionListener>& _rxListener)
            throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeApproveActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XApproveActionListener>& _rxListener)
            throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XDispatchProviderInterception
        virtual void SAL_CALL registerDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL releaseDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        virtual void actionPerformed_Impl( bool bNotifyListener, const ::com::sun::star::awt::MouseEvent& rEvt );

        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > _getTypes() SAL_OVERRIDE;

        /** approves the action by calling the approve listeners
            @return <TRUE/> if and only if the action has <em>not</em> been cancelled by a listener
        */
        bool    approveAction( );

        /** retrieves (and if necessary creates) the image producer thread.

            Must be called with our mutex locked
        */
        OImageProducerThread_Impl* getImageProducerThread();

    private:
        void implSubmit(
            const ::com::sun::star::awt::MouseEvent& _rEvent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& aHandler
        );
    };

    class OImageProducerThread_Impl: public OComponentEventThread
    {
    protected:

        // This method was called to duplicate the Event by taking its type into account
        virtual ::com::sun::star::lang::EventObject* cloneEvent( const ::com::sun::star::lang::EventObject* _pEvt ) const SAL_OVERRIDE;

        // Process an Event.
        // The mutex is not locked, pCompImpl stays valid in any case
        virtual void processEvent( ::cppu::OComponentHelper *pCompImpl,
                                const ::com::sun::star::lang::EventObject*,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>&,
                                bool ) SAL_OVERRIDE;

    public:
        OImageProducerThread_Impl( OClickableImageBaseControl *pControl ) :
            OComponentEventThread( pControl )
        {}

        void addEvent() { ::com::sun::star::lang::EventObject aEvt; OComponentEventThread::addEvent( &aEvt ); }

    protected:
        using OComponentEventThread::addEvent;
    };


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_CLICKABLEIMAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
