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

#include "ImageControl.hxx"

#include <strings.hrc>
#include <frm_resource.hxx>
#include <services.hxx>
#include <componenttools.hxx>

#include <svtools/imageresourceaccess.hxx>
#include <sfx2/filedlghelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
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
#include <vcl/window.hxx>
#include <unotools/streamhelper.hxx>
#include <comphelper/guarding.hxx>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svl/urihelper.hxx>

#include <memory>

#define ID_OPEN_GRAPHICS            1
#define ID_CLEAR_GRAPHICS           2

namespace frm
{

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


//= OImageControlModel

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


// OImageControlModel

Sequence<Type> OImageControlModel::_getTypes()
{
    return concatSequences(
        OBoundControlModel::_getTypes(),
        OImageControlModel_Base::getTypes()
    );
}


OImageControlModel::OImageControlModel(const Reference<XComponentContext>& _rxFactory)
    :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_IMAGECONTROL, FRM_SUN_CONTROL_IMAGECONTROL, false, false, false )
                    // use the old control name for compytibility reasons
    ,m_bExternalGraphic( true )
    ,m_bReadOnly( false )
    ,m_sImageURL()
    ,m_xGraphicObject()
{
    m_nClassId = FormComponentType::IMAGECONTROL;
    initOwnValueProperty( PROPERTY_IMAGE_URL );

    implConstruct();
}


OImageControlModel::OImageControlModel( const OImageControlModel* _pOriginal, const Reference< XComponentContext >& _rxFactory )
    :OBoundControlModel( _pOriginal, _rxFactory )
                // use the old control name for compytibility reasons
    ,m_bExternalGraphic( true )
    ,m_bReadOnly( _pOriginal->m_bReadOnly )
    ,m_sImageURL( _pOriginal->m_sImageURL )
    ,m_xGraphicObject( _pOriginal->m_xGraphicObject )
{
    implConstruct();

    osl_atomic_increment( &m_refCount );
    {
        // simulate a propertyChanged event for the ImageURL
        ::osl::MutexGuard aGuard( m_aMutex );
        impl_handleNewImageURL_lck( eOther );
    }
    osl_atomic_decrement( &m_refCount );
}


void OImageControlModel::implConstruct()
{
    m_xImageProducer = new ImageProducer;
    m_xImageProducer->SetDoneHdl( LINK( this, OImageControlModel, OnImageImportDone ) );
}


OImageControlModel::~OImageControlModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

}

// XCloneable

IMPLEMENT_DEFAULT_CLONING( OImageControlModel )

// XServiceInfo

css::uno::Sequence<OUString>  OImageControlModel::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_IMAGECONTROL;
    pArray[aSupported.getLength()-1] = FRM_COMPONENT_IMAGECONTROL;
    return aSupported;
}


Any SAL_CALL OImageControlModel::queryAggregation(const Type& _rType)
{
    // Order matters: we want to "override" the XImageProducer interface of the aggregate without
    // own XImageProducer interface, thus we need to query OImageControlModel_Base first
    Any aReturn = OImageControlModel_Base::queryInterface( _rType );

    // BUT: _don't_ let it feel responsible for the XTypeProvider interface
    // (as this is implemented by our base class in the proper way)
    if  (   _rType.equals( cppu::UnoType<XTypeProvider>::get() )
        ||  !aReturn.hasValue()
        )
        aReturn = OBoundControlModel::queryAggregation( _rType );

    return aReturn;
}


bool OImageControlModel::approveDbColumnType( sal_Int32 _nColumnType )
{
    return ImageStoreInvalid != lcl_getImageStoreType( _nColumnType );
}


void OImageControlModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_READONLY:
            rValue <<= m_bReadOnly;
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


void OImageControlModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue)
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
                m_xGraphicObject = graphic::GraphicObject::create( m_xContext );
                m_xGraphicObject->setGraphic( xGraphic );
            }

            if ( m_bExternalGraphic )
            {
                m_sImageURL = OUString();
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


sal_Bool OImageControlModel::convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue)
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


void OImageControlModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 4, OBoundControlModel )
        DECL_IFACE_PROP2( GRAPHIC,   XGraphic,        BOUND, TRANSIENT );
        DECL_PROP1      ( IMAGE_URL, OUString, BOUND );
        DECL_BOOL_PROP1 ( READONLY,                   BOUND );
        DECL_PROP1      ( TABINDEX,  sal_Int16,       BOUND );
    END_DESCRIBE_PROPERTIES();
}


