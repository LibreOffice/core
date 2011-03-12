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
#include "calc/CTables.hxx"
#include "calc/CTable.hxx"
#include "file/FCatalog.hxx"
#include "file/FConnection.hxx"
#include "calc/CCatalog.hxx"
#include <comphelper/types.hxx>
#include <rtl/logfile.hxx>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::calc;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
namespace starutil      = ::com::sun::star::util;

sdbcx::ObjectType OCalcTables::createObject(const ::rtl::OUString& _rName)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcTables::createObject" );
    OCalcTable* pTable = new OCalcTable(this,(OCalcConnection*)static_cast<OFileCatalog&>(m_rParent).getConnection(),
                                        _rName,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE")));
    sdbcx::ObjectType xRet = pTable;
    pTable->construct();
    return xRet;
}
// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
