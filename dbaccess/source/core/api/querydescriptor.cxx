/*************************************************************************
 *
 *  $RCSfile: querydescriptor.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-05 16:37:09 $
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

#ifndef _DBA_COREAPI_QUERYDESCRIPTOR_HXX_
#include "querydescriptor.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _DBACORE_DEFINITIONCOLUMN_HXX_
#include "definitioncolumn.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::comphelper;
using namespace ::osl;
using namespace ::cppu;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OQueryDescriptor
//==========================================================================
DBG_NAME(OQueryDescriptor)
//--------------------------------------------------------------------------
OQueryDescriptor::OQueryDescriptor()
    : ODataSettings(m_aBHelper)
    ,OQueryDescriptor_Base(m_aMutex,*this)
{
    DBG_CTOR(OQueryDescriptor,NULL);
    registerProperties();
    ODataSettings::registerProperties(this);
}

//--------------------------------------------------------------------------
OQueryDescriptor::OQueryDescriptor(const Reference< XPropertySet >& _rxCommandDefinition)
    :OQueryDescriptor_Base(m_aMutex,*this)
    ,ODataSettings(m_aBHelper)
{
    DBG_CTOR(OQueryDescriptor,NULL);
    registerProperties();
    ODataSettings::registerProperties(this);

    osl_incrementInterlockedCount(&m_refCount);

    OSL_ENSURE(_rxCommandDefinition.is(), "OQueryDescriptor_Base::OQueryDescriptor_Base : invalid source property set !");
    try
    {
        ::comphelper::copyProperties(_rxCommandDefinition,this);
    }
    catch(Exception&)
    {
        OSL_ENSURE(sal_False, "OQueryDescriptor_Base::OQueryDescriptor_Base: caught an exception!");
    }
    osl_decrementInterlockedCount(&m_refCount);
}

//--------------------------------------------------------------------------
OQueryDescriptor::OQueryDescriptor(const OQueryDescriptor_Base& _rSource)
    :OQueryDescriptor_Base(_rSource,*this)
    ,ODataSettings(m_aBHelper)
{
    DBG_CTOR(OQueryDescriptor,NULL);
    registerProperties();
    ODataSettings::registerProperties(this);
}
// -----------------------------------------------------------------------------
IMPLEMENT_TYPEPROVIDER2(OQueryDescriptor,OQueryDescriptor_Base,ODataSettings);
IMPLEMENT_FORWARD_XINTERFACE3( OQueryDescriptor,OWeakObject,OQueryDescriptor_Base,ODataSettings)
//--------------------------------------------------------------------------
void OQueryDescriptor::registerProperties()
{
    // the properties which OCommandBase supplies (it has no own registration, as it's not derived from
    // a OPropertyStateContainer)
    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND|PropertyAttribute::CONSTRAINED,
                    &m_sElementName, ::getCppuType(&m_sElementName));

    registerProperty(PROPERTY_COMMAND, PROPERTY_ID_COMMAND, PropertyAttribute::BOUND,
                    &m_sCommand, ::getCppuType(&m_sCommand));

    registerProperty(PROPERTY_USE_ESCAPE_PROCESSING, PROPERTY_ID_USE_ESCAPE_PROCESSING, PropertyAttribute::BOUND,
                    &m_bEscapeProcessing, ::getBooleanCppuType());

    registerProperty(PROPERTY_UPDATE_TABLENAME, PROPERTY_ID_UPDATE_TABLENAME, PropertyAttribute::BOUND,
                    &m_sUpdateTableName, ::getCppuType(&m_sUpdateTableName));

    registerProperty(PROPERTY_UPDATE_SCHEMANAME, PROPERTY_ID_UPDATE_SCHEMANAME, PropertyAttribute::BOUND,
                    &m_sUpdateSchemaName, ::getCppuType(&m_sUpdateSchemaName));

    registerProperty(PROPERTY_UPDATE_CATALOGNAME, PROPERTY_ID_UPDATE_CATALOGNAME, PropertyAttribute::BOUND,
                    &m_sUpdateCatalogName, ::getCppuType(&m_sUpdateCatalogName));

    registerProperty(PROPERTY_LAYOUTINFORMATION, PROPERTY_ID_LAYOUTINFORMATION, PropertyAttribute::BOUND,
                    &m_aLayoutInformation, ::getCppuType(&m_aLayoutInformation));
}
// -----------------------------------------------------------------------------
//--------------------------------------------------------------------------
Reference< XPropertySetInfo > SAL_CALL OQueryDescriptor::getPropertySetInfo(  ) throw(RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OQueryDescriptor::getInfoHelper()
{
    return *getArrayHelper();
}
//--------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OQueryDescriptor::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

// -----------------------------------------------------------------------------
DBG_NAME(OQueryDescriptor_Base);
//--------------------------------------------------------------------------
OQueryDescriptor_Base::OQueryDescriptor_Base(::osl::Mutex&  _rMutex,::cppu::OWeakObject& _rMySelf)
    :m_bColumnsOutOfDate(sal_True)
    ,m_rMutex(_rMutex)
{
    DBG_CTOR(OQueryDescriptor_Base,NULL);
    m_pColumns = new OColumns(_rMySelf, m_rMutex, sal_True,::std::vector< ::rtl::OUString>(), this,this);
}
//--------------------------------------------------------------------------
OQueryDescriptor_Base::OQueryDescriptor_Base(const OQueryDescriptor_Base& _rSource,::cppu::OWeakObject& _rMySelf)
    :m_bColumnsOutOfDate(sal_True)
    ,m_rMutex(_rSource.m_rMutex)
{
    DBG_CTOR(OQueryDescriptor_Base,NULL);
    m_pColumns = new OColumns(_rMySelf, m_rMutex, sal_True,::std::vector< ::rtl::OUString>(), this,this);

    m_sCommand = _rSource.m_sCommand;
    m_bEscapeProcessing = _rSource.m_bEscapeProcessing;
    m_sUpdateTableName = _rSource.m_sUpdateTableName;
    m_sUpdateSchemaName = _rSource.m_sUpdateSchemaName;
    m_sUpdateCatalogName = _rSource.m_sUpdateCatalogName;
    m_aLayoutInformation = _rSource.m_aLayoutInformation;
}

//--------------------------------------------------------------------------
OQueryDescriptor_Base::~OQueryDescriptor_Base()
{
    if ( m_pColumns )
    {
        m_pColumns->acquire();
        m_pColumns->disposing();
        delete m_pColumns;
    }

    DBG_DTOR(OQueryDescriptor_Base,NULL);
}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL OQueryDescriptor_Base::getSomething( const Sequence< sal_Int8 >& _rIdentifier ) throw(RuntimeException)
{
    if (_rIdentifier.getLength() != 16)
        return NULL;

    if (0 == rtl_compareMemory(getImplementationId().getConstArray(),  _rIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return NULL;
}
//--------------------------------------------------------------------------
IMPLEMENT_IMPLEMENTATION_ID(OQueryDescriptor_Base)
//--------------------------------------------------------------------------
void OQueryDescriptor_Base::setColumnsOutOfDate( sal_Bool _bOutOfDate )
{
    m_bColumnsOutOfDate = _bOutOfDate;
    if ( !m_bColumnsOutOfDate )
        m_pColumns->setInitialized();
}

//--------------------------------------------------------------------------
void OQueryDescriptor_Base::implAppendColumn( const ::rtl::OUString& _rName, OColumn* _pColumn )
{
    m_pColumns->append( _rName, _pColumn );
}

//--------------------------------------------------------------------------
void OQueryDescriptor_Base::clearColumns( )
{
    m_pColumns->clearColumns();

    setColumnsOutOfDate();
}

//--------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL OQueryDescriptor_Base::getColumns( ) throw (RuntimeException)
{
    MutexGuard aGuard(m_rMutex);

    Reference< XNameAccess > xColumns;
    if ( m_bEscapeProcessing )
    {
        if ( isColumnsOutOfDate() )
        {
            // load the columns
            refreshColumns();

            setColumnsOutOfDate( sal_False );
            m_pColumns->setInitialized();
        }
        xColumns = m_pColumns;
    }
    return xColumns;
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryDescriptor_Base::getImplementationName(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.sdb.OQueryDescriptor");
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OQueryDescriptor_Base::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OQueryDescriptor_Base::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(2);
    aSupported.getArray()[0] = SERVICE_SDB_DATASETTINGS;
    aSupported.getArray()[1] = SERVICE_SDB_QUERYDESCRIPTOR;
    return aSupported;
}
//--------------------------------------------------------------------------
void OQueryDescriptor_Base::disposeColumns()
{
    m_pColumns->disposing();
}

// -----------------------------------------------------------------------------
Reference< XPropertySet > OQueryDescriptor_Base::createEmptyObject()
{
    return NULL;
}

// -----------------------------------------------------------------------------
void OQueryDescriptor_Base::rebuildColumns( )
{
}

// -----------------------------------------------------------------------------
void OQueryDescriptor_Base::refreshColumns()
{
    MutexGuard aGuard(m_rMutex);

    // clear the current columns
    clearColumns();

    // do the real rebuild
    rebuildColumns();
}

//------------------------------------------------------------------------------
OColumn* OQueryDescriptor_Base::createColumn(const ::rtl::OUString& _rName) const
{
    return new OTableColumn(_rName);
}
// -----------------------------------------------------------------------------
//........................................................................
}   // namespace dbaccess
//........................................................................


