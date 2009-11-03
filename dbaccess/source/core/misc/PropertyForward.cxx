/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PropertyForward.cxx,v $
 * $Revision: 1.11 $
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

#include "PropertyForward.hxx"
#include "dbastrings.hrc"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>

#include <comphelper/property.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>


//........................................................................
namespace dbaccess
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::lang;

    DBG_NAME(OPropertyForward)

    //------------------------------------------------------------------------
    OPropertyForward::OPropertyForward( const Reference< XPropertySet>& _xSource, const Reference< XNameAccess>& _xDestContainer,
            const ::rtl::OUString& _sName, const ::std::vector< ::rtl::OUString>& _aPropertyList )
        :m_xSource( _xSource, UNO_SET_THROW )
        ,m_xDestContainer( _xDestContainer, UNO_SET_THROW )
        ,m_sName( _sName )
        ,m_bInInsert( sal_False )
    {
        DBG_CTOR(OPropertyForward,NULL);

        osl_incrementInterlockedCount(&m_refCount);
        try
        {
            if ( _aPropertyList.empty() )
                _xSource->addPropertyChangeListener( ::rtl::OUString(), this );
            else
            {
                ::std::vector< ::rtl::OUString >::const_iterator aIter = _aPropertyList.begin();
                ::std::vector< ::rtl::OUString >::const_iterator aEnd = _aPropertyList.end();
                for (; aIter != aEnd ; ++aIter )
                    _xSource->addPropertyChangeListener( *aIter, this );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        osl_decrementInterlockedCount( &m_refCount );
    }

    // -----------------------------------------------------------------------------
    OPropertyForward::~OPropertyForward()
    {
        DBG_DTOR(OPropertyForward,NULL);
    }

    // -----------------------------------------------------------------------------
    void SAL_CALL OPropertyForward::propertyChange( const PropertyChangeEvent& evt ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( !m_xDestContainer.is() )
            throw DisposedException( ::rtl::OUString(), *this );

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

    // -----------------------------------------------------------------------------
    void SAL_CALL OPropertyForward::disposing( const ::com::sun::star::lang::EventObject& /*_rSource*/ ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);

        if ( !m_xSource.is() )
            throw DisposedException( ::rtl::OUString(), *this );

        m_xSource->removePropertyChangeListener( ::rtl::OUString(), this );
        m_xSource = NULL;
        m_xDestContainer = NULL;
        m_xDestInfo = NULL;
        m_xDest = NULL;
    }

    // -----------------------------------------------------------------------------
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

//........................................................................
}   // namespace dbaccess
//........................................................................

