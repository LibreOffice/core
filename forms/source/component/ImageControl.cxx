/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ImageControl.cxx,v $
 * $Revision: 1.49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "ImageControl.hxx"

#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif
#include "frm_resource.hxx"
#include "services.hxx"
#include <svtools/imageresourceaccess.hxx>
#include <unotools/ucblockbytes.hxx>
#include <sfx2/filedlghelper.hxx>
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
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <unotools/streamhelper.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/guarding.hxx>
#include <unotools/ucbstreamhelper.hxx>

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

//==================================================================
// OImageControlModel
//==================================================================

//------------------------------------------------------------------
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
    ,m_bReadOnly( sal_False )
{
    DBG_CTOR( OImageControlModel, NULL );
    m_nClassId = FormComponentType::IMAGECONTROL;
    initValueProperty( PROPERTY_IMAGE_URL, PROPERTY_ID_IMAGE_URL);

    implConstruct();
}

//------------------------------------------------------------------
OImageControlModel::OImageControlModel( const OImageControlModel* _pOriginal, const Reference< XMultiServiceFactory >& _rxFactory )
    :OBoundControlModel( _pOriginal, _rxFactory )
                // use the old control name for compytibility reasons
    ,m_pImageProducer( NULL )
{
    DBG_CTOR( OImageControlModel, NULL );
    implConstruct();
    m_bReadOnly = _pOriginal->m_bReadOnly;

    osl_incrementInterlockedCount( &m_refCount );
    {
        // simulate a propertyChanged event for the ImageURL
        // 2003-05-15 - #109591# - fs@openoffice.org
        Any aImageURL;
        getFastPropertyValue( aImageURL, PROPERTY_ID_IMAGE_URL );
        ::rtl::OUString sImageURL;
        aImageURL >>= sImageURL;

        ::osl::MutexGuard aGuard( m_aMutex );   // handleNewImageURL expects this
        handleNewImageURL( sImageURL, eOther );
    }
    osl_decrementInterlockedCount( &m_refCount );
}

//------------------------------------------------------------------
void OImageControlModel::implConstruct()
{
    m_pImageProducer = new ImageProducer;
    m_xImageProducer = m_pImageProducer;
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
    // oder matters: we want to "override" the XImageProducer interface of the aggreate with out
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
sal_Bool OImageControlModel::approveDbColumnType(sal_Int32 _nColumnType)
{
    // zulaessing sind die binary Typen, OTHER- und LONGVARCHAR-Felder
    if ((_nColumnType == DataType::BINARY) || (_nColumnType == DataType::VARBINARY)
        || (_nColumnType == DataType::LONGVARBINARY) || (_nColumnType == DataType::OTHER)
        || (_nColumnType == DataType::LONGVARCHAR))
        return sal_True;

    return sal_False;
}


//------------------------------------------------------------------------------
void OImageControlModel::_propertyChanged( const PropertyChangeEvent& _rEvent )
                                            throw( RuntimeException )
{
    if ( m_xColumnUpdate.is() )
    {
        OBoundControlModel::_propertyChanged( _rEvent );
    }
    else
    {   // we're not bound. In this case, we have to manually care for updating the
        // image producer, since the base class will not do this
        ::rtl::OUString sImageURL;
        _rEvent.NewValue >>= sImageURL;

        ::osl::MutexGuard aGuard( m_aMutex );   // handleNewImageURL expects this
        handleNewImageURL( sImageURL, eOther );
    }
}

//------------------------------------------------------------------------------
void OImageControlModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_READONLY       : rValue <<= (sal_Bool)m_bReadOnly; break;
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

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue);
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

        default:
            return OBoundControlModel::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);
    }
}

//------------------------------------------------------------------------------
void OImageControlModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 2, OBoundControlModel )
        DECL_BOOL_PROP1 ( READONLY,             BOUND );
        DECL_PROP1      ( TABINDEX, sal_Int16,  BOUND );
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
::rtl::OUString OImageControlModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_IMAGECONTROL;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OImageControlModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    // Basisklasse
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
            DBG_ERROR("OImageControlModel::read : unknown version !");
            m_bReadOnly = sal_False;
            defaultCommonProperties();
            break;
    }
    // Nach dem Lesen die Defaultwerte anzeigen
    if ( getControlSource().getLength() )
    {   // (not if we don't have a control source - the "State" property acts like it is persistent, then
        ::osl::MutexGuard aGuard(m_aMutex); // resetNoBroadcast expects this mutex guarding
        resetNoBroadcast();
    }
}

