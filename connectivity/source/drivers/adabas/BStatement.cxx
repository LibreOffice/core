/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BStatement.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:23:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CONNECTIVITY_ADABAS_STATEMENT_HXX
#include "adabas/BStatement.hxx"
#endif
#ifndef CONNECTIVITY_ADABAS_RESULTSET_HXX
#include "adabas/BResultSet.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif


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

typedef ::connectivity::odbc::OStatement OAdabasStatement_BASE;
// -----------------------------------------------------------------------------
OResultSet* OAdabasStatement::createResulSet()
{
    return new OAdabasResultSet(m_aStatementHandle,this,m_aSelectColumns);
}
// -----------------------------------------------------------------------------
void OAdabasStatement::setUsingBookmarks(sal_Bool _bUseBookmark)
{
    // adabas doesn't support bookmarks
}
// -----------------------------------------------------------------------------
void OAdabasStatement::setResultSetConcurrency(sal_Int32 _par0)
{
}
// -----------------------------------------------------------------------------
void OAdabasStatement::setResultSetType(sal_Int32 _par0)
{
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OAdabasStatement::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    m_aSelectColumns = m_pOwnConnection->createSelectColumns(sql);
    return OAdabasStatement_BASE::execute(sql);
}
// -----------------------------------------------------------------------------



