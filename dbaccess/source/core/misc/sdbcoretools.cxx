/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdbcoretools.cxx,v $
 * $Revision: 1.10 $
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

#ifndef DBACORE_SDBCORETOOLS_HXX
#include "sdbcoretools.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDOCUMENTDATASOURCE_HPP_
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif

//.........................................................................
namespace dbaccess
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

    // =========================================================================
    // -------------------------------------------------------------------------
    void notifyDataSourceModified(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxObject,sal_Bool _bModified)
    {
        Reference< XInterface > xDs = getDataSource( _rxObject );
        Reference<XDocumentDataSource> xDocumentDataSource(xDs,UNO_QUERY);
        if ( xDocumentDataSource.is() )
            xDs = xDocumentDataSource->getDatabaseDocument();
        Reference< XModifiable > xModi( xDs, UNO_QUERY );
        if ( xModi.is() )
            xModi->setModified(_bModified);
    }

    // -------------------------------------------------------------------------
    Reference< XInterface > getDataSource( const Reference< XInterface >& _rxDependentObject )
    {
        Reference< XInterface > xParent = _rxDependentObject;
        Reference< XInterface > xReturn;
        while( xParent.is() )
        {
            xReturn = xParent;
            Reference<XChild> xChild(xParent,UNO_QUERY);
            xParent.set(xChild.is() ? xChild->getParent() : Reference< XInterface >(),UNO_QUERY);
        }
        return xReturn;
    }

// -----------------------------------------------------------------------------
//.........................................................................
}   // namespace dbaccess
//.........................................................................

