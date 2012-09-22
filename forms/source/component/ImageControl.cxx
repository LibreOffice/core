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

#include "ImageControl.hxx"

#include "property.hrc"
#include "frm_resource.hrc"
#include "frm_resource.hxx"
#include "services.hxx"
#include "componenttools.hxx"

#include <svtools/imageresourceaccess.hxx>
#include <unotools/ucblockbytes.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/awt/PopupMenu.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/awt/PopupMenuDirection.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicObject.hpp>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <unotools/streamhelper.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/guarding.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svl/urihelper.hxx>

#include <memory>

#define ID_OPEN_GRAPHICS            1
#define ID_CLEAR_GRAPHICS           2

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::frame;

//==============================================================================
//= OImageControlModel
//==============================================================================
namespace
{
    enum ImageStoreType
    {
        ImageStoreBinary,
        ImageStoreLink,

        ImageStoreInvalid
    };

    ImageStoreType lcl_getImageStoreType( const sal_Int32 _nFieldType )
    {
        // binary/longvarchar types could be used to store images in binary representation
        if  (   ( _nFieldType == DataType::BINARY )
            ||  ( _nFieldType == DataType::VARBINARY )
            ||  ( _nFieldType == DataType::LONGVARBINARY )
            ||  ( _nFieldType == DataType::OTHER )
            ||  ( _nFieldType == DataType::OBJECT )
            ||  ( _nFieldType == DataType::BLOB )
            ||  ( _nFieldType == DataType::LONGVARCHAR )
            ||  ( _nFieldType == DataType::CLOB )
            )
            return ImageStoreBinary;

        // char types could be used to store links to images
        if  (   ( _nFieldType == DataType::CHAR )
            ||  ( _nFieldType == DataType::VARCHAR )
            )
            return ImageStoreLink;

        return ImageStoreInvalid;
    }
}

//==============================================================================
// OImageControlModel
//==============================================================================

//------------------------------------------------------------------------------
InterfaceRef SAL_CALL OImageControlModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OImageControlModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OImageControlModel::_getTypes()
{
    return concatSequences(
        OBoundControlModel::_getTypes(),
        OImageControlModel_Base::getTypes()
    );
}

DBG_NAME(OImageControlModel)
//------------------------------------------------------------------
OImageControlModel::OImageControlModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_IMAGECONTROL, FRM_SUN_CONTROL_IMAGECONTROL, sal_False, sal_False, sal_False )
                    // use the old control name for compytibility reasons
    ,m_pImageProducer( NULL )
    ,m_bExternalGraphic( true )
    ,m_bReadOnly( sal_False )
    ,m_sImageURL()
    ,m_xGraphicObject()
{
    DBG_CTOR( OImageControlModel, NULL );
    m_nClassId = FormComponentType::IMAGECONTROL;
    initOwnValueProperty( PROPERTY_IMAGE_URL );

    implConstruct();
}

//------------------------------------------------------------------
OImageControlModel::OImageControlModel( const OImageControlModel* _pOriginal, const Reference< XMultiServiceFactory >& _rxFactory )
    :OBoundControlModel( _pOriginal, _rxFactory )
                // use the old control name for compytibility reasons
    ,m_pImageProducer( NULL )
    ,m_bExternalGraphic( true )
    ,m_bReadOnly( _pOriginal->m_bReadOnly )
    ,m_sImageURL( _pOriginal->m_sImageURL )
    ,m_xGraphicObject( _pOriginal->m_xGraphicObject )
{
    DBG_CTOR( OImageControlModel, NULL );
    implConstruct();

    osl_atomic_increment( &m_refCount );
    {
        // simulate a propertyChanged event for the ImageURL
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_handleNewImageURL_lck( eOther );
    }
    osl_atomic_decrement( &m_refCount );
}

//------------------------------------------------------------------
void OImageControlModel::implConstruct()
{
    m_pImageProducer = new ImageProducer;
    m_xImageProducer = m_pImageProducer;
    m_pImageProducer->SetDoneHdl( LINK( this, OImageControlModel, OnImageImportDone ) );
}

//------------------------------------------------------------------
OImageControlModel::~OImageControlModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(OImageControlModel,NULL);
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OImageControlModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence  OImageControlModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_IMAGECONTROL;
    return aSupported;
}

