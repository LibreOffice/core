/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ContainerMediator.cxx,v $
 * $Revision: 1.11.42.1 $
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
#ifndef DBA_CONTAINERMEDIATOR_HXX
#include "ContainerMediator.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef DBA_PROPERTYSETFORWARD_HXX
#include "PropertyForward.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#include <com/sun/star/sdbcx/XRename.hpp>
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif


//........................................................................
namespace dbaccess
{
//........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

DBG_NAME(OContainerMediator)
OContainerMediator::OContainerMediator( const Reference< XContainer >& _xContainer, const Reference< XNameAccess >& _xSettings,
    const Reference< XConnection >& _rxConnection, ContainerType _eType )
    : m_xSettings( _xSettings )
    , m_xContainer( _xContainer )
    , m_aConnection( _rxConnection )
    , m_eType( _eType )
{
    DBG_CTOR(OContainerMediator,NULL);

    if ( _xSettings.is() && _xContainer.is() )
    {
        osl_incrementInterlockedCount(&m_refCount);
        try
        {
            m_xContainer->addContainerListener(this);
            Reference< XContainer > xContainer(_xSettings, UNO_QUERY);
            if ( xContainer.is() )
                xContainer->addContainerListener(this);
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OContainerMediator::OContainerMediator: caught an exception!");
        }
        osl_decrementInterlockedCount( &m_refCount );
    }
    else
    {
        m_xSettings.clear();
        m_xContainer.clear();
    }
}
// -----------------------------------------------------------------------------
OContainerMediator::~OContainerMediator()
{
    DBG_DTOR(OContainerMediator,NULL);
    acquire();
    impl_cleanup_nothrow();
}

// -----------------------------------------------------------------------------
void OContainerMediator::impl_cleanup_nothrow()
{
    try
    {
        Reference< XContainer > xContainer( m_xSettings, UNO_QUERY );
        if ( xContainer.is() )
            xContainer->removeContainerListener( this );
        m_xSettings.clear();

        xContainer = m_xContainer;
        if ( xContainer.is() )
            xContainer->removeContainerListener( this );
        m_xContainer.clear();//WeakReference< XContainer >();

        m_aForwardList.clear();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
void SAL_CALL OContainerMediator::elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( _rEvent.Source == m_xSettings && m_xSettings.is() )
    {
        ::rtl::OUString sElementName;
        _rEvent.Accessor >>= sElementName;
        PropertyForwardList::iterator aFind = m_aForwardList.find(sElementName);
        if ( aFind != m_aForwardList.end() )
        {
            Reference< XPropertySet> xDest(_rEvent.Element,UNO_QUERY);
            aFind->second->setDefinition( xDest );
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OContainerMediator::elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    Reference< XContainer > xContainer = m_xContainer;
    if ( _rEvent.Source == xContainer && xContainer.is() )
    {
        ::rtl::OUString sElementName;
        _rEvent.Accessor >>= sElementName;
        m_aForwardList.erase(sElementName);
        try
        {
            Reference<XNameContainer> xNameContainer( m_xSettings, UNO_QUERY );
            if ( xNameContainer.is() && m_xSettings->hasByName( sElementName ) )
                xNameContainer->removeByName( sElementName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OContainerMediator::elementReplaced( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    Reference< XContainer > xContainer = m_xContainer;
    if ( _rEvent.Source == xContainer && xContainer.is() )
    {
        ::rtl::OUString sElementName;
        _rEvent.ReplacedElement >>= sElementName;

        PropertyForwardList::iterator aFind = m_aForwardList.find(sElementName);
        if ( aFind != m_aForwardList.end() )
        {
            ::rtl::OUString sNewName;
            _rEvent.Accessor >>= sNewName;
            try
            {
                Reference<XNameContainer> xNameContainer( m_xSettings, UNO_QUERY_THROW );
                if ( xNameContainer.is() && m_xSettings->hasByName( sElementName ) )
                {
                    Reference<XRename> xSource(m_xSettings->getByName(sElementName),UNO_QUERY_THROW);
                    xSource->rename(sNewName);
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            aFind->second->setName(sNewName);
        }
    }
}

// -----------------------------------------------------------------------------
void SAL_CALL OContainerMediator::disposing( const EventObject& /*Source*/ ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

//    Reference< XContainer > xContainer = m_xContainer;
//  if ( Source.Source == xContainer || Source.Source == m_xSettings )
    // this can only be one of them :-) So no check needed here
    impl_cleanup_nothrow();
}

// -----------------------------------------------------------------------------
Reference< XPropertySet > OContainerMediator::impl_getSettingsForInitialization_nothrow( const ::rtl::OUString& _rName,
    const Reference< XPropertySet >& _rxDestination ) const
{
    Reference< XPropertySet > xSettings;
    try
    {
        if ( m_xSettings.is() && m_xSettings->hasByName( _rName ) )
            OSL_VERIFY( m_xSettings->getByName( _rName ) >>= xSettings );
        else if ( m_eType == eColumns )
        {
            do  // artifial loop for easier flow control
            {

            Reference< XConnection > xConnection( m_aConnection );
            if ( !xConnection.is() )
                break;

            Reference< XPropertySetInfo > xPSI( _rxDestination->getPropertySetInfo(), UNO_QUERY_THROW );
            if  (   !xPSI->hasPropertyByName( PROPERTY_TABLENAME )
                ||  !xPSI->hasPropertyByName( PROPERTY_REALNAME )
                )
                break;

            // determine the composed table name, plus the column name, as indicated by the
            // respective properties at the destination object
            ::rtl::OUString sCatalog, sSchema, sTable, sColumn;
            if ( xPSI->hasPropertyByName( PROPERTY_CATALOGNAME ) )
            {
                OSL_VERIFY( _rxDestination->getPropertyValue( PROPERTY_CATALOGNAME ) >>= sCatalog );
            }
            if ( xPSI->hasPropertyByName( PROPERTY_SCHEMANAME ) )
            {
                OSL_VERIFY( _rxDestination->getPropertyValue( PROPERTY_SCHEMANAME ) >>= sSchema );
            }
            OSL_VERIFY( _rxDestination->getPropertyValue( PROPERTY_TABLENAME ) >>= sTable );
            OSL_VERIFY( _rxDestination->getPropertyValue( PROPERTY_REALNAME ) >>= sColumn );

            ::rtl::OUString sComposedTableName = ::dbtools::composeTableName(
                xConnection->getMetaData(), sCatalog, sSchema, sTable, sal_False, ::dbtools::eComplete );

            // retrieve the table in question
            Reference< XTablesSupplier > xSuppTables( xConnection, UNO_QUERY_THROW );
            Reference< XNameAccess > xTables( xSuppTables->getTables(), UNO_QUERY_THROW );
            if ( !xTables->hasByName( sComposedTableName ) )
                break;

            Reference< XColumnsSupplier > xSuppCols( xTables->getByName( sComposedTableName ), UNO_QUERY_THROW );
            Reference< XNameAccess > xColumns( xSuppCols->getColumns(), UNO_QUERY_THROW );
            if ( !xColumns->hasByName( sColumn ) )
                break;

            xSettings.set( xColumns->getByName( sColumn ), UNO_QUERY );

            }
            while ( false );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return xSettings;
}

// -----------------------------------------------------------------------------
void OContainerMediator::notifyElementCreated(const ::rtl::OUString& _sName,const Reference<XPropertySet>& _xDest)
{
    PropertyForwardList::iterator aFind = m_aForwardList.find(_sName);
    if ( (aFind == m_aForwardList.end() || !aFind->second->getDefinition().is() )&& m_xSettings.is() )
    {
        ::std::vector< ::rtl::OUString> aPropertyList;

        try
        {
            // initially copy from the settings object (if existent) to the newly created object
            Reference< XPropertySet > xSetting( impl_getSettingsForInitialization_nothrow( _sName, _xDest ) );
            if ( xSetting.is() )
                ::comphelper::copyProperties( xSetting, _xDest );

            // collect the to-be-monitored properties
            Reference< XPropertySetInfo > xPSI( _xDest->getPropertySetInfo(), UNO_QUERY_THROW );
            Sequence< Property > aProperties( xPSI->getProperties() );
            const Property* property = aProperties.getConstArray();
            const Property* propertyEnd = aProperties.getConstArray() + aProperties.getLength();
            for ( ; property != propertyEnd; ++property )
            {
                if ( ( property->Attributes & PropertyAttribute::READONLY ) != 0 )
                    continue;
                if ( ( property->Attributes & PropertyAttribute::BOUND ) == 0 )
                    continue;

                aPropertyList.push_back( property->Name );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        ::rtl::Reference< OPropertyForward > pForward( new OPropertyForward( _xDest, m_xSettings, _sName, aPropertyList ) );
        m_aForwardList[_sName] = pForward;
    } // if ( aFind == m_aForwardList.end() && m_xSettings.is() )
}
// -----------------------------------------------------------------------------
//........................................................................
}   // namespace dbaccess
//........................................................................
