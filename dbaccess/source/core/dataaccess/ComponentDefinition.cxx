/*************************************************************************
 *
 *  $RCSfile: ComponentDefinition.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:05:39 $
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

#ifndef DBA_COREDATAACESS_COMPONENTDEFINITION_HXX
#include "ComponentDefinition.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
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
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _DBACORE_DEFINITIONCOLUMN_HXX_
#include "definitioncolumn.hxx"
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

extern "C" void SAL_CALL createRegistryInfo_OComponentDefinition()
{
    static ::dbaccess::OMultiInstanceAutoRegistration< ::dbaccess::OComponentDefinition > aAutoRegistration;
}

//........................................................................
namespace dbaccess
{
//........................................................................
//==========================================================================
//= OComponentDefinition
//==========================================================================
//--------------------------------------------------------------------------
DBG_NAME(OComponentDefinition)
//--------------------------------------------------------------------------
void OComponentDefinition::registerProperties()
{
    OComponentDefinition_Impl* pItem = static_cast<OComponentDefinition_Impl*>(m_pImpl.get());
    OSL_ENSURE(pItem,"Illegal impl struct!");
    ODataSettings::registerProperties(pItem);

    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND | PropertyAttribute::READONLY|PropertyAttribute::CONSTRAINED,
                    &pItem->m_aProps.aTitle, ::getCppuType(&pItem->m_aProps.aTitle));

    if ( m_bTable )
    {
        registerProperty(PROPERTY_SCHEMANAME, PROPERTY_ID_SCHEMANAME, PropertyAttribute::BOUND,
                        &pItem->m_sSchemaName, ::getCppuType(&pItem->m_sSchemaName));

        registerProperty(PROPERTY_CATALOGNAME, PROPERTY_ID_CATALOGNAME, PropertyAttribute::BOUND,
                        &pItem->m_sCatalogName, ::getCppuType(&pItem->m_sCatalogName));
    }
}

//--------------------------------------------------------------------------
OComponentDefinition::OComponentDefinition(const Reference< XMultiServiceFactory >& _xORB
                                           ,const Reference< XInterface >&  _xParentContainer
                                           ,const TContentPtr& _pImpl
                                           ,sal_Bool _bTable)
    :ODataSettings(m_aBHelper)
    ,OContentHelper(_xORB,_xParentContainer,_pImpl)
    ,m_bTable(_bTable)
{
    DBG_CTOR(OComponentDefinition, NULL);
    registerProperties();
}
//--------------------------------------------------------------------------
OComponentDefinition::~OComponentDefinition()
{
    DBG_DTOR(OComponentDefinition, NULL);
}

//--------------------------------------------------------------------------
OComponentDefinition::OComponentDefinition( const Reference< XInterface >& _rxContainer
                                       ,const ::rtl::OUString& _rElementName
                                       ,const Reference< XMultiServiceFactory >& _xORB
                                       ,const TContentPtr& _pImpl
                                       ,sal_Bool _bTable)
    :ODataSettings(m_aBHelper)
    ,OContentHelper(_xORB,_rxContainer,_pImpl)
    ,m_bTable(_bTable)
{
    DBG_CTOR(OComponentDefinition, NULL);

    registerProperties();

    m_pImpl->m_aProps.aTitle = _rElementName;

    DBG_ASSERT(m_pImpl->m_aProps.aTitle.getLength() != 0, "OComponentDefinition::OComponentDefinition : invalid name !");
}

//--------------------------------------------------------------------------
IMPLEMENT_IMPLEMENTATION_ID(OComponentDefinition);
IMPLEMENT_GETTYPES3(OComponentDefinition,ODataSettings,OContentHelper,OComponentDefinition_BASE);
IMPLEMENT_FORWARD_XINTERFACE3( OComponentDefinition,OContentHelper,ODataSettings,OComponentDefinition_BASE)
//--------------------------------------------------------------------------
::rtl::OUString OComponentDefinition::getImplementationName_Static(  ) throw(RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.dba.OComponentDefinition"));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OComponentDefinition::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > OComponentDefinition::getSupportedServiceNames_Static(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aServices(2);
    aServices.getArray()[0] = SERVICE_SDB_TABLEDEFINITION;
    aServices.getArray()[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.Content"));

    return aServices;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OComponentDefinition::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
Reference< XInterface > OComponentDefinition::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new OComponentDefinition(_rxFactory,NULL,TContentPtr(new OComponentDefinition_Impl)));
}
// -----------------------------------------------------------------------------
void SAL_CALL OComponentDefinition::disposing()
{
    OContentHelper::disposing();
    if ( m_pColumns.get() )
        m_pColumns->disposing();
}
// -----------------------------------------------------------------------------
IPropertyArrayHelper& OComponentDefinition::getInfoHelper()
{
    return *getArrayHelper();
}
//--------------------------------------------------------------------------
IPropertyArrayHelper* OComponentDefinition::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new OPropertyArrayHelper(aProps);
}
//--------------------------------------------------------------------------
Reference< XPropertySetInfo > SAL_CALL OComponentDefinition::getPropertySetInfo(  ) throw(RuntimeException)
{
    Reference<XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}
// -----------------------------------------------------------------------------
Reference< XNameAccess> OComponentDefinition::getColumns() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OContentHelper::rBHelper.bDisposed);

    if ( !m_pColumns.get() )
    {
        OComponentDefinition_Impl* pItem = static_cast<OComponentDefinition_Impl*>(m_pImpl.get());
        OSL_ENSURE(pItem,"Invalid impl data!");
        ::std::vector< ::rtl::OUString> aNames;
        aNames.reserve(pItem->m_aColumnNames.size());
        OComponentDefinition_Impl::TColumnsIndexAccess::iterator aIter = pItem->m_aColumns.begin();
        OComponentDefinition_Impl::TColumnsIndexAccess::iterator aEnd = pItem->m_aColumns.end();
        for (; aIter != aEnd; ++aIter)
        {
            aNames.push_back((*aIter)->first);
        }
        m_pColumns.reset(new OColumns(*this, m_aMutex, sal_True, aNames, this,NULL,sal_True,sal_False,sal_False));
        m_pColumns->setParent(*this);
    }
    return m_pColumns.get();
}
// -----------------------------------------------------------------------------
OColumn* OComponentDefinition::createColumn(const ::rtl::OUString& _rName) const
{
    OComponentDefinition_Impl* pItem = static_cast<OComponentDefinition_Impl*>(m_pImpl.get());
    OSL_ENSURE(pItem,"Invalid impl data!");
    OComponentDefinition_Impl::TColumns::iterator aFind = pItem->m_aColumnNames.find(_rName);
    if ( aFind != pItem->m_aColumnNames.end() )
        return new OTableColumnWrapper(aFind->second,aFind->second,sal_True);
    return new OTableColumn(_rName);
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::beans::XPropertySet > OComponentDefinition::createEmptyObject()
{
    return new OTableColumnDescriptor();
}
// -----------------------------------------------------------------------------
void OComponentDefinition::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    ODataSettings::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    notifyDataSourceModified();
}
// -----------------------------------------------------------------------------
void OComponentDefinition::columnDropped(const ::rtl::OUString& _sName)
{
    OComponentDefinition_Impl* pItem = static_cast<OComponentDefinition_Impl*>(m_pImpl.get());
    OSL_ENSURE(pItem,"Invalid impl data!");
    OComponentDefinition_Impl::TColumns::iterator aFind = pItem->m_aColumnNames.find(_sName);
    if ( aFind != pItem->m_aColumnNames.end() )
    {
        pItem->m_aColumns.erase(::std::find(pItem->m_aColumns.begin(),pItem->m_aColumns.end(),aFind));
        pItem->m_aColumnNames.erase(aFind);
    }
    notifyDataSourceModified();
}
// -----------------------------------------------------------------------------
void OComponentDefinition::columnCloned(const Reference< XPropertySet >& _xClone)
{
    OSL_ENSURE(_xClone.is(),"Ivalid column!");
    ::rtl::OUString sName;
    _xClone->getPropertyValue(PROPERTY_NAME) >>= sName;
    OComponentDefinition_Impl* pItem = static_cast<OComponentDefinition_Impl*>(m_pImpl.get());
    OSL_ENSURE(pItem,"Invalid impl data!");
    Reference<XPropertySet> xProp = new OTableColumnDescriptor();
    ::comphelper::copyProperties(_xClone,xProp);
    pItem->m_aColumns.push_back(pItem->m_aColumnNames.insert(OComponentDefinition_Impl::TColumns::value_type(sName,xProp)).first);

    // helptext etc. may be modified
    notifyDataSourceModified();
}
//........................................................................
}   // namespace dbaccess
//........................................................................

