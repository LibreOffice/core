/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "ComponentDefinition.hxx"
#include "apitools.hxx"
#include "dbastrings.hrc"
#include "module_dba.hxx"

#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/property.hxx>
#include "definitioncolumn.hxx"
#include <cppuhelper/implbase1.hxx>
#include <comphelper/componentcontext.hxx>

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
    static ::dba::OAutoRegistration< ::dbaccess::OComponentDefinition > aAutoRegistration;
}

namespace dbaccess
{

/// helper class for column property change events which holds the OComponentDefinition weak
typedef ::cppu::WeakImplHelper1 < XPropertyChangeListener > TColumnPropertyListener_BASE;
class OColumnPropertyListener : public TColumnPropertyListener_BASE
{
    OComponentDefinition* m_pComponent;

    OColumnPropertyListener(const OColumnPropertyListener&);
    void operator =(const OColumnPropertyListener&);
protected:
    virtual ~OColumnPropertyListener(){}
public:
    OColumnPropertyListener(OComponentDefinition* _pComponent) : m_pComponent(_pComponent){}
    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const PropertyChangeEvent& /*_rEvent*/ ) throw (RuntimeException)
    {
        if ( m_pComponent )
            m_pComponent->notifyDataSourceModified();
    }
    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& /*_rSource*/ ) throw (RuntimeException)
    {
    }
    void clear() { m_pComponent = NULL; }
};
DBG_NAME(OComponentDefinition_Impl)
OComponentDefinition_Impl::OComponentDefinition_Impl()
{
    DBG_CTOR(OComponentDefinition_Impl,NULL);
}

OComponentDefinition_Impl::~OComponentDefinition_Impl()
{
    DBG_DTOR(OComponentDefinition_Impl,NULL);
}
//==========================================================================
//= OComponentDefinition
//==========================================================================

DBG_NAME(OComponentDefinition)

void OComponentDefinition::registerProperties()
{
    m_xColumnPropertyListener = ::comphelper::ImplementationReference<OColumnPropertyListener,XPropertyChangeListener>(new OColumnPropertyListener(this));
    OComponentDefinition_Impl& rDefinition( getDefinition() );
    ODataSettings::registerPropertiesFor( &rDefinition );

    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND | PropertyAttribute::READONLY|PropertyAttribute::CONSTRAINED,
                    &rDefinition.m_aProps.aTitle, ::getCppuType(&rDefinition.m_aProps.aTitle));

    if ( m_bTable )
    {
        registerProperty(PROPERTY_SCHEMANAME, PROPERTY_ID_SCHEMANAME, PropertyAttribute::BOUND,
                        &rDefinition.m_sSchemaName, ::getCppuType(&rDefinition.m_sSchemaName));

        registerProperty(PROPERTY_CATALOGNAME, PROPERTY_ID_CATALOGNAME, PropertyAttribute::BOUND,
                        &rDefinition.m_sCatalogName, ::getCppuType(&rDefinition.m_sCatalogName));
    }
}

OComponentDefinition::OComponentDefinition(const Reference< XMultiServiceFactory >& _xORB
                                           ,const Reference< XInterface >&  _xParentContainer
                                           ,const TContentPtr& _pImpl
                                           ,sal_Bool _bTable)
    :OContentHelper(_xORB,_xParentContainer,_pImpl)
    ,ODataSettings(OContentHelper::rBHelper,!_bTable)
    ,m_bTable(_bTable)
{
    DBG_CTOR(OComponentDefinition, NULL);
    registerProperties();
}

OComponentDefinition::~OComponentDefinition()
{
    DBG_DTOR(OComponentDefinition, NULL);
}


OComponentDefinition::OComponentDefinition( const Reference< XInterface >& _rxContainer
                                       ,const ::rtl::OUString& _rElementName
                                       ,const Reference< XMultiServiceFactory >& _xORB
                                       ,const TContentPtr& _pImpl
                                       ,sal_Bool _bTable)
    :OContentHelper(_xORB,_rxContainer,_pImpl)
    ,ODataSettings(OContentHelper::rBHelper,!_bTable)
    ,m_bTable(_bTable)
{
    DBG_CTOR(OComponentDefinition, NULL);
    registerProperties();

    m_pImpl->m_aProps.aTitle = _rElementName;
    OSL_ENSURE(m_pImpl->m_aProps.aTitle.getLength() != 0, "OComponentDefinition::OComponentDefinition : invalid name !");
}

IMPLEMENT_IMPLEMENTATION_ID(OComponentDefinition);
IMPLEMENT_GETTYPES3(OComponentDefinition,ODataSettings,OContentHelper,OComponentDefinition_BASE);
IMPLEMENT_FORWARD_XINTERFACE3( OComponentDefinition,OContentHelper,ODataSettings,OComponentDefinition_BASE)