//------------------------------------------------------------------------------
Any SAL_CALL OImageControlModel::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    // Order matters: we want to "override" the XImageProducer interface of the aggreate with out
    // own XImageProducer interface, thus we need to query OImageControlModel_Base first
    Any aReturn = OImageControlModel_Base::queryInterface( _rType );

    // BUT: _don't_ let it feel responsible for the XTypeProvider interface
    // (as this is implemented by our base class in the proper way)
    if  (   _rType.equals( ::getCppuType( static_cast< Reference< XTypeProvider >* >( NULL ) ) )
        ||  !aReturn.hasValue()
        )
        aReturn = OBoundControlModel::queryAggregation( _rType );

    return aReturn;
}

//------------------------------------------------------------------------------
sal_Bool OImageControlModel::approveDbColumnType( sal_Int32 _nColumnType )
{
    return ImageStoreInvalid != lcl_getImageStoreType( _nColumnType );
}

//------------------------------------------------------------------------------
void OImageControlModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_READONLY:
            rValue <<= (sal_Bool)m_bReadOnly;
            break;
        case PROPERTY_ID_IMAGE_URL:
            rValue <<= m_sImageURL;
            break;
        case PROPERTY_ID_GRAPHIC:
            rValue <<= m_xGraphicObject.is() ? m_xGraphicObject->getGraphic() : Reference< XGraphic >();
            break;
        default:
            OBoundControlModel::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
void OImageControlModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) throw ( ::com::sun::star::uno::Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_READONLY :
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_BOOLEAN, "OImageControlModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            m_bReadOnly = getBOOL(rValue);
            break;

        case PROPERTY_ID_IMAGE_URL:
            OSL_VERIFY( rValue >>= m_sImageURL );
            impl_handleNewImageURL_lck( eOther );
            {
                ControlModelLock aLock( *this );
                    // that's a fake ... onValuePropertyChange expects to receive the only lock to our instance,
                    // but we're already called with our mutex locked ...
                onValuePropertyChange( aLock );
            }
            break;

        case PROPERTY_ID_GRAPHIC:
        {
            Reference< XGraphic > xGraphic;
            OSL_VERIFY( rValue >>= xGraphic );
            if ( !xGraphic.is() )
                m_xGraphicObject.clear();
            else
            {
                m_xGraphicObject = GraphicObject::create( m_aContext.getUNOContext() );
                m_xGraphicObject->setGraphic( xGraphic );
            }

            if ( m_bExternalGraphic )
            {
                // if that's an external graphic, i.e. one which has not been loaded by ourselves in response to a
                // new image URL, then also adjust our ImageURL.
                ::rtl::OUString sNewImageURL;
                if ( m_xGraphicObject.is() )
                {
                    sNewImageURL = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.GraphicObject:" ) );
                    sNewImageURL = sNewImageURL + m_xGraphicObject->getUniqueID();
                }
                m_sImageURL = sNewImageURL;
                // TODO: speaking strictly, this would need to be notified, since ImageURL is a bound property. However,
                // this method here is called with a locked mutex, so we cannot simply call listeners ...
                // I think the missing notification (and thus clients which potentially cannot observe the change)
                // is less severe than the potential deadlock ...
            }
        }
        break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue);
            break;
    }
}

//------------------------------------------------------------------------------
sal_Bool OImageControlModel::convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue)
                                throw( IllegalArgumentException )
{
    switch (nHandle)
    {
        case PROPERTY_ID_READONLY :
            return tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bReadOnly);

        case PROPERTY_ID_IMAGE_URL:
            return tryPropertyValue( rConvertedValue, rOldValue, rValue, m_sImageURL );

        case PROPERTY_ID_GRAPHIC:
        {
            const Reference< XGraphic > xGraphic( getFastPropertyValue( PROPERTY_ID_GRAPHIC ), UNO_QUERY );
            return tryPropertyValue( rConvertedValue, rOldValue, rValue, xGraphic );
        }

        default:
            return OBoundControlModel::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);
    }
}

