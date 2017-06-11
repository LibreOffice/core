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
#include "stringconstants.hxx"

#include <osl/diagnose.h>
#include <comphelper/sequence.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/property.hxx>
#include "definitioncolumn.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::cppu;

namespace dbaccess
{

/// helper class for column property change events which holds the OComponentDefinition weak
class OColumnPropertyListener:
    public ::cppu::WeakImplHelper< XPropertyChangeListener >
{
    OComponentDefinition* m_pComponent;
protected:
    virtual ~OColumnPropertyListener() override {}
public:
    explicit OColumnPropertyListener(OComponentDefinition* _pComponent) : m_pComponent(_pComponent){}
    OColumnPropertyListener(const OColumnPropertyListener&) = delete;
    const OColumnPropertyListener& operator=(const OColumnPropertyListener&) = delete;
    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const PropertyChangeEvent& /*_rEvent*/ ) override
    {
        if ( m_pComponent )
            m_pComponent->notifyDataSourceModified();
    }
    // XEventListener
    virtual void SAL_CALL disposing( const EventObject& /*_rSource*/ ) override
    {
    }
    void clear() { m_pComponent = nullptr; }
};

OComponentDefinition_Impl::OComponentDefinition_Impl()
{
}

OComponentDefinition_Impl::~OComponentDefinition_Impl()
{
}

// OComponentDefinition


void OComponentDefinition::initialize( const Sequence< Any >& aArguments )
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
    m_xColumnPropertyListener = new OColumnPropertyListener(this);
    OComponentDefinition_Impl& rDefinition( getDefinition() );
    ODataSettings::registerPropertiesFor( &rDefinition );

    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND | PropertyAttribute::READONLY|PropertyAttribute::CONSTRAINED,
                    &rDefinition.m_aProps.aTitle, cppu::UnoType<decltype(rDefinition.m_aProps.aTitle)>::get());

    if ( m_bTable )
    {
        registerProperty(PROPERTY_SCHEMANAME, PROPERTY_ID_SCHEMANAME, PropertyAttribute::BOUND,
                        &rDefinition.m_sSchemaName, cppu::UnoType<decltype(rDefinition.m_sSchemaName)>::get());

        registerProperty(PROPERTY_CATALOGNAME, PROPERTY_ID_CATALOGNAME, PropertyAttribute::BOUND,
                        &rDefinition.m_sCatalogName, cppu::UnoType<decltype(rDefinition.m_sCatalogName)>::get());
    }
}

OComponentDefinition::OComponentDefinition(const Reference< XComponentContext >& _xORB
                                           ,const Reference< XInterface >&  _xParentContainer
                                           ,const TContentPtr& _pImpl
                                           ,bool _bTable)
    :OContentHelper(_xORB,_xParentContainer,_pImpl)
    ,ODataSettings(OContentHelper::rBHelper,!_bTable)
    ,m_bTable(_bTable)
{
    registerProperties();
}

OComponentDefinition::~OComponentDefinition()
{
}

OComponentDefinition::OComponentDefinition( const Reference< XInterface >& _rxContainer
                                       ,const OUString& _rElementName
                                       ,const Reference< XComponentContext >& _xORB
                                       ,const TContentPtr& _pImpl
                                       ,bool _bTable)
    :OContentHelper(_xORB,_rxContainer,_pImpl)
    ,ODataSettings(OContentHelper::rBHelper,!_bTable)
    ,m_bTable(_bTable)
{
    registerProperties();

    m_pImpl->m_aProps.aTitle = _rElementName;
    OSL_ENSURE(!m_pImpl->m_aProps.aTitle.isEmpty(), "OComponentDefinition::OComponentDefinition : invalid name !");
}

css::uno::Sequence<sal_Int8> OComponentDefinition::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

IMPLEMENT_GETTYPES3(OComponentDefinition,ODataSettings,OContentHelper,OComponentDefinition_BASE);
IMPLEMENT_FORWARD_XINTERFACE3( OComponentDefinition,OContentHelper,ODataSettings,OComponentDefinition_BASE)

OUString SAL_CALL OComponentDefinition::getImplementationName()
{
    return OUString("com.sun.star.comp.dba.OComponentDefinition");
}

Sequence< OUString > SAL_CALL OComponentDefinition::getSupportedServiceNames()
{
    return { "com.sun.star.sdb.TableDefinition", "com.sun.star.ucb.Content" };
}

void SAL_CALL OComponentDefinition::disposing()
{
    OContentHelper::disposing();
    if ( m_xColumns.is() )
        m_xColumns->disposing();
    m_xColumns.clear();
    m_xColumnPropertyListener->clear();
    m_xColumnPropertyListener.clear();
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

Reference< XPropertySetInfo > SAL_CALL OComponentDefinition::getPropertySetInfo(  )
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

Reference< XNameAccess> OComponentDefinition::getColumns()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OContentHelper::rBHelper.bDisposed);

    if ( !m_xColumns.is() )
    {
        std::vector< OUString> aNames;

        const OComponentDefinition_Impl& rDefinition( getDefinition() );
        aNames.reserve( rDefinition.size() );

        OComponentDefinition_Impl::const_iterator aIter = rDefinition.begin();
        OComponentDefinition_Impl::const_iterator aEnd = rDefinition.end();
        for ( ; aIter != aEnd; ++aIter )
            aNames.push_back( aIter->first );

        m_xColumns = new OColumns( *this, m_aMutex, true, aNames, this, nullptr, true, false, false );
        m_xColumns->setParent( *this );
    }
    return m_xColumns.get();
}

OColumn* OComponentDefinition::createColumn(const OUString& _rName) const
{
    const OComponentDefinition_Impl& rDefinition( getDefinition() );
    OComponentDefinition_Impl::const_iterator aFind = rDefinition.find( _rName );
    if ( aFind != rDefinition.end() )
    {
        aFind->second->addPropertyChangeListener(OUString(),m_xColumnPropertyListener.get());
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

void OComponentDefinition::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
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
    // by traveling up the parent hierarchy until there's an XDataSource. This didn't work (which
    // for instance causes #i65023#). We need another way to properly ensure this.

    notifyDataSourceModified();
}

}   // namespace dbaccess

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_dba_OComponentDefinition(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new dbaccess::OComponentDefinition(
            context, nullptr, dbaccess::TContentPtr(new dbaccess::OComponentDefinition_Impl)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
