/*************************************************************************
 *
 *  $RCSfile: FormComponent.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2000-10-24 13:02:32 $
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

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XMARKABLESTREAM_HPP_
#include <com/sun/star/io/XMarkableStream.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif

using namespace dbtools;

//... namespace frm .......................................................
namespace frm
{
//.........................................................................

//=========================================================================
//= base class for form layer controls
//=========================================================================
DBG_NAME(frm_OControl)
//------------------------------------------------------------------------------
OControl::OControl(const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& _rxFactory, const rtl::OUString& _sService)
            :OComponentHelper(m_aMutex)
            ,m_aService(_sService)
            ,m_xServiceFactory(_rxFactory)
{
    DBG_CTOR(frm_OControl, NULL);
    // VCL-Control aggregieren
    // bei Aggregation den Refcount um eins erhoehen da im setDelegator
    // das Aggregat selbst den Refcount erhoeht
    increment(m_refCount);
    {
                m_xAggregate = com::sun::star::uno::Reference<com::sun::star::uno::XAggregation>(
                        _rxFactory->createInstance(_sService), com::sun::star::uno::UNO_QUERY);
                m_xControl = com::sun::star::uno::Reference<starawt::XControl>(m_xAggregate, com::sun::star::uno::UNO_QUERY);
    }

    if (m_xAggregate.is())
    {
                m_xAggregate->setDelegator(static_cast<com::sun::star::uno::XWeak*>(this));
    }

    // Refcount wieder bei NULL
    sal_Int32 n = decrement(m_refCount);
}

//------------------------------------------------------------------------------
OControl::~OControl()
{
    DBG_DTOR(frm_OControl, NULL);
    // Freigeben der Aggregation
    if (m_xAggregate.is())
    {
        m_xAggregate->setDelegator(InterfaceRef());
    }
}

// UNO Anbindung
//------------------------------------------------------------------------------
com::sun::star::uno::Any SAL_CALL OControl::queryAggregation( const com::sun::star::uno::Type& _rType ) throw(com::sun::star::uno::RuntimeException)
{
        com::sun::star::uno::Any aReturn;

    // ask the base class
    aReturn = OComponentHelper::queryAggregation(_rType);
    // ask our own interfaces
    if (!aReturn.hasValue())
        aReturn = cppu::queryInterface(_rType
            ,static_cast<starawt::XControl*>(this)
                        ,static_cast<com::sun::star::lang::XEventListener*>(this)
                        ,static_cast<com::sun::star::lang::XServiceInfo*>(this)
        );

    // ask our aggregate
    if (!aReturn.hasValue() && m_xAggregate.is())
        aReturn = m_xAggregate->queryAggregation(_rType);

    return aReturn;
}

//------------------------------------------------------------------------------
com::sun::star::uno::Sequence<sal_Int8> SAL_CALL OControl::getImplementationId() throw(com::sun::star::uno::RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

//------------------------------------------------------------------------------
com::sun::star::uno::Sequence<com::sun::star::uno::Type> SAL_CALL OControl::getTypes() throw(com::sun::star::uno::RuntimeException)
{
        com::sun::star::uno::Sequence<com::sun::star::uno::Type> aOwnTypes = _getTypes();
        com::sun::star::uno::Reference<com::sun::star::lang::XTypeProvider> xProv;

    if (query_aggregation(m_xAggregate, xProv))
        return concatSequences(aOwnTypes, xProv->getTypes());
    else
        return aOwnTypes;
}

//------------------------------------------------------------------------------
com::sun::star::uno::Sequence<com::sun::star::uno::Type> OControl::_getTypes()
{
        static com::sun::star::uno::Sequence<com::sun::star::uno::Type> aTypes;
    if (!aTypes.getLength())
    {
        // my base class types
                com::sun::star::uno::Sequence<com::sun::star::uno::Type> aBaseTypes = OComponentHelper::getTypes();

                com::sun::star::uno::Sequence<com::sun::star::uno::Type> aOwnTypes(3);
                com::sun::star::uno::Type* pOwnTypes = aOwnTypes.getArray();
                pOwnTypes[0] = getCppuType((com::sun::star::uno::Reference<starawt::XControl>*)NULL);
                pOwnTypes[1] = getCppuType((com::sun::star::uno::Reference<com::sun::star::lang::XEventListener>*)NULL);
                pOwnTypes[2] = getCppuType((com::sun::star::uno::Reference<com::sun::star::lang::XServiceInfo>*)NULL);

        aTypes = concatSequences(aTypes, aOwnTypes);
    }
    return aTypes;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OControl::disposing()
{
    OComponentHelper::disposing();

        com::sun::star::uno::Reference<com::sun::star::lang::XComponent> xComp;
    if (query_aggregation(m_xAggregate, xComp))
        xComp->dispose();
}

// XServiceInfo
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OControl::supportsService(const rtl::OUString& _rsServiceName)
{
        com::sun::star::uno::Sequence<rtl::OUString> aSupported = getSupportedServiceNames();
    const rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rsServiceName))
            return sal_True;
    return sal_False;
}

//------------------------------------------------------------------------------
com::sun::star::uno::Sequence<rtl::OUString> SAL_CALL OControl::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
        com::sun::star::uno::Sequence<rtl::OUString> aSupported;

    // ask our aggregate
        com::sun::star::uno::Reference<com::sun::star::lang::XServiceInfo> xInfo;
    if (query_aggregation(m_xAggregate, xInfo))
        aSupported = xInfo->getSupportedServiceNames();

    return aSupported;
}

// XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OControl::disposing(const com::sun::star::lang::EventObject& _rEvent) throw (com::sun::star::uno::RuntimeException)
{
    InterfaceRef xAggAsIface;
    query_aggregation(m_xAggregate, xAggAsIface);

    // does the disposing come from the aggregate ?
        if (xAggAsIface != InterfaceRef(_rEvent.Source, com::sun::star::uno::UNO_QUERY))
    {   // no -> forward it
                com::sun::star::uno::Reference<com::sun::star::lang::XEventListener> xListener;
        if (query_aggregation(m_xAggregate, xListener))
            xListener->disposing(_rEvent);
    }
}

// XControl
//------------------------------------------------------------------------------
void SAL_CALL OControl::setContext(const InterfaceRef& Context) throw (com::sun::star::uno::RuntimeException)
{
    if (m_xControl.is())
        m_xControl->setContext(Context);
}

//------------------------------------------------------------------------------
InterfaceRef SAL_CALL OControl::getContext() throw (com::sun::star::uno::RuntimeException)
{
    return m_xControl.is() ? m_xControl->getContext() : InterfaceRef();
}

//------------------------------------------------------------------------------
void SAL_CALL OControl::createPeer(const com::sun::star::uno::Reference<starawt::XToolkit>& Toolkit, const com::sun::star::uno::Reference<starawt::XWindowPeer>& Parent) throw (com::sun::star::uno::RuntimeException)
{
    if (m_xControl.is())
        m_xControl->createPeer(Toolkit, Parent);
}

//------------------------------------------------------------------------------
com::sun::star::uno::Reference<starawt::XWindowPeer> SAL_CALL OControl::getPeer()
{
        return m_xControl.is() ? m_xControl->getPeer() : com::sun::star::uno::Reference<starawt::XWindowPeer>();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OControl::setModel(const com::sun::star::uno::Reference<starawt::XControlModel>& Model)
{
    return m_xControl.is() ? m_xControl->setModel( Model ) : sal_False;
}

//------------------------------------------------------------------------------
com::sun::star::uno::Reference<starawt::XControlModel> SAL_CALL OControl::getModel()
{
        return m_xControl.is() ? m_xControl->getModel() : com::sun::star::uno::Reference<starawt::XControlModel>();
}

//------------------------------------------------------------------------------
com::sun::star::uno::Reference<starawt::XView> SAL_CALL OControl::getView()
{
        return m_xControl.is() ? m_xControl->getView() : com::sun::star::uno::Reference<starawt::XView>();
}

//------------------------------------------------------------------------------
void SAL_CALL OControl::setDesignMode(sal_Bool bOn)
{
    if (m_xControl.is())
        m_xControl->setDesignMode(bOn);
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OControl::isDesignMode()
{
    return m_xControl.is() ? m_xControl->isDesignMode() : sal_True;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OControl::isTransparent()
{
    return m_xControl.is() ? m_xControl->isTransparent() : sal_True;
}

//==================================================================
//= OBoundControl
//==================================================================
DBG_NAME(frm_OBoundControl);
//------------------------------------------------------------------
OBoundControl::OBoundControl(const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& _rxFactory, const ::rtl::OUString& _sService)
        :OControl(_rxFactory, _sService)
        ,m_bLocked(sal_False)
{
    DBG_CTOR(frm_OBoundControl, NULL);
}

//------------------------------------------------------------------
OBoundControl::~OBoundControl()
{
    DBG_DTOR(frm_OBoundControl, NULL);
}

//------------------------------------------------------------------
com::sun::star::uno::Any SAL_CALL OBoundControl::queryAggregation(const com::sun::star::uno::Type& _rType) throw(com::sun::star::uno::RuntimeException)
{
        com::sun::star::uno::Any aReturn;

    // ask the base class
    aReturn = OControl::queryAggregation(_rType);
    // ask our own interfaces
    if (!aReturn.hasValue())
        aReturn = cppu::queryInterface(_rType, static_cast<starform::XBoundControl*>(this));

    return aReturn;
}

//------------------------------------------------------------------
sal_Bool SAL_CALL OBoundControl::getLock() throw(com::sun::star::uno::RuntimeException)
{
    return m_bLocked;
}

//------------------------------------------------------------------
void SAL_CALL OBoundControl::setLock(sal_Bool _bLock) throw(com::sun::star::uno::RuntimeException)
{
    if (m_bLocked == _bLock)
        return;

    osl::MutexGuard aGuard(m_aMutex);
    _setLock(_bLock);
    m_bLocked = _bLock;
}

//------------------------------------------------------------------
void OBoundControl::_setLock(sal_Bool _bLock)
{
    // try to set the text component to readonly
        com::sun::star::uno::Reference<starawt::XWindowPeer> xPeer = getPeer();
        com::sun::star::uno::Reference<com::sun::star::awt::XTextComponent> xText(xPeer, com::sun::star::uno::UNO_QUERY);

    if (xText.is())
        xText->setEditable(!_bLock);
    else
    {
        // disable the window
                com::sun::star::uno::Reference<starawt::XWindow> xComp(xPeer, com::sun::star::uno::UNO_QUERY);
        if (xComp.is())
            xComp->setEnable(!_bLock);
    }
}

//==================================================================
//= OControlModel
//==================================================================
ConcretInfoService OControlModel::s_aPropInfos;
DBG_NAME(OControlModel)
//------------------------------------------------------------------
com::sun::star::uno::Sequence<sal_Int8> SAL_CALL OControlModel::getImplementationId() throw(com::sun::star::uno::RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

//------------------------------------------------------------------
com::sun::star::uno::Sequence<com::sun::star::uno::Type> SAL_CALL OControlModel::getTypes() throw(com::sun::star::uno::RuntimeException)
{
        com::sun::star::uno::Sequence<com::sun::star::uno::Type> aOwnTypes = _getTypes();
        com::sun::star::uno::Reference<com::sun::star::lang::XTypeProvider> xProv;

    if (query_aggregation(m_xAggregate, xProv))
        return concatSequences(aOwnTypes, xProv->getTypes());
    else
        return aOwnTypes;
}

//------------------------------------------------------------------------------
com::sun::star::uno::Sequence<com::sun::star::uno::Type> OControlModel::_getTypes()
{
        static com::sun::star::uno::Sequence<com::sun::star::uno::Type> aTypes;
    if (!aTypes.getLength())
    {
        // my two base classes
                com::sun::star::uno::Sequence<com::sun::star::uno::Type> aComponentTypes = OComponentHelper::getTypes();
                com::sun::star::uno::Sequence<com::sun::star::uno::Type> aPropertyTypes = OPropertySetAggregationHelper::getTypes();

                com::sun::star::uno::Sequence<com::sun::star::uno::Type> aOwnTypes(4);
                com::sun::star::uno::Type* pOwnTypes = aOwnTypes.getArray();
                pOwnTypes[0] = getCppuType((com::sun::star::uno::Reference<starform::XFormComponent>*)NULL);
                pOwnTypes[1] = getCppuType((com::sun::star::uno::Reference<stario::XPersistObject>*)NULL);
                pOwnTypes[1] = getCppuType((com::sun::star::uno::Reference<starcontainer::XNamed>*)NULL);
                pOwnTypes[3] = getCppuType((com::sun::star::uno::Reference<com::sun::star::lang::XServiceInfo>*)NULL);

        aTypes = concatSequences(aComponentTypes, aPropertyTypes, aOwnTypes);
    }
    return aTypes;
}

//------------------------------------------------------------------
com::sun::star::uno::Any SAL_CALL OControlModel::queryAggregation(const com::sun::star::uno::Type& _rType) throw (com::sun::star::uno::RuntimeException)
{
        com::sun::star::uno::Any aReturn;

    // base class 1
    aReturn = OComponentHelper::queryAggregation(_rType);

    // base class 2
    if (!aReturn.hasValue())
        aReturn = OPropertySetAggregationHelper::queryInterface(_rType);

    // our own interfaces
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<starform::XFormComponent*>(this)
            ,static_cast<starcontainer::XChild*>(this)
            ,static_cast<stario::XPersistObject*>(this)
            ,static_cast<starcontainer::XNamed*>(this)
                        ,static_cast<com::sun::star::lang::XServiceInfo*>(this)
        );

    // our aggregate
    if (!aReturn.hasValue() && m_xAggregate.is())
        aReturn = m_xAggregate->queryAggregation(_rType);
    return aReturn;
}

//------------------------------------------------------------------
OControlModel::OControlModel(
                        const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
            const ::rtl::OUString& _rUnoControlModelTypeName,
            const ::rtl::OUString& rDefault)
    :OComponentHelper(m_aMutex)
    ,OPropertySetAggregationHelper(OComponentHelper::rBHelper)
    ,m_aUnoControlModelTypeName(_rUnoControlModelTypeName)
    ,m_nTabIndex(FRM_DEFAULT_TABINDEX)
    ,m_nClassId(starform::FormComponentType::CONTROL)
    ,m_xServiceFactory(_rxFactory)
{
    DBG_CTOR(OControlModel, NULL);
    if (_rUnoControlModelTypeName.getLength())  // the is a model we have to aggregate
    {
        increment(m_refCount);

        // Muss im eigenen Block,
        // da xAgg vor dem delegator setzen wieder freigesetzt sein muﬂ!
        {
                        m_xAggregate = com::sun::star::uno::Reference<com::sun::star::uno::XAggregation>(_rxFactory->createInstance(_rUnoControlModelTypeName), com::sun::star::uno::UNO_QUERY);
            setAggregation(m_xAggregate);

            if (m_xAggregateSet.is() && rDefault.len())
                                m_xAggregateSet->setPropertyValue(PROPERTY_DEFAULTCONTROL, com::sun::star::uno::makeAny(rDefault));
        }

        if (m_xAggregate.is())
        {
                        m_xAggregate->setDelegator(static_cast<com::sun::star::uno::XWeak*>(this));
        }

        // Refcount wieder bei NULL
        sal_Int32 n = decrement(m_refCount);
    }
}

//------------------------------------------------------------------
OControlModel::~OControlModel()
{
    DBG_DTOR(OControlModel, NULL);
    // release the aggregate
    if (m_xAggregate.is())
        m_xAggregate->setDelegator(InterfaceRef());
}

// XChild
//------------------------------------------------------------------------------
void SAL_CALL OControlModel::setParent(const InterfaceRef& _rxParent) throw(com::sun::star::lang::NoSupportException, com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

        com::sun::star::uno::Reference<com::sun::star::lang::XComponent> xComp(m_xParent, com::sun::star::uno::UNO_QUERY);
    if (xComp.is())
                xComp->removeEventListener(static_cast<com::sun::star::beans::XPropertiesChangeListener*>(this));

        xComp = com::sun::star::uno::Reference<com::sun::star::lang::XComponent>(_rxParent, com::sun::star::uno::UNO_QUERY);
    if (xComp.is())
                xComp->addEventListener(static_cast<com::sun::star::beans::XPropertiesChangeListener*>(this));
    m_xParent = _rxParent;
}

// XNamed
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OControlModel::getName() throw(com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aReturn;
    OPropertySetHelper::getFastPropertyValue(PROPERTY_ID_NAME) >>= aReturn;
    return aReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL OControlModel::setName(const ::rtl::OUString& _rName) throw(com::sun::star::uno::RuntimeException)
{
        setFastPropertyValue(PROPERTY_ID_NAME, com::sun::star::uno::makeAny(_rName));
}

// XServiceInfo
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OControlModel::supportsService(const rtl::OUString& _rServiceName)
{
        com::sun::star::uno::Sequence<rtl::OUString> aSupported = getSupportedServiceNames();
    const rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rServiceName))
            return sal_True;
    return sal_False;
}

//------------------------------------------------------------------------------
com::sun::star::uno::Sequence<rtl::OUString> SAL_CALL OControlModel::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
        com::sun::star::uno::Sequence<rtl::OUString> aSupported;

    // ask our aggregate
        com::sun::star::uno::Reference<com::sun::star::lang::XServiceInfo> xInfo;
    if (query_aggregation(m_xAggregate, xInfo))
        aSupported = xInfo->getSupportedServiceNames();

    aSupported.realloc(aSupported.getLength() + 2);
    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString::createFromAscii("com.sun.star.form.FormComponent");
    pArray[aSupported.getLength()-1] = ::rtl::OUString::createFromAscii("com.sun.star.form.FormControlModel");

    return aSupported;
}

// XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OControlModel::disposing(const com::sun::star::lang::EventObject& _rSource) throw (com::sun::star::uno::RuntimeException)
{
    // release the parent
    if (_rSource.Source == m_xParent)
    {
        osl::MutexGuard aGuard(m_aMutex);
        m_xParent = NULL;
    }
    else
    {
                com::sun::star::uno::Reference<com::sun::star::lang::XEventListener> xEvtLst;
        if (query_aggregation(m_xAggregate, xEvtLst))
        {
            osl::MutexGuard aGuard(m_aMutex);
            xEvtLst->disposing(_rSource);
        }
    }
}

// OComponentHelper
//-----------------------------------------------------------------------------
void OControlModel::disposing()
{
    OPropertySetAggregationHelper::disposing();

        com::sun::star::uno::Reference<com::sun::star::lang::XComponent> xComp;
    if (query_aggregation(m_xAggregate, xComp))
        xComp->dispose();

        setParent(com::sun::star::uno::Reference<starform::XFormComponent>());
}

//------------------------------------------------------------------------------
void SAL_CALL OControlModel::write(const com::sun::star::uno::Reference<stario::XObjectOutputStream>& _rxOutStream)
                        throw(stario::IOException, com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

    // 1. Schreiben des UnoControls
        com::sun::star::uno::Reference<stario::XMarkableStream> xMark(_rxOutStream, com::sun::star::uno::UNO_QUERY);
    sal_Int32 nMark = xMark->createMark();
    sal_Int32 nLen = 0;

    _rxOutStream->writeLong(nLen);

        com::sun::star::uno::Reference<stario::XPersistObject> xPersist;
    if (query_aggregation(m_xAggregate, xPersist))
        xPersist->write(_rxOutStream);

    // feststellen der Laenge
    nLen = xMark->offsetToMark(nMark) - 4;
    xMark->jumpToMark(nMark);
    _rxOutStream->writeLong(nLen);
    xMark->jumpToFurthest();
    xMark->deleteMark(nMark);

    // 2. Schreiben einer VersionsNummer
    _rxOutStream->writeShort(0x0003);

    // 3. Schreiben der allgemeinen Properties
    _rxOutStream << m_aName;
    _rxOutStream->writeShort(m_nTabIndex);
    _rxOutStream << m_aTag; // 3. Version

    // !!! IMPORTANT NOTE !!!
    // don't write any new members here : this wouldn't be compatible with older versions, as OControlModel
    // is a base class which is called in derived classes "read" method. So if you increment the version
    // and write new stuff, older office versions will read this in the _derived_ classes, which may result
    // in anything from data loss to crash.
    // !!! EOIN !!!
}

//------------------------------------------------------------------------------
void OControlModel::read(const com::sun::star::uno::Reference<stario::XObjectInputStream>& InStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

    // 1. Lesen des UnoControls
    sal_Int32 nLen = InStream->readLong();
    if (nLen)
    {
                com::sun::star::uno::Reference<stario::XMarkableStream> xMark(InStream, com::sun::star::uno::UNO_QUERY);
        sal_Int32 nMark = xMark->createMark();

                com::sun::star::uno::Reference<stario::XPersistObject> xPersist;
        if (query_aggregation(m_xAggregate, xPersist))
            xPersist->read(InStream);

        xMark->jumpToMark(nMark);
        InStream->skipBytes(nLen);
        xMark->deleteMark(nMark);
    }

    // 2. Lesen des Versionsnummer
    UINT16 nVersion = InStream->readShort();

    // 3. Lesen der allgemeinen Properties
    InStream >> m_aName;
    m_nTabIndex  = InStream->readShort();

    if (nVersion > 0x0002)
        InStream >> m_aTag;

    // we had a version where we wrote the help text
    if (nVersion == 0x0004)
        InStream >> m_aHelpText;

    DBG_ASSERT(nVersion < 5, "OControlModel::read : suspicious version number !");
    // 4 was the version where we wrote the help text
    // later versions shouldn't exist (see write for a detailed comment)
}

//------------------------------------------------------------------------------
void OControlModel::getFastPropertyValue( com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            rValue <<= m_aName;
            break;
        case PROPERTY_ID_TAG:
            rValue <<= m_aTag;
            break;
        case PROPERTY_ID_CLASSID:
            rValue <<= m_nClassId;
            break;
        case PROPERTY_ID_TABINDEX:
            rValue <<= m_nTabIndex;
            break;
        case PROPERTY_ID_HELPTEXT:
            rValue <<= m_aHelpText;
            break;
        default:
            OPropertySetAggregationHelper::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
sal_Bool OControlModel::convertFastPropertyValue(
                        com::sun::star::uno::Any& _rConvertedValue, com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const com::sun::star::uno::Any& _rValue)
                        throw (com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (_nHandle)
    {
        case PROPERTY_ID_NAME:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aName);
            break;
        case PROPERTY_ID_TAG:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aTag);
            break;
        case PROPERTY_ID_HELPTEXT:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aHelpText);
            break;
        case PROPERTY_ID_TABINDEX:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_nTabIndex);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OControlModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const com::sun::star::uno::Any& _rValue)
                        throw (com::sun::star::uno::Exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_NAME:
            DBG_ASSERT(_rValue.getValueType() == getCppuType((const ::rtl::OUString*)NULL),
                "OControlModel::setFastPropertyValue_NoBroadcast : invalid type" );
            _rValue >>= m_aName;
            break;
        case PROPERTY_ID_TAG:
            DBG_ASSERT(_rValue.getValueType() == getCppuType((const ::rtl::OUString*)NULL),
                "OControlModel::setFastPropertyValue_NoBroadcast : invalid type" );
            _rValue >>= m_aTag;
            break;
        case PROPERTY_ID_TABINDEX:
            DBG_ASSERT(_rValue.getValueType() == getCppuType((const sal_Int16*)NULL),
                "OControlModel::setFastPropertyValue_NoBroadcast : invalid type" );
            _rValue >>= m_nTabIndex;
            break;
        case PROPERTY_ID_HELPTEXT:
            DBG_ASSERT(_rValue.getValueType() == getCppuType((const ::rtl::OUString*)NULL),
                "OControlModel::setFastPropertyValue_NoBroadcast : invalid type" );
            _rValue >>= m_aHelpText;
            break;
    }
}

//==================================================================
//= OBoundControlModel
//==================================================================
DBG_NAME(frm_OBoundControlModel);
//------------------------------------------------------------------
com::sun::star::uno::Any SAL_CALL OBoundControlModel::queryAggregation( const com::sun::star::uno::Type& _rType ) throw (com::sun::star::uno::RuntimeException)
{
        com::sun::star::uno::Any aReturn;

    aReturn = OControlModel::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = cppu::queryInterface(_rType
                        ,static_cast<com::sun::star::beans::XPropertyChangeListener*>(this)
            ,static_cast<starform::XReset*>(this)
            ,static_cast<starform::XLoadListener*>(this)
                        ,static_cast<com::sun::star::lang::XEventListener*>(static_cast<starform::XLoadListener*>(this))
        );

    if (!aReturn.hasValue() && m_bCommitable)
        aReturn = cppu::queryInterface(_rType
            ,static_cast<starform::XBoundComponent*>(this)
            ,static_cast<starform::XUpdateBroadcaster*>(this)
        );

    return aReturn;
}

//------------------------------------------------------------------
OBoundControlModel::OBoundControlModel(
                                const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
                const ::rtl::OUString& _rUnoControlModelTypeName,
                const ::rtl::OUString& _rDefault,
                sal_Bool _bCommitable)
      :OControlModel(_rxFactory, _rUnoControlModelTypeName, _rDefault)
      ,m_aUpdateListeners(m_aMutex)
      ,m_aResetListeners(m_aMutex)
      ,m_bLoaded(sal_False)
      ,m_bRequired(sal_False)
      ,m_bCommitable(_bCommitable)
      ,m_aLabelServiceName(FRM_SUN_COMPONENT_FIXEDTEXT)
      ,m_bResetting(sal_False)
      ,m_bForwardValueChanges(sal_True)
{
    DBG_CTOR(frm_OBoundControlModel, NULL);
}

//------------------------------------------------------------------
OBoundControlModel::~OBoundControlModel()
{
    DBG_DTOR(frm_OBoundControlModel, NULL);
}

//-----------------------------------------------------------------------------
com::sun::star::uno::Sequence<com::sun::star::uno::Type> OBoundControlModel::_getTypes()
{
        com::sun::star::uno::Sequence<com::sun::star::uno::Type> aReturn = OControlModel::_getTypes();

    sal_Int32 nBaseLen = aReturn.getLength();
    aReturn.realloc(nBaseLen + (m_bCommitable ? 4 : 3));
        com::sun::star::uno::Type* pReturn = aReturn.getArray() + nBaseLen;
        pReturn[0] = ::getCppuType((com::sun::star::uno::Reference<starform::XLoadListener>*)NULL);
        pReturn[1] = ::getCppuType((com::sun::star::uno::Reference<starform::XReset>*)NULL);
        pReturn[2] = ::getCppuType((com::sun::star::uno::Reference<com::sun::star::beans::XPropertyChangeListener>*)NULL);
    if (m_bCommitable)
                pReturn[3] = ::getCppuType((com::sun::star::uno::Reference<starform::XBoundComponent>*)NULL);

    return aReturn;
}

// OComponentHelper
//-----------------------------------------------------------------------------
void OBoundControlModel::disposing()
{
    OControlModel::disposing();

    osl::MutexGuard aGuard(m_aMutex);
        com::sun::star::lang::EventObject aEvt(static_cast<com::sun::star::uno::XWeak*>(this));
    m_aResetListeners.disposeAndClear(aEvt);
    m_aUpdateListeners.disposeAndClear(aEvt);

    if (m_xField.is())
    {
        m_xField->removePropertyChangeListener(PROPERTY_VALUE, this);
        resetField();
    }
    m_xCursor = NULL;

        com::sun::star::uno::Reference<com::sun::star::lang::XComponent> xComp(m_xLabelControl, com::sun::star::uno::UNO_QUERY);
    if (xComp.is())
                xComp->removeEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<com::sun::star::beans::XPropertyChangeListener*>(this)));
}

// XChild
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::setParent(const com::sun::star::uno::Reference<com::sun::star::uno::XInterface>& _rxParent) throw(com::sun::star::lang::NoSupportException, com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

    // log off old listeners
        com::sun::star::uno::Reference<starform::XLoadable> xLoadable(m_xParent, com::sun::star::uno::UNO_QUERY);
    if (xLoadable.is())
        xLoadable->removeLoadListener(this);

    // log on new listeners
        xLoadable = com::sun::star::uno::Reference<starform::XLoadable>(_rxParent, com::sun::star::uno::UNO_QUERY);
    if (xLoadable.is())
        xLoadable->addLoadListener(this);

    OControlModel::setParent(_rxParent);
}

// XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::disposing(const com::sun::star::lang::EventObject& _rEvent) throw (com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    if (m_xField == _rEvent.Source)
    {
        resetField();
    }
    else if (m_xLabelControl == _rEvent.Source)
    {
                com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xOldValue = m_xLabelControl;
        m_xLabelControl = NULL;

        // fire a property change event
                com::sun::star::uno::Any aOldValue; aOldValue <<= xOldValue;
                com::sun::star::uno::Any aNewValue; aNewValue <<= m_xLabelControl;
        sal_Int32 nHandle = PROPERTY_ID_CONTROLLABEL;
        OPropertySetHelper::fire(&nHandle, &aNewValue, &aOldValue, 1, sal_False);
    }
    else
        OControlModel::disposing(_rEvent);
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OBoundControlModel::getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException)
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = ::rtl::OUString::createFromAscii("com.sun.star.form.DataAwareControlModel");
    return aSupported;
}


// XPersist
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::write( const com::sun::star::uno::Reference<stario::XObjectOutputStream>& _rxOutStream ) throw(stario::IOException, com::sun::star::uno::RuntimeException)
{
    OControlModel::write(_rxOutStream);

    osl::MutexGuard aGuard(m_aMutex);

    // Version
    _rxOutStream->writeShort(0x0002);

    // Controlsource
    _rxOutStream << m_aControlSource;

    // !!! IMPORTANT NOTE !!!
    // don't write any new members here : this wouldn't be compatible with older versions, as OBoundControlModel
    // is a base class which is called in derived classes "read" method. So if you increment the version
    // and write new stuff, older office versions will read this in the _derived_ classes, which may result
    // in anything from data loss to crash.
    // (use writeCommonProperties instead, this is called in derived classes write-method)
    // !!! EOIN !!!
    // FS - 68876 - 28.09.1999
}

//------------------------------------------------------------------------------
void OBoundControlModel::defaultCommonProperties()
{
        com::sun::star::uno::Reference<com::sun::star::lang::XComponent> xComp(m_xLabelControl, com::sun::star::uno::UNO_QUERY);
    if (xComp.is())
                xComp->removeEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<com::sun::star::beans::XPropertyChangeListener*>(this)));
    m_xLabelControl = NULL;
}

//------------------------------------------------------------------------------
void OBoundControlModel::readCommonProperties(const com::sun::star::uno::Reference<stario::XObjectInputStream>& _rxInStream)
{
    sal_Int32 nLen = _rxInStream->readLong();

        com::sun::star::uno::Reference<stario::XMarkableStream> xMark(_rxInStream, com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xMark.is(), "OBoundControlModel::readCommonProperties : can only work with markable streams !");
    sal_Int32 nMark = xMark->createMark();

    // read the reference to the label control
        com::sun::star::uno::Reference<stario::XPersistObject> xPersist;
    sal_Int32 nUsedFlag;
    nUsedFlag = _rxInStream->readLong();
    if (nUsedFlag)
        xPersist = _rxInStream->readObject();
        m_xLabelControl = com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>(xPersist, com::sun::star::uno::UNO_QUERY);
        com::sun::star::uno::Reference<com::sun::star::lang::XComponent> xComp(m_xLabelControl, com::sun::star::uno::UNO_QUERY);
    if (xComp.is())
                xComp->addEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<com::sun::star::beans::XPropertyChangeListener*>(this)));

    // read any other new common properties here

    // skip the remaining bytes
    xMark->jumpToMark(nMark);
    _rxInStream->skipBytes(nLen);
    xMark->deleteMark(nMark);
}

//------------------------------------------------------------------------------
void OBoundControlModel::writeCommonProperties(const com::sun::star::uno::Reference<stario::XObjectOutputStream>& _rxOutStream)
{
        com::sun::star::uno::Reference<stario::XMarkableStream> xMark(_rxOutStream, com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xMark.is(), "OBoundControlModel::writeCommonProperties : can only work with markable streams !");
    sal_Int32 nMark = xMark->createMark();

    // a placeholder where we will write the overall length (later in this method)
    sal_Int32 nLen = 0;
    _rxOutStream->writeLong(nLen);

    // write the reference to the label control
        com::sun::star::uno::Reference<stario::XPersistObject> xPersist(m_xLabelControl, com::sun::star::uno::UNO_QUERY);
    sal_Int32 nUsedFlag = 0;
    if (xPersist.is())
        nUsedFlag = 1;
    _rxOutStream->writeLong(nUsedFlag);
    if (xPersist.is())
        _rxOutStream->writeObject(xPersist);

    // write any other new common properties here

    // write the correct length at the beginning of the block
    nLen = xMark->offsetToMark(nMark) - sizeof(nLen);
    xMark->jumpToMark(nMark);
    _rxOutStream->writeLong(nLen);
    xMark->jumpToFurthest();
    xMark->deleteMark(nMark);
}

//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::read( const com::sun::star::uno::Reference< stario::XObjectInputStream >& _rxInStream ) throw(stario::IOException, com::sun::star::uno::RuntimeException)
{
    OControlModel::read(_rxInStream);

    osl::MutexGuard aGuard(m_aMutex);
    UINT16 nVersion = _rxInStream->readShort();
    _rxInStream >> m_aControlSource;
}

//------------------------------------------------------------------------------
void OBoundControlModel::getFastPropertyValue(com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_CONTROLSOURCEPROPERTY:
            rValue <<= m_sDataFieldConnectivityProperty;
            break;
        case PROPERTY_ID_CONTROLSOURCE:
            rValue <<= m_aControlSource;
            break;
        case PROPERTY_ID_BOUNDFIELD:
            rValue <<= m_xField;
            break;
        case PROPERTY_ID_CONTROLLABEL:
            if (!m_xLabelControl.is())
                rValue.clear();
            else
                rValue <<= m_xLabelControl;
            break;
        default:
            OControlModel::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
sal_Bool OBoundControlModel::convertFastPropertyValue(
                                com::sun::star::uno::Any& _rConvertedValue, com::sun::star::uno::Any& _rOldValue,
                sal_Int32 _nHandle,
                                const com::sun::star::uno::Any& _rValue)
                throw (com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (_nHandle)
    {
        case PROPERTY_ID_CONTROLSOURCE:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aControlSource);
            break;
        case PROPERTY_ID_BOUNDFIELD:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_xField);
            break;
        case PROPERTY_ID_CONTROLLABEL:
            if (!_rValue.hasValue())
            {   // property set to void
                                _rConvertedValue = com::sun::star::uno::Any();
                getFastPropertyValue(_rOldValue, _nHandle);
                bModified = m_xLabelControl.is();
            }
            else
                bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_xLabelControl);
            break;
        default:
            bModified = OControlModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OBoundControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const com::sun::star::uno::Any& rValue ) throw (com::sun::star::uno::Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_CONTROLSOURCE:
                        DBG_ASSERT(rValue.getValueType().getTypeClass() == com::sun::star::uno::TypeClass_STRING, "invalid type" );
            rValue >>= m_aControlSource;
            break;
        case PROPERTY_ID_BOUNDFIELD:
            DBG_ERROR("OBoundControlModel::setFastPropertyValue_NoBroadcast : BoundField should be a read-only property !");
                        throw com::sun::star::lang::IllegalArgumentException();
            break;
        case PROPERTY_ID_CONTROLLABEL:
        {
                        DBG_ASSERT(!rValue.hasValue() || (rValue.getValueType().getTypeClass() == com::sun::star::uno::TypeClass_INTERFACE),
                "OBoundControlModel::setFastPropertyValue_NoBroadcast : invalid argument !");
            if (!rValue.hasValue())
            {   // set property to "void"
                                com::sun::star::uno::Reference<com::sun::star::lang::XComponent> xComp(m_xLabelControl, com::sun::star::uno::UNO_QUERY);
                if (xComp.is())
                                        xComp->removeEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<com::sun::star::beans::XPropertyChangeListener*>(this)));
                m_xLabelControl = NULL;
                break;
            }

            InterfaceRef xNewValue = *(InterfaceRef*)rValue.getValue();

                        com::sun::star::uno::Reference<starawt::XControlModel> xAsModel(xNewValue, com::sun::star::uno::UNO_QUERY);
                        com::sun::star::uno::Reference<com::sun::star::lang::XServiceInfo> xAsServiceInfo(xNewValue, com::sun::star::uno::UNO_QUERY);
                        com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xAsPropSet(xNewValue, com::sun::star::uno::UNO_QUERY);
                        com::sun::star::uno::Reference<starcontainer::XChild> xAsChild(xNewValue, com::sun::star::uno::UNO_QUERY);
            if (!xAsModel.is() || !xAsServiceInfo.is() || !xAsPropSet.is() || !xAsChild.is())
            {
                                throw com::sun::star::lang::IllegalArgumentException();
            }

            if (!xAsServiceInfo->supportsService(m_aLabelServiceName))
            {
                                throw com::sun::star::lang::IllegalArgumentException();
            }

            // check if weself and the given model have a common anchestor (up to the forms collection)
                        com::sun::star::uno::Reference<starcontainer::XChild> xCont;
                        query_interface(static_cast<com::sun::star::uno::XWeak*>(this), xCont);
            InterfaceRef xMyTopLevel = xCont->getParent();
            while (xMyTopLevel.is())
            {
                                com::sun::star::uno::Reference<starform::XForm> xAsForm(xMyTopLevel, com::sun::star::uno::UNO_QUERY);
                if (!xAsForm.is())
                    // found my root
                    break;

                                com::sun::star::uno::Reference<starcontainer::XChild> xAsChild(xMyTopLevel, com::sun::star::uno::UNO_QUERY);
                xMyTopLevel = xAsChild.is() ? xAsChild->getParent() : InterfaceRef();
            }
            InterfaceRef xNewTopLevel = xAsChild->getParent();
            while (xNewTopLevel.is())
            {
                                com::sun::star::uno::Reference<starform::XForm> xAsForm(xNewTopLevel, com::sun::star::uno::UNO_QUERY);
                if (!xAsForm.is())
                    break;

                                com::sun::star::uno::Reference<starcontainer::XChild> xAsChild(xNewTopLevel, com::sun::star::uno::UNO_QUERY);
                xNewTopLevel = xAsChild.is() ? xAsChild->getParent() : InterfaceRef();
            }
            if (xNewTopLevel != xMyTopLevel)
            {
                // the both objects don't belong to the same forms collection -> not acceptable
                                throw com::sun::star::lang::IllegalArgumentException();
            }

            m_xLabelControl = xAsPropSet;
                        com::sun::star::uno::Reference<com::sun::star::lang::XComponent> xComp(m_xLabelControl, com::sun::star::uno::UNO_QUERY);
            if (xComp.is())
                                xComp->addEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<com::sun::star::beans::XPropertyChangeListener*>(this)));
        }
        break;
        default:
            OControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue );
    }
}

// XPropertyChangeListener
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::propertyChange( const com::sun::star::beans::PropertyChangeEvent& evt ) throw(com::sun::star::uno::RuntimeException)
{
    // Bei Wertaenderung neu initialisieren
    if (evt.PropertyName.equals(PROPERTY_VALUE))
    {
        osl::MutexGuard aGuard(m_aMutex);
        if (m_bForwardValueChanges && m_xColumn.is())
            _onValueChanged();
    }
}

// XBoundComponent
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::addUpdateListener(const com::sun::star::uno::Reference<starform::XUpdateListener>& _rxListener) throw(com::sun::star::uno::RuntimeException)
{
    m_aUpdateListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::removeUpdateListener(const com::sun::star::uno::Reference< starform::XUpdateListener>& _rxListener) throw(com::sun::star::uno::RuntimeException)
{
    m_aUpdateListeners.removeInterface(_rxListener);
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OBoundControlModel::commit() throw(com::sun::star::uno::RuntimeException)
{
    DBG_ASSERT(m_bCommitable, "OBoundControlModel::commit : invalid call (I'm not commitable !) ");
    {
        osl::MutexGuard aGuard(m_aMutex);
        if (!m_xField.is())
            return sal_True;
    }

    cppu::OInterfaceIteratorHelper aIter(m_aUpdateListeners);
        com::sun::star::lang::EventObject aEvt;
        aEvt.Source = static_cast<com::sun::star::uno::XWeak*>(this);
    sal_Bool bSucceed = sal_True;
    while (aIter.hasMoreElements() && bSucceed)
        bSucceed = ((starform::XUpdateListener*)aIter.next())->approveUpdate(aEvt);

    if (bSucceed)
    {
        osl::MutexGuard aGuard(m_aMutex);
        try
        {
            bSucceed = _commit();
        }
        catch(...)
        {
            bSucceed = sal_False;
        }
    }

    if (bSucceed)
    {
        cppu::OInterfaceIteratorHelper aIter(m_aUpdateListeners);
        while (aIter.hasMoreElements())
            ((starform::XUpdateListener*)aIter.next())->updated(aEvt);
    }

    return bSucceed;
}

//------------------------------------------------------------------------------
sal_Bool OBoundControlModel::connectToField(const com::sun::star::uno::Reference<starsdbc::XRowSet>& rForm)
{
    // wenn eine Verbindung zur Datenbank existiert
    if (rForm.is() && getConnection(rForm).is())
    {
        // Feld bestimmen und PropertyChangeListener
        m_xCursor = rForm;
                com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xFieldCandidate;

        if (m_xCursor.is())
        {
                        com::sun::star::uno::Reference<starsdbcx::XColumnsSupplier> xColumnsSupplier(m_xCursor, com::sun::star::uno::UNO_QUERY);
            DBG_ASSERT(xColumnsSupplier.is(), "OBoundControlModel::connectToField : the row set should support the com::sun::star::sdb::ResultSet service !");
            if (xColumnsSupplier.is())
            {
                                com::sun::star::uno::Reference<starcontainer::XNameAccess> xColumns(xColumnsSupplier->getColumns(), com::sun::star::uno::UNO_QUERY);
                if (xColumns.is() && xColumns->hasByName(m_aControlSource))
                {
                                        com::sun::star::uno::Any aElement(xColumns->getByName(m_aControlSource));
                                        DBG_ASSERT(xColumns->getElementType().equals(::getCppuType(reinterpret_cast<com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>*>(NULL))),
                        "OBoundControlModel::connectToField : the columns container should contain XPropertySets !");
                    // if this assertion fails we probably should do a queryInterface ....
                    aElement >>= xFieldCandidate;
                }
            }
        }

        // darf ich mich ueberhaupt an dieses Feld binden (Typ-Check)
        if (xFieldCandidate.is())
        {
            sal_Int32 nFieldType;
            xFieldCandidate->getPropertyValue(PROPERTY_FIELDTYPE) >>= nFieldType;
            if (_approve(nFieldType))
                m_xField = xFieldCandidate;
        }
        else
            m_xField = NULL;

        if (m_xField.is())
        {
            if(m_xField->getPropertySetInfo()->hasPropertyByName(PROPERTY_VALUE))
            {
                // an wertaenderungen horchen
                m_xField->addPropertyChangeListener(PROPERTY_VALUE, this);
                                m_xColumnUpdate = com::sun::star::uno::Reference<starsdb::XColumnUpdate>(m_xField, com::sun::star::uno::UNO_QUERY);
                                m_xColumn = com::sun::star::uno::Reference<starsdb::XColumn>(m_xField, com::sun::star::uno::UNO_QUERY);
                INT32 nNullableFlag; m_xField->getPropertyValue(PROPERTY_ISNULLABLE) >>= nNullableFlag;
                m_bRequired = (starsdbc::ColumnValue::NO_NULLS == nNullableFlag);
                    // we're optimistic : in case of ColumnValue_NULLABLE_UNKNOWN we assume nullability ....
            }
            else
            {
                OSL_ASSERT("OBoundControlModel::connectToField: property NAME not supported!");
                m_xField = NULL;
            }
        }
    }
    return m_xField.is();
}

//------------------------------------------------------------------------------
sal_Bool OBoundControlModel::_approve(sal_Int32 _nColumnType)
{
    if ((_nColumnType == starsdbc::DataType::BINARY) || (_nColumnType == starsdbc::DataType::VARBINARY)
        || (_nColumnType == starsdbc::DataType::LONGVARBINARY) || (_nColumnType == starsdbc::DataType::OTHER)
        || (_nColumnType == starsdbc::DataType::OBJECT) || (_nColumnType == starsdbc::DataType::DISTINCT)
        || (_nColumnType == starsdbc::DataType::STRUCT) || (_nColumnType == starsdbc::DataType::ARRAY)
        || (_nColumnType == starsdbc::DataType::BLOB) || (_nColumnType == starsdbc::DataType::CLOB)
        || (_nColumnType == starsdbc::DataType::REF) || (_nColumnType == starsdbc::DataType::SQLNULL))
        return sal_False;

    return sal_True;
}

// XLoadListener
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::loaded(const com::sun::star::lang::EventObject& _rEvent) throw(com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
        com::sun::star::uno::Reference<starsdbc::XRowSet> xForm(_rEvent.Source, com::sun::star::uno::UNO_QUERY);
    connectToField(xForm);

    m_bLoaded = sal_True;
    _loaded(_rEvent);

    if (m_xField.is())
        _onValueChanged();
}


//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::unloaded( const com::sun::star::lang::EventObject& aEvent ) throw(com::sun::star::uno::RuntimeException)
{
}

//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::reloading( const com::sun::star::lang::EventObject& aEvent ) throw(com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    m_bForwardValueChanges = sal_False;
}

//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::unloading(const com::sun::star::lang::EventObject& aEvent) throw(com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    _unloaded();

    if (m_xField.is())
    {
        m_xField->removePropertyChangeListener(PROPERTY_VALUE, this);
        resetField();
    }
    m_xCursor = NULL;
    m_bLoaded = sal_False;
}

//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::reloaded(const com::sun::star::lang::EventObject& aEvent) throw(com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);
    // did we lost the connection to the field because there was a new created?
    if (!m_xField.is())
    {
                com::sun::star::uno::Reference<starsdbc::XRowSet> xForm(aEvent.Source, com::sun::star::uno::UNO_QUERY);
        connectToField(xForm);
    }

    m_bForwardValueChanges = sal_True;

    // do we have a field, than get the new value
    if (m_xField.is())
        _onValueChanged();
}

//------------------------------------------------------------------------------
void OBoundControlModel::_loaded(const com::sun::star::lang::EventObject& rEvent)
{
}

//------------------------------------------------------------------------------
void OBoundControlModel::_unloaded()
{
}

// XReset
//-----------------------------------------------------------------------------
void OBoundControlModel::_reset()
{
}

//-----------------------------------------------------------------------------
void OBoundControlModel::addResetListener(const com::sun::star::uno::Reference<starform::XResetListener>& l) throw (com::sun::star::uno::RuntimeException)
{
    m_aResetListeners.addInterface(l);
}

//-----------------------------------------------------------------------------
void OBoundControlModel::removeResetListener(const com::sun::star::uno::Reference<starform::XResetListener>& l) throw (com::sun::star::uno::RuntimeException)
{
    m_aResetListeners.removeInterface(l);
}

//-----------------------------------------------------------------------------
void OBoundControlModel::reset() throw (com::sun::star::uno::RuntimeException)
{
    cppu::OInterfaceIteratorHelper aIter(m_aResetListeners);
        com::sun::star::lang::EventObject aResetEvent(static_cast<com::sun::star::uno::XWeak*>(this));
    sal_Bool bContinue = sal_True;
    while (aIter.hasMoreElements() && bContinue)
        bContinue = reinterpret_cast<starform::XResetListener*>(aIter.next())->approveReset(aResetEvent);

    if (!bContinue)
        return;

    osl::ClearableMutexGuard aGuard(m_aMutex);
    m_bResetting = sal_True;

    // gesondertes Verhalten bei Datenbankverbindung
    if (m_xField.is())
    {
        // nur wenn der derzeitige Wert des Feldes auf NULL steht
        // werden die Defaultwerte gesetzt
        // ansonsten soll der aktuelle Werte nochmals aus dem Feld gesetzt werden
        // dieses Verhalten ist noch nicht ganz ausgereift, was passiert wenn das Feld einen Defaultwert besitzt
        // und das Control auch?

        sal_Bool bIsNull = sal_True;
        // we have to access the field content at least once to get a reliable result by XColumn::wasNull
        try
        {
            m_xColumn->getString();
        }
        catch(...)
        {
            DBG_ERROR("OBoundControlModel::reset : XColumn::getString is expected to always succeed !");
        }
        bIsNull = m_xColumn->wasNull();

        if (bIsNull)
        {
                        com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xSet(m_xCursor, com::sun::star::uno::UNO_QUERY);
            sal_Bool bIsNewRecord = sal_False;
            if (xSet.is())
                xSet->getPropertyValue(PROPERTY_ISNEW) >>= bIsNewRecord;
            if (bIsNewRecord)
            {
                _reset();   // setzen der Werte,
                _commit();  // uebertragen der Werte ins Feld
                            // fuer das zuruecksetzen des modifyflags ist das Formular zust‰ndig
            }
            else
                _onValueChanged();
        }
        else
            _onValueChanged();
    }
    else
        _reset();

    m_bResetting = sal_False;
    aGuard.clear();

    cppu::OInterfaceIteratorHelper aIterDone(m_aResetListeners);
    while (aIterDone.hasMoreElements())
        reinterpret_cast<starform::XResetListener*>(aIterDone.next())->resetted(aResetEvent);
}

//.........................................................................
}
//... namespace frm .......................................................

