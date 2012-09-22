/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <svtools/imageresourceaccess.hxx>
#define LOCAL_URL_PREFIX    '#'

//.........................................................................
namespace frm
{
//.........................................................................

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

    //==================================================================
    // OClickableImageBaseControl
    //==================================================================
    //------------------------------------------------------------------------------
    Sequence<Type> OClickableImageBaseControl::_getTypes()
    {
        static Sequence<Type> aTypes;
        if (!aTypes.getLength())
            aTypes = concatSequences(OControl::_getTypes(), OClickableImageBaseControl_BASE::getTypes());
        return aTypes;
    }

    //------------------------------------------------------------------------------
    OClickableImageBaseControl::OClickableImageBaseControl(const Reference<XMultiServiceFactory>& _rxFactory, const ::rtl::OUString& _aService)
        :OControl(_rxFactory, _aService)
        ,m_pThread(NULL)
        ,m_aSubmissionVetoListeners( m_aMutex )
        ,m_aApproveActionListeners( m_aMutex )
        ,m_aActionListeners( m_aMutex )
    {
        m_pFeatureInterception.reset( new ControlFeatureInterception( _rxFactory ) );
    }

    //------------------------------------------------------------------------------
    OClickableImageBaseControl::~OClickableImageBaseControl()
    {
        if (!OComponentHelper::rBHelper.bDisposed)
        {
            acquire();
            dispose();
        }
    }

    // UNO Anbindung
    //------------------------------------------------------------------------------
    Any SAL_CALL OClickableImageBaseControl::queryAggregation(const Type& _rType) throw (RuntimeException)
    {
        Any aReturn = OControl::queryAggregation(_rType);
        if (!aReturn.hasValue())
            aReturn = OClickableImageBaseControl_BASE::queryInterface(_rType);
        return aReturn;
    }

    // XApproveActionBroadcaster
    //------------------------------------------------------------------------------
    void OClickableImageBaseControl::addApproveActionListener(
            const Reference<XApproveActionListener>& l) throw( RuntimeException )
    {
        m_aApproveActionListeners.addInterface(l);
    }

    //------------------------------------------------------------------------------
    void OClickableImageBaseControl::removeApproveActionListener(
            const Reference<XApproveActionListener>& l) throw( RuntimeException )
    {
        m_aApproveActionListeners.removeInterface(l);
    }

