/*************************************************************************
 *
 *  $RCSfile: VCatalog.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-19 07:32:58 $
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

#ifndef _CONNECTIVITY_SDBCX_CATALOG_HXX_
#include "connectivity/sdbcx/VCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE dbtools
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_
#include "connectivity/sdbcx/VDescriptor.hxx"
#endif

using namespace connectivity::dbtools;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OCatalog,"com.sun.star.comp.connectivity.OCatalog","com.sun.star.sdbcx.DatabaseDefinition")
//------------------------------------------------------------------------------
OCatalog::OCatalog(const Reference< XConnection> &_xConnection) : OCatalog_BASE(m_aMutex)
            ,connectivity::OSubComponent<OCatalog>(_xConnection, this)
            ,m_pTables(NULL)
            ,m_pViews(NULL)
            ,m_pGroups(NULL)
            ,m_pUsers(NULL)
{
}
//-----------------------------------------------------------------------------
OCatalog::~OCatalog()
{
    delete m_pTables;
    delete m_pViews;
    delete m_pGroups;
    delete m_pUsers;
}
//-----------------------------------------------------------------------------
void SAL_CALL OCatalog::release() throw(RuntimeException)
{
    relase_ChildImpl();
}
//------------------------------------------------------------------------------
void SAL_CALL OCatalog::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pTables)
        m_pTables->disposing();
    if(m_pViews)
        m_pViews->disposing();
    if(m_pGroups)
        m_pGroups->disposing();
    if(m_pUsers)
        m_pUsers->disposing();

    dispose_ChildImpl();
    OCatalog_BASE::disposing();
}
//------------------------------------------------------------------------------
// XTablesSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getTables(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (OCatalog_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return const_cast<OCatalog*>(this)->m_pTables;
}
// -------------------------------------------------------------------------
// XViewsSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getViews(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (OCatalog_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return const_cast<OCatalog*>(this)->m_pViews;
}
// -------------------------------------------------------------------------
// XUsersSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getUsers(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (OCatalog_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return const_cast<OCatalog*>(this)->m_pUsers;
}
// -------------------------------------------------------------------------
// XGroupsSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getGroups(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (OCatalog_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return const_cast<OCatalog*>(this)->m_pGroups;
}
// -------------------------------------------------------------------------
void ODescriptor::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : ::com::sun::star::beans::PropertyAttribute::READONLY;
    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME ,nAttrib,&m_Name,::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}
// -------------------------------------------------------------------------
ODescriptor::~ODescriptor()
{
}
// -----------------------------------------------------------------------------

