/*************************************************************************
 *
 *  $RCSfile: FormComponent.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:57:42 $
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
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
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
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif

#ifndef _FRM_RESOURCE_HXX_
#include "frm_resource.hxx"
#endif
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif


//... namespace frm .......................................................
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
using namespace ::drafts::com::sun::star::form;
using namespace ::dbtools;
using namespace ::comphelper;

//=========================================================================
//= base class for form layer controls
//=========================================================================
DBG_NAME(frm_OControl)
//------------------------------------------------------------------------------
OControl::OControl(const Reference<com::sun::star::lang::XMultiServiceFactory>& _rxFactory, const rtl::OUString& _sService)
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
        m_xAggregate = Reference<XAggregation>(_rxFactory->createInstance(_sService), UNO_QUERY);
        m_xControl = Reference<XControl>(m_xAggregate, UNO_QUERY);
    }

    if (m_xAggregate.is())
    {
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
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
Any SAL_CALL OControl::queryAggregation( const Type& _rType ) throw(RuntimeException)
{
    // ask the base class
    Any aReturn( OComponentHelper::queryAggregation(_rType) );
    // ask our own interfaces
    if (!aReturn.hasValue())
    {
        aReturn = OControl_BASE::queryInterface(_rType);
        // ask our aggregate
        if (!aReturn.hasValue() && m_xAggregate.is())
            aReturn = m_xAggregate->queryAggregation(_rType);
    }

    return aReturn;
}

//------------------------------------------------------------------------------
Sequence<sal_Int8> SAL_CALL OControl::getImplementationId() throw(RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

//------------------------------------------------------------------------------
Sequence<Type> SAL_CALL OControl::getTypes() throw(RuntimeException)
{
    Sequence<Type> aOwnTypes = _getTypes();
    Reference<com::sun::star::lang::XTypeProvider> xProv;

    if (query_aggregation(m_xAggregate, xProv))
        return concatSequences(aOwnTypes, xProv->getTypes());
    else
        return aOwnTypes;
}

//------------------------------------------------------------------------------
Sequence<Type> OControl::_getTypes()
{
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
        aTypes = concatSequences(OComponentHelper::getTypes(), OControl_BASE::getTypes());
    return aTypes;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OControl::disposing()
{
    OComponentHelper::disposing();

    Reference<com::sun::star::lang::XComponent> xComp;
    if (query_aggregation(m_xAggregate, xComp))
        xComp->dispose();
}

// XServiceInfo
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OControl::supportsService(const rtl::OUString& _rsServiceName) throw ( RuntimeException)
{
        Sequence<rtl::OUString> aSupported = getSupportedServiceNames();
    const rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rsServiceName))
            return sal_True;
    return sal_False;
}

//------------------------------------------------------------------------------
Sequence<rtl::OUString> SAL_CALL OControl::getSupportedServiceNames() throw(RuntimeException)
{
    Sequence< rtl::OUString > aSupported;

    // ask our aggregate
    Reference< com::sun::star::lang::XServiceInfo > xInfo;
    if ( query_aggregation( m_xAggregate, xInfo ) )
        aSupported = xInfo->getSupportedServiceNames();

    return aSupported;
}

// XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OControl::disposing(const com::sun::star::lang::EventObject& _rEvent) throw (RuntimeException)
{
    InterfaceRef xAggAsIface;
    query_aggregation(m_xAggregate, xAggAsIface);

    // does the disposing come from the aggregate ?
    if (xAggAsIface != InterfaceRef(_rEvent.Source, UNO_QUERY))
    {   // no -> forward it
                Reference<com::sun::star::lang::XEventListener> xListener;
        if (query_aggregation(m_xAggregate, xListener))
            xListener->disposing(_rEvent);
    }
}

// XControl
//------------------------------------------------------------------------------
void SAL_CALL OControl::setContext(const InterfaceRef& Context) throw (RuntimeException)
{
    if (m_xControl.is())
        m_xControl->setContext(Context);
}

//------------------------------------------------------------------------------
InterfaceRef SAL_CALL OControl::getContext() throw (RuntimeException)
{
    return m_xControl.is() ? m_xControl->getContext() : InterfaceRef();
}

//------------------------------------------------------------------------------
void SAL_CALL OControl::createPeer(const Reference<XToolkit>& Toolkit, const Reference<XWindowPeer>& Parent) throw (RuntimeException)
{
    if (m_xControl.is())
        m_xControl->createPeer(Toolkit, Parent);
}

//------------------------------------------------------------------------------
Reference<XWindowPeer> SAL_CALL OControl::getPeer() throw ( RuntimeException)
{
        return m_xControl.is() ? m_xControl->getPeer() : Reference<XWindowPeer>();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OControl::setModel(const Reference<XControlModel>& Model) throw ( RuntimeException)
{
    return m_xControl.is() ? m_xControl->setModel( Model ) : sal_False;
}

//------------------------------------------------------------------------------
Reference<XControlModel> SAL_CALL OControl::getModel() throw ( RuntimeException)
{
        return m_xControl.is() ? m_xControl->getModel() : Reference<XControlModel>();
}

//------------------------------------------------------------------------------
Reference<XView> SAL_CALL OControl::getView() throw ( RuntimeException)
{
        return m_xControl.is() ? m_xControl->getView() : Reference<XView>();
}

//------------------------------------------------------------------------------
void SAL_CALL OControl::setDesignMode(sal_Bool bOn) throw ( RuntimeException)
{
    if (m_xControl.is())
        m_xControl->setDesignMode(bOn);
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OControl::isDesignMode() throw ( RuntimeException)
{
    return m_xControl.is() ? m_xControl->isDesignMode() : sal_True;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OControl::isTransparent() throw ( RuntimeException)
{
    return m_xControl.is() ? m_xControl->isTransparent() : sal_True;
}

//==================================================================
//= OBoundControl
//==================================================================
DBG_NAME(frm_OBoundControl);
//------------------------------------------------------------------
OBoundControl::OBoundControl(const Reference<com::sun::star::lang::XMultiServiceFactory>& _rxFactory, const ::rtl::OUString& _sService)
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
// -----------------------------------------------------------------------------
Sequence< Type> OBoundControl::_getTypes()
{
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
        aTypes = concatSequences(OControl::_getTypes(), OBoundControl_BASE::getTypes());
    return aTypes;
}
//------------------------------------------------------------------
Any SAL_CALL OBoundControl::queryAggregation(const Type& _rType) throw(RuntimeException)
{
    // ask the base class
    Any aReturn(OControl::queryAggregation(_rType));
    // ask our own interfaces
    if (!aReturn.hasValue())
        aReturn = OBoundControl_BASE::queryInterface(_rType);

    return aReturn;
}

//------------------------------------------------------------------
sal_Bool SAL_CALL OBoundControl::getLock() throw(RuntimeException)
{
    return m_bLocked;
}

//------------------------------------------------------------------
void SAL_CALL OBoundControl::setLock(sal_Bool _bLock) throw(RuntimeException)
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
        Reference<XWindowPeer> xPeer = getPeer();
        Reference<com::sun::star::awt::XTextComponent> xText(xPeer, UNO_QUERY);

    if (xText.is())
        xText->setEditable(!_bLock);
    else
    {
        // disable the window
                Reference<XWindow> xComp(xPeer, UNO_QUERY);
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
Sequence<sal_Int8> SAL_CALL OControlModel::getImplementationId() throw(RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

//------------------------------------------------------------------
Sequence<Type> SAL_CALL OControlModel::getTypes() throw(RuntimeException)
{
    Sequence<Type> aOwnTypes = _getTypes();
    Reference<com::sun::star::lang::XTypeProvider> xProv;

    if (query_aggregation(m_xAggregate, xProv))
        return concatSequences(aOwnTypes, xProv->getTypes());
    else
        return aOwnTypes;
}

//------------------------------------------------------------------------------
Sequence<Type> OControlModel::_getTypes()
{
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
    {
        // my two base classes
        aTypes = concatSequences(OComponentHelper::getTypes(), OPropertySetAggregationHelper::getTypes(), OControlModel_BASE::getTypes());
    }
    return aTypes;
}

//------------------------------------------------------------------
Any SAL_CALL OControlModel::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    // base class 1
    Any aReturn(OComponentHelper::queryAggregation(_rType));

    // base class 2
    if (!aReturn.hasValue())
    {
        aReturn = OControlModel_BASE::queryInterface(_rType);

        // our own interfaces
        if (!aReturn.hasValue())
        {
            aReturn = OPropertySetAggregationHelper::queryInterface(_rType);
            // our aggregate
            if (!aReturn.hasValue() && m_xAggregate.is() && !_rType.equals(::getCppuType(static_cast< Reference< XCloneable>* >(NULL))))
                aReturn = m_xAggregate->queryAggregation(_rType);
        }
    }
    return aReturn;
}

//------------------------------------------------------------------------------
void OControlModel::readHelpTextCompatibly(const staruno::Reference< stario::XObjectInputStream >& _rxInStream)
{
    ::rtl::OUString sHelpText;
    ::comphelper::operator>>( _rxInStream, sHelpText);
    try
    {
        if (m_xAggregateSet.is())
            m_xAggregateSet->setPropertyValue(PROPERTY_HELPTEXT, makeAny(sHelpText));
    }
    catch(const Exception&)
    {
        OSL_ENSURE(sal_False, "OControlModel::readHelpTextCompatibly: could not forward the property value to the aggregate!");
    }
}

//------------------------------------------------------------------------------
void OControlModel::writeHelpTextCompatibly(const staruno::Reference< stario::XObjectOutputStream >& _rxOutStream)
{
    ::rtl::OUString sHelpText;
    try
    {
        if (m_xAggregateSet.is())
            m_xAggregateSet->getPropertyValue(PROPERTY_HELPTEXT) >>= sHelpText;
    }
    catch(const Exception&)
    {
        OSL_ENSURE(sal_False, "OControlModel::writeHelpTextCompatibly: could not retrieve the property value from the aggregate!");
    }
    ::comphelper::operator<<( _rxOutStream, sHelpText);
}

//------------------------------------------------------------------
OControlModel::OControlModel(
                        const Reference<com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
            const ::rtl::OUString& _rUnoControlModelTypeName,
            const ::rtl::OUString& rDefault, const sal_Bool _bSetDelegator)
    :OComponentHelper(m_aMutex)
    ,OPropertySetAggregationHelper(OComponentHelper::rBHelper)
    ,m_nTabIndex(FRM_DEFAULT_TABINDEX)
    ,m_nClassId(FormComponentType::CONTROL)
    ,m_xServiceFactory(_rxFactory)
{
    DBG_CTOR(OControlModel, NULL);
    if (_rUnoControlModelTypeName.getLength())  // the is a model we have to aggregate
    {
        increment(m_refCount);

        {
            m_xAggregate = Reference<XAggregation>(_rxFactory->createInstance(_rUnoControlModelTypeName), UNO_QUERY);
            setAggregation(m_xAggregate);

            if (m_xAggregateSet.is() && rDefault.getLength())
                m_xAggregateSet->setPropertyValue(PROPERTY_DEFAULTCONTROL, makeAny(rDefault));
        }

        if (_bSetDelegator)
            doSetDelegator();

        // Refcount wieder bei NULL
        decrement(m_refCount);
    }
}

//------------------------------------------------------------------
OControlModel::OControlModel( const OControlModel* _pOriginal, const Reference< XMultiServiceFactory>& _rxFactory, const sal_Bool _bSetDelegator )
    :OComponentHelper( m_aMutex )
    ,OPropertySetAggregationHelper( OComponentHelper::rBHelper )
    ,m_nTabIndex( FRM_DEFAULT_TABINDEX )
    ,m_nClassId( FormComponentType::CONTROL )
    ,m_xServiceFactory( _rxFactory )
{
    DBG_CTOR( OControlModel, NULL );
    DBG_ASSERT( _pOriginal, "OControlModel::OControlModel: invalid original!" );

    // copy members
    m_aName = _pOriginal->m_aName;
    m_aTag = _pOriginal->m_aTag;
    m_nTabIndex = _pOriginal->m_nTabIndex;
    m_nClassId = _pOriginal->m_nClassId;

    // temporarily increment refcount because of temporary references to ourself in the following
    increment( m_refCount );

    {
        // transfer the (only, at the very moment!) ref count
        m_xAggregate = createAggregateClone( _pOriginal );

        // set aggregation (retrieve other direct interfaces of the aggregate)
        setAggregation( m_xAggregate );
    }

    // set the delegator, if allowed by our derived class
    if ( _bSetDelegator )
        doSetDelegator();

    // decrement ref count
    decrement( m_refCount );
}

//------------------------------------------------------------------
OControlModel::~OControlModel()
{
    // release the aggregate
    doResetDelegator( );

    DBG_DTOR(OControlModel, NULL);
}

//------------------------------------------------------------------
void OControlModel::clonedFrom( const OControlModel* _pOriginal )
{
    // nothing to do in this base class
}

//------------------------------------------------------------------------------
void OControlModel::doResetDelegator()
{
    if (m_xAggregate.is())
        m_xAggregate->setDelegator(NULL);
}

//------------------------------------------------------------------------------
void OControlModel::doSetDelegator()
{
    increment(m_refCount);
    if (m_xAggregate.is())
    {
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
    }
    decrement(m_refCount);
}

// XChild
//------------------------------------------------------------------------------
InterfaceRef SAL_CALL OControlModel::getParent() throw(RuntimeException)
{
    return m_xParent;
}

//------------------------------------------------------------------------------
void SAL_CALL OControlModel::setParent(const InterfaceRef& _rxParent) throw(com::sun::star::lang::NoSupportException, RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

    Reference<XComponent> xComp(m_xParent, UNO_QUERY);
    if (xComp.is())
        xComp->removeEventListener(static_cast<XPropertiesChangeListener*>(this));

    {
        xComp = xComp.query( _rxParent );
        RTL_LOGFILE_CONTEXT( aLogger, "forms::OControlModel::setParent::logOnEventListener" );
        if ( xComp.is() )
            xComp->addEventListener(static_cast<XPropertiesChangeListener*>(this));
    }

    m_xParent = _rxParent;
}

// XNamed
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OControlModel::getName() throw(RuntimeException)
{
    ::rtl::OUString aReturn;
    OPropertySetHelper::getFastPropertyValue(PROPERTY_ID_NAME) >>= aReturn;
    return aReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL OControlModel::setName(const ::rtl::OUString& _rName) throw(RuntimeException)
{
        setFastPropertyValue(PROPERTY_ID_NAME, makeAny(_rName));
}

// XServiceInfo
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OControlModel::supportsService(const rtl::OUString& _rServiceName) throw ( RuntimeException)
{
        Sequence<rtl::OUString> aSupported = getSupportedServiceNames();
    const rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rServiceName))
            return sal_True;
    return sal_False;
}

//------------------------------------------------------------------------------
Sequence<rtl::OUString> SAL_CALL OControlModel::getSupportedServiceNames() throw(RuntimeException)
{
        Sequence<rtl::OUString> aSupported;

    // ask our aggregate
        Reference<com::sun::star::lang::XServiceInfo> xInfo;
    if (query_aggregation(m_xAggregate, xInfo))
        aSupported = xInfo->getSupportedServiceNames();

    aSupported.realloc(aSupported.getLength() + 2);
    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_FORMCOMPONENT;
    pArray[aSupported.getLength()-1] = ::rtl::OUString::createFromAscii("com.sun.star.form.FormControlModel");

    return aSupported;
}

// XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OControlModel::disposing(const com::sun::star::lang::EventObject& _rSource) throw (RuntimeException)
{
    // release the parent
    if (_rSource.Source == m_xParent)
    {
        osl::MutexGuard aGuard(m_aMutex);
        m_xParent = NULL;
    }
    else
    {
        Reference<com::sun::star::lang::XEventListener> xEvtLst;
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

    Reference<com::sun::star::lang::XComponent> xComp;
    if (query_aggregation(m_xAggregate, xComp))
        xComp->dispose();

    setParent(Reference<XFormComponent>());
}

//------------------------------------------------------------------------------
void SAL_CALL OControlModel::write(const Reference<stario::XObjectOutputStream>& _rxOutStream)
                        throw(stario::IOException, RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

    // 1. Schreiben des UnoControls
    Reference<stario::XMarkableStream> xMark(_rxOutStream, UNO_QUERY);
    if ( !xMark.is() )
    {
        throw IOException(
            FRM_RES_STRING( RID_STR_INVALIDSTREAM ),
            static_cast< ::cppu::OWeakObject* >( this )
        );
    }

    sal_Int32 nMark = xMark->createMark();
    sal_Int32 nLen = 0;

    _rxOutStream->writeLong(nLen);

    Reference<stario::XPersistObject> xPersist;
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
    ::comphelper::operator<<( _rxOutStream, m_aName);
    _rxOutStream->writeShort(m_nTabIndex);
    ::comphelper::operator<<( _rxOutStream, m_aTag); // 3. version

    // !!! IMPORTANT NOTE !!!
    // don't write any new members here : this wouldn't be compatible with older versions, as OControlModel
    // is a base class which is called in derived classes "read" method. So if you increment the version
    // and write new stuff, older office versions will read this in the _derived_ classes, which may result
    // in anything from data loss to crash.
    // !!! EOIN !!!
}

//------------------------------------------------------------------------------
void OControlModel::read(const Reference<stario::XObjectInputStream>& InStream) throw (::com::sun::star::io::IOException, RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

    Reference<stario::XMarkableStream> xMark(InStream, UNO_QUERY);
    if ( !xMark.is() )
    {
        throw IOException(
            FRM_RES_STRING( RID_STR_INVALIDSTREAM ),
            static_cast< ::cppu::OWeakObject* >( this )
        );
    }

    // 1. Lesen des UnoControls
    sal_Int32 nLen = InStream->readLong();
    if (nLen)
    {
        sal_Int32 nMark = xMark->createMark();

        Reference<stario::XPersistObject> xPersist;
        if (query_aggregation(m_xAggregate, xPersist))
            xPersist->read(InStream);

        xMark->jumpToMark(nMark);
        InStream->skipBytes(nLen);
        xMark->deleteMark(nMark);
    }

    // 2. Lesen des Versionsnummer
    UINT16 nVersion = InStream->readShort();

    // 3. Lesen der allgemeinen Properties
    ::comphelper::operator>>( InStream, m_aName);
    m_nTabIndex  = InStream->readShort();

    if (nVersion > 0x0002)
        ::comphelper::operator>>( InStream, m_aTag);

    // we had a version where we wrote the help text
    if (nVersion == 0x0004)
        readHelpTextCompatibly(InStream);

    DBG_ASSERT(nVersion < 5, "OControlModel::read : suspicious version number !");
    // 4 was the version where we wrote the help text
    // later versions shouldn't exist (see write for a detailed comment)
}

//------------------------------------------------------------------------------
PropertyState OControlModel::getPropertyStateByHandle( sal_Int32 _nHandle )
{
    Any aDefault = getPropertyDefaultByHandle( _nHandle );
    Any aCurrent;
    getFastPropertyValue( aCurrent, _nHandle );

    if ( ::comphelper::compare( aDefault, aCurrent ) )
        return PropertyState_DEFAULT_VALUE;

    return PropertyState_DIRECT_VALUE;
}

//------------------------------------------------------------------------------
void OControlModel::setPropertyToDefaultByHandle( sal_Int32 _nHandle)
{
    Any aDefault = getPropertyDefaultByHandle( _nHandle );

    Any aConvertedValue, aOldValue;
    if ( convertFastPropertyValue( aConvertedValue, aOldValue, _nHandle, aDefault ) )
    {
        setFastPropertyValue_NoBroadcast( _nHandle, aConvertedValue );
        // TODO: fire the property change
    }
}

//------------------------------------------------------------------------------
Any OControlModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
{
    Any aReturn;
    switch ( _nHandle )
    {
        case PROPERTY_ID_NAME:
        case PROPERTY_ID_TAG:
            aReturn <<= ::rtl::OUString();
            break;

        case PROPERTY_ID_CLASSID:
            aReturn <<= (sal_Int16)FormComponentType::CONTROL;
            break;

        case PROPERTY_ID_TABINDEX:
            aReturn <<= (sal_Int16)FRM_DEFAULT_TABINDEX;
            break;
    }
    return aReturn;
}

//------------------------------------------------------------------------------
void OControlModel::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
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
        default:
            OPropertySetAggregationHelper::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
sal_Bool OControlModel::convertFastPropertyValue(
                        Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
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
        case PROPERTY_ID_TABINDEX:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_nTabIndex);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OControlModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
                        throw (Exception)
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
    }
}

//==================================================================
//= OBoundControlModel
//==================================================================
DBG_NAME(frm_OBoundControlModel);
//------------------------------------------------------------------
Any SAL_CALL OBoundControlModel::queryAggregation( const Type& _rType ) throw (RuntimeException)
{
    Any aReturn( OControlModel::queryAggregation(_rType) );
    if (!aReturn.hasValue())
    {
        aReturn = OBoundControlModel_BASE1::queryInterface(_rType);

        if ( !aReturn.hasValue() && m_bCommitable )
            aReturn = OBoundControlModel_BASE2::queryInterface( _rType );

        if ( !aReturn.hasValue() && m_bSupportsExternalBinding )
            aReturn = OBoundControlModel_BASE3::queryInterface( _rType );
    }

    return aReturn;
}

//------------------------------------------------------------------
OBoundControlModel::OBoundControlModel(
        const Reference<com::sun::star::lang::XMultiServiceFactory>& _rxFactory,
        const ::rtl::OUString& _rUnoControlModelTypeName,
        const ::rtl::OUString& _rDefault,
        const sal_Bool _bCommitable,
        const sal_Bool _bSupportExternalBinding)
    :OControlModel( _rxFactory, _rUnoControlModelTypeName, _rDefault, sal_False )
    ,OPropertyChangeListener( m_aMutex )
    ,m_aUpdateListeners(m_aMutex)
    ,m_aResetListeners(m_aMutex)
    ,m_bLoaded(sal_False)
    ,m_bRequired(sal_False)
    ,m_bCommitable(_bCommitable)
    ,m_bSupportsExternalBinding( _bSupportExternalBinding )
    ,m_aLabelServiceName(FRM_SUN_COMPONENT_FIXEDTEXT)
    ,m_bForwardValueChanges(sal_True)
    ,m_bLoadListening( sal_False )
    ,m_nValuePropertyAggregateHandle( -1 )
    ,m_pAggPropMultiplexer( NULL )
    ,m_bSettingAggregateState( sal_False )
{
    DBG_CTOR(frm_OBoundControlModel, NULL);

    // start property listening at the aggregate
    implInitAggMultiplexer( );
}

//------------------------------------------------------------------
OBoundControlModel::OBoundControlModel(
        const OBoundControlModel* _pOriginal, const Reference< XMultiServiceFactory>& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory, sal_False )
    ,OPropertyChangeListener( m_aMutex )
    ,m_aUpdateListeners( m_aMutex )
    ,m_aResetListeners( m_aMutex )
    ,m_bLoaded( sal_False )
    ,m_bRequired( sal_False )
    ,m_bCommitable( _pOriginal->m_bCommitable )
    ,m_bSupportsExternalBinding( _pOriginal->m_bSupportsExternalBinding )
    ,m_bForwardValueChanges( sal_True )
    ,m_bLoadListening( sal_False )
    ,m_nValuePropertyAggregateHandle( _pOriginal->m_nValuePropertyAggregateHandle )
    ,m_pAggPropMultiplexer( NULL )
    ,m_bSettingAggregateState( sal_False )
{
    DBG_CTOR(frm_OBoundControlModel, NULL);

    // start property listening at the aggregate
    implInitAggMultiplexer( );

    m_aLabelServiceName = _pOriginal->m_aLabelServiceName;
    m_sValuePropertyName = _pOriginal->m_sValuePropertyName;
    m_nValuePropertyAggregateHandle = _pOriginal->m_nValuePropertyAggregateHandle;
    m_aControlSource = _pOriginal->m_aControlSource;
    // m_xLabelControl, though being a property, is not to be cloned, not even the reference will be transfered.
    // (the former should be clear - a clone of the object we're only referencing does not make sense)
    // (the second would violate the restriction for label controls that they're part of the
    // same form component hierarchy - we ourself are no part, yet, so we can't have a label control)

    // start listening for changes at the value property
    implInitValuePropertyListening( );
}

//------------------------------------------------------------------
OBoundControlModel::~OBoundControlModel()
{
    if ( !OComponentHelper::rBHelper.bDisposed )
    {
        acquire();
        dispose();
    }

    doResetDelegator( );

    OSL_ENSURE( m_pAggPropMultiplexer, "OBoundControlModel::~OBoundControlModel: what about my property multiplexer?" );
    if ( m_pAggPropMultiplexer )
    {
        m_pAggPropMultiplexer->dispose();
        m_pAggPropMultiplexer->release();
        m_pAggPropMultiplexer = NULL;
    }

    DBG_DTOR(frm_OBoundControlModel, NULL);
}

//------------------------------------------------------------------
void OBoundControlModel::clonedFrom( const OControlModel* _pOriginal )
{
    const OBoundControlModel* pBoundOriginal = static_cast< const OBoundControlModel* >( _pOriginal );
    // the value binding can be handled as if somebody called setValueBinding here
    // By definition, bindings can be share between bindables
    if ( pBoundOriginal && pBoundOriginal->m_xExternalBinding.is() )
        setValueBinding( pBoundOriginal->m_xExternalBinding );
}

//-----------------------------------------------------------------------------
void OBoundControlModel::implInitAggMultiplexer( )
{
    increment( m_refCount );
    if ( m_xAggregateSet.is() )
    {
        m_pAggPropMultiplexer = new OPropertyChangeMultiplexer( this, m_xAggregateSet, sal_False );
        m_pAggPropMultiplexer->acquire();
    }
    decrement( m_refCount );

       doSetDelegator();
}

//-----------------------------------------------------------------------------
void OBoundControlModel::implInitValuePropertyListening( ) const
{
    // start listening for changes at the value property
    // There are two pre-requisites for this to be done:
    // 1. We support external value bindings. In this case, the changes in the control value need to
    //    be propagated to the external binding immediately when they happen
    // 2. We are not committable. In this case, changes in the control value need to be propagated
    //    to the database column immediately when they happen.
    if ( m_bSupportsExternalBinding || !m_bCommitable )
    {
        OSL_ENSURE( m_pAggPropMultiplexer, "OBoundControlModel::implInitValuePropertyListening: no multiplexer!" );
        if ( m_pAggPropMultiplexer && m_sValuePropertyName.getLength() )
            m_pAggPropMultiplexer->addProperty( m_sValuePropertyName );
    }
}

//-----------------------------------------------------------------------------
void OBoundControlModel::initValueProperty( const ::rtl::OUString& _rValuePropertyName, sal_Int32 _nValuePropertyExternalHandle )
{
    OSL_PRECOND( !m_sValuePropertyName.getLength() && -1 == m_nValuePropertyAggregateHandle,
        "OBoundControlModel::initValueProperty: already called before!" );
    OSL_ENSURE( _rValuePropertyName.getLength(), "OBoundControlModel::initValueProperty: invalid property name!" );
    OSL_ENSURE( _nValuePropertyExternalHandle != -1, "OBoundControlModel::initValueProperty: invalid property handle!" );

    m_sValuePropertyName = _rValuePropertyName;
    m_nValuePropertyAggregateHandle = getOriginalHandle( _nValuePropertyExternalHandle );
    OSL_ENSURE( m_nValuePropertyAggregateHandle != -1, "OBoundControlModel::initValueProperty: unable to find the original handle!" );

    // start listening for changes at the value property
    implInitValuePropertyListening( );
}

//-----------------------------------------------------------------------------
void OBoundControlModel::suspendValueListening( )
{
    OSL_PRECOND( m_sValuePropertyName.getLength(), "OBoundControlModel::suspendValueListening: don't have a value property!" );
    OSL_PRECOND( m_pAggPropMultiplexer, "OBoundControlModel::suspendValueListening: I *am* not listening!" );

    if ( m_pAggPropMultiplexer )
        m_pAggPropMultiplexer->lock();
}

//-----------------------------------------------------------------------------
void OBoundControlModel::resumeValueListening( )
{
    OSL_PRECOND( m_sValuePropertyName.getLength(), "OBoundControlModel::resumeValueListening: don't have a value property!" );
    OSL_PRECOND( m_pAggPropMultiplexer, "OBoundControlModel::resumeValueListening: I *am* not listening at all!" );
    OSL_PRECOND( !m_pAggPropMultiplexer || m_pAggPropMultiplexer->locked(), "OBoundControlModel::resumeValueListening: listening not suspended currently!" );

    if ( m_pAggPropMultiplexer )
        m_pAggPropMultiplexer->unlock();
}

//-----------------------------------------------------------------------------
namespace
{
    template< class T >
    void appendSequence( Sequence< T >& _rInOut, const Sequence< T >& _rAppend )
    {
        sal_Int32 nLengthBefore = _rInOut.getLength();
        sal_Int32 nLengthAppend = _rAppend.getLength();
        _rInOut.realloc( nLengthBefore + nLengthAppend );
        ::std::copy(
            _rAppend.getConstArray(),
            _rAppend.getConstArray() + nLengthAppend,
            _rInOut.getArray() + nLengthBefore
        );
    }
}

//-----------------------------------------------------------------------------
Sequence< Type > OBoundControlModel::_getTypes()
{
    Sequence< Type > aTypes( concatSequences(
        OControlModel::_getTypes(),
        OBoundControlModel_BASE1::getTypes()
    ) );

    if ( m_bCommitable )
        appendSequence( aTypes, OBoundControlModel_BASE2::getTypes() );

    if ( m_bSupportsExternalBinding )
        appendSequence( aTypes, OBoundControlModel_BASE3::getTypes() );

    return aTypes;
}

// OComponentHelper
//-----------------------------------------------------------------------------
void OBoundControlModel::disposing()
{
    OControlModel::disposing();

    osl::MutexGuard aGuard(m_aMutex);

    if ( m_pAggPropMultiplexer )
        m_pAggPropMultiplexer->dispose();

    // notify all our listeners
    com::sun::star::lang::EventObject aEvt( static_cast< XWeak* >( this ) );
    m_aResetListeners.disposeAndClear( aEvt );
    m_aUpdateListeners.disposeAndClear( aEvt );

    // disconnect from our database column
    // TODO: could we replace the following 5 lines with a call to disconnectDatabaseColumn?
    // The only more thing which it does is calling onDisconnectedDbColumn - could this
    // cause trouble? At least when we continue to call OControlModel::disposing before, it *may*.
    if ( m_xField.is() )
    {
        m_xField->removePropertyChangeListener( PROPERTY_VALUE, this );
        resetField();
    }
    m_xCursor = NULL;

    // disconnect from our external value binding
    if ( hasExternalValueBinding() )
        disconnectExternalValueBinding();

    Reference< XComponent > xComp( m_xLabelControl, UNO_QUERY );
    if ( xComp.is() )
        xComp->removeEventListener(static_cast< XEventListener* >( static_cast< XPropertyChangeListener* >( this ) ) );
}

//------------------------------------------------------------------------------
void OBoundControlModel::_propertyChanged( const PropertyChangeEvent& _rEvt ) throw ( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bSettingAggregateState )
        // we caused this ourself
        return;

    OSL_ENSURE( _rEvt.PropertyName == m_sValuePropertyName,
        "OBoundControlModel::_propertyChanged: where did this come from (1)?" );
    OSL_ENSURE( m_pAggPropMultiplexer && !m_pAggPropMultiplexer->locked(),
        "OBoundControlModel::_propertyChanged: where did this come from (2)?" );

    if ( _rEvt.PropertyName == m_sValuePropertyName )
    {
        if ( hasExternalValueBinding() )
        {   // the control value changed, while we have an external value binding
            // -> forward the value to it
            transferControlValueToExternal( );
        }
        else if ( !m_bCommitable && m_xColumnUpdate.is() )
        {   // the control value changed, while we have are  bound to a database column,
            // but not committable (which means changes in the control have to be reflected to
            // the underlying database column immediately)
            // -> forward the value to the database column
            commitControlValueToDbColumn( false );
        }
    }
}

//------------------------------------------------------------------------------
void OBoundControlModel::startAggregatePropertyListening( const ::rtl::OUString& _rPropertyName )
{
    OSL_PRECOND( m_pAggPropMultiplexer, "OBoundControlModel::startAggregatePropertyListening: no multiplexer!" );
    OSL_ENSURE( _rPropertyName.getLength(), "OBoundControlModel::startAggregatePropertyListening: invalid property name!" );

    if ( m_pAggPropMultiplexer && _rPropertyName.getLength() )
    {
        m_pAggPropMultiplexer->addProperty( _rPropertyName );
    }
}

//------------------------------------------------------------------------------
void OBoundControlModel::startLoadListening( )
{
    OSL_PRECOND( !isLoadListening(), "OBoundControlModel::startLoadListening: already listening!" );
    OSL_PRECOND( m_xParent.is(), "OBoundControlModel::startLoadListening: no parent to listen at!" );
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::startLoadListening: external value binding should overrule the database binding!" );

    Reference< XLoadable > xLoadable( m_xParent, UNO_QUERY );
    if ( xLoadable.is() )
    {
        RTL_LOGFILE_CONTEXT( aLogger, "forms::OBoundControlModel::startLoadListening" );
        xLoadable->addLoadListener( this );
        m_bLoadListening = sal_True;
    }
}

//------------------------------------------------------------------------------
void OBoundControlModel::stopLoadListening( )
{
    OSL_PRECOND( isLoadListening(), "OBoundControlModel::stopLoadListening: not listening!" );

    Reference< XLoadable > xLoadable( m_xParent, UNO_QUERY );
    if ( xLoadable.is() && isLoadListening() )
    {
        xLoadable->removeLoadListener( this );
        m_bLoadListening = sal_False;
    }
}

// XChild
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::setParent(const Reference<XInterface>& _rxParent) throw(com::sun::star::lang::NoSupportException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // log off old listeners
    if ( isLoadListening() )
        stopLoadListening( );

    OControlModel::setParent(_rxParent);

    // log on new listeners - only in case we do not have an external value binding
    if ( m_xParent.is() && !hasExternalValueBinding() )
        startLoadListening( );
}

// XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::disposing(const com::sun::star::lang::EventObject& _rEvent) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( _rEvent.Source == m_xField )
    {
        resetField();
    }
    else if ( _rEvent.Source == m_xLabelControl )
    {
        Reference<XPropertySet> xOldValue = m_xLabelControl;
        m_xLabelControl = NULL;

        // fire a property change event
        Any aOldValue; aOldValue <<= xOldValue;
        Any aNewValue; aNewValue <<= m_xLabelControl;
        sal_Int32 nHandle = PROPERTY_ID_CONTROLLABEL;
        OPropertySetHelper::fire( &nHandle, &aNewValue, &aOldValue, 1, sal_False );
    }
    else if ( _rEvent.Source == m_xExternalBinding )
    {
        disconnectExternalValueBinding( );
    }
    else
        OControlModel::disposing(_rEvent);
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OBoundControlModel::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = ::rtl::OUString::createFromAscii("com.sun.star.form.DataAwareControlModel");
    return aSupported;
}


// XPersist
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::write( const Reference<stario::XObjectOutputStream>& _rxOutStream ) throw(stario::IOException, RuntimeException)
{
    OControlModel::write(_rxOutStream);

    osl::MutexGuard aGuard(m_aMutex);

    // Version
    _rxOutStream->writeShort(0x0002);

    // Controlsource
    ::comphelper::operator<<( _rxOutStream, m_aControlSource);

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
    Reference<com::sun::star::lang::XComponent> xComp(m_xLabelControl, UNO_QUERY);
    if (xComp.is())
        xComp->removeEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<XPropertyChangeListener*>(this)));
    m_xLabelControl = NULL;
}

//------------------------------------------------------------------------------
void OBoundControlModel::readCommonProperties(const Reference<stario::XObjectInputStream>& _rxInStream)
{
    sal_Int32 nLen = _rxInStream->readLong();

        Reference<stario::XMarkableStream> xMark(_rxInStream, UNO_QUERY);
    DBG_ASSERT(xMark.is(), "OBoundControlModel::readCommonProperties : can only work with markable streams !");
    sal_Int32 nMark = xMark->createMark();

    // read the reference to the label control
        Reference<stario::XPersistObject> xPersist;
    sal_Int32 nUsedFlag;
    nUsedFlag = _rxInStream->readLong();
    if (nUsedFlag)
        xPersist = _rxInStream->readObject();
        m_xLabelControl = Reference<XPropertySet>(xPersist, UNO_QUERY);
        Reference<com::sun::star::lang::XComponent> xComp(m_xLabelControl, UNO_QUERY);
    if (xComp.is())
                xComp->addEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<XPropertyChangeListener*>(this)));

    // read any other new common properties here

    // skip the remaining bytes
    xMark->jumpToMark(nMark);
    _rxInStream->skipBytes(nLen);
    xMark->deleteMark(nMark);
}

//------------------------------------------------------------------------------
void OBoundControlModel::writeCommonProperties(const Reference<stario::XObjectOutputStream>& _rxOutStream)
{
        Reference<stario::XMarkableStream> xMark(_rxOutStream, UNO_QUERY);
    DBG_ASSERT(xMark.is(), "OBoundControlModel::writeCommonProperties : can only work with markable streams !");
    sal_Int32 nMark = xMark->createMark();

    // a placeholder where we will write the overall length (later in this method)
    sal_Int32 nLen = 0;
    _rxOutStream->writeLong(nLen);

    // write the reference to the label control
    Reference<stario::XPersistObject> xPersist(m_xLabelControl, UNO_QUERY);
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
void SAL_CALL OBoundControlModel::read( const Reference< stario::XObjectInputStream >& _rxInStream ) throw(stario::IOException, RuntimeException)
{
    OControlModel::read(_rxInStream);

    osl::MutexGuard aGuard(m_aMutex);
    UINT16 nVersion = _rxInStream->readShort();
    ::comphelper::operator>>( _rxInStream, m_aControlSource);
}

//------------------------------------------------------------------------------
void OBoundControlModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_CONTROLSOURCEPROPERTY:
            rValue <<= m_sValuePropertyName;
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
                                Any& _rConvertedValue, Any& _rOldValue,
                sal_Int32 _nHandle,
                                const Any& _rValue)
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
                _rConvertedValue = Any();
                getFastPropertyValue(_rOldValue, _nHandle);
                bModified = m_xLabelControl.is();
            }
            else
            {
                bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_xLabelControl);
                if (!m_xLabelControl.is())
                    // an empty interface is interpreted as VOID
                    _rOldValue.clear();
            }
            break;
        default:
            bModified = OControlModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OBoundControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_CONTROLSOURCE:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_STRING, "invalid type" );
            rValue >>= m_aControlSource;
            break;
        case PROPERTY_ID_BOUNDFIELD:
            DBG_ERROR("OBoundControlModel::setFastPropertyValue_NoBroadcast : BoundField should be a read-only property !");
                throw com::sun::star::lang::IllegalArgumentException();
            break;
        case PROPERTY_ID_CONTROLLABEL:
        {
            DBG_ASSERT(!rValue.hasValue() || (rValue.getValueType().getTypeClass() == TypeClass_INTERFACE),
                "OBoundControlModel::setFastPropertyValue_NoBroadcast : invalid argument !");
            if (!rValue.hasValue())
            {   // set property to "void"
                Reference<com::sun::star::lang::XComponent> xComp(m_xLabelControl, UNO_QUERY);
                if (xComp.is())
                    xComp->removeEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<XPropertyChangeListener*>(this)));
                m_xLabelControl = NULL;
                break;
            }

            InterfaceRef xNewValue;
            rValue >>= xNewValue;

            Reference<XControlModel> xAsModel(xNewValue, UNO_QUERY);
            Reference<com::sun::star::lang::XServiceInfo> xAsServiceInfo(xNewValue, UNO_QUERY);
            Reference<XPropertySet> xAsPropSet(xNewValue, UNO_QUERY);
            Reference<XChild> xAsChild(xNewValue, UNO_QUERY);
            if (!xAsModel.is() || !xAsServiceInfo.is() || !xAsPropSet.is() || !xAsChild.is())
            {
                throw com::sun::star::lang::IllegalArgumentException();
            }

            if (!xAsServiceInfo->supportsService(m_aLabelServiceName))
            {
                throw com::sun::star::lang::IllegalArgumentException();
            }

            // check if weself and the given model have a common anchestor (up to the forms collection)
            Reference<XChild> xCont;
            query_interface(static_cast<XWeak*>(this), xCont);
            InterfaceRef xMyTopLevel = xCont->getParent();
            while (xMyTopLevel.is())
            {
                Reference<XForm> xAsForm(xMyTopLevel, UNO_QUERY);
                if (!xAsForm.is())
                    // found my root
                    break;

                Reference<XChild> xAsChild(xMyTopLevel, UNO_QUERY);
                xMyTopLevel = xAsChild.is() ? xAsChild->getParent() : InterfaceRef();
            }
            InterfaceRef xNewTopLevel = xAsChild->getParent();
            while (xNewTopLevel.is())
            {
                Reference<XForm> xAsForm(xNewTopLevel, UNO_QUERY);
                if (!xAsForm.is())
                    break;

                Reference<XChild> xAsChild(xNewTopLevel, UNO_QUERY);
                xNewTopLevel = xAsChild.is() ? xAsChild->getParent() : InterfaceRef();
            }
            if (xNewTopLevel != xMyTopLevel)
            {
                // the both objects don't belong to the same forms collection -> not acceptable
                throw com::sun::star::lang::IllegalArgumentException();
            }

            m_xLabelControl = xAsPropSet;
            Reference<com::sun::star::lang::XComponent> xComp(m_xLabelControl, UNO_QUERY);
            if (xComp.is())
                xComp->addEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<XPropertyChangeListener*>(this)));
        }
        break;
        default:
            OControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue );
    }
}

// XPropertyChangeListener
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::propertyChange( const PropertyChangeEvent& evt ) throw(RuntimeException)
{
    // TODO: check how this works with external bindings
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::propertyChange: Hey, I have an external binding, so how this?" );
        // if we have an external value binding, then we should *not* be a property change
        // listener at an sdb::Column

    // Bei Wertaenderung neu initialisieren
    if ( evt.PropertyName.equals( PROPERTY_VALUE ) )
    {
        osl::MutexGuard aGuard(m_aMutex);
        if ( m_bForwardValueChanges && m_xColumn.is() )
            transferDbValueToControl();
    }
}

// XBoundComponent
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::addUpdateListener(const Reference<XUpdateListener>& _rxListener) throw(RuntimeException)
{
    m_aUpdateListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::removeUpdateListener(const Reference< XUpdateListener>& _rxListener) throw(RuntimeException)
{
    m_aUpdateListeners.removeInterface(_rxListener);
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OBoundControlModel::commit() throw(RuntimeException)
{
    OSL_PRECOND( m_bCommitable, "OBoundControlModel::commit: invalid call (I'm not commitable !) " );
    if ( hasExternalValueBinding() )
    {
        // in most cases, no action is required: For most derivees, we know the value property of
        // our control (see initValueProperty), and when an external binding is active, we
        // instantly forward all changes in this property to the external binding.
        if ( !m_sValuePropertyName.getLength() )
            // but for those derivees which did not use this feature, we need an
            // explicit transfer
            transferControlValueToExternal( );
        return sal_True;
    }

    OSL_ENSURE( !hasExternalValueBinding(), "OBoundControlModel::commit: control flow broken!" );
        // we reach this only if we're not working with an external binding

    {
        osl::MutexGuard aGuard(m_aMutex);
        if ( !m_xField.is() )
            return sal_True;
    }

    ::cppu::OInterfaceIteratorHelper aIter( m_aUpdateListeners );
    EventObject aEvt;
    aEvt.Source = static_cast< XWeak* >( this );
    sal_Bool bSucceed = sal_True;
    while (aIter.hasMoreElements() && bSucceed)
        bSucceed = static_cast< XUpdateListener* >( aIter.next() )->approveUpdate( aEvt );

    if ( bSucceed )
    {
        osl::MutexGuard aGuard(m_aMutex);
        try
        {
            if ( m_xColumnUpdate.is() )
                bSucceed = commitControlValueToDbColumn( sal_False );
        }
        catch(Exception&)
        {
            bSucceed = sal_False;
        }
    }

    if ( bSucceed )
    {
        cppu::OInterfaceIteratorHelper aIter( m_aUpdateListeners );
        while ( aIter.hasMoreElements() )
            static_cast< XUpdateListener* >( aIter.next() )->updated( aEvt );
    }

    return bSucceed;
}

//------------------------------------------------------------------------------
void OBoundControlModel::resetField()
{
    m_xColumnUpdate.clear();
    m_xColumn.clear();
    m_xField.clear();
}

//------------------------------------------------------------------------------
sal_Bool OBoundControlModel::connectToField(const Reference<XRowSet>& rForm)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::connectToField: invalid call (have an external binding)!" );

    // wenn eine Verbindung zur Datenbank existiert
    if (rForm.is() && getConnection(rForm).is())
    {
        // Feld bestimmen und PropertyChangeListener
        m_xCursor = rForm;
        Reference<XPropertySet> xFieldCandidate;

        if (m_xCursor.is())
        {
            Reference<XColumnsSupplier> xColumnsSupplier(m_xCursor, UNO_QUERY);
            DBG_ASSERT(xColumnsSupplier.is(), "OBoundControlModel::connectToField : the row set should support the com::sun::star::sdb::ResultSet service !");
            if (xColumnsSupplier.is())
            {
                Reference<XNameAccess> xColumns(xColumnsSupplier->getColumns(), UNO_QUERY);
                if (xColumns.is() && xColumns->hasByName(m_aControlSource))
                {
                    Any aElement(xColumns->getByName(m_aControlSource));
                    DBG_ASSERT(xColumns->getElementType().equals(::getCppuType(reinterpret_cast<Reference<XPropertySet>*>(NULL))),
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
            if (approveDbColumnType(nFieldType))
                setField(xFieldCandidate,sal_False);
        }
        else
            setField(NULL,sal_False);

        if (m_xField.is())
        {
            if(m_xField->getPropertySetInfo()->hasPropertyByName(PROPERTY_VALUE))
            {
                // an wertaenderungen horchen
                m_xField->addPropertyChangeListener( PROPERTY_VALUE, this );
                m_xColumnUpdate = Reference< XColumnUpdate >( m_xField, UNO_QUERY );
                m_xColumn = Reference< XColumn >( m_xField, UNO_QUERY );

                INT32 nNullableFlag = ColumnValue::NO_NULLS;
                m_xField->getPropertyValue(PROPERTY_ISNULLABLE) >>= nNullableFlag;
                m_bRequired = (ColumnValue::NO_NULLS == nNullableFlag);
                    // we're optimistic : in case of ColumnValue_NULLABLE_UNKNOWN we assume nullability ....
            }
            else
            {
                OSL_ENSURE(sal_False, "OBoundControlModel::connectToField: property NAME not supported!");
                setField(NULL,sal_False);
            }
        }
    }
    return m_xField.is();
}

//------------------------------------------------------------------------------
sal_Bool OBoundControlModel::approveDbColumnType(sal_Int32 _nColumnType)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::approveDbColumnType: invalid call (have an external binding)!" );

    if ((_nColumnType == DataType::BINARY) || (_nColumnType == DataType::VARBINARY)
        || (_nColumnType == DataType::LONGVARBINARY) || (_nColumnType == DataType::OTHER)
        || (_nColumnType == DataType::OBJECT) || (_nColumnType == DataType::DISTINCT)
        || (_nColumnType == DataType::STRUCT) || (_nColumnType == DataType::ARRAY)
        || (_nColumnType == DataType::BLOB) || (_nColumnType == DataType::CLOB)
        || (_nColumnType == DataType::REF) || (_nColumnType == DataType::SQLNULL))
        return sal_False;

    return sal_True;
}

//==============================================================================
// value binding handling

//------------------------------------------------------------------------------
void OBoundControlModel::connectDatabaseColumn( const Reference< XRowSet >& _rxRowSet, bool _bFromReload )
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::connectDatabaseColumn: not to be called with an external value binding!" );
    ::osl::MutexGuard aGuard( m_aMutex );

    // consistency checks
    DBG_ASSERT( !( m_xField.is() && !_bFromReload ),
        "OBoundControlModel::connectDatabaseColumn: the form is just *loaded*, but we already have a field!" );

    Reference< XPropertySet >   xOldField = m_xField;
    if ( !m_xField.is() )
    {
        // connect to the column
        connectToField( _rxRowSet );
    }

    // now that we're connected (more or less, even if we did not find a column),
    // we definately want to forward any potentially occuring value changes
    m_bForwardValueChanges = sal_True;

    // let derived classes react on this new connection
    m_bLoaded = sal_True;
    onConnectedDbColumn( _rxRowSet );

    // did we successfully connect to a database column?
    if ( m_xField.is() )
    {   // initially transfer the db column value to the control
        // but only if the rowset if posisitioned on a valid record
        if ( _rxRowSet.is() )
        {
            if ( !_rxRowSet->isBeforeFirst() && !_rxRowSet->isAfterLast() )
                transferDbValueToControl();
        }
    }

    if ( xOldField != m_xField )
    {
        Any aNewValue; aNewValue <<= m_xField;
        Any aOldValue; aOldValue <<= xOldField;
        sal_Int32 nHandle = PROPERTY_ID_BOUNDFIELD;
        OPropertySetHelper::fire(&nHandle, &aNewValue, &aOldValue, 1, sal_False);
    }
}

//------------------------------------------------------------------------------
void OBoundControlModel::disconnectDatabaseColumn( )
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::disconnectDatabaseColumn: not to be called with an external value binding!" );
    ::osl::MutexGuard aGuard( m_aMutex );

    // let derived classes react on this
    onDisconnectedDbColumn();

    if ( m_xField.is() )
    {
        m_xField->removePropertyChangeListener( PROPERTY_VALUE, this );
        resetField();
    }

    m_xCursor = NULL;
    m_bLoaded = sal_False;
}

//==============================================================================
// XLoadListener
//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::loaded( const EventObject& _rEvent ) throw(RuntimeException)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::loaded: we should never reach this with an external value binding!" );
    if ( hasExternalValueBinding() )
        return;

    // connect to the database column described by our SQL-binding-related properties
    Reference< XRowSet > xRowSet( _rEvent.Source, UNO_QUERY );
    DBG_ASSERT( xRowSet.is(), "OBoundControlModel::loaded: event source is no RowSet?!" );
    connectDatabaseColumn( xRowSet, false );
}


//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::unloaded( const com::sun::star::lang::EventObject& aEvent ) throw(RuntimeException)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::unloaded: we should never reach this with an external value binding!" );
}

//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::reloading( const com::sun::star::lang::EventObject& aEvent ) throw(RuntimeException)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::reloading: we should never reach this with an external value binding!" );
    if ( hasExternalValueBinding() )
        return;

    osl::MutexGuard aGuard(m_aMutex);
    m_bForwardValueChanges = sal_False;
}

//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::unloading(const com::sun::star::lang::EventObject& aEvent) throw(RuntimeException)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::unloading: we should never reach this with an external value binding!" );
    if ( hasExternalValueBinding() )
        return;

    // disconnect from the database column described by our SQL-binding-related properties
    disconnectDatabaseColumn();
}

//------------------------------------------------------------------------------
void SAL_CALL OBoundControlModel::reloaded( const EventObject& _rEvent ) throw(RuntimeException)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::reloaded: we should never reach this with an external value binding!" );
    if ( hasExternalValueBinding() )
        return;

    Reference< XRowSet > xRowSet( _rEvent.Source, UNO_QUERY );
    DBG_ASSERT( xRowSet.is(), "OBoundControlModel::reloaded: event source is no RowSet?!" );
    connectDatabaseColumn( xRowSet, true );
}

//------------------------------------------------------------------------------
void OBoundControlModel::setControlValue( const Any& _rValue )
{
    OSL_PRECOND( m_xAggregateFastSet.is() && m_xAggregateSet.is(),
        "OBoundControlModel::setControlValue: invalid aggregate !" );
    OSL_PRECOND( m_sValuePropertyName.getLength() || ( m_nValuePropertyAggregateHandle != -1 ),
        "OBoundControlModel::setControlValue: please override if you have own value property handling!" );

    m_bSettingAggregateState = sal_True;
    {
        // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease( m_aMutex );
        if ( ( m_nValuePropertyAggregateHandle != -1 ) && m_xAggregateFastSet.is() )
        {
            m_xAggregateFastSet->setFastPropertyValue( m_nValuePropertyAggregateHandle, _rValue );
        }
        else if ( m_sValuePropertyName.getLength() && m_xAggregateSet.is() )
        {
            m_xAggregateSet->setPropertyValue( m_sValuePropertyName, _rValue );
        }
    }
    m_bSettingAggregateState = sal_False;
}

//------------------------------------------------------------------------------
void OBoundControlModel::onConnectedExternalValue( )
{
}

//------------------------------------------------------------------------------
void OBoundControlModel::onDisconnectedExternalValue( )
{
}

//------------------------------------------------------------------------------
void OBoundControlModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::onConnectedDbColumn: how this? There's an external value binding!" );
}

//------------------------------------------------------------------------------
void OBoundControlModel::onDisconnectedDbColumn()
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::onDisconnectedDbColumn: how this? There's an external value binding!" );
}

// XReset
//-----------------------------------------------------------------------------
Any OBoundControlModel::getDefaultForReset() const
{
    return Any();
}

//-----------------------------------------------------------------------------
void OBoundControlModel::resetNoBroadcast()
{
    setControlValue( getDefaultForReset() );
}

//-----------------------------------------------------------------------------
void OBoundControlModel::addResetListener(const Reference<XResetListener>& l) throw (RuntimeException)
{
    m_aResetListeners.addInterface(l);
}

//-----------------------------------------------------------------------------
void OBoundControlModel::removeResetListener(const Reference<XResetListener>& l) throw (RuntimeException)
{
    m_aResetListeners.removeInterface(l);
}

//-----------------------------------------------------------------------------
void OBoundControlModel::reset() throw (RuntimeException)
{
    cppu::OInterfaceIteratorHelper aIter(m_aResetListeners);
    EventObject aResetEvent(static_cast<XWeak*>(this));
    sal_Bool bContinue = sal_True;
    while ( aIter.hasMoreElements() && bContinue )
        bContinue = static_cast< XResetListener* >( aIter.next() )->approveReset( aResetEvent );

    if (!bContinue)
        return;

    osl::ClearableMutexGuard aGuard(m_aMutex);

    sal_Bool bSimpleReset =
                        (   !m_xColumn.is()                     // no connection to a database column
                        ||  (   m_xCursor.is()                  // OR   we have an improperly positioned cursor
                            &&  (   m_xCursor->isAfterLast()
                                ||  m_xCursor->isBeforeFirst()
                                )
                            )
                        ||  hasExternalValueBinding()           // OR we have an external value binding
                        );

    if ( !bSimpleReset )
    {
        // The default values will be set if and only if the current value of the field which we're bound
        // to is NULL.
        // Else, the current field value should be refreshed
        // This behaviour is not completely ... "matured": What should happen if the field as well as the
        // control have a default value?

        sal_Bool bIsNull = sal_True;
        // we have to access the field content at least once to get a reliable result by XColumn::wasNull
        try
        {
            // TODO: this is expensive in the case of binary fields
            m_xColumn->getString();
            bIsNull = m_xColumn->wasNull();
        }
        catch(Exception&)
        {
            DBG_ERROR("OBoundControlModel::reset : XColumn::getString and wasNull are expected to always succeed !");
        }

        sal_Bool bNeedValueTransfer = sal_True;

        if ( bIsNull )
        {
            sal_Bool bIsNewRecord = sal_False;
            Reference<XPropertySet> xSet( m_xCursor, UNO_QUERY );
            if ( xSet.is() )
                xSet->getPropertyValue( PROPERTY_ISNEW ) >>= bIsNewRecord;

            if ( bIsNewRecord )
            {
                // reset the control to it's default
                resetNoBroadcast();
                // and immediately commit the changes to the DB column, to keep consistency
                commitControlValueToDbColumn( sal_True );

                bNeedValueTransfer = sal_False;
            }
        }

        if ( bNeedValueTransfer )
            transferDbValueToControl();
    }
    else
    {
        resetNoBroadcast();
        if ( hasExternalValueBinding() )
            transferControlValueToExternal();
    }

    aGuard.clear();

    cppu::OInterfaceIteratorHelper aIterDone(m_aResetListeners);
    while (aIterDone.hasMoreElements())
        reinterpret_cast<XResetListener*>(aIterDone.next())->resetted(aResetEvent);
}
// -----------------------------------------------------------------------------
void OBoundControlModel::setField( const Reference< XPropertySet>& _rxField,sal_Bool _bFire )
{
    DBG_ASSERT( !hasExternalValueBinding(), "OBoundControlModel::setField: We have an external value binding!" );

    // fire a property change event
    if ( m_xField != _rxField )
    {
        Any aOldValue; aOldValue <<= m_xField;
        m_xField = _rxField;
        if ( _bFire )
        {
            Any aNewValue; aNewValue <<= _rxField;
            sal_Int32 nHandle = PROPERTY_ID_BOUNDFIELD;
            OPropertySetHelper::fire(&nHandle, &aNewValue, &aOldValue, 1, sal_False);
        }
    }
}
//--------------------------------------------------------------------
sal_Bool OBoundControlModel::approveValueBinding( const Reference< XValueBinding >& _rxBinding )
{
    // reject everything. Derived classes need to override this if they want to
    // benefit from this data binding functionality
    return sal_False;
}

//--------------------------------------------------------------------
void OBoundControlModel::connectExternalValueBinding( const Reference< XValueBinding >& _rxBinding )
{
    OSL_PRECOND( _rxBinding.is(), "OBoundControlModel::connectExternalValueBinding: invalid binding instance!" );
    OSL_PRECOND( approveValueBinding( _rxBinding ), "OBoundControlModel::connectExternalValueBinding: binding is not approved!" );
    OSL_PRECOND( !hasExternalValueBinding( ), "OBoundControlModel::connectExternalValueBinding: precond not met (currently have a binding)!" );

    // Suspend being a load listener at our parent form. This is because
    // an external value binding overrules a possible database binding
    if ( isLoadListening() )
        stopLoadListening( );

    // TODO: if we're already connected to a db column, we should disconnect from it here,
    // shouldn't we?

    // remember this new binding
    m_xExternalBinding = _rxBinding;

    // add as value listener so we get notified when the value changes
    Reference< XModifyBroadcaster > xModifiable( m_xExternalBinding, UNO_QUERY );
    if ( xModifiable.is() )
        xModifiable->addModifyListener( this );

    // tell the derivee
    onConnectedExternalValue();

    // propagate our new value
    transferExternalValueToControl( );
}

//--------------------------------------------------------------------
void OBoundControlModel::disconnectExternalValueBinding( )
{
    // not listening at the binding anymore
    Reference< XModifyBroadcaster > xModifiable( m_xExternalBinding, UNO_QUERY );
    if ( xModifiable.is() )
        xModifiable->removeModifyListener( this );

    // no binding anymore
    m_xExternalBinding.clear();

    // be a load listener at our parent, again. This was suspended while we had
    // an external value binding in place.
    if ( m_xParent.is() )
        startLoadListening( );

    // TODO: anything to care for here? Changing values? Falling back to a
    // database binding if appropriate?

    // tell the derivee
    onDisconnectedExternalValue();
}

//--------------------------------------------------------------------
void SAL_CALL OBoundControlModel::setValueBinding( const Reference< XValueBinding >& _rxBinding ) throw (IncompatibleTypesException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OSL_PRECOND( m_bSupportsExternalBinding, "OBoundControlModel::setValueBinding: How did you reach this method?" );
        // the interface for this method should not have been exposed if we do not
        // support binding to external data

    if ( _rxBinding.is() && !approveValueBinding( _rxBinding ) )
    {
        throw IncompatibleTypesException(
            ::rtl::OUString::createFromAscii( "The value types supported by the binding cannot be used for exchanging data with this control." ),
            *this
        );
        // TODO: localize this error message
    }

    // disconnect from the old binding
    if ( hasExternalValueBinding() )
        disconnectExternalValueBinding( );

    // connect to the new binding
    if ( _rxBinding.is() )
        connectExternalValueBinding( _rxBinding );
}

//--------------------------------------------------------------------
Reference< XValueBinding > SAL_CALL OBoundControlModel::getValueBinding(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_PRECOND( m_bSupportsExternalBinding, "OBoundControlModel::getValueBinding: How did you reach this method?" );
        // the interface for this method should not have been exposed if we do not
        // support binding to external data

    return m_xExternalBinding;
}

//--------------------------------------------------------------------
void SAL_CALL OBoundControlModel::modified( const EventObject& _rEvent ) throw ( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OSL_PRECOND( hasExternalValueBinding(), "OBoundControlModel::modified: Where did this come from?" );
    if ( hasExternalValueBinding() && ( m_xExternalBinding == _rEvent.Source ) )
    {
        transferExternalValueToControl( );
    }
}

//--------------------------------------------------------------------
void OBoundControlModel::transferDbValueToControl( )
{
    setControlValue( translateDbColumnToControlValue() );
}

//------------------------------------------------------------------------------
void OBoundControlModel::transferExternalValueToControl( )
{
    try
    {
        setControlValue( translateExternalValueToControlValue( ) );
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "OBoundControlModel::transferExternalValueToControl: caught an exception!" );
    }
}

//------------------------------------------------------------------------------
void OBoundControlModel::transferControlValueToExternal( )
{
    OSL_PRECOND( m_bSupportsExternalBinding && hasExternalValueBinding(),
        "OBoundControlModel::transferControlValueToExternal: precondition not met!" );

    if ( m_xExternalBinding.is() )
    {
        try
        {
            m_xExternalBinding->setValue( translateControlValueToExternalValue( ) );
        }
        catch( const IncompatibleTypesException& )
        {
            OSL_ENSURE( sal_False, "OBoundControlModel::transferControlValueToExternal: could not commit the value (incomptible types)!" );
        }
        catch( const NoSupportException& )
        {
            OSL_ENSURE( sal_False, "OBoundControlModel::transferControlValueToExternal: could not commit the value (no support)!" );
        }
    }
}

// -----------------------------------------------------------------------------
Any OBoundControlModel::translateExternalValueToControlValue( )
{
    OSL_PRECOND( m_bSupportsExternalBinding && hasExternalValueBinding(),
        "OBoundControlModel::translateExternalValueToControlValue: precondition not met!" );

    // determine the type of our value property
    if ( m_sValuePropertyName.getLength() && m_xAggregateSet.is() )
    {
        Reference< XPropertySetInfo > xPropInfo( m_xAggregateSet->getPropertySetInfo( ) );
        if ( xPropInfo.is() )
        {
            Type aValuePropType = xPropInfo->getPropertyByName( m_sValuePropertyName ).Type;
            if ( m_xExternalBinding.is() && m_xExternalBinding->supportsType( aValuePropType ) )
                return m_xExternalBinding->getValue( aValuePropType );
        }
    }

    OSL_ENSURE( sal_False, "OBoundControlModel::translateExternalValueToControlValue: no default implementation available!" );
    return Any();
}

//------------------------------------------------------------------------------
Any OBoundControlModel::translateControlValueToExternalValue( )
{
    OSL_PRECOND( m_xAggregateFastSet.is() && m_xAggregateSet.is(),
        "OBoundControlModel::translateControlValueToExternalValue: invalid aggregate !" );
    OSL_PRECOND( m_sValuePropertyName.getLength() || ( m_nValuePropertyAggregateHandle != -1 ),
        "OBoundControlModel::translateControlValueToExternalValue: please override if you have own value property handling!" );

    // determine the current control value
    Any aControlValue;
    if ( ( m_nValuePropertyAggregateHandle != -1 ) && m_xAggregateFastSet.is() )
    {
        aControlValue = m_xAggregateFastSet->getFastPropertyValue( m_nValuePropertyAggregateHandle );
    }
    else if ( m_sValuePropertyName.getLength() && m_xAggregateSet.is() )
    {
        aControlValue = m_xAggregateSet->getPropertyValue( m_sValuePropertyName );
    }

    return aControlValue;
}

// -----------------------------------------------------------------------------

//.........................................................................
}
//... namespace frm .......................................................