    //--------------------------------------------------------------------
    void SAL_CALL OClickableImageBaseControl::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException)
    {
        m_pFeatureInterception->registerDispatchProviderInterceptor( _rxInterceptor  );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OClickableImageBaseControl::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException)
    {
        m_pFeatureInterception->releaseDispatchProviderInterceptor( _rxInterceptor  );
    }

    // OComponentHelper
    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
    bool OClickableImageBaseControl::approveAction( )
    {
        sal_Bool bCancelled = sal_False;
        EventObject aEvent( static_cast< XWeak* >( this ) );

        ::cppu::OInterfaceIteratorHelper aIter( m_aApproveActionListeners );
        while( !bCancelled && aIter.hasMoreElements() )
        {
            // Jede approveAction-Methode muss thread-safe sein!!!
            if( !static_cast< XApproveActionListener* >( aIter.next() )->approveAction( aEvent ) )
                bCancelled = sal_True;
        }

        return !bCancelled;
    }

    //------------------------------------------------------------------------------
    // Diese Methode wird auch aus einem Thread gerufen und muss deshalb
    // thread-safe sein.
    void OClickableImageBaseControl::actionPerformed_Impl(sal_Bool bNotifyListener, const MouseEvent& rEvt)
    {
        if( bNotifyListener )
        {
            if ( !approveAction() )
                return;
        }

        // Ob der Rest des Codes Thread-Safe ist weiss man nicht genau. Deshalb
        // wird das meiste bei gelocktem Solar-Mutex erledigen.
        Reference<XPropertySet>  xSet;
        Reference< XInterface > xModelsParent;
        FormButtonType eButtonType = FormButtonType_PUSH;
        {
            SolarMutexGuard aGuard;

            // Parent holen
            Reference<XFormComponent>  xComp(getModel(), UNO_QUERY);
            if (!xComp.is())
                return;

            xModelsParent = xComp->getParent();
            if (!xModelsParent.is())
                return;

            // which button type?
            xSet = xSet.query( xComp );
            if ( !xSet.is() )
                return;
            xSet->getPropertyValue(PROPERTY_BUTTONTYPE) >>= eButtonType;
        }

        switch (eButtonType)
        {
            case FormButtonType_RESET:
            {
                // reset-Methoden muessen thread-safe sein!
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

                ///////////////////////////////////////////////////////////////////////
                // Jetzt URL ausfuehren
                Reference< XController >  xController = xModel->getCurrentController();
                if (!xController.is())
                    return;

                Reference< XFrame >  xFrame = xController->getFrame();
                if( !xFrame.is() )
                    return;

                URL aURL;
                aURL.Complete =
                    getString(xSet->getPropertyValue(PROPERTY_TARGET_URL));

                if (!aURL.Complete.isEmpty() && (LOCAL_URL_PREFIX == aURL.Complete.getStr()[0]))
                {   // the URL contains a local URL only. Since the URLTransformer does not handle this case correctly
                    // (it can't: it does not know the document URL), we have to take care for this ourself.
                    // The real solution would be to not allow such relative URLs (there is a rule that at runtime, all
                    // URLs have to be absolute), but for compatibility reasons this is no option.
                    // The more as the user does not want to see a local URL as "file://<path>/<document>#mark" if it
                    // could be "#mark" as well.
                    // If we someday say that this hack (yes, it's kind of a hack) is not sustainable anymore, the complete
                    // solutiuon would be:
                    // * recognize URLs consisting of a mark only while _reading_ the document
                    // * for this, allow the INetURLObject (which at the moment is invoked when reading URLs) to
                    //   transform such mark-only URLs into correct absolute URLs
                    // * at the UI, show only the mark
                    // * !!!! recognize every SAVEAS on the document, so the absolute URL can be adjusted. This seems
                    // rather impossible !!!
                    aURL.Mark = aURL.Complete;
                    aURL.Complete = xModel->getURL();
                    aURL.Complete += aURL.Mark;
                }

                sal_Bool bDispatchUrlInternal = sal_False;
                xSet->getPropertyValue(PROPERTY_DISPATCHURLINTERNAL) >>= bDispatchUrlInternal;
                if ( bDispatchUrlInternal )
                {
                    m_pFeatureInterception->getTransformer().parseSmartWithAsciiProtocol( aURL, INET_FILE_SCHEME );

                    ::rtl::OUString aTargetFrame;
                    xSet->getPropertyValue(PROPERTY_TARGET_FRAME) >>= aTargetFrame;

                    Reference< XDispatch >  xDisp = Reference< XDispatchProvider > (xFrame,UNO_QUERY)->queryDispatch( aURL, aTargetFrame,
                            FrameSearchFlag::SELF | FrameSearchFlag::PARENT |
                            FrameSearchFlag::SIBLINGS | FrameSearchFlag::CREATE );

                    Sequence<PropertyValue> aArgs(1);
                    PropertyValue& rProp = aArgs.getArray()[0];
                    rProp.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Referer") );
                    rProp.Value <<= xModel->getURL();

                    if (xDisp.is())
                        xDisp->dispatch( aURL, aArgs );
                }
                else
                {
                    URL aHyperLink = m_pFeatureInterception->getTransformer().getStrictURLFromAscii( ".uno:OpenHyperlink" );

                    Reference< XDispatch >  xDisp = Reference< XDispatchProvider > (xFrame,UNO_QUERY)->queryDispatch(aHyperLink, ::rtl::OUString() , 0);

                    if ( xDisp.is() )
                    {
                        Sequence<PropertyValue> aProps(3);
                        aProps[0].Name  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"));
                        aProps[0].Value <<= aURL.Complete;

                        aProps[1].Name  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FrameName"));
                        aProps[1].Value = xSet->getPropertyValue(PROPERTY_TARGET_FRAME);

                        aProps[2].Name  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Referer"));
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


    //--------------------------------------------------------------------
    void SAL_CALL OClickableImageBaseControl::addSubmissionVetoListener( const Reference< submission::XSubmissionVetoListener >& listener ) throw (NoSupportException, RuntimeException)
    {
        m_aSubmissionVetoListeners.addInterface( listener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OClickableImageBaseControl::removeSubmissionVetoListener( const Reference< submission::XSubmissionVetoListener >& listener ) throw (NoSupportException, RuntimeException)
    {
        m_aSubmissionVetoListeners.removeInterface( listener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OClickableImageBaseControl::submitWithInteraction( const Reference< XInteractionHandler >& _rxHandler ) throw (VetoException, WrappedTargetException, RuntimeException)
    {
        implSubmit( MouseEvent(), _rxHandler );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OClickableImageBaseControl::submit(  ) throw (VetoException, WrappedTargetException, RuntimeException)
    {
        implSubmit( MouseEvent(), NULL );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OClickableImageBaseControl::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported = OControl::getSupportedServiceNames();
        aSupported.realloc( aSupported.getLength() + 1 );

        ::rtl::OUString* pArray = aSupported.getArray();
        pArray[ aSupported.getLength() - 1 ] = FRM_SUN_CONTROL_SUBMITBUTTON;

        return aSupported;
    }

    //--------------------------------------------------------------------
    void OClickableImageBaseControl::implSubmit( const MouseEvent& _rEvent, const Reference< XInteractionHandler >& _rxHandler ) SAL_THROW((VetoException, WrappedTargetException, RuntimeException))
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
                    xParentSubmission = xParentSubmission.query( xChild->getParent() );
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
            throw WrappedTargetException( ::rtl::OUString(), *this, makeAny( e ) );
        }
    }

    //==================================================================
    // OClickableImageBaseModel
    //==================================================================
    //------------------------------------------------------------------------------
    Sequence<Type> OClickableImageBaseModel::_getTypes()
    {
        return concatSequences(
            OControlModel::_getTypes(),
            OClickableImageBaseModel_Base::getTypes()
        );
    }

    //------------------------------------------------------------------
    DBG_NAME( OClickableImageBaseModel )
    //------------------------------------------------------------------
    OClickableImageBaseModel::OClickableImageBaseModel( const Reference< XMultiServiceFactory >& _rxFactory, const ::rtl::OUString& _rUnoControlModelTypeName,
            const ::rtl::OUString& rDefault )
        :OControlModel( _rxFactory, _rUnoControlModelTypeName, rDefault )
        ,OPropertyChangeListener(m_aMutex)
        ,m_pMedium(NULL)
        ,m_pProducer( NULL )
        ,m_bDispatchUrlInternal(sal_False)
        ,m_bDownloading(sal_False)
        ,m_bProdStarted(sal_False)
    {
        DBG_CTOR( OClickableImageBaseModel, NULL );
        implConstruct();
        m_eButtonType = FormButtonType_PUSH;
    }

    //------------------------------------------------------------------
    OClickableImageBaseModel::OClickableImageBaseModel( const OClickableImageBaseModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
        :OControlModel( _pOriginal, _rxFactory )
        ,OPropertyChangeListener( m_aMutex )
        ,m_pMedium( NULL )
        ,m_pProducer( NULL )
        ,m_bDispatchUrlInternal(sal_False)
        ,m_bDownloading( sal_False )
        ,m_bProdStarted( sal_False )
    {
        DBG_CTOR( OClickableImageBaseModel, NULL );
        implConstruct();

        // copy properties
        m_eButtonType           = _pOriginal->m_eButtonType;
        m_sTargetURL            = _pOriginal->m_sTargetURL;
        m_sTargetFrame          = _pOriginal->m_sTargetFrame;
        m_bDispatchUrlInternal  = _pOriginal->m_bDispatchUrlInternal;
    }

    //------------------------------------------------------------------------------
    void OClickableImageBaseModel::implInitializeImageURL( )
    {
        osl_atomic_increment( &m_refCount );
        {
            // simulate a propertyChanged event for the ImageURL
            Any aImageURL;
            getFastPropertyValue( aImageURL, PROPERTY_ID_IMAGE_URL );
            _propertyChanged( PropertyChangeEvent( *this, PROPERTY_IMAGE_URL, sal_False, PROPERTY_ID_IMAGE_URL, Any( ), aImageURL ) );
        }
        osl_atomic_decrement( &m_refCount );
    }

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
    OClickableImageBaseModel::~OClickableImageBaseModel()
    {
        if (!OComponentHelper::rBHelper.bDisposed)
        {
            acquire();
            dispose();
        }
        DBG_ASSERT(m_pMedium == NULL, "OClickableImageBaseModel::~OClickableImageBaseModel : leaving a memory leak ...");
            // spaetestens im dispose sollte das aufgeraeumt worden sein

        DBG_DTOR( OClickableImageBaseModel, NULL );
    }

    // XImageProducer
    //--------------------------------------------------------------------
    void SAL_CALL OClickableImageBaseModel::addConsumer( const Reference< XImageConsumer >& _rxConsumer ) throw (RuntimeException)
    {
        ImageModelMethodGuard aGuard( *this );
        GetImageProducer()->addConsumer( _rxConsumer );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OClickableImageBaseModel::removeConsumer( const Reference< XImageConsumer >& _rxConsumer ) throw (RuntimeException)
    {
        ImageModelMethodGuard aGuard( *this );
        GetImageProducer()->removeConsumer( _rxConsumer );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OClickableImageBaseModel::startProduction(  ) throw (RuntimeException)
    {
        ImageModelMethodGuard aGuard( *this );
        GetImageProducer()->startProduction();
    }

    //--------------------------------------------------------------------
    Reference< submission::XSubmission > SAL_CALL OClickableImageBaseModel::getSubmission() throw (RuntimeException)
    {
        return m_xSubmissionDelegate;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OClickableImageBaseModel::setSubmission( const Reference< submission::XSubmission >& _submission ) throw (RuntimeException)
    {
        m_xSubmissionDelegate = _submission;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OClickableImageBaseModel::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported = OControlModel::getSupportedServiceNames();
        aSupported.realloc( aSupported.getLength() + 1 );

        ::rtl::OUString* pArray = aSupported.getArray();
        pArray[ aSupported.getLength() - 1 ] = FRM_SUN_COMPONENT_SUBMITBUTTON;

        return aSupported;
    }

    // OComponentHelper
    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
    Any SAL_CALL OClickableImageBaseModel::queryAggregation(const Type& _rType) throw (RuntimeException)
    {
        // order matters:
        // we definately want to "overload" the XImageProducer interface of our aggregate,
        // thus check OClickableImageBaseModel_Base (which provides this) first
        Any aReturn = OClickableImageBaseModel_Base::queryInterface( _rType );

        // BUT: _don't_ let it feel responsible for the XTypeProvider interface
        // (as this is implemented by our base class in the proper way)
        if  (   _rType.equals( ::getCppuType( static_cast< Reference< XTypeProvider >* >( NULL ) ) )
            ||  !aReturn.hasValue()
            )
            aReturn = OControlModel::queryAggregation( _rType );

        return aReturn;
    }

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
    void OClickableImageBaseModel::StartProduction()
    {
        ImageProducer *pImgProd = GetImageProducer();
        // grab the ImageURL
        rtl::OUString sURL;
        getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ImageURL") ) ) >>= sURL;
        if (!m_pMedium)
        {
            if ( ::svt::GraphicAccess::isSupportedURL( sURL )  )
                pImgProd->SetImage( sURL );
            else
                // caution: the medium may be NULL if somebody gave us a invalid URL to work with
                pImgProd->SetImage(String());
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
            pImgProd->SetImage(String());
            delete m_pMedium;
            m_pMedium = 0;
            m_bDownloading = sal_False;
        }
    }

    //------------------------------------------------------------------------------
    void OClickableImageBaseModel::SetURL( const ::rtl::OUString& rURL )
    {
        if (m_pMedium || rURL.isEmpty())
        {
            // Den Stream am Producer freigeben, bevor das Medium geloscht wird.
            GetImageProducer()->SetImage(String());
            delete m_pMedium;
            m_pMedium = NULL;
        }

        // the SfxMedium is not allowed to be created with an invalid URL, so we have to check this first
        INetURLObject aUrl(rURL);
        if (INET_PROT_NOT_VALID == aUrl.GetProtocol())
            // we treat an invalid URL like we would treat no URL
            return;

        if (!rURL.isEmpty() && !::svt::GraphicAccess::isSupportedURL( rURL ) )
       {
            if (m_pMedium)
                delete m_pMedium;

            m_pMedium = new SfxMedium(rURL, STREAM_STD_READ);

            // Das XModel suchen, um an die Object-Shell oder zumindest den
            // Referer zu gelangen.
            // Das Model findet man allerdings nur beim Laden von HTML-Dokumenten
            // und dann, wenn die URL in einem bereits geladenen Dokument
            // geaendert wird. Waehrend des Ladens kommt man nicht an das
            // Model ran.
            Reference< XModel >  xModel;
            InterfaceRef  xIfc( *this );
            while( !xModel.is() && xIfc.is() )
            {
                Reference<XChild>  xChild( xIfc, UNO_QUERY );
                xIfc = xChild->getParent();
                query_interface(xIfc, xModel);
            }

            // Die Object-Shell suchen, indem wir
            // ueber alle Object-Shells iterieren und deren XModel mit dem
            // eigenen vergleichen. Als Optimierung probieren wir aber erstmal
            // die aktuelle Object-Shell.
            // wir unser XModel mit dem aller Object
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
                // Target-Frame uebertragen, damit auch javascript:-URLs
                // "geladen" werden koennen.
                const SfxMedium *pShMedium = pObjSh->GetMedium();
                if( pShMedium )
                    m_pMedium->SetLoadTargetFrame(pShMedium->GetLoadTargetFrame());
            }
    #else
            if( pObjSh )
            {
                // Target-Frame uebertragen, damit auch javascript:-URLs
                // "geladen" werden koennen.
                const SfxMedium *pShMedium = pObjSh->GetMedium();
                if( pShMedium )
                    m_pMedium->SetLoadTargetFrame(pShMedium->GetLoadTargetFrame());
            }
    #endif

            // Downloading-Flag auf sal_True setzen. Es werden dann auch
            // Data-Available-Links, wenn wir in den Pending-Staus gelangen.
            m_bDownloading = sal_True;
            m_bProdStarted = sal_False;

            // Download anstossen (Achtung: Kann auch synchron sein).
            m_pMedium->DownLoad(STATIC_LINK(this, OClickableImageBaseModel, DownloadDoneLink));
        }
        else
        {
            if ( ::svt::GraphicAccess::isSupportedURL( rURL )  )
                GetImageProducer()->SetImage( rURL );
            GetImageProducer()->startProduction();
        }
    }

    //------------------------------------------------------------------------------
    void OClickableImageBaseModel::DataAvailable()
    {
        if (!m_bProdStarted)
            StartProduction();

        GetImageProducer()->NewDataAvailable();
    }

    //------------------------------------------------------------------------------
    void OClickableImageBaseModel::DownloadDone()
    {
        DataAvailable();
        m_bDownloading = sal_False;
    }

    //------------------------------------------------------------------------------
    IMPL_STATIC_LINK( OClickableImageBaseModel, DownloadDoneLink, void*, EMPTYARG )
    {
        ::osl::MutexGuard aGuard( pThis->m_aMutex );
        pThis->DownloadDone();
        return 0;
    }

    //------------------------------------------------------------------------------
    void OClickableImageBaseModel::_propertyChanged( const PropertyChangeEvent& rEvt )
        throw( RuntimeException )
    {
        // Wenn eine URL gesetzt worden ist, muss die noch an den ImageProducer
        // weitergereicht werden.
        ::osl::MutexGuard aGuard(m_aMutex);
        SetURL( getString(rEvt.NewValue) );
    }

    // -----------------------------------------------------------------------------
    Any OClickableImageBaseModel::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
    {
        switch (nHandle)
        {
            case PROPERTY_ID_BUTTONTYPE             : return makeAny( FormButtonType_PUSH );
            case PROPERTY_ID_TARGET_URL             :
            case PROPERTY_ID_TARGET_FRAME           : return makeAny( ::rtl::OUString() );
            case PROPERTY_ID_DISPATCHURLINTERNAL    : return makeAny( sal_False );
            default:
                return OControlModel::getPropertyDefaultByHandle(nHandle);
        }
    }

    //==================================================================
    // OImageProducerThread_Impl
    //==================================================================
    //------------------------------------------------------------------
    EventObject* OImageProducerThread_Impl::cloneEvent( const EventObject* _pEvt ) const
    {
        return new EventObject( *_pEvt );
    }

    //------------------------------------------------------------------
    void OImageProducerThread_Impl::processEvent( ::cppu::OComponentHelper *pCompImpl,
                                                const EventObject* pEvt,
                                                const Reference<XControl>&,
                                                sal_Bool )
    {
        ((OClickableImageBaseControl *)pCompImpl)->actionPerformed_Impl( sal_True, *(MouseEvent *)pEvt );
    }

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
