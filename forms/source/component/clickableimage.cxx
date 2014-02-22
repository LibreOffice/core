/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    using ::com::sun::star::awt::MouseEvent;
    using ::com::sun::star::task::XInteractionHandler;

    
    
    
    
    Sequence<Type> OClickableImageBaseControl::_getTypes()
    {
        static Sequence<Type> aTypes;
        if (!aTypes.getLength())
            aTypes = concatSequences(OControl::_getTypes(), OClickableImageBaseControl_BASE::getTypes());
        return aTypes;
    }

    
    OClickableImageBaseControl::OClickableImageBaseControl(const Reference<XComponentContext>& _rxFactory, const OUString& _aService)
        :OControl(_rxFactory, _aService)
        ,m_pThread(NULL)
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

    
    
    Any SAL_CALL OClickableImageBaseControl::queryAggregation(const Type& _rType) throw (RuntimeException)
    {
        Any aReturn = OControl::queryAggregation(_rType);
        if (!aReturn.hasValue())
            aReturn = OClickableImageBaseControl_BASE::queryInterface(_rType);
        return aReturn;
    }

    
    
    void OClickableImageBaseControl::addApproveActionListener(
            const Reference<XApproveActionListener>& l) throw( RuntimeException )
    {
        m_aApproveActionListeners.addInterface(l);
    }

    
    void OClickableImageBaseControl::removeApproveActionListener(
            const Reference<XApproveActionListener>& l) throw( RuntimeException )
    {
        m_aApproveActionListeners.removeInterface(l);
    }

    
    void SAL_CALL OClickableImageBaseControl::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException)
    {
        m_pFeatureInterception->registerDispatchProviderInterceptor( _rxInterceptor  );
    }

    
    void SAL_CALL OClickableImageBaseControl::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException)
    {
        m_pFeatureInterception->releaseDispatchProviderInterceptor( _rxInterceptor  );
    }

    
    
    void OClickableImageBaseControl::disposing()
    {
        EventObject aEvent( static_cast< XWeak* >( this ) );
        m_aApproveActionListeners.disposeAndClear( aEvent );
        m_aActionListeners.disposeAndClear( aEvent );
        m_aSubmissionVetoListeners.disposeAndClear( aEvent );
        m_pFeatureInterception->dispose();

        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if( m_pThread )
            {
                m_pThread->release();
                m_pThread = NULL;
            }
        }

        OControl::disposing();
    }

    
    OImageProducerThread_Impl* OClickableImageBaseControl::getImageProducerThread()
    {
        if ( !m_pThread )
        {
            m_pThread = new OImageProducerThread_Impl( this );
            m_pThread->acquire();
            m_pThread->create();
        }
        return m_pThread;
    }

    
    bool OClickableImageBaseControl::approveAction( )
    {
        sal_Bool bCancelled = sal_False;
        EventObject aEvent( static_cast< XWeak* >( this ) );

        ::cppu::OInterfaceIteratorHelper aIter( m_aApproveActionListeners );
        while( !bCancelled && aIter.hasMoreElements() )
        {
            
            if( !static_cast< XApproveActionListener* >( aIter.next() )->approveAction( aEvent ) )
                bCancelled = sal_True;
        }

        return !bCancelled;
    }

    
    
    
    void OClickableImageBaseControl::actionPerformed_Impl(sal_Bool bNotifyListener, const MouseEvent& rEvt)
    {
        if( bNotifyListener )
        {
            if ( !approveAction() )
                return;
        }

        
        
        Reference<XPropertySet>  xSet;
        Reference< XInterface > xModelsParent;
        FormButtonType eButtonType = FormButtonType_PUSH;
        {
            SolarMutexGuard aGuard;

            
            Reference<XFormComponent>  xComp(getModel(), UNO_QUERY);
            if (!xComp.is())
                return;

            xModelsParent = xComp->getParent();
            if (!xModelsParent.is())
                return;

            
            xSet = xSet.query( xComp );
            if ( !xSet.is() )
                return;
            xSet->getPropertyValue(PROPERTY_BUTTONTYPE) >>= eButtonType;
        }

        switch (eButtonType)
        {
            case FormButtonType_RESET:
            {
                
                Reference<XReset>  xReset(xModelsParent, UNO_QUERY);
                if (!xReset.is())
                    return;

                xReset->reset();
            }
            break;

            case FormButtonType_SUBMIT:
            {
                
                Reference< XInteractionHandler > xHandler( m_pFeatureInterception->queryDispatch( "private:/InteractionHandler" ), UNO_QUERY );
                try
                {
                    implSubmit( rEvt, xHandler );
                }
                catch( const Exception& )
                {
                    
                }
            }
            break;

            case FormButtonType_URL:
            {
                SolarMutexGuard aGuard;

                Reference< XModel >  xModel = getXModel(xModelsParent);
                if (!xModel.is())
                    return;

                
                
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
                {   
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    aURL.Mark = aURL.Complete;
                    aURL.Complete = xModel->getURL();
                    aURL.Complete += aURL.Mark;
                }

                sal_Bool bDispatchUrlInternal = sal_False;
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
                    
                ActionEvent aEvt(static_cast<XWeak*>(this), m_aActionCommand);
                m_aActionListeners.notifyEach( &XActionListener::actionPerformed, aEvt );
            }
        }
    }


    
    void SAL_CALL OClickableImageBaseControl::addSubmissionVetoListener( const Reference< submission::XSubmissionVetoListener >& listener ) throw (NoSupportException, RuntimeException)
    {
        m_aSubmissionVetoListeners.addInterface( listener );
    }

    
    void SAL_CALL OClickableImageBaseControl::removeSubmissionVetoListener( const Reference< submission::XSubmissionVetoListener >& listener ) throw (NoSupportException, RuntimeException)
    {
        m_aSubmissionVetoListeners.removeInterface( listener );
    }

    
    void SAL_CALL OClickableImageBaseControl::submitWithInteraction( const Reference< XInteractionHandler >& _rxHandler ) throw (VetoException, WrappedTargetException, RuntimeException)
    {
        implSubmit( MouseEvent(), _rxHandler );
    }

    
    void SAL_CALL OClickableImageBaseControl::submit(  ) throw (VetoException, WrappedTargetException, RuntimeException)
    {
        implSubmit( MouseEvent(), NULL );
    }

    
    Sequence< OUString > SAL_CALL OClickableImageBaseControl::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        Sequence< OUString > aSupported = OControl::getSupportedServiceNames();
        aSupported.realloc( aSupported.getLength() + 1 );

        OUString* pArray = aSupported.getArray();
        pArray[ aSupported.getLength() - 1 ] = FRM_SUN_CONTROL_SUBMITBUTTON;

        return aSupported;
    }

    
    void OClickableImageBaseControl::implSubmit( const MouseEvent& _rEvent, const Reference< XInteractionHandler >& _rxHandler ) SAL_THROW((VetoException, WrappedTargetException, RuntimeException))
    {
        try
        {
            
            m_aSubmissionVetoListeners.notifyEach( &XSubmissionVetoListener::submitting, EventObject( *this ) );

            
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
                
                Reference< XChild > xChild( getModel(), UNO_QUERY );
                Reference< XSubmit > xParentSubmission;
                if ( xChild.is() )
                    xParentSubmission = xParentSubmission.query( xChild->getParent() );
                if ( xParentSubmission.is() )
                    xParentSubmission->submit( this, _rEvent );
            }
        }
        catch( const VetoException& )
        {
            
            throw;
        }
        catch( const RuntimeException& )
        {
            
            throw;
        }
        catch( const WrappedTargetException& )
        {
            
            throw;
        }
        catch( const Exception& e )
        {
            OSL_FAIL( "OClickableImageBaseControl::implSubmit: caught an unknown exception!" );
            throw WrappedTargetException( OUString(), *this, makeAny( e ) );
        }
    }

    
    
    
    
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
        ,m_pMedium(NULL)
        ,m_pProducer( NULL )
        ,m_bDispatchUrlInternal(sal_False)
        ,m_bDownloading(sal_False)
        ,m_bProdStarted(sal_False)
    {
        implConstruct();
        m_eButtonType = FormButtonType_PUSH;
    }

    
    OClickableImageBaseModel::OClickableImageBaseModel( const OClickableImageBaseModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
        :OControlModel( _pOriginal, _rxFactory )
        ,OPropertyChangeListener( m_aMutex )
        ,m_pMedium( NULL )
        ,m_pProducer( NULL )
        ,m_bDispatchUrlInternal(sal_False)
        ,m_bDownloading( sal_False )
        ,m_bProdStarted( sal_False )
    {
        implConstruct();

        
        m_eButtonType           = _pOriginal->m_eButtonType;
        m_sTargetURL            = _pOriginal->m_sTargetURL;
        m_sTargetFrame          = _pOriginal->m_sTargetFrame;
        m_bDispatchUrlInternal  = _pOriginal->m_bDispatchUrlInternal;
    }

    
    void OClickableImageBaseModel::implInitializeImageURL( )
    {
        osl_atomic_increment( &m_refCount );
        {
            
            Any aImageURL;
            getFastPropertyValue( aImageURL, PROPERTY_ID_IMAGE_URL );
            _propertyChanged( PropertyChangeEvent( *this, PROPERTY_IMAGE_URL, sal_False, PROPERTY_ID_IMAGE_URL, Any( ), aImageURL ) );
        }
        osl_atomic_decrement( &m_refCount );
    }

    
    void OClickableImageBaseModel::implConstruct()
    {
        m_pProducer = new ImageProducer;
        increment( m_refCount );
        {
            m_xProducer = m_pProducer;

            if ( m_xAggregateSet.is() )
            {
                OPropertyChangeMultiplexer* pMultiplexer = new OPropertyChangeMultiplexer( this, m_xAggregateSet );
                pMultiplexer->addProperty( PROPERTY_IMAGE_URL );
            }
        }
        decrement(m_refCount);
    }

    
    OClickableImageBaseModel::~OClickableImageBaseModel()
    {
        if (!OComponentHelper::rBHelper.bDisposed)
        {
            acquire();
            dispose();
        }
        DBG_ASSERT(m_pMedium == NULL, "OClickableImageBaseModel::~OClickableImageBaseModel : leaving a memory leak ...");
            

    }

    
    
    void SAL_CALL OClickableImageBaseModel::addConsumer( const Reference< XImageConsumer >& _rxConsumer ) throw (RuntimeException)
    {
        ImageModelMethodGuard aGuard( *this );
        GetImageProducer()->addConsumer( _rxConsumer );
    }

    
    void SAL_CALL OClickableImageBaseModel::removeConsumer( const Reference< XImageConsumer >& _rxConsumer ) throw (RuntimeException)
    {
        ImageModelMethodGuard aGuard( *this );
        GetImageProducer()->removeConsumer( _rxConsumer );
    }

    
    void SAL_CALL OClickableImageBaseModel::startProduction(  ) throw (RuntimeException)
    {
        ImageModelMethodGuard aGuard( *this );
        GetImageProducer()->startProduction();
    }

    
    Reference< submission::XSubmission > SAL_CALL OClickableImageBaseModel::getSubmission() throw (RuntimeException)
    {
        return m_xSubmissionDelegate;
    }

    
    void SAL_CALL OClickableImageBaseModel::setSubmission( const Reference< submission::XSubmission >& _submission ) throw (RuntimeException)
    {
        m_xSubmissionDelegate = _submission;
    }

    
    Sequence< OUString > SAL_CALL OClickableImageBaseModel::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        Sequence< OUString > aSupported = OControlModel::getSupportedServiceNames();
        aSupported.realloc( aSupported.getLength() + 1 );

        OUString* pArray = aSupported.getArray();
        pArray[ aSupported.getLength() - 1 ] = FRM_SUN_COMPONENT_SUBMITBUTTON;

        return aSupported;
    }

    
    
    void OClickableImageBaseModel::disposing()
    {
        OControlModel::disposing();
        if (m_pMedium)
        {
            delete m_pMedium;
            m_pMedium = NULL;
        }

        m_xProducer = NULL;
        m_pProducer = NULL;
    }

    
    Any SAL_CALL OClickableImageBaseModel::queryAggregation(const Type& _rType) throw (RuntimeException)
    {
        
        
        
        Any aReturn = OClickableImageBaseModel_Base::queryInterface( _rType );

        
        
        if  (   _rType.equals( ::getCppuType( static_cast< Reference< XTypeProvider >* >( NULL ) ) )
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

    
    void OClickableImageBaseModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) throw ( Exception)
    {
        switch (nHandle)
        {
            case PROPERTY_ID_BUTTONTYPE :
                DBG_ASSERT(isA(rValue, static_cast<FormButtonType*>(NULL)), "OClickableImageBaseModel::setFastPropertyValue_NoBroadcast : invalid type !" );
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
                                throw( IllegalArgumentException )
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
        
        OUString sURL;
        getPropertyValue("ImageURL") >>= sURL;
        if (!m_pMedium)
        {
            if ( ::svt::GraphicAccess::isSupportedURL( sURL )  )
                pImgProd->SetImage( sURL );
            else
                
                pImgProd->SetImage(OUString());
            m_bDownloading = sal_False;
            return;
        }
        if (m_pMedium->GetErrorCode()==0)
        {
            SvStream* pStream = m_pMedium->GetInStream();

            pImgProd->SetImage(*pStream);
            pImgProd->startProduction();
            m_bProdStarted = sal_True;
        }
        else
        {
            pImgProd->SetImage(OUString());
            delete m_pMedium;
            m_pMedium = 0;
            m_bDownloading = sal_False;
        }
    }

    
    void OClickableImageBaseModel::SetURL( const OUString& rURL )
    {
        if (m_pMedium || rURL.isEmpty())
        {
            
            GetImageProducer()->SetImage(OUString());
            delete m_pMedium;
            m_pMedium = NULL;
        }

        
        INetURLObject aUrl(rURL);
        if (INET_PROT_NOT_VALID == aUrl.GetProtocol())
            
            return;

        if (!rURL.isEmpty() && !::svt::GraphicAccess::isSupportedURL( rURL ) )
       {
            if (m_pMedium)
                delete m_pMedium;

            m_pMedium = new SfxMedium(rURL, STREAM_STD_READ);

            
            
            
            
            
            
            Reference< XModel >  xModel;
            InterfaceRef  xIfc( *this );
            while( !xModel.is() && xIfc.is() )
            {
                Reference<XChild>  xChild( xIfc, UNO_QUERY );
                xIfc = xChild->getParent();
                query_interface(xIfc, xModel);
            }

            
            
            
            
            
            SfxObjectShell *pObjSh = 0;

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
                
                
                const SfxMedium *pShMedium = pObjSh->GetMedium();
                if( pShMedium )
                    m_pMedium->SetLoadTargetFrame(pShMedium->GetLoadTargetFrame());
            }
    #else
            if( pObjSh )
            {
                
                
                const SfxMedium *pShMedium = pObjSh->GetMedium();
                if( pShMedium )
                    m_pMedium->SetLoadTargetFrame(pShMedium->GetLoadTargetFrame());
            }
    #endif

            
            
            m_bDownloading = sal_True;
            m_bProdStarted = sal_False;

            
            m_pMedium->DownLoad(STATIC_LINK(this, OClickableImageBaseModel, DownloadDoneLink));
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
        m_bDownloading = sal_False;
    }

    
    IMPL_STATIC_LINK( OClickableImageBaseModel, DownloadDoneLink, void*, EMPTYARG )
    {
        ::osl::MutexGuard aGuard( pThis->m_aMutex );
        pThis->DownloadDone();
        return 0;
    }

    
    void OClickableImageBaseModel::_propertyChanged( const PropertyChangeEvent& rEvt )
        throw( RuntimeException )
    {
        
        
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
            case PROPERTY_ID_DISPATCHURLINTERNAL    : return makeAny( sal_False );
            default:
                return OControlModel::getPropertyDefaultByHandle(nHandle);
        }
    }

    
    
    
    
    EventObject* OImageProducerThread_Impl::cloneEvent( const EventObject* _pEvt ) const
    {
        return new EventObject( *_pEvt );
    }

    
    void OImageProducerThread_Impl::processEvent( ::cppu::OComponentHelper *pCompImpl,
                                                const EventObject* pEvt,
                                                const Reference<XControl>&,
                                                sal_Bool )
    {
        ((OClickableImageBaseControl *)pCompImpl)->actionPerformed_Impl( sal_True, *(MouseEvent *)pEvt );
    }


}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
