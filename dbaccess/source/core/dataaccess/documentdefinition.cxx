/*************************************************************************
 *
 *  $RCSfile: documentdefinition.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-25 07:42:28 $
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

#ifndef _DBA_COREDATAACCESS_DOCUMENTDEFINITION_HXX_
#include "documentdefinition.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_CORE_REGISTRYHELPER_HXX_
#include "registryhelper.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= ODocumentDefinition
//==========================================================================
DBG_NAME(ODocumentDefinition)

extern "C" void SAL_CALL createRegistryInfo_ODocumentDefinition()
{
    static OMultiInstanceAutoRegistration< ODocumentDefinition > aAutoRegistration;
}

//--------------------------------------------------------------------------
ODocumentDefinition::ODocumentDefinition()
    :OConfigurationFlushable(m_aMutex)
    ,OPropertySetHelper(m_aBHelper)
    ,m_aFlushListeners(m_aMutex)
{
    DBG_CTOR(ODocumentDefinition, NULL);
}

//--------------------------------------------------------------------------
ODocumentDefinition::ODocumentDefinition(const Reference< XInterface >& _rxContainer, const ::rtl::OUString& _rElementName, const OConfigurationTreeRoot& _rObjectNode)
    :OConfigurationFlushable(m_aMutex)
    ,OPropertySetHelper(m_aBHelper)
    ,m_sElementName(_rElementName)
    ,m_aFlushListeners(m_aMutex)
{
    DBG_CTOR(ODocumentDefinition, NULL);

    DBG_ASSERT(m_xContainer.is(), "ODocumentDefinition::ODocumentDefinition : invalid container !");
    DBG_ASSERT(m_sElementName.getLength() != 0, "ODocumentDefinition::ODocumentDefinition : invalid name !");
    DBG_ASSERT(m_aConfigurationNode.isValid(), "ODocumentDefinition::ODocumentDefinition : invalid configuration node !");

    m_aConfigurationNode = _rObjectNode;
    if (m_aConfigurationNode.isValid())
        initializeFromConfiguration();
}

//--------------------------------------------------------------------------
ODocumentDefinition::~ODocumentDefinition()
{
    DBG_DTOR(ODocumentDefinition, NULL);
}

//--------------------------------------------------------------------------
Any SAL_CALL ODocumentDefinition::queryInterface( const Type& _rType ) throw(RuntimeException)
{
    Any aReturn = ::cppu::queryInterface(_rType,
        static_cast< XPropertySet* >(this),
        static_cast< XMultiPropertySet* >(this),
        static_cast< XFastPropertySet* >(this)
        );

    if (!aReturn.hasValue())
        aReturn = ODocumentDefinition_Base::queryInterface(_rType);

    return aReturn;
}

//--------------------------------------------------------------------------
::rtl::OUString ODocumentDefinition::getImplementationName_Static(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.sdb.dbaccess.ODocumentDefinition");
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODocumentDefinition::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > ODocumentDefinition::getSupportedServiceNames_Static(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.DocumentDefinition");
    return aServices;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL ODocumentDefinition::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    return findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODocumentDefinition::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//------------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODocumentDefinition::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new ODocumentDefinition());
}

//--------------------------------------------------------------------------
sal_Int64 SAL_CALL ODocumentDefinition::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rIdentifier ) throw(::com::sun::star::uno::RuntimeException)
{
    if (_rIdentifier.getLength() != 16)
        return 0;

    // the implid of our base class
    if (0 == rtl_compareMemory(OContainerElement::getUnoTunnelImplementationId().getConstArray(),  _rIdentifier.getConstArray(), 16))
        return reinterpret_cast<sal_Int64>(static_cast<OContainerElement*>(this));

    return 0;
}

//------------------------------------------------------------------------------

void ODocumentDefinition::flush_NoBroadcast_NoCommit(  )
{
    // only one property to write to the configuration
    m_aConfigurationNode.setNodeValue(CONFIGKEY_DBLINK_DOCUMENTLOCAITON, makeAny(m_sDocumentLocation));
}

//--------------------------------------------------------------------------
Reference< XPropertySetInfo > SAL_CALL ODocumentDefinition::getPropertySetInfo(  ) throw(RuntimeException)
{
    staruno::Reference<starbeans::XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//--------------------------------------------------------------------------
IPropertyArrayHelper& ODocumentDefinition::getInfoHelper()
{
    return *getArrayHelper();
}

//--------------------------------------------------------------------------
sal_Bool ODocumentDefinition::convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue) throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sElementName);
            break;
        case PROPERTY_ID_DOCUMENTLOCATION:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sDocumentLocation);
            break;
        default:
            DBG_ERROR("ODocumentDefinition::convertFastPropertyValue : unknown handle !");
    }
    return bModified;
}

//--------------------------------------------------------------------------
void ODocumentDefinition::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_DOCUMENTLOCATION:
            rValue >>= m_sDocumentLocation;
            break;
    }
}

//--------------------------------------------------------------------------
void ODocumentDefinition::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_DOCUMENTLOCATION:
            rValue <<= m_sDocumentLocation;
            break;
    }
}

//--------------------------------------------------------------------------
IPropertyArrayHelper* ODocumentDefinition::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(2)
        DECL_PROP1(DOCUMENTLOCATION,    ::rtl::OUString,    BOUND);
        DECL_PROP1(NAME,                ::rtl::OUString,    READONLY);
    END_PROPERTY_HELPER();
}

//--------------------------------------------------------------------------
void ODocumentDefinition::inserted(const Reference< XInterface >& _rxContainer,
    const ::rtl::OUString& _rElementName,
    const OConfigurationTreeRoot& _rConfigRoot)
{
    MutexGuard aGuard(m_aMutex);

    DBG_ASSERT(!m_xContainer.is(), "ODocumentDefinition::inserted : invalid call : I'm already part of a container !");

    DBG_ASSERT(_rxContainer.is(), "ODocumentDefinition::inserted : invalid container !");
    DBG_ASSERT(_rElementName.getLength() != 0, "ODocumentDefinition::inserted : invalid name !");
    DBG_ASSERT(_rConfigRoot.isValid(), "ODocumentDefinition::inserted : invalid configuration node !");

    m_xContainer = _rxContainer;
    m_sElementName = _rElementName;
    m_aConfigurationNode = _rConfigRoot;

    if (m_aConfigurationNode.isValid())
        flush_NoBroadcast_NoCommit();
}

//--------------------------------------------------------------------------
void ODocumentDefinition::removed()
{
    MutexGuard aGuard(m_aMutex);

    DBG_ASSERT(m_xContainer.is(), "ODocumentDefinition::removed: invalid call : I'm not part of a container !");

    m_xContainer = NULL;
    m_sElementName = ::rtl::OUString();
    m_aConfigurationNode.clear();
}

//--------------------------------------------------------------------------
void ODocumentDefinition::initializeFromConfiguration()
{
    if (!m_aConfigurationNode.isValid())
    {
        DBG_ERROR("ODocumentDefinition::initializeFromConfiguration : no configuration location !");
        return;
    }

    m_aConfigurationNode.getNodeValue(CONFIGKEY_DBLINK_DOCUMENTLOCAITON) >>= m_sDocumentLocation;
}

//........................................................................
}   // namespace dbaccess
//........................................................................

