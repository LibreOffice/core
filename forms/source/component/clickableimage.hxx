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

    typedef ::cppu::ImplHelper3 <   css::form::XImageProducerSupplier
                                ,   css::awt::XImageProducer
                                ,   css::form::submission::XSubmissionSupplier
                                >   OClickableImageBaseModel_Base;

    class OClickableImageBaseModel  :public OClickableImageBaseModel_Base
                                    ,public OControlModel
                                    ,public OPropertyChangeListener
    {
    protected:
        css::form::FormButtonType        m_eButtonType; // Type of the button (push, submit, reset)
        OUString                         m_sTargetURL;         // URL for the URL button
        OUString                         m_sTargetFrame;       // TargetFrame to open

        // ImageProducer stuff
        css::uno::Reference< css::awt::XImageProducer>    m_xProducer;
        // Store the image in a graphic object to make it accessible via graphic cache using graphic ID.
        css::uno::Reference< css::graphic::XGraphicObject > m_xGraphicObject;
        SfxMedium*                          m_pMedium;     // Download medium
        ImageProducer*                      m_pProducer;
        bool                                m_bDispatchUrlInternal; // property: is not allowed to set : 1
        bool                                m_bDownloading : 1;     // Is a download in progress?
        bool                                m_bProdStarted : 1;

        // XSubmission stuff
        css::uno::Reference< css::form::submission::XSubmission >
                                                m_xSubmissionDelegate;

        DECL_LINK_TYPED( DownloadDoneLink, void*, void );

        inline ImageProducer* GetImageProducer() { return m_pProducer; }

        void StartProduction();
        void SetURL(const OUString& rURL);
        void DataAvailable();
        void DownloadDone();

        css::uno::Sequence< css::uno::Type> _getTypes() override;
        inline bool isDispatchUrlInternal() const { return m_bDispatchUrlInternal; }
        inline void     setDispatchUrlInternal(bool _bDispatch) { m_bDispatchUrlInternal = _bDispatch; }

    public:
        OClickableImageBaseModel(
            const css::uno::Reference< css::uno::XComponentContext>& _rxFactory,
            const OUString& _rUnoControlModelTypeName,
            const OUString& _rDefault
        );
        DECLARE_DEFAULT_CLONE_CTOR( OClickableImageBaseModel )
        virtual ~OClickableImageBaseModel();

        // UNO Binding
        DECLARE_UNO3_AGG_DEFAULTS(OClickableImageBaseModel, OControlModel)
        virtual css::uno::Any SAL_CALL queryAggregation(const css::uno::Type& _rType) throw(css::uno::RuntimeException, std::exception) override;

    protected:
        // OComponentHelper
        virtual void SAL_CALL disposing() override;

        // css::form::XImageProducerSupplier
        virtual css::uno::Reference< css::awt::XImageProducer> SAL_CALL getImageProducer() throw (css::uno::RuntimeException, std::exception) override { return m_xProducer; }

        // OPropertySetHelper
        virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle ) const override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const css::uno::Any& rValue) throw (css::uno::Exception, std::exception) override;

        virtual sal_Bool SAL_CALL convertFastPropertyValue(css::uno::Any& rConvertedValue, css::uno::Any& rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue )
            throw(css::lang::IllegalArgumentException) override;

        using ::cppu::OPropertySetHelper::getFastPropertyValue;

        // OPropertyChangeListener
        virtual void _propertyChanged(const css::beans::PropertyChangeEvent&) throw(css::uno::RuntimeException, std::exception) override;

        // XPropertyState
        virtual css::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const override;

        // XImageProducer
        virtual void SAL_CALL addConsumer( const css::uno::Reference< css::awt::XImageConsumer >& xConsumer ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeConsumer( const css::uno::Reference< css::awt::XImageConsumer >& xConsumer ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL startProduction(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XSubmissionSupplier
        virtual css::uno::Reference< css::form::submission::XSubmission > SAL_CALL getSubmission() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setSubmission( const css::uno::Reference< css::form::submission::XSubmission >& _submission ) throw (css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

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
        explicit ImageModelMethodGuard( OClickableImageBaseModel& _rModel )
            :GuardBase( _rModel.getMutex( OClickableImageBaseModel::GuardAccess() ) )
        {
            if ( nullptr == _rModel.getImageProducer( OClickableImageBaseModel::GuardAccess() ) )
                throw css::lang::DisposedException(
                    OUString(),
                    static_cast< css::form::XImageProducerSupplier* >( &_rModel )
                );
        }
    };


    // OClickableImageBaseControl

    typedef ::cppu::ImplHelper3 <   css::form::XApproveActionBroadcaster
                                ,   css::form::submission::XSubmission
                                ,   css::frame::XDispatchProviderInterception
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
        virtual void SAL_CALL submit(  ) throw (css::util::VetoException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL submitWithInteraction( const css::uno::Reference< css::task::XInteractionHandler >& aHandler ) throw (css::util::VetoException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addSubmissionVetoListener( const css::uno::Reference< css::form::submission::XSubmissionVetoListener >& listener ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeSubmissionVetoListener( const css::uno::Reference< css::form::submission::XSubmissionVetoListener >& listener ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        using OControl::disposing;

    public:
        OClickableImageBaseControl(
            const css::uno::Reference< css::uno::XComponentContext>& _rxFactory,
            const OUString& _aService);
        virtual ~OClickableImageBaseControl();

    protected:
        // UNO Binding
        DECLARE_UNO3_AGG_DEFAULTS(OClickableImageBaseControl, OControl)
        virtual css::uno::Any SAL_CALL queryAggregation(const css::uno::Type& _rType) throw(css::uno::RuntimeException, std::exception) override;

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

        // css::form::XApproveActionBroadcaster
        virtual void SAL_CALL addApproveActionListener(const css::uno::Reference< css::form::XApproveActionListener>& _rxListener)
            throw(css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeApproveActionListener(const css::uno::Reference< css::form::XApproveActionListener>& _rxListener)
            throw(css::uno::RuntimeException, std::exception) override;

        // XDispatchProviderInterception
        virtual void SAL_CALL registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;

    protected:
        virtual void actionPerformed_Impl( bool bNotifyListener, const css::awt::MouseEvent& rEvt );

        css::uno::Sequence< css::uno::Type > _getTypes() override;

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
            const css::awt::MouseEvent& _rEvent,
            const css::uno::Reference< css::task::XInteractionHandler >& aHandler
        );
    };

    class OImageProducerThread_Impl: public OComponentEventThread
    {
    protected:

        // This method was called to duplicate the Event by taking its type into account
        virtual css::lang::EventObject* cloneEvent( const css::lang::EventObject* _pEvt ) const override;

        // Process an Event.
        // The mutex is not locked, pCompImpl stays valid in any case
        virtual void processEvent( ::cppu::OComponentHelper *pCompImpl,
                                const css::lang::EventObject*,
                                const css::uno::Reference< css::awt::XControl>&,
                                bool ) override;

    public:
        explicit OImageProducerThread_Impl( OClickableImageBaseControl *pControl ) :
            OComponentEventThread( pControl )
        {}

        void addEvent() { css::lang::EventObject aEvt; OComponentEventThread::addEvent( &aEvt ); }

    protected:
        using OComponentEventThread::addEvent;
    };


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_CLICKABLEIMAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