::rtl::OUString OComponentDefinition::getImplementationName_static(  ) throw(RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.dba.OComponentDefinition"));
}

::rtl::OUString SAL_CALL OComponentDefinition::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_static();
}

Sequence< ::rtl::OUString > OComponentDefinition::getSupportedServiceNames_static(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aServices(2);
    aServices.getArray()[0] = SERVICE_SDB_TABLEDEFINITION;
    aServices.getArray()[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.Content"));

    return aServices;
}

Sequence< ::rtl::OUString > SAL_CALL OComponentDefinition::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_static();
}

Reference< XInterface > OComponentDefinition::Create( const Reference< XComponentContext >& _rxContext )
{
    ::comphelper::ComponentContext aContext( _rxContext );
    return *(new OComponentDefinition( aContext.getLegacyServiceFactory(), NULL, TContentPtr( new OComponentDefinition_Impl ) ) );
}

void SAL_CALL OComponentDefinition::disposing()
{
    OContentHelper::disposing();
    if ( m_pColumns.get() )
        m_pColumns->disposing();
    m_xColumnPropertyListener->clear();
    m_xColumnPropertyListener.dispose();
}

IPropertyArrayHelper& OComponentDefinition::getInfoHelper()
{
    return *getArrayHelper();
}

IPropertyArrayHelper* OComponentDefinition::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new OPropertyArrayHelper(aProps);
}

Reference< XPropertySetInfo > SAL_CALL OComponentDefinition::getPropertySetInfo(  ) throw(RuntimeException)
{
    Reference<XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::rtl::OUString OComponentDefinition::determineContentType() const
{
    return m_bTable
        ?   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.org.openoffice.DatabaseTable" ) )
        :   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.org.openoffice.DatabaseCommandDefinition" ) );
}

Reference< XNameAccess> OComponentDefinition::getColumns() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OContentHelper::rBHelper.bDisposed);

    if ( !m_pColumns.get() )
    {
        ::std::vector< ::rtl::OUString> aNames;

        const OComponentDefinition_Impl& rDefinition( getDefinition() );
        aNames.reserve( rDefinition.size() );

        OComponentDefinition_Impl::const_iterator aIter = rDefinition.begin();
        OComponentDefinition_Impl::const_iterator aEnd = rDefinition.end();
        for ( ; aIter != aEnd; ++aIter )
            aNames.push_back( aIter->first );

        m_pColumns.reset( new OColumns( *this, m_aMutex, sal_True, aNames, this, NULL, sal_True, sal_False, sal_False ) );
        m_pColumns->setParent( *this );
    }
    return m_pColumns.get();
}

OColumn* OComponentDefinition::createColumn(const ::rtl::OUString& _rName) const
{
    const OComponentDefinition_Impl& rDefinition( getDefinition() );
    OComponentDefinition_Impl::const_iterator aFind = rDefinition.find( _rName );
    if ( aFind != rDefinition.end() )
    {
        aFind->second->addPropertyChangeListener(::rtl::OUString(),m_xColumnPropertyListener.getRef());
        return new OTableColumnWrapper( aFind->second, aFind->second, true );
    }
    OSL_FAIL( "OComponentDefinition::createColumn: is this a valid case?" );
        // This here is the last place creating a OTableColumn, and somehow /me thinks it is not needed ...
    return new OTableColumn( _rName );
}

Reference< XPropertySet > OComponentDefinition::createColumnDescriptor()
{
    return new OTableColumnDescriptor( true );
}

void OComponentDefinition::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    ODataSettings::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    notifyDataSourceModified();
}

void OComponentDefinition::columnDropped(const ::rtl::OUString& _sName)
{
    getDefinition().erase( _sName );
    notifyDataSourceModified();
}

void OComponentDefinition::columnAppended( const Reference< XPropertySet >& _rxSourceDescriptor )
{
    ::rtl::OUString sName;
    _rxSourceDescriptor->getPropertyValue( PROPERTY_NAME ) >>= sName;

    Reference<XPropertySet> xColDesc = new OTableColumnDescriptor( true );
    ::comphelper::copyProperties( _rxSourceDescriptor, xColDesc );
    getDefinition().insert( sName, xColDesc );

    // formerly, here was a setParent at the xColDesc. The parent used was an adapter (ChildHelper_Impl)
    // which held another XChild weak, and forwarded all getParent requests to this other XChild.
    // m_pColumns was used for this. This was nonsense, since m_pColumns dies when our instance dies,
    // but xColDesc will live longer than this. So effectively, the setParent call was pretty useless.
    //
    // The intention for this parenting was that the column descriptor is able to find the data source,
    // by traveling up the parent hierachy until there's an XDataSource. This didn't work (which
    // for instance causes #i65023#). We need another way to properly ensure this.

    notifyDataSourceModified();
}

}   // namespace dbaccess
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
