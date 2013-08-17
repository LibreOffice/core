/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

// OComponentDefinition

DBG_NAME(OComponentDefinition)

void OComponentDefinition::initialize( const Sequence< Any >& aArguments ) throw(Exception)
{
    OUString rName;
    if( (aArguments.getLength() == 1) && (aArguments[0] >>= rName) )
    {
        Sequence< Any > aNewArgs(1);
        PropertyValue aValue;
        aValue.Name = PROPERTY_NAME;
        aValue.Value <<= rName;
        aNewArgs[0] <<= aValue;
        OContentHelper::initialize(aNewArgs);
    }
    else
        OContentHelper::initialize(aArguments);
}

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

OComponentDefinition::OComponentDefinition(const Reference< XComponentContext >& _xORB
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
                                       ,const OUString& _rElementName
                                       ,const Reference< XComponentContext >& _xORB
                                       ,const TContentPtr& _pImpl
                                       ,sal_Bool _bTable)
    :OContentHelper(_xORB,_rxContainer,_pImpl)
    ,ODataSettings(OContentHelper::rBHelper,!_bTable)
    ,m_bTable(_bTable)
{
    DBG_CTOR(OComponentDefinition, NULL);
    registerProperties();

    m_pImpl->m_aProps.aTitle = _rElementName;
    OSL_ENSURE(!m_pImpl->m_aProps.aTitle.isEmpty(), "OComponentDefinition::OComponentDefinition : invalid name !");
}

IMPLEMENT_IMPLEMENTATION_ID(OComponentDefinition);
IMPLEMENT_GETTYPES3(OComponentDefinition,ODataSettings,OContentHelper,OComponentDefinition_BASE);
IMPLEMENT_FORWARD_XINTERFACE3( OComponentDefinition,OContentHelper,ODataSettings,OComponentDefinition_BASE)

OUString OComponentDefinition::getImplementationName_static(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.dba.OComponentDefinition");
}

OUString SAL_CALL OComponentDefinition::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_static();
}

Sequence< OUString > OComponentDefinition::getSupportedServiceNames_static(  ) throw(RuntimeException)
{
    Sequence< OUString > aServices(2);
    aServices.getArray()[0] = OUString("com.sun.star.sdb.TableDefinition");
    aServices.getArray()[1] = OUString("com.sun.star.ucb.Content");

    return aServices;
}

Sequence< OUString > SAL_CALL OComponentDefinition::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_static();
}

Reference< XInterface > OComponentDefinition::Create( const Reference< XComponentContext >& _rxContext )
{
    return *(new OComponentDefinition( _rxContext, NULL, TContentPtr( new OComponentDefinition_Impl ) ) );
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

OUString OComponentDefinition::determineContentType() const
{
    return m_bTable
        ?   OUString( "application/vnd.org.openoffice.DatabaseTable" )
        :   OUString( "application/vnd.org.openoffice.DatabaseCommandDefinition" );
}

Reference< XNameAccess> OComponentDefinition::getColumns() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OContentHelper::rBHelper.bDisposed);

    if ( !m_pColumns.get() )
    {
        ::std::vector< OUString> aNames;

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

OColumn* OComponentDefinition::createColumn(const OUString& _rName) const
{
    const OComponentDefinition_Impl& rDefinition( getDefinition() );
    OComponentDefinition_Impl::const_iterator aFind = rDefinition.find( _rName );
    if ( aFind != rDefinition.end() )
    {
        aFind->second->addPropertyChangeListener(OUString(),m_xColumnPropertyListener.getRef());
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

void OComponentDefinition::columnDropped(const OUString& _sName)
{
    getDefinition().erase( _sName );
    notifyDataSourceModified();
}

void OComponentDefinition::columnAppended( const Reference< XPropertySet >& _rxSourceDescriptor )
{
    OUString sName;
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