void OImageControlModel::describeAggregateProperties( Sequence< Property >& /* [out] */ o_rAggregateProperties ) const
{
    OBoundControlModel::describeAggregateProperties( o_rAggregateProperties );
    // remove ImageURL and Graphic properties, we "override" them.
    // This is because our aggregate synchronizes those
    // two, but we have an own sychronization mechanism.
    RemoveProperty( o_rAggregateProperties, PROPERTY_IMAGE_URL );
    RemoveProperty( o_rAggregateProperties, PROPERTY_GRAPHIC );
}


OUString OImageControlModel::getServiceName()
{
    return OUString(FRM_COMPONENT_IMAGECONTROL);  // old (non-sun) name for compatibility !
}


void OImageControlModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
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


void OImageControlModel::read(const Reference<XObjectInputStream>& _rxInStream)
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
            m_bReadOnly = false;
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


bool OImageControlModel::impl_updateStreamForURL_lck( const OUString& _rURL, ValueChangeInstigator _eInstigator )
{
    // create a stream for the image specified by the URL
    std::unique_ptr< SvStream > pImageStream;
    Reference< XInputStream > xImageStream;

    if ( ::svt::GraphicAccess::isSupportedURL( _rURL ) )
    {
        xImageStream = ::svt::GraphicAccess::getImageXStream( getContext(), _rURL );
    }
    else
    {
        pImageStream = ::utl::UcbStreamHelper::CreateStream( _rURL, StreamMode::READ );
        bool bSetNull = (pImageStream == nullptr) || (ERRCODE_NONE != pImageStream->GetErrorCode());

        if ( !bSetNull )
        {
            // get the size of the stream
            sal_uInt64 const nSize = pImageStream->remainingSize();
            if (pImageStream->GetBufferSize() < 8192)
                pImageStream->SetBufferSize(8192);
            pImageStream->Seek(STREAM_SEEK_TO_BEGIN);

            xImageStream = new ::utl::OInputStreamHelper( new SvLockBytes( pImageStream.get(), false ), nSize );
        }
    }

    if ( xImageStream.is() )
    {
        if ( m_xColumnUpdate.is() )
            m_xColumnUpdate->updateBinaryStream( xImageStream, xImageStream->available() );
        else
            setControlValue( makeAny( xImageStream ), _eInstigator );
        xImageStream->closeInput();
        return true;
    }

    return false;
}


void OImageControlModel::impl_handleNewImageURL_lck( ValueChangeInstigator _eInstigator )
{
    switch ( lcl_getImageStoreType( getFieldType() ) )
    {
    case ImageStoreBinary:
        if ( impl_updateStreamForURL_lck( m_sImageURL, _eInstigator ) )
            return;
        break;

    case ImageStoreLink:
    {
        OUString sCommitURL( m_sImageURL );
        if ( !m_sDocumentURL.isEmpty() )
            sCommitURL = URIHelper::simpleNormalizedMakeRelative( m_sDocumentURL, sCommitURL );
        OSL_ENSURE( m_xColumnUpdate.is(), "OImageControlModel::impl_handleNewImageURL_lck: no bound field, but ImageStoreLink?!" );
        if ( m_xColumnUpdate.is() )
        {
            m_xColumnUpdate->updateString( sCommitURL );
            return;
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
}


bool OImageControlModel::commitControlValueToDbColumn( bool _bPostReset )
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
        impl_handleNewImageURL_lck( eDbColumnBinding );
    }

    return true;
}


namespace
{
    bool lcl_isValidDocumentURL( const OUString& _rDocURL )
    {
        return ( !_rDocURL.isEmpty() && _rDocURL != "private:object" );
    }
}


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
        DBG_UNHANDLED_EXCEPTION("forms.component");
    }
}


void OImageControlModel::onDisconnectedDbColumn()
{
    OBoundControlModel::onDisconnectedDbColumn();

    m_sDocumentURL.clear();
}


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
        OUString sImageLink( m_xColumn->getString() );
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


Any OImageControlModel::getControlValue( ) const
{
    return makeAny( m_sImageURL );
}


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
        OUString sImageURL;
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
        rtl::Reference< ImageProducer > xProducer = m_xImageProducer;
        {
            // release our mutex once (it's acquired in the calling method!), as starting the image production may
            // result in the locking of the solar mutex (unfortunately the default implementation of our aggregate,
            // VCLXImageControl, does this locking)
            MutexRelease aRelease(m_aMutex);
            xProducer->startProduction();
        }
    }
}

