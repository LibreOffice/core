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

#include "Image.hxx"

#include <com/sun/star/form/XSubmit.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/awt/XActionListener.hpp>

#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/objsh.hxx>
#include <osl/mutex.hxx>

#include "services.hxx"
#include <comphelper/container.hxx>

#include <comphelper/property.hxx>

namespace binfilter {

#define LOCAL_URL_PREFIX	'#'

//.........................................................................
namespace frm
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
//using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;


//==================================================================
// OImageControl
//==================================================================
//------------------------------------------------------------------------------
Sequence<Type> OImageControl::_getTypes()
{
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
        aTypes = concatSequences(OControl::_getTypes(), OImageControl_BASE::getTypes());
    return aTypes;
}

//------------------------------------------------------------------------------
OImageControl::OImageControl(const Reference<XMultiServiceFactory>& _rxFactory, const ::rtl::OUString& _aService)
                :OControl(_rxFactory, _aService)
                ,m_aApproveActionListeners(m_aMutex)
                ,m_aActionListeners(m_aMutex)
                ,m_pThread(NULL)
{
}

//------------------------------------------------------------------------------
OImageControl::~OImageControl()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
}

// UNO Anbindung
//------------------------------------------------------------------------------
Any SAL_CALL OImageControl::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = OImageControl_BASE::queryInterface(_rType);
    return aReturn;
}

// XApproveActionBroadcaster
//------------------------------------------------------------------------------
void OImageControl::addApproveActionListener(
        const Reference<XApproveActionListener>& l) throw( RuntimeException )
{
    m_aApproveActionListeners.addInterface(l);
}

//------------------------------------------------------------------------------
void OImageControl::removeApproveActionListener(
        const Reference<XApproveActionListener>& l) throw( RuntimeException )
{
    m_aApproveActionListeners.removeInterface(l);
}

