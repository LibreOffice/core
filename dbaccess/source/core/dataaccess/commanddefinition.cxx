/*************************************************************************
 *
 *  $RCSfile: commanddefinition.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:40 $
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

#ifndef _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_
#include "commanddefinition.hxx"
#endif
#ifndef _DBA_CORE_REGISTRYHELPER_HXX_
#include "registryhelper.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::utl;
using namespace ::cppu;
using namespace dbaccess;

//==========================================================================
//= OCommandDefinition
//==========================================================================
extern "C" void SAL_CALL createRegistryInfo_OCommandDefinition()
{
    static OMultiInstanceAutoRegistration< OCommandDefinition > aAutoRegistration;
}

//--------------------------------------------------------------------------
DBG_NAME(OCommandDefinition)
//--------------------------------------------------------------------------
void OCommandDefinition::registerProperties()
{
    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND | PropertyAttribute::READONLY,
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
}

//--------------------------------------------------------------------------
OCommandDefinition::OCommandDefinition()
    :OSimplePropertyContainer(m_aBHelper)
    ,OConfigurationFlushable(m_aMutex)
{
    DBG_CTOR(OCommandDefinition, NULL);
    registerProperties();
}

//--------------------------------------------------------------------------
OCommandDefinition::~OCommandDefinition()
{
    DBG_DTOR(OCommandDefinition, NULL);
}

//--------------------------------------------------------------------------
OCommandDefinition::OCommandDefinition(const Reference< XInterface >& _rxContainer, const ::rtl::OUString& _rElementName,
            const Reference< XRegistryKey >& _rxConfigurationRoot)
    :OSimplePropertyContainer(m_aBHelper)
    ,OConfigurationFlushable(m_aMutex)
    ,m_xContainer(_rxContainer)
{
    DBG_CTOR(OCommandDefinition, NULL);

    registerProperties();

    m_sElementName = _rElementName;
    m_xConfigurationNode = _rxConfigurationRoot;

    DBG_ASSERT(m_xContainer.is(), "OCommandDefinition::OCommandDefinition : invalid container !");
    DBG_ASSERT(m_sElementName.getLength() != 0, "OCommandDefinition::OCommandDefinition : invalid name !");
    DBG_ASSERT(m_xConfigurationNode.is(), "OCommandDefinition::OCommandDefinition : invalid configuration node !");

    if (m_xConfigurationNode.is())
        initializeFromConfiguration();
}

//--------------------------------------------------------------------------
Any SAL_CALL OCommandDefinition::queryInterface( const Type& _rType ) throw(RuntimeException)
{
    Any aReturn = OCommandDefinition_Base::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = OSimplePropertyContainer::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = OConfigurationFlushable::queryInterface(_rType);

    return aReturn;
}

//--------------------------------------------------------------------------
::rtl::OUString OCommandDefinition::getImplementationName_Static(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.sdb.dbaccess.OCommandDefinition");
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OCommandDefinition::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > OCommandDefinition::getSupportedServiceNames_Static(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = SERVICE_SDB_QUERYDEFINITION;
    return aServices;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OCommandDefinition::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    return findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OCommandDefinition::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//------------------------------------------------------------------------------
Reference< XInterface > OCommandDefinition::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new OCommandDefinition());
}

//--------------------------------------------------------------------------
sal_Int64 SAL_CALL OCommandDefinition::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rIdentifier ) throw(::com::sun::star::uno::RuntimeException)
{
    if (_rIdentifier.getLength() != 16)
        return 0;

    // the implid of our base class
    if (0 == rtl_compareMemory(OContainerElement::getUnoTunnelImplementationId().getConstArray(),  _rIdentifier.getConstArray(), 16))
        return reinterpret_cast<sal_Int64>(static_cast<OContainerElement*>(this));

    return 0;
}

//------------------------------------------------------------------------------
void OCommandDefinition::flush_NoBroadcast(  ) throw(RuntimeException)
{
    if (!m_xConfigurationNode.is())
        throw DisposedException();

    OCommandBase::storeTo(m_xConfigurationNode);
}

//--------------------------------------------------------------------------
Reference< XPropertySetInfo > SAL_CALL OCommandDefinition::getPropertySetInfo(  ) throw(RuntimeException)
{
    staruno::Reference<starbeans::XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//--------------------------------------------------------------------------
IPropertyArrayHelper& OCommandDefinition::getInfoHelper()
{
    return *getArrayHelper();
}

//--------------------------------------------------------------------------
IPropertyArrayHelper* OCommandDefinition::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new OPropertyArrayHelper(aProps);
}

//--------------------------------------------------------------------------
void OCommandDefinition::inserted(const Reference< XInterface >& _rxContainer,
    const ::rtl::OUString& _rElementName,
    const Reference< XRegistryKey >& _rxConfigRoot)
{
    MutexGuard aGuard(m_aMutex);

    DBG_ASSERT(!m_xContainer.is(), "OCommandDefinition::inserted : invalid call : I'm already part of a container !");

    DBG_ASSERT(_rxContainer.is(), "OCommandDefinition::inserted : invalid container !");
    DBG_ASSERT(_rElementName.getLength() != 0, "OCommandDefinition::inserted : invalid name !");
    DBG_ASSERT(_rxConfigRoot.is(), "OCommandDefinition::inserted : invalid configuration node !");

    m_xContainer = _rxContainer;
    m_sElementName = _rElementName;
    m_xConfigurationNode = _rxConfigRoot;

    if (m_xConfigurationNode.is())
        initializeFromConfiguration();
}

//--------------------------------------------------------------------------
void OCommandDefinition::removed()
{
    MutexGuard aGuard(m_aMutex);

    DBG_ASSERT(m_xContainer.is(), "OCommandDefinition::inserted : invalid call : I'm not part of a container !");
    m_xContainer = NULL;
    m_sElementName = ::rtl::OUString();
    m_xConfigurationNode = NULL;
}

//--------------------------------------------------------------------------
void OCommandDefinition::initializeFromConfiguration()
{
    if (!m_xConfigurationNode.is())
    {
        DBG_ERROR("OCommandDefinition::initializeFromConfiguration : no configuration location !");
        return;
    }

    OCommandBase::initializeFrom(m_xConfigurationNode);
}


