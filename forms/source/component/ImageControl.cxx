/*************************************************************************
 *
 *  $RCSfile: ImageControl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-19 11:52:16 $
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
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OImageControlModel
//==================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OImageControlModel_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new OImageControlModel(_rxFactory));
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> OImageControlModel::_getTypes()
{
    static staruno::Sequence<staruno::Type> aTypes;
    if (!aTypes.getLength())
    {
        // my base class
        staruno::Sequence<staruno::Type> aBaseClassTypes = OBoundControlModel::_getTypes();

        staruno::Sequence<staruno::Type> aOwnTypes(1);
        staruno::Type* pOwnTypes = aOwnTypes.getArray();
        pOwnTypes[0] = getCppuType((staruno::Reference<starform::XImageProducerSupplier>*)NULL);

        aTypes = concatSequences(aBaseClassTypes, aOwnTypes);
    }
    return aTypes;
}

DBG_NAME(OImageControlModel)
//------------------------------------------------------------------
OImageControlModel::OImageControlModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
                    :OBoundControlModel(_rxFactory, VCL_CONTROLMODEL_IMAGECONTROL, FRM_CONTROL_IMAGECONTROL, sal_False)
                                    // use the old control name for compytibility reasons
                    ,OPropertyChangeListener(m_aMutex)
                    ,m_pImageProducer(new ImageProducer)
                    ,m_bReadOnly(sal_False)
{
    DBG_CTOR(OImageControlModel,NULL);

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

// starlang::XServiceInfo
//------------------------------------------------------------------------------
StringSequence  OImageControlModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString::createFromAscii("com.sun.star.form.component.starform");
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_IMAGECONTROL;
    return aSupported;
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OImageControlModel::queryAggregation(const staruno::Type& _rType) throw (staruno::RuntimeException)
{
    staruno::Any aReturn;

    aReturn = OBoundControlModel::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<starform::XImageProducerSupplier*>(this)
        );

    return aReturn;
}

//------------------------------------------------------------------------------
sal_Bool OImageControlModel::_approve(sal_Int32 _nColumnType)
{
    // zulaessing sind die binary Typen, OTHER- und LONGVARCHAR-Felder
    if ((_nColumnType == starsdbc::DataType::BINARY) || (_nColumnType == starsdbc::DataType::VARBINARY)
        || (_nColumnType == starsdbc::DataType::LONGVARBINARY) || (_nColumnType == starsdbc::DataType::OTHER)
        || (_nColumnType == starsdbc::DataType::LONGVARCHAR))
        return sal_True;

    return sal_False;
}


//------------------------------------------------------------------------------
void OImageControlModel::_propertyChanged( const starbeans::PropertyChangeEvent& rEvt )
                                            throw( staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // Wenn eine starutil::URL gesetzt worden ist, muss die noch an den ImageProducer
    // weitergereicht werden.
    // xInStream erzeugen

    staruno::Reference<stario::XActiveDataSink>  xSink(
        m_xServiceFactory->createInstance(
        ::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")), staruno::UNO_QUERY);
    if (!xSink.is())
        return;

    // SvStream am xInStream setzen
    INetURLObject aURLObj(getString(rEvt.NewValue));
    String aPath = INetURLObject::decode(aURLObj.PathToFileName(), '%', INetURLObject::DECODE_UNAMBIGUOUS);

    SvFileStream* pFileStream = new SvFileStream();
    pFileStream->Open(aPath, STREAM_READ);
    sal_Bool bSetNull = !pFileStream->IsOpen();

    if (!bSetNull)
    {
        // get the size of the stream
        pFileStream->Seek(STREAM_SEEK_TO_END);
        sal_Int32 nSize = (sal_Int32)pFileStream->Tell();
        if (pFileStream->GetBufferSize() < 8192)
            pFileStream->SetBufferSize(8192);

        staruno::Reference<stario::XInputStream> xInput
            (new ::utl::OInputStreamHelper(new SvLockBytes(pFileStream, sal_True),
                                           pFileStream->GetBufferSize()));
        xSink->setInputStream(xInput);
        staruno::Reference<stario::XInputStream>  xInStream(xSink, staruno::UNO_QUERY);
        if (m_xColumnUpdate.is())
            m_xColumnUpdate->updateBinaryStream(xInStream, xInput->available());
        else
        {
            GetImageProducer()->setImage( xInStream );
            m_xImageProducer->startProduction();
        }

        // usually the setBinaryStream should close the input, but just in case ....
        try { xInStream->closeInput(); } catch (stario::NotConnectedException& e) { e; }
    }
    else
    {
        if (m_xColumnUpdate.is())
            m_xColumnUpdate->updateNull();
        else
        {
            staruno::Reference<stario::XInputStream> xInStream;
            GetImageProducer()->setImage( xInStream );
            m_xImageProducer->startProduction();
        }
        delete pFileStream;
    }
}

//------------------------------------------------------------------------------
void OImageControlModel::getFastPropertyValue(staruno::Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_READONLY       : rValue <<= (sal_Bool)m_bReadOnly; break;
        default:
            OBoundControlModel::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
void OImageControlModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const staruno::Any& rValue)
{
    switch (nHandle)
    {
        case PROPERTY_ID_READONLY :
            DBG_ASSERT(rValue.getValueType().getTypeClass() == staruno::TypeClass_BOOLEAN, "OImageControlModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            m_bReadOnly = getBOOL(rValue);
            break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue);
    }
}

//------------------------------------------------------------------------------
sal_Bool OImageControlModel::convertFastPropertyValue(staruno::Any& rConvertedValue, staruno::Any& rOldValue, sal_Int32 nHandle, const staruno::Any& rValue)
                                throw( starlang::IllegalArgumentException )
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
staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL OImageControlModel::getPropertySetInfo() throw( staruno::RuntimeException )
{
    staruno::Reference<starbeans::XPropertySetInfo>  xInfo(createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void OImageControlModel::fillProperties(
        staruno::Sequence< starbeans::Property >& _rProps,
        staruno::Sequence< starbeans::Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(9)
//      ModifyPropertyAttributes(_rAggregateProps, PROPERTY_IMAGE_URL, starbeans::PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,             sal_Int16,                  READONLY, TRANSIENT);
        DECL_BOOL_PROP1(READONLY,                                   BOUND);
        DECL_PROP1(NAME,                ::rtl::OUString,            BOUND);
        DECL_PROP1(TAG,                 ::rtl::OUString,            BOUND);
        DECL_PROP1(CONTROLSOURCE,       ::rtl::OUString,            BOUND);
        DECL_PROP1(HELPTEXT,            ::rtl::OUString,            BOUND);
        DECL_IFACE_PROP2(BOUNDFIELD,    starbeans::XPropertySet,    READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,  starbeans::XPropertySet,    BOUND, MAYBEVOID);
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
void OImageControlModel::write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream)
{
    // Basisklasse
    OBoundControlModel::write(_rxOutStream);
    // Version
    _rxOutStream->writeShort(0x0003);
    // Name
    _rxOutStream->writeBoolean(m_bReadOnly);
    _rxOutStream << m_aHelpText;
    // from version 0x0003 : common properties
    writeCommonProperties(_rxOutStream);
}

//------------------------------------------------------------------------------
void OImageControlModel::read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream)
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
            _rxInStream >> m_aHelpText;
            break;
        case 0x0003:
            m_bReadOnly = _rxInStream->readBoolean();
            _rxInStream >> m_aHelpText;
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

// starbeans::XPropertyChangeListener
//------------------------------------------------------------------------------
void OImageControlModel::_onValueChanged()
{
    UpdateFromField();
}

//------------------------------------------------------------------------------
staruno::Any OImageControlModel::_getControlValue() const
{
    // hier macht ein Vergleich keinen Sinn, daher void siehe OBoundControlModel
    return staruno::Any();
}

// OComponentHelper
//------------------------------------------------------------------
void OImageControlModel::disposing()
{
    OBoundControlModel::disposing();

#if SUPD<583
    XInputStreamRef  xInStream;
#else
    staruno::Reference<stario::XInputStream>  xInStream;
#endif
    GetImageProducer()->setImage( xInStream );
    m_xImageProducer->startProduction();
}

//------------------------------------------------------------------------------
void OImageControlModel::_reset()
{
#if SUPD<583
    XInputStreamRef  xDummy;
#else
    staruno::Reference<stario::XInputStream>  xDummy;
#endif
    GetImageProducer()->setImage(xDummy);
    staruno::Reference<starawt::XImageProducer> xProducer = m_xImageProducer;
    {   // release our mutex once (it's acquired in the calling method !), as starting the image production may
        // result in the locking of the solar mutex (unfortunally the default implementation of our aggregate,
        // VCLXImageControl, does this locking)
        // FS - 74438 - 30.03.00
        MutexRelease aRelease(m_aMutex);
        xProducer->startProduction();
    }
}

// Helper functions
//------------------------------------------------------------------------------
void OImageControlModel::UpdateFromField()
{
    staruno::Reference<stario::XInputStream>  xInStream;
    xInStream = m_xColumn->getBinaryStream();

#if SUPD<583
    //------------------------------------------------------------
    // temporary as long as the ImageProducer is a Smart-UNO-Class
    XInputStreamRef xUsrIFace;
    convertIFace(xInStream, xUsrIFace);
    GetImageProducer()->setImage(xUsrIFace);
    //------------------------------------------------------------
#else
    GetImageProducer()->setImage(xInStream);
#endif

    m_xImageProducer->startProduction();
}

//==================================================================
// OImageControlControl
//==================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OImageControlControl_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new OImageControlControl(_rxFactory));
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> OImageControlControl::_getTypes()
{
    static staruno::Sequence<staruno::Type> aTypes;
    if (!aTypes.getLength())
    {
        // my base class
        staruno::Sequence<staruno::Type> aBaseClassTypes = OBoundControl::_getTypes();

        staruno::Sequence<staruno::Type> aOwnTypes(1);
        staruno::Type* pOwnTypes = aOwnTypes.getArray();
        pOwnTypes[0] = getCppuType((staruno::Reference<starawt::XMouseListener>*)NULL);

        aTypes = concatSequences(aBaseClassTypes, aOwnTypes);
    }
    return aTypes;
}

//------------------------------------------------------------------------------
OImageControlControl::OImageControlControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
                       :OBoundControl(_rxFactory, VCL_CONTROL_IMAGECONTROL)
{
    increment(m_refCount);
    {
        // als Focus- und MouseListener anmelden
        staruno::Reference<starawt::XWindow>  xComp;
        query_aggregation( m_xAggregate, xComp);
        if (xComp.is())
            xComp->addMouseListener(this);
    }
    // Refcount bei 1 fuer den Listener
    sal_Int32 n = decrement(m_refCount);
}

// UNO Anbindung
//------------------------------------------------------------------------------
staruno::Any SAL_CALL OImageControlControl::queryAggregation(const staruno::Type& _rType) throw (staruno::RuntimeException)
{
    staruno::Any aReturn;

    aReturn = OBoundControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<starawt::XMouseListener*>(this)
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

// starawt::XControl
//------------------------------------------------------------------------------
void SAL_CALL OImageControlControl::createPeer(const staruno::Reference<starawt::XToolkit>& _rxToolkit, const staruno::Reference<starawt::XWindowPeer>& Parent) throw( staruno::RuntimeException )
{
    OBoundControl::createPeer(_rxToolkit, Parent);
    if (!m_xControl.is())
        return;

    // ImageConsumer vom Control holen
    staruno::Reference<starawt::XWindowPeer>  xPeer = m_xControl->getPeer();
    staruno::Reference<starawt::XImageConsumer>  xImageConsumer(xPeer, staruno::UNO_QUERY);
    if (!xImageConsumer.is())
        return;

    // ImageConsumer am Imageproducer setzen
    staruno::Reference<starform::XImageProducerSupplier>  xImageSource(m_xControl->getModel(), staruno::UNO_QUERY);
    if (!xImageSource.is())
        return;
    staruno::Reference<starawt::XImageProducer>  xImageProducer = xImageSource->getImageProducer();

    xImageProducer->addConsumer(xImageConsumer);
    xImageProducer->startProduction();
}

// MouseListener
//------------------------------------------------------------------------------
void OImageControlControl::mousePressed(const starawt::MouseEvent& e)
{
    //////////////////////////////////////////////////////////////////////
    // Nur linke Maustaste
    if (e.Buttons != starawt::MouseButton::LEFT)
        return;

    //////////////////////////////////////////////////////////////////////
    // Doppelclick
    if (e.ClickCount == 2)
    {

        staruno::Reference<starbeans::XPropertySet>  xSet(getModel(), staruno::UNO_QUERY);
        if (!xSet.is())
            return;

        // wenn Control nicht gebunden ist, kein Dialog (da die zu schickende URL hinterher sowieso
        // versanden wuerde)
        // FS - #64946# - 19.04.99
        staruno::Reference<starbeans::XPropertySet> xBoundField;
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
        staruno::Sequence<staruno::Any> aParams(4);
        aParams.getArray()[0] = staruno::makeAny(starbeans::PropertyValue(
            ::rtl::OUString::createFromAscii("Title"), -1,
            staruno::makeAny(sTitle), starbeans::PropertyState_DIRECT_VALUE
        ));
        aParams.getArray()[1] = staruno::makeAny(starbeans::PropertyValue(
            ::rtl::OUString::createFromAscii("PreviewActive"), -1,
            staruno::makeAny((sal_Bool)sal_True), starbeans::PropertyState_DIRECT_VALUE
        ));
        aParams.getArray()[2] = staruno::makeAny(starbeans::PropertyValue(
            ::rtl::OUString::createFromAscii("AllowEmptyFileNames"), -1,
            staruno::makeAny((sal_Bool)sal_True), starbeans::PropertyState_DIRECT_VALUE
        ));
        staruno::Reference<starawt::XWindow> xWindow(getPeer(), staruno::UNO_QUERY);
        aParams.getArray()[3] = staruno::makeAny(starbeans::PropertyValue(
            ::rtl::OUString::createFromAscii("ParentWindow"), -1,
            staruno::makeAny(xWindow), starbeans::PropertyState_DIRECT_VALUE
        ));

        try
        {
            staruno::Reference<starawt::XDialog> xDialog(
                m_xServiceFactory->createInstanceWithArguments(
                    ::rtl::OUString::createFromAscii("com.sun.star.dialogs.ImportGraphicsDialog"),
                    aParams),
                staruno::UNO_QUERY
            );

            staruno::Reference<starbeans::XPropertySet> xDialogProps(xDialog, staruno::UNO_QUERY);
            if (xDialog.is() && xDialogProps.is() && xDialog->execute())
            {
                staruno::Any aSelectedPath = xDialogProps->getPropertyValue(::rtl::OUString::createFromAscii("SelectedPath"));
                if (aSelectedPath.getValueType().getTypeClass() == staruno::TypeClass_STRING)
                {
                    xSet->setPropertyValue(PROPERTY_IMAGE_URL, ::com::sun::star::uno::makeAny(::rtl::OUString()));
                        // reset the url property in case it already has the value we're about to set - in this case
                        // our propertyChanged would not get called without this.
                    xSet->setPropertyValue(PROPERTY_IMAGE_URL, aSelectedPath);
                }
            }
        }
        catch(...)
        {
        }
    }
}

//.........................................................................
}   // namespace frm
//.........................................................................