// OComponentHelper
//------------------------------------------------------------------------------
void OImageControl::disposing()
{
    EventObject aEvt(static_cast<XWeak*>(this));
    m_aApproveActionListeners.disposeAndClear(aEvt);
    m_aActionListeners.disposeAndClear(aEvt);

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
void OImageControl::propertyChange( const PropertyChangeEvent& rEvt )
    throw( RuntimeException )
{
    Reference<XWindowPeer>  xPeer(getPeer());
    if (!xPeer.is())
        return;

    // Wenn eine URL gesetzt worden ist, muss die Hand angezeigt werden
    if( ( rEvt.PropertyName == PROPERTY_TARGET_URL ) &&
        ( rEvt.NewValue.getValueType().getTypeClass() == TypeClass_STRING ) )
    {

        Reference<XPointer> xPoint(
            m_xServiceFactory->createInstance(SRV_AWT_POINTER), UNO_QUERY);
        if (xPoint.is())
            if (getString(rEvt.NewValue).getLength())
                xPoint->setType(SystemPointer::REFHAND);
            else
                xPoint->setType(SystemPointer::ARROW);

        xPeer->setPointer(xPoint);
    }
}

// XControl
//------------------------------------------------------------------------------
void SAL_CALL OImageControl::createPeer(const Reference<XToolkit>& _rxToolkit, const Reference<XWindowPeer>& Parent) throw( RuntimeException )
{
    OControl::createPeer(_rxToolkit, Parent);
    Reference<XWindowPeer>  xPeer = getPeer();

    // Pointer umsetzen
    if (!xPeer.is())
        return;

    Reference<XFastPropertySet>  xSet(getModel(), UNO_QUERY);
    if (xSet.is())
    {
        // ist eine TargetUrl gesetzt, wird die Refhand verwendet
        if (getString(xSet->getFastPropertyValue(PROPERTY_ID_TARGET_URL)).getLength())
        {
            Reference<XPointer> xPoint(
                m_xServiceFactory->createInstance(SRV_AWT_POINTER), UNO_QUERY);
            if (xPoint.is())
                xPoint->setType(SystemPointer::REFHAND);
            xPeer->setPointer(xPoint);
        }
    }

    // the following is not necessary anymore. The aggregated control (from the toolkit project)
    // itself will register as image consumer at the image producer, so there's no need to do this ourself.
    // This holds since our model is an XImageProducer itself, and thus hiding the XImageProducer of the aggregated
    // model. Before, we had two ImageProducers working in parallel.
    // 2003-05-15 - 109591 - fs@openoffice.org

/*
    // dem ImageProducer einen neuen Consumer bekannt geben
    Reference<XImageProducerSupplier>  xImgSrc(getModel(), UNO_QUERY);
    if (!xImgSrc.is())
        return;

    Reference<XImageProducer>  xImgProd = xImgSrc->getImageProducer();
    Reference<XImageConsumer>  xImgCons(xPeer, UNO_QUERY);

    xImgProd->addConsumer(xImgCons);
    xImgProd->startProduction();
*/
}

//------------------------------------------------------------------------------
sal_Bool OImageControl::setModel(const Reference<XControlModel>& Model) throw ( RuntimeException)
{
    Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
    if (xSet.is())
        xSet->removePropertyChangeListener(PROPERTY_TARGET_URL, this);

    sal_Bool bRes = OControl::setModel(Model);
    if (bRes)
    {
        xSet = Reference<XPropertySet> (Model, UNO_QUERY);
        if (xSet.is())
            xSet->addPropertyChangeListener(PROPERTY_TARGET_URL, this);
    }
    return bRes;
}

//------------------------------------------------------------------------------
Reference< XModel >  OImageControl::getXModel(const InterfaceRef& xIface) const
{
    Reference< XModel >  xModel(xIface, UNO_QUERY);
    if (xModel.is())
        return xModel;
    else
    {
        Reference<XChild>  xChild(xIface, UNO_QUERY);
        if (xChild.is())
        {
            InterfaceRef  xParent = xChild->getParent();
            return getXModel(xParent);
        }
        else
            return NULL;
    }
}

//------------------------------------------------------------------------------
// Diese Methode wird auch aus einem Thread gerufen und muss deshalb
// thread-safe sein.
void OImageControl::actionPerformed_Impl(sal_Bool bNotifyListener, const ::com::sun::star::awt::MouseEvent& rEvt)
{
    if( bNotifyListener )
    {
        sal_Bool bCanceled = sal_False;
        ::cppu::OInterfaceIteratorHelper aIter(m_aApproveActionListeners);
        EventObject aEvt(static_cast<XWeak*>(this));
        while( !bCanceled && aIter.hasMoreElements() )
        {
            XInterface *pNext = aIter.next();
            // Jede approveAction-Methode muss thread-safe sein!!!
            if( !((XApproveActionListener*)pNext)->approveAction(aEvt) )
                bCanceled = sal_True;
        }

        if (bCanceled)
            return;
    }

    // Ob der Rest des Codes Thread-Safe ist weiss man nicht genau. Deshalb
    // wird das meiste bei gelocktem Solar-Mutex erledigen.
    Reference<XPropertySet>  xSet;
    InterfaceRef  xParent;
    FormButtonType eButtonType;
    {
        SolarMutexGuard aGuard;

        // Parent holen
        Reference<XFormComponent>  xComp(getModel(), UNO_QUERY);
        if (!xComp.is())
            return;

        xParent = xComp->getParent();
        if (!xParent.is())
            return;

        // Welchen ButtonType haben wir?
        if (!query_interface((Reference< XInterface >)xComp, xSet))
            return;
        eButtonType = *(FormButtonType*)xSet->getPropertyValue(PROPERTY_BUTTONTYPE).getValue();
    }

    switch (eButtonType)
    {
        case FormButtonType_RESET:
        {
            // reset-Methoden muessen thread-safe sein!
            Reference<XReset>  xReset(xParent, UNO_QUERY);
            if (!xReset.is())
                return;

            xReset->reset();
        }
        break;

        case FormButtonType_SUBMIT:
        {
            // submit-Methoden muessen thread-safe sein!
            Reference<XSubmit>  xSubmit(xParent, UNO_QUERY);
            if (!xSubmit.is())
                return;

            xSubmit->submit(this, rEvt);
        }
        break;

        case FormButtonType_URL:
        {
            SolarMutexGuard aGuard;

            Reference< XModel >  xModel = getXModel(xParent);
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

            if (aURL.Complete.getLength() && (LOCAL_URL_PREFIX == aURL.Complete.getStr()[0]))
            {	// the URL contains a local URL only. Since the URLTransformer does not handle this case correctly
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
                // 89752 - 23.07.2001 - frank.schoenheit@sun.com
                aURL.Mark = aURL.Complete;
                aURL.Complete = xModel->getURL();
                aURL.Complete += aURL.Mark;
            }

            Reference<XURLTransformer>
                xTransformer(m_xServiceFactory->createInstance(
                    ::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer")), UNO_QUERY);

            sal_Bool bDispatchUrlInternal = sal_False;
            xSet->getPropertyValue(PROPERTY_DISPATCHURLINTERNAL) >>= bDispatchUrlInternal;
            if ( bDispatchUrlInternal )
            {
                if ( xTransformer.is() )
                    xTransformer->parseSmart( aURL, ::rtl::OUString::createFromAscii(INET_FILE_SCHEME) );

                ::rtl::OUString aTargetFrame;
                xSet->getPropertyValue(PROPERTY_TARGET_FRAME) >>= aTargetFrame;

                Reference< XDispatch >  xDisp = Reference< XDispatchProvider > (xFrame,UNO_QUERY)->queryDispatch( aURL, aTargetFrame,
                        FrameSearchFlag::SELF | FrameSearchFlag::PARENT |
                        FrameSearchFlag::SIBLINGS | FrameSearchFlag::CREATE );

                Sequence<PropertyValue> aArgs(1);
                PropertyValue& rProp = aArgs.getArray()[0];
                rProp.Name = ::rtl::OUString::createFromAscii("Referer");
                rProp.Value <<= xModel->getURL();

                if (xDisp.is())
                    xDisp->dispatch( aURL, aArgs );
            }
            else
            {
                ::com::sun::star::util::URL aHyperLink;
                aHyperLink.Complete = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:OpenHyperlink"));
                if (xTransformer.is())
                    xTransformer->parseStrict( aHyperLink );

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
            m_aActionListeners.notifyEach(
                &XActionListener::actionPerformed, aEvt);
        }
    }
}


//==================================================================
// OImageModel
//==================================================================
//------------------------------------------------------------------------------
Sequence<Type> OImageModel::_getTypes()
{
    return concatSequences(
        OControlModel::_getTypes(),
        OImageModel_Base::getTypes()
    );
}

//------------------------------------------------------------------
DBG_NAME( OImageModel )
//------------------------------------------------------------------
OImageModel::OImageModel( const Reference< XMultiServiceFactory >& _rxFactory, const ::rtl::OUString& _rUnoControlModelTypeName,
        const ::rtl::OUString& rDefault, const sal_Bool _bSetDelegator )
    :OControlModel( _rxFactory, _rUnoControlModelTypeName, rDefault, _bSetDelegator )
    ,OPropertyChangeListener(m_aMutex)
    ,m_pProducer( NULL )
    ,m_pMedium(NULL)
    ,m_bDownloading(sal_False)
    ,m_bProdStarted(sal_False)
    ,m_bDispatchUrlInternal(sal_False)
{
    DBG_CTOR( OImageModel, NULL );
    implConstruct();
    m_eButtonType = FormButtonType_PUSH;
}

//------------------------------------------------------------------
OImageModel::OImageModel( const OImageModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory, const sal_Bool _bSetDelegator )
    :OControlModel( _pOriginal, _rxFactory, _bSetDelegator )
    ,OPropertyChangeListener( m_aMutex )
    ,m_pProducer( NULL )
    ,m_pMedium( NULL )
    ,m_bDownloading( sal_False )
    ,m_bProdStarted( sal_False )
    ,m_bDispatchUrlInternal(sal_False)
{
    DBG_CTOR( OImageModel, NULL );
    implConstruct();

    // copy properties
    m_eButtonType			= _pOriginal->m_eButtonType;
    m_sTargetURL			= _pOriginal->m_sTargetURL;
    m_sTargetFrame			= _pOriginal->m_sTargetFrame;
    m_bDispatchUrlInternal	= _pOriginal->m_bDispatchUrlInternal;
}

//------------------------------------------------------------------------------
void OImageModel::implInitializeImageURL( )
{
    osl_incrementInterlockedCount( &m_refCount );
    {
        // simulate a propertyChanged event for the ImageURL
        // 2003-05-15 - #109591# - fs@openoffice.org
        Any aImageURL;
        getFastPropertyValue( aImageURL, PROPERTY_ID_IMAGE_URL );
        _propertyChanged( PropertyChangeEvent( *this, PROPERTY_IMAGE_URL, sal_False, PROPERTY_ID_IMAGE_URL, Any( ), aImageURL ) );
    }
    osl_decrementInterlockedCount( &m_refCount );
}

//------------------------------------------------------------------------------
void OImageModel::implConstruct()
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
OImageModel::~OImageModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
    DBG_ASSERT(m_pMedium == NULL, "OImageModel::~OImageModel : leaving a memory leak ...");
        // spaetestens im dispose sollte das aufgeraeumt worden sein

    DBG_DTOR( OImageModel, NULL );
}

// XImageProducer
//--------------------------------------------------------------------
void SAL_CALL OImageModel::addConsumer( const Reference< XImageConsumer >& _rxConsumer ) throw (RuntimeException)
{
    GetImageProducer()->addConsumer( _rxConsumer );
}

//--------------------------------------------------------------------
void SAL_CALL OImageModel::removeConsumer( const Reference< XImageConsumer >& _rxConsumer ) throw (RuntimeException)
{
    GetImageProducer()->removeConsumer( _rxConsumer );
}

//--------------------------------------------------------------------
void SAL_CALL OImageModel::startProduction(  ) throw (RuntimeException)
{
    GetImageProducer()->startProduction();
}

// OComponentHelper
//------------------------------------------------------------------------------
void OImageModel::disposing()
{
    OControlModel::disposing();
    if (m_pMedium)
    {
        delete m_pMedium;
        m_pMedium = NULL;
    }

    m_xProducer = NULL;
}

//------------------------------------------------------------------------------
Any SAL_CALL OImageModel::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    // order matters:
    // we definately want to "overload" the XImageProducer interface of our aggregate,
    // thus check OImageModel_Base (which provides this) first
    Any aReturn = OImageModel_Base::queryInterface( _rType );

    // BUT: _don't_ let it feel responsible for the XTypeProvider interface
    // (as this is implemented by our base class in the proper way)
    if  (   _rType.equals( ::getCppuType( static_cast< Reference< XTypeProvider >* >( NULL ) ) )
        ||  !aReturn.hasValue()
        )
        aReturn = OControlModel::queryAggregation( _rType );

    return aReturn;
}

//------------------------------------------------------------------------------
void OImageModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_BUTTONTYPE				: rValue <<= m_eButtonType; break;
        case PROPERTY_ID_TARGET_URL				: rValue <<= m_sTargetURL; break;
        case PROPERTY_ID_TARGET_FRAME			: rValue <<= m_sTargetFrame; break;
        case PROPERTY_ID_DISPATCHURLINTERNAL	: rValue <<= m_bDispatchUrlInternal; break;
        default:
            OControlModel::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
void OImageModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) throw ( Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_BUTTONTYPE :
            DBG_ASSERT(isA(rValue, static_cast<FormButtonType*>(NULL)), "OImageModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            rValue >>= m_eButtonType;
            break;

        case PROPERTY_ID_TARGET_URL :
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_STRING, "OImageModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            rValue >>= m_sTargetURL;
            break;

        case PROPERTY_ID_TARGET_FRAME :
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_STRING, "OImageModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            rValue >>= m_sTargetFrame;
            break;

        case PROPERTY_ID_DISPATCHURLINTERNAL:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_BOOLEAN, "OImageModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            rValue >>= m_bDispatchUrlInternal;
            break;

        default:
            OControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue);
    }
}