//------------------------------------------------------------------------------
sal_Bool OImageControlModel::handleNewImageURL( const ::rtl::OUString& _rURL, ValueChangeInstigator _eInstigator )
{
    // if the image URL has been set, we have to forward this to the image producer
    // xInStream erzeugen
    Reference< XActiveDataSink > xSink(
        m_xServiceFactory->createInstance(
        ::rtl::OUString::createFromAscii( "com.sun.star.io.ObjectInputStream" ) ), UNO_QUERY );
    if ( !xSink.is() )
        return sal_False;

    // create a stream for the image specified by the URL
    ::std::auto_ptr< SvStream > pImageStream;
    Reference< XInputStream > xImageStream;

    if ( ::svt::ImageResourceAccess::isImageResourceURL( _rURL ) )
    {
        xImageStream = ::svt::ImageResourceAccess::getImageXStream( getORB(), _rURL );
    }
    else
    {
        pImageStream.reset( ::utl::UcbStreamHelper::CreateStream( _rURL, STREAM_READ ) );
        sal_Bool bSetNull = ( pImageStream.get() == NULL ) || ( ERRCODE_NONE != pImageStream->GetErrorCode() );

        if (!bSetNull)
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
        xSink->setInputStream( xImageStream );
        Reference< XInputStream >  xInStream(xSink, UNO_QUERY);

        if ( m_xColumnUpdate.is() )
            updateColumnWithStream( xInStream );
        else
            setControlValue( makeAny( xInStream ), _eInstigator );

        // close the stream, just to be on the safe side (should have been done elsewhere ...)
        try
        {
            xInStream->closeInput();
        }
        catch (NotConnectedException&)
        {
        }
    }
    else
    {
        if ( m_xColumnUpdate.is() )
            updateColumnWithStream( NULL );
        else
            setControlValue( Any(), _eInstigator );
    }

    return sal_True;
}

//------------------------------------------------------------------------------
sal_Bool OImageControlModel::commitControlValueToDbColumn( bool _bPostReset )
{
    if ( _bPostReset )
    {
        // since this is a "commit after reset", we can simply update the column
        // with null - this is our "default" which we were just reset to
        updateColumnWithStream( NULL );
    }
    else
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        ::rtl::OUString sImageURL;
        m_xAggregateSet->getPropertyValue( PROPERTY_IMAGE_URL ) >>= sImageURL;
        return handleNewImageURL( sImageURL, eDbColumnBinding );
    }

    return sal_True;
}

//------------------------------------------------------------------------------
void OImageControlModel::updateColumnWithStream( const Reference< XInputStream >& _rxStream )
{
    OSL_PRECOND( m_xColumnUpdate.is(), "OImageControlModel::updateColumnWithStream: no column update interface!" );
    if ( m_xColumnUpdate.is() )
    {
        if ( _rxStream.is() )
            m_xColumnUpdate->updateBinaryStream( _rxStream, _rxStream->available() );
        else
            m_xColumnUpdate->updateNull();

        // note that this will fire a value change for the column, which
        // will result in us (our base class, actually) syncing the
        // db column content to our control content - finally, this will arrive
        // in setControlValue
    }
}

//------------------------------------------------------------------------------
Any OImageControlModel::translateDbColumnToControlValue()
{
    return makeAny( m_xColumn->getBinaryStream() );
}

//------------------------------------------------------------------------------
void OImageControlModel::doSetControlValue( const Any& _rValue )
{
    DBG_ASSERT( GetImageProducer() && m_xImageProducer.is(), "OImageControlModel::doSetControlValue: no image producer!" );
    if ( !GetImageProducer() || !m_xImageProducer.is() )
        return;

    // give the image producer the stream
    Reference< XInputStream > xInStream;
    _rValue >>= xInStream;
    GetImageProducer()->setImage( xInStream );

    // and start production
    Reference< XImageProducer > xProducer = m_xImageProducer;
    {
        // release our mutex once (it's acquired in the calling method!), as starting the image production may
        // result in the locking of the solar mutex (unfortunally the default implementation of our aggregate,
        // VCLXImageControl, does this locking)
        // FS - 74438 - 30.03.00
        MutexRelease aRelease(m_aMutex);
        xProducer->startProduction();
    }
}

