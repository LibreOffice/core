/*************************************************************************
 *
 *  $RCSfile: column.cxx,v $
 *
 *  $Revision: 1.45 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:02:01 $
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

#ifndef _DBA_COREAPI_COLUMN_HXX_
#include "column.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include <connectivity/sdbcx/VColumn.hxx>
#endif
#ifndef CONNECTIVITY_TABLEHELPER_HXX
#include <connectivity/TTableHelper.hxx>
#endif
#ifndef _DBACORE_DEFINITIONCOLUMN_HXX_
#include "definitioncolumn.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef DBA_CONTAINERMEDIATOR_HXX
#include "ContainerMediator.hxx"
#endif
#ifndef DBACORE_SDBCORETOOLS_HXX
#include "sdbcoretools.hxx"
#endif

#include <algorithm>

using namespace dbaccess;
using namespace connectivity;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

DBG_NAME(OColumn);

//============================================================
//= OColumn
//============================================================
//--------------------------------------------------------------------------
OColumn::OColumn()
        :OColumnBase(m_aMutex)
        , OPropertySetHelper(OColumnBase::rBHelper)
{
    DBG_CTOR(OColumn, NULL);
}

//--------------------------------------------------------------------------
OColumn::~OColumn()
{
    DBG_DTOR(OColumn, NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OColumn::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XPropertySet > *)0 ),
                           ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                           OColumnBase::getTypes());
    return aTypes.getTypes();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any OColumn::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OColumnBase::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XPropertySet * >( this ),
                    static_cast< XMultiPropertySet * >( this ));

    return aIface;
}

//--------------------------------------------------------------------------
void OColumn::acquire() throw()
{
    OColumnBase::acquire();
}

//--------------------------------------------------------------------------
void OColumn::release() throw()
{
    OColumnBase::release();
}

// ::com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OColumn::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OColumn");
}

//------------------------------------------------------------------------------
sal_Bool OColumn::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OColumn::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDBCX_COLUMN;
    return aSNS;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OColumn::disposing()
{
    OPropertySetHelper::disposing();
}

// com::sun::star::beans::XPropertySet
//------------------------------------------------------------------------------
Reference< XPropertySetInfo > OColumn::getPropertySetInfo() throw (RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

//------------------------------------------------------------------------------
void OColumn::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            rValue <<= m_sName;
            break;
    }
}

//------------------------------------------------------------------------------
sal_Bool OColumn::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sName);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OColumn::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            OSL_ENSURE(rValue.getValueType().equals(::getCppuType(static_cast< ::rtl::OUString* >(NULL))),
                "OColumn::setFastPropertyValue_NoBroadcast(NAME) : invalid value !");
            rValue >>= m_sName;
            break;
    }
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OColumn::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OColumn::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return 0;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OColumn::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_sName;
}
// -----------------------------------------------------------------------------
void SAL_CALL OColumn::setName( const ::rtl::OUString& _rName ) throw(::com::sun::star::uno::RuntimeException)
{
    m_sName = _rName;
}
// -----------------------------------------------------------------------------
void OColumn::fireValueChange(const ::connectivity::ORowSetValue& _rOldValue)
{
}
// -----------------------------------------------------------------------------
//============================================================
//= OColumnSettings
//============================================================
DBG_NAME( OColumnSettings )
//------------------------------------------------------------------------------
OColumnSettings::OColumnSettings()
    :m_bHidden(sal_False)
{
    DBG_CTOR( OColumnSettings, NULL );
}

//------------------------------------------------------------------------------
OColumnSettings::~OColumnSettings()
{
    DBG_DTOR( OColumnSettings, NULL );
}

//------------------------------------------------------------------------------
void OColumnSettings::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
            rValue = m_aAlignment;
            break;
        case PROPERTY_ID_NUMBERFORMAT:
            rValue = m_aFormatKey;
            break;
        case PROPERTY_ID_RELATIVEPOSITION:
            rValue = m_aRelativePosition;
            break;
        case PROPERTY_ID_WIDTH:
            rValue = m_aWidth;
            break;
        case PROPERTY_ID_HIDDEN:
            rValue.setValue(&m_bHidden, getBooleanCppuType());
            break;
        case PROPERTY_ID_CONTROLMODEL:
            rValue <<= m_xControlModel;
            break;
        case PROPERTY_ID_HELPTEXT:
            rValue = m_aHelpText;
            break;
        case PROPERTY_ID_CONTROLDEFAULT:
            rValue = m_aControlDefault;
            break;
    }
}

//------------------------------------------------------------------------------
sal_Bool OColumnSettings::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
            bModified = !uno_type_equalData(
                                const_cast< void* >( m_aAlignment.getValue() ), m_aAlignment.getValueType().getTypeLibType(),
                                const_cast< void* >( rValue.getValue() ), rValue.getValueType().getTypeLibType(),
                                cpp_queryInterface, cpp_release
                            );
            if ( bModified )
            {
                rConvertedValue = rValue;
                rOldValue = m_aAlignment;
            }
            break;
        case PROPERTY_ID_WIDTH:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aWidth,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_HIDDEN:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bHidden);
            break;
        case PROPERTY_ID_RELATIVEPOSITION:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aRelativePosition,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_NUMBERFORMAT:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aFormatKey,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_CONTROLMODEL:
        {
            Reference< XPropertySet > xTest;
            if (!::cppu::extractInterface(xTest, rValue))
                throw IllegalArgumentException();
            if (xTest.get() != m_xControlModel.get())
            {
                bModified = sal_True;
                rOldValue <<= m_xControlModel;
                rConvertedValue <<= rValue;
            }
        }
        break;
        case PROPERTY_ID_HELPTEXT:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aHelpText,
                ::getCppuType(static_cast< ::rtl::OUString* >(NULL)));
            break;
        case PROPERTY_ID_CONTROLDEFAULT:
            if ( bModified = !::comphelper::compare(rValue,m_aControlDefault) )
            {
                rConvertedValue = rValue;
                rOldValue = m_aControlDefault;
            }
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OColumnSettings::setFastPropertyValue_NoBroadcast(
                                            sal_Int32 nHandle,
                                            const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
            {
                if ( rValue.hasValue() )
                {
                    sal_Int32 nAlign = 0;
                    sal_Bool bSuccess =
                    // copy the data from the to-be-set value
                    uno_type_assignData(
                        static_cast< void* >( &nAlign ),    ::getCppuType(static_cast< sal_Int32* >(NULL)).getTypeLibType(),
                        const_cast< void* >( rValue.getValue() ),   rValue.getValueType().getTypeLibType(),
                        cpp_queryInterface,
                        cpp_acquire, cpp_release );

                    OSL_ENSURE( bSuccess,
                        "OPropertyStateContainer::setFastPropertyValue_NoBroadcast : ooops .... the value could not be assigned!");
                    if ( bSuccess )
                        m_aAlignment <<= nAlign;
                }
                else
                    m_aAlignment = rValue;
            }

            break;
        case PROPERTY_ID_WIDTH:
            OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(WIDTH) : invalid value !");
            m_aWidth = rValue;
            break;
        case PROPERTY_ID_NUMBERFORMAT:
            OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(NUMBERFORMAT) : invalid value !");
            m_aFormatKey = rValue;
            break;
        case PROPERTY_ID_RELATIVEPOSITION:
            OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(ID_RELATIVEPOSITION) : invalid value !");
            m_aRelativePosition = rValue;
            break;
        case PROPERTY_ID_HIDDEN:
            OSL_ENSURE(rValue.getValueType().equals(::getBooleanCppuType()),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(HIDDEN) : invalid value !");
            m_bHidden = ::comphelper::getBOOL(rValue);
            break;
        case PROPERTY_ID_HELPTEXT:
            OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< ::rtl::OUString* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(ID_RELATIVEPOSITION) : invalid value !");
            m_aHelpText = rValue;
            break;
        case PROPERTY_ID_CONTROLDEFAULT:
            m_aControlDefault = rValue;
            break;
    }
}


//------------------------------------------------------------------------------
sal_Bool OColumnSettings::isDefaulted() const
{
    return  !m_aAlignment.hasValue()
        &&  !m_aWidth.hasValue()
        &&  !m_aFormatKey.hasValue()
        &&  !m_aRelativePosition.hasValue()
        &&  !m_aHelpText.hasValue()
        &&  !m_aControlDefault.hasValue()
        &&  !m_bHidden;
}


//============================================================
//= OColumns
//============================================================
DBG_NAME(OColumns);

//--------------------------------------------------------------------------
OColumns::OColumns(::cppu::OWeakObject& _rParent,
                   ::osl::Mutex& _rMutex,
                   sal_Bool _bCaseSensitive,const ::std::vector< ::rtl::OUString> &_rVector,
                   IColumnFactory* _pColFactory,
                   ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
                   sal_Bool _bAddColumn,
                   sal_Bool _bDropColumn,
                   sal_Bool _bUseHardRef)
                   : OColumns_BASE(_rParent,_bCaseSensitive,_rMutex,_rVector,_bUseHardRef)
    ,m_bInitialized(sal_False)
    ,m_bAddColumn(_bAddColumn)
    ,m_bDropColumn(_bDropColumn)
    ,m_xDrvColumns(NULL)
    ,m_pColFactoryImpl(_pColFactory)
    ,m_pRefreshColumns(_pRefresh)
    ,m_pMediator(NULL)
{
    DBG_CTOR(OColumns, NULL);
}
// -------------------------------------------------------------------------
OColumns::OColumns(::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxDrvColumns,
        sal_Bool _bCaseSensitive,const ::std::vector< ::rtl::OUString> &_rVector,
        IColumnFactory* _pColFactory,
        ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
        sal_Bool _bAddColumn,
        sal_Bool _bDropColumn,
        sal_Bool _bUseHardRef)
       : OColumns_BASE(_rParent,_bCaseSensitive,_rMutex,_rVector,_bUseHardRef)
    ,m_bInitialized(sal_False)
    ,m_bAddColumn(_bAddColumn)
    ,m_bDropColumn(_bDropColumn)
    ,m_xDrvColumns(_rxDrvColumns)
    ,m_pColFactoryImpl(_pColFactory)
    ,m_pRefreshColumns(_pRefresh)
    ,m_pMediator(NULL)
{
    DBG_CTOR(OColumns, NULL);
}
//--------------------------------------------------------------------------
OColumns::~OColumns()
{
    DBG_DTOR(OColumns, NULL);
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OColumns::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OColumns");
}

//------------------------------------------------------------------------------
sal_Bool OColumns::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OColumns::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDBCX_CONTAINER;
    return aSNS;
}

//------------------------------------------------------------------
void OColumns::append( const ::rtl::OUString& _rName, OColumn* _pColumn )
{
    MutexGuard aGuard(m_rMutex);

    OSL_ENSURE( _pColumn, "OColumns::append: invalid column!" );
    OSL_ENSURE( !m_pElements->exists( _rName ),"OColumns::append: Column already exists");

    _pColumn->m_sName = _rName;

    // now really insert the column
    insertElement( _rName, _pColumn );
}

//------------------------------------------------------------------
void OColumns::clearColumns()
{
    MutexGuard aGuard(m_rMutex);
    disposing();
}

// -----------------------------------------------------------------------------
void SAL_CALL OColumns::disposing(void)
{
    MutexGuard aGuard(m_rMutex);
    m_xDrvColumns = NULL;
    m_pMediator = NULL;
    m_pColFactoryImpl = NULL;
    OColumns_BASE::disposing();
}

// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
    if (m_pRefreshColumns)
        m_pRefreshColumns->refreshColumns();
}

// -------------------------------------------------------------------------
Reference< XNamed > OColumns::createObject(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(m_pColFactoryImpl, "OColumns::createObject: no column factory!");

    Reference< XNamed > xRet;
    if ( m_pColFactoryImpl )
    {
        xRet = m_pColFactoryImpl->createColumn(_rName);
        Reference<XChild> xChild(xRet,UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(*this);
    }

    Reference<XPropertySet> xDest(xRet,UNO_QUERY);
    if ( m_pMediator )
        m_pMediator->notifyElementCreated(_rName,xDest);

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumns::createEmptyObject()
{
    if ( m_pColFactoryImpl )
    {
        Reference<XPropertySet> xRet = m_pColFactoryImpl->createEmptyObject();
        Reference<XChild> xChild(xRet,UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(*this);
        return xRet;
    }
    else
        return Reference< XPropertySet >();
}
// -------------------------------------------------------------------------
Any SAL_CALL OColumns::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet;
    if(m_xDrvColumns.is())
    {
        aRet = m_xDrvColumns->queryInterface(rType);
        if ( aRet.hasValue() )
            aRet = OColumns_BASE::queryInterface( rType);
        if ( !aRet.hasValue() )
            aRet = TXChild::queryInterface( rType);
        return aRet;
    }
    else if(!m_pTable || (m_pTable && !m_pTable->isNew()))
    {
        if(!m_bAddColumn    && rType == getCppuType( (Reference<XAppend>*)0))
            return Any();
        if(!m_bDropColumn   && rType == getCppuType( (Reference<XDrop>*)0))
            return Any();
    }

    return OColumns_BASE::queryInterface( rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OColumns::getTypes(  ) throw(RuntimeException)
{
    sal_Bool bAppendFound = sal_False,bDropFound = sal_False;

    sal_Int32 nSize = 0;
    Type aAppendType = getCppuType( (Reference<XAppend>*)0);
    Type aDropType   = getCppuType( (Reference<XDrop>*)0);
    if(m_xDrvColumns.is())
    {
        Reference<XTypeProvider> xTypes(m_xDrvColumns,UNO_QUERY);
        Sequence< Type > aTypes(xTypes->getTypes());

        Sequence< Type > aSecTypes(OColumns_BASE::getTypes());


        const Type* pBegin = aTypes.getConstArray();
        const Type* pEnd = pBegin + aTypes.getLength();
        for (;pBegin != pEnd ; ++pBegin)
        {
            if(aAppendType == *pBegin)
                bAppendFound = sal_True;
            else if(aDropType == *pBegin)
                bDropFound = sal_True;
        }
        nSize = (bDropFound ? (bAppendFound ? 0 : 1) : (bAppendFound ? 1 : 2));
    }
    else
    {
        nSize = ((m_pTable && m_pTable->isNew()) ? 0 :
                    ((m_bDropColumn ?
                        (m_bAddColumn ? 0 : 1) : (m_bAddColumn ? 1 : 2))));
        bDropFound      = (m_pTable && m_pTable->isNew()) || m_bDropColumn;
        bAppendFound    = (m_pTable && m_pTable->isNew()) || m_bAddColumn;
    }
    Sequence< Type > aTypes(::comphelper::concatSequences(OColumns_BASE::getTypes(),TXChild::getTypes()));
    Sequence< Type > aRet(aTypes.getLength() - nSize);

    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(sal_Int32 i=0;pBegin != pEnd ;++pBegin)
    {
        if(*pBegin != aAppendType && *pBegin != aDropType)
            aRet.getArray()[i++] = *pBegin;
        else if(bDropFound && *pBegin == aDropType)
            aRet.getArray()[i++] = *pBegin;
        else if(bAppendFound && *pBegin == aAppendType)
            aRet.getArray()[i++] = *pBegin;
    }
    return aRet;
}
// -------------------------------------------------------------------------
// XAppend
void OColumns::appendObject( const Reference< XPropertySet >& descriptor )
{
    Reference<XAppend> xAppend(m_xDrvColumns,UNO_QUERY);
    if(xAppend.is())
    {
        xAppend->appendByDescriptor(descriptor);
    }
    else if(m_pTable && !m_pTable->isNew() && m_bAddColumn)
    {
        OColumns_BASE::appendObject(descriptor);
    }
    else if(m_pTable && !m_pTable->isNew() && !m_bAddColumn)
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_COLUMN_ADD),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,Any());
    ::dbaccess::notifyDataSourceModified(m_xParent,sal_True);
}
// -------------------------------------------------------------------------
// XDrop
void OColumns::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    Reference<XDrop> xDrop(m_xDrvColumns,UNO_QUERY);
    if(xDrop.is())
    {
        xDrop->dropByName(_sElementName);
    }
    else if(m_pTable && !m_pTable->isNew() && m_bDropColumn)
    {
        OColumns_BASE::dropObject(_nPos,_sElementName);
    }
    else if(m_pTable && !m_pTable->isNew() && !m_bDropColumn)
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_COLUMN_DROP),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,Any());
    if ( m_pColFactoryImpl )
        m_pColFactoryImpl->columnDropped(_sElementName);
    ::dbaccess::notifyDataSourceModified(m_xParent,sal_True);
}
// -------------------------------------------------------------------------
Reference< XNamed > OColumns::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    Reference<XPropertySet> xProp = createEmptyObject();
    Reference< XNamed > xName(xProp,UNO_QUERY);
    OSL_ENSURE(xName.is(),"Must be a XName interface here !");
    if(xProp.is())
        ::comphelper::copyProperties(_xDescriptor,xProp);
    if ( m_pColFactoryImpl )
        m_pColFactoryImpl->columnCloned(xProp);
    return xName;
}
// -----------------------------------------------------------------------------

Reference< XInterface > SAL_CALL OColumns::getParent(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return m_xParent;
}
// -----------------------------------------------------------------------------
void SAL_CALL OColumns::setParent( const Reference< XInterface >& _xParent ) throw (NoSupportException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    m_xParent = _xParent;
}
// -----------------------------------------------------------------------------
