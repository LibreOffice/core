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
#include "precompiled_connectivity.hxx"
#include "adabas/BPreparedStatement.hxx"
#include "adabas/BResultSet.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/dbexception.hxx>

using namespace connectivity;
using namespace connectivity::adabas;
using namespace connectivity::odbc;
//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

OAdabasPreparedStatement::OAdabasPreparedStatement( OAdabasConnection* _pConnection,const ::rtl::OUString& sql)
: ::connectivity::odbc::OPreparedStatement( _pConnection,sql)
{
    m_aSelectColumns = _pConnection->createSelectColumns(sql);
}
// -----------------------------------------------------------------------------
OResultSet* OAdabasPreparedStatement::createResulSet()
{
    return new OAdabasResultSet(m_aStatementHandle,this,m_aSelectColumns);
}
// -----------------------------------------------------------------------------
void OAdabasPreparedStatement::setUsingBookmarks(sal_Bool /*_bUseBookmark*/)
{
    ::dbtools::throwFeatureNotImplementedException( "bookmarks", *this );
    // adabas doesn't support bookmarks
}
// -----------------------------------------------------------------------------
void OAdabasPreparedStatement::setResultSetConcurrency(sal_Int32 /*_par0*/)
{
    ::dbtools::throwFeatureNotImplementedException( "PreparedStatement:ResultSetConcurrency", *this );
}
// -----------------------------------------------------------------------------
void OAdabasPreparedStatement::setResultSetType(sal_Int32 /*_par0*/)
{
    ::dbtools::throwFeatureNotImplementedException( "PreparedStatement:ResultSetType", *this );
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