//------------------------------------------------------------------------------
void OImageControlModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 4, OBoundControlModel )
        DECL_IFACE_PROP2( GRAPHIC,   XGraphic,        BOUND, TRANSIENT );
        DECL_PROP1      ( IMAGE_URL, ::rtl::OUString, BOUND );
        DECL_BOOL_PROP1 ( READONLY,                   BOUND );
        DECL_PROP1      ( TABINDEX,  sal_Int16,       BOUND );
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
void OImageControlModel::describeAggregateProperties( Sequence< Property >& /* [out] */ o_rAggregateProperties ) const
{
    OBoundControlModel::describeAggregateProperties( o_rAggregateProperties );
    // remove ImageULR and Graphic properties, we "overload" them. This is because our aggregate synchronizes those
    // two, but we have an own sychronization mechanism.
    RemoveProperty( o_rAggregateProperties, PROPERTY_IMAGE_URL );
    RemoveProperty( o_rAggregateProperties, PROPERTY_GRAPHIC );
}

//------------------------------------------------------------------------------
::rtl::OUString OImageControlModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_IMAGECONTROL;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OImageControlModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    // Base class
    OBoundControlModel::write(_rxOutStream);
    // Version
    _rxOutStream->writeShort(0x0003);
    // Name
    _rxOutStream->writeBoolean(m_bReadOnly);
    writeHelpTextCompatibly(_rxOutStream);
    // from version 0x0003 : common properties
    writeCommonProperties(_rxOutStream);
}

//------------------------------------------------------------------------------
void OImageControlModel::read(const Reference<XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OBoundControlModel::read(_rxInStream);

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();
    switch (nVersion)
    {
        case 0x0001:
            m_bReadOnly = _rxInStream->readBoolean();
            break;
        case 0x0002:
            m_bReadOnly = _rxInStream->readBoolean();
            readHelpTextCompatibly(_rxInStream);
            break;
        case 0x0003:
            m_bReadOnly = _rxInStream->readBoolean();
            readHelpTextCompatibly(_rxInStream);
            readCommonProperties(_rxInStream);
            break;
        default :
            OSL_FAIL("OImageControlModel::read : unknown version !");
            m_bReadOnly = sal_False;
            defaultCommonProperties();
            break;
    }
    // Display default values after read
    if ( !getControlSource().isEmpty() )
    {   // (not if we don't have a control source - the "State" property acts like it is persistent, then
        ::osl::MutexGuard aGuard(m_aMutex); // resetNoBroadcast expects this mutex guarding
        resetNoBroadcast();
    }
}

//------------------------------------------------------------------------------
sal_Bool OImageControlModel::impl_updateStreamForURL_lck( const ::rtl::OUString& _rURL, ValueChangeInstigator _eInstigator )
{
    // create a stream for the image specified by the URL
    ::std::auto_ptr< SvStream > pImageStream;
    Reference< XInputStream > xImageStream;

    if ( ::svt::GraphicAccess::isSupportedURL( _rURL ) )
    {
        xImageStream = ::svt::GraphicAccess::getImageXStream( getContext().getLegacyServiceFactory(), _rURL );
    }
    else
    {
        pImageStream.reset( ::utl::UcbStreamHelper::CreateStream( _rURL, STREAM_READ ) );
        sal_Bool bSetNull = ( pImageStream.get() == NULL ) || ( ERRCODE_NONE != pImageStream->GetErrorCode() );

        if ( !bSetNull )
        {
            // get the size of the stream
            pImageStream->Seek(STREAM_SEEK_TO_END);
            sal_Int32 nSize = (sal_Int32)pImageStream->Tell();
            if (pImageStream->GetBufferSize() < 8192)
                pImageStream->SetBufferSize(8192);
            pImageStream->Seek(STREAM_SEEK_TO_BEGIN);

            xImageStream = new ::utl::OInputStreamHelper( new SvLockBytes( pImageStream.get(), sal_False ), nSize );
        }
    }

    if ( xImageStream.is() )
    {
        if ( m_xColumnUpdate.is() )
            m_xColumnUpdate->updateBinaryStream( xImageStream, xImageStream->available() );
        else
            setControlValue( makeAny( xImageStream ), _eInstigator );
        xImageStream->closeInput();
        return sal_True;
    }

    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool OImageControlModel::impl_handleNewImageURL_lck( ValueChangeInstigator _eInstigator )
{
    switch ( lcl_getImageStoreType( getFieldType() ) )
    {
    case ImageStoreBinary:
        if ( impl_updateStreamForURL_lck( m_sImageURL, _eInstigator ) )
            return sal_True;
        break;

    case ImageStoreLink:
    {
        ::rtl::OUString sCommitURL( m_sImageURL );
        if ( !m_sDocumentURL.isEmpty() )
            sCommitURL = URIHelper::simpleNormalizedMakeRelative( m_sDocumentURL, sCommitURL );
        OSL_ENSURE( m_xColumnUpdate.is(), "OImageControlModel::impl_handleNewImageURL_lck: no bound field, but ImageStoreLink?!" );
        if ( m_xColumnUpdate.is() )
        {
            m_xColumnUpdate->updateString( sCommitURL );
            return sal_True;
        }
    }
    break;

    case ImageStoreInvalid:
        OSL_FAIL( "OImageControlModel::impl_handleNewImageURL_lck: image storage type type!" );
        break;
    }

    // if we're here, then the above code was unable to update our field/control from the given URL
    // => fall back to NULL/VOID
    if ( m_xColumnUpdate.is() )
        m_xColumnUpdate->updateNull();
    else
        setControlValue( Any(), _eInstigator );

    return sal_True;
}

//------------------------------------------------------------------------------
sal_Bool OImageControlModel::commitControlValueToDbColumn( bool _bPostReset )
{
    if ( _bPostReset )
    {
        // since this is a "commit after reset", we can simply update the column
        // with null - this is our "default" which we were just reset to
        if ( m_xColumnUpdate.is() )
            m_xColumnUpdate->updateNull();
    }
    else
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        return impl_handleNewImageURL_lck( eDbColumnBinding );
    }

    return sal_True;
}

