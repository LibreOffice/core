/*************************************************************************
 *
 *  $RCSfile: PropertyForward.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:15:57 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBA_PROPERTYSETFORWARD_HXX
#include "PropertyForward.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


//........................................................................
namespace dbaccess
{
//........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbcx;

DBG_NAME(OPropertyForward)
OPropertyForward::OPropertyForward(const Reference< XPropertySet>& _xSource
                                ,const Reference< XNameAccess>& _xDestContainer
                                ,const ::rtl::OUString& _sName
                                ,const ::std::vector< ::rtl::OUString>& _aPropertyList)
                                : m_xDestContainer(_xDestContainer)
                                , m_sName(_sName)
                                , m_xSource(_xSource)
                                , m_bInInsert(sal_False)
{
    DBG_CTOR(OPropertyForward,NULL);
    OSL_ENSURE(_xDestContainer.is(),"Destanation should be valid!");
    osl_incrementInterlockedCount(&m_refCount);
    {
        if ( _aPropertyList.empty() )
            _xSource->addPropertyChangeListener(::rtl::OUString(), this);
        else
        {
            ::std::vector< ::rtl::OUString>::const_iterator aIter = _aPropertyList.begin();
            ::std::vector< ::rtl::OUString>::const_iterator aEnd = _aPropertyList.end();
            for (; aIter != aEnd ; ++aIter )
                _xSource->addPropertyChangeListener(*aIter, this);
        }
    }
    osl_decrementInterlockedCount(&m_refCount);
}
// -----------------------------------------------------------------------------
OPropertyForward::~OPropertyForward()
{
    DBG_DTOR(OPropertyForward,NULL);
}
// -----------------------------------------------------------------------------
void SAL_CALL OPropertyForward::propertyChange( const PropertyChangeEvent& evt ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_xDestContainer.is() )
    {
        if ( m_xDestContainer->hasByName(m_sName) )
        {
            m_xDest.set(m_xDestContainer->getByName(m_sName),UNO_QUERY);
        }
        else
        {
            Reference<XDataDescriptorFactory> xFactory(m_xDestContainer,UNO_QUERY);
            if ( xFactory.is() )
            {
                m_xDest = xFactory->createDataDescriptor();
                ::comphelper::copyProperties(m_xSource,m_xDest);
                m_bInInsert = sal_True;
                Reference<XAppend> xAppend(m_xDestContainer,UNO_QUERY);
                if ( xAppend.is() )
                    xAppend->appendByDescriptor(m_xDest);
                m_bInInsert = sal_False;
            }
        }
        if ( m_xDest.is() )
            m_xDestInfo = m_xDest->getPropertySetInfo();
    }

    if ( m_xDestInfo.is() && m_xDestInfo->hasPropertyByName(evt.PropertyName) )
    {
        m_xDest->setPropertyValue(evt.PropertyName,evt.NewValue);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OPropertyForward::disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( m_xSource.is() )
    {
        m_xSource->removePropertyChangeListener(::rtl::OUString(), this);
        m_xSource = NULL;
    }
    m_xDestContainer = NULL;
    m_xDestInfo = NULL;
    m_xDest = NULL;
}
// -----------------------------------------------------------------------------
void OPropertyForward::setDefinition(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xDest)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( !m_bInInsert )
    {
        OSL_ENSURE( !m_xDest.is(),"Definition object is already set!");

        m_xDest = _xDest;
        if ( m_xDest.is() )
        {
            m_xDestInfo = m_xDest->getPropertySetInfo();
            ::comphelper::copyProperties(m_xDest,m_xSource);
        }
    }
}
//........................................................................
}   // namespace dbaccess
//........................................................................

