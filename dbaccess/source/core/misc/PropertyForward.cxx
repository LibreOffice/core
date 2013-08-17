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

#include "PropertyForward.hxx"
#include "dbastrings.hrc"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>

#include <comphelper/property.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

namespace dbaccess
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::lang;

    DBG_NAME(OPropertyForward)

    OPropertyForward::OPropertyForward( const Reference< XPropertySet>& _xSource, const Reference< XNameAccess>& _xDestContainer,
            const OUString& _sName, const ::std::vector< OUString>& _aPropertyList )
        :m_xSource( _xSource, UNO_SET_THROW )
        ,m_xDestContainer( _xDestContainer, UNO_SET_THROW )
        ,m_sName( _sName )
        ,m_bInInsert( sal_False )
    {
        DBG_CTOR(OPropertyForward,NULL);

        osl_atomic_increment(&m_refCount);
        try
        {
            if ( _aPropertyList.empty() )
                _xSource->addPropertyChangeListener( OUString(), this );
            else
            {
                ::std::vector< OUString >::const_iterator aIter = _aPropertyList.begin();
                ::std::vector< OUString >::const_iterator aEnd = _aPropertyList.end();
                for (; aIter != aEnd ; ++aIter )
                    _xSource->addPropertyChangeListener( *aIter, this );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        osl_atomic_decrement( &m_refCount );
    }

    OPropertyForward::~OPropertyForward()
    {
        DBG_DTOR(OPropertyForward,NULL);
    }

    void SAL_CALL OPropertyForward::propertyChange( const PropertyChangeEvent& evt ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !m_xDestContainer.is() )
            throw DisposedException( OUString(), *this );

        try
        {
            if ( !m_xDest.is() )
            {
                if ( m_xDestContainer->hasByName( m_sName ) )
                {
                    m_xDest.set( m_xDestContainer->getByName( m_sName ), UNO_QUERY_THROW );
                }
                else
                {
                    Reference< XDataDescriptorFactory > xFactory( m_xDestContainer, UNO_QUERY_THROW );
                    m_xDest.set( xFactory->createDataDescriptor(), UNO_SET_THROW );

                    ::comphelper::copyProperties( m_xSource, m_xDest );

                    m_bInInsert = sal_True;
                    Reference< XAppend > xAppend( m_xDestContainer, UNO_QUERY_THROW );
                    xAppend->appendByDescriptor( m_xDest );
                    m_bInInsert = sal_False;
                }

                m_xDestInfo.set( m_xDest->getPropertySetInfo(), UNO_SET_THROW );
            }

            if ( m_xDestInfo->hasPropertyByName( evt.PropertyName ) )
            {
                m_xDest->setPropertyValue( evt.PropertyName, evt.NewValue );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    void SAL_CALL OPropertyForward::disposing( const ::com::sun::star::lang::EventObject& /*_rSource*/ ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        if ( !m_xSource.is() )
            throw DisposedException( OUString(), *this );

        m_xSource->removePropertyChangeListener( OUString(), this );
        m_xSource = NULL;
        m_xDestContainer = NULL;
        m_xDestInfo = NULL;
        m_xDest = NULL;
    }

    void OPropertyForward::setDefinition( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xDest )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_bInInsert )
            return;

        OSL_ENSURE( !m_xDest.is(), "OPropertyForward::setDefinition: definition object is already set!" );
        try
        {
            m_xDest.set( _xDest, UNO_SET_THROW );
            m_xDestInfo.set( m_xDest->getPropertySetInfo(), UNO_SET_THROW );
            ::comphelper::copyProperties( m_xDest, m_xSource );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
