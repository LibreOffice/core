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

#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif

#ifndef _FRM_IMAGE_CONTROL_HXX_
#include "ImageControl.hxx"
#endif

#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif
#ifndef _FRM_RESOURCE_HXX_
#include "frm_resource.hxx"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef _COM_SUN_STAR_AWT_XPOPUPMENU_HPP_
#include <com/sun/star/awt/XPopupMenu.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POPUPMENUDIRECTION_HPP_
#include <com/sun/star/awt/PopupMenuDirection.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MOUSEBUTTON_HPP_
#include <com/sun/star/awt/MouseButton.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif


#ifndef _UNOTOOLS_STREAMHELPER_HXX_
#include <unotools/streamhelper.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif

#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

namespace binfilter {

#define ID_OPEN_GRAPHICS			1
#define	ID_CLEAR_GRAPHICS			2

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
//using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

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
    :OBoundControlModel(_rxFactory, VCL_CONTROLMODEL_IMAGECONTROL, FRM_CONTROL_IMAGECONTROL, sal_False, sal_False)
                    // use the old control name for compytibility reasons
    ,OPropertyChangeListener( m_aMutex )
    ,m_pImageProducer( new ImageProducer )
    ,m_bReadOnly( sal_False )
    ,m_pAggregatePropertyMultiplexer( NULL )
{
    DBG_CTOR( OImageControlModel, NULL );
    m_nClassId = FormComponentType::IMAGECONTROL;

    implConstruct();
}

//------------------------------------------------------------------
OImageControlModel::OImageControlModel( const OImageControlModel* _pOriginal, const Reference< XMultiServiceFactory >& _rxFactory )
    :OBoundControlModel( _pOriginal, _rxFactory, sal_False, sal_False )
                // use the old control name for compytibility reasons
    ,OPropertyChangeListener( m_aMutex )
    ,m_pImageProducer( NULL )
    ,m_pAggregatePropertyMultiplexer( NULL )
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
        _propertyChanged( PropertyChangeEvent( *this, PROPERTY_IMAGE_URL, sal_False, PROPERTY_ID_IMAGE_URL, Any( ), aImageURL ) );
    }
    osl_decrementInterlockedCount( &m_refCount );
}

//------------------------------------------------------------------
void OImageControlModel::implConstruct()
{
    m_pImageProducer = new ImageProducer;
    m_xImageProducer = m_pImageProducer;
    m_sDataFieldConnectivityProperty = PROPERTY_IMAGE_URL;

    increment(m_refCount);
    if ( m_xAggregateSet.is() )
    {
        m_pAggregatePropertyMultiplexer = new OPropertyChangeMultiplexer( this, m_xAggregateSet, sal_False );
        m_pAggregatePropertyMultiplexer->acquire();
        m_pAggregatePropertyMultiplexer->addProperty( PROPERTY_IMAGE_URL );
    }
    decrement(m_refCount);

    doSetDelegator();
}

//------------------------------------------------------------------
OImageControlModel::~OImageControlModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    doResetDelegator();

    if (m_pAggregatePropertyMultiplexer)
    {
        m_pAggregatePropertyMultiplexer->dispose();
        m_pAggregatePropertyMultiplexer->release();
        m_pAggregatePropertyMultiplexer = NULL;
    }

    DBG_DTOR(OImageControlModel,NULL);
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OImageControlModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence	OImageControlModel::getSupportedServiceNames() throw()
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
sal_Bool OImageControlModel::_approve(sal_Int32 _nColumnType)
{
    // zulaessing sind die binary Typen, OTHER- und LONGVARCHAR-Felder
    if ((_nColumnType == DataType::BINARY) || (_nColumnType == DataType::VARBINARY)
        || (_nColumnType == DataType::LONGVARBINARY) || (_nColumnType == DataType::OTHER)
        || (_nColumnType == DataType::LONGVARCHAR))
        return sal_True;

    return sal_False;
}


