/*************************************************************************
 *
 *  $RCSfile: query.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 13:18:24 $
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

#ifndef _DBA_COREAPI_QUERY_HXX_
#include "query.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#include "registryhelper.hxx"

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_AGGREGATION_HXX_
#include <comphelper/propagg.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

using namespace dbaccess;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::registry;
using namespace ::comphelper;
using namespace ::osl;
using namespace ::cppu;

#define AGG_PROPERTY(handle, propname_out)  \
    static_cast< ::comphelper::OPropertyArrayAggregationHelper* >(const_cast< OQuery_LINUX* >(this)->getArrayHelper())->fillAggregatePropertyInfoByHandle(&propname_out, NULL, handle)

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OQuery_LINUX
//==========================================================================
DBG_NAME(OQuery_LINUX)
//--------------------------------------------------------------------------
OQuery_LINUX::OQuery_LINUX(const Reference< XPropertySet >& _rxCommandDefinition, const Reference< XConnection >& _rxConn)
    :OConfigurationFlushable(m_aMutex)
    ,OQueryDescriptor(_rxCommandDefinition)
    ,m_bColumnsOutOfDate(sal_True)
    ,m_bCaseSensitiv(sal_True)
    ,m_xCommandDefinition(_rxCommandDefinition)
    ,m_eDoingCurrently(NONE)
    ,m_xConnection(_rxConn)
{
    DBG_CTOR(OQuery_LINUX, NULL);

    DBG_ASSERT(m_xCommandDefinition.is(), "OQuery_LINUX::OQuery_LINUX : invalid CommandDefinition object !");
    if (m_xCommandDefinition.is())
    {
        m_xCommandDefinition->addPropertyChangeListener(::rtl::OUString(), this);

        // TODO : be a listener on the configuration node which is responsible for my properties not belonging
        // to the CommandDefinition
    }
    DBG_ASSERT(m_xConnection.is(), "OQuery_LINUX::OQuery_LINUX : invalid connection !");
}

//--------------------------------------------------------------------------
OQuery_LINUX::~OQuery_LINUX()
{
    DBG_DTOR(OQuery_LINUX, NULL);
}

// XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > SAL_CALL OQuery_LINUX::getTypes() throw (RuntimeException)
{
    return ::comphelper::concatSequences(OQueryDescriptor::getTypes(), OQuery_Base::getTypes(), OConfigurationFlushable::getTypes());
}

// XInterface
//--------------------------------------------------------------------------
Any SAL_CALL OQuery_LINUX::queryInterface( const Type& _rType ) throw(RuntimeException)
{
    Any aReturn = OQuery_Base::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = OQueryDescriptor::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = OConfigurationFlushable::queryInterface(_rType);
    return aReturn;
}

// XColumnsSupplier
//--------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL OQuery_LINUX::getColumns(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (m_bColumnsOutOfDate)
    {
        m_aColumns.clearColumns();

        // fill the columns with columns from teh statement
        try
        {
            Reference< XStatement > xStmt = m_xConnection->createStatement();
            OSL_ENSURE(xStmt.is(),"No Statement created!");
            if(xStmt.is())
            {
                Reference< XColumnsSupplier > xRs(xStmt->executeQuery(m_sCommand),UNO_QUERY);
                OSL_ENSURE(xRs.is(),"No Resultset created!");
                if(xRs.is())
                {
                    Reference< XNameAccess > xColumns = xRs->getColumns();
                    if(xColumns.is())
                    {
                        Sequence< ::rtl::OUString> aNames = xColumns->getElementNames();
                        const ::rtl::OUString* pBegin = aNames.getConstArray();
                        const ::rtl::OUString* pEnd   = pBegin + aNames.getLength();
                        for(;pBegin != pEnd;++pBegin)
                        {
                            ODescriptorColumn* pColumn = new ODescriptorColumn(*pBegin);
                            Reference<XPropertySet> xSet = pColumn;
                            Reference<XPropertySet> xSource;
                            ::cppu::extractInterface(xSource,xColumns->getByName(*pBegin));
                            ::comphelper::copyProperties(xSource,xSet);
                            m_aColumns.append(*pBegin,pColumn);
                        }
                    }
                    ::comphelper::disposeComponent(xRs);
                }
                ::comphelper::disposeComponent(xStmt);
            }

            m_bColumnsOutOfDate = sal_False;
            m_aColumns.setInitialized();
        }
        catch(SQLException&)
        {
        }
    }
    return &m_aColumns;
}

// XServiceInfo
//--------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO3(OQuery_LINUX, "com.sun.star.sdb.dbaccess.OQuery", SERVICE_SDB_DATASETTINGS, SERVICE_SDB_QUERY, SERVICE_SDB_QUERYDEFINITION)

// ::com::sun::star::beans::XPropertyChangeListener
//--------------------------------------------------------------------------
void SAL_CALL OQuery_LINUX::propertyChange( const PropertyChangeEvent& _rSource ) throw(RuntimeException)
{
    sal_Int32 nOwnHandle = -1;
    {
        MutexGuard aGuard(m_aMutex);

        DBG_ASSERT(_rSource.Source.get() == Reference< XInterface >(m_xCommandDefinition, UNO_QUERY).get(),
            "OQuery_LINUX::propertyChange : where did this call come from ?");

        if (m_eDoingCurrently == SETTING_PROPERTIES)
            // we're setting the property ourself, so we will do the neccessary notifications later
            return;

        // forward this to our own member holding a copy of the property value
        if (getArrayHelper()->hasPropertyByName(_rSource.PropertyName))
        {
            Property aOwnProp = getArrayHelper()->getPropertyByName(_rSource.PropertyName);
            nOwnHandle = aOwnProp.Handle;
            OQueryDescriptor::setFastPropertyValue_NoBroadcast(nOwnHandle, _rSource.NewValue);
                // don't use our own setFastPropertyValue_NoBroadcast, this would forward it to the CommandSettings,
                // again
                // and don't use the "real" setPropertyValue, this is to expensive and not sure to succeed
        }
        else
        {
            DBG_ERROR("OQuery_LINUX::propertyChange : my CommandDefinition has more properties than I do !");
        }
    }

    fire(&nOwnHandle, &_rSource.NewValue, &_rSource.OldValue, 1, sal_False);
}

//--------------------------------------------------------------------------
void SAL_CALL OQuery_LINUX::disposing( const EventObject& _rSource )
{
    MutexGuard aGuard(m_aMutex);

    DBG_ASSERT(_rSource.Source.get() == Reference< XInterface >(m_xCommandDefinition, UNO_QUERY).get(),
        "OQuery_LINUX::disposing : where did this call come from ?");

    m_xCommandDefinition->removePropertyChangeListener(::rtl::OUString(), this);
    m_xCommandDefinition = NULL;
}

// XDataDescriptorFactory
//--------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL OQuery_LINUX::createDataDescriptor(  ) throw(RuntimeException)
{
    return new OQueryDescriptor(*this);
}

// OQueryDescriptor
//--------------------------------------------------------------------------
void OQuery_LINUX::initializeFrom(const OConfigurationNode& _rConfigLocation)
{
    OQueryDescriptor::initializeFrom(_rConfigLocation);

    m_aConfigurationNode = _rConfigLocation.cloneAsRoot();
}

// OConfigurationFlushable
//--------------------------------------------------------------------------
void OQuery_LINUX::flush_NoBroadcast_NoCommit()
{
    if (!m_aConfigurationNode.isValid())
        throw DisposedException();
    OQueryDescriptor::storeTo(m_aConfigurationNode);
}

// pseudo-XComponent
//--------------------------------------------------------------------------
void SAL_CALL OQuery_LINUX::dispose()
{
    MutexGuard aGuard(m_aMutex);
    if (m_xCommandDefinition.is())
    {
        m_xCommandDefinition->removePropertyChangeListener(::rtl::OUString(), this);
        m_xCommandDefinition = NULL;
    }
    m_aConfigurationNode.clear();
    OQueryDescriptor::dispose();
}

//--------------------------------------------------------------------------
void OQuery_LINUX::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw (Exception)
{
    OQueryDescriptor::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    ::rtl::OUString sAggPropName;
    sal_Int16 nAttr = 0;
    if (getInfoHelper().fillPropertyMembersByHandle(&sAggPropName,&nAttr,_nHandle))
    {   // the base class holds the property values itself, but we have to forward this to our CommandDefinition
        m_eDoingCurrently = SETTING_PROPERTIES;
        OAutoActionReset(this);
        m_xCommandDefinition->setPropertyValue(sAggPropName, _rValue);
    }
}

//--------------------------------------------------------------------------
Reference< XPropertySetInfo > SAL_CALL OQuery_LINUX::getPropertySetInfo(  ) throw(RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OQuery_LINUX::getInfoHelper()
{
    return *getArrayHelper();
}

//--------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OQuery::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    // our own props
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

//........................................................................
}   // namespace dbaccess
//........................................................................