// OComponentHelper
//------------------------------------------------------------------
void OImageControlModel::disposing()
{
    OBoundControlModel::disposing();

    {
        ::osl::MutexGuard aGuard( m_aMutex ); // setControlValue expects this
        setControlValue( Any(), eOther );
    }
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
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
    {
        // my base class
        Sequence<Type> aBaseClassTypes = OBoundControl::_getTypes();

        Sequence<Type> aOwnTypes(1);
        Type* pOwnTypes = aOwnTypes.getArray();
        pOwnTypes[0] = getCppuType((Reference<XMouseListener>*)NULL);

        aTypes = concatSequences(aBaseClassTypes, aOwnTypes);
    }
    return aTypes;
}

//------------------------------------------------------------------------------
OImageControlControl::OImageControlControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_IMAGECONTROL)
    ,m_pImageIndicator( new OImageIndicator )
{
    increment(m_refCount);
    {
        // als Focus- und MouseListener anmelden
        Reference< XWindow > xComp;
        query_aggregation( m_xAggregate, xComp );
        if ( xComp.is() )
            xComp->addMouseListener( this );
    }
    decrement(m_refCount);
}

// UNO Anbindung
//------------------------------------------------------------------------------
Any SAL_CALL OImageControlControl::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OBoundControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<XMouseListener*>(this)
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