void OImageControlModel::resetNoBroadcast()
{
    if ( hasField() )          // only reset when we are connected to a column
        OBoundControlModel::resetNoBroadcast( );
}


Reference< XImageProducer > SAL_CALL OImageControlModel::getImageProducer()
{
    return this;
}


void SAL_CALL OImageControlModel::addConsumer( const Reference< XImageConsumer >& _rxConsumer )
{
    GetImageProducer()->addConsumer( _rxConsumer );
}


void SAL_CALL OImageControlModel::removeConsumer( const Reference< XImageConsumer >& _rxConsumer )
{
    GetImageProducer()->removeConsumer( _rxConsumer );
}


void SAL_CALL OImageControlModel::startProduction(  )
{
    GetImageProducer()->startProduction();
}


IMPL_LINK( OImageControlModel, OnImageImportDone, ::Graphic*, i_pGraphic, void )
{
    const Reference< XGraphic > xGraphic(i_pGraphic != nullptr ? i_pGraphic->GetXGraphic() : nullptr);
    m_bExternalGraphic = false;
    try
    {
        setPropertyValue( PROPERTY_GRAPHIC, makeAny( xGraphic ) );
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("forms.component");
    }
    m_bExternalGraphic = true;
}


// OImageControlControl

Sequence<Type> OImageControlControl::_getTypes()
{
    return concatSequences(
        OBoundControl::_getTypes(),
        OImageControlControl_Base::getTypes()
    );
}


OImageControlControl::OImageControlControl(const Reference<XComponentContext>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_IMAGECONTROL)
    ,m_aModifyListeners( m_aMutex )
{
    osl_atomic_increment(&m_refCount);
    {
        // Add as Focus- and MouseListener
        Reference< XWindow > xComp;
        query_aggregation( m_xAggregate, xComp );
        if ( xComp.is() )
            xComp->addMouseListener( this );
    }
    osl_atomic_decrement(&m_refCount);
}


Any SAL_CALL OImageControlControl::queryAggregation(const Type& _rType)
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


css::uno::Sequence<OUString>  OImageControlControl::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_CONTROL_IMAGECONTROL;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_IMAGECONTROL;
    return aSupported;
}


void SAL_CALL OImageControlControl::addModifyListener( const Reference< XModifyListener >& Listener )
{
    m_aModifyListeners.addInterface( Listener );
}


void SAL_CALL OImageControlControl::removeModifyListener( const Reference< XModifyListener >& Listener )
{
    m_aModifyListeners.removeInterface( Listener );
}


void SAL_CALL OImageControlControl::disposing()
{
    EventObject aEvent( *this );
    m_aModifyListeners.disposeAndClear( aEvent );

    OBoundControl::disposing();
}


void SAL_CALL OImageControlControl::disposing( const EventObject& Event )
{
    OBoundControl::disposing( Event );
}


void OImageControlControl::implClearGraphics( bool _bForce )
{
    Reference< XPropertySet > xSet( getModel(), UNO_QUERY );
    if ( xSet.is() )
    {
        if ( _bForce )
        {
            OUString sOldImageURL;
            xSet->getPropertyValue( PROPERTY_IMAGE_URL ) >>= sOldImageURL;

            if ( sOldImageURL.isEmpty() )
                // the ImageURL is already empty, so simply setting a new empty one would not suffice
                // (since it would be ignored)
                xSet->setPropertyValue( PROPERTY_IMAGE_URL, makeAny( OUString( "private:emptyImage" ) ) );
                    // (the concrete URL we're passing here doesn't matter. It's important that
                    // the model cannot resolve it to a valid resource describing an image stream
        }

        xSet->setPropertyValue( PROPERTY_IMAGE_URL, makeAny( OUString() ) );
    }
}


