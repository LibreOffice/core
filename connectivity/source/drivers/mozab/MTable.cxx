/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MTable.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2006-10-18 13:09:27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#ifndef _CONNECTIVITY_MOZAB_TABLE_HXX_
#include "MTable.hxx"
#endif
#ifndef _CONNECTIVITY_MOZAB_TABLES_HXX_
#include "MTables.hxx"
#endif
#ifndef _CONNECTIVITY_MOZAB_COLUMNS_HXX_
#include "MColumns.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef CONNECTIVITY_TKEYS_HXX
#include "connectivity/TKeys.hxx"
#endif
#ifndef CONNECTIVITY_INDEXESHELPER_HXX
#include "connectivity/TIndexes.hxx"
#endif
#ifndef _CONNECTIVITY_MOZAB_CATALOG_HXX_
#include "MCatalog.hxx"
#endif


using namespace ::comphelper;
using namespace connectivity::mozab;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OTable::OTable( sdbcx::OCollection* _pTables, OConnection* _pConnection)
    :OTable_Base( _pTables, _pConnection, sal_True )
    ,m_pConnection( _pConnection )
{
    construct();
}
// -------------------------------------------------------------------------
OTable::OTable( sdbcx::OCollection* _pTables, OConnection* _pConnection,
                const ::rtl::OUString& _Name, const ::rtl::OUString& _Type, const ::rtl::OUString& _Description )
    :OTable_Base(_pTables, _pConnection, sal_True, _Name, _Type, _Description )
    ,m_pConnection( _pConnection )
{
    construct();
}

//--------------------------------------------------------------------------
sdbcx::OCollection* OTable::createColumns( const TStringVector& _rNames )
{
    return new OColumns( this, m_aMutex, _rNames );
}

//--------------------------------------------------------------------------
sdbcx::OCollection* OTable::createKeys(const TStringVector& _rNames)
{
    return new OKeysHelper( this, m_aMutex, _rNames );
}

//--------------------------------------------------------------------------
sdbcx::OCollection* OTable::createIndexes(const TStringVector& _rNames)
{
    return new OIndexesHelper( this, m_aMutex, _rNames );
}

// -----------------------------------------------------------------------------



