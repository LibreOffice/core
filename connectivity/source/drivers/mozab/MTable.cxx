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
#include "MTable.hxx"
#include "MTables.hxx"
#include "MColumns.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include "connectivity/dbtools.hxx"
#include "connectivity/TKeys.hxx"
#include "connectivity/TIndexes.hxx"
#include "MCatalog.hxx"


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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