bool OImageControlControl::implInsertGraphics()
{
    Reference< XPropertySet > xSet( getModel(), UNO_QUERY );
    if ( !xSet.is() )
        return false;

    OUString sTitle = FRM_RES_STRING(RID_STR_IMPORT_GRAPHIC);
    // build some arguments for the upcoming dialog
    try
    {
        Reference< XWindowPeer > xWindowPeer = getPeer();
        VclPtr<vcl::Window> xWin = VCLUnoHelper::GetWindow(xWindowPeer);
        ::sfx2::FileDialogHelper aDialog(TemplateDescription::FILEOPEN_LINK_PREVIEW, FileDialogFlags::Graphic,
                                         xWin ? xWin->GetFrameWeld() : nullptr);
        aDialog.SetTitle( sTitle );

        Reference< XFilePickerControlAccess > xController( aDialog.GetFilePicker(), UNO_QUERY_THROW );
        xController->setValue(ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, css::uno::Any(true));

        Reference<XPropertySet> xBoundField;
        if ( hasProperty( PROPERTY_BOUNDFIELD, xSet ) )
            xSet->getPropertyValue( PROPERTY_BOUNDFIELD ) >>= xBoundField;
        bool bHasField = xBoundField.is();

        // if the control is bound to a DB field, then it's not possible to decide whether or not to link
        xController->enableControl(ExtendedFilePickerElementIds::CHECKBOX_LINK, !bHasField );

        // if the control is bound to a DB field, then linking of the image depends on the type of the field
        bool bImageIsLinked = true;
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
            implClearGraphics( false );
            bool bIsLink = false;
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
                xSet->setPropertyValue( PROPERTY_IMAGE_URL, makeAny( aDialog.GetPath() ) );

            return true;
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("OImageControlControl::implInsertGraphics: caught an exception while attempting to execute the FilePicker!");
    }
    return false;
}


bool OImageControlControl::impl_isEmptyGraphics_nothrow() const
{
    bool bIsEmpty = true;

    try
    {
        Reference< XPropertySet > xModelProps( const_cast< OImageControlControl* >( this )->getModel(), UNO_QUERY_THROW );
        Reference< XGraphic > xGraphic;
        OSL_VERIFY( xModelProps->getPropertyValue("Graphic") >>= xGraphic );
        bIsEmpty = !xGraphic.is();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("forms.component");
    }

    return bIsEmpty;
}

// MouseListener

void OImageControlControl::mousePressed(const css::awt::MouseEvent& e)
{
    SolarMutexGuard aGuard;

    if (e.Buttons != MouseButton::LEFT)
        return;

    bool bModified = false;
    // is this a request for a context menu?
    if ( e.PopupTrigger )
    {
        Reference< XPopupMenu > xMenu( awt::PopupMenu::create( m_xContext ) );
        DBG_ASSERT( xMenu.is(), "OImageControlControl::mousePressed: could not create a popup menu!" );

        Reference< XWindowPeer > xWindowPeer = getPeer();
        DBG_ASSERT( xWindowPeer.is(), "OImageControlControl::mousePressed: no window!" );

        if ( xMenu.is() && xWindowPeer.is() )
        {
            xMenu->insertItem( ID_OPEN_GRAPHICS, FRM_RES_STRING( RID_STR_OPEN_GRAPHICS ), 0, 0 );
            xMenu->insertItem( ID_CLEAR_GRAPHICS, FRM_RES_STRING( RID_STR_CLEAR_GRAPHICS ), 0, 1 );

            // check if the ImageURL is empty
            if ( impl_isEmptyGraphics_nothrow() )
                xMenu->enableItem( ID_CLEAR_GRAPHICS, false );

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
                implClearGraphics( true );
                bModified = true;
                break;
            }
        }
    }
    else
    {

        // Double click
        if (e.ClickCount == 2)
        {

            Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
            if (!xSet.is())
                return;

            // If the Control is not bound, do not display a dialog (because the to-be-sent URL would be invalid anyway)
            Reference<XPropertySet> xBoundField;
            if (hasProperty(PROPERTY_BOUNDFIELD, xSet))
                xBoundField.set(
                    xSet->getPropertyValue(PROPERTY_BOUNDFIELD),
                    css::uno::UNO_QUERY);
            if (!xBoundField.is())
            {
                // but only if our IMAGE_URL property is handled as if it is transient, which is equivalent to
                // an empty control source
                if ( !hasProperty(PROPERTY_CONTROLSOURCE, xSet) || !::comphelper::getString(xSet->getPropertyValue(PROPERTY_CONTROLSOURCE)).isEmpty() )
                    return;
            }

            bool bReadOnly = false;
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


void SAL_CALL OImageControlControl::mouseReleased(const awt::MouseEvent& /*e*/)
{
}


void SAL_CALL OImageControlControl::mouseEntered(const awt::MouseEvent& /*e*/)
{
}


void SAL_CALL OImageControlControl::mouseExited(const awt::MouseEvent& /*e*/)
{
}

}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OImageControlModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OImageControlModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OImageControlControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OImageControlControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
