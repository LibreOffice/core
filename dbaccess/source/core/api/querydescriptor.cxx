/*************************************************************************
 *
 *  $RCSfile: querydescriptor.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-23 13:18:50 $
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
#ifndef _DBA_CORE_REGISTRYHELPER_HXX_
#include "registryhelper.hxx"
#endif
#ifndef _DBA_CONFIGNODE_HXX_
#include "confignode.hxx"
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

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::container;
using namespace ::comphelper;
using namespace ::osl;
using namespace ::cppu;
using namespace comphelper;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OQueryDescriptor
//==========================================================================
//--------------------------------------------------------------------------
void OQueryDescriptor::registerProperties()
{
    // the properties which OCommandBase supplies (it has no own registration, as it's not derived from
    // a OPropertyContainer)
    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND,
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

    registerProperty(PROPERTY_LAYOUTINFORMATION, PROPERTY_ID_LAYOUTINFORMATION, 0,
                    &m_aLayoutInformation, ::getCppuType(&m_aLayoutInformation));
}

//--------------------------------------------------------------------------
OQueryDescriptor::OQueryDescriptor()
    :ODataSettings(m_aBHelper)
    ,m_aColumns(*this, m_aMutex, sal_True,::std::vector< ::rtl::OUString>())
{
    registerProperties();
}

//--------------------------------------------------------------------------
OQueryDescriptor::OQueryDescriptor(const ::com::sun::star::uno::Reference< XPropertySet >& _rxCommandDefinition)
    :ODataSettings(m_aBHelper)
    ,m_aColumns(*this, m_aMutex, sal_True,::std::vector< ::rtl::OUString>())
{
    registerProperties();

    OSL_ENSURE(_rxCommandDefinition.is(), "OQueryDescriptor::OQueryDescriptor : invalid source property set !");
    try
    {
        _rxCommandDefinition->getPropertyValue(PROPERTY_NAME)                   >>= m_sElementName;
        _rxCommandDefinition->getPropertyValue(PROPERTY_COMMAND)                >>= m_sCommand;
        _rxCommandDefinition->getPropertyValue(PROPERTY_UPDATE_TABLENAME)       >>= m_sUpdateTableName;
        _rxCommandDefinition->getPropertyValue(PROPERTY_UPDATE_SCHEMANAME)      >>= m_sUpdateSchemaName;
        _rxCommandDefinition->getPropertyValue(PROPERTY_UPDATE_CATALOGNAME)     >>= m_sUpdateCatalogName;
        _rxCommandDefinition->getPropertyValue(PROPERTY_LAYOUTINFORMATION)      >>= m_aLayoutInformation;

        m_bEscapeProcessing = ::cppu::any2bool(_rxCommandDefinition->getPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING));
    }
    catch(Exception&)
    {
        OSL_ASSERT(0);
    }
}

//--------------------------------------------------------------------------
OQueryDescriptor::OQueryDescriptor(const OQueryDescriptor& _rSource)
    :ODataSettings(_rSource, m_aBHelper)
    ,m_aColumns(*this, m_aMutex, sal_True,::std::vector< ::rtl::OUString>())
{
    registerProperties();

    m_sCommand = _rSource.m_sCommand;
    m_bEscapeProcessing = _rSource.m_bEscapeProcessing;
    m_sUpdateTableName = _rSource.m_sUpdateTableName;
    m_sUpdateSchemaName = _rSource.m_sUpdateSchemaName;
    m_sUpdateCatalogName = _rSource.m_sUpdateCatalogName;
    m_aLayoutInformation = _rSource.m_aLayoutInformation;

    // immediately read the UI of the columns : we may live much longer than _rSource and it's column's config node,
    // so we can't just remember this node and read when needed, we have to do it here and now
//  OConfigurationNode aColumnUINode = _rSource.m_aColumns.getUILocation();
//  if (aColumnUINode.isValid())
//      m_aColumns.loadSettings(aColumnUINode, this);
    // TODO: cloning of the column information. Since a column container does not know anything about it's columns
    // config location anymore, we probably need to clone the columns explicitly (instead of re-reading them)
}

//--------------------------------------------------------------------------
OQueryDescriptor::~OQueryDescriptor()
{
}

//--------------------------------------------------------------------------
Sequence< Type > SAL_CALL OQueryDescriptor::getTypes() throw (RuntimeException)
{
    return ::comphelper::concatSequences(OQueryDescriptor_Base::getTypes(), ODataSettings::getTypes());
}

//--------------------------------------------------------------------------
sal_Int64 SAL_CALL OQueryDescriptor::getSomething( const Sequence< sal_Int8 >& _rIdentifier ) throw(RuntimeException)
{
    if (_rIdentifier.getLength() != 16)
        return NULL;

    if (0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  _rIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return NULL;
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OQueryDescriptor::getUnoTunnelImplementationId()
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

//--------------------------------------------------------------------------
Any SAL_CALL OQueryDescriptor::queryInterface( const Type& _rType ) throw(RuntimeException)
{
    Any aReturn = ::cppu::queryInterface(_rType,
        static_cast< XPropertySet* >(this),
        static_cast< XFastPropertySet* >(this),
        static_cast< XMultiPropertySet* >(this)
    );

    if (!aReturn.hasValue())
        aReturn = OQueryDescriptor_Base::queryInterface(_rType);

    return aReturn;
}

//--------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL OQueryDescriptor::getColumns( ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    // TODO : if we have any kind of late initialisation for the columns : do it here
    return Reference< XNameAccess >(&m_aColumns);
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryDescriptor::getImplementationName(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.sdb.OQueryDescriptor");
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OQueryDescriptor::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OQueryDescriptor::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(2);
    aSupported.getArray()[0] = SERVICE_SDB_DATASETTINGS;
    aSupported.getArray()[1] = SERVICE_SDB_QUERYDESCRIPTOR;
    return aSupported;
}

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

//--------------------------------------------------------------------------
void SAL_CALL OQueryDescriptor::dispose()
{
    MutexGuard aGuard(m_aMutex);
    m_aColumns.disposing();
}

//--------------------------------------------------------------------------
void OQueryDescriptor::storeTo(const OConfigurationTreeRoot& _rConfigLocation)
{
    MutexGuard aGuard(m_aMutex);
    if (!_rConfigLocation.isValid() || _rConfigLocation.isReadonly())
    {
        OSL_ENSURE(sal_False, "OQueryDescriptor::storeTo : invalid config key (NULL or readonly) !");
        return;
    }

    // -----------------
    // the data settings
    OConfigurationNode aSettingsNode = _rConfigLocation.openNode(CONFIGKEY_SETTINGS);
    if (!aSettingsNode.isValid())
    {
        OSL_ENSURE(sal_False, "OQueryDescriptor::storeTo: could not open the sub key for the data settings!");
        return;
    }
    ODataSettings::storeTo(aSettingsNode);

    // ---------------------
    // the command base stuff
    OCommandBase::storeTo(_rConfigLocation);

    // --------------------------
    // the columns UI information
    OConfigurationNode aColumnsNode = _rConfigLocation.openNode(CONFIGKEY_QRYDESCR_COLUMNS);
    if (aColumnsNode.isValid())
    {
        m_aColumns.storeSettings(aColumnsNode);
        _rConfigLocation.commit();
    }
    else
        OSL_ENSURE(sal_False, "OQueryDescriptor::storeTo : could not open the node for the columns UI information !");
}

//--------------------------------------------------------------------------
void OQueryDescriptor::initializeFrom(const OConfigurationNode& _rConfigLocation)
{
    MutexGuard aGuard(m_aMutex);
    if (!_rConfigLocation.isValid())
    {
        OSL_ENSURE(sal_False, "OQueryDescriptor::initializeFrom : invalid config key (NULL or readonly) !");
        return;
    }

    // -----------------
    // the data settings
    OConfigurationNode aSettingsNode = _rConfigLocation.openNode(CONFIGKEY_SETTINGS);
    if (!aSettingsNode.isValid())
    {
        OSL_ENSURE(sal_False, "OQueryDescriptor::initializeFrom: could not open the sub key for the data settings!");
        return;
    }
    ODataSettings::loadFrom(aSettingsNode);

    // ---------------------
    // the command base stuff
    OCommandBase::initializeFrom(_rConfigLocation);

    readColumnSettings(_rConfigLocation);
}

//------------------------------------------------------------------------------
OColumn* OQueryDescriptor::createColumn(const ::rtl::OUString& _rName) const
{
    return new OTableColumn(_rName);
}
// -----------------------------------------------------------------------------
void OQueryDescriptor::readColumnSettings(const OConfigurationNode& _rConfigLocation)
{
    // --------------------------
    // the columns UI information
    OConfigurationNode aColumnsNode = _rConfigLocation.openNode(CONFIGKEY_QRYDESCR_COLUMNS);
    m_aColumns.clearColumns();
    if (aColumnsNode.isValid())
        m_aColumns.loadSettings(aColumnsNode, this);
}
// -----------------------------------------------------------------------------
//==========================================================================
//= ODescriptorColumn
//==========================================================================
ODescriptorColumn::ODescriptorColumn(const ::rtl::OUString& _rName)
{
    m_sName = _rName;
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< sal_Int8 > ODescriptorColumn::getImplementationId() throw (RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODescriptorColumn::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(7)
        DECL_PROP2(ALIGN,               sal_Int32,          BOUND, MAYBEVOID);
        DECL_PROP1_IFACE(CONTROLMODEL,  XPropertySet,       BOUND);
        DECL_PROP2(NUMBERFORMAT,        sal_Int32,          BOUND, MAYBEVOID);
        DECL_PROP1_BOOL(HIDDEN,                             BOUND);
        DECL_PROP1(NAME,                ::rtl::OUString,    READONLY);
        DECL_PROP2(RELATIVEPOSITION,    sal_Int32,          BOUND, MAYBEVOID);
        DECL_PROP2(WIDTH,               sal_Int32,          BOUND, MAYBEVOID);
    END_PROPERTY_HELPER();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODescriptorColumn::convertFastPropertyValue(Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue) throw (IllegalArgumentException)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_NAME:
            return OColumn::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
        default:
            return OColumnSettings::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODescriptorColumn::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue ) throw (Exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_NAME:
            OColumn::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
            break;
        default:
            OColumnSettings::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
            break;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL ODescriptorColumn::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    switch (_nHandle)
    {
        case PROPERTY_ID_NAME:
            OColumn::getFastPropertyValue(_rValue, _nHandle);
            break;
        default:
            OColumnSettings::getFastPropertyValue(_rValue, _nHandle);
            break;
    }
}

//........................................................................
}   // namespace dbaccess
//........................................................................


