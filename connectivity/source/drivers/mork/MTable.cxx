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
using namespace connectivity::mork;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

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