//------------------------------------------------------------------------------
void OImageControlModel::_propertyChanged( const PropertyChangeEvent& rEvt )
                                            throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // Wenn eine URL gesetzt worden ist, muss die noch an den ImageProducer
    // weitergereicht werden.
    // xInStream erzeugen

    Reference<XActiveDataSink>  xSink(
        m_xServiceFactory->createInstance(
        ::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")), UNO_QUERY);
    if (!xSink.is())
        return;

    // SvStream am xInStream setzen
    String aPath = getString(rEvt.NewValue);

    SvStream* pFileStream = ::utl::UcbStreamHelper::CreateStream(aPath, STREAM_READ);
    sal_Bool bSetNull = (NULL == pFileStream) || (ERRCODE_NONE != pFileStream->GetErrorCode());

    if (!bSetNull)
    {
        // get the size of the stream
        pFileStream->Seek(STREAM_SEEK_TO_END);
        sal_Int32 nSize = (sal_Int32)pFileStream->Tell();
        if (pFileStream->GetBufferSize() < 8192)
            pFileStream->SetBufferSize(8192);
        pFileStream->Seek(STREAM_SEEK_TO_BEGIN);

        Reference<XInputStream> xInput
            (new ::utl::OInputStreamHelper(new SvLockBytes(pFileStream, sal_True),
                                           nSize));
        xSink->setInputStream(xInput);
        Reference<XInputStream>  xInStream(xSink, UNO_QUERY);
        if (m_xColumnUpdate.is())
            m_xColumnUpdate->updateBinaryStream(xInStream, xInput->available());
        else
        {
            GetImageProducer()->setImage( xInStream );
            GetImageProducer()->startProduction();
        }

        // usually the setBinaryStream should close the input, but just in case ....
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
        if (m_xColumnUpdate.is())
            m_xColumnUpdate->updateNull();

        Reference< com::sun::star::io::XInputStream > xNull;
        GetImageProducer()->setImage(xNull);
        GetImageProducer()->startProduction();

        delete pFileStream;
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
Reference<XPropertySetInfo> SAL_CALL OImageControlModel::getPropertySetInfo() throw( RuntimeException )
{
    Reference<XPropertySetInfo>  xInfo(createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void OImageControlModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(8)
//		ModifyPropertyAttributes(_rAggregateProps, PROPERTY_IMAGE_URL, PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,				sal_Int16,			READONLY, TRANSIENT);
        DECL_BOOL_PROP1(READONLY,							BOUND);
        DECL_PROP1(NAME,				::rtl::OUString,	BOUND);
        DECL_PROP1(TAG,					::rtl::OUString,	BOUND);
        DECL_PROP1(CONTROLSOURCE,		::rtl::OUString,	BOUND);
        DECL_IFACE_PROP3(BOUNDFIELD,	XPropertySet,		BOUND,READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,	XPropertySet,		BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,	rtl::OUString,	READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OImageControlModel::getInfoHelper()
{
    return *const_cast<OImageControlModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::rtl::OUString OImageControlModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_IMAGECONTROL;	// old (non-sun) name for compatibility !
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
    if (m_aControlSource.getLength())
    {	// (not if we don't have a control source - the "State" property acts like it is persistent, then
        ::osl::MutexGuard aGuard(m_aMutex);	// _reset expects this mutex guarding
        _reset();
    }
}

// XPropertyChangeListener
//------------------------------------------------------------------------------
void OImageControlModel::_onValueChanged()
{
    UpdateFromField();
}

//------------------------------------------------------------------------------
Any OImageControlModel::_getControlValue() const
{
    // hier macht ein Vergleich keinen Sinn, daher void siehe OBoundControlModel
    return Any();
}

// OComponentHelper
//------------------------------------------------------------------
void OImageControlModel::disposing()
{
    if (m_pAggregatePropertyMultiplexer)
        m_pAggregatePropertyMultiplexer->dispose();

    OBoundControlModel::disposing();

    Reference<XInputStream>  xInStream;
    GetImageProducer()->setImage( xInStream );
    m_xImageProducer->startProduction();
}

//------------------------------------------------------------------------------
void OImageControlModel::_reset()
{
    if(getField().is()) // only reset when we are connected to a column
    {
        Reference<XInputStream>  xDummy;
        GetImageProducer()->setImage(xDummy);
        Reference<XImageProducer> xProducer = m_xImageProducer;
        {	// release our mutex once (it's acquired in the calling method !), as starting the image production may
            // result in the locking of the solar mutex (unfortunally the default implementation of our aggregate,
            // VCLXImageControl, does this locking)
            // FS - 74438 - 30.03.00
            MutexRelease aRelease(m_aMutex);
            xProducer->startProduction();
        }
    }
}

// Helper functions
//------------------------------------------------------------------------------
void OImageControlModel::UpdateFromField()
{
    Reference<XInputStream>  xInStream = m_xColumn->getBinaryStream();
    GetImageProducer()->setImage(xInStream);
    GetImageProducer()->startProduction();
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
{
    increment(m_refCount);
    {
        // als Focus- und MouseListener anmelden
        Reference<XWindow>  xComp;
        query_aggregation( m_xAggregate, xComp);
        if (xComp.is())
            xComp->addMouseListener(this);
    }
    // Refcount bei 1 fuer den Listener
    sal_Int32 n = decrement(m_refCount);
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
StringSequence	OImageControlControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_IMAGECONTROL;
    return aSupported;
}

// XControl
//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::createPeer(const Reference<XToolkit>& _rxToolkit, const Reference<XWindowPeer>& Parent) throw( RuntimeException )
{
    OBoundControl::createPeer(_rxToolkit, Parent);
    // the following is not necessary anymore. The aggregated control (from the toolkit project)
    // itself will register as image consumer at the image producer, so there's no need to do this ourself.
    // This holds since our model is an XImageProducer itself, and thus hiding the XImageProducer of the aggregated
    // model. Before, we had two ImageProducers working in parallel.
    // 2003-05-15 - 109591 - fs@openoffice.org

/**
    if (!m_xControl.is())
        return;

    // ImageConsumer vom Control holen
    Reference<XWindowPeer>  xPeer = m_xControl->getPeer();
    Reference<XImageConsumer>  xImageConsumer(xPeer, UNO_QUERY);
    if (!xImageConsumer.is())
        return;

    // ImageConsumer am Imageproducer setzen
    Reference<XImageProducerSupplier>  xImageSource(m_xControl->getModel(), UNO_QUERY);
    if (!xImageSource.is())
        return;
    Reference<XImageProducer>  xImageProducer = xImageSource->getImageProducer();

    xImageProducer->addConsumer(xImageConsumer);
    xImageProducer->startProduction();
*/
}

//------------------------------------------------------------------------------
void OImageControlControl::implClearGraphics()
{
    Reference< XPropertySet > xSet( getModel(), UNO_QUERY );
    if ( xSet.is() )
        xSet->setPropertyValue( PROPERTY_IMAGE_URL, makeAny( ::rtl::OUString() ) );
}

//------------------------------------------------------------------------------
void OImageControlControl::implInsertGraphics()
{
//  Reference< XPropertySet > xSet( getModel(), UNO_QUERY );
//  if ( !xSet.is() )
//      return;

//  ::rtl::OUString sTitle = FRM_RES_STRING(RID_STR_IMPORT_GRAPHIC);
//  // build some arguments for the upcoming dialog
//  try
//  {
//      ::binfilter::sfx2::FileDialogHelper aDialog( ::binfilter::sfx2::FILEOPEN_LINK_PREVIEW, SFXWB_GRAPHIC );//STRIP008       ::sfx2::FileDialogHelper aDialog( ::sfx2::FILEOPEN_LINK_PREVIEW, SFXWB_GRAPHIC );
//      aDialog.SetTitle( sTitle );

//      Reference< XFilePickerControlAccess > xController(aDialog.GetFilePicker(), UNO_QUERY);
//      DBG_ASSERT( xController.is(), "OImageControlControl::implInsertGraphics: invalid file picker!" );
//      if ( xController.is() )
//      {
//          xController->setValue(ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, ::cppu::bool2any(sal_True));
//          xController->enableControl(ExtendedFilePickerElementIds::CHECKBOX_LINK, sal_False);
//      }

//      if ( ERRCODE_NONE == aDialog.Execute() )
//      {
//          // reset the url property in case it already has the value we're about to set - in this case
//          // our propertyChanged would not get called without this.
//          implClearGraphics();

//          xSet->setPropertyValue( PROPERTY_IMAGE_URL, makeAny( ::rtl::OUString( aDialog.GetPath() ) ) );
//      }
//  }
//  catch(Exception&)
//  {
//      DBG_ERROR("OImageControlControl::implInsertGraphics: caught an exception while attempting to execute the FilePicker!");
//  }
}

// MouseListener
//------------------------------------------------------------------------------
void OImageControlControl::mousePressed(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException)
{
    //////////////////////////////////////////////////////////////////////
    // Nur linke Maustaste
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
            ::rtl::OUString sCurrentURL;
            Reference< XPropertySet > xSet( getModel(), UNO_QUERY );
            if ( xSet.is() )
                xSet->getPropertyValue( PROPERTY_IMAGE_URL ) >>= sCurrentURL;
            if ( 0 == sCurrentURL.getLength() )
                xMenu->enableItem( ID_CLEAR_GRAPHICS, sal_False );

            awt::Rectangle aRect( e.X, e.Y, 0, 0 );
            if ( ( e.X < 0 ) || ( e.Y < 0 ) )
            {	// context menu triggered by keyboard
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
                implClearGraphics();
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

            sal_Bool bReadOnly;
            xSet->getPropertyValue(PROPERTY_READONLY) >>= bReadOnly;
            if (bReadOnly)
                return;

            implInsertGraphics();
        }
    }
}

//.........................................................................
}	// namespace frm
//.........................................................................

}
