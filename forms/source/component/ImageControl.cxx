/*************************************************************************
 *
 *  $RCSfile: ImageControl.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-02 10:28:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#include "conversion.hxx"
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef _UNTOOLS_UCBLOCKBYTES_HXX
#include <unotools/ucblockbytes.hxx>
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
#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_NOTCONNECTEDEXCEPTION_HPP_
#include <com/sun/star/io/NotConnectedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _SVX_IMPGRF_HXX
#include <svx/impgrf.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _UNOTOOLS_STREAMHELPER_HXX_
#include <unotools/streamhelper.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif

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
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
    {
        // my base class
        Sequence<Type> aBaseClassTypes = OBoundControlModel::_getTypes();

        Sequence<Type> aOwnTypes(1);
        Type* pOwnTypes = aOwnTypes.getArray();
        pOwnTypes[0] = getCppuType((Reference<XImageProducerSupplier>*)NULL);

        aTypes = concatSequences(aBaseClassTypes, aOwnTypes);
    }
    return aTypes;
}

DBG_NAME(OImageControlModel)
//------------------------------------------------------------------
OImageControlModel::OImageControlModel(const Reference<XMultiServiceFactory>& _rxFactory)
                    :OBoundControlModel(_rxFactory, VCL_CONTROLMODEL_IMAGECONTROL, FRM_CONTROL_IMAGECONTROL, sal_False)
                                    // use the old control name for compytibility reasons
                    ,OPropertyChangeListener(m_aMutex)
                    ,m_pImageProducer(new ImageProducer)
                    ,m_bReadOnly(sal_False)
{
    DBG_CTOR(OImageControlModel,NULL);
    m_nClassId = FormComponentType::IMAGECONTROL;

    m_xImageProducer = m_pImageProducer;

    m_sDataFieldConnectivityProperty = PROPERTY_IMAGE_URL;
    increment(m_refCount);
    if (m_xAggregateSet.is())
    {
        OPropertyChangeMultiplexer* pMultiplexer = new OPropertyChangeMultiplexer(this, m_xAggregateSet);
        pMultiplexer->addProperty(PROPERTY_IMAGE_URL);
    }
    decrement(m_refCount);
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
    Any aReturn = OBoundControlModel::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<XImageProducerSupplier*>(this)
        );

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
    //  INetURLObject aURLObj(getString(rEvt.NewValue));
    //  String aPath = INetURLObject::decode(aURLObj.PathToFileName(), '%', INetURLObject::DECODE_UNAMBIGUOUS);

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
                                           pFileStream->GetBufferSize()));
        xSink->setInputStream(xInput);
        Reference<XInputStream>  xInStream(xSink, UNO_QUERY);
        if (m_xColumnUpdate.is())
            m_xColumnUpdate->updateBinaryStream(xInStream, xInput->available());
        else
        {
            GetImageProducer()->setImage( xInStream );
            m_xImageProducer->startProduction();
        }

        // usually the setBinaryStream should close the input, but just in case ....
        try { xInStream->closeInput(); } catch (NotConnectedException& e) { e; }
    }
    else
    {
        if (m_xColumnUpdate.is())
            m_xColumnUpdate->updateNull();
        else
        {
            Reference<XInputStream> xInStream;
            GetImageProducer()->setImage( xInStream );
            m_xImageProducer->startProduction();
        }
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
void OImageControlModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue)
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
//      ModifyPropertyAttributes(_rAggregateProps, PROPERTY_IMAGE_URL, PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,             sal_Int16,                  READONLY, TRANSIENT);
        DECL_BOOL_PROP1(READONLY,                                   BOUND);
        DECL_PROP1(NAME,                ::rtl::OUString,            BOUND);
        DECL_PROP1(TAG,                 ::rtl::OUString,            BOUND);
        DECL_PROP1(CONTROLSOURCE,       ::rtl::OUString,            BOUND);
        DECL_IFACE_PROP2(BOUNDFIELD,    XPropertySet,   READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,  XPropertySet,   BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,  READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OImageControlModel::getInfoHelper()
{
    return *const_cast<OImageControlModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::rtl::OUString OImageControlModel::getServiceName()
{
    return FRM_COMPONENT_IMAGECONTROL;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OImageControlModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
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
            DBG_ERROR("OImageControlModel::read : unknown version !");
            m_bReadOnly = sal_False;
            defaultCommonProperties();
            break;
    }
    // Nach dem Lesen die Defaultwerte anzeigen
    if (m_aControlSource.getLength())
    {   // (not if we don't have a control source - the "State" property acts like it is persistent, then
        ::osl::MutexGuard aGuard(m_aMutex); // _reset expects this mutex guarding
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
    OBoundControlModel::disposing();

    Reference<XInputStream>  xInStream;
    GetImageProducer()->setImage( xInStream );
    m_xImageProducer->startProduction();
}

//------------------------------------------------------------------------------
void OImageControlModel::_reset()
{
    if(m_xField.is()) // only reset when we are connected to a column
    {
        Reference<XInputStream>  xDummy;
        GetImageProducer()->setImage(xDummy);
        Reference<XImageProducer> xProducer = m_xImageProducer;
        {   // release our mutex once (it's acquired in the calling method !), as starting the image production may
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
    Reference<XInputStream>  xInStream;
    xInStream = m_xColumn->getBinaryStream();

    GetImageProducer()->setImage(xInStream);

    m_xImageProducer->startProduction();
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
StringSequence  OImageControlControl::getSupportedServiceNames() throw()
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
}

// MouseListener
//------------------------------------------------------------------------------
void OImageControlControl::mousePressed(const ::com::sun::star::awt::MouseEvent& e)
{
    //////////////////////////////////////////////////////////////////////
    // Nur linke Maustaste
    if (e.Buttons != MouseButton::LEFT)
        return;

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

        ::rtl::OUString sTitle = FRM_RES_STRING(RID_STR_IMPORT_GRAPHIC);
        // build some arguments for the upcoming dialog
        Sequence<Any> aParams(4);
        aParams.getArray()[0] = makeAny(PropertyValue(
            ::rtl::OUString::createFromAscii("Title"), -1,
            makeAny(sTitle), PropertyState_DIRECT_VALUE
        ));
        aParams.getArray()[1] = makeAny(PropertyValue(
            ::rtl::OUString::createFromAscii("PreviewActive"), -1,
            makeAny((sal_Bool)sal_True), PropertyState_DIRECT_VALUE
        ));
        aParams.getArray()[2] = makeAny(PropertyValue(
            ::rtl::OUString::createFromAscii("AllowEmptyFileNames"), -1,
            makeAny((sal_Bool)sal_True), PropertyState_DIRECT_VALUE
        ));
        Reference<XWindow> xWindow(getPeer(), UNO_QUERY);
        aParams.getArray()[3] = makeAny(PropertyValue(
            ::rtl::OUString::createFromAscii("ParentWindow"), -1,
            makeAny(xWindow), PropertyState_DIRECT_VALUE
        ));

        try
        {
            Reference<XDialog> xDialog(
                m_xServiceFactory->createInstanceWithArguments(
                    ::rtl::OUString::createFromAscii("com.sun.star.dialogs.ImportGraphicsDialog"),
                    aParams),
                UNO_QUERY
            );

            Reference<XPropertySet> xDialogProps(xDialog, UNO_QUERY);
            if (xDialog.is() && xDialogProps.is() && xDialog->execute())
            {
                Any aSelectedPath = xDialogProps->getPropertyValue(::rtl::OUString::createFromAscii("SelectedPath"));
                if (aSelectedPath.getValueType().getTypeClass() == TypeClass_STRING)
                {
                    xSet->setPropertyValue(PROPERTY_IMAGE_URL, ::com::sun::star::uno::makeAny(::rtl::OUString()));
                        // reset the url property in case it already has the value we're about to set - in this case
                        // our propertyChanged would not get called without this.
                    xSet->setPropertyValue(PROPERTY_IMAGE_URL, aSelectedPath);
                }
            }
        }
        catch(Exception&)
        {
        }
    }
}

//.........................................................................
}   // namespace frm
//.........................................................................