//--------------------------------------------------------------------
sal_Bool SAL_CALL OImageControlControl::setModel(const Reference<starawt::XControlModel>& _rxModel ) throw (RuntimeException)
{
    Reference< XImageProducer > xProducer( getModel(), UNO_QUERY );
    if ( xProducer.is() )
        xProducer->removeConsumer( m_pImageIndicator.getRef() );

    sal_Bool bReturn = OBoundControl::setModel( _rxModel );

    xProducer = xProducer.query( getModel() );
    if ( xProducer.is() )
    {
        m_pImageIndicator->reset();
        xProducer->addConsumer( m_pImageIndicator.getRef() );
    }

    return bReturn;
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

            if ( !sOldImageURL.getLength() )
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
void OImageControlControl::implInsertGraphics()
{
    Reference< XPropertySet > xSet( getModel(), UNO_QUERY );
    if ( !xSet.is() )
        return;

    ::rtl::OUString sTitle = FRM_RES_STRING(RID_STR_IMPORT_GRAPHIC);
    // build some arguments for the upcoming dialog
    try
    {
        ::sfx2::FileDialogHelper aDialog( TemplateDescription::FILEOPEN_LINK_PREVIEW, SFXWB_GRAPHIC );
        aDialog.SetTitle( sTitle );

        Reference< XFilePickerControlAccess > xController(aDialog.GetFilePicker(), UNO_QUERY);
        DBG_ASSERT( xController.is(), "OImageControlControl::implInsertGraphics: invalid file picker!" );
        if ( xController.is() )
        {
            xController->setValue(ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, ::cppu::bool2any(sal_True));
            xController->enableControl(ExtendedFilePickerElementIds::CHECKBOX_LINK, sal_False);
        }

        if ( ERRCODE_NONE == aDialog.Execute() )
        {
            // reset the url property in case it already has the value we're about to set - in this case
            // our propertyChanged would not get called without this.
            implClearGraphics( sal_False );

            xSet->setPropertyValue( PROPERTY_IMAGE_URL, makeAny( ::rtl::OUString( aDialog.GetPath() ) ) );
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("OImageControlControl::implInsertGraphics: caught an exception while attempting to execute the FilePicker!");
    }
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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if (e.Buttons != MouseButton::LEFT)
        return;

    // is this a request for a context menu?
    if ( e.PopupTrigger )
    {
        Reference< XPopupMenu > xMenu( m_xServiceFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.awt.PopupMenu" ) ), UNO_QUERY );
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
                // 102205 - 16.08.2002 - fs@openoffice.org
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
                break;

            case ID_CLEAR_GRAPHICS:
                implClearGraphics( sal_True );
                break;
            }
        }
    }
    else
    {
        //////////////////////////////////////////////////////////////////////
        // Doppelclick
        if (e.ClickCount == 2)
        {

            Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
            if (!xSet.is())
                return;

            // wenn Control nicht gebunden ist, kein Dialog (da die zu schickende URL hinterher sowieso
            // versanden wuerde)
            // FS - #64946# - 19.04.99
            Reference<XPropertySet> xBoundField;
            if (hasProperty(PROPERTY_BOUNDFIELD, xSet))
                ::cppu::extractInterface(xBoundField, xSet->getPropertyValue(PROPERTY_BOUNDFIELD));
            if (!xBoundField.is())
            {
                // but only if our IMAGE_URL property is handled as if it is transient, which is equivalent to
                // an empty control source
                if (!hasProperty(PROPERTY_CONTROLSOURCE, xSet) || (::comphelper::getString(xSet->getPropertyValue(PROPERTY_CONTROLSOURCE)).getLength() != 0))
                    return;
            }

            sal_Bool bReadOnly = false;
            xSet->getPropertyValue(PROPERTY_READONLY) >>= bReadOnly;
            if (bReadOnly)
                return;

            implInsertGraphics();
        }
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::mouseReleased(const awt::MouseEvent& /*e*/) throw ( RuntimeException )
{
    // TODO: place your code here
}

//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::mouseEntered(const awt::MouseEvent& /*e*/) throw ( RuntimeException )
{
    // TODO: place your code here
}

//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::mouseExited(const awt::MouseEvent& /*e*/) throw ( RuntimeException )
{
    // TODO: place your code here
}


//==============================================================================
//= OImageIndicator
//==============================================================================
DBG_NAME( OImageIndicator )
//------------------------------------------------------------------------------
OImageIndicator::OImageIndicator( )
    :m_bIsProducing( sal_False )
    ,m_bIsEmptyImage( sal_True )
{
    DBG_CTOR( OImageIndicator, NULL );
}

//------------------------------------------------------------------------------
OImageIndicator::~OImageIndicator( )
{
    DBG_DTOR( OImageIndicator, NULL );
}

//--------------------------------------------------------------------
void OImageIndicator::reset()
{
    OSL_ENSURE( !m_bIsProducing, "OImageIndicator::reset: sure you know what you're doing? The producer is currently producing!" );
    m_bIsProducing = sal_True;
}

//--------------------------------------------------------------------
void SAL_CALL OImageIndicator::init( sal_Int32 /*Width*/, sal_Int32 /*Height*/ ) throw (RuntimeException)
{
    m_bIsProducing = sal_True;
    m_bIsEmptyImage = sal_True;
}

//--------------------------------------------------------------------
void SAL_CALL OImageIndicator::setColorModel( sal_Int16 /*BitCount*/, const Sequence< sal_Int32 >& /*RGBAPal*/, sal_Int32 /*RedMask*/, sal_Int32 /*GreenMask*/, sal_Int32 /*BlueMask*/, sal_Int32 /*AlphaMask*/ ) throw (RuntimeException)
{
}

//--------------------------------------------------------------------
void SAL_CALL OImageIndicator::setPixelsByBytes( sal_Int32 /*nX*/, sal_Int32 /*nY*/, sal_Int32 /*nWidth*/, sal_Int32 /*nHeight*/, const Sequence< sal_Int8 >& /*aProducerData*/, sal_Int32 /*nOffset*/, sal_Int32 /*nScanSize*/ ) throw (RuntimeException)
{
    m_bIsEmptyImage = sal_False;
}

//--------------------------------------------------------------------
void SAL_CALL OImageIndicator::setPixelsByLongs( sal_Int32 /*nX*/, sal_Int32 /*nY*/, sal_Int32 /*nWidth*/, sal_Int32 /*nHeight*/, const Sequence< sal_Int32 >& /*aProducerData*/, sal_Int32 /*nOffset*/, sal_Int32 /*nScanSize*/ ) throw (RuntimeException)
{
    m_bIsEmptyImage = sal_False;
}

//--------------------------------------------------------------------
void SAL_CALL OImageIndicator::complete( sal_Int32 /*Status*/, const Reference< XImageProducer >& /*xProducer*/ ) throw (RuntimeException)
{
    m_bIsProducing = sal_False;
}

//.........................................................................
}   // namespace frm
//.........................................................................