//------------------------------------------------------------------------------
namespace
{
    bool lcl_isValidDocumentURL( const ::rtl::OUString& _rDocURL )
    {
        return ( !_rDocURL.isEmpty() && _rDocURL != "private:object" );
    }
}

//------------------------------------------------------------------------------
void OImageControlModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    OBoundControlModel::onConnectedDbColumn( _rxForm );

    try
    {
        Reference< XModel > xDocument( getXModel( *this ) );
        if ( xDocument.is() )
        {
            m_sDocumentURL = xDocument->getURL();
            if ( !lcl_isValidDocumentURL( m_sDocumentURL ) )
            {
                Reference< XChild > xAsChild( xDocument, UNO_QUERY );
                while ( xAsChild.is() && !lcl_isValidDocumentURL( m_sDocumentURL ) )
                {
                    xDocument.set( xAsChild->getParent(), UNO_QUERY );
                    if ( xDocument.is() )
                        m_sDocumentURL = xDocument->getURL();
                    xAsChild.set( xDocument, UNO_QUERY );
                }
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------------
void OImageControlModel::onDisconnectedDbColumn()
{
    OBoundControlModel::onDisconnectedDbColumn();

    m_sDocumentURL = ::rtl::OUString();
}

//------------------------------------------------------------------------------
Any OImageControlModel::translateDbColumnToControlValue()
{
    switch ( lcl_getImageStoreType( getFieldType() ) )
    {
    case ImageStoreBinary:
    {
        Reference< XInputStream > xImageStream( m_xColumn->getBinaryStream() );
        if ( m_xColumn->wasNull() )
            xImageStream.clear();
        return makeAny( xImageStream );
    }
    case ImageStoreLink:
    {
        ::rtl::OUString sImageLink( m_xColumn->getString() );
        if ( !m_sDocumentURL.isEmpty() )
            sImageLink = INetURLObject::GetAbsURL( m_sDocumentURL, sImageLink );
        return makeAny( sImageLink );
    }
    case ImageStoreInvalid:
        OSL_FAIL( "OImageControlModel::translateDbColumnToControlValue: invalid field type!" );
        break;
    }
    return Any();
}

//------------------------------------------------------------------------------
Any OImageControlModel::getControlValue( ) const
{
    return makeAny( m_sImageURL );
}

//------------------------------------------------------------------------------
void OImageControlModel::doSetControlValue( const Any& _rValue )
{
    DBG_ASSERT( GetImageProducer() && m_xImageProducer.is(), "OImageControlModel::doSetControlValue: no image producer!" );
    if ( !GetImageProducer() || !m_xImageProducer.is() )
        return;

    bool bStartProduction = false;
    switch ( lcl_getImageStoreType( getFieldType() ) )
    {
    case ImageStoreBinary:
    {
        // give the image producer the stream
        Reference< XInputStream > xInStream;
        _rValue >>= xInStream;
        GetImageProducer()->setImage( xInStream );
        bStartProduction = true;
    }
    break;

    case ImageStoreLink:
    {
        ::rtl::OUString sImageURL;
        _rValue >>= sImageURL;
        GetImageProducer()->SetImage( sImageURL );
        bStartProduction = true;
    }
    break;

    case ImageStoreInvalid:
        OSL_FAIL( "OImageControlModel::doSetControlValue: invalid field type!" );
        break;

    }   // switch ( lcl_getImageStoreType( getFieldType() ) )

    if ( bStartProduction )
    {
        // start production
        Reference< XImageProducer > xProducer = m_xImageProducer;
        {
            // release our mutex once (it's acquired in the calling method!), as starting the image production may
            // result in the locking of the solar mutex (unfortunally the default implementation of our aggregate,
            // VCLXImageControl, does this locking)
            MutexRelease aRelease(m_aMutex);
            xProducer->startProduction();
        }
    }
}

// OComponentHelper
//------------------------------------------------------------------
void SAL_CALL OImageControlModel::disposing()
{
    OBoundControlModel::disposing();
}

//------------------------------------------------------------------------------
void OImageControlModel::resetNoBroadcast()
{
    if ( hasField() )          // only reset when we are connected to a column
        OBoundControlModel::resetNoBroadcast( );
}

//--------------------------------------------------------------------
Reference< XImageProducer > SAL_CALL OImageControlModel::getImageProducer() throw ( RuntimeException)
{
    return this;
}

//--------------------------------------------------------------------
void SAL_CALL OImageControlModel::addConsumer( const Reference< XImageConsumer >& _rxConsumer ) throw (RuntimeException)
{
    GetImageProducer()->addConsumer( _rxConsumer );
}

//--------------------------------------------------------------------
void SAL_CALL OImageControlModel::removeConsumer( const Reference< XImageConsumer >& _rxConsumer ) throw (RuntimeException)
{
    GetImageProducer()->removeConsumer( _rxConsumer );
}

//--------------------------------------------------------------------
void SAL_CALL OImageControlModel::startProduction(  ) throw (RuntimeException)
{
    GetImageProducer()->startProduction();
}

//------------------------------------------------------------------------------
IMPL_LINK( OImageControlModel, OnImageImportDone, ::Graphic*, i_pGraphic )
{
    const Reference< XGraphic > xGraphic( i_pGraphic != NULL ? Image( i_pGraphic->GetBitmapEx() ).GetXGraphic() : NULL );
    m_bExternalGraphic = false;
    try
    {
        setPropertyValue( PROPERTY_GRAPHIC, makeAny( xGraphic ) );
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_bExternalGraphic = true;
    return 1L;
}

//==================================================================
// OImageControlControl
//==================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OImageControlControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OImageControlControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OImageControlControl::_getTypes()
{
    return concatSequences(
        OBoundControl::_getTypes(),
        OImageControlControl_Base::getTypes()
    );
}

//------------------------------------------------------------------------------
OImageControlControl::OImageControlControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_IMAGECONTROL)
    ,m_aModifyListeners( m_aMutex )
{
    increment(m_refCount);
    {
        // Add as Focus- and MouseListener
        Reference< XWindow > xComp;
        query_aggregation( m_xAggregate, xComp );
        if ( xComp.is() )
            xComp->addMouseListener( this );
    }
    decrement(m_refCount);
}

//------------------------------------------------------------------------------
Any SAL_CALL OImageControlControl::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OBoundControl::queryAggregation( _rType );
    if ( !aReturn.hasValue() )
        aReturn = ::cppu::queryInterface(
            _rType,
            static_cast< XMouseListener* >( this ),
            static_cast< XModifyBroadcaster* >( this )
        );

    return aReturn;
}

//------------------------------------------------------------------------------
StringSequence  OImageControlControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_IMAGECONTROL;
    return aSupported;
}

//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::addModifyListener( const Reference< XModifyListener >& _Listener ) throw (RuntimeException)
{
    m_aModifyListeners.addInterface( _Listener );
}

//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::removeModifyListener( const Reference< XModifyListener >& _Listener ) throw (RuntimeException)
{
    m_aModifyListeners.removeInterface( _Listener );
}

//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::disposing()
{
    EventObject aEvent( *this );
    m_aModifyListeners.disposeAndClear( aEvent );

    OBoundControl::disposing();
}

//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::disposing( const EventObject& _Event ) throw(RuntimeException)
{
    OBoundControl::disposing( _Event );
}

//------------------------------------------------------------------------------
void OImageControlControl::implClearGraphics( sal_Bool _bForce )
{
    Reference< XPropertySet > xSet( getModel(), UNO_QUERY );
    if ( xSet.is() )
    {
        if ( _bForce )
        {
            ::rtl::OUString sOldImageURL;
            xSet->getPropertyValue( PROPERTY_IMAGE_URL ) >>= sOldImageURL;

            if ( sOldImageURL.isEmpty() )
                // the ImageURL is already empty, so simply setting a new empty one would not suffice
                // (since it would be ignored)
                xSet->setPropertyValue( PROPERTY_IMAGE_URL, makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:emptyImage" ) ) ) );
                    // (the concrete URL we're passing here doens't matter. It's important that
                    // the model cannot resolve it to a a valid resource describing an image stream
        }

        xSet->setPropertyValue( PROPERTY_IMAGE_URL, makeAny( ::rtl::OUString() ) );
    }
}

//------------------------------------------------------------------------------
bool OImageControlControl::implInsertGraphics()
{
    Reference< XPropertySet > xSet( getModel(), UNO_QUERY );
    if ( !xSet.is() )
        return false;

    ::rtl::OUString sTitle = FRM_RES_STRING(RID_STR_IMPORT_GRAPHIC);
    // build some arguments for the upcoming dialog
    try
    {
        ::sfx2::FileDialogHelper aDialog( TemplateDescription::FILEOPEN_LINK_PREVIEW, SFXWB_GRAPHIC );
        aDialog.SetTitle( sTitle );

        Reference< XFilePickerControlAccess > xController( aDialog.GetFilePicker(), UNO_QUERY_THROW );
        xController->setValue(ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, ::cppu::bool2any(sal_True));

        Reference<XPropertySet> xBoundField;
        if ( hasProperty( PROPERTY_BOUNDFIELD, xSet ) )
            xSet->getPropertyValue( PROPERTY_BOUNDFIELD ) >>= xBoundField;
        sal_Bool bHasField = xBoundField.is();

        // if the control is bound to a DB field, then it's not possible to decide whether or not to link
        xController->enableControl(ExtendedFilePickerElementIds::CHECKBOX_LINK, !bHasField );

        // if the control is bound to a DB field, then linking of the image depends on the type of the field
        sal_Bool bImageIsLinked = sal_True;
        if ( bHasField )
        {
            sal_Int32 nFieldType = DataType::OTHER;
            OSL_VERIFY( xBoundField->getPropertyValue( PROPERTY_FIELDTYPE ) >>= nFieldType );
            bImageIsLinked = ( lcl_getImageStoreType( nFieldType ) == ImageStoreLink );
        }
        xController->setValue(ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, makeAny( bImageIsLinked ) );

        if ( ERRCODE_NONE == aDialog.Execute() )
        {
            // reset the url property in case it already has the value we're about to set - in this case
            // our propertyChanged would not get called without this.
            implClearGraphics( sal_False );
            sal_Bool bIsLink = sal_False;
            xController->getValue(ExtendedFilePickerElementIds::CHECKBOX_LINK, 0) >>= bIsLink;
            // Force bIsLink to be sal_True if we're bound to a field. Though we initialized the file picker with IsLink=TRUE
            // in this case, and disabled the respective control, there might be picker implementations which do not
            // respect this, and return IsLink=FALSE here. In this case, "normalize" the flag.
            // #i112659#
            bIsLink |= bHasField;
            if ( !bIsLink )
            {
                Graphic aGraphic;
                aDialog.GetGraphic( aGraphic );
                 xSet->setPropertyValue( PROPERTY_GRAPHIC, makeAny( aGraphic.GetXGraphic() ) );
            }
            else
                xSet->setPropertyValue( PROPERTY_IMAGE_URL, makeAny( ::rtl::OUString( aDialog.GetPath() ) ) );

            return true;
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("OImageControlControl::implInsertGraphics: caught an exception while attempting to execute the FilePicker!");
    }
    return false;
}

//------------------------------------------------------------------------------
bool OImageControlControl::impl_isEmptyGraphics_nothrow() const
{
    bool bIsEmpty = true;

    try
    {
        Reference< XPropertySet > xModelProps( const_cast< OImageControlControl* >( this )->getModel(), UNO_QUERY_THROW );
        Reference< XGraphic > xGraphic;
        OSL_VERIFY( xModelProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Graphic" ) ) ) >>= xGraphic );
        bIsEmpty = !xGraphic.is();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return bIsEmpty;
}

// MouseListener
//------------------------------------------------------------------------------
void OImageControlControl::mousePressed(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if (e.Buttons != MouseButton::LEFT)
        return;

    bool bModified = false;
    // is this a request for a context menu?
    if ( e.PopupTrigger )
    {
        Reference< XPopupMenu > xMenu( awt::PopupMenu::create( m_aContext.getUNOContext() ) );
        DBG_ASSERT( xMenu.is(), "OImageControlControl::mousePressed: could not create a popup menu!" );

        Reference< XWindowPeer > xWindowPeer = getPeer();
        DBG_ASSERT( xWindowPeer.is(), "OImageControlControl::mousePressed: no window!" );

        if ( xMenu.is() && xWindowPeer.is() )
        {
            xMenu->insertItem( ID_OPEN_GRAPHICS, FRM_RES_STRING( RID_STR_OPEN_GRAPHICS ), 0, 0 );
            xMenu->insertItem( ID_CLEAR_GRAPHICS, FRM_RES_STRING( RID_STR_CLEAR_GRAPHICS ), 0, 1 );

            // check if the ImageURL is empty
            if ( impl_isEmptyGraphics_nothrow() )
                xMenu->enableItem( ID_CLEAR_GRAPHICS, sal_False );

            awt::Rectangle aRect( e.X, e.Y, 0, 0 );
            if ( ( e.X < 0 ) || ( e.Y < 0 ) )
            {   // context menu triggered by keyboard
                // position it in the center of the control
                Reference< XWindow > xWindow( static_cast< ::cppu::OWeakObject* >( this ), UNO_QUERY );
                OSL_ENSURE( xWindow.is(), "OImageControlControl::mousePressed: me not a window? How this?" );
                if ( xWindow.is() )
                {
                    awt::Rectangle aPosSize = xWindow->getPosSize();
                    aRect.X = aPosSize.Width / 2;
                    aRect.Y = aPosSize.Height / 2;
                }
            }

            const sal_Int16 nResult = xMenu->execute( xWindowPeer, aRect, PopupMenuDirection::EXECUTE_DEFAULT );

            switch ( nResult )
            {
            case ID_OPEN_GRAPHICS:
                implInsertGraphics();
                bModified = true;
                break;

            case ID_CLEAR_GRAPHICS:
                implClearGraphics( sal_True );
                bModified = true;
                break;
            }
        }
    }
    else
    {
        //////////////////////////////////////////////////////////////////////
        // Double click
        if (e.ClickCount == 2)
        {

            Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
            if (!xSet.is())
                return;

            // If the Control is not bound, do not display a dialog (because the to-be-sent URL would be invalid anyway)
            Reference<XPropertySet> xBoundField;
            if (hasProperty(PROPERTY_BOUNDFIELD, xSet))
                ::cppu::extractInterface(xBoundField, xSet->getPropertyValue(PROPERTY_BOUNDFIELD));
            if (!xBoundField.is())
            {
                // but only if our IMAGE_URL property is handled as if it is transient, which is equivalent to
                // an empty control source
                if ( !hasProperty(PROPERTY_CONTROLSOURCE, xSet) || !::comphelper::getString(xSet->getPropertyValue(PROPERTY_CONTROLSOURCE)).isEmpty() )
                    return;
            }

            sal_Bool bReadOnly = false;
            xSet->getPropertyValue(PROPERTY_READONLY) >>= bReadOnly;
            if (bReadOnly)
                return;

            if ( implInsertGraphics() )
                bModified = true;
        }
    }

    if ( bModified )
    {
        EventObject aEvent( *this );
        m_aModifyListeners.notifyEach( &XModifyListener::modified, aEvent );
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::mouseReleased(const awt::MouseEvent& /*e*/) throw ( RuntimeException )
{
}

//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::mouseEntered(const awt::MouseEvent& /*e*/) throw ( RuntimeException )
{
}

//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::mouseExited(const awt::MouseEvent& /*e*/) throw ( RuntimeException )
{
}

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
