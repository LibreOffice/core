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

#include "clickableimage.hxx"
#include "controlfeatureinterception.hxx"
#include "urltransformer.hxx"
#include "componenttools.hxx"
#include <com/sun/star/form/XSubmit.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/ActionEvent.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicObject.hpp>
#include <com/sun/star/util/VetoException.hpp>
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <osl/mutex.hxx>
#include "services.hxx"
#include <comphelper/container.hxx>
#include <comphelper/listenernotification.hxx>
#include <comphelper/processfactory.hxx>
#include <svtools/imageresourceaccess.hxx>
#define LOCAL_URL_PREFIX    '#'


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::form::submission;
    using namespace ::com::sun::star::graphic;
    using ::com::sun::star::awt::MouseEvent;
    using ::com::sun::star::task::XInteractionHandler;


    // OClickableImageBaseControl


    Sequence<Type> OClickableImageBaseControl::_getTypes()
    {
        static Sequence<Type> aTypes;
        if (!aTypes.getLength())
            aTypes = concatSequences(OControl::_getTypes(), OClickableImageBaseControl_BASE::getTypes());
        return aTypes;
    }


    OClickableImageBaseControl::OClickableImageBaseControl(const Reference<XComponentContext>& _rxFactory, const OUString& _aService)
        :OControl(_rxFactory, _aService)
        ,m_aSubmissionVetoListeners( m_aMutex )
        ,m_aApproveActionListeners( m_aMutex )
        ,m_aActionListeners( m_aMutex )
    {
        m_pFeatureInterception.reset( new ControlFeatureInterception( _rxFactory ) );
    }


    OClickableImageBaseControl::~OClickableImageBaseControl()
    {
        if (!OComponentHelper::rBHelper.bDisposed)
        {
            acquire();
            dispose();
        }
    }

    // UNO Binding

    Any SAL_CALL OClickableImageBaseControl::queryAggregation(const Type& _rType)
    {
        Any aReturn = OControl::queryAggregation(_rType);
        if (!aReturn.hasValue())
            aReturn = OClickableImageBaseControl_BASE::queryInterface(_rType);
        return aReturn;
    }

    // XApproveActionBroadcaster

    void OClickableImageBaseControl::addApproveActionListener(
            const Reference<XApproveActionListener>& l)
    {
        m_aApproveActionListeners.addInterface(l);
    }


    void OClickableImageBaseControl::removeApproveActionListener(
            const Reference<XApproveActionListener>& l)
    {
        m_aApproveActionListeners.removeInterface(l);
    }


    void SAL_CALL OClickableImageBaseControl::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor )
    {
        m_pFeatureInterception->registerDispatchProviderInterceptor( _rxInterceptor  );
    }


    void SAL_CALL OClickableImageBaseControl::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor )
    {
        m_pFeatureInterception->releaseDispatchProviderInterceptor( _rxInterceptor  );
    }

    // OComponentHelper

    void OClickableImageBaseControl::disposing()
    {
        EventObject aEvent( static_cast< XWeak* >( this ) );
        m_aApproveActionListeners.disposeAndClear( aEvent );
        m_aActionListeners.disposeAndClear( aEvent );
        m_aSubmissionVetoListeners.disposeAndClear( aEvent );
        m_pFeatureInterception->dispose();

        {
            ::osl::MutexGuard aGuard( m_aMutex );
            m_pThread.clear();
        }

        OControl::disposing();
    }


    OImageProducerThread_Impl* OClickableImageBaseControl::getImageProducerThread()
    {
        if ( !m_pThread.is() )
        {
            m_pThread = new OImageProducerThread_Impl( this );
            m_pThread->create();
        }
        return m_pThread.get();
    }


    bool OClickableImageBaseControl::approveAction( )
    {
        bool bCancelled = false;
        EventObject aEvent( static_cast< XWeak* >( this ) );

        ::comphelper::OInterfaceIteratorHelper2 aIter( m_aApproveActionListeners );
        while( !bCancelled && aIter.hasMoreElements() )
        {
            // Every approveAction method must be thread-safe!
            if( !static_cast< XApproveActionListener* >( aIter.next() )->approveAction( aEvent ) )
                bCancelled = true;
        }

        return !bCancelled;
    }


    // This method is also called from a thread and thus must be thread-safe.
    void OClickableImageBaseControl::actionPerformed_Impl(bool bNotifyListener, const MouseEvent& rEvt)
    {
        if( bNotifyListener )
        {
            if ( !approveAction() )
                return;
        }

        // Whether the rest of the code is thread-safe, one can't tell. Therefore
        // we do most of the work on a locked solar mutex.
        Reference<XPropertySet>  xSet;
        Reference< XInterface > xModelsParent;
        FormButtonType eButtonType = FormButtonType_PUSH;
        {
            SolarMutexGuard aGuard;

            // Get parent
            Reference<XFormComponent>  xComp(getModel(), UNO_QUERY);
            if (!xComp.is())
                return;

            xModelsParent = xComp->getParent();
            if (!xModelsParent.is())
                return;

            // Which button type?
            xSet.set(xComp, css::uno::UNO_QUERY);
            if ( !xSet.is() )
                return;
            xSet->getPropertyValue(PROPERTY_BUTTONTYPE) >>= eButtonType;
        }

        switch (eButtonType)
        {
            case FormButtonType_RESET:
            {
                // Reset methods must be thread-safe!
                Reference<XReset>  xReset(xModelsParent, UNO_QUERY);
                if (!xReset.is())
                    return;

                xReset->reset();
            }
            break;

            case FormButtonType_SUBMIT:
            {
                // if some outer component can provide an interaction handler, use it
                Reference< XInteractionHandler > xHandler( m_pFeatureInterception->queryDispatch( "private:/InteractionHandler" ), UNO_QUERY );
                try
                {
                    implSubmit( rEvt, xHandler );
                }
                catch( const Exception& )
                {
                    // ignore
                }
            }
            break;

            case FormButtonType_URL:
            {
                SolarMutexGuard aGuard;

                Reference< XModel >  xModel = getXModel(xModelsParent);
                if (!xModel.is())
                    return;


                // Execute URL now
                Reference< XController >  xController = xModel->getCurrentController();
                if (!xController.is())
                    return;

                Reference< XFrame >  xFrame = xController->getFrame();
                if( !xFrame.is() )
                    return;

                URL aURL;
                aURL.Complete =
                    getString(xSet->getPropertyValue(PROPERTY_TARGET_URL));

                if (!aURL.Complete.isEmpty() && (LOCAL_URL_PREFIX == aURL.Complete[0]))
                {   // FIXME: The URL contains a local URL only. Since the URLTransformer does not handle this case correctly
                    // (it can't: it does not know the document URL), we have to take care for this ourself.
                    // The real solution would be to not allow such relative URLs (there is a rule that at runtime, all
                    // URLs have to be absolute), but for compatibility reasons this is no option.
                    // The more as the user does not want to see a local URL as "file://<path>/<document>#mark" if it
                    // could be "#mark" as well.
                    // If we someday say that this hack (yes, it's kind of a hack) is not sustainable anymore, the complete
                    // solution would be:
                    // * recognize URLs consisting of a mark only while _reading_ the document
                    // * for this, allow the INetURLObject (which at the moment is invoked when reading URLs) to
                    //   transform such mark-only URLs into correct absolute URLs
                    // * at the UI, show only the mark
                    // * !!! recognize every SAVEAS on the document, so the absolute URL can be adjusted. This seems
                    // rather impossible !!!
                    aURL.Mark = aURL.Complete;
                    aURL.Complete = xModel->getURL();
                    aURL.Complete += aURL.Mark;
                }

                bool bDispatchUrlInternal = false;
                xSet->getPropertyValue(PROPERTY_DISPATCHURLINTERNAL) >>= bDispatchUrlInternal;
                if ( bDispatchUrlInternal )
                {
                    m_pFeatureInterception->getTransformer().parseSmartWithAsciiProtocol( aURL, INET_FILE_SCHEME );

                    OUString aTargetFrame;
                    xSet->getPropertyValue(PROPERTY_TARGET_FRAME) >>= aTargetFrame;

                    Reference< XDispatch >  xDisp = Reference< XDispatchProvider > (xFrame,UNO_QUERY)->queryDispatch( aURL, aTargetFrame,
                            FrameSearchFlag::SELF | FrameSearchFlag::PARENT |
                            FrameSearchFlag::SIBLINGS | FrameSearchFlag::CREATE );

                    Sequence<PropertyValue> aArgs(1);
                    PropertyValue& rProp = aArgs.getArray()[0];
                    rProp.Name = "Referer";
                    rProp.Value <<= xModel->getURL();

                    if (xDisp.is())
                        xDisp->dispatch( aURL, aArgs );
                }
                else
                {
                    URL aHyperLink = m_pFeatureInterception->getTransformer().getStrictURLFromAscii( ".uno:OpenHyperlink" );

                    Reference< XDispatch >  xDisp = Reference< XDispatchProvider > (xFrame,UNO_QUERY)->queryDispatch(aHyperLink, OUString() , 0);

                    if ( xDisp.is() )
                    {
                        Sequence<PropertyValue> aProps(3);
                        aProps[0].Name  = "URL";
                        aProps[0].Value <<= aURL.Complete;

                        aProps[1].Name  = "FrameName";
                        aProps[1].Value = xSet->getPropertyValue(PROPERTY_TARGET_FRAME);

                        aProps[2].Name  = "Referer";
                        aProps[2].Value <<= xModel->getURL();

                        xDisp->dispatch( aHyperLink, aProps );
                    }
                }
            }   break;
            default:
            {
                // notify the action listeners for a push button
                ActionEvent aEvt(static_cast<XWeak*>(this), m_aActionCommand);
                m_aActionListeners.notifyEach( &XActionListener::actionPerformed, aEvt );
            }
        }
    }


    void SAL_CALL OClickableImageBaseControl::addSubmissionVetoListener( const Reference< submission::XSubmissionVetoListener >& listener )
    {
        m_aSubmissionVetoListeners.addInterface( listener );
    }


    void SAL_CALL OClickableImageBaseControl::removeSubmissionVetoListener( const Reference< submission::XSubmissionVetoListener >& listener )
    {
        m_aSubmissionVetoListeners.removeInterface( listener );
    }


    void SAL_CALL OClickableImageBaseControl::submitWithInteraction( const Reference< XInteractionHandler >& _rxHandler )
    {
        implSubmit( MouseEvent(), _rxHandler );
    }


    void SAL_CALL OClickableImageBaseControl::submit(  )
    {
        implSubmit( MouseEvent(), nullptr );
    }


    Sequence< OUString > SAL_CALL OClickableImageBaseControl::getSupportedServiceNames(  )
    {
        Sequence< OUString > aSupported = OControl::getSupportedServiceNames();
        aSupported.realloc( aSupported.getLength() + 1 );

        OUString* pArray = aSupported.getArray();
        pArray[ aSupported.getLength() - 1 ] = FRM_SUN_CONTROL_SUBMITBUTTON;

        return aSupported;
    }


    void OClickableImageBaseControl::implSubmit( const MouseEvent& _rEvent, const Reference< XInteractionHandler >& _rxHandler )
    {
        try
        {
            // allow the veto listeners to join the game
            m_aSubmissionVetoListeners.notifyEach( &XSubmissionVetoListener::submitting, EventObject( *this ) );

            // see whether there's an "submit interceptor" set at our model
            Reference< submission::XSubmissionSupplier > xSubmissionSupp( getModel(), UNO_QUERY );
            Reference< XSubmission > xSubmission;
            if ( xSubmissionSupp.is() )
                xSubmission = xSubmissionSupp->getSubmission();

            if ( xSubmission.is() )
            {
                if ( !_rxHandler.is() )
                    xSubmission->submit();
                else
                    xSubmission->submitWithInteraction( _rxHandler );
            }
            else
            {
                // no "interceptor" -> ordinary (old-way) submission
                Reference< XChild > xChild( getModel(), UNO_QUERY );
                Reference< XSubmit > xParentSubmission;
                if ( xChild.is() )
                    xParentSubmission.set(xChild->getParent(), css::uno::UNO_QUERY);
                if ( xParentSubmission.is() )
                    xParentSubmission->submit( this, _rEvent );
            }
        }
        catch( const VetoException& )
        {
            // allowed to leave
            throw;
        }
        catch( const RuntimeException& )
        {
            // allowed to leave
            throw;
        }
        catch( const WrappedTargetException& )
        {
            // allowed to leave
            throw;
        }
        catch( const Exception& e )
        {
            OSL_FAIL( "OClickableImageBaseControl::implSubmit: caught an unknown exception!" );
            throw WrappedTargetException( OUString(), *this, makeAny( e ) );
        }
    }


    // OClickableImageBaseModel


    Sequence<Type> OClickableImageBaseModel::_getTypes()
    {
        return concatSequences(
            OControlModel::_getTypes(),
            OClickableImageBaseModel_Base::getTypes()
        );
    }


    OClickableImageBaseModel::OClickableImageBaseModel( const Reference< XComponentContext >& _rxFactory, const OUString& _rUnoControlModelTypeName,
            const OUString& rDefault )
        :OControlModel( _rxFactory, _rUnoControlModelTypeName, rDefault )
        ,OPropertyChangeListener(m_aMutex)
        ,m_xGraphicObject()
        ,m_pMedium(nullptr)
        ,m_bDispatchUrlInternal(false)
        ,m_bDownloading(false)
        ,m_bProdStarted(false)
    {
        implConstruct();
        m_eButtonType = FormButtonType_PUSH;
    }


    OClickableImageBaseModel::OClickableImageBaseModel( const OClickableImageBaseModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
        :OControlModel( _pOriginal, _rxFactory )
        ,OPropertyChangeListener( m_aMutex )
        ,m_xGraphicObject( _pOriginal->m_xGraphicObject )
        ,m_pMedium( nullptr )
        ,m_bDispatchUrlInternal(false)
        ,m_bDownloading( false )
        ,m_bProdStarted( false )
    {
        implConstruct();

        // copy properties
        m_eButtonType           = _pOriginal->m_eButtonType;
        m_sTargetURL            = _pOriginal->m_sTargetURL;
        m_sTargetFrame          = _pOriginal->m_sTargetFrame;
        m_bDispatchUrlInternal  = _pOriginal->m_bDispatchUrlInternal;
    }


    void OClickableImageBaseModel::implInitializeImageURL( )
    {
        osl_atomic_increment( &m_refCount );
        {
            // simulate a propertyChanged event for the ImageURL
            Any aImageURL;
            getFastPropertyValue( aImageURL, PROPERTY_ID_IMAGE_URL );
            _propertyChanged( PropertyChangeEvent( *this, PROPERTY_IMAGE_URL, false, PROPERTY_ID_IMAGE_URL, Any( ), aImageURL ) );
        }
        osl_atomic_decrement( &m_refCount );
    }


    void OClickableImageBaseModel::implConstruct()
    {
        m_xProducer = new ImageProducer;
        m_xProducer->SetDoneHdl( LINK( this, OClickableImageBaseModel, OnImageImportDone ) );
        osl_atomic_increment( &m_refCount );
        {
            if ( m_xAggregateSet.is() )
            {
                OPropertyChangeMultiplexer* pMultiplexer = new OPropertyChangeMultiplexer( this, m_xAggregateSet );
                pMultiplexer->addProperty( PROPERTY_IMAGE_URL );
            }
        }
        osl_atomic_decrement(&m_refCount);
    }


    OClickableImageBaseModel::~OClickableImageBaseModel()
    {
        if (!OComponentHelper::rBHelper.bDisposed)
        {
            acquire();
            dispose();
        }
        DBG_ASSERT(m_pMedium == nullptr, "OClickableImageBaseModel::~OClickableImageBaseModel : leaving a memory leak ...");
        // This should be cleaned up at least in the dispose

    }

    // XImageProducer

    void SAL_CALL OClickableImageBaseModel::addConsumer( const Reference< XImageConsumer >& _rxConsumer )
    {
        ImageModelMethodGuard aGuard( *this );
        GetImageProducer()->addConsumer( _rxConsumer );
    }


    void SAL_CALL OClickableImageBaseModel::removeConsumer( const Reference< XImageConsumer >& _rxConsumer )
    {
        ImageModelMethodGuard aGuard( *this );
        GetImageProducer()->removeConsumer( _rxConsumer );
    }


    void SAL_CALL OClickableImageBaseModel::startProduction(  )
    {
        ImageModelMethodGuard aGuard( *this );
        GetImageProducer()->startProduction();
    }


    Reference< submission::XSubmission > SAL_CALL OClickableImageBaseModel::getSubmission()
    {
        return m_xSubmissionDelegate;
    }


    void SAL_CALL OClickableImageBaseModel::setSubmission( const Reference< submission::XSubmission >& _submission )
    {
        m_xSubmissionDelegate = _submission;
    }


    Sequence< OUString > SAL_CALL OClickableImageBaseModel::getSupportedServiceNames(  )
    {
        Sequence< OUString > aSupported = OControlModel::getSupportedServiceNames();
        aSupported.realloc( aSupported.getLength() + 1 );

        OUString* pArray = aSupported.getArray();
        pArray[ aSupported.getLength() - 1 ] = FRM_SUN_COMPONENT_SUBMITBUTTON;

        return aSupported;
    }

    // OComponentHelper

    void OClickableImageBaseModel::disposing()
    {
        OControlModel::disposing();
        if (m_pMedium)
        {
            delete m_pMedium;
            m_pMedium = nullptr;
        }

        m_xProducer.clear();
    }


    Any SAL_CALL OClickableImageBaseModel::queryAggregation(const Type& _rType)
    {
        // order matters:
        // we definitely want to "override" the XImageProducer interface of our aggregate,
        // thus check OClickableImageBaseModel_Base (which provides this) first
        Any aReturn = OClickableImageBaseModel_Base::queryInterface( _rType );

        // BUT: _don't_ let it feel responsible for the XTypeProvider interface
        // (as this is implemented by our base class in the proper way)
        if  (   _rType.equals( cppu::UnoType<XTypeProvider>::get() )
            ||  !aReturn.hasValue()
            )
            aReturn = OControlModel::queryAggregation( _rType );

        return aReturn;
    }


    void OClickableImageBaseModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
    {
        switch (nHandle)
        {
            case PROPERTY_ID_BUTTONTYPE             : rValue <<= m_eButtonType; break;
            case PROPERTY_ID_TARGET_URL             : rValue <<= m_sTargetURL; break;
            case PROPERTY_ID_TARGET_FRAME           : rValue <<= m_sTargetFrame; break;
            case PROPERTY_ID_DISPATCHURLINTERNAL    : rValue <<= m_bDispatchUrlInternal; break;
            default:
                OControlModel::getFastPropertyValue(rValue, nHandle);
        }
    }


    void OClickableImageBaseModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue)
    {
        switch (nHandle)
        {
            case PROPERTY_ID_BUTTONTYPE :
                DBG_ASSERT(rValue.has<FormButtonType>(), "OClickableImageBaseModel::setFastPropertyValue_NoBroadcast : invalid type !" );
                rValue >>= m_eButtonType;
                break;

            case PROPERTY_ID_TARGET_URL :
                DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_STRING, "OClickableImageBaseModel::setFastPropertyValue_NoBroadcast : invalid type !" );
                rValue >>= m_sTargetURL;
                break;

            case PROPERTY_ID_TARGET_FRAME :
                DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_STRING, "OClickableImageBaseModel::setFastPropertyValue_NoBroadcast : invalid type !" );
                rValue >>= m_sTargetFrame;
                break;

            case PROPERTY_ID_DISPATCHURLINTERNAL:
                DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_BOOLEAN, "OClickableImageBaseModel::setFastPropertyValue_NoBroadcast : invalid type !" );
                rValue >>= m_bDispatchUrlInternal;
                break;

            default:
                OControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue);
        }
    }


    sal_Bool OClickableImageBaseModel::convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue)
    {
        switch (nHandle)
        {
            case PROPERTY_ID_BUTTONTYPE :
                return tryPropertyValueEnum( rConvertedValue, rOldValue, rValue, m_eButtonType );

            case PROPERTY_ID_TARGET_URL :
                return tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sTargetURL);

            case PROPERTY_ID_TARGET_FRAME :
                return tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sTargetFrame);

            case PROPERTY_ID_DISPATCHURLINTERNAL :
                return tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bDispatchUrlInternal);

            default:
                return OControlModel::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);
        }
    }


    void OClickableImageBaseModel::StartProduction()
    {
        ImageProducer *pImgProd = GetImageProducer();
        // grab the ImageURL
        OUString sURL;
        getPropertyValue("ImageURL") >>= sURL;
        if (!m_pMedium)
        {
            if ( ::svt::GraphicAccess::isSupportedURL( sURL )  )
                pImgProd->SetImage( sURL );
            else
                // caution: the medium may be NULL if somebody gave us an invalid URL to work with
                pImgProd->SetImage(OUString());
            m_bDownloading = false;
            return;
        }
        if (m_pMedium->GetErrorCode()==0)
        {
            SvStream* pStream = m_pMedium->GetInStream();

            pImgProd->SetImage(*pStream);
            pImgProd->startProduction();
            m_bProdStarted = true;
        }
        else
        {
            pImgProd->SetImage(OUString());
            delete m_pMedium;
            m_pMedium = nullptr;
            m_bDownloading = false;
        }
    }


    void OClickableImageBaseModel::SetURL( const OUString& rURL )
    {
        if (m_pMedium || rURL.isEmpty())
        {
            // Free the stream at the Producer, before the medium is deleted
            GetImageProducer()->SetImage(OUString());
            delete m_pMedium;
            m_pMedium = nullptr;
        }

        // the SfxMedium is not allowed to be created with an invalid URL, so we have to check this first
        INetURLObject aUrl(rURL);
        if (INetProtocol::NotValid == aUrl.GetProtocol())
            // we treat an invalid URL like we would treat no URL
            return;

        if (!rURL.isEmpty() && !::svt::GraphicAccess::isSupportedURL( rURL ) )
       {
            delete m_pMedium;

            m_pMedium = new SfxMedium(rURL, StreamMode::STD_READ);

            // Find the XModel to get to the Object shell or at least the
            // Referer.
            // There's only a Model if we load HTML documents and the URL is
            // changed in a document that is already loaded. There's no way
            // we can get to the Model during loading.
            Reference< XModel >  xModel;
            css::uno::Reference<css::uno::XInterface>  xIfc( *this );
            while( !xModel.is() && xIfc.is() )
            {
                Reference<XChild>  xChild( xIfc, UNO_QUERY );
                xIfc = xChild->getParent();
                xModel.set(xIfc, css::uno::UNO_QUERY);
            }

            // Search for the Object shell by iterating over all Object shells
            // and comparing their XModel to ours.
            // As an optimization, we try the current Object shell first.
            SfxObjectShell *pObjSh = nullptr;

            if( xModel.is() )
            {
                SfxObjectShell *pTestObjSh = SfxObjectShell::Current();
                if( pTestObjSh )
                {
                    Reference< XModel >  xTestModel = pTestObjSh->GetModel();
                    if( xTestModel == xModel )
                        pObjSh = pTestObjSh;
                }
                if( !pObjSh )
                {
                    pTestObjSh = SfxObjectShell::GetFirst();
                    while( !pObjSh && pTestObjSh )
                    {
                        Reference< XModel > xTestModel = pTestObjSh->GetModel();
                        if( xTestModel == xModel )
                            pObjSh = pTestObjSh;
                        else
                            pTestObjSh = SfxObjectShell::GetNext( *pTestObjSh );
                    }
                }
            }

    #ifdef USE_REGISTER_TRANSFER
            if( pObjSh )
            {
                // Transfer target frame, so that javascript: URLs
                // can also be "loaded"
                const SfxMedium *pShMedium = pObjSh->GetMedium();
                if( pShMedium )
                    m_pMedium->SetLoadTargetFrame(pShMedium->GetLoadTargetFrame());
            }
    #else
            if( pObjSh )
            {
                // Transfer target frame, so that javascript: URLs
                // can also be "loaded"
                const SfxMedium *pShMedium = pObjSh->GetMedium();
                if( pShMedium )
                    m_pMedium->SetLoadTargetFrame(pShMedium->GetLoadTargetFrame());
            }
    #endif

            // Set downloading flag to true. They will be Data Available Links,
            // if get to the pending staus.
            m_bDownloading = true;
            m_bProdStarted = false;

            // Kick off download (caution: can be synchronous).
            m_pMedium->Download(LINK(this, OClickableImageBaseModel, DownloadDoneLink));
        }
        else
        {
            if ( ::svt::GraphicAccess::isSupportedURL( rURL )  )
                GetImageProducer()->SetImage( rURL );
            GetImageProducer()->startProduction();
        }
    }


    void OClickableImageBaseModel::DataAvailable()
    {
        if (!m_bProdStarted)
            StartProduction();

        GetImageProducer()->NewDataAvailable();
    }


    void OClickableImageBaseModel::DownloadDone()
    {
        DataAvailable();
        m_bDownloading = false;
    }


    IMPL_LINK_NOARG( OClickableImageBaseModel, DownloadDoneLink, void*, void )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        DownloadDone();
    }


    void OClickableImageBaseModel::_propertyChanged( const PropertyChangeEvent& rEvt )
    {
        // If a URL was set, it needs to be passed onto the ImageProducer.
        ::osl::MutexGuard aGuard(m_aMutex);
        SetURL( getString(rEvt.NewValue) );
    }


    Any OClickableImageBaseModel::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
    {
        switch (nHandle)
        {
            case PROPERTY_ID_BUTTONTYPE             : return makeAny( FormButtonType_PUSH );
            case PROPERTY_ID_TARGET_URL             :
            case PROPERTY_ID_TARGET_FRAME           : return makeAny( OUString() );
            case PROPERTY_ID_DISPATCHURLINTERNAL    : return makeAny( false );
            default:
                return OControlModel::getPropertyDefaultByHandle(nHandle);
        }
    }

    IMPL_LINK( OClickableImageBaseModel, OnImageImportDone, Graphic*, i_pGraphic, void )
    {
        const Reference< XGraphic > xGraphic( i_pGraphic != nullptr ? Graphic(i_pGraphic->GetBitmapEx()).GetXGraphic() : nullptr );
        if ( !xGraphic.is() )
        {
            m_xGraphicObject.clear();
        }
        else
        {
            m_xGraphicObject = css::graphic::GraphicObject::create( m_xContext );
            m_xGraphicObject->setGraphic( xGraphic );
        }
    }


    // OImageProducerThread_Impl


    EventObject* OImageProducerThread_Impl::cloneEvent( const EventObject* _pEvt ) const
    {
        return new EventObject( *_pEvt );
    }


    void OImageProducerThread_Impl::processEvent( ::cppu::OComponentHelper *pCompImpl,
                                                const EventObject* pEvt,
                                                const Reference<XControl>&,
                                                bool )
    {
        static_cast<OClickableImageBaseControl *>(pCompImpl)->actionPerformed_Impl( true, *static_cast<const MouseEvent *>(pEvt) );
    }


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