//------------------------------------------------------------------------------
sal_Bool OImageModel::convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue)
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
void OImageModel::StartProduction()
{
    ImageProducer *pImgProd = GetImageProducer();
    if (!m_pMedium)
    {
        // caution: the medium may be NULL if somebody gave us a invalid URL to work with
        // 11/24/2000 - 79667 - FS
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
void OImageModel::SetURL( const ::rtl::OUString& rURL )
{
    if (m_pMedium || !rURL.getLength())
    {
        // Den Stream am Producer freigeben, bevor das Medium geloscht wird.
        GetImageProducer()->SetImage(String());
        delete m_pMedium;
        m_pMedium = NULL;
    }

    // the SfxMedium is not allowed to be created with an invalid URL, so we have to check this first
    // 23.01.2001 - 81927 - FS
    INetURLObject aUrl(rURL);
    if (INET_PROT_NOT_VALID == aUrl.GetProtocol())
        // we treat an invalid URL like we would treat no URL
        return;

    if (rURL.getLength())
    {
        if (m_pMedium)
            delete m_pMedium;

        m_pMedium = new SfxMedium(rURL, STREAM_STD_READ, sal_False);
        m_pMedium->SetTransferPriority(SFX_TFPRIO_VISIBLE_HIGHRES_GRAPHIC);
        m_pMedium->SetDataAvailableLink(
                STATIC_LINK(this, OImageModel, DataAvailableLink));

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
                SfxObjectShell *pTestObjSh = SfxObjectShell::GetFirst();
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
            // Medium registrieren, damit abgebrochen werden kann
            pObjSh->RegisterTransfer( *m_pMedium );

            // Target-Frame uebertragen, damit auch javascript:-URLs
            // "geladen" werden koennen.
        }
        else
        {
            // Keine Object-Shell, aber ein Medium? Dann uebernehmen wir
            // zumindest den Referer.
            if( xModel.is() )
            {
                ::rtl::OUString sReferer( xModel->getURL() );
                if( sReferer.getLength() )
                    m_pMedium->SetReferer( OUStringToString(sReferer, CHARSET_SYSTEM) );
            }

            // Keinen Eintrag im Roter Button Menu
            m_pMedium->SetDontCreateCancellable();
        }
#else
        if( pObjSh )
        {
            m_pMedium->SetUsesCache( !pObjSh->IsReloading() );

            // Target-Frame uebertragen, damit auch javascript:-URLs
            // "geladen" werden koennen.
        }

        if( xModel.is() )
        {
            ::rtl::OUString sReferer( xModel->getURL() );
            if( sReferer.getLength() )
                m_pMedium->SetReferer( sReferer );
        }

        // Keinen Eintrag im Roter Button Menu
        m_pMedium->SetDontCreateCancellable();
#endif

        // Downloading-Flag auf sal_True setzen. Es werden dann auch
        // Data-Available-Links, wenn wir in den Pending-Staus gelangen.
        m_bDownloading = sal_True;
        m_bProdStarted = sal_False;

        // Download anstossen (Achtung: Kann auch synchron sein).
        m_pMedium->DownLoad(STATIC_LINK(this, OImageModel, DownloadDoneLink));
    }
    else
    {
        GetImageProducer()->startProduction();
    }
}

//------------------------------------------------------------------------------
void OImageModel::DataAvailable()
{
    if (!m_bProdStarted)
        StartProduction();

    GetImageProducer()->NewDataAvailable();
}

//------------------------------------------------------------------------------
void OImageModel::DownloadDone()
{
    DataAvailable();
    m_bDownloading = sal_False;
}

//------------------------------------------------------------------------------
IMPL_STATIC_LINK( OImageModel, DownloadDoneLink, void*, EMPTYARG )
{
    ::osl::MutexGuard aGuard( pThis->m_aMutex );
    pThis->DownloadDone();
    return 0;
}

//------------------------------------------------------------------------------
IMPL_STATIC_LINK( OImageModel, DataAvailableLink, void*, EMPTYARG )
{
    ::osl::MutexGuard aGuard( pThis->m_aMutex );
    pThis->DataAvailable();
    return 0;
}

//------------------------------------------------------------------------------
void OImageModel::_propertyChanged( const PropertyChangeEvent& rEvt )
    throw( RuntimeException )
{
    // Wenn eine URL gesetzt worden ist, muss die noch an den ImageProducer
    // weitergereicht werden.
    ::osl::MutexGuard aGuard(m_aMutex);
    SetURL( getString(rEvt.NewValue) );
}
// -----------------------------------------------------------------------------
void SAL_CALL OImageModel::read(const Reference< XObjectInputStream>& _rxInStream) throw(IOException, RuntimeException)
{
    OControlModel::read(_rxInStream);
    m_bDispatchUrlInternal = sal_True;
}
// -----------------------------------------------------------------------------
PropertyState OImageModel::getPropertyStateByHandle(sal_Int32 nHandle)
{
    PropertyState eState;
    switch (nHandle)
    {
        case PROPERTY_ID_DISPATCHURLINTERNAL:
            if ( m_bDispatchUrlInternal )
                eState = PropertyState_DIRECT_VALUE;
            else
                eState = PropertyState_DEFAULT_VALUE;
            break;
        default:
            eState = OControlModel::getPropertyStateByHandle(nHandle);
    }
    return eState;
}
// -----------------------------------------------------------------------------
void OImageModel::setPropertyToDefaultByHandle(sal_Int32 nHandle)
{
    switch (nHandle)
    {
        case PROPERTY_ID_DISPATCHURLINTERNAL:
            setFastPropertyValue(nHandle, makeAny(sal_False));
            break;
        default:
            OControlModel::setPropertyToDefaultByHandle(nHandle);
    }
}
// -----------------------------------------------------------------------------
Any OImageModel::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_DISPATCHURLINTERNAL:
            return makeAny(sal_False);
            break;
        default:
            return OControlModel::getPropertyDefaultByHandle(nHandle);
    }
}
// -----------------------------------------------------------------------------

//==================================================================
// OImageControlThread_Impl
//==================================================================
//------------------------------------------------------------------
EventObject* OImageControlThread_Impl::cloneEvent( const EventObject* _pEvt ) const
{
    return new EventObject( *_pEvt );
}

//------------------------------------------------------------------
void OImageControlThread_Impl::processEvent( ::cppu::OComponentHelper *pCompImpl,
                                               const EventObject* pEvt,
                                               const Reference<XControl>&,
                                               sal_Bool )
{
    ((OImageControl *)pCompImpl)->actionPerformed_Impl( sal_True, *(::com::sun::star::awt::MouseEvent *)pEvt );
}

//.........................................................................
}   // namespace frm
//.........................................................................

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
